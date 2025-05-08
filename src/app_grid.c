#include "app_grid.h"
#include "category_list.h"
#include "app_entry.h"
#include "config.h"
#include <gio-unix-2.0/gio/gdesktopappinfo.h>
#include <gdk/gdk.h>

struct _HyprMenuAppGrid
{
  GtkBox parent_instance;
  
  GtkWidget *category_list;
  GtkWidget *scrolled_window;
  GtkWidget *header_box;      // Header box for toggle button
  GtkWidget *toggle_button;   // Toggle button for grid/list view
  
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
  hyprmenu_app_grid_toggle_view(self);
}

static void
hyprmenu_app_grid_finalize (GObject *object)
{
  HyprMenuAppGrid *self = HYPRMENU_APP_GRID (object);
  
  g_free (self->filter_text);
  
  if (self->app_entries) {
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
  
  /* Create header box for toggle button */
  self->header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_halign(self->header_box, GTK_ALIGN_END);
  gtk_widget_set_margin_end(self->header_box, 8);
  gtk_widget_set_margin_top(self->header_box, 4);
  
  /* Create toggle button */
  self->toggle_button = gtk_button_new_from_icon_name(
    config->use_grid_view ? "view-list-symbolic" : "view-grid-symbolic");
  gtk_widget_add_css_class(self->toggle_button, "flat");
  gtk_widget_set_tooltip_text(self->toggle_button, 
    config->use_grid_view ? "Switch to List View" : "Switch to Grid View");
  g_signal_connect(self->toggle_button, "clicked", G_CALLBACK(on_toggle_view_clicked), self);
  
  gtk_box_append(GTK_BOX(self->header_box), self->toggle_button);
  gtk_box_append(GTK_BOX(self), self->header_box);
  
  /* Create scrolled window */
  self->scrolled_window = gtk_scrolled_window_new ();
  gtk_widget_set_vexpand (self->scrolled_window, TRUE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self->scrolled_window),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  
  /* Create category list */
  self->category_list = GTK_WIDGET (hyprmenu_category_list_new ());
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (self->scrolled_window), 
                                self->category_list);
  
  /* Set initial view mode */
  hyprmenu_category_list_set_grid_view(HYPRMENU_CATEGORY_LIST(self->category_list), 
                                      config->use_grid_view);
  
  /* Add scrolled window to self */
  gtk_box_append (GTK_BOX (self), self->scrolled_window);
  
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

  /* Clear existing array and create a new one */
  g_print("hyprmenu_app_grid_refresh: Clearing existing entries\n");
  
  if (self->app_entries) {
    g_array_unref(self->app_entries);
  }
  
  /* Create a new array */
  self->app_entries = g_array_new(FALSE, FALSE, sizeof(HyprMenuAppEntry *));
  g_array_set_clear_func(self->app_entries, (GDestroyNotify)g_object_unref);
  
  g_print("hyprmenu_app_grid_refresh: Created new app entries array\n");
  
  /* Get all desktop apps */
  g_print("hyprmenu_app_grid_refresh: Getting all applications\n");
  GList *all_apps = g_app_info_get_all();
  int total_apps = g_list_length(all_apps);
  g_print("hyprmenu_app_grid_refresh: Found %d applications total\n", total_apps);
  
  // Create a hash table to store apps by category
  GHashTable *category_apps = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  
  int valid_count = 0;
  int desktop_app_count = 0;
  int shown_app_count = 0;
  
  for (GList *l = all_apps; l != NULL; l = l->next) {
    GAppInfo *app_info = G_APP_INFO(l->data);
    
    /* Skip if not a desktop app */
    if (!G_IS_DESKTOP_APP_INFO(app_info)) {
      continue;
    }
    
    desktop_app_count++;
    
    /* Skip if shouldn't be shown */
    if (!g_app_info_should_show(app_info)) {
      continue;
    }
    
    shown_app_count++;
    
    /* Validate app info */
    const char *app_name = g_app_info_get_name(app_info);
    const char *app_id = g_app_info_get_id(app_info);
    
    g_print("Processing app: name=%s, id=%s\n", 
            app_name ? app_name : "(null)", 
            app_id ? app_id : "(null)");
    
    if (!app_name || !app_id || app_name[0] == '\0' || app_id[0] == '\0') {
      g_print("  Skipping app with invalid name or id\n");
      continue;
    }
    
    /* Create app entry */
    HyprMenuAppEntry *entry = hyprmenu_app_entry_new(G_DESKTOP_APP_INFO(app_info));
    
    /* Skip if entry creation failed */
    if (!entry) {
      g_print("  Entry creation failed for app: %s\n", app_name);
      continue;
    }
    
    /* Verify the entry was created properly */
    const char *entry_name = hyprmenu_app_entry_get_app_name(entry);
    if (!entry_name || entry_name[0] == '\0') {
      g_print("  Created entry has invalid name\n");
      g_object_unref(entry);
      continue;
    }
    
    /* Add to list */
    g_array_append_val(self->app_entries, entry);
    valid_count++;
    
    /* Get category and add to category hash table */
    const char *primary_category = "Other";
    const char **categories = hyprmenu_app_entry_get_categories(entry);
    
    if (categories && categories[0] && categories[0][0]) {
      primary_category = categories[0];
    }
    
    g_print("  Added app '%s' to category '%s'\n", entry_name, primary_category);
    
    // Get or create the list for this category
    GSList *category_list = g_hash_table_lookup(category_apps, primary_category);
    category_list = g_slist_append(category_list, entry);
    g_hash_table_insert(category_apps, g_strdup(primary_category), category_list);
  }
  
  g_print("hyprmenu_app_grid_refresh: Stats:\n");
  g_print("  Total apps: %d\n", total_apps);
  g_print("  Desktop apps: %d\n", desktop_app_count);
  g_print("  Should show apps: %d\n", shown_app_count);
  g_print("  Valid apps added: %d\n", valid_count);
  
  g_print("hyprmenu_app_grid_refresh: Added %d valid applications total\n", valid_count);
  g_list_free_full(all_apps, g_object_unref);
  
  // Get all category names and sort them alphabetically
  GList *category_names = g_hash_table_get_keys(category_apps);
  category_names = g_list_sort(category_names, (GCompareFunc)g_ascii_strcasecmp);
  
  g_print("hyprmenu_app_grid_refresh: Found %d categories\n", g_list_length(category_names));
  
  // Add categories in sorted order
  for (GList *l = category_names; l != NULL; l = l->next) {
    const char *category_name = l->data;
    GSList *apps = g_hash_table_lookup(category_apps, category_name);
    int app_count = g_slist_length(apps);
    
    g_print("  Category '%s' has %d apps\n", category_name, app_count);
    
    // Sort apps within the category alphabetically
    apps = g_slist_sort(apps, (GCompareFunc)hyprmenu_app_entry_compare_by_name);
    
    // Add each app to its category
    for (GSList *app_item = apps; app_item != NULL; app_item = app_item->next) {
      HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(app_item->data);
      if (entry) {
        const char *app_name = hyprmenu_app_entry_get_app_name(entry);
        g_print("    Adding app '%s' to category UI\n", app_name);
        
        hyprmenu_category_list_add_category(HYPRMENU_CATEGORY_LIST(self->category_list),
                                          category_name,
                                          GTK_WIDGET(entry));
      }
    }
    
    g_slist_free(apps);
  }
  
  g_list_free(category_names);
  g_hash_table_destroy(category_apps);
  
  /* Update display with filter */
  g_print("hyprmenu_app_grid_refresh: Refreshing display\n");
  hyprmenu_app_grid_filter(self, self->filter_text);
  g_print("hyprmenu_app_grid_refresh: Refresh complete\n");
}

