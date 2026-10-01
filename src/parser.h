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

#ifndef SLEEPHISTORY_PARSER_H
#define SLEEPHISTORY_PARSER_H
#include <cstdint>

#include "Structures.h"

// Führt einen Shell-Befehl aus und gibt die Zeilen als Vector zurück
vector<string> execCommand(const string& cmd);

// Ein simpler JSON-Parser, der Timestamp und Message extrahiert
// (Spart Einbindung einer riesigen externen JSON-Bibliothek)
bool parseJournalJson(const string& json_line, uint64_t& ts_us, string& msg);

// Logic
struct TimeSummary {
    uint64_t active_us = 0;
    uint64_t sleep_us = 0;
    uint64_t hibernate_us = 0;
};

class Analyzer {
public:
    static TimeSummary calculateStats(const vector<JEvent>& events, uint64_t day_start_us = 0, uint64_t day_end_us = 0) {
        TimeSummary stats;
        if (events.empty()) return stats;

        for (size_t i = 0; i < events.size() - 1; ++i) {
            uint64_t start = events[i].timestamp_us;
            uint64_t end = events[i+1].timestamp_us;

            // Für Day View: Zeiträume auf den Tag begrenzen
            if (day_start_us != 0) {
                if (end < day_start_us || start > day_end_us) continue;
                start = max(start, day_start_us);
                end = min(end, day_end_us);
            }

            uint64_t diff = end - start;

            switch (events[i].type) {
                case CONTINUED_SLEEP:
                case SLEEP_START:     stats.sleep_us += diff; break;
                case CONTINUED_HIBERNATE:
                case HIBERNATE_START: stats.hibernate_us += diff; break;
                case POWERED_ON:
                case CONTINUED_ON:
                case SLEEP_STOP:
                case HIBERNATE_STOP:  stats.active_us += diff; break;
                default: break;
            }
        }

        // Sonderfall HARD_SHUTDOWN:
        // Wenn das letzte Event kein reguläres Ende ist, zählt die Zeit davor als aktiv.
        // Das wurde oben bereits durch die Schleife bis (size-1) abgedeckt.
        return stats;
    }
};

string printSessions(const vector<unsigned int>& included_sessions);

string convertToMarkdown(
    const vector<JEvent>& events,
    ViewMode view,
    const string& date_str,
    const vector<unsigned int>& included_sessions,
    const TimeSummary& summary,
    const string& boot_id = ""
);

#endif //SLEEPHISTORY_PARSER_H