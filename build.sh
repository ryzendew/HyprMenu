#!/bin/bash

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Print with color
print_status() {
    echo -e "${GREEN}==>${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}Warning:${NC} $1"
}

print_error() {
    echo -e "${RED}Error:${NC} $1"
}

# Check dependencies
check_dependencies() {
    local deps=("meson" "ninja" "gcc" "pkg-config")
    local pkgconfig_deps=("gtk4" "gtk4-layer-shell-0" "glib-2.0" "gio-2.0" "gio-unix-2.0")
    local missing=()

    # Check for command dependencies
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            missing+=("$dep")
        fi
    done

    # Check for pkg-config dependencies
    for dep in "${pkgconfig_deps[@]}"; do
        if ! pkg-config --exists "$dep" &> /dev/null; then
            missing+=("$dep")
        fi
    done

    if [ ${#missing[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing[*]}"
        exit 1
    fi
}

# Parse arguments
BUILD_TYPE="release"
INSTALL=0
PREFIX="/usr"
CLEAN=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        --install)
            INSTALL=1
            shift
            ;;
        --prefix)
            PREFIX="$2"
            shift 2
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check dependencies
print_status "Checking dependencies..."
check_dependencies

# Clean build directory if requested
if [ $CLEAN -eq 1 ]; then
    print_status "Cleaning build directory..."
    rm -rf build/
fi

# Configure
print_status "Configuring build..."
meson setup build \
    --prefix="$PREFIX" \
    --buildtype="$BUILD_TYPE" \
    --wrap-mode=nofallback

# Build
print_status "Building HyprMenu..."
meson compile -C build

# Install if requested
if [ $INSTALL -eq 1 ]; then
    if [ "$PREFIX" = "/usr" ]; then
        if [ "$EUID" -ne 0 ]; then
            print_error "Installation to /usr requires root privileges"
            exit 1
        fi
    fi

    print_status "Installing HyprMenu..."
    meson install -C build
    
    # Update icon cache if installing system-wide
    if [ "$PREFIX" = "/usr" ]; then
        print_status "Updating icon cache..."
        gtk-update-icon-cache -f -t /usr/share/icons/hicolor
    fi
fi

print_status "Build completed successfully!"

if [ $INSTALL -eq 0 ]; then
    print_status "To install, run: $0 --install"
fi 