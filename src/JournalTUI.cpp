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

#include "JournalTUI.h"
#include "Colors.h"
#include "Help.h"


void JournalTUI::ReloadData(bool scrollreset) { // default = true
    bool future_blocked = false;
    if (current_view == ViewMode::DAY) {
        future_blocked = isFuture(current_date);
    } else if (current_view == ViewMode::WEEK) {
        future_blocked = isFuture(get_monday_of_week(current_date));
    }

    if (future_blocked) {
        current_events.clear();
        included_sessions.clear();
        current_summary = TimeSummary();
        return;
    }

    vector<unsigned int> new_sessions;
    vector<JEvent> new_events;

    if (current_view == ViewMode::DAY) {
        new_events = backend.parseDay(current_date, new_sessions);
    } else if (current_view == ViewMode::WEEK) {
        new_events = backend.parseWeek(current_date, new_sessions);
    } else {
        new_events = backend.parseSession(current_session).getEvents();
    }

    prepareEvents(current_view, current_date, current_session, new_events);
    TimeSummary new_summary = Analyzer::calculateStats(new_events);

    // Zuweisung erst nach dem Parsen (minimiert Lock-Zeit)
    {
        lock_guard<mutex> lock(data_mutex);
        if (scrollreset) scroll_y = 0;
        current_events = move(new_events);
        included_sessions = move(new_sessions);
        current_summary = move(new_summary);
    }
}

void JournalTUI::copyToClipboard(const string &text) {
    // Fire and forget Thread
    thread([text]() {
        FILE *pipe = popen("xclip -selection clipboard", "w");
        if (pipe) {
            fprintf(pipe, "%s", text.c_str());
            pclose(pipe);
        }
    }).detach();
}

string JournalTUI::saveDayToFile(const string &date, const vector<JEvent> &events,
                                      string filename, const string& boot_id ) { // default filename = ""
    try {
        // 1. Standardname generieren
        if (filename.empty()) {
            string clean_date = "export";
            if (!events.empty()) {
                // Nutze 'events' statt 'current_events' fuer Unabhaengigkeit
                if (current_view == ViewMode::DAY) {
                    clean_date = to_journal_date(current_date);
                } else {
                    clean_date = usToYYYY_timestamp(events.front().timestamp_us) +
                                 "_to_" + usToYYYY_timestamp(events.back().timestamp_us);
                }
            }
            replace(clean_date.begin(), clean_date.end(), ' ', '_');
            replace(clean_date.begin(), clean_date.end(), ':', '-'); // : ist in Pfaden boese
            filename = "sleephistory_" + clean_date + ".md";
        }

        filesystem::path p(filename);

        // 2. Pruefen, ob Verzeichnis existiert / Schreibrechte da sind
        // exists() kann werfen, wenn der uebergeordnete Ordner nicht lesbar ist
        if (filesystem::exists(p)) {
            return "Error: File already exists";
        }

        // 3. Schreiben
        ofstream file(filename);
        if (!file) {
            return "Error: Permission denied or invalid path";
        }

        file << convertToMarkdown(events, current_view, current_date, included_sessions, current_summary, boot_id);
        file.close();

        // Nur den Dateinamen zurueckgeben, nicht den vollen Pfad,
        // falls dieser extrem lang ist (fuer die TUI-Statuszeile)
        return "Saved to " + p.filename().string();
    } catch (const filesystem::filesystem_error &e) {
        // Faengt Zugriffsfehler auf Verzeichnisse ab (wie /root/)
        return "Error: System denied access";
    } catch (const exception &e) {
        return "Error: " + string(e.what());
    }
}

void JournalTUI::navigate(int delta) {
    if (current_view == ViewMode::DAY) {
        current_date = addDaysToDate(current_date, delta);
    } else if (current_view == ViewMode::WEEK) {
        current_date = addDaysToDate(current_date, delta * 7); // Wir springen eine ganze Woche
    } else {
        if (delta > 0 && current_session > 0) current_session -= 1;
        else if (delta < 0) current_session += 1;
    }
    ReloadData();
}

