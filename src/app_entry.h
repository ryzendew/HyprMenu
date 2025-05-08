#pragma once

#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>

G_BEGIN_DECLS

#define HYPRMENU_TYPE_APP_ENTRY (hyprmenu_app_entry_get_type())
G_DECLARE_FINAL_TYPE (HyprMenuAppEntry, hyprmenu_app_entry, HYPRMENU, APP_ENTRY, GtkButton)

HyprMenuAppEntry* hyprmenu_app_entry_new (GDesktopAppInfo *app_info);
const char* hyprmenu_app_entry_get_app_name (HyprMenuAppEntry *self);
const char* hyprmenu_app_entry_get_app_id (HyprMenuAppEntry *self);
const char** hyprmenu_app_entry_get_categories (HyprMenuAppEntry *self);
void hyprmenu_app_entry_set_grid_layout (HyprMenuAppEntry *self, gboolean is_grid);
GIcon* hyprmenu_app_entry_get_icon (HyprMenuAppEntry *self);

GDesktopAppInfo* hyprmenu_app_entry_get_app_info (HyprMenuAppEntry *self);
void hyprmenu_app_entry_launch (HyprMenuAppEntry *self);

int hyprmenu_app_entry_compare_by_name(HyprMenuAppEntry *a, HyprMenuAppEntry *b);

G_END_DECLS 