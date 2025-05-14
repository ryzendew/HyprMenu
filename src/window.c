#include "window.h"
#include "config.h"
#include <gtk4-layer-shell.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gdk/wayland/gdkwayland.h>
#include "app_grid.h"
#include "recent_apps.h"
#include "pinned_apps.h"

// Add this struct definition at the top of the file, after the includes
typedef struct {
  HyprMenuWindow *window;
  char *command;
  char *error_message;
} DialogData;

// Function declarations
static void on_dialog_yes_clicked(GtkButton *button, gpointer user_data);
static void on_dialog_cancel_clicked(GtkButton *button, gpointer user_data);
static void show_confirmation_dialog(HyprMenuWindow *self,
                                   const char *title,
                                   const char *message,
                                   const char *command,
                                   const char *error_message);
static void execute_system_action(HyprMenuWindow *self, 
                                const char *command,
                                const char *error_message);
static void on_logout_clicked(GtkButton *button, gpointer user_data);
static void on_shutdown_clicked(GtkButton *button, gpointer user_data);
static void on_reboot_clicked(GtkButton *button, gpointer user_data);
static void on_hibernate_clicked(GtkButton *button, gpointer user_data);
static void on_sleep_clicked(GtkButton *button, gpointer user_data);
static void on_lock_clicked(GtkButton *button, gpointer user_data);

G_DEFINE_TYPE (HyprMenuWindow, hyprmenu_window, GTK_TYPE_APPLICATION_WINDOW)

static void
on_search_changed (GtkSearchEntry *entry,
                  HyprMenuWindow *self)
{
  g_print("on_search_changed: Search text changed\n");
  
  if (!entry) {
    g_warning("on_search_changed: Search entry is NULL");
    return;
  }
  
  if (!self) {
    g_warning("on_search_changed: Window is NULL");
    return;
  }
  
  const char *text = gtk_editable_get_text (GTK_EDITABLE (entry));
  g_print("on_search_changed: Search text is '%s'\n", text ? text : "NULL");
  
  if (!self->app_grid) {
    g_warning("on_search_changed: App grid is NULL");
    return;
  }
  
  hyprmenu_app_grid_filter (HYPRMENU_APP_GRID (self->app_grid), text);
  g_print("on_search_changed: Filter applied successfully\n");
}

static void
on_search_activate (GtkSearchEntry *entry,
                   HyprMenuWindow *self)
{
  // Handle search activation if needed
  (void)entry;
  (void)self;
}

static void
on_search_stop (GtkSearchEntry *entry,
                HyprMenuWindow *self)
{
  // Handle search stop if needed
  (void)entry;
  (void)self;
}

static void
on_focus_out (GtkWindow *window,
              GParamSpec *pspec,
              gpointer user_data)
{
  (void)pspec;
  (void)user_data;
  
  // Close the window when it loses focus
  if (config->close_on_focus_out) {
    GtkApplication *app = GTK_APPLICATION(gtk_window_get_application(window));
    gtk_window_close(window);
    if (app) {
      g_application_quit(G_APPLICATION(app));
    }
  }
}

static gboolean
on_key_press (GtkEventControllerKey *controller,
              guint                  keyval,
              guint                  keycode,
              GdkModifierType        state,
              HyprMenuWindow        *self)
{
  (void)controller;
  (void)keycode;
  (void)state;
  
  // Close window on Escape key press
  if (keyval == GDK_KEY_Escape) {
    if (config->close_on_escape) {
    GtkApplication *app = GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(self)));
    gtk_window_close(GTK_WINDOW(self));
    if (app) {
      g_application_quit(G_APPLICATION(app));
    }
    return TRUE;
    }
  }
  
  // Close window on Super key press if configured
  if ((keyval == GDK_KEY_Super_L || keyval == GDK_KEY_Super_R) && 
      config->close_on_super_key) {
    GtkApplication *app = GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(self)));
    gtk_window_close(GTK_WINDOW(self));
    if (app) {
      g_application_quit(G_APPLICATION(app));
    }
    return TRUE;
  }
  
  return FALSE;
}

