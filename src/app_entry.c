#include "app_entry.h"
#include "config.h"
#include "window.h"
#include <errno.h>

// Function declarations
static void on_clicked(GtkGestureClick *gesture, gint n_press, double x, double y, gpointer user_data);
static void launch_application(GDesktopAppInfo *app_info, GtkWidget *widget);

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
  int icon_size;
};

G_DEFINE_TYPE (HyprMenuAppEntry, hyprmenu_app_entry, GTK_TYPE_BUTTON)

static void
show_context_menu(HyprMenuAppEntry *self, double x, double y)
{
  g_print("CONTEXT MENU: Creating menu for app: %s\n", self->app_name ? self->app_name : "Unknown");
  
  GtkWidget *popover = gtk_popover_new();
  gtk_widget_set_halign(popover, GTK_ALIGN_START);
  gtk_popover_set_has_arrow(GTK_POPOVER(popover), FALSE);
  
  // Create a container for the popover content
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_widget_set_margin_start(box, 8);
  gtk_widget_set_margin_end(box, 8);
  gtk_widget_set_margin_top(box, 8);
  gtk_widget_set_margin_bottom(box, 8);
  
  // Create launch button
  GtkWidget *launch_button = gtk_button_new_with_label("Launch");
  gtk_widget_add_css_class(launch_button, "menu-button");
  gtk_widget_set_can_focus(launch_button, TRUE);
  gtk_button_set_has_frame(GTK_BUTTON(launch_button), TRUE);
  g_signal_connect(launch_button, "clicked", G_CALLBACK(on_clicked), self);
  
  gtk_box_append(GTK_BOX(box), launch_button);
  
  // Set the popover content
  gtk_popover_set_child(GTK_POPOVER(popover), box);
  
  // Position the popover relative to the click coordinates rather than the widget
  GdkRectangle rect = { x, y, 1, 1 };
  gtk_popover_set_pointing_to(GTK_POPOVER(popover), &rect);
  
  // Set the parent, position and display
  gtk_widget_set_parent(popover, GTK_WIDGET(self));
  gtk_popover_set_position(GTK_POPOVER(popover), GTK_POS_RIGHT);  
  gtk_popover_popup(GTK_POPOVER(popover));
  
  g_print("CONTEXT MENU: Popover displayed\n");
}

static void
on_right_click(GtkGestureClick *gesture,
              gint n_press,
              double x,
              double y,
              gpointer user_data)
{
  g_print("RIGHT CLICK DETECTED! x=%.1f, y=%.1f, n_press=%d\n", x, y, n_press);
  
  HyprMenuAppEntry *self = HYPRMENU_APP_ENTRY(user_data);
  
  if (!self) {
    g_warning("RIGHT CLICK ERROR: self is NULL");
    return;
  }
  
  g_print("RIGHT CLICK DEBUG: App entry is for: %s\n", self->app_name ? self->app_name : "Unknown");
  
  // Make sure the gesture is recognized and claimed
  GdkEventSequence *sequence = gtk_gesture_single_get_current_sequence(GTK_GESTURE_SINGLE(gesture));
  if (sequence) {
    gtk_gesture_set_sequence_state(GTK_GESTURE(gesture), sequence, GTK_EVENT_SEQUENCE_CLAIMED);
    g_print("RIGHT CLICK DEBUG: Claimed event sequence\n");
  } else {
    g_print("RIGHT CLICK DEBUG: No event sequence to claim\n");
  }
  
  show_context_menu(self, x, y);
  g_print("RIGHT CLICK DEBUG: Context menu displayed\n");
}

