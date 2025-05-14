#!/bin/bash

# Script to manually pin an app to HyprMenu

# Get the app ID from command line
APP_ID="$1"

if [ -z "$APP_ID" ]; then
  echo "Usage: $0 <app_id.desktop>"
  echo "Example: $0 firefox.desktop"
  exit 1
fi

# Make sure the config directory exists
CONFIG_DIR="$HOME/.config/hyprmenu"
mkdir -p "$CONFIG_DIR"

# Create/append to the pinned apps file
PINNED_FILE="$CONFIG_DIR/pinned.txt"
if [ -f "$PINNED_FILE" ]; then
  # Check if already exists
  if grep -q "^$APP_ID$" "$PINNED_FILE"; then
    echo "App $APP_ID is already pinned"
    exit 0
  fi
  echo "Adding app $APP_ID to existing pinned list"
else
  echo "Creating new pinned apps file with $APP_ID"
fi

# Add the app ID to the file
echo "$APP_ID" >> "$PINNED_FILE"
echo "Successfully pinned $APP_ID - restart HyprMenu to see changes"

# Show current pinned apps
echo "Current pinned apps:"
cat "$PINNED_FILE" 