static void generate_grid_css(GString *css, const HyprMenuConfig *config) {
    g_string_append_printf(css, ".grid {\n");
    if (config->grid_halign[0] != '\0')
        g_string_append_printf(css, "  justify-content: %s;\n", config->grid_halign);
    if (config->grid_hexpand)
        g_string_append_printf(css, "  width: 100%%;\n");
    if (config->grid_columns > 0)
        g_string_append_printf(css, "  grid-template-columns: repeat(%d, 1fr);\n", config->grid_columns);
    if (config->grid_row_spacing > 0)
        g_string_append_printf(css, "  row-gap: %dpx;\n", config->grid_row_spacing);
    if (config->grid_column_spacing > 0)
        g_string_append_printf(css, "  column-gap: %dpx;\n", config->grid_column_spacing);
    if (config->grid_margin_top > 0)
        g_string_append_printf(css, "  margin-top: %dpx;\n", config->grid_margin_top);
    if (config->grid_margin_bottom > 0)
        g_string_append_printf(css, "  margin-bottom: %dpx;\n", config->grid_margin_bottom);
    if (config->grid_margin_start > 0)
        g_string_append_printf(css, "  margin-left: %dpx;\n", config->grid_margin_start);
    if (config->grid_margin_end > 0)
        g_string_append_printf(css, "  margin-right: %dpx;\n", config->grid_margin_end);
    if (config->grid_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->grid_opacity);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".grid-item {\n");
    if (config->grid_item_background_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->grid_item_background_color);
    if (config->grid_item_border_color[0] != '\0')
        g_string_append_printf(css, "  border: %dpx solid %s;\n", config->grid_item_border_width, config->grid_item_border_color);
    if (config->grid_item_corner_radius > 0)
        g_string_append_printf(css, "  border-radius: %dpx;\n", config->grid_item_corner_radius);
    if (config->grid_item_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->grid_item_opacity);
    if (config->grid_item_size > 0)
        g_string_append_printf(css, "  width: %dpx;\n  height: %dpx;\n", config->grid_item_size, config->grid_item_size);
    if (config->grid_item_padding > 0)
        g_string_append_printf(css, "  padding: %dpx;\n", config->grid_item_padding);
    if (config->grid_item_font_family[0] != '\0')
        g_string_append_printf(css, "  font-family: %s;\n", config->grid_item_font_family);
    if (config->grid_item_font_weight > 0)
        g_string_append_printf(css, "  font-weight: %d;\n", config->grid_item_font_weight);
    if (config->grid_item_font_style[0] != '\0')
        g_string_append_printf(css, "  font-style: %s;\n", config->grid_item_font_style);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".grid-item:hover {\n");
    if (config->grid_item_hover_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->grid_item_hover_color);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".grid-item:active {\n");
    if (config->grid_item_active_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->grid_item_active_color);
    g_string_append(css, "}\n\n");
}

static void generate_list_css(GString *css, const HyprMenuConfig *config) {
    g_string_append_printf(css, ".list {\n");
    if (config->list_halign[0] != '\0')
        g_string_append_printf(css, "  justify-content: %s;\n", config->list_halign);
    if (config->list_hexpand)
        g_string_append_printf(css, "  width: 100%%;\n");
    if (config->list_row_spacing > 0)
        g_string_append_printf(css, "  row-gap: %dpx;\n", config->list_row_spacing);
    if (config->list_margin_top > 0)
        g_string_append_printf(css, "  margin-top: %dpx;\n", config->list_margin_top);
    if (config->list_margin_bottom > 0)
        g_string_append_printf(css, "  margin-bottom: %dpx;\n", config->list_margin_bottom);
    if (config->list_margin_start > 0)
        g_string_append_printf(css, "  margin-left: %dpx;\n", config->list_margin_start);
    if (config->list_margin_end > 0)
        g_string_append_printf(css, "  margin-right: %dpx;\n", config->list_margin_end);
    if (config->list_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->list_opacity);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".list-item {\n");
    if (config->list_item_background_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->list_item_background_color);
    if (config->list_item_border_color[0] != '\0')
        g_string_append_printf(css, "  border: %dpx solid %s;\n", config->list_item_border_width, config->list_item_border_color);
    if (config->list_item_corner_radius > 0)
        g_string_append_printf(css, "  border-radius: %dpx;\n", config->list_item_corner_radius);
    if (config->list_item_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->list_item_opacity);
    if (config->list_item_size > 0)
        g_string_append_printf(css, "  height: %dpx;\n", config->list_item_size);
    if (config->list_item_padding > 0)
        g_string_append_printf(css, "  padding: %dpx;\n", config->list_item_padding);
    if (config->list_item_font_family[0] != '\0')
        g_string_append_printf(css, "  font-family: %s;\n", config->list_item_font_family);
    if (config->list_item_font_weight > 0)
        g_string_append_printf(css, "  font-weight: %d;\n", config->list_item_font_weight);
    if (config->list_item_font_style[0] != '\0')
        g_string_append_printf(css, "  font-style: %s;\n", config->list_item_font_style);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".list-item:hover {\n");
    if (config->list_item_hover_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->list_item_hover_color);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".list-item:active {\n");
    if (config->list_item_active_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->list_item_active_color);
    g_string_append(css, "}\n\n");
}