static gboolean
on_key_release (GtkEventControllerKey *controller,
                guint                  keyval,
                guint                  keycode,
                GdkModifierType        state,
                HyprMenuWindow        *self)
{
  (void)controller;
  (void)keycode;
  (void)state;
  
  // Close window on Super key release if configured
  if ((keyval == GDK_KEY_Super_L || keyval == GDK_KEY_Super_R) && 
      config->close_on_super_key) {
    gtk_window_close (GTK_WINDOW (self));
    return TRUE;
  }
  
  return FALSE;
}

static void
on_click_outside(GtkGestureClick *gesture,
                 gint n_press,
                 double x,
                 double y,
                 gpointer user_data)
{
  // Only proceed if configured to close on click outside
  if (!config->close_on_click_outside) {
    return;
  }
  
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  GtkWidget *widget = GTK_WIDGET(self);
  GtkWidget *clicked_widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
  
  // Get the widget under the pointer
  GtkWidget *target = gtk_widget_pick(widget, x, y, GTK_PICK_DEFAULT);
  g_print("Click outside - target widget: %s\n", target ? gtk_widget_get_name(target) : "NULL");
  
  // If the click is on a system button or its child (icon), don't close
  GtkWidget *ancestor = target;
  while (ancestor) {
    if (GTK_IS_BUTTON(ancestor) && 
        gtk_widget_has_css_class(ancestor, "system-button")) {
      g_print("Click on system button - not closing\n");
      return;
    }
    ancestor = gtk_widget_get_parent(ancestor);
  }
  
  // Get the main box bounds
  graphene_rect_t bounds;
  if (!gtk_widget_compute_bounds(self->main_box, widget, &bounds)) {
    return;  // Failed to compute bounds
  }
  
  // Convert coordinates to widget space
  graphene_point_t point = GRAPHENE_POINT_INIT(x, y);
  graphene_point_t transformed;
  if (!gtk_widget_compute_point(clicked_widget, self->main_box, &point, &transformed)) {
    return;  // Failed to compute point
  }
  
  // Check if the click is outside the main box
  if (transformed.x < bounds.origin.x || 
      transformed.y < bounds.origin.y || 
      transformed.x > bounds.origin.x + bounds.size.width || 
      transformed.y > bounds.origin.y + bounds.size.height) {
    g_print("Click outside main box - closing window\n");
    GtkApplication *app = gtk_window_get_application(GTK_WINDOW(self));
    gtk_window_close(GTK_WINDOW(self));
    if (app) {
      g_application_quit(G_APPLICATION(app));
    }
  }
}

static void
on_dialog_yes_clicked(GtkButton *button, gpointer user_data)
{
  DialogData *data = (DialogData *)user_data;
  g_print("Executing command: %s\n", data->command);
  
  // Execute the command
  execute_system_action(data->window, data->command, data->error_message);
  
  // Get dialog window and destroy it
  GtkWidget *dialog = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW);
  if (dialog) {
    gtk_window_destroy(GTK_WINDOW(dialog));
  }
  
  // Free data
  g_free(data->command);
  g_free(data->error_message);
  g_free(data);
}

static void
on_dialog_cancel_clicked(GtkButton *button, gpointer user_data)
{
  DialogData *data = (DialogData *)user_data;
  
  // Get dialog window and destroy it
  GtkWidget *dialog = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW);
  if (dialog) {
    gtk_window_destroy(GTK_WINDOW(dialog));
  }
  
  // Free data
  g_free(data->command);
  g_free(data->error_message);
  g_free(data);
}

