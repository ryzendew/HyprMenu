# HyprMenu Rebuild Plan

## Better Technology Stack for Hyprland

### Option 1: GTK4 with Layer Shell (Recommended)
- **Benefits**:
  - Native Wayland support
  - Direct layer-shell protocol integration
  - Better Hyprland compatibility
  - Modern UI toolkit
  - Excellent theming support
- **Steps**:
  - Use GTK4 + libgtk-layer-shell
  - Use GObject-based structure
  - Can still categorize apps as planned

### Option 2: Use eww (Elkowar's Wacky Widgets)
- **Benefits**:
  - Specifically designed for Wayland compositors
  - Built for creating Hyprland widgets
  - SCSS styling for modern look
  - Simple configuration
  - Used by many Hyprland rice creators
- **Steps**:
  - Define menu in Yuck markup
  - Style with SCSS
  - Use eww's scripting to list applications

### Option 3: Use a Rofi-based approach
- **Benefits**:
  - Rofi is already well-tested on Hyprland
  - Simple theming
  - Fast
- **Steps**:
  - Create custom Rofi mode
  - Style to match Windows 11

## Recommended Approach: GTK4 with Layer Shell

This approach gives us the most flexibility while ensuring native compatibility with Hyprland.

### Project Structure
```
src/
├── main.c            # Application entry point
├── menu_window.c     # Main menu window with layer shell  
├── application.c     # Application management
├── app_list.c        # Application listing widget
├── category_view.c   # Category-based app view
├── search.c          # Search functionality
├── settings.c        # Settings dialog
└── utils/
    ├── desktop.c     # Desktop entry handling
    └── config.c      # Configuration handling
```

### Implementation Steps
1. Create basic GTK4 app with layer shell integration
2. Add XDG app scanning logic
3. Implement category-based grouping
4. Add search functionality
5. Implement settings and configuration
6. Add modern styling and animations

### Dependencies
- gtk4
- gtk-layer-shell
- json-glib
- libadwaita (optional, for modern UI) 