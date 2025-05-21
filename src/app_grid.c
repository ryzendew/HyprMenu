#include "app_grid.h"
#include "category_list.h"
#include "app_entry.h"
#include "config.h"
#include <gio-unix-2.0/gio/gdesktopappinfo.h>
#include <gdk/gdk.h>
#include <unistd.h> // For sync() function

struct _HyprMenuAppGrid
{
  GtkBox parent_instance;
  
  GtkWidget *category_list;    // Grid view
  GtkWidget *list_view;        // List view
  GtkWidget *scrolled_window;
  GtkWidget *toggle_button;    // Toggle button for grid/list view
  GtkWidget *current_view;     // Points to either category_list or list_view
  
  GArray *app_entries;
  char *filter_text;
  
  // Add event controller for key events
  GtkEventController *key_controller;
};

G_DEFINE_TYPE (HyprMenuAppGrid, hyprmenu_app_grid, GTK_TYPE_BOX)

static void
close_window(GtkWidget *widget)
{
  GtkRoot *root = gtk_widget_get_root(widget);
  if (GTK_IS_WINDOW(root)) {
    gtk_window_close(GTK_WINDOW(root));
  }
}

static gboolean
on_key_pressed(GtkEventController *controller,
               guint keyval,
               guint keycode,
               GdkModifierType state,
               gpointer user_data)
{
  // Check if Super key was pressed and if configured to close on Super key
  if ((keyval == GDK_KEY_Super_L || keyval == GDK_KEY_Super_R) && 
      config->close_on_super_key) {
    HyprMenuAppGrid *self = HYPRMENU_APP_GRID(user_data);
    close_window(GTK_WIDGET(self));
    return TRUE;
  }
  return FALSE;
}

static void
on_click_outside(GtkGestureClick *gesture,
                 gint n_press,
                 double x,
                 double y,
                 gpointer user_data)
{
  // Only proceed if configured to close on click outside
  if (!config->close_on_click_outside) {
    return;
  }
  
  HyprMenuAppGrid *self = HYPRMENU_APP_GRID(user_data);
  GtkWidget *widget = GTK_WIDGET(self);
  graphene_rect_t bounds;
  
  // Get the widget's bounds
  if (!gtk_widget_compute_bounds(widget, widget, &bounds)) {
    return;  // Failed to compute bounds
  }
  
  // Check if the click is outside the widget's bounds
  if (x < bounds.origin.x || y < bounds.origin.y || 
      x > bounds.origin.x + bounds.size.width || 
      y > bounds.origin.y + bounds.size.height) {
    close_window(widget);
  }
}

static void
on_toggle_view_clicked(GtkButton *button, gpointer user_data)
{
  HyprMenuAppGrid *self = HYPRMENU_APP_GRID(user_data);
  g_print("Toggle button clicked! Current mode: %s\n", config->grid_hexpand ? "grid" : "list");
  hyprmenu_app_grid_toggle_view(self);
}

static void
hyprmenu_app_grid_finalize (GObject *object)
{
  HyprMenuAppGrid *self = HYPRMENU_APP_GRID (object);
  
  g_free (self->filter_text);
  
  if (self->app_entries) {
    for (guint i = 0; i < self->app_entries->len; i++) {
      HyprMenuAppEntry *entry = g_array_index(self->app_entries, HyprMenuAppEntry*, i);
      if (entry) {
        g_object_unref(entry);
      }
    }
    g_array_unref (self->app_entries);
  }
  
  G_OBJECT_CLASS (hyprmenu_app_grid_parent_class)->finalize (object);
}

