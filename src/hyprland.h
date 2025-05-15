#pragma once

#include <gdk/gdk.h>
#include <gdk/wayland/gdkwayland.h>
#include "config.h"

// Hyprland specific hints for window properties
// These are used to communicate with the Hyprland compositor

// Function to detect if running under Hyprland
gboolean hyprmenu_is_hyprland(void);

// Apply Hyprland-specific window properties to fix corner artifacts
void hyprmenu_apply_hyprland_hints(GdkSurface *surface); 