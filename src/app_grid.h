#pragma once

#include <gtk/gtk.h>
#include "list_view.h"

G_BEGIN_DECLS

#define HYPRMENU_TYPE_APP_GRID (hyprmenu_app_grid_get_type())
G_DECLARE_FINAL_TYPE (HyprMenuAppGrid, hyprmenu_app_grid, HYPRMENU, APP_GRID, GtkBox)

HyprMenuAppGrid* hyprmenu_app_grid_new (void);
void hyprmenu_app_grid_refresh (HyprMenuAppGrid *self);
void hyprmenu_app_grid_filter (HyprMenuAppGrid *self, const char *search_text);
void hyprmenu_app_grid_toggle_view (HyprMenuAppGrid *self);
GtkWidget* hyprmenu_app_grid_get_toggle_button(HyprMenuAppGrid *self);

G_END_DECLS 