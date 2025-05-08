#include "window.h"
#include "config.h"
#include <gtk4-layer-shell.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include "app_grid.h"
#include "recent_apps.h"

G_DEFINE_TYPE (HyprMenuWindow, hyprmenu_window, GTK_TYPE_APPLICATION_WINDOW)

static void
on_search_changed (GtkSearchEntry *entry,
                  HyprMenuWindow *self)
{
  g_print("on_search_changed: Search text changed\n");
  
  if (!entry) {
    g_warning("on_search_changed: Search entry is NULL");
    return;
  }
  
  if (!self) {
    g_warning("on_search_changed: Window is NULL");
    return;
  }
  
  const char *text = gtk_editable_get_text (GTK_EDITABLE (entry));
  g_print("on_search_changed: Search text is '%s'\n", text ? text : "NULL");
  
  if (!self->app_grid) {
    g_warning("on_search_changed: App grid is NULL");
    return;
  }
  
  hyprmenu_app_grid_filter (HYPRMENU_APP_GRID (self->app_grid), text);
  g_print("on_search_changed: Filter applied successfully\n");
}

static void
on_search_activate (GtkSearchEntry *entry,
                   HyprMenuWindow *self)
{
  // Handle search activation if needed
  (void)entry;
  (void)self;
}

static void
on_search_stop (GtkSearchEntry *entry,
                HyprMenuWindow *self)
{
  // Handle search stop if needed
  (void)entry;
  (void)self;
}

static void
on_focus_out (GtkWindow *window,
              GParamSpec *pspec,
              gpointer user_data)
{
  (void)pspec;
  (void)user_data;
  
  // Close the window when it loses focus
  if (config->close_on_focus_out) {
    gtk_window_close (window);
  }
}

static gboolean
on_key_press (GtkEventControllerKey *controller,
              guint                  keyval,
              guint                  keycode,
              GdkModifierType        state,
              HyprMenuWindow        *self)
{
  (void)controller;
  (void)keycode;
  (void)state;
  
  // Close window on Escape key press
  if (keyval == GDK_KEY_Escape) {
    GtkApplication *app = GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(self)));
    gtk_window_close(GTK_WINDOW(self));
    if (app) {
      g_application_quit(G_APPLICATION(app));
    }
    return TRUE;
  }
  
  // Close window on Super key press if configured
  if ((keyval == GDK_KEY_Super_L || keyval == GDK_KEY_Super_R) && 
      config->close_on_super_key) {
    gtk_window_close(GTK_WINDOW(self));
    return TRUE;
  }
  
  return FALSE;
}

