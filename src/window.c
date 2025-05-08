#include "window.h"
#include "config.h"
#include <gtk4-layer-shell.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gdk/wayland/gdkwayland.h>
#include "app_grid.h"
#include "recent_apps.h"

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
    GtkApplication *app = GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(self)));
    gtk_window_close(GTK_WINDOW(self));
    if (app) {
      g_application_quit(G_APPLICATION(app));
    }
    return TRUE;
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
  (void)gesture;  // Silence unused parameter warning
  (void)n_press;  // Silence unused parameter warning
  
  // Only proceed if configured to close on click outside
  if (!config->close_on_click_outside) {
    return;
  }
  
  HyprMenuWindow *self = HYPRMENU_WINDOW(user_data);
  GtkWidget *widget = GTK_WIDGET(self);
  GtkWidget *main_box = self->main_box;
  
  // Get the widget's bounds using the new API
  graphene_rect_t bounds;
  if (!gtk_widget_compute_bounds(main_box, widget, &bounds)) {
    return;  // Failed to compute bounds
  }
  
  // Convert coordinates to widget space using the new API
  graphene_point_t point = GRAPHENE_POINT_INIT(x, y);
  graphene_point_t transformed;
  if (!gtk_widget_compute_point(main_box, widget, &point, &transformed)) {
    return;  // Failed to compute point
  }
  
  // Check if the click is outside the main box
  if (transformed.x < bounds.origin.x || 
      transformed.y < bounds.origin.y || 
      transformed.x > bounds.origin.x + bounds.size.width || 
      transformed.y > bounds.origin.y + bounds.size.height) {
    GtkApplication *app = GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(self)));
    gtk_window_close(GTK_WINDOW(self));
    if (app) {
      g_application_quit(G_APPLICATION(app));
    }
  }
}

// System action functions
static void
on_logout_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;
  
  g_spawn_command_line_async("hyprctl dispatch exit", NULL);
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(user_data));
  if (app) {
    g_application_quit(G_APPLICATION(app));
  }
}

static void
on_shutdown_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;
  
  g_spawn_command_line_async("systemctl poweroff", NULL);
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(user_data));
  if (app) {
    g_application_quit(G_APPLICATION(app));
  }
}

static void
on_reboot_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;
  
  g_spawn_command_line_async("systemctl reboot", NULL);
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(user_data));
  if (app) {
    g_application_quit(G_APPLICATION(app));
  }
}

static void
on_hibernate_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;
  
  g_spawn_command_line_async("systemctl hibernate", NULL);
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(user_data));
  if (app) {
    g_application_quit(G_APPLICATION(app));
  }
}

static void
on_sleep_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;
  
  g_spawn_command_line_async("systemctl suspend", NULL);
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(user_data));
  if (app) {
    g_application_quit(G_APPLICATION(app));
  }
}

static void
on_lock_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  (void)user_data;
  
  g_spawn_command_line_async("swaylock", NULL);
  GtkApplication *app = gtk_window_get_application(GTK_WINDOW(user_data));
  if (app) {
    g_application_quit(G_APPLICATION(app));
  }
}

static GtkWidget*
create_system_button (const char *icon_name, const char *label, GCallback callback, gpointer user_data)
{
  GtkWidget *button = gtk_button_new();
  
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
  
  // Connect signal
  if (callback) {
    g_signal_connect(button, "clicked", callback, user_data);
  }
  
  return button;
}

