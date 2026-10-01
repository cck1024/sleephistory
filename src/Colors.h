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

#ifndef SLEEPHISTORY_COLORS_H
#define SLEEPHISTORY_COLORS_H
#include "ftxui/screen/color.hpp"

extern std::string ANSI_CYAN, ANSI_BLUE, ANSI_PINK, ANSI_RESET, ANSI_BOLD, ANSI_DIM, ANSI_GREEN,
ANSI_SPRINGGREEN3, ANSI_ORANGE, ANSI_RED, ANSI_SKYBLUE, ANSI_HOTPINK, ANSI_DEEPPINK;

enum class ColorMode {
    MONO = 0,   // -c0
    BASIC = 1,  // -c1 (8 Farben)
    RICH = 2    // -c2 (256 Farben)
};

// Globaler Status (initialisiert in main)
extern ColorMode color_mode;

void auto_detect_color_mode();
void init_cli_Colors();
// Hilfsfunktion für FTXUI Farben
ftxui::Color get_tui_color(ftxui::Color rich_color, ftxui::Color basic_color);

#endif //SLEEPHISTORY_COLORS_H
