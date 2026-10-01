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

#include <chrono>

#include "timehelpers.h"

//helper

string usToYYYY_timestamp(uint64_t timestamp_us) { // using systems local time
    time_t tt = timestamp_us / 1000000;
    tm* local_tm = localtime(&tt); // Nutzt die lokale Systemzeit

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_tm);
    return string(buffer);
}

string usToYYYYus_timestamp(uint64_t timestamp_us) {
    time_t tt = timestamp_us / 1000000;
    uint32_t micros = timestamp_us % 1000000;
    tm* local_tm = localtime(&tt); // Nutzt die lokale Systemzeit

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_tm);
    char out[48];
    snprintf(out, sizeof(out), "%s.%06u", buffer, micros);
    return string(out);
}

string beautiful_timestamp(uint64_t timestamp_us) { // using systems local time
    time_t tt = timestamp_us / 1000000;
    tm* local_tm = localtime(&tt); // Nutzt die lokale Systemzeit

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S %d/%m/%y", local_tm);
    return string(buffer);
}
//
// string today_Date() { // z.b. 2026-03-11
//     time_t t = time(nullptr);
//     tm tm;
//     localtime_r(&t, &tm); // thread-safe POSIX
//     ostringstream oss;
//     oss << put_time(&tm, "%Y-%m-%d");
//     return oss.str();
// }

string today_Date_h() { // z.b. 11/03/2026
    time_t t = time(nullptr);
    tm tm;
    localtime_r(&t, &tm); // thread-safe POSIX
    ostringstream oss;
    oss << put_time(&tm, "%d/%m/%Y");
    return oss.str();
}

// Konvertiert 11/03/2026 zu 2026-03-11 fuer journalctl
string to_journal_date(const string& dd_mm_yyyy) {
    tm tm = {};
    tm.tm_isdst = -1; // OS entscheidet ueber Sommer-/Winterzeit
    istringstream ss(dd_mm_yyyy);
    ss >> get_time(&tm, "%d/%m/%Y");
    ostringstream oss;
    oss << put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

// Tag um delta verschieben (Navigieren)
string addDaysToDate(const string& dd_mm_yyyy, int delta) {
    tm tm = {};
    tm.tm_isdst = -1; // OS entscheidet ueber Sommer-/Winterzeit
    istringstream ss(dd_mm_yyyy);
    ss >> get_time(&tm, "%d/%m/%Y");

    tm.tm_mday += delta;
    mktime(&tm); // mktime normalisiert das Datum (z.B. 32. Maerz -> 1. April)

    ostringstream out;
    out << put_time(&tm, "%d/%m/%Y");
    return out.str();
}

// z.b. 01:23:53 (d.h. 1h 23min 53sec)
string us_to_h_m_s(uint64_t us) {
    auto s = chrono::duration_cast<chrono::seconds>(chrono::microseconds(us));
    int h = s.count() / 3600;
    int m = (s.count() % 3600) / 60;
    int sec = s.count() % 60;
    char buf[20];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, sec);
    return string(buf);
}

string us_to_d_h(uint64_t us) {
    auto s = chrono::duration_cast<chrono::seconds>(chrono::microseconds(us));
    int d = s.count() / 86400;
    int h = (s.count() % 86400) / 3600;
    char buf[20];
    snprintf(buf, sizeof(buf), "%dd %dh", d, h);
    return string(buf);
}

// Hilfsfunktion: Wandelt Datum und Zeit in Microsekunden um
// uint64_t dateTimeToUs(const string& date_dd_mm_yyyy, const string& time) {
//     tm tm = {};
//     tm.tm_isdst = -1; // OS entscheidet ueber Sommer-/Winterzeit
//     istringstream ss(date_dd_mm_yyyy + " " + time);
//     ss >> get_time(&tm, "%d/%m/%Y %H:%M:%S");
//     return static_cast<uint64_t>(mktime(&tm)) * 1000000ULL;
// }

