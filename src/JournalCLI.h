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

#ifndef SLEEPHISTORY_JOURNALCLI_H
#define SLEEPHISTORY_JOURNALCLI_H

#include <iostream>
#include <vector>
#include <sstream>

#include "JournalBackend.h"
#include "Structures.h"
#include "Colors.h"
#include "debugging.h"

inline std::string getAnsiColorForEvent(EventType t) {
    switch(t) {
        // --- GRUEN: Aktiv / Start / Ende ---
        case POWERED_ON:
        case END_OF_DAY:
        case END_OF_WEEK:
        case PRESENT_TIME:
        case CONTINUED_ON:
            return ANSI_GREEN;
        case SLEEP_STOP:
        case HIBERNATE_STOP:
            return ANSI_SPRINGGREEN3;
            // --- ORANGE: Sicher Aus ---
        case POWERED_OFF:
        case CONTINUED_OFF:
            return ANSI_ORANGE;

            // --- ROT: Kritisch / Crash ---
        case HARD_SHUTDOWN:
        case CONTINUED_HARD_SHUTDOWN:
            return ANSI_RED;

            // --- BLAU: Sleep ---
        case SLEEP_START:
        case CONTINUED_SLEEP:
            return ANSI_SKYBLUE;

            // --- PINK: Hibernate ---
        case HIBERNATE_START:
        case CONTINUED_HIBERNATE:
            return ANSI_HOTPINK;

            // --- DEEP PINK: SysRq ---
        case MAGIC_SYSRQ:
        case CONTINUED_SYSRQ:
            return ANSI_DEEPPINK;

        default:
            return ANSI_RESET;
    }
}

class JournalCLI {
public:
    static void Run(JournalBackend& backend, const std::string& arg);
};

#endif