ftxui::Color JournalTUI::colorForEvent(EventType t) {
    using namespace ftxui;
    switch (t) {
        case POWERED_ON:
        case END_OF_DAY:
        case END_OF_WEEK:
        case PRESENT_TIME:
        case CONTINUED_ON:
            return get_tui_color(Color::Green, Color::Green);
        case POWERED_OFF:
        case CONTINUED_OFF:
            return get_tui_color(Color::Orange3, Color::Yellow);
        case MAGIC_SYSRQ:
        case CONTINUED_SYSRQ:
            return get_tui_color(Color::DeepPink1, Color::Magenta);
        case HARD_SHUTDOWN:
        case CONTINUED_HARD_SHUTDOWN:
            return get_tui_color(Color::Red, Color::Red);
        case SLEEP_START:
        case CONTINUED_SLEEP:
            return get_tui_color(Color::SkyBlue1, Color::Cyan);
        case HIBERNATE_START:
        case CONTINUED_HIBERNATE:
            return get_tui_color(Color::HotPink, Color::Magenta);
        case SLEEP_STOP:
        case HIBERNATE_STOP:
            return get_tui_color(Color::SpringGreen3, Color::Green); // ACTIVE
        default: return Color::Default;
    }
}

int JournalTUI::getGraphHeight(EventType t) {
    // Hoehere Zahl = weiter oben im Graphen
    switch (t) {
        case POWERED_ON:
        case SLEEP_STOP:
        case HIBERNATE_STOP:
        case CONTINUED_ON:
        case PRESENT_TIME:
        case END_OF_DAY:
        case END_OF_WEEK:
            return 4; // Ganz oben (Aktiv)
        case SLEEP_START:
        case CONTINUED_SLEEP:
            return 3; // Leicht abgesenkt (Sleep)
        case HIBERNATE_START:
        case CONTINUED_HIBERNATE:
            return 2; // Tiefer (Hibernate)
        case POWERED_OFF:
        case CONTINUED_OFF:
        case HARD_SHUTDOWN:
        case CONTINUED_HARD_SHUTDOWN:
        case MAGIC_SYSRQ:
        case CONTINUED_SYSRQ:
            return 1; // Ganz unten (Aus/Crash)
        default: return 0;
    }
}

