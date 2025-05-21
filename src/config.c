#include "config.h"
#include "hyprland.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Global config instance
HyprMenuConfig *config = NULL;

// Global pywal color instance
PywalColors pywal_colors = {0};

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
  config->inner_border_color = g_strdup("");
  config->window_shadow_color = g_strdup("");
  config->window_shadow_radius = 20;
  config->window_halign = g_strdup("");
  config->window_valign = g_strdup("");
  
  // Grid options
  config->grid_margin_start = 12;
  config->grid_margin_end = 12;
  config->grid_margin_top = 12;
  config->grid_margin_bottom = 12;
  config->grid_row_spacing = 12;
  config->grid_column_spacing = 12;
  config->grid_halign = g_strdup("");
  config->grid_valign = g_strdup("");
  config->grid_hexpand = TRUE;
  config->grid_vexpand = FALSE;
  config->grid_columns = 5;
  config->grid_item_size = 100;
  config->grid_item_corner_radius = 8;
  config->grid_item_border_width = 1;
  config->grid_item_border_color = g_strdup("");
  config->grid_item_background_color = g_strdup("");
  config->grid_opacity = 1.0;
  config->grid_item_opacity = 1.0;
  
  // List options
  config->list_item_size = 48;
  config->list_item_corner_radius = 6;
  config->list_item_border_width = 1;
  config->list_item_border_color = g_strdup("");
  config->list_item_background_color = g_strdup("");
  config->list_row_spacing = 8;
  config->list_halign = g_strdup("");
  config->list_valign = g_strdup("");
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
  config->app_icon_background_color = g_strdup("");
  config->app_name_font_size = 12;
  config->app_name_color = g_strdup("");
  config->app_desc_font_size = 10;
  config->app_desc_color = g_strdup("");
  config->app_entry_padding = 6;
  config->app_entry_hover_color = g_strdup("");
  config->app_entry_active_color = g_strdup("");
  config->app_entry_opacity = 1.0;
  config->app_icon_opacity = 1.0;
  config->app_name_opacity = 1.0;
  config->app_desc_opacity = 1.0;
  
  // Category options
  config->category_background_color = g_strdup("");
  config->category_background_opacity = 1.0;
  config->category_corner_radius = 10;
  config->category_text_color = g_strdup("");
  config->category_font_size = 13;
  config->category_font_family = g_strdup("");
  config->category_padding = 6;
  config->category_show_separators = TRUE;
  config->category_separator_color = g_strdup("");
  config->category_opacity = 1.0;
  config->category_title_opacity = 1.0;
  
  // Search options
  config->search_background_color = g_strdup("");
  config->search_background_opacity = 1.0;
  config->search_corner_radius = 8;
  config->search_text_color = g_strdup("");
  config->search_font_size = 14;
  config->search_font_family = g_strdup("");
  config->search_padding = 8;
  config->search_min_height = 20;
  config->search_left_padding = 2;
  config->search_length = 0;
  config->search_placeholder_text = g_strdup("");
  config->search_icon_size = 16;
  config->search_icon_color = g_strdup("");
  config->search_focus_border_color = g_strdup("");
  config->search_focus_shadow_color = g_strdup("");
  config->search_opacity = 1.0;
  config->search_text_opacity = 1.0;
  config->search_icon_opacity = 1.0;
  
  // SystemButton options
  config->system_button_background_color = g_strdup("");
  config->system_button_icon_color = g_strdup("");
  config->system_button_hover_color = g_strdup("");
  config->system_button_active_color = g_strdup("");
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
  config->max_recent_apps = 10;
  
  // File paths
  config->config_dir = g_build_filename(g_get_home_dir(), ".config", "hyprmenu", NULL);
  config->config_file = g_build_filename(config->config_dir, "hyprmenu.conf", NULL);
  config->css_file = g_build_filename(config->config_dir, "hyprmenu.css", NULL);

  // Outer border (window)
  config->outer_border_width = 3;
  config->outer_border_color = g_strdup("");
  config->outer_border_radius = 16;
  
  // Hyprland-specific settings
  config->use_hyprland_corner_fix = TRUE;
  config->hyprland_corner_radius = 16;

  // Font options
  config->app_name_font_family = g_strdup("");
  config->app_desc_font_family = g_strdup("");
  config->grid_item_font_family = g_strdup("");
  config->list_item_font_family = g_strdup("");
  config->app_name_font_weight = 400;
  config->app_desc_font_weight = 400;
  config->grid_item_font_weight = 400;
  config->list_item_font_weight = 400;
  config->app_name_font_style = g_strdup("normal");
  config->app_desc_font_style = g_strdup("normal");
  config->grid_item_font_style = g_strdup("normal");
  config->list_item_font_style = g_strdup("normal");

  // Grid hover/active
  config->grid_item_hover_color = g_strdup("");
  config->grid_item_active_color = g_strdup("");
  config->grid_item_padding = 6;

  // List hover/active
  config->list_item_hover_color = g_strdup("");
  config->list_item_active_color = g_strdup("");
  config->list_item_padding = 6;

  // Category hover/active
  config->category_hover_color = g_strdup("");
  config->category_active_color = g_strdup("");
  config->category_separator_thickness = 1;

  // Scrollbar
  config->scrollbar_color = g_strdup("");
  config->scrollbar_hover_color = g_strdup("");
  config->scrollbar_active_color = g_strdup("");
  config->scrollbar_width = 8;
  config->scrollbar_corner_radius = 4;
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
    hyprmenu_config_load(config);
  } else {
    // Save default configuration
    hyprmenu_config_save(config);
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

void
hyprmenu_config_load(HyprMenuConfig *config)
{
  if (!config) return;
  
  GKeyFile *keyfile = g_key_file_new();
  GError *error = NULL;
  
  if (!g_key_file_load_from_file(keyfile, config->config_file, G_KEY_FILE_NONE, &error)) {
    g_warning("Failed to load config file: %s", error->message);
    g_error_free(error);
    g_key_file_free(keyfile);
    return;
  }
  
  // Window layout
  config->window_width = g_key_file_get_integer(keyfile, "Window", "width", NULL);
  config->window_height = g_key_file_get_integer(keyfile, "Window", "height", NULL);
  config->top_margin = g_key_file_get_integer(keyfile, "Window", "top_margin", NULL);
  config->left_margin = g_key_file_get_integer(keyfile, "Window", "left_margin", NULL);
  config->center_window = g_key_file_get_boolean(keyfile, "Window", "center_window", NULL);
  gchar *menu_position_str = g_key_file_get_string(keyfile, "Window", "menu_position", NULL);
  config->menu_position = hyprmenu_position_from_string(menu_position_str);
  g_free(menu_position_str);
  config->bottom_offset = g_key_file_get_integer(keyfile, "Window", "bottom_offset", NULL);
  config->top_offset = g_key_file_get_integer(keyfile, "Window", "top_offset", NULL);
  config->window_padding = g_key_file_get_integer(keyfile, "Window", "padding", NULL);
  
  // Window style
  config->window_background_opacity = g_key_file_get_double(keyfile, "Window", "background_opacity", NULL);
  config->window_background_blur = g_key_file_get_double(keyfile, "Window", "background_blur", NULL);
  config->window_background_color = g_key_file_get_string(keyfile, "Window", "background_color", NULL);
  config->inner_border_radius = g_key_file_get_integer(keyfile, "Window", "inner_border_radius", NULL);
  config->inner_border_width = g_key_file_get_integer(keyfile, "Window", "inner_border_width", NULL);
  config->inner_border_color = g_key_file_get_string(keyfile, "Window", "inner_border_color", NULL);
  config->window_shadow_color = g_key_file_get_string(keyfile, "Window", "shadow_color", NULL);
  config->window_shadow_radius = g_key_file_get_integer(keyfile, "Window", "shadow_radius", NULL);
  config->window_halign = g_key_file_get_string(keyfile, "Window", "halign", NULL);
  config->window_valign = g_key_file_get_string(keyfile, "Window", "valign", NULL);
  
  // Grid options
  config->grid_margin_start = g_key_file_get_integer(keyfile, "Grid", "margin_start", NULL);
  config->grid_margin_end = g_key_file_get_integer(keyfile, "Grid", "margin_end", NULL);
  config->grid_margin_top = g_key_file_get_integer(keyfile, "Grid", "margin_top", NULL);
  config->grid_margin_bottom = g_key_file_get_integer(keyfile, "Grid", "margin_bottom", NULL);
  config->grid_row_spacing = g_key_file_get_integer(keyfile, "Grid", "row_spacing", NULL);
  config->grid_column_spacing = g_key_file_get_integer(keyfile, "Grid", "column_spacing", NULL);
  config->grid_halign = g_key_file_get_string(keyfile, "Grid", "halign", NULL);
  config->grid_valign = g_key_file_get_string(keyfile, "Grid", "valign", NULL);
  config->grid_hexpand = g_key_file_get_boolean(keyfile, "Grid", "hexpand", NULL);
  config->grid_vexpand = g_key_file_get_boolean(keyfile, "Grid", "vexpand", NULL);
  config->grid_columns = g_key_file_get_integer(keyfile, "Grid", "columns", NULL);
  config->grid_item_size = g_key_file_get_integer(keyfile, "Grid", "item_size", NULL);
  config->grid_item_corner_radius = g_key_file_get_integer(keyfile, "Grid", "item_corner_radius", NULL);
  config->grid_item_border_width = g_key_file_get_integer(keyfile, "Grid", "item_border_width", NULL);
  config->grid_item_border_color = g_key_file_get_string(keyfile, "Grid", "item_border_color", NULL);
  config->grid_item_background_color = g_key_file_get_string(keyfile, "Grid", "item_background_color", NULL);
  config->grid_opacity = g_key_file_get_double(keyfile, "Grid", "opacity", NULL);
  config->grid_item_opacity = g_key_file_get_double(keyfile, "Grid", "item_opacity", NULL);
  
  // List options
  config->list_item_size = g_key_file_get_integer(keyfile, "List", "item_size", NULL);
  config->list_item_corner_radius = g_key_file_get_integer(keyfile, "List", "item_corner_radius", NULL);
  config->list_item_border_width = g_key_file_get_integer(keyfile, "List", "item_border_width", NULL);
  config->list_item_border_color = g_key_file_get_string(keyfile, "List", "item_border_color", NULL);
  config->list_item_background_color = g_key_file_get_string(keyfile, "List", "item_background_color", NULL);
  config->list_row_spacing = g_key_file_get_integer(keyfile, "List", "row_spacing", NULL);
  config->list_halign = g_key_file_get_string(keyfile, "List", "halign", NULL);
  config->list_valign = g_key_file_get_string(keyfile, "List", "valign", NULL);
  config->list_hexpand = g_key_file_get_boolean(keyfile, "List", "hexpand", NULL);
  config->list_vexpand = g_key_file_get_boolean(keyfile, "List", "vexpand", NULL);
  config->list_margin_start = g_key_file_get_integer(keyfile, "List", "margin_start", NULL);
  config->list_margin_end = g_key_file_get_integer(keyfile, "List", "margin_end", NULL);
  config->list_margin_top = g_key_file_get_integer(keyfile, "List", "margin_top", NULL);
  config->list_margin_bottom = g_key_file_get_integer(keyfile, "List", "margin_bottom", NULL);
  config->list_opacity = g_key_file_get_double(keyfile, "List", "opacity", NULL);
  config->list_item_opacity = g_key_file_get_double(keyfile, "List", "item_opacity", NULL);
  
  // AppEntry options
  config->app_icon_size = g_key_file_get_integer(keyfile, "AppEntry", "icon_size", NULL);
  config->app_icon_corner_radius = g_key_file_get_integer(keyfile, "AppEntry", "icon_corner_radius", NULL);
  config->app_icon_background_color = g_key_file_get_string(keyfile, "AppEntry", "icon_background_color", NULL);
  config->app_name_font_size = g_key_file_get_integer(keyfile, "AppEntry", "name_font_size", NULL);
  config->app_name_color = g_key_file_get_string(keyfile, "AppEntry", "name_color", NULL);
  config->app_desc_font_size = g_key_file_get_integer(keyfile, "AppEntry", "desc_font_size", NULL);
  config->app_desc_color = g_key_file_get_string(keyfile, "AppEntry", "desc_color", NULL);
  config->app_entry_padding = g_key_file_get_integer(keyfile, "AppEntry", "padding", NULL);
  config->app_entry_hover_color = g_key_file_get_string(keyfile, "AppEntry", "hover_color", NULL);
  config->app_entry_active_color = g_key_file_get_string(keyfile, "AppEntry", "active_color", NULL);
  config->app_entry_opacity = g_key_file_get_double(keyfile, "AppEntry", "opacity", NULL);
  config->app_icon_opacity = g_key_file_get_double(keyfile, "AppEntry", "icon_opacity", NULL);
  config->app_name_opacity = g_key_file_get_double(keyfile, "AppEntry", "name_opacity", NULL);
  config->app_desc_opacity = g_key_file_get_double(keyfile, "AppEntry", "desc_opacity", NULL);
  
  // Category options
  config->category_background_color = g_key_file_get_string(keyfile, "Category", "background_color", NULL);
  config->category_background_opacity = g_key_file_get_double(keyfile, "Category", "background_opacity", NULL);
  config->category_corner_radius = g_key_file_get_integer(keyfile, "Category", "corner_radius", NULL);
  config->category_text_color = g_key_file_get_string(keyfile, "Category", "text_color", NULL);
  config->category_font_size = g_key_file_get_integer(keyfile, "Category", "font_size", NULL);
  config->category_font_family = g_key_file_get_string(keyfile, "Category", "font_family", NULL);
  config->category_padding = g_key_file_get_integer(keyfile, "Category", "padding", NULL);
  config->category_show_separators = g_key_file_get_boolean(keyfile, "Category", "show_separators", NULL);
  config->category_separator_color = g_key_file_get_string(keyfile, "Category", "separator_color", NULL);
  config->category_opacity = g_key_file_get_double(keyfile, "Category", "opacity", NULL);
  config->category_title_opacity = g_key_file_get_double(keyfile, "Category", "title_opacity", NULL);
  
  // Search options
  config->search_background_color = g_key_file_get_string(keyfile, "Search", "background_color", NULL);
  config->search_background_opacity = g_key_file_get_double(keyfile, "Search", "background_opacity", NULL);
  config->search_corner_radius = g_key_file_get_integer(keyfile, "Search", "corner_radius", NULL);
  config->search_text_color = g_key_file_get_string(keyfile, "Search", "text_color", NULL);
  config->search_font_size = g_key_file_get_integer(keyfile, "Search", "font_size", NULL);
  config->search_font_family = g_key_file_get_string(keyfile, "Search", "font_family", NULL);
  config->search_padding = g_key_file_get_integer(keyfile, "Search", "padding", NULL);
  config->search_min_height = g_key_file_get_integer(keyfile, "Search", "min_height", NULL);
  config->search_left_padding = g_key_file_get_integer(keyfile, "Search", "left_padding", NULL);
  config->search_length = g_key_file_get_integer(keyfile, "Search", "length", NULL);
  config->search_placeholder_text = g_key_file_get_string(keyfile, "Search", "placeholder_text", NULL);
  config->search_icon_size = g_key_file_get_integer(keyfile, "Search", "icon_size", NULL);
  config->search_icon_color = g_key_file_get_string(keyfile, "Search", "icon_color", NULL);
  config->search_focus_border_color = g_key_file_get_string(keyfile, "Search", "focus_border_color", NULL);
  config->search_focus_shadow_color = g_key_file_get_string(keyfile, "Search", "focus_shadow_color", NULL);
  config->search_opacity = g_key_file_get_double(keyfile, "Search", "opacity", NULL);
  config->search_text_opacity = g_key_file_get_double(keyfile, "Search", "text_opacity", NULL);
  config->search_icon_opacity = g_key_file_get_double(keyfile, "Search", "icon_opacity", NULL);
  
  // SystemButton options
  config->system_button_background_color = g_key_file_get_string(keyfile, "SystemButton", "background_color", NULL);
  config->system_button_icon_color = g_key_file_get_string(keyfile, "SystemButton", "icon_color", NULL);
  config->system_button_hover_color = g_key_file_get_string(keyfile, "SystemButton", "hover_color", NULL);
  config->system_button_active_color = g_key_file_get_string(keyfile, "SystemButton", "active_color", NULL);
  config->system_button_corner_radius = g_key_file_get_integer(keyfile, "SystemButton", "corner_radius", NULL);
  config->system_button_size = g_key_file_get_integer(keyfile, "SystemButton", "size", NULL);
  config->system_button_spacing = g_key_file_get_integer(keyfile, "SystemButton", "spacing", NULL);
  config->system_button_opacity = g_key_file_get_double(keyfile, "SystemButton", "opacity", NULL);
  config->system_button_icon_opacity = g_key_file_get_double(keyfile, "SystemButton", "icon_opacity", NULL);
  
  // Behavior options
  config->close_on_click_outside = g_key_file_get_boolean(keyfile, "Behavior", "close_on_click_outside", NULL);
  config->close_on_super_key = g_key_file_get_boolean(keyfile, "Behavior", "close_on_super_key", NULL);
  config->close_on_app_launch = g_key_file_get_boolean(keyfile, "Behavior", "close_on_app_launch", NULL);
  config->focus_search_on_open = g_key_file_get_boolean(keyfile, "Behavior", "focus_search_on_open", NULL);
  config->close_on_escape = g_key_file_get_boolean(keyfile, "Behavior", "close_on_escape", NULL);
  config->close_on_focus_out = g_key_file_get_boolean(keyfile, "Behavior", "close_on_focus_out", NULL);
  config->show_categories = g_key_file_get_boolean(keyfile, "Behavior", "show_categories", NULL);
  config->show_descriptions = g_key_file_get_boolean(keyfile, "Behavior", "show_descriptions", NULL);
  config->show_icons = g_key_file_get_boolean(keyfile, "Behavior", "show_icons", NULL);
  config->show_search = g_key_file_get_boolean(keyfile, "Behavior", "show_search", NULL);
  config->show_scrollbar = g_key_file_get_boolean(keyfile, "Behavior", "show_scrollbar", NULL);
  config->show_border = g_key_file_get_boolean(keyfile, "Behavior", "show_border", NULL);
  config->show_shadow = g_key_file_get_boolean(keyfile, "Behavior", "show_shadow", NULL);
  config->blur_background = g_key_file_get_boolean(keyfile, "Behavior", "blur_background", NULL);
  config->blur_strength = g_key_file_get_integer(keyfile, "Behavior", "blur_strength", NULL);
  config->opacity = g_key_file_get_double(keyfile, "Behavior", "opacity", NULL);
  config->max_recent_apps = g_key_file_get_integer(keyfile, "Behavior", "max_recent_apps", NULL);
  
  // Outer border (window)
  config->outer_border_width = g_key_file_get_integer(keyfile, "Window", "outer_border_width", NULL);
  config->outer_border_color = g_key_file_get_string(keyfile, "Window", "outer_border_color", NULL);
  config->outer_border_radius = g_key_file_get_integer(keyfile, "Window", "outer_border_radius", NULL);
  
  // Hyprland-specific settings
  config->use_hyprland_corner_fix = g_key_file_get_boolean(keyfile, "Hyprland", "use_corner_fix", NULL);
  config->hyprland_corner_radius = g_key_file_get_integer(keyfile, "Hyprland", "corner_radius", NULL);
  
  // Font options
  config->app_name_font_family = g_key_file_get_string(keyfile, "AppEntry", "name_font_family", NULL);
  config->app_desc_font_family = g_key_file_get_string(keyfile, "AppEntry", "desc_font_family", NULL);
  config->grid_item_font_family = g_key_file_get_string(keyfile, "Grid", "item_font_family", NULL);
  config->list_item_font_family = g_key_file_get_string(keyfile, "List", "item_font_family", NULL);
  config->app_name_font_weight = g_key_file_get_integer(keyfile, "AppEntry", "name_font_weight", NULL);
  config->app_desc_font_weight = g_key_file_get_integer(keyfile, "AppEntry", "desc_font_weight", NULL);
  config->grid_item_font_weight = g_key_file_get_integer(keyfile, "Grid", "item_font_weight", NULL);
  config->list_item_font_weight = g_key_file_get_integer(keyfile, "List", "item_font_weight", NULL);
  config->app_name_font_style = g_key_file_get_string(keyfile, "AppEntry", "name_font_style", NULL);
  config->app_desc_font_style = g_key_file_get_string(keyfile, "AppEntry", "desc_font_style", NULL);
  config->grid_item_font_style = g_key_file_get_string(keyfile, "Grid", "item_font_style", NULL);
  config->list_item_font_style = g_key_file_get_string(keyfile, "List", "item_font_style", NULL);

  // Grid hover/active
  config->grid_item_hover_color = g_key_file_get_string(keyfile, "Grid", "item_hover_color", NULL);
  config->grid_item_active_color = g_key_file_get_string(keyfile, "Grid", "item_active_color", NULL);
  config->grid_item_padding = g_key_file_get_integer(keyfile, "Grid", "item_padding", NULL);

  // List hover/active
  config->list_item_hover_color = g_key_file_get_string(keyfile, "List", "item_hover_color", NULL);
  config->list_item_active_color = g_key_file_get_string(keyfile, "List", "item_active_color", NULL);
  config->list_item_padding = g_key_file_get_integer(keyfile, "List", "item_padding", NULL);

  // Category hover/active
  config->category_hover_color = g_key_file_get_string(keyfile, "Category", "hover_color", NULL);
  config->category_active_color = g_key_file_get_string(keyfile, "Category", "active_color", NULL);
  config->category_separator_thickness = g_key_file_get_integer(keyfile, "Category", "separator_thickness", NULL);

  // Scrollbar
  config->scrollbar_color = g_key_file_get_string(keyfile, "Scrollbar", "color", NULL);
  config->scrollbar_hover_color = g_key_file_get_string(keyfile, "Scrollbar", "hover_color", NULL);
  config->scrollbar_active_color = g_key_file_get_string(keyfile, "Scrollbar", "active_color", NULL);
  config->scrollbar_width = g_key_file_get_integer(keyfile, "Scrollbar", "width", NULL);
  config->scrollbar_corner_radius = g_key_file_get_integer(keyfile, "Scrollbar", "corner_radius", NULL);
  
  g_key_file_free(keyfile);
}

void
hyprmenu_config_save(HyprMenuConfig *config)
{
  if (!config) return;
  
  GKeyFile *keyfile = g_key_file_new();
  GError *error = NULL;
  
  // Try to load existing config first
  if (g_file_test(config->config_file, G_FILE_TEST_EXISTS)) {
    if (!g_key_file_load_from_file(keyfile, config->config_file, G_KEY_FILE_NONE, &error)) {
      g_warning("Failed to load existing config file: %s", error->message);
      g_error_free(error);
    }
  }
  
  // Only set values that don't exist in the config file
  if (!g_key_file_has_key(keyfile, "Window", "width", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "width", config->window_width);
  }
  if (!g_key_file_has_key(keyfile, "Window", "height", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "height", config->window_height);
  }
  if (!g_key_file_has_key(keyfile, "Window", "top_margin", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "top_margin", config->top_margin);
  }
  if (!g_key_file_has_key(keyfile, "Window", "left_margin", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "left_margin", config->left_margin);
  }
  if (!g_key_file_has_key(keyfile, "Window", "center_window", NULL)) {
    g_key_file_set_boolean(keyfile, "Window", "center_window", config->center_window);
  }
  if (!g_key_file_has_key(keyfile, "Window", "menu_position", NULL)) {
    g_key_file_set_string(keyfile, "Window", "menu_position", hyprmenu_position_to_string(config->menu_position));
  }
  if (!g_key_file_has_key(keyfile, "Window", "bottom_offset", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "bottom_offset", config->bottom_offset);
  }
  if (!g_key_file_has_key(keyfile, "Window", "top_offset", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "top_offset", config->top_offset);
  }
  if (!g_key_file_has_key(keyfile, "Window", "padding", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "padding", config->window_padding);
  }
  
  // Window style
  if (!g_key_file_has_key(keyfile, "Window", "background_opacity", NULL)) {
    g_key_file_set_double(keyfile, "Window", "background_opacity", config->window_background_opacity);
  }
  if (!g_key_file_has_key(keyfile, "Window", "background_blur", NULL)) {
    g_key_file_set_double(keyfile, "Window", "background_blur", config->window_background_blur);
  }
  if (!g_key_file_has_key(keyfile, "Window", "background_color", NULL)) {
    g_key_file_set_string(keyfile, "Window", "background_color", config->window_background_color);
    g_key_file_set_comment(keyfile, "Window", "background_color", "# Added by HyprMenu vX.Y", NULL);
  }
  if (!g_key_file_has_key(keyfile, "Window", "inner_border_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "inner_border_radius", config->inner_border_radius);
  }
  if (!g_key_file_has_key(keyfile, "Window", "inner_border_width", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "inner_border_width", config->inner_border_width);
  }
  if (!g_key_file_has_key(keyfile, "Window", "inner_border_color", NULL)) {
    g_key_file_set_string(keyfile, "Window", "inner_border_color", config->inner_border_color);
  }
  if (!g_key_file_has_key(keyfile, "Window", "shadow_color", NULL)) {
    g_key_file_set_string(keyfile, "Window", "shadow_color", config->window_shadow_color);
  }
  if (!g_key_file_has_key(keyfile, "Window", "shadow_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "shadow_radius", config->window_shadow_radius);
  }
  if (!g_key_file_has_key(keyfile, "Window", "halign", NULL)) {
    g_key_file_set_string(keyfile, "Window", "halign", config->window_halign);
  }
  if (!g_key_file_has_key(keyfile, "Window", "valign", NULL)) {
    g_key_file_set_string(keyfile, "Window", "valign", config->window_valign);
  }
  
  // Grid options
  if (!g_key_file_has_key(keyfile, "Grid", "margin_start", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "margin_start", config->grid_margin_start);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "margin_end", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "margin_end", config->grid_margin_end);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "margin_top", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "margin_top", config->grid_margin_top);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "margin_bottom", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "margin_bottom", config->grid_margin_bottom);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "row_spacing", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "row_spacing", config->grid_row_spacing);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "column_spacing", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "column_spacing", config->grid_column_spacing);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "halign", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "halign", config->grid_halign);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "valign", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "valign", config->grid_valign);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "hexpand", NULL)) {
    g_key_file_set_boolean(keyfile, "Grid", "hexpand", config->grid_hexpand);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "vexpand", NULL)) {
    g_key_file_set_boolean(keyfile, "Grid", "vexpand", config->grid_vexpand);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "columns", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "columns", config->grid_columns);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_size", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "item_size", config->grid_item_size);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "item_corner_radius", config->grid_item_corner_radius);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_border_width", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "item_border_width", config->grid_item_border_width);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_border_color", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "item_border_color", config->grid_item_border_color);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_background_color", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "item_background_color", config->grid_item_background_color);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "opacity", NULL)) {
    g_key_file_set_double(keyfile, "Grid", "opacity", config->grid_opacity);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_opacity", NULL)) {
    g_key_file_set_double(keyfile, "Grid", "item_opacity", config->grid_item_opacity);
  }
  
  // List options
  if (!g_key_file_has_key(keyfile, "List", "item_size", NULL)) {
    g_key_file_set_integer(keyfile, "List", "item_size", config->list_item_size);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "List", "item_corner_radius", config->list_item_corner_radius);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_border_width", NULL)) {
    g_key_file_set_integer(keyfile, "List", "item_border_width", config->list_item_border_width);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_border_color", NULL)) {
    g_key_file_set_string(keyfile, "List", "item_border_color", config->list_item_border_color);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_background_color", NULL)) {
    g_key_file_set_string(keyfile, "List", "item_background_color", config->list_item_background_color);
  }
  if (!g_key_file_has_key(keyfile, "List", "row_spacing", NULL)) {
    g_key_file_set_integer(keyfile, "List", "row_spacing", config->list_row_spacing);
  }
  if (!g_key_file_has_key(keyfile, "List", "halign", NULL)) {
    g_key_file_set_string(keyfile, "List", "halign", config->list_halign);
  }
  if (!g_key_file_has_key(keyfile, "List", "valign", NULL)) {
    g_key_file_set_string(keyfile, "List", "valign", config->list_valign);
  }
  if (!g_key_file_has_key(keyfile, "List", "hexpand", NULL)) {
    g_key_file_set_boolean(keyfile, "List", "hexpand", config->list_hexpand);
  }
  if (!g_key_file_has_key(keyfile, "List", "vexpand", NULL)) {
    g_key_file_set_boolean(keyfile, "List", "vexpand", config->list_vexpand);
  }
  if (!g_key_file_has_key(keyfile, "List", "margin_start", NULL)) {
    g_key_file_set_integer(keyfile, "List", "margin_start", config->list_margin_start);
  }
  if (!g_key_file_has_key(keyfile, "List", "margin_end", NULL)) {
    g_key_file_set_integer(keyfile, "List", "margin_end", config->list_margin_end);
  }
  if (!g_key_file_has_key(keyfile, "List", "margin_top", NULL)) {
    g_key_file_set_integer(keyfile, "List", "margin_top", config->list_margin_top);
  }
  if (!g_key_file_has_key(keyfile, "List", "margin_bottom", NULL)) {
    g_key_file_set_integer(keyfile, "List", "margin_bottom", config->list_margin_bottom);
  }
  if (!g_key_file_has_key(keyfile, "List", "opacity", NULL)) {
    g_key_file_set_double(keyfile, "List", "opacity", config->list_opacity);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_opacity", NULL)) {
    g_key_file_set_double(keyfile, "List", "item_opacity", config->list_item_opacity);
  }
  
  // AppEntry options
  if (!g_key_file_has_key(keyfile, "AppEntry", "icon_size", NULL)) {
    g_key_file_set_integer(keyfile, "AppEntry", "icon_size", config->app_icon_size);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "icon_corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "AppEntry", "icon_corner_radius", config->app_icon_corner_radius);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "icon_background_color", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "icon_background_color", config->app_icon_background_color);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "name_font_size", NULL)) {
    g_key_file_set_integer(keyfile, "AppEntry", "name_font_size", config->app_name_font_size);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "name_color", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "name_color", config->app_name_color);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "desc_font_size", NULL)) {
    g_key_file_set_integer(keyfile, "AppEntry", "desc_font_size", config->app_desc_font_size);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "desc_color", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "desc_color", config->app_desc_color);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "padding", NULL)) {
    g_key_file_set_integer(keyfile, "AppEntry", "padding", config->app_entry_padding);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "hover_color", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "hover_color", config->app_entry_hover_color);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "active_color", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "active_color", config->app_entry_active_color);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "opacity", NULL)) {
    g_key_file_set_double(keyfile, "AppEntry", "opacity", config->app_entry_opacity);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "icon_opacity", NULL)) {
    g_key_file_set_double(keyfile, "AppEntry", "icon_opacity", config->app_icon_opacity);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "name_opacity", NULL)) {
    g_key_file_set_double(keyfile, "AppEntry", "name_opacity", config->app_name_opacity);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "desc_opacity", NULL)) {
    g_key_file_set_double(keyfile, "AppEntry", "desc_opacity", config->app_desc_opacity);
  }
  
  // Category options
  if (!g_key_file_has_key(keyfile, "Category", "background_color", NULL)) {
    g_key_file_set_string(keyfile, "Category", "background_color", config->category_background_color);
  }
  if (!g_key_file_has_key(keyfile, "Category", "background_opacity", NULL)) {
    g_key_file_set_double(keyfile, "Category", "background_opacity", config->category_background_opacity);
  }
  if (!g_key_file_has_key(keyfile, "Category", "corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Category", "corner_radius", config->category_corner_radius);
  }
  if (!g_key_file_has_key(keyfile, "Category", "text_color", NULL)) {
    g_key_file_set_string(keyfile, "Category", "text_color", config->category_text_color);
  }
  if (!g_key_file_has_key(keyfile, "Category", "font_size", NULL)) {
    g_key_file_set_integer(keyfile, "Category", "font_size", config->category_font_size);
  }
  if (!g_key_file_has_key(keyfile, "Category", "font_family", NULL)) {
    g_key_file_set_string(keyfile, "Category", "font_family", config->category_font_family);
  }
  if (!g_key_file_has_key(keyfile, "Category", "padding", NULL)) {
    g_key_file_set_integer(keyfile, "Category", "padding", config->category_padding);
  }
  if (!g_key_file_has_key(keyfile, "Category", "show_separators", NULL)) {
    g_key_file_set_boolean(keyfile, "Category", "show_separators", config->category_show_separators);
  }
  if (!g_key_file_has_key(keyfile, "Category", "separator_color", NULL)) {
    g_key_file_set_string(keyfile, "Category", "separator_color", config->category_separator_color);
  }
  if (!g_key_file_has_key(keyfile, "Category", "opacity", NULL)) {
    g_key_file_set_double(keyfile, "Category", "opacity", config->category_opacity);
  }
  if (!g_key_file_has_key(keyfile, "Category", "title_opacity", NULL)) {
    g_key_file_set_double(keyfile, "Category", "title_opacity", config->category_title_opacity);
  }
  
  // Search options
  if (!g_key_file_has_key(keyfile, "Search", "background_color", NULL)) {
    g_key_file_set_string(keyfile, "Search", "background_color", config->search_background_color);
  }
  if (!g_key_file_has_key(keyfile, "Search", "background_opacity", NULL)) {
    g_key_file_set_double(keyfile, "Search", "background_opacity", config->search_background_opacity);
  }
  if (!g_key_file_has_key(keyfile, "Search", "corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Search", "corner_radius", config->search_corner_radius);
  }
  if (!g_key_file_has_key(keyfile, "Search", "text_color", NULL)) {
    g_key_file_set_string(keyfile, "Search", "text_color", config->search_text_color);
  }
  if (!g_key_file_has_key(keyfile, "Search", "font_size", NULL)) {
    g_key_file_set_integer(keyfile, "Search", "font_size", config->search_font_size);
  }
  if (!g_key_file_has_key(keyfile, "Search", "font_family", NULL)) {
    g_key_file_set_string(keyfile, "Search", "font_family", config->search_font_family);
  }
  if (!g_key_file_has_key(keyfile, "Search", "padding", NULL)) {
    g_key_file_set_integer(keyfile, "Search", "padding", config->search_padding);
  }
  if (!g_key_file_has_key(keyfile, "Search", "min_height", NULL)) {
    g_key_file_set_integer(keyfile, "Search", "min_height", config->search_min_height);
  }
  if (!g_key_file_has_key(keyfile, "Search", "left_padding", NULL)) {
    g_key_file_set_integer(keyfile, "Search", "left_padding", config->search_left_padding);
  }
  if (!g_key_file_has_key(keyfile, "Search", "length", NULL)) {
    g_key_file_set_integer(keyfile, "Search", "length", config->search_length);
  }
  if (!g_key_file_has_key(keyfile, "Search", "placeholder_text", NULL)) {
    g_key_file_set_string(keyfile, "Search", "placeholder_text", config->search_placeholder_text);
  }
  if (!g_key_file_has_key(keyfile, "Search", "icon_size", NULL)) {
    g_key_file_set_integer(keyfile, "Search", "icon_size", config->search_icon_size);
  }
  if (!g_key_file_has_key(keyfile, "Search", "icon_color", NULL)) {
    g_key_file_set_string(keyfile, "Search", "icon_color", config->search_icon_color);
  }
  if (!g_key_file_has_key(keyfile, "Search", "focus_border_color", NULL)) {
    g_key_file_set_string(keyfile, "Search", "focus_border_color", config->search_focus_border_color);
  }
  if (!g_key_file_has_key(keyfile, "Search", "focus_shadow_color", NULL)) {
    g_key_file_set_string(keyfile, "Search", "focus_shadow_color", config->search_focus_shadow_color);
  }
  if (!g_key_file_has_key(keyfile, "Search", "opacity", NULL)) {
    g_key_file_set_double(keyfile, "Search", "opacity", config->search_opacity);
  }
  if (!g_key_file_has_key(keyfile, "Search", "text_opacity", NULL)) {
    g_key_file_set_double(keyfile, "Search", "text_opacity", config->search_text_opacity);
  }
  if (!g_key_file_has_key(keyfile, "Search", "icon_opacity", NULL)) {
    g_key_file_set_double(keyfile, "Search", "icon_opacity", config->search_icon_opacity);
  }
  
  // SystemButton options
  if (!g_key_file_has_key(keyfile, "SystemButton", "background_color", NULL)) {
    g_key_file_set_string(keyfile, "SystemButton", "background_color", config->system_button_background_color);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "icon_color", NULL)) {
    g_key_file_set_string(keyfile, "SystemButton", "icon_color", config->system_button_icon_color);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "hover_color", NULL)) {
    g_key_file_set_string(keyfile, "SystemButton", "hover_color", config->system_button_hover_color);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "active_color", NULL)) {
    g_key_file_set_string(keyfile, "SystemButton", "active_color", config->system_button_active_color);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "SystemButton", "corner_radius", config->system_button_corner_radius);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "size", NULL)) {
    g_key_file_set_integer(keyfile, "SystemButton", "size", config->system_button_size);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "spacing", NULL)) {
    g_key_file_set_integer(keyfile, "SystemButton", "spacing", config->system_button_spacing);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "opacity", NULL)) {
    g_key_file_set_double(keyfile, "SystemButton", "opacity", config->system_button_opacity);
  }
  if (!g_key_file_has_key(keyfile, "SystemButton", "icon_opacity", NULL)) {
    g_key_file_set_double(keyfile, "SystemButton", "icon_opacity", config->system_button_icon_opacity);
  }
  
  // Behavior options
  if (!g_key_file_has_key(keyfile, "Behavior", "close_on_click_outside", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "close_on_click_outside", config->close_on_click_outside);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "close_on_super_key", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "close_on_super_key", config->close_on_super_key);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "close_on_app_launch", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "close_on_app_launch", config->close_on_app_launch);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "focus_search_on_open", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "focus_search_on_open", config->focus_search_on_open);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "close_on_escape", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "close_on_escape", config->close_on_escape);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "close_on_focus_out", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "close_on_focus_out", config->close_on_focus_out);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "show_categories", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "show_categories", config->show_categories);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "show_descriptions", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "show_descriptions", config->show_descriptions);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "show_icons", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "show_icons", config->show_icons);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "show_search", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "show_search", config->show_search);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "show_scrollbar", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "show_scrollbar", config->show_scrollbar);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "show_border", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "show_border", config->show_border);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "show_shadow", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "show_shadow", config->show_shadow);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "blur_background", NULL)) {
    g_key_file_set_boolean(keyfile, "Behavior", "blur_background", config->blur_background);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "blur_strength", NULL)) {
    g_key_file_set_integer(keyfile, "Behavior", "blur_strength", config->blur_strength);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "opacity", NULL)) {
    g_key_file_set_double(keyfile, "Behavior", "opacity", config->opacity);
  }
  if (!g_key_file_has_key(keyfile, "Behavior", "max_recent_apps", NULL)) {
    g_key_file_set_integer(keyfile, "Behavior", "max_recent_apps", config->max_recent_apps);
  }
  
  // Outer border (window)
  if (!g_key_file_has_key(keyfile, "Window", "outer_border_width", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "outer_border_width", config->outer_border_width);
  }
  if (!g_key_file_has_key(keyfile, "Window", "outer_border_color", NULL)) {
    g_key_file_set_string(keyfile, "Window", "outer_border_color", config->outer_border_color);
  }
  if (!g_key_file_has_key(keyfile, "Window", "outer_border_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Window", "outer_border_radius", config->outer_border_radius);
  }
  
  // Hyprland-specific settings
  if (!g_key_file_has_key(keyfile, "Hyprland", "use_corner_fix", NULL)) {
    g_key_file_set_boolean(keyfile, "Hyprland", "use_corner_fix", config->use_hyprland_corner_fix);
  }
  if (!g_key_file_has_key(keyfile, "Hyprland", "corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Hyprland", "corner_radius", config->hyprland_corner_radius);
  }
  
  // Grid new options
  if (!g_key_file_has_key(keyfile, "Grid", "item_font_family", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "item_font_family", config->grid_item_font_family);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_font_weight", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "item_font_weight", config->grid_item_font_weight);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_font_style", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "item_font_style", config->grid_item_font_style);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_hover_color", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "item_hover_color", config->grid_item_hover_color);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_active_color", NULL)) {
    g_key_file_set_string(keyfile, "Grid", "item_active_color", config->grid_item_active_color);
  }
  if (!g_key_file_has_key(keyfile, "Grid", "item_padding", NULL)) {
    g_key_file_set_integer(keyfile, "Grid", "item_padding", config->grid_item_padding);
  }

  // List new options
  if (!g_key_file_has_key(keyfile, "List", "item_font_family", NULL)) {
    g_key_file_set_string(keyfile, "List", "item_font_family", config->list_item_font_family);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_font_weight", NULL)) {
    g_key_file_set_integer(keyfile, "List", "item_font_weight", config->list_item_font_weight);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_font_style", NULL)) {
    g_key_file_set_string(keyfile, "List", "item_font_style", config->list_item_font_style);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_hover_color", NULL)) {
    g_key_file_set_string(keyfile, "List", "item_hover_color", config->list_item_hover_color);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_active_color", NULL)) {
    g_key_file_set_string(keyfile, "List", "item_active_color", config->list_item_active_color);
  }
  if (!g_key_file_has_key(keyfile, "List", "item_padding", NULL)) {
    g_key_file_set_integer(keyfile, "List", "item_padding", config->list_item_padding);
  }

  // AppEntry new options
  if (!g_key_file_has_key(keyfile, "AppEntry", "name_font_family", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "name_font_family", config->app_name_font_family);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "name_font_weight", NULL)) {
    g_key_file_set_integer(keyfile, "AppEntry", "name_font_weight", config->app_name_font_weight);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "name_font_style", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "name_font_style", config->app_name_font_style);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "desc_font_family", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "desc_font_family", config->app_desc_font_family);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "desc_font_weight", NULL)) {
    g_key_file_set_integer(keyfile, "AppEntry", "desc_font_weight", config->app_desc_font_weight);
  }
  if (!g_key_file_has_key(keyfile, "AppEntry", "desc_font_style", NULL)) {
    g_key_file_set_string(keyfile, "AppEntry", "desc_font_style", config->app_desc_font_style);
  }

  // Category new options
  if (!g_key_file_has_key(keyfile, "Category", "hover_color", NULL)) {
    g_key_file_set_string(keyfile, "Category", "hover_color", config->category_hover_color);
  }
  if (!g_key_file_has_key(keyfile, "Category", "active_color", NULL)) {
    g_key_file_set_string(keyfile, "Category", "active_color", config->category_active_color);
  }
  if (!g_key_file_has_key(keyfile, "Category", "separator_thickness", NULL)) {
    g_key_file_set_integer(keyfile, "Category", "separator_thickness", config->category_separator_thickness);
  }

  // Scrollbar options
  if (!g_key_file_has_key(keyfile, "Scrollbar", "color", NULL)) {
    g_key_file_set_string(keyfile, "Scrollbar", "color", config->scrollbar_color);
  }
  if (!g_key_file_has_key(keyfile, "Scrollbar", "hover_color", NULL)) {
    g_key_file_set_string(keyfile, "Scrollbar", "hover_color", config->scrollbar_hover_color);
  }
  if (!g_key_file_has_key(keyfile, "Scrollbar", "active_color", NULL)) {
    g_key_file_set_string(keyfile, "Scrollbar", "active_color", config->scrollbar_active_color);
  }
  if (!g_key_file_has_key(keyfile, "Scrollbar", "width", NULL)) {
    g_key_file_set_integer(keyfile, "Scrollbar", "width", config->scrollbar_width);
  }
  if (!g_key_file_has_key(keyfile, "Scrollbar", "corner_radius", NULL)) {
    g_key_file_set_integer(keyfile, "Scrollbar", "corner_radius", config->scrollbar_corner_radius);
  }
  
  // Write config to file
  gchar *data = g_key_file_to_data(keyfile, NULL, &error);
  if (error) {
    g_warning("Failed to convert config to string: %s", error->message);
    g_error_free(error);
    g_key_file_free(keyfile);
    return;
  }
  
  if (!g_file_set_contents(config->config_file, data, strlen(data), &error)) {
    g_warning("Failed to write config file: %s", error->message);
    g_error_free(error);
  }
  
  g_free(data);
  g_key_file_free(keyfile);
}

