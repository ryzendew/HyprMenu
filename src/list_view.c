#include "list_view.h"
#include "config.h"
#include <string.h>

struct _HyprMenuListView {
    GtkWidget parent_instance;
    
    // Main containers
    GtkWidget* scroll_window;    // Scrolled window container
    GtkWidget* main_box;         // Main vertical box
    GtkWidget* categories_box;   // Box containing category boxes
    
    // Data storage
    GHashTable* category_boxes;  // Maps category names to their GtkBox widgets
    GHashTable* app_entries;     // Maps app IDs to AppEntry structs
    GList* categories;           // Ordered list of category names
    
    // Settings
    gboolean show_descriptions;
    char* filter_text;
    
    // State tracking
    gboolean initialized;
    guint visible_apps_count;
    GError* last_error;
};

typedef struct {
    char* id;                  // Application ID
    char* name;                // Display name
    char* description;         // Description or comment
    char* primary_category;    // Main category
    GDesktopAppInfo* app_info; // Application info
    GtkWidget* row;           // The row widget containing the app entry
    GtkWidget* icon;          // Icon widget
    GtkWidget* label_box;     // Box containing name and description labels
    GtkWidget* name_label;    // Name label
    GtkWidget* desc_label;    // Description label (may be NULL)
    HyprMenuListView* view;   // Back reference to containing view
    gboolean visible;         // Visibility state
} AppEntry;

G_DEFINE_TYPE(HyprMenuListView, hyprmenu_list_view, GTK_TYPE_WIDGET)

static void
app_entry_free(AppEntry* entry)
{
    if (!entry) return;
    
    LIST_VIEW_DEBUG("Freeing app entry: %s", entry->name ? entry->name : "(null)");
    
    g_free(entry->id);
    g_free(entry->name);
    g_free(entry->description);
    g_free(entry->primary_category);
    
    if (entry->app_info) {
        g_object_unref(entry->app_info);
    }
    
    if (entry->row) {
        gtk_widget_unparent(entry->row);
    }
    
    g_free(entry);
}

static void
on_app_activated(GtkGestureClick* gesture,
                 gint n_press,
                 gdouble x,
                 gdouble y,
                 gpointer user_data)
{
    AppEntry* entry = (AppEntry*)user_data;
    
    if (!entry || !entry->app_info) {
        LIST_VIEW_WARNING("App activation failed: Invalid entry or app_info");
        return;
    }
    
    LIST_VIEW_DEBUG("Launching app: %s", entry->name);
    
    GError* error = NULL;
    if (!g_app_info_launch(G_APP_INFO(entry->app_info), NULL, NULL, &error)) {
        LIST_VIEW_ERROR("Failed to launch application %s: %s", 
                       entry->name, error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        return;
    }
    
    LIST_VIEW_DEBUG("Successfully launched app: %s", entry->name);
}

static GtkWidget*
create_category_label(const char* category)
{
    g_return_val_if_fail(category != NULL, NULL);
    
    LIST_VIEW_DEBUG("Creating category label: %s", category);
    
    GtkWidget* label = gtk_label_new(NULL);
    char* markup = g_markup_printf_escaped("<span weight='bold' size='larger'>%s</span>", category);
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
    
    gtk_widget_add_css_class(label, "hyprmenu-category-title");
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_widget_set_margin_start(label, config->category_padding);
    gtk_widget_set_margin_top(label, 12);
    gtk_widget_set_margin_bottom(label, 6);
    
    return label;
}

static GtkWidget*
get_or_create_category_box(HyprMenuListView* self, const char* category)
{
    g_return_val_if_fail(HYPRMENU_IS_LIST_VIEW(self), NULL);
    g_return_val_if_fail(category != NULL, NULL);
    
    GtkWidget* category_box = g_hash_table_lookup(self->category_boxes, category);
    if (category_box) {
        LIST_VIEW_DEBUG("Found existing category box for: %s", category);
        return category_box;
    }
    
    LIST_VIEW_DEBUG("Creating new category box for: %s", category);
    
    // Create new category box
    category_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, config->category_padding);
    gtk_widget_add_css_class(category_box, "hyprmenu-category");
    
    // Add category label
    GtkWidget* label = create_category_label(category);
    if (!label) {
        LIST_VIEW_ERROR("Failed to create category label for: %s", category);
        return NULL;
    }
    gtk_box_append(GTK_BOX(category_box), label);
    
    // Add to main container and store
    gtk_box_append(GTK_BOX(self->categories_box), category_box);
    g_hash_table_insert(self->category_boxes, g_strdup(category), category_box);
    
    // Add to ordered category list
    self->categories = g_list_insert_sorted(self->categories, g_strdup(category), 
                                          (GCompareFunc)g_ascii_strcasecmp);
    
    return category_box;
}