ftxui::Element JournalTUI::RenderGraph(int width_chars, int height_chars) {
    using namespace ftxui;
    lock_guard<mutex> lock(data_mutex);

    bool is_week_future = (current_view == ViewMode::WEEK && isFuture(get_monday_of_week(current_date)));
    bool is_day_future = (current_view == ViewMode::DAY && isFuture(current_date));

    if (is_day_future || is_week_future) {
        return text("Date is in the future. No graph available.") | center | color(
                   get_tui_color(Color::Red, Color::Red));
    } else if (current_events.empty()) {
        return text("No data available for graph.") | center | dim;
    }

    uint64_t start_time = current_events.front().timestamp_us;
    uint64_t end_time = current_events.back().timestamp_us;
    // Wenn current_events nur ein Element hat oder Start==Ende
    if (current_events.size() < 2 || start_time >= end_time) {
        return text("Not enough events to draw a graph.") | center | dim;
    }

    uint64_t duration = end_time - start_time;

    // Exakte Canvas Berechnung
    // Wir ziehen 2 Zeilen ab, da der Separator und die Achsenbeschriftung
    // unten genau 2 Zeilen Platz einnehmen.
    int canvas_lines = max(1, height_chars - 2);
    int canvas_cols = max(2, width_chars);

    // Breite: Zellen * 2, Hoehe: verbleibende Zeilen * 4
    int canvas_width = canvas_cols * 2;
    int canvas_height = canvas_lines * 4;

    auto c = Canvas(canvas_width, canvas_height);

    // Zeichnen
    for (size_t i = 0; i < current_events.size() - 1; ++i) {
        const auto &ev = current_events[i];
        const auto &next_ev = current_events[i + 1];

        // Relative Positionen (0.0 bis 1.0)
        double x1_rel = static_cast<double>(ev.timestamp_us - start_time) / duration;
        double x2_rel = static_cast<double>(next_ev.timestamp_us - start_time) / duration;

        // In Canvas-Pixel umrechnen
        int x1 = round(x1_rel * (canvas_width - 1));
        int x2 = round(x2_rel * (canvas_width - 1));

        auto getY = [&](EventType t) {
            int level = getGraphHeight(t); // 1-4
            return (4 - level) * (canvas_height - 1) / 3;
        };

        // Da ev nie das letzte Element ist, ist es garantiert kein Marker
        int y = getY(ev.type);

        int next_y;
        EventType next_color_type;

        // Effizienter Check: Ist das naechste Event der Abschlussmarker?
        if (next_ev.type == END_OF_DAY || next_ev.type == END_OF_WEEK) {
            next_y = y; // Halte die Hoehe des aktuellen Events
            next_color_type = ev.type; // Halte die Farbe
        } else {
            next_y = getY(next_ev.type);
            next_color_type = next_ev.type;
        }

        // Horizontale Linie zeichnen
        c.DrawBlockLine(x1, y, x2, y, colorForEvent(ev.type));

        // Vertikale Verbindung bei Statuswechsel
        // (Wird im letzten Durchlauf bei einem Marker automatisch uebersprungen, da y == next_y)
        if (y != next_y) {
            c.DrawPointLine(x2, y, x2, next_y, colorForEvent(next_color_type));
        }
    }

    // Achsenbeschriftung (dynamisch nach Breite)
    bool is_short_duration = (duration < 86400000000ULL);

    // Generiere ein Muster-Label, um die Laenge abzuschaetzen
    string sample_label = beautiful_timestamp(start_time);
    if (is_short_duration) {
        sample_label = sample_label.substr(0, 8);
    }
    int label_length = sample_label.length();

    // Wie viel Platz braucht ein Label mindestens? (Label-Laenge + etwas Puffer fuer Lesbarkeit)
    int min_spacing = 6; // Mindestens 6 Leerzeichen zwischen den Uhrzeiten
    int space_per_label = label_length + min_spacing;

    // Berechne, wie viele Labels in die Breite passen. Mindestens 2 (Start und Ende).
    int num_labels = max(2, width_chars / space_per_label);

    // maximale Anzahl, damit es nicht zu unuebersichtlich wird: Alle 2 Stunden markierung
    num_labels = min(num_labels, 13);

    ftxui::Elements label_elements;

    for (int i = 0; i < num_labels; ++i) {
        // Zeitpunkte proportional aufteilen (von 0.0 bis 1.0)
        double fraction = static_cast<double>(i) / (num_labels - 1);
        uint64_t t = start_time + static_cast<uint64_t>(duration * fraction);

        string label_str = beautiful_timestamp(t + 1000000);
        // damit die timestamps schoener (gerader aussehen) (z.B. 12:00:00 statt 11:59:59)
        if (is_short_duration) {
            label_str = label_str.substr(0, 8);
        }

        // Label hinzufuegen
        label_elements.push_back(text(label_str) | dim);

        // Filler zwischen den Elementen hinzufuegen (aber nicht nach dem allerletzten Element)
        if (i < num_labels - 1) {
            label_elements.push_back(filler());
        }
    }

    // Den dynamischen Vector an hbox uebergeben
    auto axis_labels = hbox(move(label_elements)) | size(WIDTH, LESS_THAN, canvas_cols);

    return vbox({
        canvas(move(c)),
        separator() | dim,
        axis_labels
    });
}

