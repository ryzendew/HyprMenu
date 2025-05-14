#include "app_entry.h"
#include "config.h"
#include "recent_apps.h"
#include "window.h"
#include "pinned_apps.h"
#include <errno.h>

// Function declarations
static void direct_pin_app(HyprMenuAppEntry *entry);
static gboolean on_pin_button_released(GtkWidget *button, GdkEvent *event, gpointer user_data);
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
on_pin_app_clicked(GtkButton *button, gpointer user_data)
{
  g_print("PIN BUTTON CLICKED! Starting pin process...\n");
  g_print("======================================\n");
  
  HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(user_data);
  if (!entry) {
    g_warning("PIN ERROR: Entry is NULL!");
    return;
  }
  
  const char *app_id = entry->app_id;
  g_print("PIN DEBUG: App ID is: '%s'\n", app_id ? app_id : "NULL");
  
  if (!app_id) {
    g_warning("PIN ERROR: App ID is NULL, cannot pin");
    
    // Fallback - try to get app_id directly from app_info
    if (entry->app_info) {
      app_id = g_app_info_get_id(G_APP_INFO(entry->app_info));
      if (app_id) {
        g_print("PIN DEBUG: Retrieved app_id from app_info: '%s'\n", app_id);
      } else {
        g_warning("PIN ERROR: Could not get app_id from app_info");
        return;
      }
    } else {
      g_warning("PIN ERROR: app_info is also NULL");
      return;
    }
  }
  
  // Clean app_id if needed
  char *clean_app_id = g_strdup(app_id);
  g_strstrip(clean_app_id);  // Remove leading/trailing whitespace
  g_print("PIN DEBUG: Using clean app_id: '%s'\n", clean_app_id);
  
  // Debug the button state
  g_print("PIN DEBUG: Button is %s and %s\n", 
          gtk_widget_is_sensitive(GTK_WIDGET(button)) ? "sensitive" : "not sensitive",
          gtk_widget_get_can_focus(GTK_WIDGET(button)) ? "can focus" : "cannot focus");
  
  /* Create the configuration directory */
  char *config_dir = g_build_filename(g_get_home_dir(), ".config/hyprmenu", NULL);
  g_print("PIN DEBUG: Force creating config dir: %s\n", config_dir);
  
  // Try multiple ways to ensure directory exists
  if (g_mkdir_with_parents(config_dir, 0755) == -1) {
    g_warning("PIN ERROR: Failed to create directory: %s (error: %s)", config_dir, g_strerror(errno));
    // Try system command as fallback
    system("mkdir -p ~/.config/hyprmenu");
  }
  g_free(config_dir);
  
  /* Ensure the pinned.txt file exists */
  char *pinned_file = g_build_filename(g_get_home_dir(), ".config/hyprmenu/pinned.txt", NULL);
  g_print("PIN DEBUG: Checking if pinned file exists: %s\n", pinned_file);
  
  // If file doesn't exist, create it
  gboolean file_created = FALSE;
  if (!g_file_test(pinned_file, G_FILE_TEST_EXISTS)) {
    g_print("PIN DEBUG: Creating empty pinned.txt file\n");
    
    // Try with fopen first
    FILE *file = fopen(pinned_file, "w");
    if (file) {
      fclose(file);
      file_created = TRUE;
      g_print("PIN DEBUG: Created empty pinned.txt file with fopen\n");
    } else {
      g_warning("PIN ERROR: Could not create file with fopen: %s", g_strerror(errno));
      
      // Try with g_file_set_contents
  GError *error = NULL;
      if (g_file_set_contents(pinned_file, "", 0, &error)) {
        file_created = TRUE;
        g_print("PIN DEBUG: Created empty pinned.txt file with g_file_set_contents\n");
      } else {
        g_warning("PIN ERROR: Could not create file with g_file_set_contents: %s", error->message);
        g_clear_error(&error);
        
        // Last resort: system command
        system("touch ~/.config/hyprmenu/pinned.txt");
        if (g_file_test(pinned_file, G_FILE_TEST_EXISTS)) {
          file_created = TRUE;
          g_print("PIN DEBUG: Created empty pinned.txt file with system command\n");
        } else {
          g_warning("PIN ERROR: All methods to create pinned.txt failed!");
        }
      }
    }
  } else {
    file_created = TRUE;
    g_print("PIN DEBUG: Pinned.txt file already exists\n");
  }
  g_free(pinned_file);
  
  if (!file_created) {
    g_warning("PIN ERROR: Failed to create or verify pinned.txt file, but continuing anyway");
  }
  
  /* Now pin the app using direct approach with clean app_id */
  g_print("PIN DEBUG: Calling direct_pin_app to pin the app\n");
  direct_pin_app(entry);
  
  // Manually append to file if direct_pin_app might have failed
  GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(entry));
  if (GTK_IS_WINDOW(root)) {
    HyprMenuWindow *window = HYPRMENU_WINDOW(root);
    if (window && window->pinned_apps) {
      HyprMenuPinnedApps *pinned_apps = HYPRMENU_PINNED_APPS(window->pinned_apps);
      if (!hyprmenu_pinned_apps_has_app(pinned_apps, clean_app_id)) {
        // Try to add directly to file
        g_print("PIN DEBUG: App wasn't pinned by direct_pin_app, trying file append\n");
        char *cmd = g_strdup_printf("echo \"%s\" >> ~/.config/hyprmenu/pinned.txt", clean_app_id);
        system(cmd);
        g_free(cmd);
        
        // Force refresh the display
        hyprmenu_pinned_apps_refresh(pinned_apps);
      }
    }
  }
  
  // Free resources
  g_free(clean_app_id);
  
  // Close the popover
  GtkWidget *popover = gtk_widget_get_parent(GTK_WIDGET(button));
  if (GTK_IS_POPOVER(popover)) {
    gtk_popover_popdown(GTK_POPOVER(popover));
    g_print("PIN DEBUG: Popover closed\n");
  } else {
    g_warning("PIN ERROR: Parent is not a GTK_POPOVER");
  }
  
  g_print("PIN DEBUG: Pin process completed\n");
  g_print("======================================\n");
}

