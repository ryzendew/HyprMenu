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

## License

This project is licensed under the MIT License. 