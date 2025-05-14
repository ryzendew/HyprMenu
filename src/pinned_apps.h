#pragma once

#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>

G_BEGIN_DECLS

#define HYPRMENU_TYPE_PINNED_APPS (hyprmenu_pinned_apps_get_type())
G_DECLARE_FINAL_TYPE (HyprMenuPinnedApps, hyprmenu_pinned_apps, HYPRMENU, PINNED_APPS, GtkBox)

HyprMenuPinnedApps* hyprmenu_pinned_apps_new (void);
void hyprmenu_pinned_apps_refresh (HyprMenuPinnedApps *self);
void hyprmenu_pinned_apps_add_app (HyprMenuPinnedApps *self, const char *app_id);
gboolean hyprmenu_pinned_apps_remove_app (HyprMenuPinnedApps *self, const char *app_id);
gboolean hyprmenu_pinned_apps_has_app (HyprMenuPinnedApps *self, const char *app_id);

G_END_DECLS 