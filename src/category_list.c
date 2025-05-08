#include "category_list.h"
#include "config.h"
#include "app_entry.h"

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
  
  /* Add main box to self */
  gtk_widget_set_parent (self->main_box, GTK_WIDGET (self));
  
  /* Create flow box for grid view */
  self->all_apps_grid = gtk_flow_box_new();
  gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(self->all_apps_grid), GTK_SELECTION_NONE);
  gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(self->all_apps_grid), 4);
  
  /* Force items to their native size rather than trying to make them homogeneous */
  gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(self->all_apps_grid), FALSE);
  
  /* Set spacing for grid-like appearance */
  gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(self->all_apps_grid), 8);
  gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(self->all_apps_grid), 8);
  gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(self->all_apps_grid), TRUE);
  
  gtk_widget_add_css_class(self->all_apps_grid, "hyprmenu-app-grid");
  gtk_widget_set_visible(self->all_apps_grid, FALSE);  // Hide by default (list view is default)
  gtk_widget_set_margin_start(self->all_apps_grid, 12);
  gtk_widget_set_margin_end(self->all_apps_grid, 12);
  gtk_widget_set_margin_top(self->all_apps_grid, 12);
  gtk_widget_set_margin_bottom(self->all_apps_grid, 12);
  
  /* Initialize category hash table */
  self->category_boxes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static void
hyprmenu_category_list_class_init (HyprMenuCategoryListClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = hyprmenu_category_list_finalize;
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
  
  /* Clear all category boxes */
  if (self->category_boxes) {
    GHashTableIter iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&iter, self->category_boxes);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
      GtkWidget *box = GTK_WIDGET(value);
      if (box) {
        GtkWidget *child = gtk_widget_get_first_child(box);
        while (child) {
          GtkWidget *next = gtk_widget_get_next_sibling(child);
          if (!GTK_IS_LABEL(child)) { // Skip the category label
            gtk_box_remove(GTK_BOX(box), child);
          }
          child = next;
        }
      }
    }
  }
  
  /* Clear grid view */
  GtkWidget *child = gtk_widget_get_first_child(self->all_apps_grid);
  while (child) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_flow_box_remove(GTK_FLOW_BOX(self->all_apps_grid), child);
    child = next;
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
  
  /* List view layout - ensure app widget is in list layout mode */
  hyprmenu_app_entry_set_grid_layout(HYPRMENU_APP_ENTRY(app_widget), FALSE);
  
  /* List view logic */
  GtkWidget *category_box = NULL;
  
  /* Find existing category box */
  category_box = g_hash_table_lookup(self->category_boxes, category_name);
  
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
  
  /* Add app widget to category box */
  gtk_box_append (GTK_BOX (category_box), app_widget);
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
    // Always enforce 4 columns in grid view
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(self->all_apps_grid), 4);
  }
  
  /* Show/hide the appropriate view */
  if (use_grid_view) {
    /* Grid view - collect all app widgets and move them to the grid */
    GList *app_widgets = NULL;
    
    /* First, collect all app widgets from categories */
    if (self->category_boxes) {
      GHashTableIter iter;
      gpointer key, value;
      
      g_hash_table_iter_init(&iter, self->category_boxes);
      while (g_hash_table_iter_next(&iter, &key, &value)) {
        GtkWidget *box = GTK_WIDGET(value);
        if (box) {
          GtkWidget *child = gtk_widget_get_first_child(box);
          while (child) {
            GtkWidget *next = gtk_widget_get_next_sibling(child);
            /* Skip the category label (first child) */
            if (!GTK_IS_LABEL(child)) {
              /* Set to grid layout */
              if (HYPRMENU_IS_APP_ENTRY(child)) {
                hyprmenu_app_entry_set_grid_layout(HYPRMENU_APP_ENTRY(child), TRUE);
                
                /* Remove from category and add to our list */
                gtk_box_remove(GTK_BOX(box), child);
                app_widgets = g_list_append(app_widgets, child);
              }
            }
            child = next;
          }
        }
      }
    }
    
    /* Now add all app widgets to the grid */
    for (GList *l = app_widgets; l != NULL; l = l->next) {
      gtk_flow_box_append(GTK_FLOW_BOX(self->all_apps_grid), GTK_WIDGET(l->data));
    }
    g_list_free(app_widgets);
    
    /* Set visibility */
    if (!gtk_widget_get_parent(self->all_apps_grid)) {
      gtk_widget_set_parent(self->all_apps_grid, GTK_WIDGET(self));
    }
    gtk_widget_set_visible(self->main_box, FALSE);
    gtk_widget_set_visible(self->all_apps_grid, TRUE);
  } else {
    /* List view - move app widgets from grid back to categories */
    GList *app_widgets = NULL;
    
    /* First, collect all app widgets from the grid */
    GtkWidget *child = gtk_widget_get_first_child(self->all_apps_grid);
    while (child) {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      if (HYPRMENU_IS_APP_ENTRY(child)) {
        HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(child);
        
        /* Set to list layout */
        hyprmenu_app_entry_set_grid_layout(entry, FALSE);
        
        /* Get the first category */
        const char **categories = hyprmenu_app_entry_get_categories(entry);
        const char *category = categories && categories[0] ? categories[0] : "Other";
        
        /* Store with its category */
        app_widgets = g_list_append(app_widgets, g_object_new(
          G_TYPE_OBJECT,
          "app-widget", entry,
          "category", category,
          NULL));
        
        /* Remove from grid */
        gtk_flow_box_remove(GTK_FLOW_BOX(self->all_apps_grid), child);
      }
      child = next;
    }
    
    /* Add app widgets back to appropriate categories */
    for (GList *l = app_widgets; l != NULL; l = l->next) {
      GObject *obj = G_OBJECT(l->data);
      GtkWidget *app_widget = GTK_WIDGET(g_object_get_data(obj, "app-widget"));
      const char *category = g_object_get_data(obj, "category");
      
      if (app_widget && category) {
        hyprmenu_category_list_add_category(self, category, app_widget);
      }
      g_object_unref(obj);
    }
    g_list_free(app_widgets);
    
    /* Set visibility */
    gtk_widget_set_visible(self->all_apps_grid, FALSE);
    gtk_widget_set_visible(self->main_box, TRUE);
  }
} 