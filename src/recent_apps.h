#pragma once

#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>

G_BEGIN_DECLS

#define HYPRMENU_TYPE_RECENT_APPS (hyprmenu_recent_apps_get_type())
G_DECLARE_FINAL_TYPE (HyprMenuRecentApps, hyprmenu_recent_apps, HYPRMENU, RECENT_APPS, GtkBox)

HyprMenuRecentApps* hyprmenu_recent_apps_new (void);
void hyprmenu_recent_apps_refresh (HyprMenuRecentApps *self);
void hyprmenu_recent_apps_add_app (HyprMenuRecentApps *self, const char *app_id);

G_END_DECLS 