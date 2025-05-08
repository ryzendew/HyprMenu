#!/bin/bash
set -e

# Check if running in Wayland
if [ -z "$WAYLAND_DISPLAY" ]; then
  echo "ERROR: No Wayland display detected. HyprMenu requires Wayland."
  echo "Make sure you're running this from a Hyprland session."
  exit 1
fi

# Create build directory if needed
if [ ! -d "build" ]; then
  echo "Creating build directory..."
  mkdir -p build
fi

# Build the application
echo "Building HyprMenu..."
cd build
meson setup .. || meson setup --reconfigure ..
ninja
cd ..

# Kill any existing instances
if pgrep -f "hyprmenu" > /dev/null; then
  echo "Killing existing HyprMenu instances..."
  pkill -f "hyprmenu" || true
  sleep 0.5
fi

echo "Starting HyprMenu on Wayland display: $WAYLAND_DISPLAY"

# Set environment variables to ensure GTK uses Wayland
export GDK_BACKEND=wayland

# Run the application
build/hyprmenu 