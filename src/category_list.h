#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HYPRMENU_TYPE_CATEGORY_LIST (hyprmenu_category_list_get_type())
G_DECLARE_FINAL_TYPE (HyprMenuCategoryList, hyprmenu_category_list, HYPRMENU, CATEGORY_LIST, GtkBox)

HyprMenuCategoryList* hyprmenu_category_list_new (void);
void hyprmenu_category_list_add_category (HyprMenuCategoryList *self, const char *category_name, GtkWidget *content);
GtkWidget* hyprmenu_category_list_get_category_content (HyprMenuCategoryList *self, const char *category_name);

G_END_DECLS 