static void
on_clicked(GtkGestureClick *gesture,
           gint n_press,
           double x,
           double y,
           gpointer user_data)
{
  // Suppress unused parameter warnings
  (void)gesture;
  (void)n_press;
  (void)x;
  (void)y;
  
  g_print("DEBUG: on_clicked() handler called\n");
  
  HyprMenuAppEntry *self = HYPRMENU_APP_ENTRY(user_data);
  
  if (!self) {
    g_warning("LAUNCH ERROR: App entry is NULL in on_clicked");
    return;
  }
  
  if (!self->app_info) {
    g_warning("LAUNCH ERROR: App info is NULL for entry %s", 
              self->app_name ? self->app_name : "(unknown)");
    return;
  }
  
  g_print("DEBUG: Launching application: %s\n", self->app_name ? self->app_name : "(unknown)");
  
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
  
  // Set the height of the list item and ensure it's respected
  gtk_widget_set_size_request(box, -1, config->list_item_size);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  
  /* Create a fixed size icon */
  GtkWidget *icon_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(icon_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(icon_box, GTK_ALIGN_CENTER);
  
  // Set icon box size to 75% of list item height
  int icon_size = config->list_item_size * 0.75;
  gtk_widget_set_size_request(icon_box, icon_size, icon_size);
  gtk_box_append(GTK_BOX(box), icon_box);
  
  // Create the icon
  GtkWidget *icon = gtk_image_new_from_icon_name("application-x-executable");
  gtk_image_set_pixel_size(GTK_IMAGE(icon), icon_size * 0.8);  // Icon slightly smaller than box
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
  
  /* Create a label box for vertical alignment */
  GtkWidget *label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand(label_box, TRUE);
  gtk_widget_set_valign(label_box, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(box), label_box);
  
  // Create label with app name
  GtkWidget *name_label = gtk_label_new(NULL);
  char *markup = g_markup_printf_escaped("<span weight='bold' size='large'>%s</span>", 
                                        self->app_name ? self->app_name : "Unknown");
  gtk_label_set_markup(GTK_LABEL(name_label), markup);
  g_free(markup);
  
  gtk_label_set_xalign(GTK_LABEL(name_label), 0);
  gtk_widget_set_hexpand(name_label, TRUE);
  gtk_widget_set_valign(name_label, GTK_ALIGN_CENTER);
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
  gtk_widget_set_size_request(box, config->grid_item_size, config->grid_item_size);
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
  gtk_image_set_pixel_size(GTK_IMAGE(icon), config->app_icon_size); // Use configured icon size
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
  g_print("INIT: Creating app entry\n");
  
  /* Initialize layout mode - default to horizontal */
  self->is_grid_layout = FALSE;
  self->icon_size = 48;  // Default icon size
  
  /* Create main box - horizontal for list view by default */
  self->main_box = create_list_layout(self);
  
  /* Add main box to self */
  gtk_widget_set_parent(self->main_box, GTK_WIDGET(self));
  
  /* Add context menu styles */
  static gboolean menu_styles_added = FALSE;
  if (!menu_styles_added) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
      ".menu-button {"
      "  padding: 8px 12px;"
      "  margin: 2px;"
      "  border-radius: 4px;"
      "}"
      ".menu-button:hover {"
      "  background-color: alpha(@theme_selected_bg_color, 0.1);"
      "}"
      ".menu-button:active {"
      "  background-color: alpha(@theme_selected_bg_color, 0.2);"
      "}");
    
    GdkDisplay *display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(display, 
                                            GTK_STYLE_PROVIDER(provider),
                                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    menu_styles_added = TRUE;
    g_print("INIT: Added context menu styles\n");
  }
  
  g_print("INIT: Adding click gestures\n");
  
  /* Make sure button is clickable */
  gtk_widget_set_can_focus(GTK_WIDGET(self), TRUE);
  gtk_widget_set_focusable(GTK_WIDGET(self), TRUE);
  
  /* Add left-click gesture */
  GtkGesture *left_click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(left_click), GDK_BUTTON_PRIMARY);
  g_signal_connect(left_click, "released", G_CALLBACK(on_clicked), self);
  g_signal_connect(left_click, "pressed", G_CALLBACK(on_clicked), self);  // Try both signals
  gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(left_click));
  
  /* Connect the clicked signal for GtkButton base class functionality */
  g_signal_connect(self, "clicked", G_CALLBACK(on_clicked), self);
  
  /* Add right-click gesture */
  GtkGesture *right_click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(right_click), GDK_BUTTON_SECONDARY);
  
  /* Connect to both released and pressed signals to increase chances of capturing */
  g_signal_connect(right_click, "released", G_CALLBACK(on_right_click), self);
  g_signal_connect(right_click, "pressed", G_CALLBACK(on_right_click), self);
  
  /* Add controller in capture phase to catch events early */
  gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(right_click), 
                                           GTK_PHASE_CAPTURE);
  
  gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(right_click));
  
  g_print("INIT: App entry created successfully\n");
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

void
hyprmenu_app_entry_set_icon_size(HyprMenuAppEntry *self, int size)
{
  g_return_if_fail(HYPRMENU_IS_APP_ENTRY(self));
  
  self->icon_size = size;
  if (GTK_IS_IMAGE(self->icon)) {
    gtk_image_set_pixel_size(GTK_IMAGE(self->icon), size);
  }
}

static void
update_layout(HyprMenuAppEntry *self)
{
  if (config->grid_hexpand) {
    // Create grid layout
    GtkWidget *new_box = create_grid_layout(self);
    gtk_widget_set_parent(new_box, GTK_WIDGET(self));
    gtk_widget_unparent(self->main_box);
    self->main_box = new_box;
    self->is_grid_layout = TRUE;
  } else {
    // Create list layout
    GtkWidget *new_box = create_list_layout(self);
    gtk_widget_set_parent(new_box, GTK_WIDGET(self));
    gtk_widget_unparent(self->main_box);
    self->main_box = new_box;
    self->is_grid_layout = FALSE;
  }
}

// Add the launch_application function back
static void
launch_application(GDesktopAppInfo *app_info, GtkWidget *widget)
{
  g_print("DEBUG: launch_application() called\n");
  
  if (!app_info) {
    g_warning("LAUNCH ERROR: app_info is NULL");
    return;
  }
  
  if (!widget) {
    g_warning("LAUNCH ERROR: widget is NULL");
    return;
  }
  
  const char *app_name = g_app_info_get_name(G_APP_INFO(app_info));
  const char *app_cmd = g_app_info_get_commandline(G_APP_INFO(app_info));
  g_print("DEBUG: Launching app '%s' with command: %s\n", 
          app_name ? app_name : "(unknown)", 
          app_cmd ? app_cmd : "(unknown)");
  
  GError *error = NULL;
  
  if (!g_app_info_launch(G_APP_INFO(app_info), NULL, NULL, &error)) {
    g_warning("Failed to launch application: %s", error->message);
    g_error_free(error);
    return;
  }
  
  g_print("DEBUG: App launch successful\n");
  
  // Close the window if configured to do so
  if (config->close_on_app_launch) {
    g_print("DEBUG: Configured to close on app launch, closing window\n");
    // Get the parent window and close it
    GtkRoot *root = gtk_widget_get_root(widget);
    if (GTK_IS_WINDOW(root)) {
      gtk_window_close(GTK_WINDOW(root));
    }
  }
}

void
hyprmenu_app_entry_activate(HyprMenuAppEntry *self)
{
  g_return_if_fail(HYPRMENU_IS_APP_ENTRY(self));
  
  launch_application(self->app_info, GTK_WIDGET(self));
}