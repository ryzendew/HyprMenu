#include "config.h"
#include <stdio.h>
#include <string.h>

// Global config instance
HyprMenuConfig *config = NULL;

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
  
  // Window style
  config->background_opacity = 0.85;
  config->background_blur = 5.0;
  config->background_color = g_strdup("#1e1e1e");
  config->corner_radius = 12;
  config->border_width = 1;
  config->border_color = g_strdup("rgba(255, 255, 255, 0.1)");
  
  // Search entry style
  config->search_background_color = g_strdup("#323232");
  config->search_background_opacity = 0.7;
  config->search_corner_radius = 8;
  config->search_text_color = g_strdup("#ffffff");
  config->search_font_size = 14;
  config->search_font_family = g_strdup("Sans");
  config->search_padding = 8;
  
  // App entry style
  config->app_entry_background_color = g_strdup("#3c3c3c");
  config->app_entry_background_opacity = 0.7;
  config->app_entry_corner_radius = 6;
  config->app_entry_text_color = g_strdup("#ffffff");
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
  
  // File paths
  config->config_dir = g_build_filename(g_get_user_config_dir(), "hyprmenu", NULL);
  config->config_file = g_build_filename(config->config_dir, "hyprmenu.conf", NULL);
  config->css_file = g_build_filename(config->config_dir, "style.css", NULL);
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
  
  // Free config struct
  g_free(config);
  config = NULL;
}

gboolean
hyprmenu_config_load()
{
  g_autoptr(GKeyFile) keyfile = g_key_file_new();
  
  // Load keyfile
  if (!g_key_file_load_from_file(keyfile, config->config_file, G_KEY_FILE_NONE, NULL)) {
    g_warning("Failed to load config file: %s", config->config_file);
    return FALSE;
  }
  
  // Window layout
  if (g_key_file_has_group(keyfile, "Layout")) {
    config->window_width = g_key_file_get_integer(keyfile, "Layout", "window_width", NULL);
    config->window_height = g_key_file_get_integer(keyfile, "Layout", "window_height", NULL);
    config->top_margin = g_key_file_get_integer(keyfile, "Layout", "top_margin", NULL);
    config->left_margin = g_key_file_get_integer(keyfile, "Layout", "left_margin", NULL);
    config->center_window = g_key_file_get_boolean(keyfile, "Layout", "center_window", NULL);
  }
  
  // Window style
  if (g_key_file_has_group(keyfile, "Style")) {
    config->background_opacity = g_key_file_get_double(keyfile, "Style", "background_opacity", NULL);
    config->background_blur = g_key_file_get_double(keyfile, "Style", "background_blur", NULL);
    g_free(config->background_color);
    config->background_color = g_key_file_get_string(keyfile, "Style", "background_color", NULL);
    config->corner_radius = g_key_file_get_integer(keyfile, "Style", "corner_radius", NULL);
    config->border_width = g_key_file_get_integer(keyfile, "Style", "border_width", NULL);
    g_free(config->border_color);
    config->border_color = g_key_file_get_string(keyfile, "Style", "border_color", NULL);
  }
  
  // Behavior section
  if (g_key_file_has_group(keyfile, "Behavior")) {
    config->close_on_click_outside = g_key_file_get_boolean(keyfile, "Behavior", "close_on_click_outside", NULL);
    config->close_on_super_key = g_key_file_get_boolean(keyfile, "Behavior", "close_on_super_key", NULL);
    config->close_on_app_launch = g_key_file_get_boolean(keyfile, "Behavior", "close_on_app_launch", NULL);
    config->focus_search_on_open = g_key_file_get_boolean(keyfile, "Behavior", "focus_search_on_open", NULL);
    config->max_recent_apps = g_key_file_get_integer(keyfile, "Behavior", "max_recent_apps", NULL);
  }
  
  return TRUE;
}

gboolean
hyprmenu_config_save()
{
  g_autoptr(GKeyFile) keyfile = g_key_file_new();
  
  // Window layout
  g_key_file_set_integer(keyfile, "Layout", "window_width", config->window_width);
  g_key_file_set_integer(keyfile, "Layout", "window_height", config->window_height);
  g_key_file_set_integer(keyfile, "Layout", "top_margin", config->top_margin);
  g_key_file_set_integer(keyfile, "Layout", "left_margin", config->left_margin);
  g_key_file_set_boolean(keyfile, "Layout", "center_window", config->center_window);
  
  // Window style
  g_key_file_set_double(keyfile, "Style", "background_opacity", config->background_opacity);
  g_key_file_set_double(keyfile, "Style", "background_blur", config->background_blur);
  g_key_file_set_string(keyfile, "Style", "background_color", config->background_color);
  g_key_file_set_integer(keyfile, "Style", "corner_radius", config->corner_radius);
  g_key_file_set_integer(keyfile, "Style", "border_width", config->border_width);
  g_key_file_set_string(keyfile, "Style", "border_color", config->border_color);
  
  // Behavior
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_click_outside", config->close_on_click_outside);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_super_key", config->close_on_super_key);
  g_key_file_set_boolean(keyfile, "Behavior", "close_on_app_launch", config->close_on_app_launch);
  g_key_file_set_boolean(keyfile, "Behavior", "focus_search_on_open", config->focus_search_on_open);
  g_key_file_set_integer(keyfile, "Behavior", "max_recent_apps", config->max_recent_apps);
  
  // Add comment
  g_key_file_set_comment(keyfile, NULL, NULL, 
    "HyprMenu Configuration\n"
    "This file is auto-generated and can be edited to customize HyprMenu.\n"
    "Changes will be applied the next time HyprMenu is started.\n", 
    NULL);
  
  // Save to file
  g_autoptr(GError) error = NULL;
  gsize length;
  g_autofree gchar *data = g_key_file_to_data(keyfile, &length, &error);
  
  if (error) {
    g_warning("Failed to generate config data: %s", error->message);
    return FALSE;
  }
  
  if (!g_file_set_contents(config->config_file, data, length, &error)) {
    g_warning("Failed to save config file: %s", error->message);
    return FALSE;
  }
  
  g_print("Configuration saved to %s\n", config->config_file);
  return TRUE;
}

