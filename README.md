# HyprMenu

A highly customizable, modern application launcher for Wayland compositors (Hyprland, Sway, etc.)

## Features

- **Fully customizable appearance** via `hyprmenu.conf`
- **Outer and inner borders**: independently style the menu's outer and inner borders
- **Grid and list views**: switchable, with configurable columns, spacing, and item size
- **Most Used Apps**: automatically tracks and displays frequently used applications at the top
- **Flat, icon-only system buttons** (power, lock, etc.)
- **Alphabetically organized config** for easy navigation
- **Per-section options** for Window, Grid, List, AppEntry, Category, Search, SystemButton, Behavior, Style, Layout
- **Live reload**: changes to config are applied instantly
- **Dark mode** and transparency support
- **Extensive color and spacing options**

## Configuration

All options are set in `~/.config/hyprmenu/hyprmenu.conf`. Sections and keys are alphabetical for clarity.

### Example: Borders

```ini
[Window]
outer_border_width=3
outer_border_color=#888888
outer_border_radius=16
inner_border_width=2
inner_border_color=#444444
inner_border_radius=12
```

### Major Sections and Options

- **[Window]**: Outer/inner borders, background, padding, shadow, alignment
- **[Grid]**: Grid columns, item size, spacing, margins, alignment
- **[List]**: List item size, spacing, margins, alignment
- **[AppEntry]**: Icon size, font sizes, colors, padding, hover/active colors
- **[Category]**: Background, text color, font, padding, separators
- **[Search]**: Bar color, font, padding, icon, placeholder, focus border
- **[SystemButton]**: Icon color, hover/active color, size, spacing, flat style
- **[Behavior]**: Close on click, super key, app launch, focus, most used apps, show/hide UI elements
- **[Style]**: Global background, blur, transparency, AGS effects
- **[Layout]**: Window size, margins, position, offsets

### System Buttons: Flat/Icon-Only

System buttons are styled to be flat and icon-only by default. You can further customize their color and hover effect in the `[SystemButton]` section.

### Most Used Apps

HyprMenu tracks which applications you launch and displays your most frequently used apps at the top of the menu for quick access. You can configure the number of most used apps to display:

```ini
[Behavior]
max_recent_apps=5
```

### Example Config Snippet

```ini
[Window]
outer_border_width=3
outer_border_color=#888888
outer_border_radius=16
inner_border_width=2
inner_border_color=#444444
inner_border_radius=12
background_opacity=0.85
background_blur=5.0
shadow_color=rgba(0,0,0,0.3)
shadow_radius=20

[Grid]
grid_columns=5
grid_item_size=100
row_spacing=12
column_spacing=12

[SystemButton]
icon_color=#fff
hover_color=rgba(255,255,255,0.08)
background_color=transparent
corner_radius=0

[Behavior]
max_recent_apps=5
```

## Customization Tips

- **Change border colors/thickness**: Edit `outer_border_*` and `inner_border_*` in `[Window]`.
- **Switch between grid/list**: Use the toggle button or set defaults in `[Grid]`/`[List]`.
- **Make system buttons flat**: Use the default config or set `background_color=transparent` and `corner_radius=0` in `[SystemButton]`.
- **Adjust spacing and padding**: All major sections have `padding`, `margin`, or `spacing` options.
- **Colors**: All color values support hex, rgb(), or rgba().
- **Customize most used apps**: Change the number of apps shown in the "Most Used Apps" section by setting `max_recent_apps` in `[Behavior]`.

## Getting Started

1. Copy or generate a config: `~/.config/hyprmenu/hyprmenu.conf`
2. Edit the config to your liking (see above for options)
3. Run HyprMenu and enjoy your custom launcher!

---

For more details, see the comments in the config file or open an issue for help.

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
- **Most Used Apps section:** Automatically tracks and displays your most frequently used applications at the top of the menu
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

### NixOS

1. Add `hyprmenu` as a flake input

In your top-level `flake.nix`:

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    hyprmenu.url = "github:ryzendew/HyprMenu";
  };

  outputs = { self, nixpkgs, hyprmenu, ... }@inputs: {
    nixosConfigurations.your-hostname = nixpkgs.lib.nixosSystem {
      system = "x86_64-linux";
      modules = [
        {
          environment.systemPackages = [ hyprmenu.packages.${system}.default ];
        }
        # ... your other modules ...
      ];
    };
  };
}
```

---

2. Use in Home Manager

To add `hyprmenu` to your Home Manager config:

```nix
{
  programs.home-manager.enable = true;

  home.packages = [
    inputs.hyprmenu.packages.${pkgs.system}.default
  ];
}
```

> Tip: You may need to pass `inputs` and `pkgs` into your Home Manager module depending on how you’ve structured your flake.

---

3. Run directly

You can run `hyprmenu` directly with:

```sh
nix run github:ryzendew/HyprMenu
```

---

4. Add to systemPackages

If you’re using NixOS and want to install `HyprMenu` system-wide:

```nix
{
  environment.systemPackages = with pkgs; [
    hyprmenu
  ];
}
```

> Make sure you’ve added the flake input and forwarded the `hyprmenu` package as shown above.

## License

This project is licensed under the MIT License.
