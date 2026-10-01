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

#include <memory>
#include <stdio.h>
#include <array>
#include <vector>
#include <string>

#include "parser.h"

// Fuehrt einen Shell-Befehl aus und gibt die Zeilen als Vector zurueck
vector<string> execCommand(const string& cmd) {
    array<char, 4096> buffer;
    vector<string> result;

    // Wir nutzen hier eine normale FILE* Struktur, um die volle Kontrolle zu haben
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return {}; // Oder Log-Fehler

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        string line = buffer.data();
        if (!line.empty() && line.back() == '\n') line.pop_back();
        result.push_back(line);
    }
    // pclose gibt den Exit-Status zurueck. Wir ignorieren ihn hier bewusst,
    // damit SIGPIPE (13) bei 'head' Aufrufen nicht zum Abbruch fuehrt.
    pclose(pipe);

    return result;
}

// Ein simpler JSON-Parser, der Timestamp und Message extrahiert
// (Spart uns die Einbindung einer riesigen externen JSON-Bibliothek)
bool parseJournalJson(const string& json_line, uint64_t& ts_us, string& msg) {
    // 1. Timestamp extrahieren
    size_t ts_pos = json_line.find("\"__REALTIME_TIMESTAMP\":\"");
    if (ts_pos == string::npos) return false;
    ts_pos += 24; // Laenge des Suchstrings
    size_t ts_end = json_line.find("\"", ts_pos);
    ts_us = stoull(json_line.substr(ts_pos, ts_end - ts_pos));

    // 2. Message extrahieren
    size_t msg_pos = json_line.find("\"MESSAGE\":\"");
    if (msg_pos == string::npos) return false;
    msg_pos += 11;
    size_t msg_end = json_line.find("\"", msg_pos);
    msg = json_line.substr(msg_pos, msg_end - msg_pos);

    return true;
}

string printSessions(const vector<unsigned int>& included_sessions) {
    ostringstream oss;
    for (size_t i = 0; i < included_sessions.size(); ++i) {
        if (i) oss << ", ";
        oss << included_sessions[i];
    }
    return oss.str();
}

string convertToMarkdown(
    const vector<JEvent>& events,
    ViewMode view,
    const string& date_str,
    const vector<unsigned int>& included_sessions,
    const TimeSummary& summary,
    const string& boot_id // default=""
) {
    string sub_info = "";
    if (!events.empty()) {
        // Gleiche Logik wie vorher: Day und Week nutzen den Datum-String, Sessions die Timestamps
        if (view == ViewMode::DAY || view == ViewMode::WEEK) {
            sub_info = date_str + " (Sessions: " + printSessions(included_sessions) + ")";
        } else {
            sub_info = "Range: " + beautiful_timestamp(events.front().timestamp_us) +
                       " to " + beautiful_timestamp(events.back().timestamp_us);
        }
    }

    stringstream ss;
    ss << "# sleephistory for " << sub_info << "\n";

    if (view == ViewMode::SESSION && !boot_id.empty()) {
        ss << "\n# boot id: " << boot_id << "\n";
    }

    ss << "\n## Summary\n";
    uint64_t total = summary.active_us + summary.sleep_us + summary.hibernate_us;
    ss << "| Mode | Duration |\n";
    ss << "| :--- | :--- |\n";
    ss << "| **Active** | " << us_to_h_m_s(summary.active_us) + (summary.active_us>86400000000?(" ("+us_to_d_h(summary.active_us)+")"):"") << " |\n";
    ss << "| **Sleep** | " << us_to_h_m_s(summary.sleep_us) + (summary.sleep_us>86400000000?(" ("+us_to_d_h(summary.sleep_us)+")"):"") << " |\n";
    ss << "| **Hibernate** | " << us_to_h_m_s(summary.hibernate_us) + (summary.hibernate_us>86400000000?(" ("+us_to_d_h(summary.hibernate_us)+")"):"") << " |\n";
    ss << "| **Total** | " << us_to_h_m_s(total) + (total>86400000000?(" ("+us_to_d_h(total)+")"):"") << " |\n";

    ss << "\n## Events\n";
    ss << "| Time | Event | Message |\n";
    ss << "| :--- | :--- | :--- |\n";

    for (const auto& ev : events) {
        string time_str = beautiful_timestamp(ev.timestamp_us);
        string type_str = eventTypeToString(ev.type);
        ss << "| " << time_str << " | " << type_str << " | " << ev.message << " |\n";
    }

    return ss.str();
}