// Findet den Montag (Datum als String) der Woche, in der das uebergebene Datum liegt
string get_monday_of_week(const string& dd_mm_yyyy) {
    tm tm = {};
    tm.tm_isdst = -1;
    istringstream ss(dd_mm_yyyy);
    ss >> get_time(&tm, "%d/%m/%Y");
    mktime(&tm); // mktime fuellt tm.tm_wday (0 = Sonntag, 1 = Montag, ..., 6 = Samstag)

    int days_to_subtract = (tm.tm_wday == 0) ? 6 : tm.tm_wday - 1;
    return addDaysToDate(dd_mm_yyyy, -days_to_subtract);
}

// Gibt die Kalenderwoche (ISO-8601) als Zahl zurueck
int get_week_number(const string& dd_mm_yyyy) {
    tm tm = {};
    tm.tm_isdst = -1;
    istringstream ss(dd_mm_yyyy);
    ss >> get_time(&tm, "%d/%m/%Y");
    mktime(&tm);

    char buffer[4];
    strftime(buffer, sizeof(buffer), "%V", &tm); // %V ist ISO-8601 Wochennummer
    return stoi(buffer);
}

// Findet das Datum des Montags einer bestimmten Kalenderwoche
string date_from_cw(int cw, string year) {
    if (year.length() == 2) year = "20" + year;
    if (year.empty()) year = today_Date_h().substr(6, 4);

    tm tm = {};
    tm.tm_year = stoi(year) - 1900;
    tm.tm_mday = 4; // Der 4. Januar ist laut ISO immer in der ersten Woche
    tm.tm_mon = 0;
    tm.tm_isdst = -1;
    mktime(&tm);

    // Zum Montag der Woche 1 navigieren
    int days_to_subtract = (tm.tm_wday == 0) ? 6 : tm.tm_wday - 1;

    // Dann (cw - 1) Wochen hinzufuegen
    string start_jan_4 = to_string(tm.tm_mday) + "/01/" + year;
    string first_monday = addDaysToDate(start_jan_4, -days_to_subtract);

    return addDaysToDate(first_monday, (cw - 1) * 7);
}

// Hilfsfunktion: Wandelt "DD/MM/YYYY" in Mikrosekunden seit Epoch (00:00:00 Uhr) um
uint64_t to_start_of_day_us(const string& date_dd_mm_yyyy) {
    tm tm = {};
    tm.tm_isdst = -1; // OS entscheidet ueber Sommer-/Winterzeit
    istringstream ss(date_dd_mm_yyyy);
    ss >> get_time(&tm, "%d/%m/%Y");
    auto tp = chrono::system_clock::from_time_t(mktime(&tm));
    return chrono::duration_cast<chrono::microseconds>(tp.time_since_epoch()).count();
}

// Hilfsfunktion: Gibt den aktuellen Timestamp in us zurueck (wichtig fuer Live-Session)
uint64_t now_us() {
    return chrono::duration_cast<chrono::microseconds>(
        chrono::system_clock::now().time_since_epoch()).count();
}

// Hilfsfunktion: Ist das Datum in der Zukunft?
bool isFuture(const string& date_h) {
    tm tm = {};
    tm.tm_isdst = -1; // OS entscheidet ueber Sommer-/Winterzeit
    istringstream ss(date_h);
    ss >> get_time(&tm, "%d/%m/%Y");
    auto target = chrono::system_clock::from_time_t(mktime(&tm));
    return target > chrono::system_clock::now();
}

uint64_t parseTextDateToUs(const string& date_str) {
    tm t = {};
    // Parst exakt das Format: 2024-02-06 14:00:00
    if (sscanf(date_str.c_str(), "%4d-%2d-%2d %2d:%2d:%2d",
               &t.tm_year, &t.tm_mon, &t.tm_mday,
               &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        t.tm_isdst = -1; // Wichtig: Ueberlaesst Sommer-/Winterzeit dem System
        time_t time = mktime(&t); // mktime rechnet lokale Zeit in Epoch um
        return static_cast<uint64_t>(time) * 1000000ULL;
               }
    return 0;
}