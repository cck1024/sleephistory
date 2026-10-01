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

#ifndef SLEEPHISTORY_JOURNALBACKEND_H
#define SLEEPHISTORY_JOURNALBACKEND_H
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstdlib> // fuer getenv
#include <cstdio>
#include <regex>
#include <unistd.h>

#include "parser.h"
#include "Structures.h"
#include "debugging.h"

#include "timehelpers.h"

class JournalBackend {
private:
    struct BootEntry {
        string boot_id;
        uint64_t first_us;
        uint64_t last_us;
    };

    map<unsigned int, Session> sessions;
    map<string, vector<JEvent>> day_cache; // cached jeden Day außer heute. string wie folgt speichern: d/m/Y z.b. 11/03/2026
    map<string, vector<unsigned int>> session_cache; // cache used in parseDay (so it knows which sessions are part of day)
    map<string, vector<JEvent>> week_cache;
    map<string, vector<unsigned int>> week_session_cache;

    uint64_t lastFetchedToday = 0; // stores unixtimestamp to improve current day reloading using --since

    // --- CACHE VARIABLEN ---
    bool caching_enabled = false;
    string cache_dir;
    vector<BootEntry> boot_index;

    // --- CACHE INITIALISIERUNG ---
    void initCache();
    // --- INDEX DATEI AKTUALISIEREN ---
    void updateBootIndex();
    int getSessionIdByTimestamp(uint64_t ts_us);

    // --- SESSION BINAER SPEICHERN ---
    void saveSessionToCache(const string& boot_id, const Session& session, bool as_tmp);
    // --- SESSION BINAER LADEN ---
    Session loadSessionFromCache(unsigned int session_id, const string& boot_id, bool& is_tmp);

public:
    JournalBackend() {
        if (isatty(STDERR_FILENO)) {
            std::cerr << "Loading... Please wait..." << std::flush;
        }
        initCache();
        if (isatty(STDERR_FILENO)) {
            std::cerr << "\r\033[K" << std::flush;
        }
        if (caching_enabled) {
            LOG_DEBUG("Caching is ENABLED. Total boots in index: " + to_string(boot_index.size()));
        } else {
            LOG_DEBUG("Caching is DISABLED.");
        }
    }

    // parse session function
    Session& parseSession(unsigned int session_id);
    // Laedt alle relevanten Events fuer einen bestimmten Tag
    vector<JEvent> parseDay(const string& date_dd_mm_yyyy, vector<unsigned int>& included_sessions);
    // Laedt alle relevanten Events fuer eine bestimmte Woche
    vector<JEvent> parseWeek(const string& date_dd_mm_yyyy, vector<unsigned int>& included_sessions);
    // Gibt die Boot ID fuer eine bestimmte Session-ID zurueck
    string getBootId(unsigned int session_id);
};
#endif //SLEEPHISTORY_JOURNALBACKEND_H