ftxui::Element JournalTUI::RenderHeader() {
    using namespace ftxui;
    lock_guard<mutex> lock(data_mutex);
    string sub_info = "";
    if (!current_events.empty()) {
        if (current_view == ViewMode::DAY || current_view == ViewMode::WEEK) {
            sub_info = "Included Sessions: " + printSessions(included_sessions);
        } else {
            sub_info = "Range: " + beautiful_timestamp(current_events.front().timestamp_us) +
                       " to " + beautiful_timestamp(current_events.back().timestamp_us);
        }
    }

    string title = "";
    if (current_view == ViewMode::DAY) {
        title = "DAY VIEW: " + current_date;
    } else if (current_view == ViewMode::WEEK) {
        string mon = get_monday_of_week(current_date);
        string sun = addDaysToDate(mon, 6);
        title = "WEEK VIEW: " + mon + " to " + sun + " (CW " + to_string(get_week_number(mon)) + ")";
    } else {
        string boot_id = backend.getBootId(current_session);
        title = "SESSION VIEW: " + to_string(current_session);
        if (!boot_id.empty()) {
            title += " (" + boot_id + ")";
        }
    }

    uint64_t total = current_summary.active_us + current_summary.sleep_us + current_summary.hibernate_us;

    return vbox({
               hbox({
                   text(title) | bold | color(get_tui_color(Color::Cyan, Color::Cyan)), filler(), text(sub_info) | dim
               }),
               separator(),
               hbox({
                   text("TOTAL: " + us_to_h_m_s(total) + (total > 86400000000 ? (" (" + us_to_d_h(total) + ")") : "")) |
                   bold,
                   separatorEmpty(),
                   text("ACTIVE: " + us_to_h_m_s(current_summary.active_us) + (current_summary.active_us > 86400000000
                                                                                   ? (" (" + us_to_d_h(
                                                                                           current_summary.active_us) +
                                                                                       ")")
                                                                                   : "")) | color(
                       get_tui_color(Color::SpringGreen3, Color::Green)),
                   separatorEmpty(),
                   text("SLEEP: " + us_to_h_m_s(current_summary.sleep_us) + (current_summary.sleep_us > 86400000000
                                                                                 ? (" (" + us_to_d_h(
                                                                                         current_summary.sleep_us) +
                                                                                     ")")
                                                                                 : "")) | color(
                       get_tui_color(Color::SkyBlue1, Color::Cyan)),
                   separatorEmpty(),
                   text("HIBERNATE: " + us_to_h_m_s(current_summary.hibernate_us) + (
                            current_summary.hibernate_us > 86400000000
                                ? (" (" + us_to_d_h(current_summary.hibernate_us) + ")")
                                : "")) | color(get_tui_color(Color::HotPink, Color::Magenta))
               })
           }) | border;
}

ftxui::Element JournalTUI::RenderEvents() {
    using namespace ftxui;
    lock_guard<mutex> lock(data_mutex);
    bool is_week_future = (current_view == ViewMode::WEEK && isFuture(get_monday_of_week(current_date)));
    bool is_day_future = (current_view == ViewMode::DAY && isFuture(current_date));

    if (is_day_future || is_week_future) {
        return text("Date is in the future. No data available.") | center |
               color(get_tui_color(Color::Red, Color::Red));
    }

    if ((current_view == ViewMode::SESSION && current_events.size() <= 1)||(current_view != ViewMode::SESSION && current_events.empty())) {
        return text("No events found for this selection.") | center | dim;
    }

    Elements children;
    for (size_t i = 0; i < current_events.size(); ++i) {
        const auto &ev = current_events[i];

        string label = (ev.message == "Present Time") ? "[CURRENT]" : "[" + eventTypeToString(ev.type) + "]";

        // Event-Block als Variable speichern, damit wir ftxui::focus anwenden koennen
        Element event_element = vbox({
            text(beautiful_timestamp(ev.timestamp_us)) | dim,
            text(label + " " + ev.message) | color(colorForEvent(ev.type)) | bold | dim
        });

        if ((int) i == scroll_y) {
            event_element = event_element | focus;
        }

        children.push_back(event_element);

        if (i < current_events.size() - 1) {
            uint64_t diff = current_events[i + 1].timestamp_us - ev.timestamp_us;
            string status_str = getStatusString(ev.type);
            Color col = colorForEvent(ev.type);

            children.push_back(hbox({
                                   text("   └── " + us_to_h_m_s(diff) + (diff > 86400000000
                                                                             ? (" (" + us_to_d_h(diff) + ") ")
                                                                             : " ")),
                                   text("[ STATUS: " + status_str + " ]") | bold | dim
                               }) | color(col));

            children.push_back(separator() | dim);
        }
    }
    return vbox(move(children));
}

ftxui::Element JournalTUI::RenderFooter() {
    using namespace ftxui;
    if (input_mode == InputMode::VIM) {
        return text(":" + command_buffer) | bold | color(get_tui_color(Color::Yellow, Color::Yellow));
    }
    if (!status_message.empty()) {
        return text(status_message) | color(get_tui_color(Color::GreenLight, Color::Green)) | bold;
    }
    return text(
               "[t] Toggle Session View | [v] Toggle Day/Week | [g] Toggle Graph | [wasd or Arrow keys] Navigate | [:] Command Mode | [:h] Help | [ctrl+c] Quit")
           | dim;
}

