#pragma once

#include <glib.h>
#include <gtk/gtk.h>

/* Menu position enum */
typedef enum {
  POSITION_TOP_LEFT = 0,
  POSITION_TOP_CENTER,
  POSITION_TOP_RIGHT,
  POSITION_BOTTOM_LEFT,
  POSITION_BOTTOM_CENTER,
  POSITION_BOTTOM_RIGHT,
  POSITION_CENTER
} HyprMenuPosition;

typedef struct _HyprMenuConfig {
  // Window layout
  int window_width;
  int window_height;
  int top_margin;
  int left_margin;
  gboolean center_window;
  HyprMenuPosition menu_position; // Position of the menu
  int bottom_offset; // Offset from bottom for dock/panel
  int top_offset;    // Offset from top for panel
  
  // Window style
  double background_opacity;
  double background_blur;
  char *background_color;
  int corner_radius;
  int border_width;
  char *border_color;
  
  // AGS-style effects
  gboolean blur_enabled;
  int blur_radius;
  double blur_brightness;
  double blur_contrast;
  double blur_saturation;
  double blur_noise;
  gboolean blur_grayscale;
  
  // Transparency settings
  gboolean transparency_enabled;
  double transparency_alpha;
  gboolean transparency_blur;
  gboolean transparency_shadow;
  char *transparency_shadow_color;
  int transparency_shadow_radius;
  
  // Search entry style
  char *search_background_color;
  double search_background_opacity;
  int search_corner_radius;
  char *search_text_color;
  int search_font_size;
  char *search_font_family;
  int search_padding;
  
  // App entry style
  char *app_entry_background_color;
  double app_entry_background_opacity;
  int app_entry_corner_radius;
  char *app_entry_text_color;
  int app_entry_font_size;
  char *app_entry_font_family;
  int app_entry_padding;
  int app_icon_size;
  
  // Category style
  char *category_background_color;
  double category_background_opacity;
  int category_corner_radius;
  char *category_text_color;
  int category_font_size;
  char *category_font_family;
  int category_padding;
  gboolean category_show_separators;
  
  // Behavior
  gboolean close_on_click_outside;
  gboolean close_on_super_key;
  gboolean close_on_app_launch;
  gboolean focus_search_on_open;
  int max_recent_apps;
  
  // File paths
  char *config_dir;
  char *config_file;
  char *css_file;

  gboolean close_on_focus_out;
  gchar *theme_path;
  gchar *font_family;
  gint font_size;
  gchar *text_color;
  gchar *selected_color;
  gint border_radius;
  gint padding;
  gint spacing;
  gboolean show_icons;
  gboolean show_descriptions;
  gboolean show_categories;
  gboolean show_search;
  gboolean show_scrollbar;
  gboolean show_border;
  gboolean show_shadow;
  gboolean blur_background;
  gint blur_strength;
  gdouble opacity;
  
  // View settings
  gboolean use_grid_view;     // Use grid view instead of list view
  gint grid_columns;          // Number of columns in grid view
  gint grid_item_size;        // Size of grid items (square size in pixels)
} HyprMenuConfig;

// Global config instance
extern HyprMenuConfig *config;

// Function declarations
gboolean hyprmenu_config_init();
void hyprmenu_config_free();
gboolean hyprmenu_config_load();
gboolean hyprmenu_config_save();
gboolean hyprmenu_config_save_with_error(GError **error);
void hyprmenu_config_apply_css();

// Position utility functions
const gchar* hyprmenu_position_to_string(HyprMenuPosition position);
HyprMenuPosition hyprmenu_position_from_string(const gchar *position_str); 