static void
hyprmenu_window_init (HyprMenuWindow *self)
{
  /* Set window properties */
  gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (self), TRUE);  // Allow resizing for proper grid view
  
  /* Set default size - will be adjusted by app grid based on view mode */
  gtk_window_set_default_size(GTK_WINDOW(self), 800, 600);
  
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
  gtk_layer_set_layer(GTK_WINDOW(self), GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_set_keyboard_mode(GTK_WINDOW(self), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
  gtk_layer_set_exclusive_zone(GTK_WINDOW(self), -1);
  
  /* Position window based on menu_position config */
  g_message("Setting window position to: %d", config->menu_position);
  
  switch (config->menu_position) {
    case POSITION_TOP_LEFT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, config->top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, config->left_margin);
      break;
    
    case POSITION_TOP_CENTER:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, config->top_margin);
      break;
    
    case POSITION_TOP_RIGHT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, config->top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, config->left_margin);
      break;
    
    case POSITION_BOTTOM_LEFT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, config->top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, config->left_margin);
      break;
    
    case POSITION_BOTTOM_CENTER:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, config->top_margin);
      break;
    
    case POSITION_BOTTOM_RIGHT:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, config->top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, config->left_margin);
      break;
    
    case POSITION_CENTER:
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
      break;
    
    default:
      // Default to top-left
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
      gtk_layer_set_anchor(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_TOP, config->top_margin);
      gtk_layer_set_margin(GTK_WINDOW(self), GTK_LAYER_SHELL_EDGE_LEFT, config->left_margin);
      break;
  }
  
  /* Add CSS classes for styling */
  gtk_widget_add_css_class (GTK_WIDGET (self), "hyprmenu-window");
  
  /* Create main container (vertical box) */
  GtkWidget *v_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_margin_start(v_box, 12);
  gtk_widget_set_margin_end(v_box, 12);
  gtk_widget_set_margin_top(v_box, 12);
  gtk_widget_set_margin_bottom(v_box, 12);
  gtk_window_set_child(GTK_WINDOW(self), v_box);
  
  /* Create content area */
  GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_hexpand(content_box, TRUE);
  gtk_widget_set_vexpand(content_box, TRUE);
  gtk_widget_set_halign(content_box, GTK_ALIGN_FILL);
  gtk_box_append(GTK_BOX(v_box), content_box);
  
  /* Create main box */
  self->main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_add_css_class (self->main_box, "hyprmenu-main-box");
  gtk_widget_set_hexpand (self->main_box, TRUE);
  gtk_widget_set_halign(self->main_box, GTK_ALIGN_FILL);
  gtk_box_append(GTK_BOX(content_box), self->main_box);
  
  /* Create search entry */
  self->search_entry = gtk_search_entry_new ();
  gtk_widget_add_css_class (self->search_entry, "hyprmenu-search");
  gtk_widget_set_hexpand (self->search_entry, TRUE);
  gtk_widget_set_halign(self->search_entry, GTK_ALIGN_FILL);
  gtk_widget_set_margin_start (self->search_entry, 12);
  gtk_widget_set_margin_end (self->search_entry, 12);
  gtk_widget_set_margin_top (self->search_entry, 12);
  gtk_widget_set_margin_bottom (self->search_entry, 12);
  gtk_box_append (GTK_BOX (self->main_box), self->search_entry);
  
  /* Create app grid */
  self->app_grid = GTK_WIDGET (hyprmenu_app_grid_new ());
  gtk_widget_add_css_class (self->app_grid, "hyprmenu-app-grid");
  gtk_widget_set_hexpand (self->app_grid, TRUE);
  gtk_widget_set_vexpand (self->app_grid, TRUE);
  gtk_widget_set_halign(self->app_grid, GTK_ALIGN_FILL);
  gtk_box_append (GTK_BOX (self->main_box), self->app_grid);
  
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
  // Get the grid/list toggle button from the app grid
  GtkWidget *toggle_button = hyprmenu_app_grid_get_toggle_button((HyprMenuAppGrid*)self->app_grid);
  
  gtk_box_append(GTK_BOX(self->system_buttons_box), logout_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), shutdown_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), reboot_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), hibernate_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), sleep_button);
  gtk_box_append(GTK_BOX(self->system_buttons_box), lock_button);
  // Add a separator or spacing if desired, then the toggle button
  gtk_box_append(GTK_BOX(self->system_buttons_box), toggle_button);
  
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