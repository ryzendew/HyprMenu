#include "category_list.h"
#include "config.h"
#include "app_entry.h"
#include <string.h>

// Forward declaration of the comparison function
static gint compare_rows_by_app_name(gconstpointer a, gconstpointer b);

static void
on_list_row_clicked(GtkGestureClick *gesture,
                    gint n_press,
                    double x,
                    double y,
                    gpointer user_data)
{
  (void)gesture;  // Silence unused parameter warning
  (void)n_press;  // Silence unused parameter warning
  (void)x;        // Silence unused parameter warning
  (void)y;        // Silence unused parameter warning
  
  // Launch the app using the stored reference
  HyprMenuAppEntry *app_entry = HYPRMENU_APP_ENTRY(user_data);
  hyprmenu_app_entry_launch(app_entry);
}

struct _HyprMenuCategoryList
{
  GtkBox parent_instance;
  
  GHashTable *category_boxes;
  GtkWidget *main_box;
  GtkWidget *all_apps_grid;  // Grid for grid view mode

  gboolean grid_view_mode;   // Whether we're in grid view mode
};

G_DEFINE_TYPE (HyprMenuCategoryList, hyprmenu_category_list, GTK_TYPE_BOX)

static void
hyprmenu_category_list_finalize (GObject *object)
{
  HyprMenuCategoryList *self = HYPRMENU_CATEGORY_LIST (object);
  
  if (self->category_boxes) {
    g_hash_table_unref (self->category_boxes);
  }
  
  G_OBJECT_CLASS (hyprmenu_category_list_parent_class)->finalize (object);
}

static gint
grid_sort_func(GtkFlowBoxChild *child1,
               GtkFlowBoxChild *child2,
               gpointer user_data)
{
  GtkWidget *widget1 = gtk_flow_box_child_get_child(child1);
  GtkWidget *widget2 = gtk_flow_box_child_get_child(child2);
  
  if (!HYPRMENU_IS_APP_ENTRY(widget1) || !HYPRMENU_IS_APP_ENTRY(widget2)) {
    return 0;
  }
  
  return hyprmenu_app_entry_compare_by_name(HYPRMENU_APP_ENTRY(widget1),
                                          HYPRMENU_APP_ENTRY(widget2));
}

static void
hyprmenu_category_list_init (HyprMenuCategoryList *self)
{
  /* Initialize properties */
  self->grid_view_mode = FALSE;
  
  /* Create main box for list view */
  self->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_add_css_class (self->main_box, "hyprmenu-category-list");
  gtk_widget_set_hexpand (self->main_box, TRUE);
  gtk_widget_set_vexpand (self->main_box, TRUE);
  gtk_widget_set_halign (self->main_box, GTK_ALIGN_FILL);
  gtk_widget_set_margin_start(self->main_box, 12);
  gtk_widget_set_margin_end(self->main_box, 12);
  
  /* Add main box to self */
  gtk_widget_set_parent (self->main_box, GTK_WIDGET (self));
  
  /* Create flow box for grid view */
  self->all_apps_grid = gtk_flow_box_new();
  gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(self->all_apps_grid), GTK_SELECTION_NONE);
  gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(self->all_apps_grid), 4);
  
  /* Set up sorting for grid view */
  gtk_flow_box_set_sort_func(GTK_FLOW_BOX(self->all_apps_grid),
                            grid_sort_func,
                            NULL, NULL);
  
  /* Force items to their native size rather than trying to make them homogeneous */
  gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(self->all_apps_grid), FALSE);
  
  /* Set spacing for grid-like appearance */
  gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(self->all_apps_grid), 8);
  gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(self->all_apps_grid), 8);
  gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(self->all_apps_grid), TRUE);
  
  gtk_widget_add_css_class(self->all_apps_grid, "hyprmenu-app-grid");
  gtk_widget_set_visible(self->all_apps_grid, FALSE);  // Hide by default (list view is default)
  gtk_widget_set_halign(self->all_apps_grid, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_start(self->all_apps_grid, 12);
  gtk_widget_set_margin_end(self->all_apps_grid, 12);
  gtk_widget_set_margin_top(self->all_apps_grid, 12);
  gtk_widget_set_margin_bottom(self->all_apps_grid, 12);
  
  /* Initialize category hash table */
  self->category_boxes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static void
hyprmenu_category_list_dispose (GObject *object)
{
  HyprMenuCategoryList *self = HYPRMENU_CATEGORY_LIST(object);
  
  // Clear all widgets
  hyprmenu_category_list_clear(self);
  
  // Clear hash table
  g_clear_pointer(&self->category_boxes, g_hash_table_unref);
  
  G_OBJECT_CLASS(hyprmenu_category_list_parent_class)->dispose(object);
}

static void
hyprmenu_category_list_class_init (HyprMenuCategoryListClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = hyprmenu_category_list_dispose;
}

HyprMenuCategoryList *
hyprmenu_category_list_new (void)
{
  return g_object_new (HYPRMENU_TYPE_CATEGORY_LIST, NULL);
}

void
hyprmenu_category_list_clear (HyprMenuCategoryList *self)
{
  if (!self) return;
  
  // First, clear all widgets from the grid view if it exists
  if (self->all_apps_grid) {
    GtkWidget *child = gtk_widget_get_first_child(self->all_apps_grid);
    while (child) {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      gtk_widget_unparent(child);
      child = next;
    }
  }
  
  // Clear all widgets from the main box
  if (self->main_box) {
    GtkWidget *category_box = gtk_widget_get_first_child(self->main_box);
    while (category_box) {
      GtkWidget *next_category = gtk_widget_get_next_sibling(category_box);
      // For each category box, first clear its children
      GtkWidget *child = gtk_widget_get_first_child(category_box);
      while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_widget_unparent(child);
        child = next;
      }
      // Then remove the category box itself
      gtk_widget_unparent(category_box);
      category_box = next_category;
    }
  }
  
  // Now clear the hash table
  if (self->category_boxes) {
    g_hash_table_remove_all(self->category_boxes);
  }
}

