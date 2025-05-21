#include <glib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

// Function to read pywal colors
static gboolean
read_pywal_colors(PywalColors *colors)
{
    char *home = g_get_home_dir();
    char *colors_file = g_build_filename(home, ".cache", "wal", "colors", NULL);
    
    if (!g_file_test(colors_file, G_FILE_TEST_EXISTS)) {
        g_free(colors_file);
        return FALSE;
    }
    
    FILE *f = fopen(colors_file, "r");
    if (!f) {
        g_free(colors_file);
        return FALSE;
    }
    
    char line[32];
    int i = 0;
    while (fgets(line, sizeof(line), f) && i < PYWAL_COLOR_COUNT) {
        line[strcspn(line, "\n")] = 0;
        colors->colors[i] = g_strdup(line);
        i++;
    }
    
    fclose(f);
    g_free(colors_file);
    return TRUE;
}

// Main function to detect and apply theme colors
void
hyprmenu_apply_theme_colors(HyprMenuConfig *config)
{
    // Try pywal first
    if (read_pywal_colors(&pywal_colors)) {
        g_message("Applying Pywal colors:");
        g_message("Background: %s", pywal_colors.colors[0]);
        g_message("Text: %s", pywal_colors.colors[7]);
        g_message("Hover: %s", pywal_colors.colors[8]);
        
        // Apply pywal colors
        g_free(config->window_background_color);
        config->window_background_color = g_strdup(pywal_colors.colors[0]);
        
        g_free(config->app_name_color);
        config->app_name_color = g_strdup(pywal_colors.colors[7]);
        
        g_free(config->app_entry_hover_color);
        config->app_entry_hover_color = g_strdup(pywal_colors.colors[8]);
        
        // Apply colors to more elements
        g_free(config->search_text_color);
        config->search_text_color = g_strdup(pywal_colors.colors[7]);
        
        g_free(config->category_text_color);
        config->category_text_color = g_strdup(pywal_colors.colors[7]);
        
        g_free(config->app_desc_color);
        config->app_desc_color = g_strdup(pywal_colors.colors[7]);
        
        g_free(config->system_button_icon_color);
        config->system_button_icon_color = g_strdup(pywal_colors.colors[7]);
        
        g_free(config->search_icon_color);
        config->search_icon_color = g_strdup(pywal_colors.colors[7]);
        
        hyprmenu_config_apply_css();
    } else {
        g_message("No Pywal colors found, using default colors");
    }
} 