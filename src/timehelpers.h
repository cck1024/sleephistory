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
#ifndef SLEEPHISTORY_TIMEHELPERS_H
#define SLEEPHISTORY_TIMEHELPERS_H
#include <iomanip>
#include <cstdint>

//helper

string usToYYYY_timestamp(uint64_t timestamp_us);
string usToYYYYus_timestamp(uint64_t timestamp_us);
string beautiful_timestamp(uint64_t timestamp_us);
string today_Date_h();

// Konvertiert 11/03/2026 zu 2026-03-11 fuer journalctl
string to_journal_date(const string& dd_mm_yyyy);
// Tag um delta verschieben (Navigieren)
string addDaysToDate(const string& dd_mm_yyyy, int delta);
// z.b. 01:23:53 (d.h. 1h 23min 53sec)
string us_to_h_m_s(uint64_t us);
string us_to_d_h(uint64_t us);
// Findet den Montag (Datum als String) der Woche, in der das uebergebene Datum liegt
string get_monday_of_week(const string& dd_mm_yyyy);
// Gibt die Kalenderwoche (ISO-8601) als Zahl zurueck
int get_week_number(const string& dd_mm_yyyy);
// Findet das Datum des Montags einer bestimmten Kalenderwoche
string date_from_cw(int cw, string year);
// Hilfsfunktion: Wandelt "DD/MM/YYYY" in Mikrosekunden seit Epoch (00:00:00 Uhr) um
uint64_t to_start_of_day_us(const string& date_dd_mm_yyyy);
// Hilfsfunktion: Gibt den aktuellen Timestamp in us zurueck (wichtig fuer Live-Session)
uint64_t now_us();
uint64_t parseTextDateToUs(const string& date_str);
// Hilfsfunktion: Ist das Datum in der Zukunft?
bool isFuture(const string& date_h);

#endif //SLEEPHISTORY_TIMEHELPERS_H