#!/bin/bash
set -e

# Check if running as root
if [ "$EUID" -ne 0 ]; then
  echo "This script requires root permissions to install system-wide."
  echo "Please run with sudo: sudo $0"
  exit 1
fi

# Ensure required dependencies are installed
echo "Checking dependencies..."
missing_deps=()

# Check for GTK4
if ! pkg-config --exists gtk4; then
  missing_deps+=("GTK4")
fi

# Check for GTK4-layer-shell
if ! pkg-config --exists gtk4-layer-shell-0; then
  missing_deps+=("gtk4-layer-shell")
fi

# Check for GLib
if ! pkg-config --exists glib-2.0; then
  missing_deps+=("glib2")
fi

# Check for GIO
if ! pkg-config --exists gio-2.0; then
  missing_deps+=("gio")
fi

# Check for GIO-Unix
if ! pkg-config --exists gio-unix-2.0; then
  missing_deps+=("gio-unix")
fi

# Report missing dependencies
if [ ${#missing_deps[@]} -gt 0 ]; then
  echo "Error: The following dependencies are missing:"
  for dep in "${missing_deps[@]}"; do
    echo "  - $dep"
  done
  
  echo ""
  echo "Please install the missing dependencies and try again."
  echo "For Arch-based systems: sudo pacman -S gtk4 gtk4-layer-shell"
  exit 1
fi

echo "All dependencies are satisfied."

# Create build directory if needed
if [ ! -d "build" ]; then
  echo "Creating build directory..."
  mkdir -p build
fi

# Build and install
echo "Building and installing HyprMenu..."
cd build
meson setup --prefix=/usr --buildtype=release .. || meson setup --reconfigure --prefix=/usr --buildtype=release ..
ninja
ninja install

echo ""
echo "HyprMenu has been successfully installed!"
echo ""
echo "You can now run it with:"
echo "  hyprmenu"
echo ""
echo "Add to your Hyprland config:"
echo "  bind = SUPER, space, exec, hyprmenu"
echo ""
echo "Done!" 