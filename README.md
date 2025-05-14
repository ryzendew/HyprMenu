# HyprMenu

A modern application launcher for Hyprland, built with GTK4 and GTK Layer Shell.

![image](https://github.com/user-attachments/assets/1beff4e7-e30a-4bf0-bb71-750691c8d09b)


![image](https://github.com/user-attachments/assets/913c0183-eab6-4e7d-b8fa-e3bfc295bfbc)


![image](https://github.com/user-attachments/assets/0c7de1d8-dab3-4eeb-a84e-c279d3155e39)



## Requirements

- Wayland compositor (specifically designed for Hyprland)
- GTK4
- gtk4-layer-shell

## Important Note

**This application is WAYLAND-ONLY**. It will not work on X11. The application requires a Wayland compositor (specifically Hyprland) and GTK Layer Shell to function properly.

## Features

- Modern, clean interface
- System control buttons (logout, shutdown, reboot, etc.)
- Configurable positioning
- Search functionality
- Dark theme by default
- Support for both grid and list view modes
- **Full color theming:** Every part of the menu (background, border, search, categories, app entries, system buttons, highlights, separators, scrollbar, shadow, etc.) can be themed via the config file.
- **Pywal & AGS color support:** Optionally auto-theme the menu using pywal or AGS color scripts, or override any color manually.
- **Config auto-update:** The config file is auto-regenerated to include all new options when the app is updated.
- **VSCode color picker friendly:** All color options are compatible with color pickers in editors like VSCode.
- **Dynamic grid columns:** Set the number of app columns in grid view (`grid_columns`), and the menu will auto-resize.
- **Grid/List item sizing:** Control the size of app items in both grid (`grid_item_size`) and list (`list_item_size`) views.
- **Proportional icon sizing:** Icons scale automatically with item size in both views.
- **All settings exposed:** All theming and layout options are exposed in the config and auto-documented.
- **No breaking changes:** All new features are backward compatible.

## Building

```bash
sudo ./build.sh --install
```

## Installation

After building, the application will be installed automatically.

## Configuration

The menu can be positioned in various locations:
- Top Left
- Top Center
- Top Right
- Bottom Left
- Bottom Center
- Bottom Right
- Center

### Color Theming & Auto-Color Support

- All color values support `#RRGGBB`, `#RRGGBBAA`, `rgb()`, or `rgba()` formats.
- You can use the color picker in VSCode or any compatible editor!
- To use pywal or AGS colors, set `use_pywal_colors=true` or `use_ags_colors=true` in the `[Style]` section.
- Any color can be overridden manually in the config.

### Grid and List View Customization

- `use_grid_view` (true/false): Switch between grid and list view.
- `grid_columns`: Number of app columns in grid view. Menu width auto-adjusts unless `window_width` is set.
- `grid_item_size`: Size of grid items (square size in pixels).
- `list_item_size`: Height of items in list view (pixels).

### Example `[View]` Section
```ini
[View]
use_grid_view = false
list_item_size = 48
# For grid view:
grid_columns = 4
grid_item_size = 100
```

### Example `[Style]` Section (Color Roles)
```ini
[Style]
background_color = #181825
border_color = #444444
search_background_color = #22223b
search_text_color = #cdd6f4
category_background_color = #1e1e2e
category_text_color = #f5e0dc
app_entry_background_color = #313244
app_entry_text_color = #cdd6f4
highlight_color = #f38ba8
separator_color = #585b70
scrollbar_color = #6c7086
shadow_color = #00000080
system_button_icon_color = rgba(0, 0, 0, 0.3)
use_pywal_colors = true
use_ags_colors = false
```

### All Options Are Auto-Generated
- When you update HyprMenu, your config will be auto-updated to include all new options and documentation comments.
- No manual migration needed!

## License

This project is licensed under the MIT License. 
