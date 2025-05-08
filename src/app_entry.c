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

static void
hyprmenu_app_entry_init (HyprMenuAppEntry *self)
{
  /* Initialize layout mode - default to horizontal */
  self->is_grid_layout = FALSE;
  
  /* Create main box - horizontal for list view by default */
  self->main_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_add_css_class (self->main_box, "hyprmenu-app-entry");
  gtk_widget_set_hexpand (self->main_box, TRUE);
  
  /* Create icon */
  self->icon = gtk_image_new ();
  gtk_widget_add_css_class (self->icon, "hyprmenu-app-icon");
  gtk_widget_set_size_request (self->icon, 32, 32);
  gtk_box_append (GTK_BOX (self->main_box), self->icon);
  
  /* Create name label */
  self->name_label = gtk_label_new (NULL);
  gtk_widget_add_css_class (self->name_label, "hyprmenu-app-name");
  gtk_label_set_xalign (GTK_LABEL (self->name_label), 0);
  gtk_label_set_yalign (GTK_LABEL (self->name_label), 0.5);
  gtk_widget_set_hexpand (self->name_label, TRUE);
  gtk_box_append (GTK_BOX (self->main_box), self->name_label);
  
  /* Add main box to self */
  gtk_widget_set_parent (self->main_box, GTK_WIDGET (self));
  
  /* Add click gesture */
  GtkGesture *click_gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click_gesture), GDK_BUTTON_PRIMARY);
  g_signal_connect (click_gesture, "pressed", G_CALLBACK (on_clicked), self);
  gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (click_gesture));
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
  g_print("app_entry_new: Creating entry for '%s'\n", app_name);
  
  /* Store app info */
  self->app_info = g_object_ref (app_info);
  self->app_id = g_strdup (g_app_info_get_id (G_APP_INFO (app_info)));
  self->app_name = g_strdup (app_name);
  
  g_print("app_entry_new: App ID: '%s', App name: '%s'\n", 
         self->app_id ? self->app_id : "(null)", 
         self->app_name ? self->app_name : "(null)");
  
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
  
  /* Create button content */
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  
  /* Add app icon */
  GtkWidget *icon = NULL;
  
  const char *icon_name = g_desktop_app_info_get_string (app_info, "Icon");
  if (icon_name) {
    icon = gtk_image_new_from_icon_name (icon_name);
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 24);
  } else {
    icon = gtk_image_new_from_icon_name ("application-x-executable");
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 24);
  }
  
  gtk_box_append (GTK_BOX (box), icon);
  
  /* Add app name */
  GtkWidget *label = gtk_label_new (self->app_name);
  gtk_label_set_xalign (GTK_LABEL (label), 0);
  gtk_widget_set_hexpand (label, TRUE);
  gtk_box_append (GTK_BOX (box), label);
  
  /* Set button child */
  gtk_button_set_child (GTK_BUTTON (self), box);
  
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
  
  /* Remove existing children */
  GtkWidget *old_box = self->main_box;
  GtkOrientation orientation = is_grid ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
  
  /* Create new box with the correct orientation */
  self->main_box = gtk_box_new(orientation, 8);
  gtk_widget_add_css_class(self->main_box, "hyprmenu-app-entry");
  
  if (is_grid) {
    /* Grid layout - add CSS class */
    gtk_widget_add_css_class(self->main_box, "grid-item");
    
    /* Set size request for square shape */
    gtk_widget_set_size_request(GTK_WIDGET(self), config->grid_item_size, config->grid_item_size);
    gtk_widget_set_hexpand(GTK_WIDGET(self), FALSE);
    gtk_widget_set_vexpand(GTK_WIDGET(self), FALSE);
    
    /* Center-align the items */
    gtk_widget_set_halign(self->main_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(self->main_box, GTK_ALIGN_CENTER);
    
    /* Set icon size smaller for tile-like appearance */
    gtk_image_set_pixel_size(GTK_IMAGE(self->icon), 32);
    
    /* Update label properties for grid view */
    gtk_label_set_justify(GTK_LABEL(self->name_label), GTK_JUSTIFY_CENTER);
    gtk_label_set_lines(GTK_LABEL(self->name_label), 2);
    gtk_label_set_ellipsize(GTK_LABEL(self->name_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(self->name_label), 10);
    gtk_widget_set_halign(self->name_label, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(self->name_label, 4);
    
    /* Center icon as well */
    gtk_widget_set_halign(self->icon, GTK_ALIGN_CENTER);
  } else {
    /* List layout */
    gtk_widget_remove_css_class(self->main_box, "grid-item");
    
    /* Reset size request */
    gtk_widget_set_size_request(GTK_WIDGET(self), -1, -1);
    
    /* Reset icon size */
    gtk_image_set_pixel_size(GTK_IMAGE(self->icon), 32);
    
    /* Reset label properties */
    gtk_label_set_justify(GTK_LABEL(self->name_label), GTK_JUSTIFY_LEFT);
    gtk_label_set_lines(GTK_LABEL(self->name_label), 1);
    gtk_label_set_max_width_chars(GTK_LABEL(self->name_label), -1);
    gtk_widget_set_halign(self->name_label, GTK_ALIGN_START);
  }
  
  /* Reparent icon and label to new box */
  g_object_ref(self->icon);
  g_object_ref(self->name_label);
  
  gtk_box_remove(GTK_BOX(old_box), self->icon);
  gtk_box_remove(GTK_BOX(old_box), self->name_label);
  
  gtk_box_append(GTK_BOX(self->main_box), self->icon);
  gtk_box_append(GTK_BOX(self->main_box), self->name_label);
  
  g_object_unref(self->icon);
  g_object_unref(self->name_label);
  
  /* Replace the old box with the new one */
  gtk_widget_unparent(old_box);
  gtk_widget_set_parent(self->main_box, GTK_WIDGET(self));
} 