static void generate_app_entry_css(GString *css, const HyprMenuConfig *config) {
    g_string_append(css, ".app-entry {\n");
    if (config->app_entry_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->app_entry_opacity);
    if (config->app_entry_padding > 0)
        g_string_append_printf(css, "  padding: %dpx;\n", config->app_entry_padding);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".app-name {\n");
    if (config->app_name_color[0] != '\0')
        g_string_append_printf(css, "  color: %s;\n", config->app_name_color);
    if (config->app_name_font_size > 0)
        g_string_append_printf(css, "  font-size: %dpx;\n", config->app_name_font_size);
    if (config->app_name_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->app_name_opacity);
    if (config->app_name_font_family[0] != '\0')
        g_string_append_printf(css, "  font-family: %s;\n", config->app_name_font_family);
    if (config->app_name_font_weight > 0)
        g_string_append_printf(css, "  font-weight: %d;\n", config->app_name_font_weight);
    if (config->app_name_font_style[0] != '\0')
        g_string_append_printf(css, "  font-style: %s;\n", config->app_name_font_style);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".app-desc {\n");
    if (config->app_desc_color[0] != '\0')
        g_string_append_printf(css, "  color: %s;\n", config->app_desc_color);
    if (config->app_desc_font_size > 0)
        g_string_append_printf(css, "  font-size: %dpx;\n", config->app_desc_font_size);
    if (config->app_desc_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->app_desc_opacity);
    if (config->app_desc_font_family[0] != '\0')
        g_string_append_printf(css, "  font-family: %s;\n", config->app_desc_font_family);
    if (config->app_desc_font_weight > 0)
        g_string_append_printf(css, "  font-weight: %d;\n", config->app_desc_font_weight);
    if (config->app_desc_font_style[0] != '\0')
        g_string_append_printf(css, "  font-style: %s;\n", config->app_desc_font_style);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".app-icon {\n");
    if (config->app_icon_background_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->app_icon_background_color);
    if (config->app_icon_corner_radius > 0)
        g_string_append_printf(css, "  border-radius: %dpx;\n", config->app_icon_corner_radius);
    if (config->app_icon_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->app_icon_opacity);
    if (config->app_icon_size > 0)
        g_string_append_printf(css, "  width: %dpx;\n  height: %dpx;\n", config->app_icon_size, config->app_icon_size);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".app-entry:hover {\n");
    if (config->app_entry_hover_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->app_entry_hover_color);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".app-entry:active {\n");
    if (config->app_entry_active_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->app_entry_active_color);
    g_string_append(css, "}\n\n");
}

static void generate_category_css(GString *css, const HyprMenuConfig *config) {
    g_string_append(css, ".category {\n");
    if (config->category_background_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->category_background_color);
    if (config->category_background_opacity < 1.0)
        g_string_append_printf(css, "  background-opacity: %.2f;\n", config->category_background_opacity);
    if (config->category_corner_radius > 0)
        g_string_append_printf(css, "  border-radius: %dpx;\n", config->category_corner_radius);
    if (config->category_font_family[0] != '\0')
        g_string_append_printf(css, "  font-family: %s;\n", config->category_font_family);
    if (config->category_font_size > 0)
        g_string_append_printf(css, "  font-size: %dpx;\n", config->category_font_size);
    if (config->category_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->category_opacity);
    if (config->category_padding > 0)
        g_string_append_printf(css, "  padding: %dpx;\n", config->category_padding);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".category-title {\n");
    if (config->category_text_color[0] != '\0')
        g_string_append_printf(css, "  color: %s;\n", config->category_text_color);
    if (config->category_title_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->category_title_opacity);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".category:hover {\n");
    if (config->category_hover_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->category_hover_color);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".category:active {\n");
    if (config->category_active_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->category_active_color);
    g_string_append(css, "}\n\n");

    if (config->category_show_separators) {
        g_string_append(css, ".category-separator {\n");
        if (config->category_separator_color[0] != '\0')
            g_string_append_printf(css, "  background-color: %s;\n", config->category_separator_color);
        if (config->category_separator_thickness > 0)
            g_string_append_printf(css, "  height: %dpx;\n", config->category_separator_thickness);
        g_string_append(css, "}\n\n");
    }
}

