===============================================================================
                  SLEEPHISTORY - QUICKSTART GUIDE
===============================================================================

Welcome! This guide will help you get 'sleephistory' running on your computer,
even if you have never used a terminal program before.

-------------------------------------------------------------------------------
1. WHAT ARE THESE FILES? (The Basics)
-------------------------------------------------------------------------------

Here is a simple map of what is inside this folder:

*  installer.sh     -> A helper that puts everything in the right place so
                       you can find the app in your Start Menu.
*  uninstaller.sh   -> A helper to completely remove the program from your system.
*  [src]            -> The source code. Humans read this, computers
                       convert it into the actual program.
*  [native-build]   -> A folder where the program is placed after you build it.
*  native-build.sh  -> A script that turns the "recipe" into the actual program.
*  [assets]         -> Contains the icon and the shortcut for your app menu.
*  CMakeLists.txt   -> Technical instructions for the computer.
*  README.md        -> A main guide explaining what the project is, how to use it and how it works.
-------------------------------------------------------------------------------
2. HOW TO TRY IT WITHOUT INSTALLING ("Test Drive")
-------------------------------------------------------------------------------

If you just want to see if it works without making any permanent changes:

1. Open your Terminal in this folder.
2. If you see a file named 'sleephistory' here, type these two commands:

   chmod +x sleephistory
   ./sleephistory

3. If the file is missing, you must build it first (see Step 3 below).

-------------------------------------------------------------------------------
3. HOW TO INSTALL (Recommended)
-------------------------------------------------------------------------------

You have two main choices to get the program ready:

A) THE BEST WAY (Build it yourself):
   This ensures the program fits your computer perfectly.
   1. Right-click in this folder and "Open in Terminal".
   2. Type: ./installer.sh
   3. Choose Option 1 ("Run native-build.sh").
   4. Follow the on-screen instructions.

B) THE FAST WAY (Use the pre-made file):
   Use this if you don't want to wait for building.
   1. Ensure a file named 'sleephistory' is in this folder.
   2. Run: ./installer.sh
   3. Choose Option 3 ("Use precompiled binary").

-------------------------------------------------------------------------------
4. WHICH INSTALLATION TYPE SHOULD I CHOOSE?
-------------------------------------------------------------------------------

When the installer asks you, you have two options:

*  LOCAL (Option 2): Best for beginners. It doesn't ask for a password and
   only installs it for YOU.
*  GLOBAL (Option 1): Installs it for everyone using this PC. You will need
   to enter your system password (sudo).

-------------------------------------------------------------------------------
5. HOW DO I START THE APP LATER?
-------------------------------------------------------------------------------

After a successful installation:
1. Open your Application Launcher (press the "Super/Windows" key).
2. Search for "SleepHistory".
3. Click the icon to launch!

-------------------------------------------------------------------------------
6. HOW TO REMOVE EVERYTHING
-------------------------------------------------------------------------------

If you don't want the program anymore, just run:
./uninstaller.sh

Follow the prompts to clean up all files from your system.

NOTE ON DATA:
Even after using the uninstaller, a small data folder remains at:
~/.cache/sleephistory/

This folder is an "essential index." It stores processed data to make the
app much faster and allows it to keep track of logs even when they are
rotated by the system. The uninstaller does NOT delete this to prevent
accidental loss of your history.

If you are sure you will NEVER use SleepHistory again and want to wipe
everything, you can manually delete that folder.
===============================================================================