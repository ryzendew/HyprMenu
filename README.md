# HyprMenu

A modern, customizable start menu for Hyprland window manager, built with GTK4 and layer-shell protocol.

## Features

- Modern, native Wayland implementation using GTK4 and layer-shell protocol
- Dark theme with clean UI
- Category-based application organization
- Recent applications tracking
- Fast application search
- Proper overlay behavior in Hyprland

## Requirements

- Hyprland compositor
- GTK4
- gtk4-layer-shell
- GLib / GIO

## Installation

### System-wide Installation

1. Install dependencies:
```bash
# For Arch-based systems
sudo pacman -S gtk4 gtk4-layer-shell

# For other distros, find equivalent packages
```

2. Clone the repository:
```bash
git clone https://github.com/yourusername/hyprmenu.git
cd hyprmenu
```

3. Run the installation script:
```bash
sudo ./install.sh
```

This will install HyprMenu system-wide, making it available as `hyprmenu` from any terminal.

### Local Build

If you prefer to build HyprMenu without installing it system-wide:

1. Build the application:
```bash
mkdir -p build
cd build
meson setup ..
ninja
```

2. Run the application:
```bash
./hyprmenu
```

Alternatively, use the included run script:
```bash
./run.sh
```

## Usage

1. Launch HyprMenu with the `hyprmenu` command or by pressing your configured shortcut
2. Browse applications by category (all categories are expandable)
3. Use the search bar to quickly find applications
4. Click on an application to launch it
5. Recently used applications will appear in the "Recent Applications" section

## Integration with Hyprland

Add to your Hyprland config:

```
# If installed system-wide:
bind = SUPER, space, exec, hyprmenu

# If using local build:
bind = SUPER, space, exec, $HOME/path/to/hyprmenu/build/hyprmenu
```

## License

This project is licensed under the MIT License. 