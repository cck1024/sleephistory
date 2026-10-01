#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

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
echo -e "${MAG}    sleephistory Installer Started      ${NC}"
echo -e "${MAG}========================================${NC}\n"

# ---------------------------------------------------------
# STEP 1: Select and prepare the application binary
# ---------------------------------------------------------
echo -e "${MAG}[1/3] Preparing the application file (Binary)${NC}"
echo "It is highly recommended to compile the program natively on your system,"
echo "as this generally provides better performance and system compatibility."
echo ""
echo "Please choose how you want to proceed:"
echo "  1) Run native-build.sh and create a fresh build (Recommended)"
echo "  2) Use existing native build (./native-build/sleephistory)"
echo "  3) Use precompiled binary (./sleephistory)"
read -p "Your choice (1/2/3): " build_choice

SELECTED_BIN=""

case "$build_choice" in
    1)
        if ask_confirm "I will now run './native-build.sh'. Do you agree"; then
            echo -e "${GREEN}-> Running native-build.sh...${NC}"
            if [ -f "./native-build.sh" ]; then
                bash ./native-build.sh
                SELECTED_BIN="./native-build/sleephistory"
            else
                echo -e "${RED}Error: native-build.sh not found!${NC}"
                exit 1
            fi
        else
            echo -e "${RED}Aborted by user.${NC}"
            exit 1
        fi
        ;;
    2)
        SELECTED_BIN="./native-build/sleephistory"
        echo -e "${GREEN}-> Using existing native build.${NC}"
        ;;
    3)
        SELECTED_BIN="./sleephistory"
        echo -e "${GREEN}-> Using precompiled binary.${NC}"
        ;;
    *)
        echo -e "${RED}Invalid input. Aborting.${NC}"
        exit 1
        ;;
esac

# Check if the selected file exists and make it executable
if [ ! -f "$SELECTED_BIN" ]; then
    echo -e "${RED}Error: The file '$SELECTED_BIN' does not exist. Please ensure it is present.${NC}"
    exit 1
fi

if ask_confirm "I will now grant execution rights (chmod +x) to '$SELECTED_BIN'. Do you agree"; then
    chmod +x "$SELECTED_BIN"
    echo -e "${GREEN}-> Execution rights successfully granted.${NC}\n"
else
    echo -e "${RED}The application cannot be installed without execution rights. Aborting.${NC}"
    exit 1
fi

# ---------------------------------------------------------
# STEP 2: Verify assets
# ---------------------------------------------------------
echo -e "${MAG}[2/3] Checking assets (Icon & Desktop entry)${NC}"

ASSET_DIR="./assets"
FILE_ICON="$ASSET_DIR/sleephistory.png"
FILE_DESKTOP="$ASSET_DIR/sleephistory.desktop"

echo "Checking if the required files exist in the '$ASSET_DIR' folder..."

if [ ! -f "$FILE_ICON" ] || [ ! -f "$FILE_DESKTOP" ]; then
    echo -e "${RED}Error: Missing files in the '$ASSET_DIR' folder!${NC}"
    echo "Make sure that the following files exist:"
    echo " - $FILE_ICON"
    echo " - $FILE_DESKTOP"
    echo -e "${RED}Installation aborted.${NC}"
    exit 1
else
    echo -e "${GREEN}-> All assets found successfully.${NC}\n"
fi

# ---------------------------------------------------------
# STEP 3: Choose installation target scope
# ---------------------------------------------------------
echo -e "${MAG}[3/3] Installation${NC}"
echo "Should the program be installed for ALL users on this system (requires root privileges)"
echo "or only LOCALLY for the current user?"
echo "  1) All users (Global)"
echo "  2) Only for me (Local)"
read -p "Your choice (1/2): " install_choice

if [ "$install_choice" == "1" ]; then
    # Global Installation
    echo -e "\n${YELLOW}You selected global installation.${NC}"
    echo "The following actions will take place (Root password may be requested):"
    echo "1. Copy '$SELECTED_BIN' to '/usr/local/bin/'"
    echo "2. Copy '$FILE_ICON' to '/usr/share/pixmaps/'"
    echo "3. Copy '$FILE_DESKTOP' to '/usr/share/applications/'"
    
    if ask_confirm "Should the installation be carried out now"; then
        echo -e "${GREEN}-> Copying files...${NC}"
        sudo cp "$SELECTED_BIN" /usr/local/bin/sleephistory
        sudo cp "$FILE_ICON" /usr/share/pixmaps/sleephistory.png
        sudo cp "$FILE_DESKTOP" /usr/share/applications/sleephistory.desktop
        echo -e "${GREEN}-> Global installation completed successfully!${NC}"
    else
        echo -e "${RED}Installation aborted.${NC}"
        exit 1
    fi

elif [ "$install_choice" == "2" ]; then
    # Local Installation
    LOCAL_BIN="$HOME/.local/bin"
    LOCAL_ICON="$HOME/.local/share/icons"
    LOCAL_DESKTOP="$HOME/.local/share/applications"

    echo -e "\n${YELLOW}You selected local installation.${NC}"
    echo "The following actions will take place:"
    echo "1. Ensure local target directories exist."
    echo "2. Copy '$SELECTED_BIN' to '$LOCAL_BIN/'"
    echo "3. Copy '$FILE_ICON' to '$LOCAL_ICON/'"
    echo "4. Copy '$FILE_DESKTOP' to '$LOCAL_DESKTOP/'"

    if ask_confirm "Should the installation be carried out now"; then
        echo -e "${GREEN}-> Creating target directories (if not existing)...${NC}"
        mkdir -p "$LOCAL_BIN" "$LOCAL_ICON" "$LOCAL_DESKTOP"

        echo -e "${GREEN}-> Copying files...${NC}"
        cp "$SELECTED_BIN" "$LOCAL_BIN/sleephistory"
        cp "$FILE_ICON" "$LOCAL_ICON/sleephistory.png"
        cp "$FILE_DESKTOP" "$LOCAL_DESKTOP/sleephistory.desktop"
        
        # Update desktop database if the command is available
        if command -v update-desktop-database &> /dev/null; then
            update-desktop-database "$HOME/.local/share/applications" &> /dev/null || true
        fi

        echo -e "${GREEN}-> Local installation completed successfully!${NC}"
        echo -e "${YELLOW}Note:${NC} Make sure that '$LOCAL_BIN' is included in your \$PATH variable if you wish to launch the application from anywhere in the terminal."
    else
        echo -e "${RED}Installation aborted.${NC}"
        exit 1
    fi
else
    echo -e "${RED}Invalid input. Aborting.${NC}"
    exit 1
fi

echo -e "\n${MAG}Thank you for using sleephistory!${NC}"