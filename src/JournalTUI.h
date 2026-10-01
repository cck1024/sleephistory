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

#ifndef JOURNALTUI_H
#define JOURNALTUI_H

#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
#include <algorithm> // Fuer max
#include <filesystem>
#include <fstream>
#include <cmath>
#include <mutex>
#include <thread>
#include <atomic>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/canvas.hpp>

#include "JournalBackend.h"

enum class InputMode { NORMAL, VIM };
enum class LayoutMode { LIST_ONLY, HORIZONTAL, VERTICAL, GRAPH_ONLY };

class JournalTUI {
private:
    JournalBackend& backend; // Referenz auf Backend
    mutex data_mutex; // CRITICAL: Schuetzt current_events und summary

    ViewMode current_view = ViewMode::DAY;
    ViewMode active_time_mode = ViewMode::DAY; // Merkt sich, ob Day oder Week aktiv war
    InputMode input_mode = InputMode::NORMAL;
    LayoutMode current_layout = LayoutMode::LIST_ONLY;

    string current_date;
    unsigned int current_session = 0;
    string command_buffer = "";
    string status_message = "";
    vector<unsigned int> included_sessions; // not working perfectly (e.g. when jumping)

    // help screen
    bool show_help_screen = false;
    int help_scroll_y = 0;
    ftxui::Element RenderHelpScreen();

    // Scroll-Zustand (repraesentiert jetzt den Index des fokussierten Events)
    int scroll_y = 0;

    // Geladene Daten fuer die aktuelle Ansicht
    vector<JEvent> current_events;
    TimeSummary current_summary;


    void copyToClipboard(const string& text);
    string saveDayToFile(const string& date, const vector<JEvent>& events, string filename = "", const string& boot_id = "");

    void ReloadData(bool scrollreset = true);
    void navigate(int delta);
    ftxui::Color colorForEvent(EventType t);
    int getGraphHeight(EventType t);

    ftxui::Element RenderGraph(int width_chars, int height_chars);
    ftxui::Element RenderHeader();
    ftxui::Element RenderEvents();
    ftxui::Element RenderFooter();
    void ExecuteVimCommand(ftxui::ScreenInteractive& screen);

public:
    explicit JournalTUI(JournalBackend& b);
    void Run();
};

#endif // JOURNALTUI_H