static void
show_confirmation_dialog(HyprMenuWindow *self,
                        const char *title,
                        const char *message,
                        const char *command,
                        const char *error_message)
{
  g_print("Showing confirmation dialog: %s\n", title);
  
  // Create a new window for the dialog
  GtkWidget *dialog = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(self));
  gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
  
  // Initialize layer shell for dialog
  gtk_layer_init_for_window(GTK_WINDOW(dialog));
  gtk_layer_set_layer(GTK_WINDOW(dialog), GTK_LAYER_SHELL_LAYER_OVERLAY);
  gtk_layer_set_anchor(GTK_WINDOW(dialog), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(dialog), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(dialog), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
  gtk_layer_set_anchor(GTK_WINDOW(dialog), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
  
  // Create main container
  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(main_box, "session-bg");
  
  // Add click-outside handler using a gesture controller
  GtkWidget *overlay_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkGesture *click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);
  g_signal_connect(click, "pressed", G_CALLBACK(on_click_outside), dialog);
  gtk_widget_add_controller(overlay_box, GTK_EVENT_CONTROLLER(click));
  gtk_box_append(GTK_BOX(main_box), overlay_box);
  
  // Create content box
  GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_widget_set_vexpand(content_box, TRUE);
  gtk_widget_set_valign(content_box, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(content_box, GTK_ALIGN_CENTER);
  gtk_widget_add_css_class(content_box, "spacing-v-15");
  
  // Add title and description
  GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_margin_bottom(title_box, 10);
  
  GtkWidget *title_label = gtk_label_new(title);
  gtk_widget_add_css_class(title_label, "txt-title");
  gtk_box_append(GTK_BOX(title_box), title_label);
  
  GtkWidget *desc_label = gtk_label_new(message);
  gtk_widget_add_css_class(desc_label, "txt-small");
  gtk_label_set_justify(GTK_LABEL(desc_label), GTK_JUSTIFY_CENTER);
  gtk_box_append(GTK_BOX(title_box), desc_label);
  
  gtk_box_append(GTK_BOX(content_box), title_box);
  
  // Create button grid
  GtkWidget *button_grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(button_grid), 15);
  gtk_grid_set_column_spacing(GTK_GRID(button_grid), 15);
  gtk_widget_set_halign(button_grid, GTK_ALIGN_CENTER);
  
  // Create Yes button
  GtkWidget *yes_button = gtk_button_new();
  gtk_widget_add_css_class(yes_button, "session-button");
  gtk_widget_add_css_class(yes_button, "session-color-5"); // Use red color for power actions
  
  GtkWidget *yes_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *yes_icon = gtk_image_new_from_icon_name("system-shutdown-symbolic");
  gtk_image_set_pixel_size(GTK_IMAGE(yes_icon), 48);
  gtk_box_append(GTK_BOX(yes_box), yes_icon);
  
  GtkWidget *yes_label = gtk_label_new("Yes");
  gtk_widget_add_css_class(yes_label, "session-button-desc");
  gtk_widget_set_margin_top(yes_label, 5);
  gtk_box_append(GTK_BOX(yes_box), yes_label);
  
  gtk_button_set_child(GTK_BUTTON(yes_button), yes_box);
  
  // Store command and error message for the callback
  DialogData *data = g_new(DialogData, 1);
  data->window = self;
  data->command = g_strdup(command);
  data->error_message = g_strdup(error_message);
  
  g_signal_connect(yes_button, "clicked", G_CALLBACK(on_dialog_yes_clicked), data);
  
  // Create No button
  GtkWidget *no_button = gtk_button_new();
  gtk_widget_add_css_class(no_button, "session-button");
  gtk_widget_add_css_class(no_button, "session-color-7"); // Use neutral color for cancel
  
  GtkWidget *no_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *no_icon = gtk_image_new_from_icon_name("window-close-symbolic");
  gtk_image_set_pixel_size(GTK_IMAGE(no_icon), 48);
  gtk_box_append(GTK_BOX(no_box), no_icon);
  
  GtkWidget *no_label = gtk_label_new("No");
  gtk_widget_add_css_class(no_label, "session-button-desc");
  gtk_widget_set_margin_top(no_label, 5);
  gtk_box_append(GTK_BOX(no_box), no_label);
  
  gtk_button_set_child(GTK_BUTTON(no_button), no_box);
  g_signal_connect(no_button, "clicked", G_CALLBACK(on_dialog_cancel_clicked), data);
  
  // Add buttons to grid
  gtk_grid_attach(GTK_GRID(button_grid), yes_button, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(button_grid), no_button, 1, 0, 1, 1);
  
  gtk_box_append(GTK_BOX(content_box), button_grid);
  gtk_box_append(GTK_BOX(main_box), content_box);
  
  gtk_window_set_child(GTK_WINDOW(dialog), main_box);
  gtk_window_present(GTK_WINDOW(dialog));
}

