#pragma once

#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>

G_BEGIN_DECLS

/* Debug macros */
#define HYPRMENU_DEBUG_ENV "HYPRMENU_DEBUG"

#ifdef G_ENABLE_DEBUG
#define LIST_VIEW_DEBUG(fmt, ...) g_message("DEBUG: [ListView] " fmt, ##__VA_ARGS__)
#define LIST_VIEW_ERROR(fmt, ...) g_critical("ERROR: [ListView] " fmt, ##__VA_ARGS__)
#define LIST_VIEW_WARNING(fmt, ...) g_warning("WARNING: [ListView] " fmt, ##__VA_ARGS__)
#define LIST_VIEW_INFO(fmt, ...) g_info("INFO: [ListView] " fmt, ##__VA_ARGS__)
#else
#define LIST_VIEW_DEBUG(fmt, ...)
#define LIST_VIEW_ERROR(fmt, ...)
#define LIST_VIEW_WARNING(fmt, ...)
#define LIST_VIEW_INFO(fmt, ...)
#endif

#define HYPRMENU_TYPE_LIST_VIEW (hyprmenu_list_view_get_type())
G_DECLARE_FINAL_TYPE(HyprMenuListView, hyprmenu_list_view, HYPRMENU, LIST_VIEW, GtkWidget)

/**
 * Create a new list view widget
 */
GtkWidget* hyprmenu_list_view_new(void);

/**
 * Add an application to the list view
 * @param self The list view instance
 * @param app_info The application info to add
 * @return TRUE if the app was added successfully, FALSE otherwise
 */
gboolean hyprmenu_list_view_add_app(HyprMenuListView* self, GDesktopAppInfo* app_info);

/**
 * Clear all applications from the list view
 * @param self The list view instance
 */
void hyprmenu_list_view_clear(HyprMenuListView* self);

/**
 * Filter applications based on search text
 * @param self The list view instance
 * @param text The search text to filter by
 * @return TRUE if the filter was applied successfully, FALSE otherwise
 */
gboolean hyprmenu_list_view_filter(HyprMenuListView* self, const char* text);

/**
 * Set whether to show application descriptions
 * @param self The list view instance
 * @param show_descriptions Whether to show descriptions
 */
void hyprmenu_list_view_set_show_descriptions(HyprMenuListView* self, gboolean show_descriptions);

/**
 * Get the number of visible applications
 * @param self The list view instance
 * @return The number of visible applications
 */
guint hyprmenu_list_view_get_visible_count(HyprMenuListView* self);

/**
 * Check if the list view is valid and ready
 * @param self The list view instance
 * @return TRUE if the list view is valid, FALSE otherwise
 */
gboolean hyprmenu_list_view_is_valid(HyprMenuListView* self);

G_END_DECLS 