static gboolean
on_key_release (GtkEventControllerKey *controller,
                guint                  keyval,
                guint                  keycode,
                GdkModifierType        state,
                HyprMenuWindow        *self)
{
  (void)controller;
  (void)keycode;
  (void)state;
  
  // Close window on Super key release if configured
  if ((keyval == GDK_KEY_Super_L || keyval == GDK_KEY_Super_R) && 
      config->close_on_super_key) {
    gtk_window_close (GTK_WINDOW (self));
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
  
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  GtkWidget *widget = GTK_WIDGET(self);
  GtkWidget *main_box = self->main_box;
  
  // Get the widget's allocation
  GtkAllocation allocation;
  gtk_widget_get_allocation(main_box, &allocation);
  
  // Convert coordinates to widget space
  double widget_x, widget_y;
  gtk_widget_translate_coordinates(widget, main_box, x, y, &widget_x, &widget_y);
  
  // Check if the click is outside the main box
  if (widget_x < 0 || widget_y < 0 || 
      widget_x > allocation.width || 
      widget_y > allocation.height) {
    gtk_window_close(GTK_WINDOW(self));
  }
}

static void
hyprmenu_window_init (HyprMenuWindow *self)
{
  /* Set window properties */
  gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (self), FALSE);
  gtk_window_set_default_size (GTK_WINDOW (self), config->window_width, config->window_height);
  
  /* Set up layer shell */
  gtk_layer_init_for_window (GTK_WINDOW (self));
  gtk_layer_set_layer (GTK_WINDOW (self), GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_set_keyboard_mode (GTK_WINDOW (self), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
  gtk_layer_set_exclusive_zone (GTK_WINDOW (self), -1);
  
  /* Position window based on config */
  if (config->center_window) {
    /* Center the window */
    gtk_layer_set_anchor (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_anchor (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
  } else {
    /* Position at top-left with margins */
    gtk_layer_set_anchor (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_anchor (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_margin (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_TOP, config->top_margin);
    gtk_layer_set_margin (GTK_WINDOW (self), GTK_LAYER_SHELL_EDGE_LEFT, config->left_margin);
  }
  
  /* Add CSS classes for styling */
  gtk_widget_add_css_class (GTK_WIDGET (self), "hyprmenu-window");
  
  /* Create main box */
  self->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class (self->main_box, "hyprmenu-main-box");
  gtk_widget_set_margin_start (self->main_box, 12);
  gtk_widget_set_margin_end (self->main_box, 12);
  gtk_widget_set_margin_top (self->main_box, 12);
  gtk_widget_set_margin_bottom (self->main_box, 12);
  gtk_window_set_child (GTK_WINDOW (self), self->main_box);
  
  /* Create search entry */
  self->search_entry = gtk_search_entry_new ();
  gtk_widget_add_css_class (self->search_entry, "hyprmenu-search");
  gtk_widget_set_hexpand (self->search_entry, TRUE);
  gtk_widget_set_margin_start (self->search_entry, 12);
  gtk_widget_set_margin_end (self->search_entry, 12);
  gtk_widget_set_margin_top (self->search_entry, 12);
  gtk_widget_set_margin_bottom (self->search_entry, 12);
  gtk_box_append (GTK_BOX (self->main_box), self->search_entry);
  
  /* Create app grid */
  self->app_grid = GTK_WIDGET (hyprmenu_app_grid_new ());
  gtk_widget_add_css_class (self->app_grid, "hyprmenu-app-grid");
  gtk_widget_set_hexpand (self->app_grid, TRUE);
  gtk_widget_set_vexpand (self->app_grid, TRUE);
  gtk_box_append (GTK_BOX (self->main_box), self->app_grid);
  
  /* Connect signals */
  g_signal_connect (self->search_entry, "search-changed",
                   G_CALLBACK (on_search_changed), self);
  g_signal_connect (self->search_entry, "activate",
                   G_CALLBACK (on_search_activate), self);
  g_signal_connect (self->search_entry, "stop-search",
                   G_CALLBACK (on_search_stop), self);
  
  /* Add key controller for Escape key and Super key */
  self->key_controller = gtk_event_controller_key_new();
  g_signal_connect(self->key_controller, "key-pressed", G_CALLBACK(on_key_press), self);
  g_signal_connect(self->key_controller, "key-released", G_CALLBACK(on_key_release), self);
  gtk_widget_add_controller(GTK_WIDGET(self), self->key_controller);
  
  /* Add click gesture for click-outside */
  GtkGesture *gesture = gtk_gesture_click_new();
  self->click_gesture = GTK_GESTURE_CLICK(gesture);
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(self->click_gesture), GDK_BUTTON_PRIMARY);
  gtk_gesture_single_set_exclusive(GTK_GESTURE_SINGLE(self->click_gesture), TRUE);
  g_signal_connect(self->click_gesture, "pressed", G_CALLBACK(on_click_outside), self);
  gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
  
  /* Connect focus-out signal */
  g_signal_connect(self, "notify::has-focus", G_CALLBACK(on_focus_out), NULL);
  
  /* Apply custom CSS from configuration */
  hyprmenu_config_apply_css();
  
  /* Set dark color scheme */
  GtkSettings *settings = gtk_settings_get_default();
  g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
  
  /* Focus search entry if configured */
  if (config->focus_search_on_open) {
    g_signal_connect(self, "map", G_CALLBACK(gtk_widget_grab_focus), self->search_entry);
  }
}

static void
hyprmenu_window_dispose(GObject *object)
{
  HyprMenuWindow *self = HYPRMENU_WINDOW(object);
  
  // Remove controllers before disposing
  if (self->key_controller) {
    gtk_widget_remove_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(self->key_controller));
    self->key_controller = NULL;
  }
  
  if (self->click_gesture) {
    gtk_widget_remove_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(self->click_gesture));
    self->click_gesture = NULL;
  }
  
  // Chain up
  G_OBJECT_CLASS(hyprmenu_window_parent_class)->dispose(object);
}

static void
hyprmenu_window_finalize(GObject *object)
{
  // Chain up
  G_OBJECT_CLASS(hyprmenu_window_parent_class)->finalize(object);
}

static void
hyprmenu_window_class_init(HyprMenuWindowClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  
  object_class->dispose = hyprmenu_window_dispose;
  object_class->finalize = hyprmenu_window_finalize;
}

HyprMenuWindow *
hyprmenu_window_new (GtkApplication *app)
{
  return g_object_new (HYPRMENU_TYPE_WINDOW,
                       "application", app,
                       NULL);
}

void
hyprmenu_window_show (HyprMenuWindow *self)
{
  hyprmenu_app_grid_refresh (HYPRMENU_APP_GRID (self->app_grid));
  gtk_window_present (GTK_WINDOW (self));
} 