void
hyprmenu_category_list_add_category (HyprMenuCategoryList *self,
                                    const char *category_name,
                                    GtkWidget *app_widget)
{
  if (!self || !category_name || !app_widget) return;
  
  /* If we're in grid view mode, just add to the grid */
  if (self->grid_view_mode) {
    /* Set the app widget to grid layout mode */
    hyprmenu_app_entry_set_grid_layout(HYPRMENU_APP_ENTRY(app_widget), TRUE);
    
    /* Add to flow box */
    gtk_flow_box_append(GTK_FLOW_BOX(self->all_apps_grid), app_widget);
    return;
  }

  /* LIST VIEW IMPLEMENTATION */
  HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(app_widget);
  
  /* Find existing category box */
  GtkWidget *category_box = g_hash_table_lookup(self->category_boxes, category_name);
  
  /* Create new category box if not found */
  if (!category_box) {
    category_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class (category_box, "hyprmenu-category");
    gtk_widget_set_hexpand (category_box, TRUE);
    
    /* Add category title */
    GtkWidget *title = gtk_label_new (category_name);
    gtk_widget_add_css_class (title, "hyprmenu-category-title");
    gtk_label_set_xalign (GTK_LABEL (title), 0);
    gtk_box_append (GTK_BOX (category_box), title);
    
    /* Store category name */
    g_object_set_data_full (G_OBJECT (category_box), "category-name",
                           g_strdup (category_name), g_free);
    
    /* Add to main box */
    gtk_box_append (GTK_BOX (self->main_box), category_box);
    
    /* Store in hash table */
    g_hash_table_insert (self->category_boxes, g_strdup(category_name), category_box);
  }

  /* Create a custom row for this app */
  GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_add_css_class(row, "hyprmenu-list-row");
  gtk_widget_set_hexpand(row, TRUE);
  
  /* Get app info directly */
  const char *app_name = hyprmenu_app_entry_get_app_name(entry);
  
  /* Store the app widget reference in the row */
  g_object_set_data_full(G_OBJECT(row), "app-widget", g_object_ref(app_widget), g_object_unref);
  
  /* Add click handler */
  GtkGesture *click_gesture = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), GDK_BUTTON_PRIMARY);
  g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_list_row_clicked), app_widget);
  gtk_widget_add_controller(row, GTK_EVENT_CONTROLLER(click_gesture));
  
  /* Add the row to the category box */
  gtk_box_append(GTK_BOX(category_box), row);
  
  /* Sort the category box contents alphabetically */
  GList *children = NULL;
  GtkWidget *child = gtk_widget_get_first_child(category_box);
  while (child) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    if (GTK_IS_LABEL(child)) {
      // Keep the title at the top
      gtk_box_reorder_child_after(GTK_BOX(category_box), child, NULL);
    } else {
      children = g_list_append(children, child);
    }
    child = next;
  }
  
  // Sort the list of rows by app name
  children = g_list_sort(children, (GCompareFunc)compare_rows_by_app_name);
  
  // Reorder the rows
  for (GList *l = children; l != NULL; l = l->next) {
    GtkWidget *row = l->data;
    gtk_box_reorder_child_after(GTK_BOX(category_box), row, NULL);
  }
  
  g_list_free(children);
}