static void
on_unpin_app_clicked(GtkButton *button, gpointer user_data)
{
  HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(user_data);
  const char *app_id = entry->app_id;
  
  GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(entry));
  if (GTK_IS_WINDOW(root)) {
    HyprMenuWindow *window = HYPRMENU_WINDOW(root);
    if (app_id && window && window->pinned_apps) {
      HyprMenuPinnedApps *pinned_apps = HYPRMENU_PINNED_APPS(window->pinned_apps);
      hyprmenu_pinned_apps_remove_app(pinned_apps, app_id);
    }
  }
  
  // Close the popover
  GtkWidget *popover = gtk_widget_get_parent(GTK_WIDGET(button));
  if (GTK_IS_POPOVER(popover)) {
    gtk_popover_popdown(GTK_POPOVER(popover));
  }
}

static void
show_context_menu(HyprMenuAppEntry *self, double x, double y)
{
  g_print("CONTEXT MENU: Creating menu for app: %s\n", self->app_name ? self->app_name : "Unknown");
  g_print("CONTEXT MENU: App ID is: '%s'\n", self->app_id ? self->app_id : "NULL");
  
  // Create popover with proper settings
  GtkWidget *popover = gtk_popover_new();
  gtk_widget_set_halign(popover, GTK_ALIGN_START);
  gtk_popover_set_has_arrow(GTK_POPOVER(popover), FALSE);
  
  // Create a container for the popover content
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_widget_set_margin_start(box, 8);
  gtk_widget_set_margin_end(box, 8);
  gtk_widget_set_margin_top(box, 8);
  gtk_widget_set_margin_bottom(box, 8);
  
  // Check if app is already pinned
  gboolean is_pinned = FALSE;
  GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(self));
  if (GTK_IS_WINDOW(root)) {
    HyprMenuWindow *window = HYPRMENU_WINDOW(root);
    if (self->app_id && window && window->pinned_apps) {
      HyprMenuPinnedApps *pinned_apps = HYPRMENU_PINNED_APPS(window->pinned_apps);
      
      // Debug: Output our current app ID for debugging purposes
      g_print("CONTEXT MENU: Checking if app is pinned. Current app ID: '%s'\n", 
              self->app_id ? self->app_id : "NULL");
      
      is_pinned = hyprmenu_pinned_apps_has_app(pinned_apps, self->app_id);
      g_print("CONTEXT MENU: App is %spinned\n", is_pinned ? "" : "not ");
    }
  }
  
  // Create pin/unpin button
  GtkWidget *button;
  
  if (!is_pinned) {
    g_print("CONTEXT MENU: Creating 'Pin to favorites' button\n");
    button = gtk_button_new_with_label("Pin to favorites");
    gtk_widget_add_css_class(button, "menu-button");
    
    // Make the button more responsive
    gtk_widget_set_can_focus(button, TRUE);
    gtk_button_set_has_frame(GTK_BUTTON(button), TRUE);
    
    // Connect ONLY the clicked signal since the other signals don't work in GTK4
    g_signal_connect(button, "clicked", G_CALLBACK(on_pin_app_clicked), self);
  } else {
    g_print("CONTEXT MENU: Creating 'Remove from favorites' button\n");
    button = gtk_button_new_with_label("Remove from favorites");
    gtk_widget_add_css_class(button, "menu-button");
    
    // Make the button more responsive
    gtk_widget_set_can_focus(button, TRUE);
    gtk_button_set_has_frame(GTK_BUTTON(button), TRUE);
    
    // Connect the signal
    g_signal_connect(button, "clicked", G_CALLBACK(on_unpin_app_clicked), self);
  }
  
  // Double-check the app ID for debugging
  g_print("CONTEXT MENU: Setting up button for app ID: %s\n", self->app_id ? self->app_id : "NULL");
  
  gtk_box_append(GTK_BOX(box), button);
  
  GtkWidget *launch_button = gtk_button_new_with_label("Launch");
  gtk_widget_add_css_class(launch_button, "menu-button");
  gtk_widget_set_can_focus(launch_button, TRUE);
  gtk_button_set_has_frame(GTK_BUTTON(launch_button), TRUE);
  g_signal_connect(launch_button, "clicked", G_CALLBACK(on_clicked), self);
  
  gtk_box_append(GTK_BOX(box), launch_button);
  
  // If not pinned, add a direct pin button as a fallback option
  if (!is_pinned) {
    GtkWidget *direct_button = gtk_button_new_with_label("Force Pin");
    gtk_widget_add_css_class(direct_button, "menu-button");
    gtk_widget_set_can_focus(direct_button, TRUE);
    gtk_button_set_has_frame(GTK_BUTTON(direct_button), TRUE);
    
    // Connect to a simple callback that directly pins without UI checks
    g_signal_connect_swapped(direct_button, "clicked", G_CALLBACK(direct_pin_app), self);
    
    gtk_box_append(GTK_BOX(box), direct_button);
    g_print("CONTEXT MENU: Added 'Force Pin' button as fallback\n");
  }
  
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

