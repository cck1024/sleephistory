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

#ifndef SLEEPHISTORY_HELP_H
#define SLEEPHISTORY_HELP_H

#include <string>

#include "JournalCLI.h"
#include "Colors.h"
#include "Requirements.h"

static constexpr auto VERSION = "1.0.5";
static constexpr auto BUILD = "2026-10-01";

inline std::string get_help_text() {
    return
    ANSI_BOLD + ANSI_CYAN + "sleephistory - Monitor computer usage\n" + ANSI_RESET +
    "============================================================\n\n" +

    ANSI_BOLD + "TLDR / USAGE\n" + ANSI_RESET +
    "  sleephistory           " + ANSI_GREEN + "Start TUI\n" + ANSI_RESET +
    "  sleephistory [arg]     " + ANSI_GREEN + "CLI output\n\n" +

    ANSI_RESET + ANSI_BOLD + "CLI ARGUMENTS\n" + ANSI_RESET +
    "  <NUMBER>                 Shows output for NUMBER sessions ago (Example: " + ANSI_DIM + "0" + ANSI_RESET + " for current session or " + ANSI_DIM + "1" + ANSI_RESET + " for last session)\n" +
    "  <DATE>                   Shows output for a specific date (see DATE FORMATS below)\n" +
    "  --markdown, -md          Parse output in Markdown format\n" +
    "  -c0, -c1, -c2            Force Color Mode (Mono, Basic, Rich)\n" +
    "  --help, -h               Shows this help page\n" +
    "  --version, -v            Shows app version and build date\n\n" +


    ANSI_BOLD + "TUI CONTROLS (Interactive Mode)\n" + ANSI_RESET +
    "  " + ANSI_BOLD + "Left / Right" + ANSI_RESET + "  or " + ANSI_BOLD + "a / d" + ANSI_RESET + "     Navigate between days, weeks or sessions\n" +
    "  " + ANSI_BOLD + "Up / Down" + ANSI_RESET + "     or " + ANSI_BOLD + "w / s" + ANSI_RESET + "     Scroll through events\n" +
    "  " + ANSI_BOLD + "t" + ANSI_RESET + "                          Toggle between " + ANSI_CYAN + "Session View" + ANSI_RESET + " and " + ANSI_CYAN + "Time View (Day/Week)\n" + ANSI_RESET +
    "  " + ANSI_BOLD + "v" + ANSI_RESET + "                          Toggle between " + ANSI_CYAN + "Day View" + ANSI_RESET + " and " + ANSI_CYAN + "Week View\n" + ANSI_RESET +
    "  " + ANSI_BOLD + "g" + ANSI_RESET + "                          Toggle between " + ANSI_CYAN + "List View" + ANSI_RESET + " and " + ANSI_CYAN + "Graph View\n" + ANSI_RESET +
    "  " + ANSI_BOLD + ":" + ANSI_RESET + "                          Switch to "  + ANSI_CYAN + "Vim Mode\n" + ANSI_RESET +
    "  " + ANSI_BOLD + "CTRL+C" + ANSI_RESET + "                     Quit the application immediately\n\n" +

    ANSI_BOLD + "VIM MODE\n" + ANSI_RESET +
    "  sleephistory offers a Vim-like command mode in the TUI.\n" +
    "  Press " + ANSI_BOLD + "':'" + ANSI_RESET + " in normal mode to enter. Press " + ANSI_BOLD + "Esc" + ANSI_RESET + " to exit VIM Mode, or " + ANSI_BOLD + "Enter" + ANSI_RESET + " to apply a command.\n\n" +
    "  " + ANSI_BOLD + "c" + ANSI_RESET + "                  Copy current screen to clipboard in Markdown format\n" +
    "  " + ANSI_CYAN + "TIP:" + ANSI_RESET + " To copy text as displayed, hold Shift, select the text, and press CTRL+Shift+C.\n" +
    "  " + ANSI_BOLD + "w [filename]" + ANSI_RESET + "       Write current screen as a Markdown file (see EXPORT below)\n" +
    "  " + ANSI_BOLD + "<NUMBER>" + ANSI_RESET + "           Jump directly to a specific session\n" +
    "  " + ANSI_BOLD + "<DATE>" + ANSI_RESET + "             Jump directly to a specific date\n" +
    "  " + ANSI_BOLD + "<CW>" + ANSI_RESET + "               Jump to a Calendar Week (e.g. 'cw12' or 'kw12.25')\n" +
    "  " + ANSI_BOLD + "h" + ANSI_RESET + "                  Show TUI help page\n" +
    "  " + ANSI_BOLD + "q" + ANSI_RESET + "                  Quit the application\n\n" +

    ANSI_BOLD + "EXPORT & SAVING\n" + ANSI_RESET +
    "  " + ANSI_BOLD + "w" + ANSI_RESET + "                  Save with auto-generated name\n" +
    "                     " +"(e.g. " + ANSI_DIM + "sleephistory_2026-03-12.md"+ ANSI_RESET +" or "+ANSI_DIM+"sleephistory_2026-03-15_12-30-12_to_2026-03-18_21-08-29.md"+ANSI_RESET+")\n" +
    "  " + ANSI_BOLD + "w [path]" + ANSI_RESET + "           Save to a custom filename in current directory ($PWD) (e.g. "+ANSI_DIM+" w mylog.txt"+ANSI_RESET+") or absolute path "+ANSI_DIM+"w /tmp/log.md"+ANSI_RESET+")\n\n" +
    "  " + ANSI_CYAN + "Note:" + ANSI_RESET + " For security, sleephistory " + ANSI_BOLD + "will not overwrite" + ANSI_RESET + " existing files.\n" +
    "  If you see an error like 'Access denied', check your folder permissions.\n\n" +

    ANSI_BOLD + "DATE FORMATS\n" + ANSI_RESET +
    "  Supported formats for CLI arguments and TUI Vim mode.\n" +
    "  Leading zeros can be omitted (e.g., '5.4' instead of '05.04').\n\n" +
    "  " + ANSI_BOLD + "DD.MM" + ANSI_RESET + "              Date in current year  (Example: " + ANSI_DIM + "25.4" + ANSI_RESET + " or " + ANSI_DIM + "02.11" + ANSI_RESET + ")\n" +
    "  " + ANSI_BOLD + "DD.MM.[YY]YY" + ANSI_RESET + "       Specific date         (Example: " + ANSI_DIM + "25.04.2025" + ANSI_RESET + " or " + ANSI_DIM + "25.04.25" + ANSI_RESET + ")\n" +
    "  " + ANSI_BOLD + "DD/MM" + ANSI_RESET + "              Date in current year  (Example: " + ANSI_DIM + "25/4" + ANSI_RESET + " or " + ANSI_DIM + "02/11" + ANSI_RESET + ")\n" +
    "  " + ANSI_BOLD + "DD/MM/[YY]YY" + ANSI_RESET + "       Specific date         (Example: " + ANSI_DIM + "25/04/2025" + ANSI_RESET + " or " + ANSI_DIM + "25/04/25" + ANSI_RESET + ")\n\n" +
    "  " + ANSI_BOLD + "cw[KW]" + ANSI_RESET + "             Week in current year  (Example: " + ANSI_DIM + "cw20" + ANSI_RESET + " or " + ANSI_DIM + "kw20" + ANSI_RESET + ")\n" +
    "  " + ANSI_BOLD + "cw[KW].[YY]YY" + ANSI_RESET + "      Week in specific year (Example: " + ANSI_DIM + "cw10.2025" + ANSI_RESET + " or " + ANSI_DIM + "kw10.25" + ANSI_RESET + ")\n" +
    "  " + ANSI_BOLD + "cw[KW]/[YY]YY" + ANSI_RESET + "      Week in specific year (Example: " + ANSI_DIM + "cw10/2025" + ANSI_RESET + " or " + ANSI_DIM + "kw10/25" + ANSI_RESET + ")\n\n" +

    ANSI_BOLD + "LEGEND & STATUS SYMBOLS\n" + ANSI_RESET +
    "  " + ANSI_GREEN + "[POWERED ON]" + ANSI_RESET + "     System boot or log start\n" +
    "  " + ANSI_GREEN + "[ACTIVE]" + ANSI_RESET + "         System is running (after wake-up)\n" +
    "  " + ANSI_ORANGE + "[POWERED OFF]" + ANSI_RESET + "    Clean shutdown\n" +
    "  " + ANSI_RED + "[HARD SHUTDOWN]" + ANSI_RESET + "  Unclean shutdown, crash, or power loss\n" +
    "  " + ANSI_SKYBLUE + "[SLEEP]" + ANSI_RESET + "          System suspended to RAM (Standby)\n" +
    "  " + ANSI_HOTPINK + "[HIBERNATE]" + ANSI_RESET + "      System suspended to disk\n" +
    "  " + ANSI_DEEPPINK + "[MAGIC SYSRQ]" + ANSI_RESET + "    Emergency kernel command issued\n\n" +

    ANSI_BOLD + "TERMINAL COLOR MODES" + ANSI_RESET + "\n" +
    "  sleephistory automatically detects your terminal's color support, but you can also force a mode.\n" +
    "  " + ANSI_BOLD + "-c0" + ANSI_RESET + "  Monochrome: No colors, plain text\n" +
    "  " + ANSI_BOLD + "-c1" + ANSI_RESET + "  Basic:      8 colors\n" +
    "  " + ANSI_BOLD + "-c2" + ANSI_RESET + "  Rich:       256 colors (Default for modern terminals)\n\n" +

    ANSI_BOLD + "HOW TO READ THE OUTPUT\n" + ANSI_RESET +
    "  " + ANSI_DIM + "- Carried Over" + ANSI_RESET + "   Status from the previous day (e.g., if PC stayed on)\n" +
    "  " + ANSI_DIM + "- End of Day" + ANSI_RESET + "     Status at midnight (marks the transition)\n" +
    "  " + ANSI_DIM + "- End of Week" + ANSI_RESET + "    Status at Sunday 23:59:59 (Week View transition)\n" +
    "  " + ANSI_DIM + "- Log Start" + ANSI_RESET + "      Journal logs begin here (earlier data rotated)\n\n" +

    "  The time shown on the left is the exact moment the event was recorded.\n" +
    "  The duration in the status line indicates how long the system stayed\n" +
    "  in that specific state.\n\n" +

    "============================================================\n" +
    ANSI_CYAN + "TIP:" + ANSI_RESET + " You may pipe this help page into 'more' or 'less' to scroll in a TTY environment\n" +
    "     (i.e. "+ ANSI_DIM +"sleephistory -h | more"+ ANSI_RESET + " or "+ ANSI_DIM +"sleephistory -h | less -R"+ ANSI_RESET+")\n";
}