static gboolean
app_passes_filter(HyprMenuAppEntry *entry, const char *filter_text)
{
  // No filter means everything passes
  if (!filter_text || !*filter_text) {
    return TRUE;
  }
  
  // Get the app name
  const char *app_name = hyprmenu_app_entry_get_app_name(entry);
  if (!app_name || !*app_name) {
    return FALSE;
  }
  
  // Do a simple case-insensitive substring search
  char *name_lower = g_utf8_strdown(app_name, -1);
  if (!name_lower) {
    return FALSE;
  }
  
  char *filter_lower = g_strdup(filter_text);
  if (!filter_lower) {
    g_free(name_lower);
    return FALSE;
  }
  
  char *lower_filter = g_utf8_strdown(filter_lower, -1);
  g_free(filter_lower);
  
  if (!lower_filter) {
    g_free(name_lower);
    return FALSE;
  }
  
  gboolean result = (strstr(name_lower, lower_filter) != NULL);
  
  g_free(name_lower);
  g_free(lower_filter);
  
  return result;
}

void
hyprmenu_app_grid_filter (HyprMenuAppGrid *self, const char *search_text)
{
  // Validate self pointer
  if (!self) {
    g_warning("hyprmenu_app_grid_filter: NULL self pointer");
    return;
  }

  // Free previous filter text
  if (self->filter_text) {
    g_free(self->filter_text);
    self->filter_text = NULL;
  }
  
  // Set new filter text if provided
  if (search_text && *search_text) {
    self->filter_text = g_strdup(search_text);
  }
  
  // Only continue if we have app entries to filter
  if (!self->app_entries || !self->app_entries->len) {
    return;
  }
  
  // Create a hash table to track which categories have visible items
  GHashTable *category_entries = g_hash_table_new(g_str_hash, g_str_equal);
  
  // First pass: mark all entries as visible/hidden based on filter
  for (guint i = 0; i < self->app_entries->len; i++) {
    HyprMenuAppEntry *entry = g_array_index(self->app_entries, HyprMenuAppEntry *, i);
    if (!entry) continue;
    
    gboolean visible = app_passes_filter(entry, self->filter_text);
    gtk_widget_set_visible(GTK_WIDGET(entry), visible);
    
    // If entry is visible, mark its category as having visible items
    if (visible) {
      const char *primary_category = "Other";
      const char **categories = hyprmenu_app_entry_get_categories(entry);
      
      if (categories && categories[0] && categories[0][0]) {
        primary_category = categories[0];
      }
      
      g_hash_table_insert(category_entries, (gpointer)primary_category, GINT_TO_POINTER(1));
    }
  }
  
  // Second pass: update category visibility
  GtkWidget *category_list = self->category_list;
  if (category_list) {
    GtkWidget *child = gtk_widget_get_first_child(category_list);
    while (child) {
      const char *category_name = g_object_get_data(G_OBJECT(child), "category-name");
      
      if (category_name) {
        gboolean has_visible_apps = g_hash_table_contains(category_entries, category_name);
        gtk_widget_set_visible(child, has_visible_apps);
      }
      
      child = gtk_widget_get_next_sibling(child);
    }
  }
  
  g_hash_table_destroy(category_entries);
}