// Direct pin function to bypass the widget system if needed
static void
direct_pin_app(HyprMenuAppEntry *entry)
{
  g_print("DIRECT PIN: Called direct_pin_app for entry %p\n", entry);
  
  if (!entry) {
    g_warning("DIRECT PIN ERROR: Entry is NULL!");
    return;
  }

  const char *app_id = entry->app_id;
  g_print("DIRECT PIN: App ID is: '%s'\n", app_id ? app_id : "NULL");
  
  if (!app_id) {
    g_warning("DIRECT PIN ERROR: App ID is NULL");
    
    // Fallback - try to get app_id directly from app_info
    if (entry->app_info) {
      app_id = g_app_info_get_id(G_APP_INFO(entry->app_info));
      if (app_id) {
        g_print("DIRECT PIN: Retrieved app_id from app_info: '%s'\n", app_id);
      } else {
        g_warning("DIRECT PIN ERROR: Could not get app_id from app_info");
        return;
      }
    } else {
      g_warning("DIRECT PIN ERROR: app_info is also NULL");
      return;
    }
  }
  
  // Clean up app_id if needed
  char *clean_app_id = g_strdup(app_id);
  g_strstrip(clean_app_id);  // Remove leading/trailing whitespace
  g_print("DIRECT PIN: Using clean app_id: '%s'\n", clean_app_id);
  
  // FORCE CREATE PINNED FILE DIRECTORY FIRST
  char *config_dir = g_build_filename(g_get_home_dir(), ".config/hyprmenu", NULL);
  g_print("DIRECT PIN: Ensuring config directory exists: %s\n", config_dir);
  
  if (g_mkdir_with_parents(config_dir, 0755) == -1) {
    g_warning("DIRECT PIN ERROR: Failed to create config directory: %s (error: %s)", 
              config_dir, g_strerror(errno));
    // Even if this fails, keep trying
  }
  
  // FORCE CREATE PINNED FILE NOW
  char *pinned_file = g_build_filename(g_get_home_dir(), ".config/hyprmenu/pinned.txt", NULL);
  g_print("DIRECT PIN: Checking if pinned file exists: %s\n", pinned_file);
  
  // If file doesn't exist, create it first
  if (!g_file_test(pinned_file, G_FILE_TEST_EXISTS)) {
    g_print("DIRECT PIN: Creating empty pinned file\n");
    
    // Try multiple methods to create the file
    FILE *file = fopen(pinned_file, "w");
    if (file) {
      fclose(file);
      g_print("DIRECT PIN: Empty pinned file created successfully\n");
    } else {
      g_warning("DIRECT PIN ERROR: Could not create pinned file with fopen: %s", g_strerror(errno));
      
      // Try system command
      char *cmd = g_strdup_printf("touch \"%s\"", pinned_file);
      int ret = system(cmd);
      g_free(cmd);
      
      if (ret == 0) {
        g_print("DIRECT PIN: Empty pinned file created via system command\n");
      } else {
        g_warning("DIRECT PIN ERROR: Could not create file via system command (ret=%d)", ret);
        // Keep going anyway
      }
    }
  }
  
  g_free(pinned_file);
  g_free(config_dir);
  
  // Now proceed with normal pin flow
  GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(entry));
  g_print("DIRECT PIN: Root is %p\n", root);
  
  if (!GTK_IS_WINDOW(root)) {
    g_warning("DIRECT PIN ERROR: Root is not a GTK_WINDOW");
    
    // Emergency fallback - just append to file directly
    g_print("DIRECT PIN: Using emergency fallback - direct file append\n");
    char *cmd = g_strdup_printf("echo \"%s\" >> ~/.config/hyprmenu/pinned.txt", clean_app_id);
    system(cmd);
    g_free(cmd);
    g_free(clean_app_id);
    return;
  }
  
  g_print("DIRECT PIN: Root is a GTK_WINDOW\n");
  HyprMenuWindow *window = HYPRMENU_WINDOW(root);
  g_print("DIRECT PIN: Window is %p\n", window);
  
  if (!window) {
    g_warning("DIRECT PIN ERROR: Window is NULL");
    
    // Emergency fallback
    g_print("DIRECT PIN: Using emergency fallback - direct file append\n");
    char *cmd = g_strdup_printf("echo \"%s\" >> ~/.config/hyprmenu/pinned.txt", clean_app_id);
    system(cmd);
    g_free(cmd);
    g_free(clean_app_id);
    return;
  }
  
  g_print("DIRECT PIN: Checking for pinned_apps\n");
  if (!window->pinned_apps) {
    g_warning("DIRECT PIN ERROR: window->pinned_apps is NULL");
    
    // Emergency fallback
    g_print("DIRECT PIN: Using emergency fallback - direct file append\n");
    char *cmd = g_strdup_printf("echo \"%s\" >> ~/.config/hyprmenu/pinned.txt", clean_app_id);
    system(cmd);
    g_free(cmd);
    g_free(clean_app_id);
    return;
  }
  
  g_print("DIRECT PIN: pinned_apps is %p\n", window->pinned_apps);
  HyprMenuPinnedApps *pinned_apps = HYPRMENU_PINNED_APPS(window->pinned_apps);
  g_print("DIRECT PIN: Cast to HYPRMENU_PINNED_APPS: %p\n", pinned_apps);
  
  if (!pinned_apps) {
    g_warning("DIRECT PIN ERROR: Failed to cast pinned_apps");
    
    // Emergency fallback
    g_print("DIRECT PIN: Using emergency fallback - direct file append\n");
    char *cmd = g_strdup_printf("echo \"%s\" >> ~/.config/hyprmenu/pinned.txt", clean_app_id);
    system(cmd);
    g_free(cmd);
    g_free(clean_app_id);
    return;
  }
  
  // First check if app is already pinned
  if (hyprmenu_pinned_apps_has_app(pinned_apps, clean_app_id)) {
    g_print("DIRECT PIN: App is already pinned, skipping\n");
    g_free(clean_app_id);
    return;
  }
  
  // Call the add function
  g_print("DIRECT PIN: About to call hyprmenu_pinned_apps_add_app with app_id: '%s'\n", clean_app_id);
  hyprmenu_pinned_apps_add_app(pinned_apps, clean_app_id);
  g_print("DIRECT PIN: Called hyprmenu_pinned_apps_add_app successfully\n");
  g_free(clean_app_id);
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