// Add this helper function for sorting
static gint
compare_rows_by_app_name(gconstpointer a, gconstpointer b)
{
  GtkWidget *row_a = GTK_WIDGET(a);
  GtkWidget *row_b = GTK_WIDGET(b);
  
  HyprMenuAppEntry *entry_a = g_object_get_data(G_OBJECT(row_a), "app-widget");
  HyprMenuAppEntry *entry_b = g_object_get_data(G_OBJECT(row_b), "app-widget");
  
  if (!entry_a || !entry_b) return 0;
  
  const char *name_a = hyprmenu_app_entry_get_app_name(entry_a);
  const char *name_b = hyprmenu_app_entry_get_app_name(entry_b);
  
  if (!name_a || !name_b) return 0;
  
  return g_ascii_strcasecmp(name_a, name_b);
}

GtkWidget *
hyprmenu_category_list_get_category_content (HyprMenuCategoryList *self, 
                                            const char *category_name)
{
  if (!self || !category_name) return NULL;
  return g_hash_table_lookup (self->category_boxes, category_name);
}

void 
hyprmenu_category_list_set_grid_view (HyprMenuCategoryList *self, gboolean use_grid_view)
{
  if (!self) return;
  
  /* Skip if already in the requested mode */
  if (self->grid_view_mode == use_grid_view) return;
  
  /* Update the mode */
  self->grid_view_mode = use_grid_view;
  
  /* Update the flow box column count (in case config changed) */
  if (use_grid_view) {
    // Use exactly 4 columns per row
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(self->all_apps_grid), 4);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(self->all_apps_grid), 4);
    gtk_widget_set_halign(self->all_apps_grid, GTK_ALIGN_CENTER);
    
    // Set width to ensure consistent alignment
    gtk_widget_set_size_request(self->all_apps_grid, -1, -1);
  }
  
  /* Show/hide the appropriate view */
  if (use_grid_view) {
    /* Grid view - collect all app entries from list rows and move them to the grid */
    GList *app_widgets = NULL;
    
    /* First, collect all app widgets from category rows */
    if (self->category_boxes) {
      g_print("Moving from list view to grid view\n");
      
      GHashTableIter iter;
      gpointer key, value;
      
      g_hash_table_iter_init(&iter, self->category_boxes);
      while (g_hash_table_iter_next(&iter, &key, &value)) {
        GtkWidget *category_box = GTK_WIDGET(value);
        if (category_box) {
          // Skip first child (category title)
          GtkWidget *child = gtk_widget_get_first_child(category_box);
          if (child) child = gtk_widget_get_next_sibling(child);
          
          while (child) {
            GtkWidget *next = gtk_widget_get_next_sibling(child);
            
            // Get the app_widget from the list row
            GtkWidget *app_widget = g_object_get_data(G_OBJECT(child), "app-widget");
            if (app_widget) {
              // Prepare for grid view
              hyprmenu_app_entry_set_grid_layout(HYPRMENU_APP_ENTRY(app_widget), TRUE);
              app_widgets = g_list_append(app_widgets, app_widget);
            }
            
            // Remove the row from category
            gtk_box_remove(GTK_BOX(category_box), child);
            child = next;
          }
        }
      }
    }
    
    /* Add all app entries to the grid */
    for (GList *l = app_widgets; l != NULL; l = l->next) {
      gtk_flow_box_append(GTK_FLOW_BOX(self->all_apps_grid), GTK_WIDGET(l->data));
    }
    g_list_free(app_widgets);
    
    /* Invalidate sort to ensure proper ordering */
    gtk_flow_box_invalidate_sort(GTK_FLOW_BOX(self->all_apps_grid));
    
    /* Set visibility */
    if (!gtk_widget_get_parent(self->all_apps_grid)) {
      gtk_widget_set_parent(self->all_apps_grid, GTK_WIDGET(self));
    }
    gtk_widget_set_visible(self->main_box, FALSE);
    gtk_widget_set_visible(self->all_apps_grid, TRUE);
  } else {
    /* List view - move app entries from grid back to categories */
    g_print("Moving from grid view to list view\n");
    
    // Ensure we have a valid hash table
    if (!self->category_boxes) {
      self->category_boxes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    }
    
    /* Collect all app entries from the grid */
    GHashTable *category_app_entries = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, 
      (GDestroyNotify)g_slist_free);
    
    GtkWidget *child = gtk_widget_get_first_child(self->all_apps_grid);
    while (child) {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      
      if (HYPRMENU_IS_APP_ENTRY(child)) {
        HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(child);
        
        /* Get the first category */
        const char **categories = hyprmenu_app_entry_get_categories(entry);
        const char *category = categories && categories[0] ? categories[0] : "Other";
        
        /* Get or create list for this category */
        GSList *category_entries = g_hash_table_lookup(category_app_entries, category);
        category_entries = g_slist_append(category_entries, entry);
        g_hash_table_insert(category_app_entries, g_strdup(category), category_entries);
        
        /* Remove from grid */
        gtk_flow_box_remove(GTK_FLOW_BOX(self->all_apps_grid), child);
      }
      
      child = next;
    }
    
    /* Create category lists and add app entries */
    GList *categories = g_hash_table_get_keys(category_app_entries);
    categories = g_list_sort(categories, (GCompareFunc)g_ascii_strcasecmp);
    
    for (GList *l = categories; l != NULL; l = l->next) {
      const char *category_name = l->data;
      GSList *entries = g_hash_table_lookup(category_app_entries, category_name);
      
      // Sort entries by name
      entries = g_slist_sort(entries, (GCompareFunc)hyprmenu_app_entry_compare_by_name);
      
      // Add each entry to its category
      for (GSList *entry_item = entries; entry_item != NULL; entry_item = entry_item->next) {
        HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(entry_item->data);
        
        // Set list layout before adding
        hyprmenu_app_entry_set_grid_layout(entry, FALSE);
        
        // Add to category
        hyprmenu_category_list_add_category(self, category_name, GTK_WIDGET(entry));
      }
    }
    
    g_list_free(categories);
    g_hash_table_destroy(category_app_entries);
    
    /* Set visibility */
    gtk_widget_set_visible(self->all_apps_grid, FALSE);
    gtk_widget_set_visible(self->main_box, TRUE);
  }
}