void JournalTUI::ExecuteVimCommand(ftxui::ScreenInteractive &screen) {
    transform(command_buffer.begin(), command_buffer.end(),
                   command_buffer.begin(),
                   [](unsigned char c) { return tolower(c); });
    if (command_buffer == "q" || command_buffer == "quit" || command_buffer == "exit") {
        screen.Exit();
    } else if (command_buffer == "t") {
        if (current_view == ViewMode::SESSION) {
            current_view = active_time_mode; // Springt dorthin zurueck, wo wir her kamen
        } else {
            current_view = ViewMode::SESSION;
        }
        ReloadData();
    } else if (command_buffer == "v") {
        if (current_view == ViewMode::SESSION) {
            active_time_mode = (active_time_mode == ViewMode::DAY) ? ViewMode::WEEK : ViewMode::DAY;
            current_view = active_time_mode;
        } else {
            active_time_mode = (current_view == ViewMode::DAY) ? ViewMode::WEEK : ViewMode::DAY;
            current_view = active_time_mode;
        }
        ReloadData();
    }

    regex layout_regex(R"(g(\d+)?)");
    smatch g_match;
    if (regex_match(command_buffer, g_match, layout_regex)) {
        int mode;
        if (g_match[1].matched) {
            // Fall "g0", "g1", "g10" etc.
            mode = stoi(g_match[1].str()) % 4;
        } else {
            // Fall nur "g" -> Inkrementieren
            mode = (static_cast<int>(current_layout) + 1) % 4;
        }
        current_layout = static_cast<LayoutMode>(mode);
        ReloadData();
    }

    if (command_buffer == "c") {
        // Hole die aktuellen Daten
        string boot_id = (current_view == ViewMode::SESSION) ? backend.getBootId(current_session) : "";
        string md = convertToMarkdown(current_events, current_view, current_date, included_sessions,
                                           current_summary, boot_id);
        copyToClipboard(md);
        status_message = "Copied to Clipboard!";
        thread([this, &screen]() {
            this_thread::sleep_for(chrono::seconds(2));
            status_message = ""; // Nachricht loeschen
            screen.PostEvent(ftxui::Event::Custom); // UI sagen: "Bitte neu zeichnen!"
        }).detach();
    }

    if (command_buffer == "h" || command_buffer == "help") {
        show_help_screen = true;
        //help_scroll_y = 0;       // Scroll resetten
        input_mode = InputMode::NORMAL; // Zurueck in Normalmodus fuer w/s/Esc
        command_buffer.clear();
        return;
    }

    // Check fuer :w [filename]
    regex write_regex(R"(w(?:\s+(.+))?)");
    smatch w_match;
    if (regex_match(command_buffer, w_match, write_regex)) {
        string filename = w_match[1].matched ? w_match[1].str() : "";
        string boot_id = (current_view == ViewMode::SESSION) ? backend.getBootId(current_session) : "";
        status_message = saveDayToFile(current_date, current_events, filename, boot_id);
        thread([this, &screen]() {
            this_thread::sleep_for(chrono::seconds(2));
            status_message = ""; // Nachricht loeschen
            screen.PostEvent(ftxui::Event::Custom); // UI sagen: "Bitte neu zeichnen!"
        }).detach();
    }

    // Date or Session parsing:
    if (parseInputString(command_buffer, current_view, current_date, current_session)) {
        ReloadData();
    };

    command_buffer.clear();
    input_mode = InputMode::NORMAL;
}

// Im Konstruktor von JournalTUI
JournalTUI::JournalTUI(JournalBackend& b) : backend(b) {
    current_date = today_Date_h();
    ReloadData();
}

ftxui::Element JournalTUI::RenderHelpScreen() {
    using namespace ftxui;
    Elements lines;

    int total_overhead = 6;
    int max_visible_lines = ftxui::Terminal::Size().dimy - total_overhead;
    if (max_visible_lines < 5) max_visible_lines = 5;

    int start_idx = help_scroll_y;
    int end_idx = std::min((int)tui_help.size(), start_idx + max_visible_lines);

    for (int i = start_idx; i < end_idx; ++i) {
        lines.push_back(text(tui_help[i]));
    }
    lines.push_back(separator());

    std::stringstream ss(get_version_text(true));
    std::string version_line;
    while (std::getline(ss, version_line)) {
        lines.push_back(paragraph(version_line));
    }

    return window(
        text(" HELP (Press Esc to close) "),
        vbox(lines)
    ) | clear_under | flex;
}

