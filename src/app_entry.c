#include "app_entry.h"
#include "config.h"
#include "recent_apps.h"

struct _HyprMenuAppEntry
{
  GtkButton parent_instance;
  
  GDesktopAppInfo *app_info;
  char *app_id;
  char *app_name;
  char **categories;
  
  GtkWidget *main_box;
  GtkWidget *icon;
  GtkWidget *name_label;
  
  gboolean is_grid_layout;  // Whether we're using grid layout (vertical)
};

G_DEFINE_TYPE (HyprMenuAppEntry, hyprmenu_app_entry, GTK_TYPE_BUTTON)

static void
launch_application (GDesktopAppInfo *app_info, GtkWidget *widget)
{
  GError *error = NULL;
  
  if (!g_app_info_launch (G_APP_INFO (app_info), NULL, NULL, &error)) {
    g_warning ("Failed to launch application: %s", error->message);
    g_error_free (error);
  }
  
  // Close the window if configured to do so
  if (config->close_on_app_launch) {
    // Get the parent window and close it
    GtkRoot *root = gtk_widget_get_root (widget);
    if (GTK_IS_WINDOW (root)) {
      gtk_window_close (GTK_WINDOW (root));
    }
  }
}

static void
on_clicked(GtkGestureClick *gesture,
           gint n_press,
           double x,
           double y,
           gpointer user_data)
{
  HyprMenuAppEntry *self = HYPRMENU_APP_ENTRY(user_data);
  
  // Launch the application
  launch_application(self->app_info, GTK_WIDGET(self));
}

static void
hyprmenu_app_entry_dispose (GObject *object)
{
  HyprMenuAppEntry *self = HYPRMENU_APP_ENTRY (object);

  // First remove all children from main_box
  if (self->main_box) {
    GtkWidget *child = gtk_widget_get_first_child(self->main_box);
    while (child) {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      gtk_widget_unparent(child);
      child = next;
    }
    // Then unparent main_box itself
    gtk_widget_unparent(self->main_box);
    self->main_box = NULL;
  }

  // Clear references to child widgets since they're owned by main_box
  self->icon = NULL;
  self->name_label = NULL;

  G_OBJECT_CLASS (hyprmenu_app_entry_parent_class)->dispose (object);
}

static void
hyprmenu_app_entry_finalize (GObject *object)
{
  HyprMenuAppEntry *self = HYPRMENU_APP_ENTRY (object);
  
  g_clear_object (&self->app_info);
  g_free (self->app_id);
  g_free (self->app_name);
  g_strfreev (self->categories);
  
  G_OBJECT_CLASS (hyprmenu_app_entry_parent_class)->finalize (object);
}

