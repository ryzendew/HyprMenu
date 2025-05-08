#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
 
# Launch HyprMenu from the build directory
"${SCRIPT_DIR}/build/hyprmenu" 