static AppEntry*
create_app_entry(HyprMenuListView* self, GDesktopAppInfo* app_info)
{
    g_return_val_if_fail(HYPRMENU_IS_LIST_VIEW(self), NULL);
    g_return_val_if_fail(G_IS_DESKTOP_APP_INFO(app_info), NULL);
    
    const char* app_id = g_app_info_get_id(G_APP_INFO(app_info));
    const char* app_name = g_app_info_get_display_name(G_APP_INFO(app_info));
    
    LIST_VIEW_DEBUG("Creating app entry - ID: %s, Name: %s", 
                   app_id ? app_id : "(null)", 
                   app_name ? app_name : "(null)");
    
    AppEntry* entry = g_new0(AppEntry, 1);
    entry->view = self;
    entry->app_info = g_object_ref(app_info);
    entry->id = g_strdup(app_id);
    entry->name = g_strdup(app_name);
    entry->description = g_strdup(g_app_info_get_description(G_APP_INFO(app_info)));
    entry->visible = TRUE;
    
    // Get primary category
    const char* categories = g_desktop_app_info_get_categories(app_info);
    if (categories) {
        char** cats = g_strsplit(categories, ";", -1);
        entry->primary_category = g_strdup(cats[0] ? cats[0] : "Other");
        g_strfreev(cats);
    } else {
        entry->primary_category = g_strdup("Other");
    }
    
    LIST_VIEW_DEBUG("App category: %s", entry->primary_category);
    
    // Create row widget
    entry->row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_add_css_class(entry->row, "hyprmenu-app-entry");
    gtk_widget_set_margin_start(entry->row, 4);
    gtk_widget_set_margin_end(entry->row, 4);
    gtk_widget_set_margin_top(entry->row, 2);
    gtk_widget_set_margin_bottom(entry->row, 2);
    // Set the row height from config
    gtk_widget_set_size_request(entry->row, -1, config->list_item_size);
    
    // Create icon
    entry->icon = gtk_image_new();
    GIcon* icon = g_app_info_get_icon(G_APP_INFO(app_info));
    if (icon) {
        gtk_image_set_from_gicon(GTK_IMAGE(entry->icon), icon);
    } else {
        gtk_image_set_from_icon_name(GTK_IMAGE(entry->icon), "application-x-executable");
    }
    // Set icon size proportional to list_item_size
    int icon_size = config->list_item_size * 0.75;
    gtk_image_set_pixel_size(GTK_IMAGE(entry->icon), icon_size);
    gtk_widget_set_margin_start(entry->icon, config->app_entry_padding);
    gtk_widget_add_css_class(entry->icon, "hyprmenu-app-icon");
    
    // Create label box
    entry->label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_hexpand(entry->label_box, TRUE);
    gtk_widget_set_valign(entry->label_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start(entry->label_box, config->app_entry_padding);
    gtk_widget_set_margin_end(entry->label_box, config->app_entry_padding);
    
    // Calculate scaling for font sizes
    double scale = (double)config->list_item_size / 48.0;
    int name_font_size = (int)(config->app_entry_font_size * scale);
    int desc_font_size = (int)((config->app_entry_font_size - 2) * scale);
    if (desc_font_size < 8) desc_font_size = 8;
    if (name_font_size < 8) name_font_size = 8;
    
    // Create name label with scaled font size
    char *name_markup = g_markup_printf_escaped("<span weight='bold' size='%d'>%s</span>", name_font_size * PANGO_SCALE, entry->name);
    entry->name_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(entry->name_label), name_markup);
    g_free(name_markup);
    gtk_label_set_xalign(GTK_LABEL(entry->name_label), 0);
    gtk_widget_set_valign(entry->name_label, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(entry->name_label, "app-name");
    
    // Create description label if needed, with scaled font size and centered vertically
    if (self->show_descriptions && entry->description) {
        char *desc_markup = g_markup_printf_escaped("<span size='%d'>%s</span>", desc_font_size * PANGO_SCALE, entry->description);
        entry->desc_label = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(entry->desc_label), desc_markup);
        g_free(desc_markup);
        gtk_label_set_xalign(GTK_LABEL(entry->desc_label), 0);
        gtk_widget_set_valign(entry->desc_label, GTK_ALIGN_CENTER);
        gtk_label_set_wrap(GTK_LABEL(entry->desc_label), TRUE);
        gtk_widget_add_css_class(entry->desc_label, "app-description");
    }
    
    // Add widgets to containers
    gtk_box_append(GTK_BOX(entry->label_box), entry->name_label);
    if (entry->desc_label) {
        gtk_box_append(GTK_BOX(entry->label_box), entry->desc_label);
    }
    
    gtk_box_append(GTK_BOX(entry->row), entry->icon);
    gtk_box_append(GTK_BOX(entry->row), entry->label_box);
    
    // Add click handler
    GtkGesture* gesture = gtk_gesture_click_new();
    g_signal_connect(gesture, "pressed", G_CALLBACK(on_app_activated), entry);
    gtk_widget_add_controller(entry->row, GTK_EVENT_CONTROLLER(gesture));
    
    return entry;
}