void JournalTUI::Run() {
    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();

    atomic<bool> refresh_ui_continue = true;

    // Hintergrund-Thread fuer den 1-Sekunden-Takt
    thread refresh_ui([&] {
        while (refresh_ui_continue) {
            // Check: Muessen wir ueberhaupt live updaten?
            bool is_live = (current_view == ViewMode::DAY && current_date == today_Date_h()) ||
                           (current_view == ViewMode::WEEK && get_monday_of_week(current_date) == get_monday_of_week(
                                today_Date_h())) ||
                           (current_view == ViewMode::SESSION && current_session == 0);

            if (is_live && input_mode == InputMode::NORMAL && !show_help_screen) {
                ReloadData(false); // Daten im Hintergrund laden
                screen.PostEvent(Event::Custom); // Nur DANN neu zeichnen
            }

            this_thread::sleep_for(chrono::seconds(1));
        }
    });

    auto renderer = Renderer([&] {
        if (show_help_screen) {
            return RenderHelpScreen();
        }
        auto term = Terminal::Size();

        // Puffer fuer Header, Footer und globale Rahmen
        int available_height = max(2, term.dimy - 8);
        // Puffer fuer Seitenraender
        int available_width = max(10, term.dimx - 2);

        int g_width = available_width;
        int g_height = available_height;

        // Layout strikt aufteilen (Harte Grenzen berechnen)
        if (current_layout == LayoutMode::HORIZONTAL) {
            g_height = max(1, (available_height - 1) / 2); // -1 fuer mittleren Separator
        } else if (current_layout == LayoutMode::VERTICAL) {
            g_width = max(5, (available_width - 1) / 2); // -1 fuer mittleren Separator
        } else if (current_layout == LayoutMode::LIST_ONLY) {
            g_height = 0;
        }

        // Elemente generieren
        Element list_view;
        Element graph_view;
        Element middle_section;

        if (current_layout != LayoutMode::GRAPH_ONLY) {
            list_view = RenderEvents() | vscroll_indicator | yflex | yframe;

            // Harte Grenzen (size) setzen, damit
            // eine breite/lange Liste niemals den Graphen wegdrueckt!
            if (current_layout == LayoutMode::HORIZONTAL) {
                list_view = list_view | size(HEIGHT, EQUAL, g_height);
            } else if (current_layout == LayoutMode::VERTICAL) {
                list_view = list_view | size(WIDTH, EQUAL, g_width);
            }
        }

        if (current_layout != LayoutMode::LIST_ONLY) {
            graph_view = (g_height > 0) ? RenderGraph(g_width, g_height) : text("");

            // Analog harte Grenzen fuer den Graph-Container
            if (current_layout == LayoutMode::HORIZONTAL) {
                graph_view = graph_view | size(HEIGHT, EQUAL, g_height);
            } else if (current_layout == LayoutMode::VERTICAL) {
                graph_view = graph_view | size(WIDTH, EQUAL, g_width);
            }
        }

        switch (current_layout) {
            case LayoutMode::LIST_ONLY: middle_section = list_view;
                break;
            case LayoutMode::GRAPH_ONLY: middle_section = graph_view | flex;
                break;
            case LayoutMode::HORIZONTAL:
                // flex wird hier unterstuetzt durch die vorigen size-Constraints!
                middle_section = vbox({list_view, separator(), graph_view});
                break;
            case LayoutMode::VERTICAL:
                middle_section = hbox({list_view, separator(), graph_view});
                break;
        }

        return vbox({
            RenderHeader(),
            separator(),
            middle_section | flex,
            separator(),
            RenderFooter()
        });
    });

    auto component = CatchEvent(renderer, [&](ftxui::Event event) {
        // --- Input Handling, wenn das Help-Menue offen ist ---
        if (show_help_screen) {
            if (event == ftxui::Event::Escape) {
                show_help_screen = false;
                return true; // true = UI neu rendern
            }
            if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('w')) {
                if (help_scroll_y > 0) help_scroll_y--;
                return true;
            }
            if (event == ftxui::Event::ArrowDown || event == ftxui::Event::Character('s')) {
                // Begrenze das Scrollen nach unten, damit man nicht ins Nichts scrollt
                int max_visible = ftxui::Terminal::Size().dimy - 6;
                int max_scroll = std::max(0, (int)tui_help.size() - max_visible);
                if (help_scroll_y < max_scroll) help_scroll_y++;
                return true;
            }
            if (event.is_mouse()) {
                if (event.mouse().button == ftxui::Mouse::WheelUp) {
                    if (help_scroll_y > 0) help_scroll_y--;
                    return true;
                }
                if (event.mouse().button == ftxui::Mouse::WheelDown) {
                    int max_visible = ftxui::Terminal::Size().dimy - 6;
                    int max_scroll = std::max(0, (int)tui_help.size() - max_visible);
                    if (help_scroll_y < max_scroll) help_scroll_y++;
                    return true;
                }
            }

            // WICHTIG: Alle anderen Inputs verschlucken wir hier, damit sich im Hintergrund
            // nicht der Tag aendert, waehrend man die Hilfe liest
            return true;
        }
        if (input_mode == InputMode::VIM) {
            if (event == ftxui::Event::Return) {
                ExecuteVimCommand(screen);
                return true;
            }
            if (event == ftxui::Event::Escape) {
                input_mode = InputMode::NORMAL;
                command_buffer.clear();
                return true;
            }
            if (event == ftxui::Event::Backspace && !command_buffer.empty()) {
                command_buffer.pop_back();
                return true;
            }
            if (event.is_character()) {
                command_buffer += event.character();
                return true;
            }
            return false;
        }

        if (event.is_character() && event.character() == ":") {
            input_mode = InputMode::VIM;
            return true;
        }
        if (event.is_character() && event.character() == "v") {
            if (current_view == ViewMode::SESSION) {
                active_time_mode = (active_time_mode == ViewMode::DAY) ? ViewMode::WEEK : ViewMode::DAY;
                current_view = active_time_mode;
            } else {
                active_time_mode = (current_view == ViewMode::DAY) ? ViewMode::WEEK : ViewMode::DAY;
                current_view = active_time_mode;
            }
            ReloadData();
            return true;
        }
        if (event.is_character() && event.character() == "t") {
            if (current_view == ViewMode::SESSION) {
                current_view = active_time_mode; // Springt dorthin zurueck, wo wir her kamen
            } else {
                current_view = ViewMode::SESSION;
            }
            ReloadData();
            return true;
        }
        if (event.is_character() && event.character() == "g") {
            // Rotiere durch die 4 Layout-Modi
            int mode = static_cast<int>(current_layout);
            mode = (mode + 1) % 4;
            current_layout = static_cast<LayoutMode>(mode);
            return true; // True loest Neuzeichnen aus
        }

        if (event == ftxui::Event::ArrowRight || event.character() == "d") {
            navigate(1);
            return true;
        }
        if (event == ftxui::Event::ArrowLeft || event.character() == "a") {
            navigate(-1);
            return true;
        }

        // --- SCROLL LOGIK (w, s, Pfeile hoch/runter) ---
        if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('w')) {
            // Wir verhindern, dass wir unter 0 (das erste Event) rutschen
            if (scroll_y > 0) scroll_y--;
            return true; // True loest einen Re-Render der TUI aus
        }
        if (event == ftxui::Event::ArrowDown || event == ftxui::Event::Character('s')) {
            // Wir scrollen nur nach unten, wenn wir nicht schon beim letzten Event sind
            if (!current_events.empty() && scroll_y < (int) current_events.size() - 1) {
                scroll_y++;
            }
            return true;
        }
        // --- MAUSRAD LOGIK ---
        if (event.is_mouse()) {
            if (event.mouse().button == ftxui::Mouse::WheelUp) {
                if (scroll_y > 0) scroll_y--;
                return true;
            }
            if (event.mouse().button == ftxui::Mouse::WheelDown) {
                if (!current_events.empty() && scroll_y < (int) current_events.size() - 1) {
                    scroll_y++;
                }
                return true;
            }
        }

        return false;
    });

    screen.Loop(component);

    refresh_ui_continue = false;
    if (refresh_ui.joinable()) refresh_ui.join();
}
