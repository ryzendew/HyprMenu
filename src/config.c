#include "config.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Global config instance
HyprMenuConfig *config = NULL;

// Position mapping
static const char* position_names[] = {
  "top-left",
  "top-center",
  "top-right",
  "bottom-left",
  "bottom-center",
  "bottom-right",
  "center"
};

// Position utility functions
const gchar* 
hyprmenu_position_to_string(HyprMenuPosition position)
{
  switch (position) {
    case POSITION_TOP_LEFT:
      return "top-left";
    case POSITION_TOP_CENTER:
      return "top-center";
    case POSITION_TOP_RIGHT:
      return "top-right";
    case POSITION_BOTTOM_LEFT:
      return "bottom-left";
    case POSITION_BOTTOM_CENTER:
      return "bottom-center";
    case POSITION_BOTTOM_RIGHT:
      return "bottom-right";
    case POSITION_CENTER:
      return "center";
    default:
      return "top-left";
  }
}

HyprMenuPosition 
hyprmenu_position_from_string(const gchar *position_str)
{
  if (!position_str) return POSITION_TOP_LEFT;
  
  if (g_strcmp0(position_str, "top-center") == 0)
    return POSITION_TOP_CENTER;
  else if (g_strcmp0(position_str, "top-right") == 0)
    return POSITION_TOP_RIGHT;
  else if (g_strcmp0(position_str, "bottom-left") == 0)
    return POSITION_BOTTOM_LEFT;
  else if (g_strcmp0(position_str, "bottom-center") == 0)
    return POSITION_BOTTOM_CENTER;
  else if (g_strcmp0(position_str, "bottom-right") == 0)
    return POSITION_BOTTOM_RIGHT;
  else if (g_strcmp0(position_str, "center") == 0)
    return POSITION_CENTER;
  else
    return POSITION_TOP_LEFT;
}

// Default configuration
static void
set_defaults(HyprMenuConfig *config)
{
  // Window layout
  config->window_width = 800;
  config->window_height = 600;
  config->top_margin = 48;
  config->left_margin = 8;
  config->center_window = FALSE;
  config->menu_position = POSITION_TOP_LEFT; // Default position is top-left
  config->bottom_offset = 55;
  config->top_offset = 48;
  
  // Window style
  config->background_opacity = 0.85;
  config->background_blur = 5.0;
  config->background_color = g_strdup("");
  config->corner_radius = 12;
  config->border_width = 5;
  config->border_color = g_strdup("#444444");
  config->border_corner_radius = 12;
  
  // AGS-style effects
  config->blur_enabled = TRUE;
  config->blur_radius = 0;
  config->blur_brightness = 1.0;
  config->blur_contrast = 1.0;
  config->blur_saturation = 1.0;
  config->blur_noise = 0.0;
  config->blur_grayscale = FALSE;
  
  // Transparency settings
  config->transparency_enabled = TRUE;
  config->transparency_alpha = 0.85;
  config->transparency_blur = TRUE;
  config->transparency_shadow = TRUE;
  config->transparency_shadow_color = g_strdup("rgba(0, 0, 0, 0.3)");
  config->transparency_shadow_radius = 20;
  
  // Search entry style
  config->search_background_color = g_strdup("#323232");
  config->search_background_opacity = 0.7;
  config->search_corner_radius = 8;
  config->search_text_color = g_strdup("#eeeeee");
  config->search_font_size = 14;
  config->search_font_family = g_strdup("Sans");
  config->search_padding = 8;
  
  // App entry style
  config->app_entry_background_color = g_strdup("#3c3c3c");
  config->app_entry_background_opacity = 0.7;
  config->app_entry_corner_radius = 6;
  config->app_entry_text_color = g_strdup("#eeeeee");
  config->app_entry_font_size = 12;
  config->app_entry_font_family = g_strdup("Sans");
  config->app_entry_padding = 6;
  config->app_icon_size = 32;
  
  // Category style
  config->category_background_color = g_strdup("#2d2d2d");
  config->category_background_opacity = 0.7;
  config->category_corner_radius = 10;
  config->category_text_color = g_strdup("rgba(255, 255, 255, 0.9)");
  config->category_font_size = 13;
  config->category_font_family = g_strdup("Sans Bold");
  config->category_padding = 6;
  config->category_show_separators = TRUE;
  
  // Behavior
  config->close_on_click_outside = TRUE;
  config->close_on_super_key = TRUE;
  config->close_on_app_launch = TRUE;
  config->focus_search_on_open = TRUE;
  config->max_recent_apps = 5;
  
  // View settings
  config->use_grid_view = FALSE;  // Default to list view
  config->grid_columns = 4;       // Exactly 4 columns in grid view
  config->grid_item_size = 80;    // Smaller square size for a tile-like appearance
  
  // File paths
  config->config_dir = g_build_filename(g_get_home_dir(), ".config", "hyprmenu", NULL);
  config->config_file = g_build_filename(config->config_dir, "hyprmenu.conf", NULL);
  config->css_file = g_build_filename(config->config_dir, "hyprmenu.css", NULL);
  
  // --- New color roles ---
  config->text_color = g_strdup("#eeeeee");
  config->button_color = g_strdup("#444444");
  config->button_text_color = g_strdup("#eeeeee");
  config->highlight_color = g_strdup("#ffb86c");
  config->search_color = g_strdup("#323232");
  config->system_button_color = g_strdup("#444444");
  config->system_button_icon_color = g_strdup("#eeeeee");
  config->use_pywal_colors = TRUE;
}

