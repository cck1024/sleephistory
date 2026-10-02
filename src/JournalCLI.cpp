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

#include "JournalCLI.h"

void JournalCLI::Run(JournalBackend &backend, const string &arg) {
    ViewMode mode;
    string date;
    unsigned int session = 0;

    string clean_arg = arg;
    bool inMarkdown = false;
    size_t pos = clean_arg.find("--markdown");
    if (pos != string::npos) {
        inMarkdown = true;
        clean_arg.erase(pos, 10); // "--markdown" ist 10 Zeichen lang
    } else {
        pos = clean_arg.find("-md");
        if (pos != string::npos) {
            inMarkdown = true;
            clean_arg.erase(pos, 3); // "-md" ist 3 Zeichen lang
        }
    }
    // removing unnecessary whitespaces
    size_t first_valid = clean_arg.find_first_not_of(" \t");
    if (first_valid == string::npos) {
        clean_arg = ""; // String ist nach dem Entfernen leer oder enthielt nur Leerzeichen
    } else {
        clean_arg.erase(0, first_valid);
        clean_arg.erase(clean_arg.find_last_not_of(" \t") + 1);
    }

    if (!parseInputString(clean_arg, mode, date, session)) {
        cerr << ANSI_RED << "Invalid argument format." << ANSI_RESET
                << " Use DD/MM/YY, session number, CW[number] (e.g. cw12) or -h for help\n";
        return;
    }

    if ((mode == ViewMode::DAY || mode == ViewMode::WEEK) && isFuture(date)) {
        cout << ANSI_RED << "Date is in the future. No data available." << ANSI_RESET << "\n";
        return;
    }

    vector<unsigned int> included;
    vector<JEvent> events;

    if (mode == ViewMode::DAY) {
        events = backend.parseDay(date, included);
    } else if (mode == ViewMode::WEEK) {
        events = backend.parseWeek(date, included);
    } else {
        events = backend.parseSession(session).getEvents();
    }

    prepareEvents(mode, date, session, events);

    if ((mode == ViewMode::SESSION && events.size() <= 1)||(mode != ViewMode::SESSION && events.empty())) {
        cout << ANSI_DIM << "No events found for this selection." << ANSI_RESET << "\n";
        return;
    }

    TimeSummary summary = Analyzer::calculateStats(events);
    string boot_id = (mode == ViewMode::SESSION) ? backend.getBootId(session) : "";
    if (inMarkdown) {
        string markdown_output = convertToMarkdown(
            events,
            mode,
            date,
            included,
            summary,
            boot_id
        );
        cout << markdown_output << endl;
    } else {
        uint64_t total = summary.active_us + summary.sleep_us + summary.hibernate_us;

        // --- HEADER LOGIK ---
        string title;
        string sub_info = "";
        if (mode == ViewMode::WEEK) {
            // Berechne CW fuer die Anzeige im Header
            int cw = get_week_number(date);
            string end_date = addDaysToDate(date, 6);
            title = "WEEK VIEW: " + date + " to " + end_date + " (CW " + to_string(cw) + ")";
            sub_info = "Included Sessions: " + printSessions(included);
        } else if (mode == ViewMode::DAY) {
            title = "DAY VIEW: " + date;
            sub_info = "Included Sessions: " + printSessions(included);
        } else {
            string boot_id = backend.getBootId(session);
            title = "SESSION VIEW: " + to_string(session);
            if (!boot_id.empty()) {
                title += " (" + boot_id + ")";
            }
            if (!events.empty()) {
                sub_info = "Range: " + beautiful_timestamp(events.front().timestamp_us) +
                           " to " + beautiful_timestamp(events.back().timestamp_us);
            }
        }

        cout << ANSI_CYAN << ANSI_BOLD << title << ANSI_RESET << "    " << ANSI_DIM << sub_info << ANSI_RESET <<
                "\n";
        cout << string(80, '-') << "\n";

        // TOTAL und Zeiten mit Paritaet
        cout << ANSI_BOLD << "TOTAL: " << us_to_h_m_s(total) << (total > 86400000000
                                                                          ? (" (" + us_to_d_h(total) + ")")
                                                                          : "") << ANSI_RESET << " | "
                << ANSI_GREEN << "ACTIVE: " << us_to_h_m_s(summary.active_us) << (summary.active_us > 86400000000
                    ? (" (" + us_to_d_h(summary.active_us) + ")")
                    : "") << ANSI_RESET << " | "
                << ANSI_SKYBLUE << "SLEEP: " << us_to_h_m_s(summary.sleep_us) << (summary.sleep_us > 86400000000
                    ? (" (" + us_to_d_h(summary.sleep_us) + ")")
                    : "") << ANSI_RESET << " | "
                << ANSI_HOTPINK << "HIBERNATE: " << us_to_h_m_s(summary.hibernate_us) << (
                    summary.hibernate_us > 86400000000 ? (" (" + us_to_d_h(summary.hibernate_us) + ")") : "") <<
                ANSI_RESET << "\n";
        cout << string(80, '-') << "\n";

        // --- EVENT LOOP ---
        for (size_t i = 0; i < events.size(); ++i) {
            const auto &ev = events[i];
            string label = (ev.type == PRESENT_TIME) ? "[CURRENT]" : "[" + eventTypeToString(ev.type) + "]";

            // Farbe auf Label UND Message anwenden, wie im TUI
            string event_color = getAnsiColorForEvent(ev.type);

            cout << ANSI_DIM << beautiful_timestamp(ev.timestamp_us) << ANSI_RESET << " "
                    << event_color << ANSI_BOLD << label << " " << ev.message << ANSI_RESET << "\n";

            if (i < events.size() - 1) {
                uint64_t diff = events[i + 1].timestamp_us - ev.timestamp_us;
                cout << event_color << "   └── " << us_to_h_m_s(diff) << (diff > 86400000000
                                                                                   ? (" (" + us_to_d_h(diff) + ") ")
                                                                                   : " ")
                        << ANSI_BOLD << "[ STATUS: " << getStatusString(ev.type) << " ]" << ANSI_RESET << "\n";
                cout << ANSI_DIM << string(80, '-') << ANSI_RESET << "\n";
            }
        }
    }
}