static std::vector<std::string> tui_help = {
	"sleephistory - Monitor computer usage",
	"============================================================",
	"TUI CONTROLS (Interactive Mode)",
	"  Left / Right  or a / d     Navigate between days, weeks or sessions",
	"  Up / Down     or w / s     Scroll through events",
	"  t                          Toggle between Session View and Time View (Day/Week)",
	"  v                          Toggle between Day View and Week View",
	"  g                          Toggle between List View and Graph View",
	"  :                          Switch to Vim Mode",
	"  CTRL+C                     Quit the application immediately",
	"",
	"VIM MODE",
	"  sleephistory offers a Vim-like command mode in the TUI.",
	"  Press ':' in normal mode to enter. Press Esc to exit VIM Mode, or Enter to apply a command.",
	"",
	"  The quick keys 't', 'v' and 'g' work here as well!",
	"  Layout Controls:",
	"    g or g[num]   Change Layout Mode:",
	"                  g0 = List View only",
	"                  g1 = Split View (Horizontal)",
	"                  g2 = Split View (Vertical)",
	"                  g3 = Graph View only",
	"",
	"  Navigation:",
	"    <NUMBER>           Jump directly to a specific session",
	"    <DATE>             Jump directly to a specific date",
	"    <CW>               Jump to a Calendar Week (e.g. 'cw12' or 'kw12.25')",
	"  Actions:",
	"    c                  Copy current screen to clipboard in Markdown format",
	"  TIP: To copy text as displayed, hold Shift, select the text, and press CTRL+Shift+C.",
	"    w [filename]       Write current screen as a Markdown file (see EXPORT below)",
	"    h                  Shows this help page",
	"    q                  Quit the application",
	"",
	"EXPORT & SAVING",
	"  w                  Save with auto-generated name",
	"                     (e.g. sleephistory_2026-03-12.md or sleephistory_2026-03-15_12-30-12_to_2026-03-18_21-08-29.md)",
	"  w [path]           Save to a custom filename in current directory ($PWD) (e.g.  w mylog.txt) or absolute path w /tmp/log.md)",
	"",
	"  Note: For security, sleephistory will not overwrite existing files.",
	"  If you see an error like 'Access denied', check your folder permissions.",
	"",
	"DATE FORMATS",
	"  Supported formats for CLI arguments and TUI Vim mode.",
	"  Leading zeros can be omitted (e.g., '5.4' instead of '05.04').",
	"",
	"  DD.MM              Date in current year  (Example: 25.4 or 02.11)",
	"  DD.MM.[YY]YY       Specific date         (Example: 25.04.2025 or 25.04.25)",
	"  DD/MM              Date in current year  (Example: 25/4 or 02/11)",
	"  DD/MM/[YY]YY       Specific date         (Example: 25/04/2025 or 25/04/25)",
	"",
	"  cw[KW]             Week in current year  (Example: cw20 or kw20)",
	"  cw[KW].[YY]YY      Week in specific year (Example: cw10.2025 or kw10.25)",
	"  cw[KW]/[YY]YY      Week in specific year (Example: cw10/2025 or kw10/25)",
	"",
	"LEGEND & STATUS SYMBOLS",
	"  [POWERED ON]     System boot or log start",
	"  [ACTIVE]         System is running (after wake-up)",
	"  [POWERED OFF]    Clean shutdown",
	"  [HARD SHUTDOWN]  Unclean shutdown, crash, or power loss",
	"  [SLEEP]          System suspended to RAM (Standby)",
	"  [HIBERNATE]      System suspended to disk",
	"  [MAGIC SYSRQ]    Emergency kernel command issued",
	"",
	"HOW TO READ THE OUTPUT",
	"  - Carried Over   Status from the previous day (e.g., if PC stayed on)",
	"  - End of Day     Status at midnight (marks the transition)",
	"  - End of Week    Status at Sunday 23:59:59 (Week View transition)",
	"  - Log Start      Journal logs begin here (earlier data rotated)",
	"",
	"  The time shown on the left is the exact moment the event was recorded.",
	"  The duration in the status line indicates how long the system stayed",
	"  in that specific state.",
	"",
	"TIP: sleephistory offers a CLI.",
	"     For a complete help documentation: $ sleephistory -h"
};

inline std::string get_version_text(bool mono=false) {
	std::stringstream ss;

	ss << "sleephistory " << VERSION << "\n";
	ss << "build date " << BUILD << "\n";

	int v = get_systemd_version();
	if (mono) {
		ss << "journalctl (systemd): "
		   << (v == -1 ? "NOT FOUND" : "version " + std::to_string(v))
		   << " (min. " << MIN_SYSTEMD_VERSION << "+ required)";
	} else {
		ss << "journalctl (systemd): "
		   << (v == -1 ? ANSI_RED + "NOT FOUND" : "version " + std::to_string(v))
		   << ANSI_RESET << " (min. " << MIN_SYSTEMD_VERSION << "+ required)";
	}

	return ss.str();
}

#endif //SLEEPHISTORY_HELP_H