# HyprMenu

A modern, highly customizable application launcher designed specifically for Wayland compositors, with a focus on Hyprland. HyprMenu offers a sleek interface with extensive theming options, grid and list views, and seamless integration with your desktop environment.

## 🌟 Key Features

- **Modern Interface**: Clean, minimal design with support for blur effects and transparency
- **Dual View Modes**: Switch between grid and list views for different use cases
- **Smart Search**: Instant search with automatic focus on launch
- **System Controls**: Quick access to power, lock, and other system functions
- **Live Configuration**: Changes apply instantly without restart
- **Dark Mode**: Built-in dark theme with customizable colors
- **Hyprland Integration**: Native support for Hyprland's blur and transparency effects

## 🚀 Quick Start

1. **Installation**:
   ```bash
   # Clone the repository
   git clone https://github.com/yourusername/HyprMenu.git
   cd HyprMenu

   # Build and install
   sudo ./build.sh --install
   ```
   This command will:
   - Compile the application
   - Install necessary dependencies
   - Copy the binary to your system
   - Generate a default configuration file

2. **Configuration**:
   - Your config file will be created at `~/.config/hyprmenu/hyprmenu.conf`
   - Edit this file to customize the appearance and behavior
   - Changes are applied instantly - no restart needed

3. **Hyprland Setup**:
   Add these rules to `~/.config/hypr/hyprland.conf` for blur effects:
   ```conf
   layerrule = blur,hyprmenu
   layerrule = xray,hyprmenu
   layerrule = ignorezero
   ```

## ⚙️ Configuration Guide

The configuration file (`hyprmenu.conf`) is organized into logical sections. Here's what each section controls:

### [Window]
Controls the main window appearance:
- `window_width` and `window_height`: Size of the menu
- `window_corner_radius`: Rounded corners
- `window_background_opacity`: Overall transparency (0.0 to 1.0)
- `window_background_blur`: Blur strength
- `window_border_width` and `window_border_color`: Outer border settings

### [Grid]
Settings for grid view mode:
- `grid_columns`: Number of columns
- `grid_item_size`: Size of each app icon
- `grid_row_spacing` and `grid_column_spacing`: Space between items
- `grid_item_corner_radius`: Rounded corners for items

### [List]
Settings for list view mode:
- `list_item_size`: Height of each item
- `list_row_spacing`: Space between items
- `list_item_corner_radius`: Rounded corners for items

### [Search]
Search bar customization:
- `search_background_color`: Background color
- `search_text_color`: Text color
- `search_font_size`: Text size
- `search_corner_radius`: Rounded corners
- `search_placeholder_text`: Placeholder text

### [Behavior]
Control how the menu behaves:
- `close_on_click_outside`: Close when clicking outside
- `close_on_super_key`: Close when pressing Super key
- `focus_search_on_open`: Auto-focus search on launch
- `show_categories`: Show app categories
- `show_descriptions`: Show app descriptions
- `show_icons`: Show app icons

### [SystemButton]
System control buttons (power, lock, etc.):
- `system_button_size`: Size of buttons
- `system_button_spacing`: Space between buttons
- `system_button_icon_color`: Icon color
- `system_button_hover_color`: Hover effect color

## 🎨 Theming

HyprMenu supports extensive theming options:
- **Color Formats**: Use hex (`#RRGGBB`), RGB (`rgb(r,g,b)`), or RGBA (`rgba(r,g,b,a)`)
- **Opacity**: Control transparency for each element (0.0 to 1.0)
- **Blur**: Adjust blur strength for background effects
- **Borders**: Customize inner and outer borders independently
- **Shadows**: Add and customize drop shadows

## 🤝 Contributing

We welcome contributions! Here's how you can help:

1. **Report Issues**:
   - Found a bug? Open an issue with detailed steps to reproduce
   - Have a feature request? Let us know!

2. **Submit Pull Requests**:
   - Fork the repository
   - Create a feature branch
   - Make your changes
   - Submit a pull request

3. **Development Setup**:
   ```bash
   # Install dependencies
   sudo pacman -S gtk4 gtk4-layer-shell

   # Build without installing
   ./build.sh

   # Run from source
   ./build/hyprmenu
   ```

## 📝 Requirements

- Wayland compositor (Hyprland recommended)
- GTK4
- gtk4-layer-shell

## ⚠️ Important Note

This application is **Wayland-only** and will not work on X11. It requires a Wayland compositor (specifically Hyprland) and GTK Layer Shell to function properly.

## 📄 License

This project is licensed under the MIT License. Feel free to use, modify, and distribute it as you wish.
