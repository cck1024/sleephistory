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

#include "Structures.h"

string eventTypeToString(EventType t) {
    switch(t) {
        case POWERED_ON: return "POWERED ON";
        case CONTINUED_ON: return "POWERED ON";
        case POWERED_OFF: return "POWERED OFF";
        case CONTINUED_OFF: return "POWERED OFF";
        case SLEEP_START: return "SLEEP START";
        case SLEEP_STOP: return "SLEEP STOP";
        case CONTINUED_SLEEP: return "SLEEP";
        case HIBERNATE_START: return "HIBERNATE START";
        case HIBERNATE_STOP: return "HIBERNATE STOP";
        case CONTINUED_HIBERNATE: return "HIBERNATE";
        case MAGIC_SYSRQ: return "MAGIC SYSRQ";
        case CONTINUED_SYSRQ: return "MAGIC SYSRQ";
        case HARD_SHUTDOWN: return "HARD SHUTDOWN";
        case CONTINUED_HARD_SHUTDOWN: return "HARD SHUTDOWN";
        case END_OF_DAY: return "END OF DAY";
        case END_OF_WEEK: return "END OF WEEK";
        case PRESENT_TIME: return "PRESENT TIME";
        default: return "UNKNOWN";
    }
}

string getStatusString(EventType type) {
    switch (type) {
        case SLEEP_STOP:
        case HIBERNATE_STOP:
        case CONTINUED_ON:
        case POWERED_ON:          return "ACTIVE";
        case CONTINUED_SLEEP:
        case SLEEP_START:         return "SLEEPING";
        case CONTINUED_HIBERNATE:
        case HIBERNATE_START:     return "HIBERNATING";
        case CONTINUED_OFF:
        case POWERED_OFF:         return "POWERED OFF";
        case CONTINUED_SYSRQ:
        case MAGIC_SYSRQ:         return "POWERED OFF (SYSRQ)";
        case CONTINUED_HARD_SHUTDOWN:
        case HARD_SHUTDOWN:       return "POWERED OFF (CRASHED)";
        case PRESENT_TIME: return "PRESENT TIME";
        default:                  return "UNKNOWN";
    }
}


// Ein zentraler Parser fuer die Eingabe (wird von CLI und TUI VIM-Mode genutzt)
bool parseInputString(const string& input, ViewMode& out_mode, string& out_date, unsigned int& out_session) {
    regex date_regex(R"((\d{1,2})[/\.](\d{1,2})([/\.](\d{2,4}))?)");
    regex cw_regex(R"((?:cw|kw)(\d+)(?:[\./](\d+))?)", regex::icase);
    smatch match;

    if (regex_match(input, match, date_regex)) {
        string day = match[1].str();
        string month = match[2].str();
        string year = match[4].str();

        if (day.length() == 1) day = "0" + day;
        if (month.length() == 1) month = "0" + month;

        // Anmerkung: today_Date_h() muss verfuegbar sein
        if (year.empty()) year = today_Date_h().substr(6, 4);
        else if (year.length() == 2) year = "20" + year;

        out_date = day + "/" + month + "/" + year;
        out_mode = ViewMode::DAY;
        return true;
    }
    if (regex_match(input, regex(R"(\d+)"))) {
        out_session = stoi(input);
        out_mode = ViewMode::SESSION;
        return true;
    }
    if (regex_match(input, match, cw_regex)) {
        int cw = stoi(match[1]);
        string year = match[2].matched ? match[2].str() : "";

        out_date = date_from_cw(cw, year);
        out_mode = ViewMode::WEEK; // WICHTIG: Er schaltet direkt in die Week-View um!
        return true;
    }
    return false;
}

void prepareEvents(ViewMode mode, const string& date, unsigned int session,
                   vector<JEvent>& events) {

    // Pruefen, ob wir im "Live-Modus" sind
    bool is_live = (mode == ViewMode::DAY && date == today_Date_h()) ||
                   (mode == ViewMode::WEEK && get_monday_of_week(date) == get_monday_of_week(today_Date_h())) ||
                   (mode == ViewMode::SESSION && session == 0);

    if (is_live) {
        // Falls das letzte Event schon PRESENT_TIME ist, nur updaten, sonst neu hinzufuegen
        if (!events.empty() && events.back().type == PRESENT_TIME) {
            events.back().timestamp_us = now_us();
        } else {
            events.push_back({PRESENT_TIME, now_us(), "Present Time"});
        }
    }

    // WICHTIG: stable_sort statt sort nutzen!
    // Da keine sequence_num existiert, behaelt stable_sort die
    // urspruengliche Reihenfolge von journalctl bei gleichem Zeitstempel bei.
    stable_sort(events.begin(), events.end(), [](const JEvent& a, const JEvent& b) {
        return a.timestamp_us < b.timestamp_us;
    });
}
