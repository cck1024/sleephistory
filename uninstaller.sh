#!/bin/bash

# We don't use 'set -e' here because 'rm' might fail if a file is already gone, 
# and we want the script to continue to check other locations.

# Colors for better readability in the terminal
GREEN='\033[0;32m'
MAG='\033[0;35m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Helper function for user confirmations
ask_confirm() {
    local message="$1"
    while true; do
        read -p "$(echo -e "${YELLOW}? $message (y/n): ${NC}")" choice
        case "$choice" in
            [YyJj]* ) return 0 ;;
            [Nn]* ) return 1 ;;
            * ) echo "Please answer with 'y' for Yes or 'n' for No." ;;
        esac
    done
}

echo -e "${MAG}========================================${NC}"
echo -e "${MAG}   sleephistory Uninstaller Started     ${NC}"
echo -e "${MAG}========================================${NC}\n"

# Define paths
LOCAL_BIN="$HOME/.local/bin/sleephistory"
LOCAL_ICON="$HOME/.local/share/icons/sleephistory.png"
LOCAL_DESKTOP="$HOME/.local/share/applications/sleephistory.desktop"

GLOBAL_BIN="/usr/local/bin/sleephistory"
GLOBAL_ICON="/usr/share/pixmaps/sleephistory.png"
GLOBAL_DESKTOP="/usr/share/applications/sleephistory.desktop"

# Variables to track what was uninstalled
LOCAL_UNINSTALLED=false
GLOBAL_UNINSTALLED=false

# ---------------------------------------------------------
# STEP 1: Check for Local Installation
# ---------------------------------------------------------
echo -e "${MAG}[1/2] Checking for LOCAL installation...${NC}"

LOCAL_FILES_FOUND=()
[ -f "$LOCAL_BIN" ] && LOCAL_FILES_FOUND+=("$LOCAL_BIN")
[ -f "$LOCAL_ICON" ] && LOCAL_FILES_FOUND+=("$LOCAL_ICON")
[ -f "$LOCAL_DESKTOP" ] && LOCAL_FILES_FOUND+=("$LOCAL_DESKTOP")

if [ ${#LOCAL_FILES_FOUND[@]} -gt 0 ]; then
    echo -e "${YELLOW}Found local installation files:${NC}"
    for file in "${LOCAL_FILES_FOUND[@]}"; do
        echo " - $file"
    done
    echo ""
    
    if ask_confirm "Do you want to completely remove the LOCAL installation"; then
        echo -e "${GREEN}-> Removing local files...${NC}"
        for file in "${LOCAL_FILES_FOUND[@]}"; do
            rm -f "$file"
            echo "   Deleted: $file"
        done
        LOCAL_UNINSTALLED=true
        echo -e "${GREEN}-> Local installation successfully removed.${NC}\n"
    else
        echo -e "${YELLOW}-> Skipping local uninstallation.${NC}\n"
    fi
else
    echo -e "No local installation found.\n"
fi

# ---------------------------------------------------------
# STEP 2: Check for Global Installation
# ---------------------------------------------------------
echo -e "${MAG}[2/2] Checking for GLOBAL installation...${NC}"

GLOBAL_FILES_FOUND=()
[ -f "$GLOBAL_BIN" ] && GLOBAL_FILES_FOUND+=("$GLOBAL_BIN")
[ -f "$GLOBAL_ICON" ] && GLOBAL_FILES_FOUND+=("$GLOBAL_ICON")
[ -f "$GLOBAL_DESKTOP" ] && GLOBAL_FILES_FOUND+=("$GLOBAL_DESKTOP")

if [ ${#GLOBAL_FILES_FOUND[@]} -gt 0 ]; then
    echo -e "${YELLOW}Found global installation files:${NC}"
    for file in "${GLOBAL_FILES_FOUND[@]}"; do
        echo " - $file"
    done
    echo ""
    
    echo "Note: Removing global files requires root privileges (sudo)."
    if ask_confirm "Do you want to completely remove the GLOBAL installation"; then
        echo -e "${GREEN}-> Removing global files (you might be asked for your password)...${NC}"
        for file in "${GLOBAL_FILES_FOUND[@]}"; do
            sudo rm -f "$file"
            echo "   Deleted: $file"
        done
        GLOBAL_UNINSTALLED=true
        echo -e "${GREEN}-> Global installation successfully removed.${NC}\n"
    else
        echo -e "${YELLOW}-> Skipping global uninstallation.${NC}\n"
    fi
else
    echo -e "No global installation found.\n"
fi

# ---------------------------------------------------------
# STEP 3: Cleanup and Exit
# ---------------------------------------------------------

if [ "$LOCAL_UNINSTALLED" = true ] || [ "$GLOBAL_UNINSTALLED" = true ]; then
    echo -e "${MAG}Running post-uninstall cleanup...${NC}"
    # Update desktop database if the command is available to remove the app from app launchers
    if command -v update-desktop-database &> /dev/null; then
        echo "Updating desktop database..."
        update-desktop-database "$HOME/.local/share/applications" &> /dev/null || true
        sudo update-desktop-database "/usr/share/applications" &> /dev/null || true
    fi
    echo -e "\n${GREEN}Uninstallation complete. sleephistory has been removed.${NC}"
else
    echo -e "\n${YELLOW}No files were removed.${NC}"
fi

echo -e "${MAG}========================================${NC}"