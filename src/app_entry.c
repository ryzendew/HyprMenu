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
hyprmenu_app_entry_finalize (GObject *object)
{
  HyprMenuAppEntry *self = HYPRMENU_APP_ENTRY (object);
  
  g_object_unref (self->app_info);
  g_free (self->app_id);
  g_free (self->app_name);
  g_strfreev (self->categories);
  
  G_OBJECT_CLASS (hyprmenu_app_entry_parent_class)->finalize (object);
}

/* Create a horizontal box for list view */
static GtkWidget*
create_list_layout(HyprMenuAppEntry *self)
{
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_add_css_class(box, "hyprmenu-app-entry");
  gtk_widget_set_hexpand(box, TRUE);
  
  /* Add icon with direct icon name setting */
  const char *icon_name = NULL;
  if (self->app_info) {
    icon_name = g_desktop_app_info_get_string(self->app_info, "Icon");
    g_print("List icon for %s: %s\n", self->app_name, icon_name ? icon_name : "NULL");
  }
  
  /* Create icon widget */
  self->icon = gtk_image_new();
  if (icon_name && icon_name[0] != '\0') {
    gtk_image_set_from_icon_name(GTK_IMAGE(self->icon), icon_name);
  } else {
    gtk_image_set_from_icon_name(GTK_IMAGE(self->icon), "application-x-executable");
  }
  gtk_image_set_pixel_size(GTK_IMAGE(self->icon), 24);
  gtk_widget_add_css_class(self->icon, "hyprmenu-app-icon");
  gtk_box_append(GTK_BOX(box), self->icon);
  
  /* Add name label with direct text setting */
  self->name_label = gtk_label_new(self->app_name);
  gtk_widget_add_css_class(self->name_label, "hyprmenu-app-name");
  gtk_label_set_xalign(GTK_LABEL(self->name_label), 0);
  gtk_label_set_yalign(GTK_LABEL(self->name_label), 0.5);
  gtk_widget_set_hexpand(self->name_label, TRUE);
  gtk_box_append(GTK_BOX(box), self->name_label);
  
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
  
  /* Create a box for the icon to center it */
  GtkWidget *icon_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(icon_box, TRUE);
  gtk_widget_set_halign(icon_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(icon_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(icon_box, 10);
  
  /* Add icon with debug logging */
  const char *icon_name = NULL;
  if (self->app_info) {
    icon_name = g_desktop_app_info_get_string(self->app_info, "Icon");
    g_print("Grid icon for %s: %s\n", self->app_name, icon_name ? icon_name : "NULL");
  }
  
  /* Create new image widget directly */
  self->icon = gtk_image_new();
  if (icon_name && icon_name[0] != '\0') {
    gtk_image_set_from_icon_name(GTK_IMAGE(self->icon), icon_name);
  } else {
    gtk_image_set_from_icon_name(GTK_IMAGE(self->icon), "application-x-executable");
  }
  
  /* Force icon size in grid view */
  gtk_image_set_pixel_size(GTK_IMAGE(self->icon), 42);
  gtk_widget_add_css_class(self->icon, "hyprmenu-app-icon");
  
  gtk_box_append(GTK_BOX(icon_box), self->icon);
  gtk_box_append(GTK_BOX(box), icon_box);
  
  /* Add name label */
  self->name_label = gtk_label_new(self->app_name);
  gtk_widget_add_css_class(self->name_label, "hyprmenu-app-name");
  gtk_label_set_justify(GTK_LABEL(self->name_label), GTK_JUSTIFY_CENTER);
  gtk_label_set_lines(GTK_LABEL(self->name_label), 1);
  gtk_label_set_ellipsize(GTK_LABEL(self->name_label), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars(GTK_LABEL(self->name_label), 10);
  gtk_widget_set_hexpand(self->name_label, TRUE);
  gtk_widget_set_halign(self->name_label, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(self->name_label, 8);
  gtk_box_append(GTK_BOX(box), self->name_label);
  
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
    gtk_widget_unparent(self->main_box);
  }
  
  /* Create appropriate layout */
  if (is_grid) {
    self->main_box = create_grid_layout(self);
  } else {
    self->main_box = create_list_layout(self);
  }
  
  /* Set parent */
  gtk_widget_set_parent(self->main_box, GTK_WIDGET(self));
} 