/* Additional fallback handler for pin button to ensure clicking works */
static gboolean
on_pin_button_released(GtkWidget *button, GdkEvent *event, gpointer user_data)
{
  // Suppress unused parameter warning
  (void)event;
  
  g_print("PIN BUTTON RELEASED EVENT! This is a fallback handler\n");
  
  HyprMenuAppEntry *entry = HYPRMENU_APP_ENTRY(user_data);
  if (!entry) {
    g_warning("PIN RELEASE ERROR: Entry is NULL!");
    return FALSE;
  }
  
  g_print("PIN RELEASE DEBUG: Directly calling pin function as fallback\n");
  direct_pin_app(entry);
  
  // Close the popover
  GtkWidget *parent = gtk_widget_get_parent(button);
  while (parent && !GTK_IS_POPOVER(parent)) {
    parent = gtk_widget_get_parent(parent);
  }
  
  if (GTK_IS_POPOVER(parent)) {
    gtk_popover_popdown(GTK_POPOVER(parent));
    g_print("PIN RELEASE DEBUG: Popover closed\n");
  }
  
  return TRUE; // Event handled
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
  
  // Add to recent apps
  GtkRoot *root = gtk_widget_get_root(widget);
  if (GTK_IS_WINDOW(root)) {
    const char *app_id = g_app_info_get_id(G_APP_INFO(app_info));
    HyprMenuWindow *window = HYPRMENU_WINDOW(root);
    if (app_id && window && window->recent_apps) {
      HyprMenuRecentApps *recent_apps = HYPRMENU_RECENT_APPS(window->recent_apps);
      hyprmenu_recent_apps_add_app(recent_apps, app_id);
      g_print("DEBUG: Added to recent apps: %s\n", app_id);
    }
  }
  
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