static void
update_entry_visibility(gpointer key, gpointer value, gpointer user_data)
{
    AppEntry* entry = (AppEntry*)value;
    HyprMenuListView* self = entry->view;
    
    gboolean visible = TRUE;
    
    if (self->filter_text && *self->filter_text) {
        char* name_down = g_utf8_strdown(entry->name, -1);
        char* filter_down = g_utf8_strdown(self->filter_text, -1);
        
        visible = strstr(name_down, filter_down) != NULL;
        
        if (!visible && entry->description) {
            char* desc_down = g_utf8_strdown(entry->description, -1);
            visible = strstr(desc_down, filter_down) != NULL;
            g_free(desc_down);
        }
        
        g_free(name_down);
        g_free(filter_down);
    }
    
    if (entry->visible != visible) {
        entry->visible = visible;
        gtk_widget_set_visible(entry->row, visible);
        
        if (visible) {
            self->visible_apps_count++;
        } else {
            self->visible_apps_count--;
        }
    }
}

static void
update_category_visibility(gpointer key, gpointer value, gpointer user_data)
{
    GtkWidget* category_box = (GtkWidget*)value;
    gboolean has_visible_children = FALSE;
    
    GtkWidget* child = gtk_widget_get_first_child(category_box);
    while (child) {
        if (gtk_widget_get_visible(child)) {
            if (GTK_IS_BOX(child)) { // Skip the category label
                has_visible_children = TRUE;
                break;
            }
        }
        child = gtk_widget_get_next_sibling(child);
    }
    
    gtk_widget_set_visible(category_box, has_visible_children);
}