static void
on_logout_clicked (GtkButton *button, gpointer user_data)
{
  g_print("Logout button clicked\n");
  (void)button;
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  
  show_confirmation_dialog(self,
                         "Logout",
                         "Are you sure you want to logout?",
                         "bash -c 'pkill Hyprland || pkill sway || pkill niri || loginctl terminate-user $USER'",
                         "Failed to execute logout command");
}

static void
on_shutdown_clicked (GtkButton *button, gpointer user_data)
{
  g_print("Shutdown button clicked\n");
  (void)button;
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  
  show_confirmation_dialog(self,
                         "Shutdown",
                         "Are you sure you want to shutdown the system?",
                         "bash -c 'systemctl poweroff || loginctl poweroff'",
                         "Failed to execute shutdown command");
}

static void
on_reboot_clicked (GtkButton *button, gpointer user_data)
{
  g_print("Reboot button clicked\n");
  (void)button;
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  
  show_confirmation_dialog(self,
                         "Reboot",
                         "Are you sure you want to reboot the system?",
                         "bash -c 'systemctl reboot || loginctl reboot'",
                         "Failed to execute reboot command");
}

static void
on_hibernate_clicked (GtkButton *button, gpointer user_data)
{
  g_print("Hibernate button clicked\n");
  (void)button;
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  
  show_confirmation_dialog(self,
                         "Hibernate",
                         "Are you sure you want to hibernate the system?",
                         "bash -c 'systemctl hibernate || loginctl hibernate'",
                         "Failed to execute hibernate command");
}

static void
on_sleep_clicked (GtkButton *button, gpointer user_data)
{
  g_print("Sleep button clicked\n");
  (void)button;
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  
  show_confirmation_dialog(self,
                         "Sleep",
                         "Are you sure you want to put the system to sleep?",
                         "bash -c 'systemctl suspend || loginctl suspend'",
                         "Failed to execute sleep command");
}

static void
on_lock_clicked (GtkButton *button, gpointer user_data)
{
  g_print("Lock button clicked\n");
  (void)button;
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  
  show_confirmation_dialog(self,
                         "Lock",
                         "Are you sure you want to lock the screen?",
                         "loginctl lock-session",
                         "Failed to execute lock command");
}

static GtkWidget*
create_system_button (const char *icon_name, const char *label, GCallback callback, gpointer user_data)
{
  g_print("Creating system button: %s\n", label);
  
  GtkWidget *button = gtk_button_new();
  
  // Set a unique name for debugging
  gtk_widget_set_name(button, g_strdup_printf("system-button-%s", label));
  
  // Create icon
  GtkWidget *icon = gtk_image_new_from_icon_name(icon_name);
  gtk_image_set_pixel_size(GTK_IMAGE(icon), 16);
  
  // Set button properties
  gtk_button_set_child(GTK_BUTTON(button), icon);
  gtk_widget_add_css_class(button, "system-button");
  gtk_widget_set_margin_top(button, 0);
  gtk_widget_set_margin_bottom(button, 0);
  
  // Set tooltip text (shown on hover)
  gtk_widget_set_tooltip_text(button, label);
  
  // Make button activatable and focusable
  gtk_widget_set_can_focus(button, TRUE);
  gtk_widget_set_focusable(button, TRUE);
  
  // Connect signal with debug print
  if (callback) {
    g_print("Connecting signal for button: %s\n", label);
    gulong handler_id = g_signal_connect(button, "clicked", callback, user_data);
    g_print("Signal handler ID for %s: %lu\n", label, handler_id);
  }
  
  // Add click controller explicitly
  GtkGesture *click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);
  gtk_widget_add_controller(button, GTK_EVENT_CONTROLLER(click));
  
  return button;
}