void
hyprmenu_config_apply_css(HyprMenuConfig *config)
{
  GString *css = g_string_new("");

  // Window style
  if (config->window_background_color && *config->window_background_color)
    g_string_append_printf(css, "window { background-color: %s; }\n", config->window_background_color);
  if (config->window_background_opacity < 1.0) {
    g_string_append_printf(css, ".hyprmenu-window { background-color: rgba(0, 0, 0, %.2f); }\n", config->window_background_opacity);
    g_string_append_printf(css, ".hyprmenu-content-box { background-color: rgba(0, 0, 0, %.2f); }\n", config->window_background_opacity);
    g_string_append_printf(css, ".hyprmenu-main-box { background-color: rgba(0, 0, 0, %.2f); }\n", config->window_background_opacity);
    g_string_append_printf(css, ".hyprmenu-content-container { background-color: rgba(0, 0, 0, %.2f); }\n", config->window_background_opacity);
  }
  if (config->window_background_blur > 0)
    g_string_append_printf(css, "window { backdrop-filter: blur(%fpx); }\n", config->window_background_blur);
  if (config->inner_border_color && *config->inner_border_color)
    g_string_append_printf(css, "window { border-color: %s; }\n", config->inner_border_color);
  if (config->inner_border_width > 0)
    g_string_append_printf(css, "window { border-width: %dpx; }\n", config->inner_border_width);
  if (config->inner_border_radius > 0)
    g_string_append_printf(css, "window { border-radius: %dpx; }\n", config->inner_border_radius);
  if (config->outer_border_color && *config->outer_border_color)
    g_string_append_printf(css, "window { outline-color: %s; }\n", config->outer_border_color);
  if (config->outer_border_width > 0)
    g_string_append_printf(css, "window { outline-width: %dpx; outline-style: solid; }\n", config->outer_border_width);
  if (config->outer_border_radius > 0)
    g_string_append_printf(css, "window { outline-radius: %dpx; }\n", config->outer_border_radius);
  if (config->window_shadow_color && *config->window_shadow_color)
    g_string_append_printf(css, "window { box-shadow: 0 0 %dpx %s; }\n", config->window_shadow_radius, config->window_shadow_color);

  // Grid style
  if (config->grid_item_background_color && *config->grid_item_background_color)
    g_string_append_printf(css, "grid-item { background-color: %s; }\n", config->grid_item_background_color);
  if (config->grid_item_border_color && *config->grid_item_border_color)
    g_string_append_printf(css, "grid-item { border-color: %s; }\n", config->grid_item_border_color);
  if (config->grid_item_border_width > 0)
    g_string_append_printf(css, "grid-item { border-width: %dpx; }\n", config->grid_item_border_width);
  if (config->grid_item_corner_radius > 0)
    g_string_append_printf(css, "grid-item { border-radius: %dpx; }\n", config->grid_item_corner_radius);
  if (config->grid_opacity < 1.0)
    g_string_append_printf(css, ".hyprmenu-app-grid { opacity: %f; }\n", config->grid_opacity);
  if (config->grid_item_opacity < 1.0)
    g_string_append_printf(css, "grid-item { opacity: %f; }\n", config->grid_item_opacity);

  // List style
  if (config->list_item_background_color && *config->list_item_background_color)
    g_string_append_printf(css, "list-item { background-color: %s; }\n", config->list_item_background_color);
  if (config->list_item_border_color && *config->list_item_border_color)
    g_string_append_printf(css, "list-item { border-color: %s; }\n", config->list_item_border_color);
  if (config->list_item_border_width > 0)
    g_string_append_printf(css, "list-item { border-width: %dpx; }\n", config->list_item_border_width);
  if (config->list_item_corner_radius > 0)
    g_string_append_printf(css, "list-item { border-radius: %dpx; }\n", config->list_item_corner_radius);
  if (config->list_opacity < 1.0)
    g_string_append_printf(css, ".hyprmenu-list-row { opacity: %f; }\n", config->list_opacity);
  if (config->list_item_opacity < 1.0)
    g_string_append_printf(css, "list-item { opacity: %f; }\n", config->list_item_opacity);

  // AppEntry style
  if (config->app_icon_background_color && *config->app_icon_background_color)
    g_string_append_printf(css, "app-icon { background-color: %s; }\n", config->app_icon_background_color);
  if (config->app_name_color && *config->app_name_color)
    g_string_append_printf(css, "app-name { color: %s; }\n", config->app_name_color);
  if (config->app_desc_color && *config->app_desc_color)
    g_string_append_printf(css, "app-desc { color: %s; }\n", config->app_desc_color);
  if (config->app_entry_hover_color && *config->app_entry_hover_color)
    g_string_append_printf(css, "app-entry:hover { background-color: %s; }\n", config->app_entry_hover_color);
  if (config->app_entry_active_color && *config->app_entry_active_color)
    g_string_append_printf(css, "app-entry:active { background-color: %s; }\n", config->app_entry_active_color);
  if (config->app_entry_opacity < 1.0)
    g_string_append_printf(css, ".hyprmenu-app-entry { opacity: %f; }\n", config->app_entry_opacity);
  if (config->app_icon_opacity < 1.0)
    g_string_append_printf(css, "app-icon { opacity: %f; }\n", config->app_icon_opacity);
  if (config->app_name_opacity < 1.0)
    g_string_append_printf(css, "app-name { opacity: %f; }\n", config->app_name_opacity);
  if (config->app_desc_opacity < 1.0)
    g_string_append_printf(css, "app-desc { opacity: %f; }\n", config->app_desc_opacity);

  // Category style
  if (config->category_background_color && *config->category_background_color)
    g_string_append_printf(css, "category { background-color: %s; }\n", config->category_background_color);
  if (config->category_text_color && *config->category_text_color)
    g_string_append_printf(css, "category-title { color: %s; }\n", config->category_text_color);
  if (config->category_font_family && *config->category_font_family)
    g_string_append_printf(css, "category-title { font-family: %s; }\n", config->category_font_family);
  if (config->category_separator_color && *config->category_separator_color)
    g_string_append_printf(css, "category-separator { background-color: %s; }\n", config->category_separator_color);
  if (config->category_opacity < 1.0)
    g_string_append_printf(css, ".hyprmenu-category { opacity: %f; }\n", config->category_opacity);
  if (config->category_title_opacity < 1.0)
    g_string_append_printf(css, "category-title { opacity: %f; }\n", config->category_title_opacity);

  // Search style
  if (config->search_background_color && *config->search_background_color)
    g_string_append_printf(css, "search { background-color: %s; }\n", config->search_background_color);
  if (config->search_text_color && *config->search_text_color)
    g_string_append_printf(css, "search { color: %s; }\n", config->search_text_color);
  if (config->search_font_family && *config->search_font_family)
    g_string_append_printf(css, "search { font-family: %s; }\n", config->search_font_family);
  if (config->search_icon_color && *config->search_icon_color)
    g_string_append_printf(css, "search-icon { color: %s; }\n", config->search_icon_color);
  if (config->search_focus_border_color && *config->search_focus_border_color)
    g_string_append_printf(css, "search:focus { border-color: %s; }\n", config->search_focus_border_color);
  if (config->search_focus_shadow_color && *config->search_focus_shadow_color)
    g_string_append_printf(css, "search:focus { box-shadow: 0 0 5px %s; }\n", config->search_focus_shadow_color);
  if (config->search_opacity < 1.0)
    g_string_append_printf(css, ".hyprmenu-search { opacity: %f; }\n", config->search_opacity);
  if (config->search_text_opacity < 1.0)
    g_string_append_printf(css, "search { text-opacity: %f; }\n", config->search_text_opacity);
  if (config->search_icon_opacity < 1.0)
    g_string_append_printf(css, "search-icon { opacity: %f; }\n", config->search_icon_opacity);

  // SystemButton style
  if (config->system_button_background_color && *config->system_button_background_color)
    g_string_append_printf(css, "system-button { background-color: %s; }\n", config->system_button_background_color);
  if (config->system_button_icon_color && *config->system_button_icon_color)
    g_string_append_printf(css, "system-button-icon { color: %s; }\n", config->system_button_icon_color);
  if (config->system_button_hover_color && *config->system_button_hover_color)
    g_string_append_printf(css, "system-button:hover { background-color: %s; }\n", config->system_button_hover_color);
  if (config->system_button_active_color && *config->system_button_active_color)
    g_string_append_printf(css, "system-button:active { background-color: %s; }\n", config->system_button_active_color);
  if (config->system_button_opacity < 1.0)
    g_string_append_printf(css, ".system-button { opacity: %f; }\n", config->system_button_opacity);
  if (config->system_button_icon_opacity < 1.0)
    g_string_append_printf(css, "system-button-icon { opacity: %f; }\n", config->system_button_icon_opacity);

  // Font options
  if (config->app_name_font_family && *config->app_name_font_family)
    g_string_append_printf(css, "app-name { font-family: %s; }\n", config->app_name_font_family);
  if (config->app_desc_font_family && *config->app_desc_font_family)
    g_string_append_printf(css, "app-desc { font-family: %s; }\n", config->app_desc_font_family);
  if (config->grid_item_font_family && *config->grid_item_font_family)
    g_string_append_printf(css, "grid-item { font-family: %s; }\n", config->grid_item_font_family);
  if (config->list_item_font_family && *config->list_item_font_family)
    g_string_append_printf(css, "list-item { font-family: %s; }\n", config->list_item_font_family);

  if (config->app_name_font_weight)
    g_string_append_printf(css, "app-name { font-weight: %d; }\n", config->app_name_font_weight);
  if (config->app_desc_font_weight)
    g_string_append_printf(css, "app-desc { font-weight: %d; }\n", config->app_desc_font_weight);
  if (config->grid_item_font_weight)
    g_string_append_printf(css, "grid-item { font-weight: %d; }\n", config->grid_item_font_weight);
  if (config->list_item_font_weight)
    g_string_append_printf(css, "list-item { font-weight: %d; }\n", config->list_item_font_weight);

  if (config->app_name_font_style && *config->app_name_font_style)
    g_string_append_printf(css, "app-name { font-style: %s; }\n", config->app_name_font_style);
  if (config->app_desc_font_style && *config->app_desc_font_style)
    g_string_append_printf(css, "app-desc { font-style: %s; }\n", config->app_desc_font_style);
  if (config->grid_item_font_style && *config->grid_item_font_style)
    g_string_append_printf(css, "grid-item { font-style: %s; }\n", config->grid_item_font_style);
  if (config->list_item_font_style && *config->list_item_font_style)
    g_string_append_printf(css, "list-item { font-style: %s; }\n", config->list_item_font_style);

  // Grid hover/active
  if (config->grid_item_hover_color && *config->grid_item_hover_color)
    g_string_append_printf(css, "grid-item:hover { background-color: %s; }\n", config->grid_item_hover_color);
  if (config->grid_item_active_color && *config->grid_item_active_color)
    g_string_append_printf(css, "grid-item:active { background-color: %s; }\n", config->grid_item_active_color);
  if (config->grid_item_padding > 0)
    g_string_append_printf(css, "grid-item { padding: %dpx; }\n", config->grid_item_padding);

  // List hover/active
  if (config->list_item_hover_color && *config->list_item_hover_color)
    g_string_append_printf(css, "list-item:hover { background-color: %s; }\n", config->list_item_hover_color);
  if (config->list_item_active_color && *config->list_item_active_color)
    g_string_append_printf(css, "list-item:active { background-color: %s; }\n", config->list_item_active_color);
  if (config->list_item_padding > 0)
    g_string_append_printf(css, "list-item { padding: %dpx; }\n", config->list_item_padding);

  // Category hover/active
  if (config->category_hover_color && *config->category_hover_color)
    g_string_append_printf(css, "category:hover { background-color: %s; }\n", config->category_hover_color);
  if (config->category_active_color && *config->category_active_color)
    g_string_append_printf(css, "category:active { background-color: %s; }\n", config->category_active_color);
  if (config->category_separator_thickness > 0)
    g_string_append_printf(css, "category-separator { height: %dpx; }\n", config->category_separator_thickness);

  // Scrollbar
  if (config->scrollbar_color && *config->scrollbar_color)
    g_string_append_printf(css, "scrollbar { background-color: %s; }\n", config->scrollbar_color);
  if (config->scrollbar_hover_color && *config->scrollbar_hover_color)
    g_string_append_printf(css, "scrollbar:hover { background-color: %s; }\n", config->scrollbar_hover_color);
  if (config->scrollbar_active_color && *config->scrollbar_active_color)
    g_string_append_printf(css, "scrollbar:active { background-color: %s; }\n", config->scrollbar_active_color);
  if (config->scrollbar_width > 0)
    g_string_append_printf(css, "scrollbar { width: %dpx; }\n", config->scrollbar_width);
  if (config->scrollbar_corner_radius > 0)
    g_string_append_printf(css, "scrollbar { border-radius: %dpx; }\n", config->scrollbar_corner_radius);

  // Write CSS to file
  if (!config->css_file || !*config->css_file) {
    g_warning("CSS file path is not set. Skipping CSS write.");
    g_string_free(css, TRUE);
    return;
  }
  GError *error = NULL;
  if (!g_file_set_contents(config->css_file, css->str, css->len, &error)) {
    g_warning("Failed to write CSS file: %s", error->message);
    g_error_free(error);
  }

  g_string_free(css, TRUE);
} 