static void
hyprmenu_list_view_dispose(GObject* object)
{
    HyprMenuListView* self = HYPRMENU_LIST_VIEW(object);
    
    LIST_VIEW_DEBUG("Disposing list view");
    
    g_clear_pointer(&self->filter_text, g_free);
    
    // First clear all app entries
    if (self->app_entries) {
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, self->app_entries);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            AppEntry* entry = (AppEntry*)value;
            if (entry->row) {
                gtk_widget_unparent(entry->row);
                entry->row = NULL;
            }
        }
        g_hash_table_destroy(self->app_entries);
        self->app_entries = NULL;
    }
    
    // Clear category boxes
    if (self->category_boxes) {
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, self->category_boxes);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            GtkWidget* box = GTK_WIDGET(value);
            if (box) {
                GtkWidget* child = gtk_widget_get_first_child(box);
                while (child) {
                    GtkWidget* next = gtk_widget_get_next_sibling(child);
                    gtk_widget_unparent(child);
                    child = next;
                }
                gtk_widget_unparent(box);
            }
        }
        g_hash_table_destroy(self->category_boxes);
        self->category_boxes = NULL;
    }
    
    // Clear categories list
    if (self->categories) {
        g_list_free_full(self->categories, g_free);
        self->categories = NULL;
    }
    
    // Finally unparent the scroll window
    if (self->scroll_window) {
        gtk_widget_unparent(self->scroll_window);
        self->scroll_window = NULL;
    }
    
    if (self->last_error) {
        g_error_free(self->last_error);
        self->last_error = NULL;
    }
    
    G_OBJECT_CLASS(hyprmenu_list_view_parent_class)->dispose(object);
}

static void
hyprmenu_list_view_class_init(HyprMenuListViewClass* class)
{
    GObjectClass* object_class = G_OBJECT_CLASS(class);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);
    
    object_class->dispose = hyprmenu_list_view_dispose;
    
    gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
    
    LIST_VIEW_DEBUG("List view class initialized");
}

static void
hyprmenu_list_view_init(HyprMenuListView* self)
{
    LIST_VIEW_DEBUG("Initializing list view");
    
    self->app_entries = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, 
                                             (GDestroyNotify)app_entry_free);
    self->category_boxes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    self->categories = NULL;
    self->show_descriptions = TRUE;
    self->filter_text = NULL;
    self->visible_apps_count = 0;
    self->initialized = FALSE;
    self->last_error = NULL;
    
    // Create scroll window
    self->scroll_window = gtk_scrolled_window_new();
    gtk_widget_set_parent(self->scroll_window, GTK_WIDGET(self));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self->scroll_window),
                                  GTK_POLICY_NEVER,
                                  GTK_POLICY_AUTOMATIC);
    
    // Create main box
    self->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(self->main_box, "hyprmenu-main-box");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(self->scroll_window), self->main_box);
    
    // Create categories box
    self->categories_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(self->categories_box, "hyprmenu-categories");
    gtk_box_append(GTK_BOX(self->main_box), self->categories_box);
    
    self->initialized = TRUE;
    LIST_VIEW_DEBUG("List view initialization complete");
}

GtkWidget*
hyprmenu_list_view_new(void)
{
    LIST_VIEW_DEBUG("Creating new list view");
    return GTK_WIDGET(g_object_new(HYPRMENU_TYPE_LIST_VIEW, NULL));
}

gboolean
hyprmenu_list_view_add_app(HyprMenuListView* self, GDesktopAppInfo* app_info)
{
    g_return_val_if_fail(HYPRMENU_IS_LIST_VIEW(self), FALSE);
    g_return_val_if_fail(G_IS_DESKTOP_APP_INFO(app_info), FALSE);
    
    if (!self->initialized) {
        LIST_VIEW_ERROR("Cannot add app: List view not properly initialized");
        return FALSE;
    }
    
    const char* app_id = g_app_info_get_id(G_APP_INFO(app_info));
    if (!app_id) {
        LIST_VIEW_WARNING("Cannot add app: Missing app ID");
        return FALSE;
    }
    
    // Check if app already exists
    if (g_hash_table_contains(self->app_entries, app_id)) {
        LIST_VIEW_DEBUG("App already exists: %s", app_id);
        return TRUE;
    }
    
    // Create new entry
    AppEntry* entry = create_app_entry(self, app_info);
    if (!entry) {
        LIST_VIEW_ERROR("Failed to create app entry for: %s", app_id);
        return FALSE;
    }
    
    // Add to category
    GtkWidget* category_box = get_or_create_category_box(self, entry->primary_category);
    if (!category_box) {
        LIST_VIEW_ERROR("Failed to get/create category box for: %s", entry->primary_category);
        app_entry_free(entry);
        return FALSE;
    }
    
    gtk_box_append(GTK_BOX(category_box), entry->row);
    
    // Store entry
    g_hash_table_insert(self->app_entries, entry->id, entry);
    self->visible_apps_count++;
    
    LIST_VIEW_DEBUG("Successfully added app: %s", app_id);
    return TRUE;
}

