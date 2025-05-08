#!/bin/bash

# Set colors for terminal output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}==> Cleaning up HyprMenu project for git...${NC}"

# Remove debug log files
echo -e "${YELLOW}Removing debug log files...${NC}"
rm -f debug_*.log app_debug.log

# Remove backup files
echo -e "${YELLOW}Removing backup files...${NC}"
find . -name "*.bak" -o -name "*~" -o -name "*.backup" -delete

# Clean build directory if requested
if [ "$1" == "--clean-build" ] || [ "$1" == "--full" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build/
fi

# Full cleanup
if [ "$1" == "--full" ]; then
    echo -e "${YELLOW}Performing full cleanup for git...${NC}"
    
    # Remove any object files
    find . -name "*.o" -delete
    
    # Remove any temporary files
    find . -name "*.tmp" -o -name "*.temp" -delete
    
    # Remove executable
    rm -f hyprmenu
    
    # Check for any leftover large files (>1MB)
    echo -e "${YELLOW}Checking for large files (>1MB) that might be accidentally included:${NC}"
    find . -type f -size +1M -not -path "./.git/*" | while read file; do
        echo -e "${RED}  - $file${NC}"
    done
fi

# Show the current status
echo -e "${GREEN}==> Cleanup complete!${NC}"
echo -e "${YELLOW}Current status:${NC}"
git status -s

echo -e "${GREEN}==> Files are now ready for git commit${NC}"
echo -e "Run the following commands to commit:"
echo -e "  git add ."
echo -e "  git commit -m \"Your commit message\"" 