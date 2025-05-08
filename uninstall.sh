#!/bin/bash
set -e

# Check if running as root
if [ "$EUID" -ne 0 ]; then
  echo "This script requires root permissions to uninstall."
  echo "Please run with sudo: sudo $0"
  exit 1
fi

echo "Uninstalling HyprMenu..."

# Remove the binary
if [ -f "/usr/bin/hyprmenu" ]; then
  rm -f /usr/bin/hyprmenu
  echo "- Removed binary"
fi

# Remove desktop file
if [ -f "/usr/share/applications/org.hyprmenu.desktop" ]; then
  rm -f /usr/share/applications/org.hyprmenu.desktop
  echo "- Removed desktop file"
fi

# Remove config files
echo -n "Do you want to remove user configuration files? (y/N): "
read -r response
if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]; then
  for user_home in /home/*; do
    if [ -d "$user_home/.config/hyprmenu" ]; then
      rm -rf "$user_home/.config/hyprmenu"
      echo "- Removed config files in $user_home"
    fi
  done
else
  echo "- Keeping user configuration files"
fi

echo ""
echo "HyprMenu has been successfully uninstalled."
echo "Done!" 