/* Create a horizontal box for list view */
static GtkWidget*
create_list_layout(HyprMenuAppEntry *self)
{
  /* Make a simple box with 10px spacing between elements */
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_add_css_class(box, "hyprmenu-list-row");
  gtk_widget_set_hexpand(box, TRUE);
  gtk_widget_set_margin_start(box, 4);
  gtk_widget_set_margin_end(box, 4);
  gtk_widget_set_margin_top(box, 2);
  gtk_widget_set_margin_bottom(box, 2);
  
  /* DIRECT ICON APPROACH */
  
  // Create a fixed size icon
  GtkWidget *icon_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(icon_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(icon_box, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(icon_box, 32, 32);
  gtk_box_append(GTK_BOX(box), icon_box);
  
  // Create the icon - use a built-in gtk icon first for guaranteed visibility
  GtkWidget *icon = gtk_image_new_from_icon_name("application-x-executable");
  gtk_image_set_pixel_size(GTK_IMAGE(icon), 24);
  gtk_widget_set_margin_start(icon, 4);
  gtk_widget_set_margin_end(icon, 4);
  gtk_box_append(GTK_BOX(icon_box), icon);
  
  // Try to replace with actual app icon if available
  if (self->app_info) {
    GIcon *app_icon = g_app_info_get_icon(G_APP_INFO(self->app_info));
    if (app_icon) {
      gtk_image_set_from_gicon(GTK_IMAGE(icon), app_icon);
    }
  }
  
  self->icon = icon;
  
  /* DIRECT TEXT APPROACH */
  
  // Create a label box for vertical alignment
  GtkWidget *label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(label_box, TRUE);
  gtk_widget_set_valign(label_box, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), label_box);
  
  // Create label with app name - using inline markup for guaranteed visibility
  GtkWidget *name_label = gtk_label_new(NULL);
  char *markup = g_markup_printf_escaped("<span weight='bold'>%s</span>", 
                                        self->app_name ? self->app_name : "Unknown");
  gtk_label_set_markup(GTK_LABEL(name_label), markup);
  g_free(markup);
  
  gtk_label_set_xalign(GTK_LABEL(name_label), 0);
  gtk_widget_set_hexpand(name_label, TRUE);
  gtk_box_append(GTK_BOX(label_box), name_label);
  
  self->name_label = name_label;
  
  return box;
}

/* Create a tile-style layout for grid view */
static GtkWidget*
create_grid_layout(HyprMenuAppEntry *self)
{
  /* Create box for tile */
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_widget_add_css_class(box, "hyprmenu-app-entry");
  gtk_widget_add_css_class(box, "grid-item");
  
  /* Set fixed size for the box */
  gtk_widget_set_size_request(box, 100, 100);
  gtk_widget_set_margin_start(box, 2);
  gtk_widget_set_margin_end(box, 2);
  gtk_widget_set_margin_top(box, 2);
  gtk_widget_set_margin_bottom(box, 2);
  
  /* Create a box for the icon to center it */
  GtkWidget *icon_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(icon_box, TRUE);
  gtk_widget_set_vexpand(icon_box, TRUE);
  gtk_widget_set_halign(icon_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(icon_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(icon_box, 8);
  gtk_box_append(GTK_BOX(box), icon_box);
  
  /* DIRECT ICON APPROACH - similar to list view */
  // Create the icon - use a built-in gtk icon first for guaranteed visibility
  GtkWidget *icon = gtk_image_new_from_icon_name("application-x-executable");
  gtk_image_set_pixel_size(GTK_IMAGE(icon), 42); // Larger icon for grid view
  gtk_box_append(GTK_BOX(icon_box), icon);
  
  // Try to replace with actual app icon if available
  if (self->app_info) {
    GIcon *app_icon = g_app_info_get_icon(G_APP_INFO(self->app_info));
    if (app_icon) {
      gtk_image_set_from_gicon(GTK_IMAGE(icon), app_icon);
    }
  }
  
  self->icon = icon;
  
  /* Create a label container for the name */
  GtkWidget *label_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(label_container, TRUE);
  gtk_widget_set_halign(label_container, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(label_container, 4);
  gtk_widget_set_margin_start(label_container, 4);
  gtk_widget_set_margin_end(label_container, 4);
  gtk_widget_set_margin_bottom(label_container, 8);
  gtk_box_append(GTK_BOX(box), label_container);
  
  /* Add name label with markup */
  GtkWidget *name_label = gtk_label_new(NULL);
  char *markup = g_markup_printf_escaped("<span size='small'>%s</span>", 
                                        self->app_name ? self->app_name : "Unknown");
  gtk_label_set_markup(GTK_LABEL(name_label), markup);
  g_free(markup);
  
  gtk_label_set_ellipsize(GTK_LABEL(name_label), PANGO_ELLIPSIZE_END);
  gtk_label_set_lines(GTK_LABEL(name_label), 2);
  gtk_label_set_max_width_chars(GTK_LABEL(name_label), 12);
  gtk_label_set_wrap(GTK_LABEL(name_label), TRUE);
  gtk_label_set_justify(GTK_LABEL(name_label), GTK_JUSTIFY_CENTER);
  gtk_widget_set_halign(name_label, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(label_container), name_label);
  
  self->name_label = name_label;
  
  return box;
}

static void
hyprmenu_app_entry_init (HyprMenuAppEntry *self)
{
  /* Initialize layout mode - default to horizontal */
  self->is_grid_layout = FALSE;
  
  /* Create main box - horizontal for list view by default */
  self->main_box = create_list_layout(self);
  
  /* Add main box to self */
  gtk_widget_set_parent(self->main_box, GTK_WIDGET(self));
  
  /* Add click gesture */
  GtkGesture *click_gesture = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_gesture), GDK_BUTTON_PRIMARY);
  g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_clicked), self);
  gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(click_gesture));
}

static void
hyprmenu_app_entry_class_init (HyprMenuAppEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->dispose = hyprmenu_app_entry_dispose;
  object_class->finalize = hyprmenu_app_entry_finalize;
}