static void
hyprmenu_window_init (HyprMenuWindow *self)
{
  /* Set window properties */
  gtk_window_set_default_size(GTK_WINDOW(self), config->window_width, config->window_height);
  gtk_window_set_resizable(GTK_WINDOW(self), FALSE);
  gtk_window_set_decorated(GTK_WINDOW(self), FALSE);
  
  /* Initialize layer shell */
  GdkDisplay *display = gtk_widget_get_display(GTK_WIDGET(self));
  if (!GDK_IS_WAYLAND_DISPLAY(display)) {
    g_error("HyprMenu requires Wayland");
    return;
  }

  if (!gtk_layer_is_supported()) {
    g_error("GTK Layer Shell is required but not supported");
    return;
  }
  
  g_message("Initializing GTK Layer Shell for window");
  gtk_layer_init_for_window(GTK_WINDOW(self));
  
  g_message("Setting layer shell properties");
  gtk_layer_set_layer(GTK_WINDOW(self), GTK_LAYER_SHELL_LAYER_OVERLAY);
  gtk_layer_set_keyboard_mode(GTK_WINDOW(self), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
  gtk_layer_set_exclusive_zone(GTK_WINDOW(self), -1);
  
  /* Position window based on menu_position config */
  g_message("Setting window position to: %d", config->menu_position);
  
  int bottom_margin = (config->bottom_offset == 0) ? 2 : (config->bottom_offset + 2);
  int top_margin = (config->top_offset == 0) ? 2 : (config->top_offset + 2);
  switch (config->menu_position) {
    case POSITION_TOP_LEFT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, config->left_margin);
      break;
    
    case POSITION_TOP_CENTER:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, top_margin);
      break;
    
    case POSITION_TOP_RIGHT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, config->left_margin);
      break;
    
    case POSITION_BOTTOM_LEFT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, FALSE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, bottom_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, config->left_margin);
      break;
    
    case POSITION_BOTTOM_CENTER:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, FALSE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, bottom_margin);
      break;
    
    case POSITION_BOTTOM_RIGHT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, FALSE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, bottom_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, config->left_margin);
      break;
    
    case POSITION_CENTER:
      // Center-center: do not anchor any edge, let the window float and center manually
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
      // No margin needed, centering will be handled elsewhere if needed
      break;
    
    default:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, config->left_margin);
      break;
  }
  
  /* Add CSS classes for styling */
  gtk_widget_add_css_class (GTK_WIDGET (self), "hyprmenu-window");
  
  /* Create main container (vertical box) */
  GtkWidget *v_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_margin_start(v_box, config->window_padding);
  gtk_widget_set_margin_end(v_box, config->window_padding);
  gtk_widget_set_margin_top(v_box, config->window_padding);
  gtk_widget_set_margin_bottom(v_box, config->window_padding);
  gtk_widget_set_hexpand(v_box, TRUE);
  gtk_widget_set_vexpand(v_box, TRUE);
  gtk_widget_set_halign(v_box, GTK_ALIGN_FILL);
  gtk_widget_set_valign(v_box, GTK_ALIGN_FILL);
  gtk_window_set_child(GTK_WINDOW(self), v_box);
  
  /* Create content area */
  GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_hexpand(content_box, TRUE);
  gtk_widget_set_vexpand(content_box, TRUE);
  gtk_widget_set_halign(content_box, GTK_ALIGN_FILL);
  gtk_box_append(GTK_BOX(v_box), content_box);
  
  /* Create main box */
  self->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class(self->main_box, "hyprmenu-main-box");
  gtk_widget_set_hexpand(self->main_box, TRUE);
  gtk_widget_set_halign(self->main_box, GTK_ALIGN_FILL);
  gtk_box_append(GTK_BOX(content_box), self->main_box);
  
  // Search bar
  self->search_entry = gtk_search_entry_new();
  gtk_widget_add_css_class(self->search_entry, "hyprmenu-search");
  gtk_widget_set_hexpand(self->search_entry, TRUE);
  gtk_widget_set_halign(self->search_entry, GTK_ALIGN_FILL);
  int search_extra_pad = 8;
  gtk_widget_set_margin_start(self->search_entry, config->window_padding + search_extra_pad);
  gtk_widget_set_margin_end(self->search_entry, config->window_padding + search_extra_pad);
  gtk_widget_set_margin_top(self->search_entry, search_extra_pad);
  gtk_widget_set_margin_bottom(self->search_entry, search_extra_pad);
  gtk_box_append(GTK_BOX(self->main_box), self->search_entry);
  
  // Pinned apps section
  HyprMenuPinnedApps *pinned_apps = hyprmenu_pinned_apps_new();
  self->pinned_apps = GTK_WIDGET(pinned_apps);
  gtk_widget_add_css_class(self->pinned_apps, "hyprmenu-pinned-apps");
  gtk_widget_set_margin_start(self->pinned_apps, config->window_padding);
  gtk_widget_set_margin_end(self->pinned_apps, config->window_padding);
  gtk_widget_set_margin_top(self->pinned_apps, 4);
  gtk_widget_set_margin_bottom(self->pinned_apps, 4);
  gtk_box_append(GTK_BOX(self->main_box), self->pinned_apps);
  
  // Initialize pinned apps display
  hyprmenu_pinned_apps_refresh(pinned_apps);
  
  // Recent apps section
  HyprMenuRecentApps *recent_apps = hyprmenu_recent_apps_new();
  self->recent_apps = GTK_WIDGET(recent_apps);
  gtk_widget_add_css_class(self->recent_apps, "hyprmenu-recent-apps");
  gtk_widget_set_margin_start(self->recent_apps, config->window_padding);
  gtk_widget_set_margin_end(self->recent_apps, config->window_padding);
  gtk_widget_set_margin_top(self->recent_apps, 4);
  gtk_widget_set_margin_bottom(self->recent_apps, 4);
  gtk_box_append(GTK_BOX(self->main_box), self->recent_apps);
  
  // Initialize recent apps display
  hyprmenu_recent_apps_refresh(recent_apps);
  
  // Add a separator between recent apps and all apps
  GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_add_css_class(separator, "hyprmenu-separator");
  gtk_widget_set_margin_start(separator, config->window_padding);
  gtk_widget_set_margin_end(separator, config->window_padding);
  gtk_widget_set_margin_top(separator, 2);
  gtk_widget_set_margin_bottom(separator, 6);
  gtk_box_append(GTK_BOX(self->main_box), separator);

  // App grid
  self->app_grid = GTK_WIDGET(hyprmenu_app_grid_new());
  gtk_widget_add_css_class(self->app_grid, "hyprmenu-app-grid");
  gtk_widget_set_hexpand(self->app_grid, TRUE);
  gtk_widget_set_halign(self->app_grid, GTK_ALIGN_FILL);
  gtk_widget_set_vexpand(self->app_grid, TRUE);
  gtk_box_append(GTK_BOX(self->main_box), self->app_grid);
  
  // Set grid columns if in grid view
  if (config->grid_hexpand) {
    GtkWidget *category_list = NULL;
    GObject *grid_obj = NULL;
    // Try to get the category list from the app_grid
    g_object_get(self->app_grid, "category_list", &category_list, NULL);
    if (category_list) {
      // Set columns for the flow box
      GtkWidget *all_apps_grid = NULL;
      g_object_get(category_list, "all_apps_grid", &all_apps_grid, NULL);
      if (all_apps_grid) {
        gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(all_apps_grid), config->grid_columns);
        gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(all_apps_grid), config->grid_columns);
      }
    }
  }
  
  /* Create system buttons box at the bottom */
  self->system_buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_widget_add_css_class(self->system_buttons_box, "hyprmenu-system-buttons");
  gtk_widget_set_halign(self->system_buttons_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(self->system_buttons_box, 6);
  gtk_widget_set_margin_bottom(self->system_buttons_box, 0);
  
  /* Add system buttons */
  GtkWidget *logout_button = create_system_button("system-log-out-symbolic", "Logout", G_CALLBACK(on_logout_clicked), self);
  GtkWidget *shutdown_button = create_system_button("system-shutdown-symbolic", "Shutdown", G_CALLBACK(on_shutdown_clicked), self);
  GtkWidget *reboot_button = create_system_button("system-reboot-symbolic", "Reboot", G_CALLBACK(on_reboot_clicked), self);
  GtkWidget *hibernate_button = create_system_button("system-suspend-hibernate-symbolic", "Hibernate", G_CALLBACK(on_hibernate_clicked), self);
  GtkWidget *sleep_button = create_system_button("system-suspend-symbolic", "Sleep", G_CALLBACK(on_sleep_clicked), self);
  GtkWidget *lock_button = create_system_button("system-lock-screen-symbolic", "Lock", G_CALLBACK(on_lock_clicked), self);
  
  gtk_box_append(GTK_BOX(self->system_buttons_box), logout_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), shutdown_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), reboot_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), hibernate_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), sleep_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), lock_button);
  
  /* Add system buttons to bottom of main container */
  gtk_box_append(GTK_BOX(v_box), self->system_buttons_box);
  
  /* Connect signals */
  g_signal_connect (self->search_entry, "search-changed",
                   G_CALLBACK (on_search_changed), self);
  g_signal_connect (self->search_entry, "activate",
                   G_CALLBACK (on_search_activate), self);
  g_signal_connect (self->search_entry, "stop-search",
                   G_CALLBACK (on_search_stop), self);
  
  /* Add key controller for Escape key and Super key */
  self->key_controller = gtk_event_controller_key_new();
  g_signal_connect(self->key_controller, "key-pressed", G_CALLBACK(on_key_press), self);
  g_signal_connect(self->key_controller, "key-released", G_CALLBACK(on_key_release), self);
  gtk_widget_add_controller(GTK_WIDGET(self), self->key_controller);
  
  /* Add click gesture for click-outside */
  GtkGesture *gesture = gtk_gesture_click_new();
  self->click_gesture = GTK_GESTURE_CLICK(gesture);
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(self->click_gesture), GDK_BUTTON_PRIMARY);
  gtk_gesture_single_set_exclusive(GTK_GESTURE_SINGLE(self->click_gesture), TRUE);
  g_signal_connect(self->click_gesture, "pressed", G_CALLBACK(on_click_outside), self);
  gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(gesture));
  
  /* Connect focus-out signal */
  g_signal_connect(self, "notify::has-focus", G_CALLBACK(on_focus_out), NULL);
  
  /* Apply custom CSS from configuration */
  hyprmenu_config_apply_css();
  
  /* Set dark color scheme */
  GtkSettings *settings = gtk_settings_get_default();
  g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
  
  /* Focus search entry if configured */
  if (config->focus_search_on_open) {
    g_signal_connect(self, "map", G_CALLBACK(gtk_widget_grab_focus), self->search_entry);
  }

  if (config->search_length > 0) {
    gtk_widget_set_size_request(self->search_entry, config->search_length, -1);
  }
}

