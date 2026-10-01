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
#include "JournalBackend.h"


// ---  CACHE INITIALISIERUNG ---
void JournalBackend::initCache() {
    const char *home_env = getenv("HOME");
    if (!home_env) { // Kein Home-Verzeichnis, kein Cache
        cerr << "\nThis can take longer as caching is disabled, because your home directory does not exist or is not accessible." << endl;
        return;
    }

    filesystem::path base_cache = string(home_env) + "/.cache";

    // 1. Pruefen, ob ~/.cache existiert
    if (!filesystem::exists(base_cache)) {
        cerr << "\nThis can take longer as caching is disabled, because your ~/.cache directory does not exist or is not accessible." << endl;
        return;
    }

    filesystem::path app_cache = base_cache / "sleephistory";

    // 2. Pruefen, ob der User das Cachen explizit verboten hat
    if (filesystem::exists(app_cache / "donotcache")) {
        cerr << "\nThis can take longer as caching is disabled ('" << app_cache.string() << "/donotcache' file exists, remove it to enable caching)" << endl;
        LOG_DEBUG("Caching disabled via donotcache file.");
        return;
    }

    // 3. Versuchen, die Ordnerstruktur anzulegen
    try {
        filesystem::create_directories(app_cache / "sessions");
        caching_enabled = true;
        cache_dir = app_cache.string();
        LOG_DEBUG("Cache dir initialized: " + cache_dir);
    } catch (...) {
        cerr << "\nThis can take longer as caching is disabled, because creating ~/.cache/sleephistory directory or files in this directory failed." << endl;
        LOG_DEBUG("Failed to create cache directories.");
        return;
    }

    updateBootIndex();
}