void
hyprmenu_config_apply_css()
{
  // Generate CSS based on configuration
  GString *css = g_string_new("");
  
  // Window styles
  g_string_append_printf(css, 
    ".hyprmenu-window {\n"
    "  background-color: rgba(%s, %.2f);\n"
    "  border-radius: %dpx;\n"
    "  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);\n",
    config->background_color, config->background_opacity, 
    config->corner_radius);
  
  if (config->border_width > 0) {
    g_string_append_printf(css, 
      "  border: %dpx solid %s;\n",
      config->border_width, config->border_color);
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
    "  background-color: rgba(%s, %.2f);\n"
    "  border-radius: %dpx;\n"
    "  padding: %dpx 12px;\n"
    "  font-size: %dpx;\n"
    "  color: %s;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.15);\n"
    "  margin: 8px 4px 12px 4px;\n"
    "}\n"
    ".hyprmenu-search:focus {\n"
    "  border-color: rgba(255, 255, 255, 0.3);\n"
    "  box-shadow: 0 0 0 1px rgba(255, 255, 255, 0.15);\n"
    "}\n",
    config->search_background_color, config->search_background_opacity,
    config->search_corner_radius, config->search_padding,
    config->search_font_size, config->search_text_color);
  
  // App grid styles
  g_string_append(css,
    ".hyprmenu-app-grid {\n"
    "  background-color: transparent;\n"
    "  padding: 4px;\n"
    "}\n"
    ".hyprmenu-category-list {\n"
    "  background-color: transparent;\n"
    "  padding: 4px;\n"
    "}\n");
  
  // Category styles
  g_string_append_printf(css, 
    ".hyprmenu-category {\n"
    "  background-color: rgba(%s, %.2f);\n"
    "  border-radius: %dpx;\n"
    "  margin: 4px 0px;\n"
    "  padding: %dpx;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.05);\n"
    "}\n"
    ".hyprmenu-category-title {\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "  font-weight: bold;\n"
    "  padding: 6px 8px;\n"
    "  margin-bottom: 4px;\n",
    config->category_background_color, config->category_background_opacity,
    config->category_corner_radius, config->category_padding,
    config->category_text_color, config->category_font_size);
  
  // Add separator if configured
  if (config->category_show_separators) {
    g_string_append(css, "  border-bottom: 1px solid rgba(255, 255, 255, 0.1);\n");
  }
  
  g_string_append(css, "}\n");
  
  // App entry styles
  g_string_append_printf(css, 
    ".hyprmenu-app-entry {\n"
    "  background-color: rgba(%s, %.2f);\n"
    "  border-radius: %dpx;\n"
    "  padding: %dpx 8px;\n"
    "  margin: 2px 4px;\n"
    "  transition: all 0.2s ease;\n"
    "}\n"
    ".hyprmenu-app-entry:hover {\n"
    "  background-color: rgba(75, 75, 75, 0.8);\n"
    "}\n"
    ".hyprmenu-app-entry:active {\n"
    "  background-color: rgba(85, 85, 85, 0.9);\n"
    "}\n"
    ".hyprmenu-app-icon {\n"
    "  margin-right: 8px;\n"
    "}\n"
    ".hyprmenu-app-name {\n"
    "  color: %s;\n"
    "  font-size: %dpx;\n"
    "}\n",
    config->app_entry_background_color, config->app_entry_background_opacity,
    config->app_entry_corner_radius, config->app_entry_padding,
    config->app_entry_text_color, config->app_entry_font_size);
  
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
  
  // Save to file
  if (g_file_set_contents(config->css_file, css->str, -1, NULL)) {
    g_print("CSS saved to %s\n", config->css_file);
  } else {
    g_warning("Failed to save CSS file: %s", config->css_file);
  }
  
  // Apply CSS to application
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(provider, css->str);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                          GTK_STYLE_PROVIDER(provider),
                                          GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
  
  // Free string
  g_string_free(css, TRUE);
} 