gboolean
hyprmenu_config_init()
{
  // Allocate config
  config = g_new0(HyprMenuConfig, 1);
  if (!config) {
    return FALSE;
  }
  
  // Set defaults
  set_defaults(config);
  
  // Create config directory if it doesn't exist
  g_mkdir_with_parents(config->config_dir, 0755);
  
  // Load configuration if it exists
  if (g_file_test(config->config_file, G_FILE_TEST_EXISTS)) {
    hyprmenu_config_load();
  } else {
    // Save default configuration
    hyprmenu_config_save();
  }
  
  return TRUE;
}

void
hyprmenu_config_free()
{
  if (!config) {
    return;
  }
  
  // Free all string fields
  g_free(config->background_color);
  g_free(config->border_color);
  g_free(config->search_background_color);
  g_free(config->search_text_color);
  g_free(config->search_font_family);
  g_free(config->app_entry_background_color);
  g_free(config->app_entry_text_color);
  g_free(config->app_entry_font_family);
  g_free(config->category_background_color);
  g_free(config->category_text_color);
  g_free(config->category_font_family);
  g_free(config->config_dir);
  g_free(config->config_file);
  g_free(config->css_file);
  
  // Free new color fields
  g_free(config->text_color);
  g_free(config->button_color);
  g_free(config->button_text_color);
  g_free(config->highlight_color);
  g_free(config->search_color);
  g_free(config->system_button_color);
  g_free(config->system_button_icon_color);
  
  // Free config struct
  g_free(config);
  config = NULL;
}

