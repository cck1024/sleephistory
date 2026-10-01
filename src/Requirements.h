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

#ifndef SLEEPHISTORY_REQUIREMENTS_H
#define SLEEPHISTORY_REQUIREMENTS_H
#include <cstdio>
#include <string>
#include <array>
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <vector>

static constexpr auto MIN_SYSTEMD_VERSION = 237;

inline int get_systemd_version() {
    std::array<char, 128> buffer;
    std::string result;

    // Nutze popen wie in execCommand Logik
    FILE* pipe = popen("journalctl --version 2>/dev/null", "r");
    if (!pipe) return -1;

    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result = buffer.data();
    }
    pclose(pipe);

    size_t pos = result.find("systemd ");
    if (pos != std::string::npos) {
        try {
            // Extrahiere den Teil nach "systemd " und wandle ihn in int um
            return std::stoi(result.substr(pos + 8));
        } catch (...) { return -1; }
    }
    return -1;
}

inline bool hasJournalPermissions() { // sleephistory requires it's user to be in group adm or systemd-journal to be used
    if (geteuid() == 0) return true; //root

    // Hilfsfunktion, um die GID zu einem Gruppennamen zu finden
    auto getGidByName = [](const std::string& name) -> gid_t {
        struct group* gr = getgrnam(name.c_str());
        return gr ? gr->gr_gid : static_cast<gid_t>(-1);
    };

    gid_t admGid = getGidByName("adm");
    gid_t journalGid = getGidByName("systemd-journal");

    // Falls keine der Gruppen auf dem System existiert, hat der User auch keine Rechte
    if (admGid == static_cast<gid_t>(-1) && journalGid == static_cast<gid_t>(-1)) {
        return false;
    }

    // Pruefen, ob die primaere/effektive Gruppe bereits matcht
    gid_t egid = getegid();
    if (egid == admGid || egid == journalGid) return true;

    // Alle sekundaeren Gruppen des Benutzers abfragen
    int numGroups = getgroups(0, nullptr);
    if (numGroups > 0) {
        std::vector<gid_t> groups(numGroups);
        if (getgroups(numGroups, groups.data()) != -1) {
            for (gid_t gid : groups) {
                if ((admGid != static_cast<gid_t>(-1) && gid == admGid) ||
                    (journalGid != static_cast<gid_t>(-1) && gid == journalGid)) {
                    return true;
                    }
            }
        }
    }

    return false;
}

#endif //SLEEPHISTORY_REQUIREMENTS_H