static void
hyprmenu_app_grid_init (HyprMenuAppGrid *self)
{
  /* Initialize data */
  self->app_entries = g_array_new (FALSE, FALSE, sizeof (HyprMenuAppEntry *));
  g_array_set_clear_func (self->app_entries, (GDestroyNotify) g_object_unref);
  self->filter_text = NULL;
  
  /* Create UI */
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
  
  /* Create toggle button */
  self->toggle_button = gtk_button_new_from_icon_name(
    config->grid_hexpand ? "view-list-symbolic" : "view-grid-symbolic");
  gtk_widget_add_css_class(self->toggle_button, "flat");
  gtk_widget_set_tooltip_text(self->toggle_button, 
    config->grid_hexpand ? "Switch to List View" : "Switch to Grid View");
  
  // Make toggle button more visible with a distinct icon size
  GtkWidget *icon = gtk_button_get_child(GTK_BUTTON(self->toggle_button));
  if (GTK_IS_IMAGE(icon)) {
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 20); // Make icon slightly larger
  }
  
  g_signal_connect(self->toggle_button, "clicked", G_CALLBACK(on_toggle_view_clicked), self);
  g_print("Toggle button created with icon %s for mode: %s\n", 
          config->grid_hexpand ? "view-list-symbolic" : "view-grid-symbolic",
          config->grid_hexpand ? "grid" : "list");
  
  /* Create scrolled window */
  self->scrolled_window = gtk_scrolled_window_new ();
  gtk_widget_set_vexpand (self->scrolled_window, TRUE);
  gtk_widget_set_hexpand(self->scrolled_window, FALSE);
  gtk_widget_set_halign(self->scrolled_window, GTK_ALIGN_CENTER);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self->scrolled_window),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  
  /* Create both views */
  self->category_list = GTK_WIDGET(hyprmenu_category_list_new());
  gtk_widget_set_hexpand(self->category_list, FALSE);
  gtk_widget_set_halign(self->category_list, GTK_ALIGN_CENTER);
  self->list_view = hyprmenu_list_view_new();
  
  /* Set initial view based on config */
  if (config->grid_hexpand) {
    self->current_view = self->category_list;
    hyprmenu_category_list_set_grid_view(HYPRMENU_CATEGORY_LIST(self->category_list), TRUE);
  } else {
    self->current_view = self->list_view;
  }
  
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(self->scrolled_window), 
                               self->current_view);
  
  /* Add scrolled window to self */
  gtk_box_append(GTK_BOX(self), self->scrolled_window);
  
  // Add key controller for Super key
  self->key_controller = gtk_event_controller_key_new();
  g_signal_connect(self->key_controller, "key-pressed", G_CALLBACK(on_key_pressed), self);
  gtk_widget_add_controller(GTK_WIDGET(self), self->key_controller);
  
  // Add click gesture for click-outside
  GtkGesture *click_gesture = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), GDK_BUTTON_PRIMARY);
  g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_click_outside), self);
  gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(click_gesture));
}

static void
hyprmenu_app_grid_class_init (HyprMenuAppGridClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = hyprmenu_app_grid_finalize;
}

HyprMenuAppGrid *
hyprmenu_app_grid_new (void)
{
  return g_object_new (HYPRMENU_TYPE_APP_GRID, NULL);
}

void
hyprmenu_app_grid_refresh (HyprMenuAppGrid *self)
{
  g_print("hyprmenu_app_grid_refresh: Starting\n");
  
  if (!self) {
    g_warning("hyprmenu_app_grid_refresh: NULL self pointer");
    return;
  }
  
  /* Clear existing entries */
  if (self->app_entries) {
    for (guint i = 0; i < self->app_entries->len; i++) {
      HyprMenuAppEntry *entry = g_array_index(self->app_entries, HyprMenuAppEntry*, i);
      if (entry) {
        g_object_unref(entry);
      }
    }
    g_array_unref(self->app_entries);
  }
  
  /* Create a new array */
  self->app_entries = g_array_new(FALSE, FALSE, sizeof(HyprMenuAppEntry *));
  g_array_set_clear_func(self->app_entries, (GDestroyNotify)g_object_unref);
  
  /* Clear both views */
  hyprmenu_category_list_clear(HYPRMENU_CATEGORY_LIST(self->category_list));
  hyprmenu_list_view_clear(HYPRMENU_LIST_VIEW(self->list_view));
  
  /* Get all desktop apps */
  GList *all_apps = g_app_info_get_all();
  
  for (GList *l = all_apps; l != NULL; l = l->next) {
    GAppInfo *app_info = G_APP_INFO(l->data);
    
    if (!G_IS_DESKTOP_APP_INFO(app_info) || !g_app_info_should_show(app_info)) {
      continue;
    }
    
    const char *app_name = g_app_info_get_name(app_info);
    const char *app_id = g_app_info_get_id(app_info);
    
    if (!app_name || !app_id || app_name[0] == '\0' || app_id[0] == '\0') {
      continue;
    }
    
    /* Create app entry for the array */
    HyprMenuAppEntry *entry = hyprmenu_app_entry_new(G_DESKTOP_APP_INFO(app_info));
    if (!entry) continue;
    
    /* Add to both views */
    gboolean category_added = hyprmenu_category_list_add_app(
      HYPRMENU_CATEGORY_LIST(self->category_list), 
      G_DESKTOP_APP_INFO(app_info)
    );
    
    gboolean list_added = hyprmenu_list_view_add_app(
      HYPRMENU_LIST_VIEW(self->list_view), 
      G_DESKTOP_APP_INFO(app_info)
    );
    
    if (!category_added || !list_added) {
      g_warning("Failed to add app to one or both views: %s", app_id);
      g_object_unref(entry);
      continue;
    }
    
    /* Store in array */
    g_array_append_val(self->app_entries, entry);
  }
  
  g_list_free(all_apps);
  
  /* Apply current filter if any */
  if (self->filter_text) {
    hyprmenu_app_grid_filter(self, self->filter_text);
  }
}