gboolean
hyprmenu_category_list_add_app (HyprMenuCategoryList *self,
                               GDesktopAppInfo *app_info)
{
  g_return_val_if_fail(HYPRMENU_IS_CATEGORY_LIST(self), FALSE);
  g_return_val_if_fail(G_IS_DESKTOP_APP_INFO(app_info), FALSE);
  
  HyprMenuAppEntry *entry = hyprmenu_app_entry_new(app_info);
  if (!entry) return FALSE;
  
  const char **categories = hyprmenu_app_entry_get_categories(entry);
  const char *category = categories && categories[0] ? categories[0] : "Other";
  
  hyprmenu_category_list_add_category(self, category, GTK_WIDGET(entry));
  return TRUE;
}

gboolean
hyprmenu_category_list_filter (HyprMenuCategoryList *self,
                             const char *search_text)
{
  g_return_val_if_fail(HYPRMENU_IS_CATEGORY_LIST(self), FALSE);
  
  GHashTableIter iter;
  gpointer key, value;
  
  g_hash_table_iter_init(&iter, self->category_boxes);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    GtkWidget *category_box = GTK_WIDGET(value);
    GtkWidget *content = gtk_widget_get_last_child(category_box);
    gboolean has_visible_items = FALSE;
    
    if (GTK_IS_FLOW_BOX(content) || GTK_IS_LIST_BOX(content)) {
      GtkWidget *child = gtk_widget_get_first_child(content);
      while (child) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        if (GTK_IS_FLOW_BOX_CHILD(child) || GTK_IS_LIST_BOX_ROW(child)) {
          GtkWidget *app = gtk_widget_get_first_child(child);
          if (HYPRMENU_IS_APP_ENTRY(app)) {
            gboolean visible = TRUE;
            if (search_text && *search_text) {
              const char *app_name = hyprmenu_app_entry_get_app_name(HYPRMENU_APP_ENTRY(app));
              char *name_lower = g_utf8_strdown(app_name, -1);
              char *search_lower = g_utf8_strdown(search_text, -1);
              
              visible = (strstr(name_lower, search_lower) != NULL);
              
              g_free(name_lower);
              g_free(search_lower);
            }
            
            gtk_widget_set_visible(child, visible);
            if (visible) has_visible_items = TRUE;
          }
        }
        child = next;
      }
    }
    
    gtk_widget_set_visible(category_box, has_visible_items);
  }
  
  return TRUE;
} 