void
hyprmenu_list_view_clear(HyprMenuListView* self)
{
    g_return_if_fail(HYPRMENU_IS_LIST_VIEW(self));
    
    LIST_VIEW_DEBUG("Clearing list view");
    
    if (self->app_entries) {
        g_hash_table_remove_all(self->app_entries);
    }
    
    if (self->category_boxes) {
        g_hash_table_remove_all(self->category_boxes);
    }
    
    if (self->categories) {
        g_list_free_full(self->categories, g_free);
        self->categories = NULL;
    }
    
    self->visible_apps_count = 0;
    
    LIST_VIEW_DEBUG("List view cleared");
}

gboolean
hyprmenu_list_view_filter(HyprMenuListView* self, const char* text)
{
    g_return_val_if_fail(HYPRMENU_IS_LIST_VIEW(self), FALSE);
    
    LIST_VIEW_DEBUG("Applying filter: %s", text ? text : "(null)");
    
    g_free(self->filter_text);
    self->filter_text = text ? g_strdup(text) : NULL;
    
    self->visible_apps_count = 0;
    
    // Update visibility of entries
    g_hash_table_foreach(self->app_entries, update_entry_visibility, NULL);
    
    // Update visibility of categories
    g_hash_table_foreach(self->category_boxes, update_category_visibility, NULL);
    
    LIST_VIEW_DEBUG("Filter applied. Visible apps: %u", self->visible_apps_count);
    return TRUE;
}

void
hyprmenu_list_view_set_show_descriptions(HyprMenuListView* self, gboolean show_descriptions)
{
    g_return_if_fail(HYPRMENU_IS_LIST_VIEW(self));
    
    if (self->show_descriptions == show_descriptions)
        return;
    
    LIST_VIEW_DEBUG("Setting show descriptions: %d", show_descriptions);
    
    self->show_descriptions = show_descriptions;
    
    // Rebuild all entries
    GList* entries = g_hash_table_get_values(self->app_entries);
    for (GList* l = entries; l; l = l->next) {
        AppEntry* entry = l->data;
        
        if (show_descriptions && entry->description && !entry->desc_label) {
            entry->desc_label = gtk_label_new(entry->description);
            gtk_label_set_xalign(GTK_LABEL(entry->desc_label), 0);
            gtk_label_set_wrap(GTK_LABEL(entry->desc_label), TRUE);
            gtk_widget_add_css_class(entry->desc_label, "app-description");
            gtk_box_append(GTK_BOX(entry->label_box), entry->desc_label);
        } else if (!show_descriptions && entry->desc_label) {
            gtk_widget_unparent(entry->desc_label);
            entry->desc_label = NULL;
        }
    }
    g_list_free(entries);
    
    LIST_VIEW_DEBUG("Show descriptions updated");
}

guint
hyprmenu_list_view_get_visible_count(HyprMenuListView* self)
{
    g_return_val_if_fail(HYPRMENU_IS_LIST_VIEW(self), 0);
    return self->visible_apps_count;
}

gboolean
hyprmenu_list_view_is_valid(HyprMenuListView* self)
{
    if (!HYPRMENU_IS_LIST_VIEW(self)) {
        return FALSE;
    }
    
    if (!self->initialized) {
        if (self->last_error) {
            g_error_free(self->last_error);
        }
        self->last_error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED,
                                      "List view not properly initialized");
        return FALSE;
    }
    
    if (!self->scroll_window || !self->main_box || !self->categories_box) {
        if (self->last_error) {
            g_error_free(self->last_error);
        }
        self->last_error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED,
                                      "Essential widgets are missing");
        return FALSE;
    }
    
    if (!self->app_entries || !self->category_boxes) {
        if (self->last_error) {
            g_error_free(self->last_error);
        }
        self->last_error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED,
                                      "Data storage containers are missing");
        return FALSE;
    }
    
    return TRUE;
} 