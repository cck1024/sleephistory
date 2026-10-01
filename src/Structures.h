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

#ifndef SLEEPHISTORY_STRUCTURES_H
#define SLEEPHISTORY_STRUCTURES_H
#include <regex>
#include <vector>

#include "timehelpers.h"

enum EventType {
    POWERED_ON, POWERED_OFF,
    SLEEP_START, SLEEP_STOP,
    CONTINUED_SLEEP,
    HIBERNATE_START, HIBERNATE_STOP,
    CONTINUED_HIBERNATE,
    MAGIC_SYSRQ, HARD_SHUTDOWN,
    CONTINUED_ON,
    CONTINUED_OFF, // computer got shutdown normal last time
    CONTINUED_SYSRQ, // computer got shutdown with sysrq last time
    CONTINUED_HARD_SHUTDOWN, // computer got shutdown hard last time
    END_OF_DAY,
    END_OF_WEEK,
    PRESENT_TIME
};

struct JEvent {
    EventType type;
    uint64_t timestamp_us; // stored as us
    string message;
};

class Session {
private:
    unsigned int session_id; // session id gets stored unsigned although e.g. session 1 gets executed as -b -1
    vector<JEvent> events;
public:
    explicit Session(unsigned int id) : session_id(id) {
        events.reserve(10);
    }

    void addEvent(JEvent e) {
        events.push_back(move(e));
    }

    const JEvent* getFirst() const {
        return events.empty() ? nullptr : &events.front();
    }

    const JEvent* getLast() const {
        return events.empty() ? nullptr : &events.back();
    }

    // Helpers
    size_t size() const { return events.size(); }

    unsigned int getSessionId() const { return session_id; }

    // Iterator
    const vector<JEvent>& getEvents() const { return events; }
};

string eventTypeToString(EventType t);
string getStatusString(EventType type);

enum class ViewMode { SESSION, DAY, WEEK };

// Ein zentraler Parser fuer die Eingabe (wird von CLI und TUI VIM-Mode genutzt)
bool parseInputString(const string& input, ViewMode& out_mode, string& out_date, unsigned int& out_session);

void prepareEvents(ViewMode mode, const string& date, unsigned int session,
                   vector<JEvent>& events);

#endif //SLEEPHISTORY_STRUCTURES_H