static void generate_search_css(GString *css, const HyprMenuConfig *config) {
    g_string_append(css, ".search-bar {\n");
    if (config->search_background_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->search_background_color);
    if (config->search_background_opacity < 1.0)
        g_string_append_printf(css, "  background-opacity: %.2f;\n", config->search_background_opacity);
    if (config->search_corner_radius > 0)
        g_string_append_printf(css, "  border-radius: %dpx;\n", config->search_corner_radius);
    if (config->search_font_family[0] != '\0')
        g_string_append_printf(css, "  font-family: %s;\n", config->search_font_family);
    if (config->search_font_size > 0)
        g_string_append_printf(css, "  font-size: %dpx;\n", config->search_font_size);
    if (config->search_icon_color[0] != '\0')
        g_string_append_printf(css, "  color: %s;\n", config->search_icon_color);
    if (config->search_icon_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->search_icon_opacity);
    if (config->search_icon_size > 0)
        g_string_append_printf(css, "  width: %dpx;\n  height: %dpx;\n", config->search_icon_size, config->search_icon_size);
    if (config->search_left_padding > 0)
        g_string_append_printf(css, "  padding-left: %dpx;\n", config->search_left_padding);
    if (config->search_min_height > 0)
        g_string_append_printf(css, "  min-height: %dpx;\n", config->search_min_height);
    if (config->search_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->search_opacity);
    if (config->search_padding > 0)
        g_string_append_printf(css, "  padding: %dpx;\n", config->search_padding);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".search-bar:focus {\n");
    if (config->search_focus_border_color[0] != '\0')
        g_string_append_printf(css, "  border: 1px solid %s;\n", config->search_focus_border_color);
    if (config->search_focus_shadow_color[0] != '\0')
        g_string_append_printf(css, "  box-shadow: 0 0 0 1px %s;\n", config->search_focus_shadow_color);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".search-text {\n");
    if (config->search_text_color[0] != '\0')
        g_string_append_printf(css, "  color: %s;\n", config->search_text_color);
    if (config->search_text_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->search_text_opacity);
    g_string_append(css, "}\n\n");
}

static void generate_system_button_css(GString *css, const HyprMenuConfig *config) {
    g_string_append(css, ".system-button {\n");
    if (config->system_button_background_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->system_button_background_color);
    if (config->system_button_corner_radius > 0)
        g_string_append_printf(css, "  border-radius: %dpx;\n", config->system_button_corner_radius);
    if (config->system_button_icon_color[0] != '\0')
        g_string_append_printf(css, "  color: %s;\n", config->system_button_icon_color);
    if (config->system_button_icon_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->system_button_icon_opacity);
    if (config->system_button_opacity < 1.0)
        g_string_append_printf(css, "  opacity: %.2f;\n", config->system_button_opacity);
    if (config->system_button_size > 0)
        g_string_append_printf(css, "  width: %dpx;\n  height: %dpx;\n", config->system_button_size, config->system_button_size);
    if (config->system_button_spacing > 0)
        g_string_append_printf(css, "  margin-right: %dpx;\n", config->system_button_spacing);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".system-button:hover {\n");
    if (config->system_button_hover_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->system_button_hover_color);
    g_string_append(css, "}\n\n");

    g_string_append(css, ".system-button:active {\n");
    if (config->system_button_active_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->system_button_active_color);
    g_string_append(css, "}\n\n");
}

static void generate_scrollbar_css(GString *css, const HyprMenuConfig *config) {
    g_string_append(css, "scrollbar {\n");
    if (config->scrollbar_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->scrollbar_color);
    if (config->scrollbar_width > 0)
        g_string_append_printf(css, "  width: %dpx;\n", config->scrollbar_width);
    if (config->scrollbar_corner_radius > 0)
        g_string_append_printf(css, "  border-radius: %dpx;\n", config->scrollbar_corner_radius);
    g_string_append(css, "}\n\n");

    g_string_append(css, "scrollbar:hover {\n");
    if (config->scrollbar_hover_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->scrollbar_hover_color);
    g_string_append(css, "}\n\n");

    g_string_append(css, "scrollbar:active {\n");
    if (config->scrollbar_active_color[0] != '\0')
        g_string_append_printf(css, "  background-color: %s;\n", config->scrollbar_active_color);
    g_string_append(css, "}\n\n");
}

char* generate_css(const HyprMenuConfig *config) {
    GString *css = g_string_new(NULL);

    // Generate CSS for each component
    generate_grid_css(css, config);
    generate_list_css(css, config);
    generate_app_entry_css(css, config);
    generate_category_css(css, config);
    generate_search_css(css, config);
    generate_system_button_css(css, config);
    generate_scrollbar_css(css, config);

    char *result = css->str;
    g_string_free(css, FALSE);
    return result;
} 