static void
hyprmenu_window_dispose(GObject *object)
{
  HyprMenuWindow *self = HYPRMENU_WINDOW(object);
  
  // Remove controllers before disposing
  if (self->key_controller) {
    gtk_widget_remove_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(self->key_controller));
    self->key_controller = NULL;
  }
  
  if (self->click_gesture) {
    gtk_widget_remove_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(self->click_gesture));
    self->click_gesture = NULL;
  }
  
  // Properly unparent child widgets
  if (self->main_box) {
    GtkWidget *child = gtk_widget_get_first_child(self->main_box);
    while (child) {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      gtk_widget_unparent(child);
      child = next;
    }
    gtk_widget_unparent(self->main_box);
    self->main_box = NULL;
  }
  
  // Clear references to child widgets
  self->app_grid = NULL;
  self->search_entry = NULL;
  self->system_buttons_box = NULL;
  
  // Chain up
  G_OBJECT_CLASS(hyprmenu_window_parent_class)->dispose(object);
}

static void
hyprmenu_window_finalize(GObject *object)
{
  // Chain up
  G_OBJECT_CLASS(hyprmenu_window_parent_class)->finalize(object);
}

static void
hyprmenu_window_class_init(HyprMenuWindowClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  
  object_class->dispose = hyprmenu_window_dispose;
  object_class->finalize = hyprmenu_window_finalize;
}

HyprMenuWindow *
hyprmenu_window_new (GtkApplication *app)
{
  return g_object_new (HYPRMENU_TYPE_WINDOW,
                       "application", app,
                       NULL);
}

void
hyprmenu_window_show (HyprMenuWindow *self)
{
  hyprmenu_app_grid_refresh (HYPRMENU_APP_GRID (self->app_grid));
  gtk_window_present (GTK_WINDOW (self));
}

static void
execute_system_action(HyprMenuWindow *self, 
                     const char *command,
                     const char *error_message)
{
  g_print("Executing system action: %s\n", command);
  
  GError *error = NULL;
  
  // First close the menu
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(self));
  if (app) {
    g_application_quit(G_APPLICATION(app));
  }

  // Execute the command asynchronously
  if (!g_spawn_command_line_async(command, &error)) {
    g_warning("%s: %s", error_message, error->message);
    g_error_free(error);
  }
} 