void
hyprmenu_app_grid_filter (HyprMenuAppGrid *self, const char *search_text)
{
  g_return_if_fail(HYPRMENU_IS_APP_GRID(self));
  
  g_free(self->filter_text);
  self->filter_text = g_strdup(search_text);
  
  /* Apply filter to both views */
  gboolean category_filtered = hyprmenu_category_list_filter(
    HYPRMENU_CATEGORY_LIST(self->category_list), 
    search_text
  );
  
  gboolean list_filtered = hyprmenu_list_view_filter(
    HYPRMENU_LIST_VIEW(self->list_view), 
    search_text
  );
  
  if (!category_filtered || !list_filtered) {
    g_warning("Failed to apply filter to one or both views");
  }
}

void
hyprmenu_app_grid_toggle_view (HyprMenuAppGrid *self)
{
  g_return_if_fail(HYPRMENU_IS_APP_GRID(self));
  
  gboolean old_mode = config->grid_hexpand;
  
  g_print("Toggle view button clicked - changing from %s to %s\n", 
          old_mode ? "grid" : "list", 
          !old_mode ? "grid" : "list");
  
  /* Update config */
  config->grid_hexpand = !config->grid_hexpand;
  
  g_print("Config updated: grid_hexpand now = %s\n", config->grid_hexpand ? "true" : "false");
  
  /* Save configuration immediately */
  hyprmenu_config_save(config);
  g_print("Configuration saved successfully with view mode: %s\n", 
          config->grid_hexpand ? "grid" : "list");
  fsync(0);  // Use fsync on stdout instead of sync()
  
  /* Verify the config value was actually changed */
  g_print("Double-checking config->grid_hexpand = %s\n", config->grid_hexpand ? "true" : "false");
  
  /* Update toggle button */
  gtk_button_set_icon_name(GTK_BUTTON(self->toggle_button),
                          config->grid_hexpand ? "view-list-symbolic" : "view-grid-symbolic");
  gtk_widget_set_tooltip_text(self->toggle_button,
                             config->grid_hexpand ? "Switch to List View" : "Switch to Grid View");
  
  /* Switch views */
  GtkWidget *new_view;
  if (config->grid_hexpand) {
    g_print("Setting view to grid\n");
    new_view = self->category_list;
    hyprmenu_category_list_set_grid_view(HYPRMENU_CATEGORY_LIST(self->category_list), TRUE);
  } else {
    // Validate list view before switching
    if (!hyprmenu_list_view_is_valid(HYPRMENU_LIST_VIEW(self->list_view))) {
      g_warning("List view is not valid, falling back to grid view");
      config->grid_hexpand = TRUE;
      new_view = self->category_list;
      hyprmenu_category_list_set_grid_view(HYPRMENU_CATEGORY_LIST(self->category_list), TRUE);
      
      // Save the config again if we had to revert
      hyprmenu_config_save(config);
    } else {
      g_print("Setting view to list\n");
      new_view = self->list_view;
    }
  }
  
  if (new_view != self->current_view) {
    g_print("Switching current view\n");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(self->scrolled_window), new_view);
    self->current_view = new_view;
    
    /* Apply current filter to new view */
    if (self->filter_text) {
      hyprmenu_app_grid_filter(self, self->filter_text);
    }
  } else {
    g_print("View didn't change (new_view == current_view)\n");
  }
}

GtkWidget* hyprmenu_app_grid_get_toggle_button(HyprMenuAppGrid *self) {
  return self->toggle_button;
} 