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
  config->menu_position = POSITION_TOP_LEFT;
  config->bottom_offset = 55;
  config->top_offset = 48;
  config->window_padding = 8;
  
  // Window style
  config->window_background_opacity = 1.0;
  config->window_background_blur = 5.0;
  config->window_background_color = g_strdup("");
  config->inner_border_radius = 12;
  config->inner_border_width = 2;
  config->inner_border_color = g_strdup("#444444");
  config->window_shadow_color = g_strdup("rgba(0,0,0,0.3)");
  config->window_shadow_radius = 20;
  config->window_halign = g_strdup("center");
  config->window_valign = g_strdup("center");
  
  // Grid options
  config->grid_margin_start = 12;
  config->grid_margin_end = 12;
  config->grid_margin_top = 12;
  config->grid_margin_bottom = 12;
  config->grid_row_spacing = 12;
  config->grid_column_spacing = 12;
  config->grid_halign = g_strdup("center");
  config->grid_valign = g_strdup("center");
  config->grid_hexpand = TRUE;
  config->grid_vexpand = FALSE;
  config->grid_columns = 5;
  config->grid_item_size = 100;
  config->grid_item_corner_radius = 8;
  config->grid_item_border_width = 1;
  config->grid_item_border_color = g_strdup("rgba(255,255,255,0.08)");
  config->grid_item_background_color = g_strdup("rgba(50,50,60,0.7)");
  config->grid_opacity = 1.0;
  config->grid_item_opacity = 1.0;
  
  // List options
  config->list_item_size = 48;
  config->list_item_corner_radius = 6;
  config->list_item_border_width = 1;
  config->list_item_border_color = g_strdup("rgba(255,255,255,0.05)");
  config->list_item_background_color = g_strdup("rgba(60,60,70,0.6)");
  config->list_row_spacing = 8;
  config->list_halign = g_strdup("fill");
  config->list_valign = g_strdup("center");
  config->list_hexpand = TRUE;
  config->list_vexpand = FALSE;
  config->list_margin_start = 12;
  config->list_margin_end = 12;
  config->list_margin_top = 12;
  config->list_margin_bottom = 12;
  config->list_opacity = 1.0;
  config->list_item_opacity = 1.0;
  
  // AppEntry options
  config->app_icon_size = 32;
  config->app_icon_corner_radius = 6;
  config->app_icon_background_color = g_strdup("rgba(60,60,70,0.6)");
  config->app_name_font_size = 12;
  config->app_name_color = g_strdup("#ffffff");
  config->app_desc_font_size = 10;
  config->app_desc_color = g_strdup("rgba(255,255,255,0.7)");
  config->app_entry_padding = 6;
  config->app_entry_hover_color = g_strdup("rgba(100,100,100,0.8)");
  config->app_entry_active_color = g_strdup("rgba(100,100,100,0.9)");
  config->app_entry_opacity = 1.0;
  config->app_icon_opacity = 1.0;
  config->app_name_opacity = 1.0;
  config->app_desc_opacity = 1.0;
  
  // Category options
  config->category_background_color = g_strdup("#2d2d2d");
  config->category_background_opacity = 1.0;
  config->category_corner_radius = 10;
  config->category_text_color = g_strdup("");
  config->category_font_size = 13;
  config->category_font_family = g_strdup("Sans Bold");
  config->category_padding = 6;
  config->category_show_separators = TRUE;
  config->category_separator_color = g_strdup("rgba(255,255,255,0.1)");
  config->category_opacity = 1.0;
  config->category_title_opacity = 1.0;
  
  // Search options
  config->search_background_color = g_strdup("rgba(34, 34, 34, 0.3)");
  config->search_background_opacity = 1.0;
  config->search_corner_radius = 8;
  config->search_text_color = g_strdup("");
  config->search_font_size = 14;
  config->search_font_family = g_strdup("Sans");
  config->search_padding = 8;
  config->search_min_height = 20;
  config->search_left_padding = 2;
  config->search_length = 0;
  config->search_placeholder_text = g_strdup("Search applications...");
  config->search_icon_size = 16;
  config->search_icon_color = g_strdup("rgba(255,255,255,0.7)");
  config->search_focus_border_color = g_strdup("rgba(255,255,255,0.2)");
  config->search_focus_shadow_color = g_strdup("rgba(0,0,0,0.1)");
  config->search_opacity = 1.0;
  config->search_text_opacity = 1.0;
  config->search_icon_opacity = 1.0;
  
  // SystemButton options
  config->system_button_background_color = g_strdup("rgba(60,60,70,0.6)");
  config->system_button_icon_color = g_strdup("rgba(255,255,255,0.7)");
  config->system_button_hover_color = g_strdup("rgba(100,100,100,0.8)");
  config->system_button_active_color = g_strdup("rgba(100,100,100,0.9)");
  config->system_button_corner_radius = 6;
  config->system_button_size = 32;
  config->system_button_spacing = 8;
  config->system_button_opacity = 1.0;
  config->system_button_icon_opacity = 1.0;
  
  // Behavior options
  config->close_on_click_outside = TRUE;
  config->close_on_super_key = TRUE;
  config->close_on_app_launch = TRUE;
  config->focus_search_on_open = TRUE;
  config->close_on_escape = TRUE;
  config->close_on_focus_out = TRUE;
  config->show_categories = TRUE;
  config->show_descriptions = TRUE;
  config->show_icons = TRUE;
  config->show_search = TRUE;
  config->show_scrollbar = TRUE;
  config->show_border = TRUE;
  config->show_shadow = TRUE;
  config->blur_background = TRUE;
  config->blur_strength = 10;
  config->opacity = 1.0;
  config->max_recent_apps = 10;  // Default to showing 10 recent apps
  
  // File paths
  config->config_dir = g_build_filename(g_get_home_dir(), ".config", "hyprmenu", NULL);
  config->config_file = g_build_filename(config->config_dir, "hyprmenu.conf", NULL);
  config->css_file = g_build_filename(config->config_dir, "hyprmenu.css", NULL);

  // Outer border (window)
  config->outer_border_width = 3;
  config->outer_border_color = g_strdup("#888888");
  config->outer_border_radius = 16;
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
  
  // Free allocated strings
  g_free(config->config_dir);
  g_free(config->config_file);
  g_free(config->css_file);
  g_free(config->window_border_color);
  g_free(config->window_background_color);
  g_free(config->window_halign);
  g_free(config->window_valign);
  g_free(config->window_shadow_color);
  g_free(config->inner_border_color);
  g_free(config->outer_border_color);
  g_free(config->grid_halign);
  g_free(config->grid_valign);
  g_free(config->grid_item_border_color);
  g_free(config->grid_item_background_color);
  g_free(config->list_item_border_color);
  g_free(config->list_item_background_color);
  g_free(config->list_halign);
  g_free(config->list_valign);
  g_free(config->app_icon_background_color);
  g_free(config->app_name_color);
  g_free(config->app_desc_color);
  g_free(config->app_entry_hover_color);
  g_free(config->app_entry_active_color);
  g_free(config->category_background_color);
  g_free(config->category_text_color);
  g_free(config->category_font_family);
  g_free(config->category_separator_color);
  g_free(config->search_background_color);
  g_free(config->search_text_color);
  g_free(config->search_font_family);
  g_free(config->search_placeholder_text);
  g_free(config->search_icon_color);
  g_free(config->search_focus_border_color);
  g_free(config->search_focus_shadow_color);
  g_free(config->system_button_background_color);
  g_free(config->system_button_icon_color);
  g_free(config->system_button_hover_color);
  g_free(config->system_button_active_color);
  
  // Free the config struct itself
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
    if (!g_key_file_has_key(keyfile, "Layout", "window_padding", NULL)) missing_option = TRUE;
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
    config->window_padding = g_key_file_get_integer(keyfile, "Layout", "window_padding", NULL);
  }
  
  // Window style
  if (g_key_file_has_group(keyfile, "Style")) {
    if (!g_key_file_has_key(keyfile, "Style", "window_background_opacity", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "window_background_blur", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "window_background_color", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "inner_border_radius", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "inner_border_width", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "inner_border_color", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "window_shadow_color", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "window_shadow_radius", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "window_halign", NULL)) missing_option = TRUE;
    if (!g_key_file_has_key(keyfile, "Style", "window_valign", NULL)) missing_option = TRUE;
    config->window_background_opacity = g_key_file_get_double(keyfile, "Style", "window_background_opacity", NULL);
    config->window_background_blur = g_key_file_get_double(keyfile, "Style", "window_background_blur", NULL);
    g_free(config->window_background_color);
    config->window_background_color = g_key_file_get_string(keyfile, "Style", "window_background_color", NULL);
    config->inner_border_radius = g_key_file_get_integer(keyfile, "Style", "inner_border_radius", NULL);
    config->inner_border_width = g_key_file_get_integer(keyfile, "Style", "inner_border_width", NULL);
    g_free(config->inner_border_color);
    config->inner_border_color = g_key_file_get_string(keyfile, "Style", "inner_border_color", NULL);
    g_free(config->window_shadow_color);
    config->window_shadow_color = g_key_file_get_string(keyfile, "Style", "window_shadow_color", NULL);
    config->window_shadow_radius = g_key_file_get_integer(keyfile, "Style", "window_shadow_radius", NULL);
    g_free(config->window_halign);
    config->window_halign = g_key_file_get_string(keyfile, "Style", "window_halign", NULL);
    g_free(config->window_valign);
    config->window_valign = g_key_file_get_string(keyfile, "Style", "window_valign", NULL);
  }
  
  // Grid options
  if (g_key_file_has_group(keyfile, "Grid")) {
    config->grid_margin_start = g_key_file_get_integer(keyfile, "Grid", "margin_start", NULL);
    config->grid_margin_end = g_key_file_get_integer(keyfile, "Grid", "margin_end", NULL);
    config->grid_margin_top = g_key_file_get_integer(keyfile, "Grid", "margin_top", NULL);
    config->grid_margin_bottom = g_key_file_get_integer(keyfile, "Grid", "margin_bottom", NULL);
    config->grid_row_spacing = g_key_file_get_integer(keyfile, "Grid", "row_spacing", NULL);
    config->grid_column_spacing = g_key_file_get_integer(keyfile, "Grid", "column_spacing", NULL);
    g_free(config->grid_halign);
    config->grid_halign = g_key_file_get_string(keyfile, "Grid", "halign", NULL);
    g_free(config->grid_valign);
    config->grid_valign = g_key_file_get_string(keyfile, "Grid", "valign", NULL);
    config->grid_hexpand = g_key_file_get_boolean(keyfile, "Grid", "hexpand", NULL);
    config->grid_vexpand = g_key_file_get_boolean(keyfile, "Grid", "vexpand", NULL);
    config->grid_columns = g_key_file_get_integer(keyfile, "Grid", "columns", NULL);
    config->grid_item_size = g_key_file_get_integer(keyfile, "Grid", "item_size", NULL);
    config->grid_item_corner_radius = g_key_file_get_integer(keyfile, "Grid", "item_corner_radius", NULL);
    g_free(config->grid_item_border_color);
    config->grid_item_border_color = g_key_file_get_string(keyfile, "Grid", "item_border_color", NULL);
    g_free(config->grid_item_background_color);
    config->grid_item_background_color = g_key_file_get_string(keyfile, "Grid", "item_background_color", NULL);
  }
  
  // List options
  if (g_key_file_has_group(keyfile, "List")) {
    config->list_item_size = g_key_file_get_integer(keyfile, "List", "item_size", NULL);
    config->list_item_corner_radius = g_key_file_get_integer(keyfile, "List", "item_corner_radius", NULL);
    g_free(config->list_item_border_color);
    config->list_item_border_color = g_key_file_get_string(keyfile, "List", "item_border_color", NULL);
    g_free(config->list_item_background_color);
    config->list_item_background_color = g_key_file_get_string(keyfile, "List", "item_background_color", NULL);
    config->list_row_spacing = g_key_file_get_integer(keyfile, "List", "row_spacing", NULL);
    g_free(config->list_halign);
    config->list_halign = g_key_file_get_string(keyfile, "List", "halign", NULL);
    g_free(config->list_valign);
    config->list_valign = g_key_file_get_string(keyfile, "List", "valign", NULL);
    config->list_hexpand = g_key_file_get_boolean(keyfile, "List", "hexpand", NULL);
    config->list_vexpand = g_key_file_get_boolean(keyfile, "List", "vexpand", NULL);
    config->list_margin_start = g_key_file_get_integer(keyfile, "List", "margin_start", NULL);
    config->list_margin_end = g_key_file_get_integer(keyfile, "List", "margin_end", NULL);
    config->list_margin_top = g_key_file_get_integer(keyfile, "List", "margin_top", NULL);
    config->list_margin_bottom = g_key_file_get_integer(keyfile, "List", "margin_bottom", NULL);
  }
  
  // AppEntry options
  if (g_key_file_has_group(keyfile, "AppEntry")) {
    config->app_icon_size = g_key_file_get_integer(keyfile, "AppEntry", "icon_size", NULL);
    config->app_icon_corner_radius = g_key_file_get_integer(keyfile, "AppEntry", "icon_corner_radius", NULL);
    g_free(config->app_icon_background_color);
    config->app_icon_background_color = g_key_file_get_string(keyfile, "AppEntry", "icon_background_color", NULL);
    config->app_name_font_size = g_key_file_get_integer(keyfile, "AppEntry", "name_font_size", NULL);
    g_free(config->app_name_color);
    config->app_name_color = g_key_file_get_string(keyfile, "AppEntry", "name_color", NULL);
    config->app_desc_font_size = g_key_file_get_integer(keyfile, "AppEntry", "desc_font_size", NULL);
    g_free(config->app_desc_color);
    config->app_desc_color = g_key_file_get_string(keyfile, "AppEntry", "desc_color", NULL);
    config->app_entry_padding = g_key_file_get_integer(keyfile, "AppEntry", "padding", NULL);
    g_free(config->app_entry_hover_color);
    config->app_entry_hover_color = g_key_file_get_string(keyfile, "AppEntry", "hover_color", NULL);
    g_free(config->app_entry_active_color);
    config->app_entry_active_color = g_key_file_get_string(keyfile, "AppEntry", "active_color", NULL);
  }
  
  // Category options
  if (g_key_file_has_group(keyfile, "Category")) {
    g_free(config->category_background_color);
    config->category_background_color = g_key_file_get_string(keyfile, "Category", "background_color", NULL);
    config->category_background_opacity = g_key_file_get_double(keyfile, "Category", "background_opacity", NULL);
    config->category_corner_radius = g_key_file_get_integer(keyfile, "Category", "corner_radius", NULL);
    g_free(config->category_text_color);
    config->category_text_color = g_key_file_get_string(keyfile, "Category", "text_color", NULL);
    config->category_font_size = g_key_file_get_integer(keyfile, "Category", "font_size", NULL);
    g_free(config->category_font_family);
    config->category_font_family = g_key_file_get_string(keyfile, "Category", "font_family", NULL);
    config->category_padding = g_key_file_get_integer(keyfile, "Category", "padding", NULL);
    config->category_show_separators = g_key_file_get_boolean(keyfile, "Category", "show_separators", NULL);
    g_free(config->category_separator_color);
    config->category_separator_color = g_key_file_get_string(keyfile, "Category", "separator_color", NULL);
  }
  
  // Search options
  if (g_key_file_has_group(keyfile, "Search")) {
    config->search_background_color = g_key_file_get_string(keyfile, "Search", "background_color", NULL);
    config->search_background_opacity = g_key_file_get_double(keyfile, "Search", "background_opacity", NULL);
    config->search_corner_radius = g_key_file_get_integer(keyfile, "Search", "corner_radius", NULL);
    g_free(config->search_text_color);
    config->search_text_color = g_key_file_get_string(keyfile, "Search", "text_color", NULL);
    config->search_font_size = g_key_file_get_integer(keyfile, "Search", "font_size", NULL);
    g_free(config->search_font_family);
    config->search_font_family = g_key_file_get_string(keyfile, "Search", "font_family", NULL);
    config->search_padding = g_key_file_get_integer(keyfile, "Search", "padding", NULL);
    config->search_min_height = g_key_file_get_integer(keyfile, "Search", "min_height", NULL);
    config->search_left_padding = g_key_file_get_integer(keyfile, "Search", "left_padding", NULL);
    config->search_length = g_key_file_get_integer(keyfile, "Search", "length", NULL);
    g_free(config->search_placeholder_text);
    config->search_placeholder_text = g_key_file_get_string(keyfile, "Search", "placeholder_text", NULL);
    config->search_icon_size = g_key_file_get_integer(keyfile, "Search", "icon_size", NULL);
    g_free(config->search_icon_color);
    config->search_icon_color = g_key_file_get_string(keyfile, "Search", "icon_color", NULL);
    g_free(config->search_focus_border_color);
    config->search_focus_border_color = g_key_file_get_string(keyfile, "Search", "focus_border_color", NULL);
    g_free(config->search_focus_shadow_color);
    config->search_focus_shadow_color = g_key_file_get_string(keyfile, "Search", "focus_shadow_color", NULL);
  }
  
  // SystemButton options
  if (g_key_file_has_group(keyfile, "SystemButton")) {
    g_free(config->system_button_background_color);
    config->system_button_background_color = g_key_file_get_string(keyfile, "SystemButton", "background_color", NULL);
    g_free(config->system_button_icon_color);
    config->system_button_icon_color = g_key_file_get_string(keyfile, "SystemButton", "icon_color", NULL);
    g_free(config->system_button_hover_color);
    config->system_button_hover_color = g_key_file_get_string(keyfile, "SystemButton", "hover_color", NULL);
    g_free(config->system_button_active_color);
    config->system_button_active_color = g_key_file_get_string(keyfile, "SystemButton", "active_color", NULL);
    config->system_button_corner_radius = g_key_file_get_integer(keyfile, "SystemButton", "corner_radius", NULL);
    config->system_button_size = g_key_file_get_integer(keyfile, "SystemButton", "size", NULL);
    config->system_button_spacing = g_key_file_get_integer(keyfile, "SystemButton", "spacing", NULL);
  }
  
  // Behavior options
  if (g_key_file_has_group(keyfile, "Behavior")) {
    if (g_key_file_has_key(keyfile, "Behavior", "close_on_click_outside", NULL)) {
    config->close_on_click_outside = g_key_file_get_boolean(keyfile, "Behavior", "close_on_click_outside", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "close_on_super_key", NULL)) {
    config->close_on_super_key = g_key_file_get_boolean(keyfile, "Behavior", "close_on_super_key", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "close_on_app_launch", NULL)) {
    config->close_on_app_launch = g_key_file_get_boolean(keyfile, "Behavior", "close_on_app_launch", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "focus_search_on_open", NULL)) {
    config->focus_search_on_open = g_key_file_get_boolean(keyfile, "Behavior", "focus_search_on_open", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "close_on_escape", NULL)) {
    config->close_on_escape = g_key_file_get_boolean(keyfile, "Behavior", "close_on_escape", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "close_on_focus_out", NULL)) {
    config->close_on_focus_out = g_key_file_get_boolean(keyfile, "Behavior", "close_on_focus_out", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "show_categories", NULL)) {
    config->show_categories = g_key_file_get_boolean(keyfile, "Behavior", "show_categories", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "show_descriptions", NULL)) {
    config->show_descriptions = g_key_file_get_boolean(keyfile, "Behavior", "show_descriptions", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "show_icons", NULL)) {
    config->show_icons = g_key_file_get_boolean(keyfile, "Behavior", "show_icons", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "show_search", NULL)) {
    config->show_search = g_key_file_get_boolean(keyfile, "Behavior", "show_search", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "show_scrollbar", NULL)) {
    config->show_scrollbar = g_key_file_get_boolean(keyfile, "Behavior", "show_scrollbar", NULL);
    }
    
    if (g_key_file_has_key(keyfile, "Behavior", "show_border", NULL)) {
    config->show_border = g_key_file_get_boolean(keyfile, "Behavior", "show_border", NULL);
  }
  
  if (g_key_file_has_key(keyfile, "Behavior", "show_shadow", NULL)) {
    config->show_shadow = g_key_file_get_boolean(keyfile, "Behavior", "show_shadow", NULL);
  }
  
    if (g_key_file_has_key(keyfile, "Behavior", "blur_background", NULL)) {
      config->blur_background = g_key_file_get_boolean(keyfile, "Behavior", "blur_background", NULL);
  }
  
    if (g_key_file_has_key(keyfile, "Behavior", "blur_strength", NULL)) {
      config->blur_strength = g_key_file_get_integer(keyfile, "Behavior", "blur_strength", NULL);
  }
  
    if (g_key_file_has_key(keyfile, "Behavior", "opacity", NULL)) {
      config->opacity = g_key_file_get_double(keyfile, "Behavior", "opacity", NULL);
    }
  }
  
