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

using namespace std;
#include <ftxui/screen/color.hpp>

#include "Colors.h"
#include "debugging.h"

string ANSI_CYAN, ANSI_BLUE, ANSI_PINK, ANSI_RESET, ANSI_BOLD, ANSI_DIM, ANSI_GREEN, ANSI_SPRINGGREEN3, ANSI_ORANGE, ANSI_RED, ANSI_SKYBLUE, ANSI_HOTPINK, ANSI_DEEPPINK;

ColorMode color_mode;

void auto_detect_color_mode() {
    const char* term_env = getenv("TERM");
    const char* colorterm_env = getenv("COLORTERM");

    // 1. Wenn kein Terminal erkannt wird oder "dumb", dann MONO
    if (!term_env || string(term_env) == "dumb") {
        color_mode = ColorMode::MONO;
        LOG_DEBUG("color mode automatically set to MONO");
        return;
    }

    // 2. Suche nach Truecolor oder 256color Hinweisen
    string term = term_env;
    if (colorterm_env && (string(colorterm_env) == "truecolor" || string(colorterm_env) == "24bit")) {
        color_mode = ColorMode::RICH;
        LOG_DEBUG("color mode automatically set to RICH");
    } else if (term.find("256color") != string::npos) {
        color_mode = ColorMode::RICH;
        LOG_DEBUG("color mode automatically set to RICH");
    } else {
        // 3. Fallback auf Standard 8 Farben
        color_mode = ColorMode::BASIC;
        LOG_DEBUG("color mode automatically set to BASIC");
    }
}

void init_cli_Colors() {
    if (color_mode == ColorMode::RICH) {
        ANSI_RESET  = "\033[0m";
        ANSI_BOLD   = "\033[1m";
        ANSI_DIM    = "\033[2m";
        ANSI_CYAN = "\033[36m";
        ANSI_BLUE = "\033[94m";       // SkyBlue1 (Light Blue)
        ANSI_PINK = "\033[95m";       // HotPink / DeepPink (Light Magenta)
        ANSI_GREEN        = "\033[38;5;34m";  // Entspricht Green
        ANSI_SPRINGGREEN3 = "\033[38;5;35m";  // Entspricht SpringGreen3
        ANSI_ORANGE       = "\033[38;5;172m"; // Entspricht Orange3
        ANSI_RED          = "\033[38;5;196m"; // Entspricht Red
        ANSI_SKYBLUE      = "\033[38;5;117m"; // Entspricht SkyBlue1
        ANSI_HOTPINK      = "\033[38;5;205m"; // Entspricht HotPink
        ANSI_DEEPPINK     = "\033[38;5;199m"; // Entspricht DeepPink1

    } else if (color_mode == ColorMode::BASIC) {
        ANSI_RESET  = "\033[0m";
        ANSI_BOLD   = "\033[1m";
        ANSI_DIM    = "\033[2m";
        ANSI_CYAN   = "\033[36m";  // Standard Cyan
        ANSI_BLUE   = "\033[34m";  // Standard Blue
        ANSI_PINK   = "\033[35m";  // Standard Magenta
        ANSI_GREEN  = "\033[32m";  // Standard Green
        ANSI_SPRINGGREEN3 = "\033[92m"; // Bright Green
        ANSI_ORANGE       = "\033[33m"; // Yellow (nahe an Orange)
        ANSI_RED          = "\033[31m"; // Standard Red
        ANSI_SKYBLUE      = "\033[96m"; // Bright Cyan (wirkt wie Hellblau)
        ANSI_HOTPINK      = "\033[95m"; // Bright Magenta
        ANSI_DEEPPINK     = "\033[35m"; // Magenta
    }
    // MONO: Farbstrings bleiben LEER.
    else {
        ANSI_CYAN = ANSI_BLUE = ANSI_PINK = ANSI_RESET = ANSI_BOLD =
        ANSI_DIM = ANSI_GREEN = ANSI_SPRINGGREEN3 = ANSI_ORANGE =
        ANSI_RED = ANSI_SKYBLUE = ANSI_HOTPINK = ANSI_DEEPPINK = "";
    }
}

// Hilfsfunktion für FTXUI Farben
ftxui::Color get_tui_color(ftxui::Color rich_color, ftxui::Color basic_color) {
    if (color_mode == ColorMode::RICH) return rich_color;
    if (color_mode == ColorMode::BASIC) return basic_color;
    return ftxui::Color::Default; // MONO: Terminal Standardvorgabe
}