HyprMenuAppEntry *
hyprmenu_app_entry_new (GDesktopAppInfo *app_info)
{
  if (!app_info) {
    g_warning("app_entry_new: Attempted to create entry with NULL app_info");
    return NULL;
  }

  const char *app_name = g_app_info_get_name(G_APP_INFO(app_info));
  if (!app_name || *app_name == '\0') {
    g_warning("app_entry_new: App has no name or empty name, skipping");
    return NULL;
  }

  HyprMenuAppEntry *self = g_object_new (HYPRMENU_TYPE_APP_ENTRY, NULL);
  
  /* Store app info */
  self->app_info = g_object_ref (app_info);
  self->app_id = g_strdup (g_app_info_get_id (G_APP_INFO (app_info)));
  self->app_name = g_strdup (app_name);
  
  /* Get categories */
  self->categories = NULL;
  const char *categories_str = g_desktop_app_info_get_categories(app_info);
  
  if (categories_str && g_utf8_validate(categories_str, -1, NULL)) {
    // Parse only if valid UTF-8
    self->categories = g_strsplit(categories_str, ";", -1);
    
    // Verify we got valid categories
    if (!self->categories || !self->categories[0] || !self->categories[0][0]) {
      // Invalid or empty categories, free and set to default
      if (self->categories) {
        g_strfreev(self->categories);
      }
      self->categories = g_strsplit("Other", ";", -1);
    }
  } else {
    // Default category
    self->categories = g_strsplit("Other", ";", -1);
  }
  
  // Note: Icon and label text are now set directly in the layout functions
  
  return self;
}

int
hyprmenu_app_entry_compare_by_name(HyprMenuAppEntry *a, HyprMenuAppEntry *b)
{
  if (!a || !b) return 0;
  
  const char *name_a = hyprmenu_app_entry_get_app_name(a);
  const char *name_b = hyprmenu_app_entry_get_app_name(b);
  
  if (!name_a) name_a = "";
  if (!name_b) name_b = "";
  
  return g_ascii_strcasecmp(name_a, name_b);
}

const char *
hyprmenu_app_entry_get_app_name (HyprMenuAppEntry *self)
{
  return self->app_name;
}

const char *
hyprmenu_app_entry_get_app_id (HyprMenuAppEntry *self)
{
  return self->app_id;
}

const char **
hyprmenu_app_entry_get_categories (HyprMenuAppEntry *self)
{
  if (!self) {
    g_warning("hyprmenu_app_entry_get_categories: NULL self pointer");
    return NULL;
  }
  
  if (!self->categories) {
    g_warning("hyprmenu_app_entry_get_categories: NULL categories for app %s", 
              self->app_name ? self->app_name : "(unknown)");
    return NULL;
  }
  
  return (const char **) self->categories;
}

/**
 * hyprmenu_app_entry_set_grid_layout:
 * @self: A #HyprMenuAppEntry
 * @is_grid: TRUE for grid/vertical layout, FALSE for list/horizontal layout
 *
 * Sets the layout of the app entry to be either vertical (for grid view) or
 * horizontal (for list view).
 */
void
hyprmenu_app_entry_set_grid_layout (HyprMenuAppEntry *self, gboolean is_grid)
{
  if (!self) return;
  
  /* Skip if already in the right layout */
  if (self->is_grid_layout == is_grid) {
    return;
  }
  
  /* Update flag */
  self->is_grid_layout = is_grid;
  
  /* Clear old children */
  if (self->main_box) {
    // Unparent will handle destroying the widget and its children
    gtk_widget_unparent(self->main_box);
    self->main_box = NULL;
    self->icon = NULL;
    self->name_label = NULL;
  }
  
  /* Create appropriate layout */
  if (is_grid) {
    self->main_box = create_grid_layout(self);
  } else {
    self->main_box = create_list_layout(self);
  }
  
  /* Set parent */
  if (self->main_box) {
    gtk_widget_set_parent(self->main_box, GTK_WIDGET(self));
  }
}

GDesktopAppInfo*
hyprmenu_app_entry_get_app_info (HyprMenuAppEntry *self)
{
  if (!self) {
    g_warning("hyprmenu_app_entry_get_app_info: NULL self pointer");
    return NULL;
  }
  
  return self->app_info;
}

void
hyprmenu_app_entry_launch (HyprMenuAppEntry *self)
{
  if (!self) {
    g_warning("hyprmenu_app_entry_launch: NULL self pointer");
    return;
  }
  
  launch_application(self->app_info, GTK_WIDGET(self));
}

GIcon*
hyprmenu_app_entry_get_icon (HyprMenuAppEntry *self)
{
  g_return_val_if_fail(HYPRMENU_IS_APP_ENTRY(self), NULL);
  
  if (self->app_info) {
    return g_app_info_get_icon(G_APP_INFO(self->app_info));
  }
  
  return NULL;
}