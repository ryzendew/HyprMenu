#include "category_list.h"

struct _HyprMenuCategoryList
{
  GtkBox parent_instance;
  
  GHashTable *category_boxes;
  GtkWidget *main_box;
};

G_DEFINE_TYPE (HyprMenuCategoryList, hyprmenu_category_list, GTK_TYPE_BOX)

static void
hyprmenu_category_list_finalize (GObject *object)
{
  HyprMenuCategoryList *self = HYPRMENU_CATEGORY_LIST (object);
  
  g_hash_table_unref (self->category_boxes);
  
  G_OBJECT_CLASS (hyprmenu_category_list_parent_class)->finalize (object);
}

static void
hyprmenu_category_list_init (HyprMenuCategoryList *self)
{
  /* Create main box */
  self->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_add_css_class (self->main_box, "hyprmenu-category-list");
  gtk_widget_set_hexpand (self->main_box, TRUE);
  gtk_widget_set_vexpand (self->main_box, TRUE);
  
  /* Add main box to self */
  gtk_widget_set_parent (self->main_box, GTK_WIDGET (self));
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
hyprmenu_category_list_add_category (HyprMenuCategoryList *self,
                                    const char *category_name,
                                    GtkWidget *app_widget)
{
  GtkWidget *category_box = NULL;
  
  /* Find existing category box */
  GtkWidget *child = gtk_widget_get_first_child (self->main_box);
  while (child) {
    const char *name = g_object_get_data (G_OBJECT (child), "category-name");
    if (name && strcmp (name, category_name) == 0) {
      category_box = child;
      break;
    }
    child = gtk_widget_get_next_sibling (child);
  }
  
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
  }
  
  /* Add app widget to category box */
  gtk_box_append (GTK_BOX (category_box), app_widget);
}

GtkWidget *
hyprmenu_category_list_get_category_content (HyprMenuCategoryList *self, 
                                            const char *category_name)
{
  return g_hash_table_lookup (self->category_boxes, category_name);
} 