// --- INDEX DATEI AKTUALISIEREN ---
void JournalBackend::updateBootIndex() {
    string index_file = cache_dir + "/boot_ids";
    vector<BootEntry> known_entries;

    // 1. Datei laden (Format: ID,FIRST,LAST)
    if (caching_enabled) {
        ifstream infile(index_file);
        string line;
        while (getline(infile, line)) {
            stringstream ss(line);
            string id, first_s, last_s;
            if (getline(ss, id, ',') && getline(ss, first_s, ',') && getline(ss, last_s)) {
                try {
                    known_entries.push_back({id, stoull(first_s), stoull(last_s)});
                } catch (...) {
                }
            }
        }
    }

    // 2. Aktuelle Boots abfragen
    auto lines = execCommand("journalctl --list-boots --output=json 2>/dev/null");
    string full_output = "";
    for (const auto &l: lines) full_output += l + "\n";

    if (full_output.empty()) return;

    // --- HYBRID PARSING LOGIC ---
    if (full_output.find("\"boot_id\"") != string::npos) {
        // --- FALL A: JSON Parsing ---
        size_t search_pos = 0;
        while (true) {
            size_t p_id = full_output.find("\"boot_id\":\"", search_pos);
            if (p_id == string::npos) break;

            string bid = full_output.substr(p_id + 11, 32);
            size_t p_first = full_output.find("\"first_entry\":", p_id);
            size_t p_last = full_output.find("\"last_entry\":", p_id);
            size_t next_boot = full_output.find("\"boot_id\":\"", p_id + 11);

            if (p_first == string::npos || (next_boot != string::npos && p_first > next_boot)) break;

            try {
                auto extract_val = [&](size_t start_pos) {
                    size_t end = full_output.find_first_of(",}", start_pos);
                    return stoull(full_output.substr(start_pos, end - start_pos));
                };

                uint64_t first = extract_val(p_first + 14);
                uint64_t last = extract_val(p_last + 13);

                auto it = find_if(known_entries.begin(), known_entries.end(),
                                       [&](const BootEntry &e) { return e.boot_id == bid; });
                if (it != known_entries.end()) it->last_us = last;
                else known_entries.push_back({bid, first, last});

                search_pos = p_last + 13;
            } catch (...) {
                search_pos = p_id + 11;
                continue;
            }
        }
    } else {
        // --- FALL B: Text Fallback (e.g. Ubuntu 22.04 and older systems have systemd version that doesnt parse list-boots as JSON) ---
        LOG_DEBUG("Systemd JSON not supported for list-boots. Using Text Regex Fallback.");

        istringstream iss(full_output);
        string line;
        // Regex fuer 32-stellige Hex-ID und das Datum YYYY-MM-DD HH:MM:SS
        regex id_regex(R"(([a-fA-F0-9]{32}))");
        regex date_regex(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");

        while (getline(iss, line)) {
            smatch id_match;
            if (!regex_search(line, id_match, id_regex)) continue;

            string bid = id_match[1];
            uint64_t first = 0, last = 0;

            auto date_begin = sregex_iterator(line.begin(), line.end(), date_regex);
            auto date_end = sregex_iterator();

            if (date_begin != date_end) {
                first = parseTextDateToUs(date_begin->str());
                auto next = std::next(date_begin);
                // Wenn ein zweites Datum da ist, ist der Boot beendet, sonst ist er "live"
                last = (next != date_end) ? parseTextDateToUs(next->str()) : now_us();
            }

            auto it = find_if(known_entries.begin(), known_entries.end(),
                                   [&](const BootEntry &e) { return e.boot_id == bid; });
            if (it != known_entries.end()) it->last_us = last;
            else known_entries.push_back({bid, first, last});
        }
    }

    // 3. Speichern & Update
    if (caching_enabled) {
        ofstream outfile(index_file);
        for (const auto &e: known_entries) {
            outfile << e.boot_id << "," << e.first_us << "," << e.last_us << "\n";
        }
    }

    boot_index = known_entries;
    LOG_DEBUG("Index updated. Total boots: " + to_string(boot_index.size()));
}

int JournalBackend::getSessionIdByTimestamp(uint64_t ts_us) {
    // Wenn Index leer, einmalig updaten (auch wenn caching aus ist)
    if (boot_index.empty()) updateBootIndex();

    for (int i = 0; i < (int) boot_index.size(); ++i) {
        if (ts_us >= boot_index[i].first_us && ts_us <= boot_index[i].last_us) {
            return (int) (boot_index.size() - 1 - i);
        }
    }
    return -1;
}

// --- SESSION BINAER SPEICHERN ---
void JournalBackend::saveSessionToCache(const string &boot_id, const Session &session, bool as_tmp) {
    string ext = as_tmp ? ".bin.tmp" : ".bin";
    string path = cache_dir + "/sessions/" + boot_id + ext;
    ofstream file(path, ios::binary);
    if (!file) return;

    const auto &events = session.getEvents();
    uint32_t count = events.size();

    file.write(reinterpret_cast<const char *>(&count), sizeof(count));

    for (const auto &ev: events) {
        file.write(reinterpret_cast<const char *>(&ev.type), sizeof(ev.type));
        file.write(reinterpret_cast<const char *>(&ev.timestamp_us), sizeof(ev.timestamp_us));

        uint32_t msg_len = ev.message.size();
        file.write(reinterpret_cast<const char *>(&msg_len), sizeof(msg_len));
        file.write(ev.message.c_str(), msg_len);
    }
    LOG_DEBUG("Successfully SAVED session to cache: " + path);
}

// --- SESSION BINAER LADEN ---
Session JournalBackend::loadSessionFromCache(unsigned int session_id, const string &boot_id, bool &is_tmp) {
    Session session(session_id);
    is_tmp = false;

    string path_bin = cache_dir + "/sessions/" + boot_id + ".bin";
    string path_tmp = cache_dir + "/sessions/" + boot_id + ".bin.tmp";
    string target_path = "";

    // 1. Zuerst nach der fertigen .bin suchen
    ifstream file(path_bin, ios::binary);
    if (file) {
        target_path = path_bin;
        is_tmp = false;
    } else {
        // 2. Fallback: Nach der unfertigen .tmp suchen
        file.open(path_tmp, ios::binary);
        if (file) {
            target_path = path_tmp;
            is_tmp = true;
        } else {
            return session; // Weder noch gefunden -> Cache Miss
        }
    }

    uint32_t count = 0;
    if (!file.read(reinterpret_cast<char *>(&count), sizeof(count))) return session;

    for (uint32_t i = 0; i < count; ++i) {
        JEvent ev;
        file.read(reinterpret_cast<char *>(&ev.type), sizeof(ev.type));
        file.read(reinterpret_cast<char *>(&ev.timestamp_us), sizeof(ev.timestamp_us));

        uint32_t msg_len = 0;
        file.read(reinterpret_cast<char *>(&msg_len), sizeof(msg_len));

        ev.message.resize(msg_len);
        file.read(&ev.message[0], msg_len);

        session.addEvent(ev);
    }
    LOG_DEBUG("Successfully LOADED session from cache: " + target_path);
    return session;
}


// parse session function
Session &JournalBackend::parseSession(unsigned int session_id) {
    LOG_DEBUG("parsing session " + to_string(session_id));

    auto it = sessions.find(session_id);
    bool is_first_time = (it == sessions.end());
    if (!is_first_time && session_id != 0) {
        return it->second;
    }

    string target_boot_id = "";
    if (boot_index.size() > session_id) {
        target_boot_id = boot_index[boot_index.size() - 1 - session_id].boot_id;
    }

    bool loaded_from_tmp = false;

    // --- 2. DISK-CACHE (Laden) ---
    if (is_first_time && caching_enabled && !target_boot_id.empty()) {
        Session cached_session = loadSessionFromCache(session_id, target_boot_id, loaded_from_tmp);
        if (cached_session.size() > 0) {
            sessions.emplace(session_id, cached_session);

            if (session_id != 0) {
                // Wenn wir eine fertige .bin geladen haben, sind wir fertig!
                if (!loaded_from_tmp) {
                    return sessions.at(session_id);
                }
                LOG_DEBUG("Loaded incomplete .tmp cache for session " + to_string(session_id) + ". Healing...");
                // Wenn es eine .tmp war, returnen wir nicht, sondern parsen weiter unten.
            }
            // else: Live Session 0: Wir gehen immer weiter, um Updates zu holen.
        }
    }

    // Neue Session anlegen, falls sie weder im RAM noch auf Disk war
    if (sessions.find(session_id) == sessions.end()) {
        sessions.emplace(session_id, Session(session_id));
    }

    Session &session = sessions.at(session_id);

    const JEvent *lastParsedEvent = session.getLast();
    string startHere = "";

    if (lastParsedEvent) {
        if (session_id != 0 && !loaded_from_tmp) return session; // Sicherheitsabbruch

        uint64_t since_ts = lastParsedEvent->timestamp_us;

        // WICHTIGER EDGE CASE: lastFetchedToday darf NUR fuer Session 0 hochgezaehlt werden!
        // Wenn wir eine alte .tmp Session heilen, nehmen wir stur deren letzten Zeitstempel.
        if (session_id == 0) {
            if (lastFetchedToday == 0) {
                lastFetchedToday = since_ts;
            }
            since_ts = lastFetchedToday;
            lastFetchedToday = now_us();
        }

        startHere = " --since=\"" + usToYYYY_timestamp(since_ts) + "\"";
        LOG_DEBUG("parsing session starting at timestamp " + usToYYYY_timestamp(since_ts));
    }

    string boot_arg = " -b " + (session_id > 0 ? "-" + to_string(session_id) : to_string(session_id));
    string base_cmd = "journalctl --output=json" + boot_arg + startHere + " 2>/dev/null";

    auto last_lines = execCommand(base_cmd + " -n 1");
    if (!lastParsedEvent) {
        if (last_lines.empty()) {
            // Die Session existiert nicht im Journal.
            // Wir fuegen ein spezielles Event hinzu, das als "Powered Off" gewertet wird,
            // aber eine klare Nachricht traegt.
            session.addEvent({POWERED_OFF, 0, "Session not recorded (Journal data unavailable)"});
            return session;
        }
    }

    uint64_t ts;
    string msg;

    // --- 1. POWERED ON ---
    if (!lastParsedEvent) {
        // journalctl -n +1 ist auf einigen Systemen zickig, 'head -n 1' ist robuster
        auto lines = execCommand(base_cmd + " | head -n 1");
        if (!lines.empty() && parseJournalJson(lines[0], ts, msg)) {
            // Wenn die Nachricht kein klassischer Boot ist,
            // nennen wir es "Log Start" statt "POWERED ON"
            string start_msg = msg;
            if (msg.find("Linux version") == string::npos &&
                msg.find("Command line") == string::npos) {
                start_msg = "[LOG START] " + msg;
            }
            session.addEvent({POWERED_ON, ts, start_msg});
        } else {
            // FALLBACK: Wenn head -n 1 wegen des Pipe-Fehlers abstuerzt,
            // wir aber wissen, dass es Logs gibt (last_lines ist nicht leer!),
            // erzwingen wir ein Event, damit events.size() nicht auf 1 abfaellt.
            uint64_t fallback_ts = 0;
            if (!last_lines.empty() && parseJournalJson(last_lines[0], ts, msg)) {
                fallback_ts = ts;
            }
            session.addEvent({POWERED_ON, fallback_ts, "[LOG START] Pipe Error (Start missing)"});
        }
    }

    // --- 2. SLEEP / HIBERNATION ---
    // journalctl --output=json -b -<session> -kg '^(PM:.*(suspend|hibernation))'
    auto sleep_lines = execCommand(base_cmd + " -kg '^(PM:.*(suspend|hibernation))'");
    for (const auto &line: sleep_lines) {
        if (parseJournalJson(line, ts, msg)) {
            // Deduplikation
            // Wenn wir schon Events haben und dieser Zeitstempel kleiner/gleich
            // dem letzten Event ist, ignorieren wir es. Es ist ein Duplikat vom --since.
            if (lastParsedEvent && ts <= lastParsedEvent->timestamp_us) continue;

            EventType type = HARD_SHUTDOWN;
            if (msg.find("suspend entry") != string::npos) type = SLEEP_START;
            else if (msg.find("suspend exit") != string::npos) type = SLEEP_STOP;
            else if (msg.find("hibernation entry") != string::npos) type = HIBERNATE_START;
            else if (msg.find("hibernation exit") != string::npos) type = HIBERNATE_STOP;
            else continue;
            session.addEvent({type, ts, msg});
        }
    }

    // --- 3. SHUTDOWN / EXIT ---
    // journalctl --output=json -b -<session> -n 1
    // Eine laufende Session (0) hat per Definition noch kein Shutdown-Event.
    if (session_id != 0) {
        if (!last_lines.empty() && parseJournalJson(last_lines[0], ts, msg)) {
            EventType end_type = HARD_SHUTDOWN;
            // a. Normaler Shutdown
            if (msg.find("Journal stopped") != string::npos) {
                end_type = POWERED_OFF;
            } else {
                // b. Magic SysRq
                string msg_lower = msg;
                transform(msg_lower.begin(), msg_lower.end(), msg_lower.begin(), ::tolower);
                if (msg_lower.find("sysrq") != string::npos) end_type = MAGIC_SYSRQ;
            }
            // Hier landen ECHTE Abstuerze: end_type bleibt HARD_SHUTDOWN,
            // msg ist die letzte vom Kernel gespuckte Zeile vor dem Crash.
            // we add 1µs for the edge case when a previous event has the exact same ts as this one
            // this may happen if pc went to sleep and didnt wake up again
            session.addEvent({end_type, ts + 1, msg});
        } else {
            // c. Logs rotiert / nicht mehr existent
            // Hier landen wir, wenn journalctl KEINE Zeilen mehr fuer die boot_id hat.
            // Sicherheitshindernis: Nur getLast() nutzen, wenn es Events gibt!
            uint64_t last_ts = 0;
            if (!session.getEvents().empty()) {
                last_ts = session.getLast()->timestamp_us + 1;
            }
            session.addEvent({POWERED_OFF, last_ts, "End of session unknown (Logs rotated out)"});
        }
    }

    // --- 3. DISK-CACHE (Schreiben & Aufraeumen) ---
    if (caching_enabled && !target_boot_id.empty() && session.size() > 0) {
        if (session_id == 0) {
            // Live Session: Nur beim allerersten Start des Programms als .tmp speichern
            if (is_first_time) {
                LOG_DEBUG("Updating live session cache on disk (.tmp) for boot_id " + target_boot_id);
                saveSessionToCache(target_boot_id, session, true); // as_tmp = true
            }
        } else {
            // Vergangene Session: Wir haben sie nun bis zum Ende geparst.
            LOG_DEBUG("Saving finalized session " + target_boot_id + " to disk (.bin)...");
            saveSessionToCache(target_boot_id, session, false); // as_tmp = false

            // Wenn wir sie vorhin als .tmp geladen haben, raeumen wir die alte .tmp jetzt auf!
            if (loaded_from_tmp) {
                string tmp_path = cache_dir + "/sessions/" + target_boot_id + ".bin.tmp";
                if (remove(tmp_path.c_str()) == 0) {
                    LOG_DEBUG("Cleaned up old .tmp file for boot_id " + target_boot_id);
                }
            }
        }
    }

    LOG_DEBUG("parsing session " + to_string(session_id) + " completed");
    return session;
}

// Laedt alle relevanten Events fuer einen bestimmten Tag
vector<JEvent> JournalBackend::parseDay(const string &date_dd_mm_yyyy,
                                             vector<unsigned int> &included_sessions) {
    LOG_DEBUG("parsing day "+date_dd_mm_yyyy);

    // 1. Cache-Check
    bool is_today = (date_dd_mm_yyyy == today_Date_h());
    if (!is_today && day_cache.count(date_dd_mm_yyyy)) {
        included_sessions = session_cache[date_dd_mm_yyyy];
        return day_cache[date_dd_mm_yyyy];
    }

    // 2. Zeitrahmen berechnen
    uint64_t start_us = to_start_of_day_us(date_dd_mm_yyyy);
    uint64_t end_us = to_start_of_day_us(addDaysToDate(date_dd_mm_yyyy, 1)) - 1;

    // 3. Direktsprung zu den Sessions via Timestamp-Index
    int start_sid = getSessionIdByTimestamp(start_us);
    int end_sid = is_today ? 0 : getSessionIdByTimestamp(end_us);


    // Falls der Tag komplett vor dem ersten Log-Eintrag liegt
    if (start_sid == -1 && !boot_index.empty() && end_us < boot_index[0].first_us) {
        included_sessions.clear();
        return {};
    }

    // Fallbacks fuer Luecken (PC war ueber Mitternacht aus)
    if (start_sid == -1) start_sid = (int) boot_index.size() - 1;
    if (end_sid == -1) end_sid = 0;

    // Sicherstellen, dass wir von der aeltesten zur neuesten Session des Tages iterieren
    // (In session_ids: end_sid ist kleiner/neuer als start_sid)
    int first_to_check = min(start_sid, end_sid);
    int last_to_check = max(start_sid, end_sid);

    vector<unsigned int> overlapping_sessions;
    for (int sid = first_to_check; sid <= last_to_check; ++sid) {
        Session &s = parseSession(sid);
        if (s.size() == 0) continue;

        uint64_t s_first = s.getFirst()->timestamp_us;
        uint64_t s_last = (sid == 0) ? now_us() : s.getLast()->timestamp_us;

        // Ueberschneidet sich die Session tatsaechlich mit dem Tag?
        if (s_first <= end_us && s_last >= start_us) {
            overlapping_sessions.push_back(sid);
        }
    }

    // 4. Den Zustand direkt vor 00:00:00 herausfinden
    // Wir nehmen die Session, die vor der ersten ueberlappenden Session kam
    int previous_session_id = -1;
    if (!overlapping_sessions.empty()) {
        // Die "aelteste" Session des Tages im Sinne der ID (hoechste Nummer)
        unsigned int max_sid = 0;
        for (unsigned int sid: overlapping_sessions) if (sid > max_sid) max_sid = sid;

        if (max_sid + 1 < boot_index.size()) {
            previous_session_id = max_sid + 1;
        }
    }

    const JEvent *last_before_day = nullptr;
    auto check_for_last_event = [&](unsigned int sid) {
        // Auch hier: parseSession stellt sicher, dass die Daten existieren
        Session &s = parseSession(sid);
        for (const auto &ev: s.getEvents()) {
            if (ev.timestamp_us < start_us) {
                if (!last_before_day || ev.timestamp_us > last_before_day->timestamp_us) {
                    last_before_day = &ev;
                }
            }
        }
    };

    for (unsigned int sid: overlapping_sessions) check_for_last_event(sid);
    if (previous_session_id != -1) check_for_last_event(previous_session_id);

    // State ableiten
    EventType start_state = CONTINUED_OFF; // Default fallback
    string start_msg = "Carried Over: Powered Off";

    if (last_before_day) {
        switch (last_before_day->type) {
            case SLEEP_START:
                start_state = CONTINUED_SLEEP;
                start_msg = "Carried Over: Sleeping";
                break;
            case HIBERNATE_START:
                start_state = CONTINUED_HIBERNATE;
                start_msg = "Carried Over: Hibernating";
                break;
            case POWERED_ON:
            case SLEEP_STOP:
            case HIBERNATE_STOP:
                start_state = CONTINUED_ON;
                start_msg = "Carried Over: Active";
                break;
            case HARD_SHUTDOWN:
                start_state = CONTINUED_HARD_SHUTDOWN;
                start_msg = "Carried Over: Hard Shutdown";
                break;
            case MAGIC_SYSRQ:
                start_state = CONTINUED_SYSRQ;
                start_msg = "Carried Over: Magic SysRq";
                break;
            case POWERED_OFF:
            default:
                start_state = CONTINUED_OFF;
                start_msg = "Carried Over: Powered Off";
                break;
        }
    }

    // 5. Day Events zusammenbauen
    vector<JEvent> day_events;

    // Das synthetische 00:00:00 Event
    day_events.push_back({start_state, start_us, start_msg});

    // Alle echten Events, die in den Tag fallen, sammeln
    for (unsigned int sid: overlapping_sessions) {
        for (const auto &ev: sessions.at(sid).getEvents()) {
            if (ev.timestamp_us >= start_us && ev.timestamp_us <= end_us) {
                day_events.push_back(ev);
            }
        }
    }

    // Da wir Sessions in absteigender Reihenfolge gesammelt haben, muessen
    // wir die Events chronologisch sortieren (aufsteigend)
    stable_sort(day_events.begin(), day_events.end(), [](const JEvent &a, const JEvent &b) {
        return a.timestamp_us < b.timestamp_us;
    });

    // 6. Das [END OF DAY] Event (nur wenn nicht heute)
    if (!is_today) {
        day_events.push_back({END_OF_DAY, end_us, ""});
        // Cachen
        day_cache[date_dd_mm_yyyy] = day_events;
        session_cache[date_dd_mm_yyyy] = overlapping_sessions;
    }
    included_sessions = overlapping_sessions;
    LOG_DEBUG("parsing day "+date_dd_mm_yyyy+" completed");
    return day_events;
}

vector<JEvent> JournalBackend::parseWeek(const string &date_dd_mm_yyyy,
                                              vector<unsigned int> &included_sessions) {
    LOG_DEBUG("parsing week "+date_dd_mm_yyyy);

    string monday_str = get_monday_of_week(date_dd_mm_yyyy);
    bool is_current_week = (monday_str == get_monday_of_week(today_Date_h()));

    // 1. Cache-Check
    if (!is_current_week && week_cache.count(monday_str)) {
        included_sessions = week_session_cache[monday_str];
        return week_cache[monday_str];
    }

    // 2. Zeitrahmen fuer die WOCHE berechnen (Mo 00:00 bis So 23:59)
    uint64_t start_us = to_start_of_day_us(monday_str);
    string next_monday_str = addDaysToDate(monday_str, 7);
    uint64_t end_us = to_start_of_day_us(next_monday_str) - 1;

    unsigned int current_sid = 0;

    // 3. Direktsprung zu den Sessions via Timestamp-Index
    int start_sid = getSessionIdByTimestamp(start_us);
    int end_sid = is_current_week ? 0 : getSessionIdByTimestamp(end_us);

    // Falls die Woche komplett vor dem ersten Log-Eintrag liegt
    if (start_sid == -1 && !boot_index.empty() && end_us < boot_index[0].first_us) {
        included_sessions.clear();
        return {};
    }

    // Fallbacks fuer Luecken
    if (start_sid == -1) start_sid = (int) boot_index.size() - 1;
    if (end_sid == -1) end_sid = 0;

    int first_to_check = min(start_sid, end_sid);
    int last_to_check = max(start_sid, end_sid);

    vector<unsigned int> overlapping_sessions;
    for (int sid = first_to_check; sid <= last_to_check; ++sid) {
        Session &s = parseSession(sid);
        if (s.size() == 0) continue;

        uint64_t s_first = s.getFirst()->timestamp_us;
        uint64_t s_last = (sid == 0) ? now_us() : s.getLast()->timestamp_us;

        if (s_first <= end_us && s_last >= start_us) {
            overlapping_sessions.push_back(sid);
        }
    }

    // 4. Den Zustand direkt vor Montagmorgen 00:00:00 herausfinden
    int previous_session_id = -1;
    if (!overlapping_sessions.empty()) {
        unsigned int max_sid = 0;
        for (unsigned int sid: overlapping_sessions) if (sid > max_sid) max_sid = sid;

        if (max_sid + 1 < boot_index.size()) {
            previous_session_id = max_sid + 1;
        }
    }

    // 4. Den Zustand direkt vor Montagmorgen 00:00:00 herausfinden
    const JEvent *last_before_week = nullptr;
    auto check_for_last_event = [&](unsigned int sid) {
        // Da wir zuvor parseSession(sid) gerufen haben, koennen wir hier sicher auf sessions.at zugreifen
        // Bei previous_session_id rufen wir sicherheitshalber parseSession auf.
        Session &s = parseSession(sid);
        for (const auto &ev: s.getEvents()) {
            if (ev.timestamp_us < start_us) {
                if (!last_before_week || ev.timestamp_us > last_before_week->timestamp_us) {
                    last_before_week = &ev;
                }
            }
        }
    };

    for (unsigned int sid: overlapping_sessions) check_for_last_event(sid);
    if (previous_session_id != -1) check_for_last_event(previous_session_id);

    EventType start_state = CONTINUED_OFF;
    string start_msg = "Carried Over: Powered Off";
    if (last_before_week) {
        switch (last_before_week->type) {
            case SLEEP_START: start_state = CONTINUED_SLEEP;
                start_msg = "Carried Over: Sleeping";
                break;
            case HIBERNATE_START: start_state = CONTINUED_HIBERNATE;
                start_msg = "Carried Over: Hibernating";
                break;
            case POWERED_ON:
            case SLEEP_STOP:
            case HIBERNATE_STOP: start_state = CONTINUED_ON;
                start_msg = "Carried Over: Active";
                break;
            case HARD_SHUTDOWN: start_state = CONTINUED_HARD_SHUTDOWN;
                start_msg = "Carried Over: Hard Shutdown";
                break;
            case MAGIC_SYSRQ: start_state = CONTINUED_SYSRQ;
                start_msg = "Carried Over: Magic SysRq";
                break;
            case POWERED_OFF: default: start_state = CONTINUED_OFF;
                start_msg = "Carried Over: Powered Off";
                break;
        }
    }

    // 5. Week Events zusammenbauen
    vector<JEvent> week_events;
    week_events.push_back({start_state, start_us, start_msg});

    for (unsigned int sid: overlapping_sessions) {
        for (const auto &ev: sessions.at(sid).getEvents()) {
            if (ev.timestamp_us >= start_us && ev.timestamp_us <= end_us) {
                week_events.push_back(ev);
            }
        }
    }

    stable_sort(week_events.begin(), week_events.end(), [](const JEvent &a, const JEvent &b) {
        return a.timestamp_us < b.timestamp_us;
    });

    // 6. Das [END OF WEEK] Event
    if (!is_current_week) {
        week_events.push_back({END_OF_WEEK, end_us, ""});
        week_cache[monday_str] = week_events;
        week_session_cache[monday_str] = overlapping_sessions;
    }
    included_sessions = overlapping_sessions;

    LOG_DEBUG("parsing week "+date_dd_mm_yyyy+" completed");
    return week_events;
}

string JournalBackend::getBootId(unsigned int session_id) {
    if (boot_index.empty()) updateBootIndex();
    if (session_id < boot_index.size()) {
        return boot_index[boot_index.size() - 1 - session_id].boot_id;
    }
    return "";
}