<<<<<<< HEAD
  // Hyprland-specific settings
  if (g_key_file_has_group(keyfile, "Hyprland")) {
    if (g_key_file_has_key(keyfile, "Hyprland", "use_hyprland_corner_fix", NULL)) {
      config->use_hyprland_corner_fix = g_key_file_get_boolean(keyfile, "Hyprland", "use_hyprland_corner_fix", NULL);
    }
    if (g_key_file_has_key(keyfile, "Hyprland", "hyprland_corner_radius", NULL)) {
      config->hyprland_corner_radius = g_key_file_get_integer(keyfile, "Hyprland", "hyprland_corner_radius", NULL);
    }
  }
  
  // Behavior options
  config->max_recent_apps = g_key_file_get_integer(keyfile, "Behavior", "max_recent_apps", NULL);
  if (config->max_recent_apps <= 0) {
    config->max_recent_apps = 10;  // Default if not set or invalid
  }
  
=======
>>>>>>> parent of a64de2c (Corner artifacts sadly)
  // Save config back if any missing options
  if (missing_option) {
    hyprmenu_config_save();
  }
  
  return TRUE;
}

gboolean
hyprmenu_config_save_with_error(GError **error)
{
  g_autoptr(GKeyFile) keyfile = g_key_file_new();
  static gboolean is_saving = FALSE;
  
  if (is_saving) {
    return TRUE;
  }
  
  is_saving = TRUE;
  
  // Add header comments
  g_key_file_set_comment(keyfile, NULL, NULL, 
    "# HyprMenu Configuration File\n"
    "# This file controls the appearance and behavior of HyprMenu\n"
    "# All color values support #RRGGBB, #RRGGBBAA, rgb(), or rgba() formats", NULL);

  // Layout section
  g_key_file_set_comment(keyfile, "Layout", NULL,
    "# Main window dimensions and positioning", NULL);
  g_key_file_set_integer(keyfile, "Layout", "window_width", config->window_width);
  g_key_file_set_comment(keyfile, "Layout", "window_width", "Width of the menu window in pixels", NULL);
  g_key_file_set_integer(keyfile, "Layout", "window_height", config->window_height);
  g_key_file_set_comment(keyfile, "Layout", "window_height", "Height of the menu window in pixels", NULL);
  g_key_file_set_integer(keyfile, "Layout", "top_margin", config->top_margin);
  g_key_file_set_comment(keyfile, "Layout", "top_margin", "Margin from the top of the screen", NULL);
  g_key_file_set_integer(keyfile, "Layout", "left_margin", config->left_margin);
  g_key_file_set_comment(keyfile, "Layout", "left_margin", "Margin from the left of the screen", NULL);
  g_key_file_set_boolean(keyfile, "Layout", "center_window", config->center_window);
  g_key_file_set_comment(keyfile, "Layout", "center_window", "Whether to center the window on screen", NULL);
  g_key_file_set_integer(keyfile, "Layout", "bottom_offset", config->bottom_offset);
  g_key_file_set_comment(keyfile, "Layout", "bottom_offset", "Offset from bottom for dock/panel (0 to respect reserved space)", NULL);
  g_key_file_set_integer(keyfile, "Layout", "top_offset", config->top_offset);
  g_key_file_set_comment(keyfile, "Layout", "top_offset", "Offset from top for panel", NULL);
  g_key_file_set_integer(keyfile, "Layout", "window_padding", config->window_padding);
  g_key_file_set_comment(keyfile, "Layout", "window_padding", "Internal padding of the main window", NULL);

  // Window section
  g_key_file_set_comment(keyfile, "Window", NULL,
    "# Main window appearance", NULL);
  g_key_file_set_string(keyfile, "Window", "background_color", config->window_background_color);
  g_key_file_set_comment(keyfile, "Window", "background_color", "Window background color (empty for transparent)", NULL);
  g_key_file_set_double(keyfile, "Window", "background_opacity", config->window_background_opacity);
  g_key_file_set_comment(keyfile, "Window", "background_opacity", "Overall window opacity (0.0 to 1.0)", NULL);
  g_key_file_set_double(keyfile, "Window", "background_blur", config->window_background_blur);
  g_key_file_set_comment(keyfile, "Window", "background_blur", "Background blur strength", NULL);
  g_key_file_set_integer(keyfile, "Window", "corner_radius", config->window_corner_radius);
  g_key_file_set_comment(keyfile, "Window", "corner_radius", "Window corner radius", NULL);
  g_key_file_set_string(keyfile, "Window", "halign", config->window_halign);
  g_key_file_set_comment(keyfile, "Window", "halign", "Horizontal alignment (start, center, end)", NULL);
  g_key_file_set_string(keyfile, "Window", "valign", config->window_valign);
  g_key_file_set_comment(keyfile, "Window", "valign", "Vertical alignment (start, center, end)", NULL);
  g_key_file_set_string(keyfile, "Window", "shadow_color", config->window_shadow_color);
  g_key_file_set_comment(keyfile, "Window", "shadow_color", "Window shadow color", NULL);
  g_key_file_set_integer(keyfile, "Window", "shadow_radius", config->window_shadow_radius);
  g_key_file_set_comment(keyfile, "Window", "shadow_radius", "Window shadow radius", NULL);
  g_key_file_set_double(keyfile, "Window", "opacity", config->opacity);
  g_key_file_set_comment(keyfile, "Window", "opacity", "Overall window opacity", NULL);

  // Border section
  g_key_file_set_comment(keyfile, "Border", NULL,
    "# Border appearance settings", NULL);
  g_key_file_set_string(keyfile, "Border", "inner_border_color", config->inner_border_color);
  g_key_file_set_comment(keyfile, "Border", "inner_border_color", "Color of the inner border", NULL);
  g_key_file_set_integer(keyfile, "Border", "inner_border_radius", config->inner_border_radius);
  g_key_file_set_comment(keyfile, "Border", "inner_border_radius", "Radius of inner border corners", NULL);
  g_key_file_set_integer(keyfile, "Border", "inner_border_width", config->inner_border_width);
  g_key_file_set_comment(keyfile, "Border", "inner_border_width", "Width of inner border", NULL);
  g_key_file_set_string(keyfile, "Border", "outer_border_color", config->outer_border_color);
  g_key_file_set_comment(keyfile, "Border", "outer_border_color", "Color of the outer border", NULL);
  g_key_file_set_integer(keyfile, "Border", "outer_border_radius", config->outer_border_radius);
  g_key_file_set_comment(keyfile, "Border", "outer_border_radius", "Radius of outer border corners", NULL);
  g_key_file_set_integer(keyfile, "Border", "outer_border_width", config->outer_border_width);
  g_key_file_set_comment(keyfile, "Border", "outer_border_width", "Width of outer border", NULL);

  // Grid section
  g_key_file_set_comment(keyfile, "Grid", NULL,
    "# Grid view settings (when showing apps in grid mode)", NULL);
  g_key_file_set_integer(keyfile, "Grid", "columns", config->grid_columns);
  g_key_file_set_comment(keyfile, "Grid", "columns", "Number of columns in grid", NULL);
  g_key_file_set_integer(keyfile, "Grid", "item_size", config->grid_item_size);
  g_key_file_set_comment(keyfile, "Grid", "item_size", "Size of each grid item", NULL);
  g_key_file_set_integer(keyfile, "Grid", "item_corner_radius", config->grid_item_corner_radius);
  g_key_file_set_comment(keyfile, "Grid", "item_corner_radius", "Corner radius of grid items", NULL);
  g_key_file_set_integer(keyfile, "Grid", "item_border_width", config->grid_item_border_width);
  g_key_file_set_comment(keyfile, "Grid", "item_border_width", "Border width of grid items", NULL);
  g_key_file_set_string(keyfile, "Grid", "item_border_color", config->grid_item_border_color);
  g_key_file_set_comment(keyfile, "Grid", "item_border_color", "Border color of grid items", NULL);
  g_key_file_set_string(keyfile, "Grid", "item_background_color", config->grid_item_background_color);
  g_key_file_set_comment(keyfile, "Grid", "item_background_color", "Background color of grid items", NULL);
  g_key_file_set_integer(keyfile, "Grid", "row_spacing", config->grid_row_spacing);
  g_key_file_set_comment(keyfile, "Grid", "row_spacing", "Vertical spacing between rows", NULL);
  g_key_file_set_integer(keyfile, "Grid", "column_spacing", config->grid_column_spacing);
  g_key_file_set_comment(keyfile, "Grid", "column_spacing", "Horizontal spacing between columns", NULL);
  g_key_file_set_integer(keyfile, "Grid", "margin_start", config->grid_margin_start);
  g_key_file_set_comment(keyfile, "Grid", "margin_start", "Left margin", NULL);
  g_key_file_set_integer(keyfile, "Grid", "margin_end", config->grid_margin_end);
  g_key_file_set_comment(keyfile, "Grid", "margin_end", "Right margin", NULL);
  g_key_file_set_integer(keyfile, "Grid", "margin_top", config->grid_margin_top);
  g_key_file_set_comment(keyfile, "Grid", "margin_top", "Top margin", NULL);
  g_key_file_set_integer(keyfile, "Grid", "margin_bottom", config->grid_margin_bottom);
  g_key_file_set_comment(keyfile, "Grid", "margin_bottom", "Bottom margin", NULL);
  g_key_file_set_string(keyfile, "Grid", "halign", config->grid_halign);
  g_key_file_set_comment(keyfile, "Grid", "halign", "Horizontal alignment of grid", NULL);
  g_key_file_set_string(keyfile, "Grid", "valign", config->grid_valign);
  g_key_file_set_comment(keyfile, "Grid", "valign", "Vertical alignment of grid", NULL);
  g_key_file_set_boolean(keyfile, "Grid", "hexpand", config->grid_hexpand);
  g_key_file_set_comment(keyfile, "Grid", "hexpand", "Whether grid expands horizontally", NULL);
  g_key_file_set_boolean(keyfile, "Grid", "vexpand", config->grid_vexpand);
  g_key_file_set_comment(keyfile, "Grid", "vexpand", "Whether grid expands vertically", NULL);
  g_key_file_set_double(keyfile, "Grid", "opacity", config->grid_opacity);
  g_key_file_set_comment(keyfile, "Grid", "opacity", "Overall grid opacity", NULL);
  g_key_file_set_double(keyfile, "Grid", "item_opacity", config->grid_item_opacity);
  g_key_file_set_comment(keyfile, "Grid", "item_opacity", "Individual item opacity", NULL);

  // List section
  g_key_file_set_comment(keyfile, "List", NULL,
    "# List view settings (when showing apps in list mode)", NULL);
  g_key_file_set_integer(keyfile, "List", "item_size", config->list_item_size);
  g_key_file_set_comment(keyfile, "List", "item_size", "Height of each list item", NULL);
  g_key_file_set_integer(keyfile, "List", "item_corner_radius", config->list_item_corner_radius);
  g_key_file_set_comment(keyfile, "List", "item_corner_radius", "Corner radius of list items", NULL);
  g_key_file_set_integer(keyfile, "List", "item_border_width", config->list_item_border_width);
  g_key_file_set_comment(keyfile, "List", "item_border_width", "Border width of list items", NULL);
  g_key_file_set_string(keyfile, "List", "item_border_color", config->list_item_border_color);
  g_key_file_set_comment(keyfile, "List", "item_border_color", "Border color of list items", NULL);
  g_key_file_set_string(keyfile, "List", "item_background_color", config->list_item_background_color);
  g_key_file_set_comment(keyfile, "List", "item_background_color", "Background color of list items", NULL);
  g_key_file_set_integer(keyfile, "List", "row_spacing", config->list_row_spacing);
  g_key_file_set_comment(keyfile, "List", "row_spacing", "Vertical spacing between items", NULL);
  g_key_file_set_integer(keyfile, "List", "margin_start", config->grid_margin_start);
  g_key_file_set_comment(keyfile, "List", "margin_start", "Left margin", NULL);
  g_key_file_set_integer(keyfile, "List", "margin_end", config->grid_margin_end);
  g_key_file_set_comment(keyfile, "List", "margin_end", "Right margin", NULL);
  g_key_file_set_integer(keyfile, "List", "margin_top", config->grid_margin_top);
  g_key_file_set_comment(keyfile, "List", "margin_top", "Top margin", NULL);
  g_key_file_set_integer(keyfile, "List", "margin_bottom", config->grid_margin_bottom);
  g_key_file_set_comment(keyfile, "List", "margin_bottom", "Bottom margin", NULL);
  g_key_file_set_string(keyfile, "List", "halign", config->list_halign);
  g_key_file_set_comment(keyfile, "List", "halign", "Horizontal alignment of list", NULL);
  g_key_file_set_string(keyfile, "List", "valign", config->list_valign);
  g_key_file_set_comment(keyfile, "List", "valign", "Vertical alignment of list", NULL);
  g_key_file_set_boolean(keyfile, "List", "hexpand", config->list_hexpand);
  g_key_file_set_comment(keyfile, "List", "hexpand", "Whether list expands horizontally", NULL);
  g_key_file_set_boolean(keyfile, "List", "vexpand", config->list_vexpand);
  g_key_file_set_comment(keyfile, "List", "vexpand", "Whether list expands vertically", NULL);
  g_key_file_set_double(keyfile, "List", "opacity", config->grid_opacity);
  g_key_file_set_comment(keyfile, "List", "opacity", "Overall list opacity", NULL);
  g_key_file_set_double(keyfile, "List", "item_opacity", config->grid_item_opacity);
  g_key_file_set_comment(keyfile, "List", "item_opacity", "Individual item opacity", NULL);

  // AppEntry section
  g_key_file_set_comment(keyfile, "AppEntry", NULL,
    "# Individual application entry appearance", NULL);
  g_key_file_set_integer(keyfile, "AppEntry", "icon_size", config->app_icon_size);
  g_key_file_set_comment(keyfile, "AppEntry", "icon_size", "Size of application icons", NULL);
  g_key_file_set_integer(keyfile, "AppEntry", "icon_corner_radius", config->app_icon_corner_radius);
  g_key_file_set_comment(keyfile, "AppEntry", "icon_corner_radius", "Corner radius of icons", NULL);
  g_key_file_set_string(keyfile, "AppEntry", "icon_background_color", config->app_icon_background_color);
  g_key_file_set_comment(keyfile, "AppEntry", "icon_background_color", "Background color behind icons", NULL);
  g_key_file_set_integer(keyfile, "AppEntry", "name_font_size", config->app_name_font_size);
  g_key_file_set_comment(keyfile, "AppEntry", "name_font_size", "Font size of application names", NULL);
  g_key_file_set_string(keyfile, "AppEntry", "name_color", config->app_name_color);
  g_key_file_set_comment(keyfile, "AppEntry", "name_color", "Color of application names", NULL);
  g_key_file_set_integer(keyfile, "AppEntry", "desc_font_size", config->app_desc_font_size);
  g_key_file_set_comment(keyfile, "AppEntry", "desc_font_size", "Font size of application descriptions", NULL);
  g_key_file_set_string(keyfile, "AppEntry", "desc_color", config->app_desc_color);
  g_key_file_set_comment(keyfile, "AppEntry", "desc_color", "Color of application descriptions", NULL);
  g_key_file_set_integer(keyfile, "AppEntry", "padding", config->app_entry_padding);
  g_key_file_set_comment(keyfile, "AppEntry", "padding", "Internal padding of entries", NULL);
  g_key_file_set_string(keyfile, "AppEntry", "hover_color", config->app_entry_hover_color);
  g_key_file_set_comment(keyfile, "AppEntry", "hover_color", "Background color on hover", NULL);
  g_key_file_set_string(keyfile, "AppEntry", "active_color", config->app_entry_active_color);
  g_key_file_set_comment(keyfile, "AppEntry", "active_color", "Background color when clicked", NULL);
  g_key_file_set_double(keyfile, "AppEntry", "opacity", config->app_entry_opacity);
  g_key_file_set_comment(keyfile, "AppEntry", "opacity", "Overall entry opacity", NULL);
  g_key_file_set_double(keyfile, "AppEntry", "icon_opacity", config->app_icon_opacity);
  g_key_file_set_comment(keyfile, "AppEntry", "icon_opacity", "Icon opacity", NULL);
  g_key_file_set_double(keyfile, "AppEntry", "name_opacity", config->app_name_opacity);
  g_key_file_set_comment(keyfile, "AppEntry", "name_opacity", "Name text opacity", NULL);
  g_key_file_set_double(keyfile, "AppEntry", "desc_opacity", config->app_desc_opacity);
  g_key_file_set_comment(keyfile, "AppEntry", "desc_opacity", "Description text opacity", NULL);

  // Category section
  g_key_file_set_comment(keyfile, "Category", NULL,
    "# Category header appearance", NULL);
  g_key_file_set_string(keyfile, "Category", "background_color", config->category_background_color);
  g_key_file_set_comment(keyfile, "Category", "background_color", "Category background color", NULL);
  g_key_file_set_double(keyfile, "Category", "background_opacity", config->category_background_opacity);
  g_key_file_set_comment(keyfile, "Category", "background_opacity", "Category background opacity", NULL);
  g_key_file_set_integer(keyfile, "Category", "corner_radius", config->category_corner_radius);
  g_key_file_set_comment(keyfile, "Category", "corner_radius", "Corner radius of category headers", NULL);
  g_key_file_set_string(keyfile, "Category", "text_color", config->category_text_color);
  g_key_file_set_comment(keyfile, "Category", "text_color", "Category text color", NULL);
  g_key_file_set_integer(keyfile, "Category", "font_size", config->category_font_size);
  g_key_file_set_comment(keyfile, "Category", "font_size", "Category text size", NULL);
  g_key_file_set_string(keyfile, "Category", "font_family", config->category_font_family);
  g_key_file_set_comment(keyfile, "Category", "font_family", "Category font family", NULL);
  g_key_file_set_integer(keyfile, "Category", "padding", config->category_padding);
  g_key_file_set_comment(keyfile, "Category", "padding", "Internal padding of categories", NULL);
  g_key_file_set_boolean(keyfile, "Category", "show_separators", config->show_categories);
  g_key_file_set_comment(keyfile, "Category", "show_separators", "Whether to show separators between categories", NULL);
  g_key_file_set_string(keyfile, "Category", "separator_color", config->category_separator_color);
  g_key_file_set_comment(keyfile, "Category", "separator_color", "Color of category separators", NULL);
  g_key_file_set_double(keyfile, "Category", "opacity", config->category_opacity);
  g_key_file_set_comment(keyfile, "Category", "opacity", "Overall category opacity", NULL);
  g_key_file_set_double(keyfile, "Category", "title_opacity", config->category_title_opacity);
  g_key_file_set_comment(keyfile, "Category", "title_opacity", "Category title opacity", NULL);

  // Search section
  g_key_file_set_comment(keyfile, "Search", NULL,
    "# Search bar appearance and behavior", NULL);
  g_key_file_set_string(keyfile, "Search", "background_color", config->search_background_color);
  g_key_file_set_comment(keyfile, "Search", "background_color", "Search bar background color", NULL);
  g_key_file_set_double(keyfile, "Search", "background_opacity", config->search_background_opacity);
  g_key_file_set_comment(keyfile, "Search", "background_opacity", "Search bar background opacity", NULL);
  g_key_file_set_integer(keyfile, "Search", "corner_radius", config->search_corner_radius);
  g_key_file_set_comment(keyfile, "Search", "corner_radius", "Corner radius of search bar", NULL);
  g_key_file_set_string(keyfile, "Search", "text_color", config->search_text_color);
  g_key_file_set_comment(keyfile, "Search", "text_color", "Search text color", NULL);
  g_key_file_set_integer(keyfile, "Search", "font_size", config->search_font_size);
  g_key_file_set_comment(keyfile, "Search", "font_size", "Search text size", NULL);
  g_key_file_set_string(keyfile, "Search", "font_family", config->search_font_family);
  g_key_file_set_comment(keyfile, "Search", "font_family", "Search text font", NULL);
  g_key_file_set_integer(keyfile, "Search", "padding", config->search_padding);
  g_key_file_set_comment(keyfile, "Search", "padding", "Internal padding of search bar", NULL);
  g_key_file_set_integer(keyfile, "Search", "min_height", config->search_min_height);
  g_key_file_set_comment(keyfile, "Search", "min_height", "Minimum height of search bar", NULL);
  g_key_file_set_integer(keyfile, "Search", "left_padding", config->search_left_padding);
  g_key_file_set_comment(keyfile, "Search", "left_padding", "Left padding of search text", NULL);
  g_key_file_set_integer(keyfile, "Search", "length", config->search_length);
  g_key_file_set_comment(keyfile, "Search", "length", "Maximum search text length (0 for unlimited)", NULL);
  g_key_file_set_string(keyfile, "Search", "placeholder_text", config->search_placeholder_text);
  g_key_file_set_comment(keyfile, "Search", "placeholder_text", "Placeholder text when empty", NULL);
  g_key_file_set_integer(keyfile, "Search", "icon_size", config->search_icon_size);
  g_key_file_set_comment(keyfile, "Search", "icon_size", "Size of search icon", NULL);
  g_key_file_set_string(keyfile, "Search", "icon_color", config->search_icon_color);
  g_key_file_set_comment(keyfile, "Search", "icon_color", "Color of search icon", NULL);
  g_key_file_set_string(keyfile, "Search", "focus_border_color", config->search_focus_border_color);
  g_key_file_set_comment(keyfile, "Search", "focus_border_color", "Border color when focused", NULL);
  g_key_file_set_string(keyfile, "Search", "focus_shadow_color", config->search_focus_shadow_color);
  g_key_file_set_comment(keyfile, "Search", "focus_shadow_color", "Shadow color when focused", NULL);
  g_key_file_set_double(keyfile, "Search", "opacity", config->search_opacity);
  g_key_file_set_comment(keyfile, "Search", "opacity", "Overall search bar opacity", NULL);
  g_key_file_set_double(keyfile, "Search", "text_opacity", config->search_text_opacity);
  g_key_file_set_comment(keyfile, "Search", "text_opacity", "Search text opacity", NULL);
  g_key_file_set_double(keyfile, "Search", "icon_opacity", config->search_icon_opacity);
  g_key_file_set_comment(keyfile, "Search", "icon_opacity", "Search icon opacity", NULL);

  // SystemButton section
  g_key_file_set_comment(keyfile, "SystemButton", NULL,
    "# System button appearance (power, settings, etc.)", NULL);
  g_key_file_set_string(keyfile, "SystemButton", "background_color", config->system_button_background_color);
  g_key_file_set_comment(keyfile, "SystemButton", "background_color", "Button background color", NULL);
  g_key_file_set_string(keyfile, "SystemButton", "icon_color", config->system_button_icon_color);
  g_key_file_set_comment(keyfile, "SystemButton", "icon_color", "Button icon color", NULL);
  g_key_file_set_string(keyfile, "SystemButton", "hover_color", config->system_button_hover_color);
  g_key_file_set_comment(keyfile, "SystemButton", "hover_color", "Background color on hover", NULL);
  g_key_file_set_string(keyfile, "SystemButton", "active_color", config->system_button_active_color);
  g_key_file_set_comment(keyfile, "SystemButton", "active_color", "Background color when clicked", NULL);
  g_key_file_set_integer(keyfile, "SystemButton", "corner_radius", config->system_button_corner_radius);
  g_key_file_set_comment(keyfile, "SystemButton", "corner_radius", "Corner radius of buttons", NULL);
  g_key_file_set_integer(keyfile, "SystemButton", "size", config->system_button_size);
  g_key_file_set_comment(keyfile, "SystemButton", "size", "Size of buttons", NULL);
  g_key_file_set_integer(keyfile, "SystemButton", "spacing", config->system_button_spacing);
  g_key_file_set_comment(keyfile, "SystemButton", "spacing", "Space between buttons", NULL);
  g_key_file_set_double(keyfile, "SystemButton", "opacity", config->system_button_opacity);
  g_key_file_set_comment(keyfile, "SystemButton", "opacity", "Overall button opacity", NULL);
  g_key_file_set_double(keyfile, "SystemButton", "icon_opacity", config->system_button_icon_opacity);
  g_key_file_set_comment(keyfile, "SystemButton", "icon_opacity", "Button icon opacity", NULL);

  // Behavior section
  g_key_file_set_comment(keyfile, "Behavior", NULL,
    "# Program behavior settings", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_click_outside", config->close_on_click_outside);
  g_key_file_set_comment(keyfile, "Behavior", "close_on_click_outside", "Close when clicking outside the menu", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_super_key", config->close_on_super_key);
  g_key_file_set_comment(keyfile, "Behavior", "close_on_super_key", "Close when pressing Super key", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_app_launch", config->close_on_app_launch);
  g_key_file_set_comment(keyfile, "Behavior", "close_on_app_launch", "Close when launching an application", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "focus_search_on_open", config->focus_search_on_open);
  g_key_file_set_comment(keyfile, "Behavior", "focus_search_on_open", "Focus search bar when opening", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_escape", config->close_on_escape);
  g_key_file_set_comment(keyfile, "Behavior", "close_on_escape", "Close when pressing Escape", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_focus_out", config->close_on_focus_out);
  g_key_file_set_comment(keyfile, "Behavior", "close_on_focus_out", "Close when losing focus", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "show_categories", config->show_categories);
  g_key_file_set_comment(keyfile, "Behavior", "show_categories", "Show application categories", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "show_descriptions", config->show_descriptions);
  g_key_file_set_comment(keyfile, "Behavior", "show_descriptions", "Show application descriptions", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "show_icons", config->show_icons);
  g_key_file_set_comment(keyfile, "Behavior", "show_icons", "Show application icons", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "show_search", config->show_search);
  g_key_file_set_comment(keyfile, "Behavior", "show_search", "Show search bar", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "show_scrollbar", config->show_scrollbar);
  g_key_file_set_comment(keyfile, "Behavior", "show_scrollbar", "Show scrollbar", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "show_border", config->show_border);
  g_key_file_set_comment(keyfile, "Behavior", "show_border", "Show window border", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "show_shadow", config->show_shadow);
  g_key_file_set_comment(keyfile, "Behavior", "show_shadow", "Show window shadow", NULL);
  g_key_file_set_boolean(keyfile, "Behavior", "blur_background", config->blur_background);
  g_key_file_set_comment(keyfile, "Behavior", "blur_background", "Enable background blur", NULL);
  g_key_file_set_integer(keyfile, "Behavior", "blur_strength", config->blur_strength);
<<<<<<< HEAD
  g_key_file_set_comment(keyfile, "Behavior", "blur_strength", "Background blur strength", NULL);
  g_key_file_set_integer(keyfile, "Behavior", "max_recent_apps", config->max_recent_apps);
  g_key_file_set_comment(keyfile, "Behavior", "max_recent_apps", "Maximum number of recent apps to show", NULL);

  // Transparency section
  g_key_file_set_comment(keyfile, "Transparency", NULL,
    "# Global transparency settings", NULL);
  g_key_file_set_boolean(keyfile, "Transparency", "enabled", config->blur_background);
  g_key_file_set_comment(keyfile, "Transparency", "enabled", "Enable transparency effects", NULL);
  g_key_file_set_double(keyfile, "Transparency", "alpha", config->opacity);
  g_key_file_set_comment(keyfile, "Transparency", "alpha", "Global alpha value (0.0 to 1.0)", NULL);
  g_key_file_set_boolean(keyfile, "Transparency", "blur", config->blur_background);
  g_key_file_set_comment(keyfile, "Transparency", "blur", "Enable blur effects", NULL);
  g_key_file_set_boolean(keyfile, "Transparency", "shadow", config->show_shadow);
  g_key_file_set_comment(keyfile, "Transparency", "shadow", "Enable shadow effects", NULL);
  g_key_file_set_string(keyfile, "Transparency", "shadow_color", config->window_shadow_color);
  g_key_file_set_comment(keyfile, "Transparency", "shadow_color", "Shadow color", NULL);
  g_key_file_set_integer(keyfile, "Transparency", "shadow_radius", config->window_shadow_radius);
  g_key_file_set_comment(keyfile, "Transparency", "shadow_radius", "Shadow radius", NULL);

=======
  g_key_file_set_double(keyfile, "Behavior", "opacity", config->opacity);
  
>>>>>>> parent of a64de2c (Corner artifacts sadly)
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
  if (!config) return;
  GString *css = g_string_new("");

  // Create CSS string
  g_string_append_printf(css,
    "window, .background {\n"
    "  border-radius: %dpx;\n"
    "  overflow: hidden;\n"
    "}\n\n"
    ".hyprmenu-window {\n"
    "  background-color: %s%s;\n"
    "  border-radius: %dpx;\n"
    "  border: %dpx solid %s;\n"
    "  padding: %dpx;\n"
<<<<<<< HEAD
    "  opacity: %.2f;\n"
=======
    "  overflow: hidden;\n"
>>>>>>> parent of a64de2c (Corner artifacts sadly)
    "}\n\n",
    config->outer_border_radius,
    // Handle empty background color with transparency
    (config->window_background_color && strlen(config->window_background_color) > 0) ? 
      config->window_background_color : 
      "rgba(20, 20, 20, ",
    // Add opacity if using the default color
    (config->window_background_color && strlen(config->window_background_color) > 0) ? 
      "" : 
      g_strdup_printf("%.2f)", config->window_background_opacity),
    config->outer_border_radius,
    config->outer_border_width,
    config->outer_border_color,
    config->window_padding,
    config->opacity);

<<<<<<< HEAD
  // Content box style (wrapper for the entire content)
  g_string_append_printf(css,
    ".hyprmenu-content-box {\n"
    "  background-color: transparent;\n"
    "  border-radius: %dpx;\n"
    "  opacity: %.2f;\n"
    "}\n\n",
    config->outer_border_radius - 2,
    config->opacity);

=======
>>>>>>> parent of a64de2c (Corner artifacts sadly)
  // Main box styles
  g_string_append_printf(css,
    ".hyprmenu-main-box {\n"
    "  background-color: transparent;\n"
    "  border-radius: %dpx;\n"
    "  padding: %dpx;\n"
<<<<<<< HEAD
    "  opacity: %.2f;\n"
    "}\n\n",
    config->outer_border_radius,
    config->outer_border_radius,
    config->outer_border_radius,
    config->outer_border_radius,
    config->outer_border_radius,
    config->window_padding,
    config->opacity);
=======
    "  overflow: hidden;\n"
    "}\n\n",
    config->inner_border_radius,
    config->window_padding);
>>>>>>> parent of a64de2c (Corner artifacts sadly)

  // Search styles
  g_string_append_printf(css,
    ".hyprmenu-search {\n"
    "  background-color: %s;\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "  font-family: %s;\n"
    "  border-radius: %dpx;\n"
    "  border-width: %dpx;\n"
    "  border-color: %s;\n"
    "  border-style: solid;\n"
    "  padding: %dpx;\n"
    "  min-height: %dpx;\n"
    "  padding-left: %dpx;\n"
    "  box-shadow: none;\n"
<<<<<<< HEAD
    "  margin: 4px 4px 8px 4px;\n"
    "  opacity: %.2f;\n"
=======
    "  margin-start: 4px;\n"
    "  margin-end: 4px;\n"
    "  margin-top: 4px;\n"
    "  margin-bottom: 8px;\n"
>>>>>>> parent of a64de2c (Corner artifacts sadly)
    "}\n\n"
    ".hyprmenu-search:focus {\n"
    "  border-color: rgba(255,255,255,0.4);\n"
    "  box-shadow: 0 0 3px %s;\n"
    "}\n\n"
    ".hyprmenu-search text {\n"
    "  opacity: %.2f;\n"
    "}\n\n"
    ".hyprmenu-search-icon {\n"
    "  opacity: %.2f;\n"
    "}\n\n",
    config->search_background_color,
    config->search_text_color,
    config->search_font_size,
    config->search_font_family,
    config->search_corner_radius,
    config->inner_border_width,
    config->inner_border_color,
    config->search_padding,
    config->search_min_height,
    config->search_left_padding,
    config->search_opacity,
    config->search_focus_shadow_color,
    config->search_text_opacity,
    config->search_icon_opacity);

  // Content container - new element to hold all content with border
  g_string_append_printf(css,
    ".hyprmenu-content-container {\n"
    "  background-color: transparent;\n"
    "  border-radius: %dpx;\n"
    "  border-width: %dpx;\n"
    "  border-color: %s;\n"
    "  border-style: solid;\n"
    "  padding: 8px;\n"
<<<<<<< HEAD
    "  opacity: %.2f;\n"
=======
    "  overflow: hidden;\n"
>>>>>>> parent of a64de2c (Corner artifacts sadly)
    "}\n\n",
    config->inner_border_radius,
    config->inner_border_width,
    config->inner_border_color,
    config->opacity);
  
  // App grid styles
  g_string_append_printf(css,
    ".hyprmenu-app-grid {\n"
<<<<<<< HEAD
    "  margin: %dpx %dpx %dpx %dpx;\n"
    "  opacity: %.2f;\n"
=======
    "  margin-top: %dpx;\n"
    "  margin-bottom: %dpx;\n"
    "  margin-start: %dpx;\n"
    "  margin-end: %dpx;\n"
>>>>>>> parent of a64de2c (Corner artifacts sadly)
    "}\n\n",
    config->grid_margin_top,
    config->grid_margin_bottom,
    config->grid_margin_start,
<<<<<<< HEAD
    config->grid_opacity);
=======
    config->grid_margin_end);
>>>>>>> parent of a64de2c (Corner artifacts sadly)

  // App entry styles
  g_string_append_printf(css,
    ".hyprmenu-app-entry {\n"
    "  background-color: transparent;\n"
    "  border-radius: %dpx;\n"
    "  padding: %dpx;\n"
    "  border: none;\n"
    "  box-shadow: none;\n"
    "  opacity: %.2f;\n"
    "}\n\n"
    ".hyprmenu-app-entry:hover {\n"
    "  background-color: %s;\n"
    "}\n\n"
    ".hyprmenu-app-entry:active {\n"
    "  background-color: %s;\n"
    "}\n\n"
    ".hyprmenu-app-icon {\n"
    "  opacity: %.2f;\n"
    "}\n\n"
    ".hyprmenu-app-name {\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "  font-weight: bold;\n"
    "  opacity: %.2f;\n"
    "}\n\n"
    ".hyprmenu-app-description {\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "  opacity: %.2f;\n"
    "}\n\n",
    config->grid_item_corner_radius,
    config->app_entry_padding,
    config->app_entry_opacity,
    config->app_entry_hover_color,
    config->app_entry_active_color,
    config->app_icon_opacity,
    config->app_name_color,
    config->app_name_font_size,
    config->app_name_opacity,
    config->app_desc_color,
    config->app_desc_font_size,
    config->app_desc_opacity);

  // Category styles
  g_string_append_printf(css,
    ".hyprmenu-category {\n"
    "  background-color: %s;\n"
    "  border-radius: %dpx;\n"
    "  padding: %dpx;\n"
    "  opacity: %.2f;\n"
    "}\n\n"
    ".hyprmenu-category-title {\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "  font-family: %s;\n"
    "  padding: %dpx;\n"
    "  opacity: %.2f;\n"
    "}\n\n",
    config->category_background_color,
    config->category_corner_radius,
    config->category_padding,
    config->category_opacity,
    config->category_text_color,
    config->category_font_size,
    config->category_font_family,
    config->category_padding,
    config->category_title_opacity);

  // System button styles (flat, icon-only)
  g_string_append_printf(css,
    ".system-button {\n"
    "  background: transparent;\n"
    "  border: none;\n"
    "  box-shadow: none;\n"
    "  border-radius: 0;\n"
    "  padding: 0 8px;\n"
    "  min-width: 0;\n"
    "  min-height: 0;\n"
    "  color: %s;\n"
    "  opacity: %.2f;\n"
    "}\n\n"
    ".system-button:hover {\n"
    "  background: rgba(255,255,255,0.08);\n"
    "}\n\n"
    ".system-button-icon {\n"
    "  opacity: %.2f;\n"
    "}\n\n",
    config->system_button_icon_color ? config->system_button_icon_color : "#fff",
    config->system_button_opacity,
    config->system_button_icon_opacity);

  // Add separator style
  g_string_append(css,
    ".hyprmenu-separator {\n"
    "  background-color: rgba(255,255,255,0.15);\n"
    "  min-height: 1px;\n"
    "}\n\n");

  // Apply CSS
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(provider, css->str, css->len);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                            GTK_STYLE_PROVIDER(provider),
                                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
  g_string_free(css, TRUE);
} 