gboolean
hyprmenu_config_load()
{
  g_autoptr(GKeyFile) keyfile = g_key_file_new();
  gboolean missing_option = FALSE;
  
  // Load keyfile
  if (!g_key_file_load_from_file(keyfile, config->config_file, G_KEY_FILE_NONE, NULL)) {
    g_warning("Failed to load config file: %s", config->config_file);
    return FALSE;
  }
  
  // Window layout
  if (g_key_file_has_group(keyfile, "Layout")) {
    if (!g_key_file_has_key(keyfile, "Layout", "window_width", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Layout", "window_height", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Layout", "top_margin", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Layout", "left_margin", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Layout", "center_window", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Layout", "menu_position", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Layout", "bottom_offset", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Layout", "top_offset", NULL)) missing_option = TRUE;
    config->window_width = g_key_file_get_integer(keyfile, "Layout", "window_width", NULL);
    config->window_height = g_key_file_get_integer(keyfile, "Layout", "window_height", NULL);
    config->top_margin = g_key_file_get_integer(keyfile, "Layout", "top_margin", NULL);
    config->left_margin = g_key_file_get_integer(keyfile, "Layout", "left_margin", NULL);
    config->center_window = g_key_file_get_boolean(keyfile, "Layout", "center_window", NULL);
    
    // Load position setting
    char *position_str = g_key_file_get_string(keyfile, "Layout", "menu_position", NULL);
    if (position_str) {
      config->menu_position = hyprmenu_position_from_string(position_str);
      g_free(position_str);
    }
    
    // Load offsets
    config->bottom_offset = g_key_file_get_integer(keyfile, "Layout", "bottom_offset", NULL);
    config->top_offset = g_key_file_get_integer(keyfile, "Layout", "top_offset", NULL);
  }
  
  // Window style
  if (g_key_file_has_group(keyfile, "Style")) {
    if (!g_key_file_has_key(keyfile, "Style", "background_opacity", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "background_blur", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "background_color", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "corner_radius", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "border_width", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "border_color", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "border_corner_radius", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "blur_enabled", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "blur_radius", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "blur_brightness", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "blur_contrast", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "blur_saturation", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "blur_noise", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "blur_grayscale", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "transparency_enabled", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "transparency_alpha", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "transparency_blur", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "transparency_shadow", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "transparency_shadow_color", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "transparency_shadow_radius", NULL)) missing_option = TRUE;
    config->background_opacity = g_key_file_get_double(keyfile, "Style", "background_opacity", NULL);
    config->background_blur = g_key_file_get_double(keyfile, "Style", "background_blur", NULL);
    g_free(config->background_color);
    config->background_color = g_key_file_get_string(keyfile, "Style", "background_color", NULL);
    config->corner_radius = g_key_file_get_integer(keyfile, "Style", "corner_radius", NULL);
    config->border_width = g_key_file_get_integer(keyfile, "Style", "border_width", NULL);
    g_free(config->border_color);
    config->border_color = g_key_file_get_string(keyfile, "Style", "border_color", NULL);
    config->border_corner_radius = g_key_file_get_integer(keyfile, "Style", "border_corner_radius", NULL);
    
    // AGS-style effects
    config->blur_enabled = g_key_file_get_boolean(keyfile, "Style", "blur_enabled", NULL);
    config->blur_radius = g_key_file_get_integer(keyfile, "Style", "blur_radius", NULL);
    config->blur_brightness = g_key_file_get_double(keyfile, "Style", "blur_brightness", NULL);
    config->blur_contrast = g_key_file_get_double(keyfile, "Style", "blur_contrast", NULL);
    config->blur_saturation = g_key_file_get_double(keyfile, "Style", "blur_saturation", NULL);
    config->blur_noise = g_key_file_get_double(keyfile, "Style", "blur_noise", NULL);
    config->blur_grayscale = g_key_file_get_boolean(keyfile, "Style", "blur_grayscale", NULL);
    
    // Transparency settings
    config->transparency_enabled = g_key_file_get_boolean(keyfile, "Style", "transparency_enabled", NULL);
    config->transparency_alpha = g_key_file_get_double(keyfile, "Style", "transparency_alpha", NULL);
    config->transparency_blur = g_key_file_get_boolean(keyfile, "Style", "transparency_blur", NULL);
    config->transparency_shadow = g_key_file_get_boolean(keyfile, "Style", "transparency_shadow", NULL);
    g_free(config->transparency_shadow_color);
    config->transparency_shadow_color = g_key_file_get_string(keyfile, "Style", "transparency_shadow_color", NULL);
    config->transparency_shadow_radius = g_key_file_get_integer(keyfile, "Style", "transparency_shadow_radius", NULL);
    
    // --- New color roles ---
    g_free(config->text_color);
    config->text_color = g_key_file_get_string(keyfile, "Style", "text_color", NULL);
    g_free(config->button_color);
    config->button_color = g_key_file_get_string(keyfile, "Style", "button_color", NULL);
    g_free(config->button_text_color);
    config->button_text_color = g_key_file_get_string(keyfile, "Style", "button_text_color", NULL);
    g_free(config->highlight_color);
    config->highlight_color = g_key_file_get_string(keyfile, "Style", "highlight_color", NULL);
    g_free(config->search_color);
    config->search_color = g_key_file_get_string(keyfile, "Style", "search_color", NULL);
    g_free(config->system_button_color);
    config->system_button_color = g_key_file_get_string(keyfile, "Style", "system_button_color", NULL);
    g_free(config->system_button_icon_color);
    config->system_button_icon_color = g_key_file_get_string(keyfile, "Style", "system_button_icon_color", NULL);
    config->use_pywal_colors = g_key_file_get_boolean(keyfile, "Style", "use_pywal_colors", NULL);
  }
  
  // Behavior section
  if (g_key_file_has_group(keyfile, "Behavior")) {
    if (!g_key_file_has_key(keyfile, "Behavior", "close_on_click_outside", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Behavior", "close_on_super_key", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Behavior", "close_on_app_launch", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Behavior", "focus_search_on_open", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Behavior", "max_recent_apps", NULL)) missing_option = TRUE;
    config->close_on_click_outside = g_key_file_get_boolean(keyfile, "Behavior", "close_on_click_outside", NULL);
    config->close_on_super_key = g_key_file_get_boolean(keyfile, "Behavior", "close_on_super_key", NULL);
    config->close_on_app_launch = g_key_file_get_boolean(keyfile, "Behavior", "close_on_app_launch", NULL);
    config->focus_search_on_open = g_key_file_get_boolean(keyfile, "Behavior", "focus_search_on_open", NULL);
    config->max_recent_apps = g_key_file_get_integer(keyfile, "Behavior", "max_recent_apps", NULL);
  }
  
  // View section
  if (g_key_file_has_group(keyfile, "View")) {
    if (!g_key_file_has_key(keyfile, "View", "use_grid_view", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "View", "grid_columns", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "View", "grid_item_size", NULL)) missing_option = TRUE;
    config->use_grid_view = g_key_file_get_boolean(keyfile, "View", "use_grid_view", NULL);
    config->grid_columns = g_key_file_get_integer(keyfile, "View", "grid_columns", NULL);
    config->grid_item_size = g_key_file_get_integer(keyfile, "View", "grid_item_size", NULL);
  }
  
  // If any option is missing, regenerate the config file
  if (missing_option) {
    g_message("Missing config option(s) detected. Regenerating config file with all options.");
    hyprmenu_config_save();
  }
  
  return TRUE;
}

gboolean
hyprmenu_config_save_with_error(GError **error)
{
  // For background saving, avoid too much debug printing
  static gboolean is_saving = FALSE;
  
  // Prevent concurrent saves
  if (is_saving) {
    g_set_error(error, G_IO_ERROR, G_IO_ERROR_BUSY, "Config save already in progress");
    return FALSE;
  }
  
  g_print("hyprmenu_config_save_with_error: Starting save operation\n");
  is_saving = TRUE;
  
  // Prevent saving as root
  if (geteuid() == 0) {
    g_set_error(error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, 
                "Refusing to save config as root. Please run HyprMenu as a normal user.");
    is_saving = FALSE;
    return FALSE;
  }
  g_print("hyprmenu_config_save_with_error: Not running as root, proceeding\n");

  // Check if config directory exists, create if not
  if (!g_file_test(config->config_dir, G_FILE_TEST_IS_DIR)) {
    g_print("Creating config directory: %s\n", config->config_dir);
    if (g_mkdir_with_parents(config->config_dir, 0755) != 0) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, 
                  "Failed to create config directory: %s", config->config_dir);
      is_saving = FALSE;
      return FALSE;
    }
  }

  // Check if config file is writable (if it exists)
  if (g_file_test(config->config_file, G_FILE_TEST_EXISTS)) {
    if (access(config->config_file, W_OK) != 0) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED,
                  "Config file %s is not writable by the current user.", config->config_file);
      is_saving = FALSE;
      return FALSE;
    }
    g_print("hyprmenu_config_save_with_error: Config file exists and is writable\n");
  } else {
    // Check if parent directory is writable
    char *parent_dir = g_path_get_dirname(config->config_file);
    if (access(parent_dir, W_OK) != 0) {
      g_set_error(error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED,
                  "Parent directory %s is not writable by the current user.", parent_dir);
      g_free(parent_dir);
      is_saving = FALSE;
      return FALSE;
    }
    g_free(parent_dir);
    g_print("hyprmenu_config_save_with_error: Config file doesn't exist yet, will create\n");
  }

  g_autoptr(GKeyFile) keyfile = g_key_file_new();
  
  // Read existing file if it exists to preserve other settings
  if (g_file_test(config->config_file, G_FILE_TEST_EXISTS)) {
    g_print("Reading existing config file to preserve settings\n");
    GError *load_error = NULL;
    gboolean loaded = g_key_file_load_from_file(keyfile, config->config_file, G_KEY_FILE_KEEP_COMMENTS, &load_error);
    if (!loaded) {
      g_warning("Failed to load existing config file: %s. Will create a new one", load_error->message);
      g_error_free(load_error);
    }
  }
  
  // Window layout
  g_key_file_set_integer(keyfile, "Layout", "window_width", config->window_width);
  g_key_file_set_integer(keyfile, "Layout", "window_height", config->window_height);
  g_key_file_set_integer(keyfile, "Layout", "top_margin", config->top_margin);
  g_key_file_set_integer(keyfile, "Layout", "left_margin", config->left_margin);
  g_key_file_set_boolean(keyfile, "Layout", "center_window", config->center_window);
  g_key_file_set_integer(keyfile, "Layout", "bottom_offset", config->bottom_offset);
  g_key_file_set_integer(keyfile, "Layout", "top_offset", config->top_offset);
  
  // Save position setting
  g_key_file_set_string(keyfile, "Layout", "menu_position", 
                       hyprmenu_position_to_string(config->menu_position));
  
  // Window style
  g_key_file_set_double(keyfile, "Style", "background_opacity", config->background_opacity);
  g_key_file_set_double(keyfile, "Style", "background_blur", config->background_blur);
  g_key_file_set_string(keyfile, "Style", "background_color", config->background_color);
  g_key_file_set_integer(keyfile, "Style", "corner_radius", config->corner_radius);
  g_key_file_set_integer(keyfile, "Style", "border_width", config->border_width);
  g_key_file_set_string(keyfile, "Style", "border_color", config->border_color);
  g_key_file_set_integer(keyfile, "Style", "border_corner_radius", config->border_corner_radius);
  
  // AGS-style effects
  g_key_file_set_boolean(keyfile, "Style", "blur_enabled", config->blur_enabled);
  g_key_file_set_integer(keyfile, "Style", "blur_radius", config->blur_radius);
  g_key_file_set_double(keyfile, "Style", "blur_brightness", config->blur_brightness);
  g_key_file_set_double(keyfile, "Style", "blur_contrast", config->blur_contrast);
  g_key_file_set_double(keyfile, "Style", "blur_saturation", config->blur_saturation);
  g_key_file_set_double(keyfile, "Style", "blur_noise", config->blur_noise);
  g_key_file_set_boolean(keyfile, "Style", "blur_grayscale", config->blur_grayscale);
  
  // Transparency settings
  g_key_file_set_boolean(keyfile, "Style", "transparency_enabled", config->transparency_enabled);
  g_key_file_set_double(keyfile, "Style", "transparency_alpha", config->transparency_alpha);
  g_key_file_set_boolean(keyfile, "Style", "transparency_blur", config->transparency_blur);
  g_key_file_set_boolean(keyfile, "Style", "transparency_shadow", config->transparency_shadow);
  g_key_file_set_string(keyfile, "Style", "transparency_shadow_color", config->transparency_shadow_color);
  g_key_file_set_integer(keyfile, "Style", "transparency_shadow_radius", config->transparency_shadow_radius);
  
  // Behavior
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_click_outside", config->close_on_click_outside);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_super_key", config->close_on_super_key);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_app_launch", config->close_on_app_launch);
  g_key_file_set_boolean(keyfile, "Behavior", "focus_search_on_open", config->focus_search_on_open);
  g_key_file_set_integer(keyfile, "Behavior", "max_recent_apps", config->max_recent_apps);
  
  // View settings - this is what we're primarily concerned with
  g_print("Writing view settings: use_grid_view=%s\n", config->use_grid_view ? "true" : "false");
  g_key_file_set_boolean(keyfile, "View", "use_grid_view", config->use_grid_view);
  
  // Verify the value was correctly set in the key file
  GError *verify_error = NULL;
  gboolean verified_value = g_key_file_get_boolean(keyfile, "View", "use_grid_view", &verify_error);
  if (verify_error) {
    g_print("Error verifying view setting: %s\n", verify_error->message);
    g_error_free(verify_error);
  } else {
    g_print("Key file value verified: use_grid_view=%s\n", verified_value ? "true" : "false");
  }
  
  g_key_file_set_integer(keyfile, "View", "grid_columns", config->grid_columns);
  g_key_file_set_integer(keyfile, "View", "grid_item_size", config->grid_item_size);
  
  // --- New color roles ---
  g_key_file_set_string(keyfile, "Style", "text_color", config->text_color);
  g_key_file_set_string(keyfile, "Style", "button_color", config->button_color);
  g_key_file_set_string(keyfile, "Style", "button_text_color", config->button_text_color);
  g_key_file_set_string(keyfile, "Style", "highlight_color", config->highlight_color);
  g_key_file_set_string(keyfile, "Style", "search_color", config->search_color);
  g_key_file_set_string(keyfile, "Style", "system_button_color", config->system_button_color);
  g_key_file_set_string(keyfile, "Style", "system_button_icon_color", config->system_button_icon_color);
  g_key_file_set_boolean(keyfile, "Style", "use_pywal_colors", config->use_pywal_colors);
  
  // Save to file
  g_print("Writing config to: %s\n", config->config_file);
  g_autofree char *data = g_key_file_to_data(keyfile, NULL, error);
  if (!data) {
    g_warning("Failed to convert config to data: %s", (*error)->message);
    is_saving = FALSE;
    return FALSE;
  }
  
  if (!g_file_set_contents(config->config_file, data, -1, error)) {
    g_warning("Failed to save config file: %s", (*error)->message);
    is_saving = FALSE;
    return FALSE;
  }
  
  // Force sync to ensure the file is written to disk
  fsync(0);  // Use fsync on stdout instead of sync()
  
  g_print("Configuration saved successfully\n");
  is_saving = FALSE;
  return TRUE;
}