void
hyprmenu_app_grid_toggle_view (HyprMenuAppGrid *self)
{
  if (!self) return;
  
  // Toggle the config setting
  config->use_grid_view = !config->use_grid_view;
  
  // Update button icon and tooltip
  gtk_button_set_icon_name(GTK_BUTTON(self->toggle_button),
    config->use_grid_view ? "view-list-symbolic" : "view-grid-symbolic");
  gtk_widget_set_tooltip_text(self->toggle_button,
    config->use_grid_view ? "Switch to List View" : "Switch to Grid View");
  
  // Always use 4 columns for grid view, regardless of config
  if (config->use_grid_view) {
    config->grid_columns = 4;
    
    // Find and resize the window
    GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(self));
    if (GTK_IS_WINDOW(root)) {
      gtk_window_set_resizable(GTK_WINDOW(root), FALSE);
      // Keep the 800x600 size
      gtk_window_set_default_size(GTK_WINDOW(root), 800, 600);
    }
  } else {
    // Reset window size for list view
    GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(self));
    if (GTK_IS_WINDOW(root)) {
      gtk_window_set_resizable(GTK_WINDOW(root), TRUE);
      // Keep the 800x600 size
      gtk_window_set_default_size(GTK_WINDOW(root), 800, 600);
    }
  }
  
  // Update the category list view mode
  hyprmenu_category_list_set_grid_view(HYPRMENU_CATEGORY_LIST(self->category_list), 
                                      config->use_grid_view);
  
  // Clear existing view and refresh
  hyprmenu_category_list_clear(HYPRMENU_CATEGORY_LIST(self->category_list));
  
  // Refresh the grid with new layout
  hyprmenu_app_grid_refresh(self);
  
  // Save config
  hyprmenu_config_save();
} 