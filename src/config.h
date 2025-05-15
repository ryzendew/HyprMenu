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
  
  // File paths
  char *config_dir;
  char *config_file;
  char *css_file;

  // Window options
  int window_corner_radius;
  int window_border_width;
  char *window_border_color;
  double window_background_opacity;
  double window_background_blur;
  char *window_background_color;
  int window_padding;
  char *window_halign;
  char *window_valign;
  char *window_shadow_color;
  int window_shadow_radius;

  // Inner border (main box)
  int inner_border_width;
  char *inner_border_color;
  int inner_border_radius;

  // Outer border (window)
  int outer_border_width;
  char *outer_border_color;
  int outer_border_radius;

  // Grid options
  int grid_margin_start;
  int grid_margin_end;
  int grid_margin_top;
  int grid_margin_bottom;
  int grid_row_spacing;
  int grid_column_spacing;
  char *grid_halign;
  char *grid_valign;
  gboolean grid_hexpand;
  gboolean grid_vexpand;
  int grid_columns;
  int grid_item_size;
  int grid_item_corner_radius;
  int grid_item_border_width;
  char *grid_item_border_color;
  char *grid_item_background_color;

  // List options
  int list_item_size;
  int list_item_corner_radius;
  int list_item_border_width;
  char *list_item_border_color;
  char *list_item_background_color;
  int list_row_spacing;
  char *list_halign;
  char *list_valign;
  gboolean list_hexpand;
  gboolean list_vexpand;
  int list_margin_start;
  int list_margin_end;
  int list_margin_top;
  int list_margin_bottom;
  
  // AppEntry options
  int app_icon_size;
  int app_icon_corner_radius;
  char *app_icon_background_color;
  int app_name_font_size;
  char *app_name_color;
  int app_desc_font_size;
  char *app_desc_color;
  int app_entry_padding;
  char *app_entry_hover_color;
  char *app_entry_active_color;
  
  // Category options
  char *category_background_color;
  double category_background_opacity;
  int category_corner_radius;
  char *category_text_color;
  int category_font_size;
  char *category_font_family;
  int category_padding;
  gboolean category_show_separators;
  char *category_separator_color;
  
  // Search options
  char *search_background_color;
  double search_background_opacity;
  int search_corner_radius;
  char *search_text_color;
  int search_font_size;
  char *search_font_family;
  int search_padding;
  int search_min_height;
  int search_left_padding;
  int search_length;
  char *search_placeholder_text;
  int search_icon_size;
  char *search_icon_color;
  char *search_focus_border_color;
  char *search_focus_shadow_color;

  // SystemButton options
  char *system_button_background_color;
  char *system_button_icon_color;
  char *system_button_hover_color;
  char *system_button_active_color;
  int system_button_corner_radius;
  int system_button_size;
  int system_button_spacing;

  // Behavior options
  gboolean close_on_click_outside;
  gboolean close_on_super_key;
  gboolean close_on_app_launch;
  gboolean focus_search_on_open;
  gboolean close_on_escape;
  gboolean close_on_focus_out;
  gboolean show_categories;
  gboolean show_descriptions;
  gboolean show_icons;
  gboolean show_search;
  gboolean show_scrollbar;
  gboolean show_border;
  gboolean show_shadow;
  gboolean blur_background;
  int blur_strength;
  double opacity;
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

// Pywal color struct
#define PYWAL_COLOR_COUNT 16

typedef struct {
  char *colors[PYWAL_COLOR_COUNT]; // colors[0] = color0, ...
  char *special_background;
  char *special_foreground;
  char *special_cursor;
} PywalColors;

// Global pywal color instance
extern PywalColors pywal_colors; 