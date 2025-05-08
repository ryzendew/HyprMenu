# HyprMenu

A modern application launcher for Hyprland, built with GTK4 and GTK Layer Shell.

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

## Building

```bash
./build.sh
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

### Changing Between Grid and List View

HyprMenu supports both grid and list viewing modes. To change between them:

1. Edit the configuration file located at `~/.config/hyprmenu/hyprmenu.conf`
2. Find the `[View]` section
3. Change the `use_grid_view` setting:
   - Set to `true` for grid view
   - Set to `false` for list view
   
Example:
```ini
[View]
use_grid_view=true   # Grid view
# or
use_grid_view=false  # List view
```

4. Save the file and restart HyprMenu

## License

This project is licensed under the MIT License. 