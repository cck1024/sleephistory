/*
SleepHistory - Monitor computer usage
Copyright (C) 2026 by cck1024

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstdint>
#include <iostream>
#include <vector>
#include <chrono>
//#include <format>
#include <fmt/core.h>
#include <array>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>

#include "debugging.h"
#include "Help.h"
#include "JournalTUI.h"
#include "JournalCLI.h"
#include "timehelpers.h"

using namespace std;

int main(int argc, char** argv) {
    try {
        string command_arg = "";
        bool color_set_manually = false;

        // --- FARBMODUS ERKENNEN ---
        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "-c0") { color_mode = ColorMode::MONO; color_set_manually = true; LOG_DEBUG("color mode manually set to MONO"); }
            else if (arg == "-c1") { color_mode = ColorMode::BASIC; color_set_manually = true; LOG_DEBUG("color mode manually set to BASIC"); }
            else if (arg == "-c2") { color_mode = ColorMode::RICH; color_set_manually = true; LOG_DEBUG("color mode manually set to RICH"); }
        }

        if (!color_set_manually) {
            auto_detect_color_mode();
        }

        // Farben initialisieren
        init_cli_Colors();

        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];

            // Farbmodi ignorieren
            if (arg == "-c0" || arg == "-c1" || arg == "-c2") continue;

            if (arg == "--version"||arg == "-v") {
                cout << get_version_text() << endl;
                LOG_DEBUG("sleephistory version shown, exiting normally.");
                return 0;
            }
            if (arg == "--help"||arg == "-h") {
                // cout << "run without arguments to start TUI mode" << endl;
                // cout << "run with DD/MM/YYYY or session id for CLI output" << endl;
                // cout << "--version for appversion" << endl;
                cout << get_help_text() << endl;
                LOG_DEBUG("sleephistory help page shown, exiting normally.");
                return 0;
            }
            // Wenn es weder --version noch --help ist, speichern wir es als Befehl
            if (!command_arg.empty()) {
                command_arg += " "; // Leerzeichen zwischen den Argumenten einfuegen
            }
            command_arg += arg;

            // } else {
            //     cerr << "Too many arguments. Run with --help." << endl;
            //     return 1;
            // }
        }

        int sysD_version = get_systemd_version();
        if (sysD_version==-1) {
            if (execCommand("which journalctl").empty()) {
                throw runtime_error(
                    "CRITICAL ERROR: 'journalctl' not found!\n"
                    "This tool requires systemd-journald to be installed and accessible in your PATH."
                );
            } else {
                cerr << ANSI_RED << "ERROR: 'journalctl' version could not be identified. Version "<<MIN_SYSTEMD_VERSION<<" or higher is required. Trying to start regardless...\n";
                this_thread::sleep_for(chrono::seconds(1));
            }
        } else if (sysD_version<MIN_SYSTEMD_VERSION) {
            cerr << ANSI_RED << "ERROR: 'journalctl' has version " << sysD_version << " but version "<<MIN_SYSTEMD_VERSION<<" or higher is required. Trying to start regardless...\n";
            this_thread::sleep_for(chrono::seconds(1));
        }

        if (!hasJournalPermissions()) {
            cerr << ANSI_RED << " ERROR: Insufficient permissions to read system journal.\n"
                      << ANSI_RESET << " ------------------------------------------------------\n"
                      << " To use sleephistory, your user must be in the 'systemd-journal' or 'adm' group.\n\n"
                      << " Run the following command to add your user:\n"
                      << ANSI_BOLD << "   sudo usermod -aG systemd-journal $USER\n\n" << ANSI_RESET
                      << " After that, please log out and log back in for the changes to take effect.\n\n"
                      << " TIP: If you don't want to do so, you can temporarily run sleephistory as root:\n"
                      << ANSI_BOLD << "   sudo sleephistory\n\n";
            return 1;
        }

        JournalBackend backend;

        // CLI or TUI?
        if (!command_arg.empty()) {
            // CLI Mode (Druckt einmal und beendet sich)
            LOG_DEBUG("starting sleephistory in CLI Mode.");
            JournalCLI::Run(backend, command_arg);
        } else {
            // TUI Mode
            // check if sleephistory was started properly. if not -> no silent running but instant exit with error message in gui.
            if (!isatty(STDIN_FILENO)) {
                LOG_DEBUG("starting sleephistory in TUI Mode FAILED.\nWE HAVE NO TERMINAL.\nstarted with gui file manager?");

                int res = std::system("kdialog --error 'sleephistory has no GUI.\\nPlease start it using the terminal or desktop shortcut.' --title 'Error' 2>/dev/null");
                if (res != 0) { // fallback with zenity
                    int res_fallback = std::system("zenity --error --title='Error' --text='sleephistory has no GUI.\\nPlease start it using the terminal or desktop shortcut.' 2>/dev/null &");
                }
                return 1;
            }
            LOG_DEBUG("starting sleephistory in TUI Mode.");
            //cout << "Loading TUI. Please wait...\n";
            JournalTUI tui(backend);
            tui.Run();
        }
    }
    catch (const runtime_error& e) {
        cerr << "\033[1;31m" << e.what() << "\033[0m" << endl;
        return 1;
    }
    catch (const exception& e) {
        cerr << "An unexpected error occurred: " << e.what() << endl;
        return 1;
    }

    return 0;
}