gboolean
hyprmenu_config_save()
{
  GError *error = NULL;
  gboolean result = hyprmenu_config_save_with_error(&error);
  
  if (!result && error) {
    g_warning("Config save failed: %s", error->message);
    g_error_free(error);
  }
  
  return result;
}

void
hyprmenu_config_apply_css()
{
  // Generate CSS based on configuration
  GString *css = g_string_new("");
  
  // Window styles with AGS effects
  g_string_append_printf(css, 
    ".hyprmenu-window {\n"
    "  background-color: %s;\n"
    "  border-radius: %dpx; /* main window radius */\n"
    "  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);\n"
    "  border: %dpx solid %s;\n"
    "  border-radius: %dpx; /* border radius, takes precedence in GTK */\n"
    "}\n",
    config->background_color,
    config->corner_radius,
    config->border_width,
    config->border_color,
    config->border_corner_radius);
  
  // Add AGS-style blur effects if enabled
  if (config->blur_enabled) {
    g_string_append_printf(css,
      "  filter: blur(%dpx) brightness(%.1f) contrast(%.1f) saturate(%.1f);\n",
      config->blur_radius,
      config->blur_brightness,
      config->blur_contrast,
      config->blur_saturation);
    
    if (config->blur_grayscale) {
      g_string_append(css, "  filter: grayscale(100%);\n");
    }
  }
  
  // Add transparency effects if enabled
  if (config->transparency_enabled) {
    g_string_append_printf(css,
      "  background-color: rgba(%s, %.2f);\n",
      config->background_color, config->transparency_alpha);
    
    if (config->transparency_shadow) {
      g_string_append_printf(css,
        "  box-shadow: 0 0 %dpx %s;\n",
        config->transparency_shadow_radius,
        config->transparency_shadow_color);
    }
  }
  
  g_string_append(css, "}\n");
  
  // Main box styles
  g_string_append(css,
    ".hyprmenu-main-box {\n"
    "  padding: 8px;\n"
    "}\n");
  
  // Search entry styles
  g_string_append_printf(css, 
    ".hyprmenu-search {\n"
    "  background-color: %s;\n"
    "  border-radius: %dpx;\n"
    "  padding: %dpx 12px;\n"
    "  font-size: %dpx;\n"
    "  color: %s;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.15);\n"
    "  margin: 8px 4px 12px 4px;\n"
    "  min-width: 200px;\n",
    config->search_background_color,
    config->search_corner_radius,
    config->search_padding,
    config->search_font_size,
    config->search_text_color);
  
  g_string_append(css,
    "}\n"
    ".hyprmenu-search:focus {\n"
    "  border-color: rgba(255, 255, 255, 0.3);\n"
    "  box-shadow: 0 0 0 1px rgba(255, 255, 255, 0.15);\n"
    "}\n");
  
  // App grid styles
  g_string_append(css,
    ".hyprmenu-app-grid {\n"
    "  background-color: transparent;\n"
    "  padding: 4px;\n"
    "  margin: 0 auto;\n"
    "}\n"
    ".hyprmenu-category-list {\n"
    "  background-color: transparent;\n"
    "  padding: 4px;\n"
    "  margin: 0 auto;\n"
    "  min-width: 200px;\n"
    "}\n");
  
  // Category styles
  g_string_append_printf(css, 
    ".hyprmenu-category {\n"
    "  background-color: %s;\n"
    "  border-radius: %dpx;\n"
    "  margin: 8px 0px;\n"
    "  padding: 0px 4px;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.05);\n",
    config->category_background_color,
    config->category_corner_radius);
  
  g_string_append_printf(css,
    "}\n"
    ".hyprmenu-category-title {\n"
    "  color: %s;\n"
    "  font-size: 14px;\n"
    "  font-weight: bold;\n"
    "  padding: 0px 4px;\n"
    "  margin-bottom: 4px;\n",
    config->category_text_color);
  
  // Add separator if configured
  if (config->category_show_separators) {
    g_string_append(css, "  border-bottom: 1px solid rgba(255, 255, 255, 0.1);\n");
  }
  
  g_string_append(css, "}\n");
  
  // App entry styles
  g_string_append_printf(css, 
    ".hyprmenu-app-entry {\n"
    "  background-color: %s;\n"
    "  border-radius: %dpx;\n"
    "  padding: %dpx 8px;\n"
    "  margin: 2px 4px;\n"
    "  transition: all 200ms ease;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.05);\n"
    "  min-height: 36px;\n",
    config->app_entry_background_color,
    config->app_entry_corner_radius,
    config->app_entry_padding);
  
  g_string_append(css,
    "}\n"
    ".hyprmenu-app-entry:hover {\n"
    "  background-color: rgba(100, 100, 100, 0.8);\n"
    "  border: 1px solid rgba(255, 255, 255, 0.2);\n"
    "}\n"
    ".hyprmenu-app-entry:active {\n"
    "  background-color: rgba(100, 100, 100, 0.9);\n"
    "  border: 1px solid rgba(255, 255, 255, 0.3);\n"
    "}\n"
    ".hyprmenu-list-row {\n"
    "  background-color: rgba(60, 60, 70, 0.6);\n"
    "  border-radius: 6px;\n"
    "  padding: 8px 12px;\n"
    "  margin: 2px 0px;\n"
    "  transition: all 150ms ease;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.05);\n"
    "  min-height: 42px;\n"
    "}\n"
    ".hyprmenu-list-row:hover {\n"
    "  background-color: rgba(80, 80, 90, 0.7);\n"
    "  border-color: rgba(255, 255, 255, 0.2);\n"
    "}\n"
    ".hyprmenu-list-row:active {\n"
    "  background-color: rgba(90, 90, 100, 0.8);\n"
    "  border-color: rgba(255, 255, 255, 0.3);\n"
    "}\n"
    ".grid-item {\n"
    "  background-color: rgba(50, 50, 60, 0.7);\n"
    "  border-radius: 8px;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.08);\n"
    "  min-width: 100px;\n"
    "  min-height: 100px;\n"
    "}\n"
    ".grid-item:hover {\n"
    "  background-color: rgba(70, 70, 80, 0.8);\n"
    "  border-color: rgba(255, 255, 255, 0.2);\n"
    "}\n"
    ".hyprmenu-app-grid flowboxchild {\n"
    "  padding: 0;\n"
    "  margin: 0;\n"
    "  min-width: 100px;\n"
    "  min-height: 100px;\n"
    "}\n");
    
  // App name styles
  g_string_append_printf(css,
    ".app-name {\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "  font-weight: bold;\n"
    "  margin-bottom: 2px;\n"
    "}\n",
    config->app_entry_text_color,
    config->app_entry_font_size);
    
  // App description styles
  g_string_append_printf(css,
    ".app-description {\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "  opacity: 0.7;\n"
    "  padding: 2px 4px;\n"
    "  border-radius: 4px;\n"
    "}\n",
    config->app_entry_text_color,
    config->app_entry_font_size - 2);
  
  // Scrollbar styles
  g_string_append(css,
    "scrollbar {\n"
    "  background-color: transparent;\n"
    "  border-radius: 6px;\n"
    "  margin: 2px;\n"
    "}\n"
    "scrollbar slider {\n"
    "  min-width: 6px;\n"
    "  min-height: 6px;\n"
    "  background-color: rgba(255, 255, 255, 0.2);\n"
    "  border-radius: 3px;\n"
    "}\n"
    "scrollbar slider:hover {\n"
    "  background-color: rgba(255, 255, 255, 0.3);\n"
    "}\n"
    "scrollbar slider:active {\n"
    "  background-color: rgba(255, 255, 255, 0.4);\n"
    "}\n");
  
  // System buttons styling
  g_string_append(css,
    ".hyprmenu-system-buttons {\n"
    "  background-color: rgba(40, 42, 54, 0.7);\n"
    "  border-radius: 6px;\n"
    "  padding: 4px 8px;\n"
    "  margin: 8px 0px 2px 0px;\n"
    "  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.2);\n"
    "}\n"
    
    ".system-button {\n"
    "  background-color: transparent;\n"
    "  border-radius: 4px;\n"
    "  border: none;\n"
    "  padding: 4px;\n"
    "  min-width: 24px;\n"
    "  min-height: 24px;\n"
    "  margin: 0 4px;\n"
    "  transition: all 150ms ease;\n"
    "  color: rgba(255, 255, 255, 0.85);\n"
    "  outline: none;\n"
    "}\n"
    
    ".system-button:hover {\n"
    "  background-color: rgba(80, 85, 100, 0.5);\n"
    "  color: rgba(255, 255, 255, 1.0);\n"
    "  box-shadow: 0 1px 1px rgba(0, 0, 0, 0.2);\n"
    "}\n"
    
    ".system-button:active {\n"
    "  background-color: rgba(90, 95, 120, 0.6);\n"
    "  transform: translateY(1px);\n"
    "  box-shadow: none;\n"
    "  color: rgba(255, 255, 255, 1.0);\n"
    "}\n"
    
    // Add dialog styling
    ".session-bg {\n"
    "  background-color: rgba(40, 42, 54, 0.6);\n"
    "}\n"
    
    ".session-button {\n"
    "  min-width: 8.182rem;\n"
    "  min-height: 8.182rem;\n"
    "  background-color: rgba(56, 58, 66, 0.95);\n"
    "  color: #f8f8f2;\n"
    "  font-size: 3rem;\n"
    "  border-radius: 1rem;\n"
    "  border: none;\n"
    "  transition: all 200ms cubic-bezier(0.4, 0, 0.2, 1);\n"
    "}\n"
    
    ".session-button:hover {\n"
    "  background-color: rgba(68, 71, 81, 0.95);\n"
    "}\n"
    
    ".session-button:focus {\n"
    "  background-color: rgba(68, 71, 81, 0.95);\n"
    "  outline: 2px solid rgba(255, 255, 255, 0.1);\n"
    "}\n"
    
    ".session-button-desc {\n"
    "  background-color: rgba(68, 71, 81, 0.95);\n"
    "  color: #f8f8f2;\n"
    "  border-radius: 0.5rem;\n"
    "  padding: 0.205rem 0.341rem;\n"
    "  font-weight: 700;\n"
    "  font-size: 0.9rem;\n"
    "}\n"
    
    ".txt-title {\n"
    "  font-size: 1.5rem;\n"
    "  font-weight: bold;\n"
    "  color: #f8f8f2;\n"
    "  margin-bottom: 0.5rem;\n"
    "}\n"
    
    ".txt-small {\n"
    "  font-size: 0.9rem;\n"
    "  color: rgba(248, 248, 242, 0.7);\n"
    "}\n"
    
    ".spacing-v-15 {\n"
    "  margin: 15px 0;\n"
    "}\n"
    
    // Session button colors
    ".session-color-1 { color: #8be9fd; }\n"  // Lock
    ".session-color-2 { color: #50fa7b; }\n"  // Logout
    ".session-color-3 { color: #f1fa8c; }\n"  // Sleep
    ".session-color-4 { color: #bd93f9; }\n"  // Hibernate
    ".session-color-5 { color: #ff5555; }\n"  // Shutdown/Yes
    ".session-color-6 { color: #ff79c6; }\n"  // Reboot
    ".session-color-7 { color: #bfbfbf; }\n"  // Cancel/No
  );
  
  // Apply CSS to application
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(provider, css->str);
  
  // Apply provider to screen
  GdkDisplay *display = gdk_display_get_default();
  gtk_style_context_add_provider_for_display(display,
                                            GTK_STYLE_PROVIDER(provider),
                                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  // Free resources
  g_object_unref(provider);
  g_string_free(css, TRUE);
  
  // Load custom CSS file if it exists
  if (g_file_test(config->css_file, G_FILE_TEST_EXISTS)) {
    GtkCssProvider *custom_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(custom_provider, config->css_file);
    
    gtk_style_context_add_provider_for_display(display,
                                              GTK_STYLE_PROVIDER(custom_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    g_object_unref(custom_provider);
  }
} 