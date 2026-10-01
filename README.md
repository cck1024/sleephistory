## sleephistory - Monitor computer usage

This README documents the following:

1. [Overview](#1-overview) 

2. [Requirements](#2-requirements)

3. [Install, Update, Uninstall](#3-install-update-uninstall)

4. [Usage (Commands)](#4-usage)

5. [Internal Architecture](#5-internal-architecture)

6. [License](#6-License)

# 1. Overview

## A. What's SleepHistory?

![SleepHistory Session View](/assets/screenshots/session_view_1.png)

SleepHistory is a tool which offers both a **CLI** and **TUI** to break down computer usage. This means displaying when the computer was:

- active

- sleeping

- hibernating

- turned off

- shut down improperly

When reffering to the "computer" we only mean the usage of the operating system that is running SleepHistory. This means if you use another Operating System by dual booting, these times will all be interpreted as "turned off" as they wont be logged by the journal.

## B. Core Features

- Views:
  
  - Day view
  
  - Week view
  
  - Session view

- Graph view

- Intuitive VIM Style control

- Jumping to any Session, Day, Calendar Week or Date

- Exporting or Copying to Clipboard as markdown file

## C. Screenshots
### TUI Views

|               Day View & Graph (horizontally split)                |                  Week View & Graph (vertically split)                    |
|:---------------------------------------------:|:-----------------------------------------------:|
| ![Day View](/assets/screenshots/current_day_view.png) |  ![Session View](/assets/screenshots/week_view.png)   |
|     *Day View of current Day*      | *Week view of a week which has already passed* |

SleepHistory can be used to determine when and how your system crashed the last time and what its last log message was


|               Day View Hard Shutdown               |                  Day View Magic Sysrq                   |
|:---------------------------------------------:|:-----------------------------------------------:|
| ![Day View Crash](/assets/screenshots/day_view_crash.png) |  ![Day View sysrq](/assets/screenshots/day_view_sysrq.png)   |
|     *In this example the system crashed and 4:35 later was started again*      | *In this example the system was force rebooted with the Magic sysrq Key combination. 52 Seconds later the system was up and running again.* |


### CLI Views


|               Day View CLI               |                  Day View CLI as Markdown                   |
|:---------------------------------------------:|:-----------------------------------------------:|
| ![Day View CLI](/assets/screenshots/day_view_cli.png) |  ![Day View CLI Markdown](/assets/screenshots/day_view_cli-md.png)   |
|     *If you pass a Number, Week or Date SleepHistory automatically works as a CLI program*      | *In CLI mode its also possible to output directly in markdown* |


# 2. Requirements

- Linux system with systemd (**version >=237**)

- User needs permission to completely read journalctl. This is granted if either
  
  - user is _root_
  - user is in group _systemd-journal_ OR
  - user is in group _adm_

- To understand **WHY** sleephistory requires systemd, read [Section 5. Internal Architecture](#5-internal-architecture)

# 3. Install, Update, Uninstall

- The Quickstart guide explains this in more depth. 
  - [(EN) Quickstart Guide](Quickstart-Guide-ReadMe.txt)
  - [(DE) Schnellstart Anleitung](Schnellstart-Anleitung-LiesMich.txt)

- The installer and uninstaller scripts are interactive scripts. They don't do anything **before asking you** about it beforehand.

## Install

Before installing, clone this repo with 

```bash
git clone https://github.com/cck1024/sleephistory
```

Now you can enter this directory and proceed.

---

sleephistory can be installed by 

1. using the precompiled binary or 

2. compiling it yourself

In both cases ONLY the installer script (*installer.sh*) should be invocated.

You DON'T have to invocate *native-build.sh* yourself, the installer script will call it.

## Update

TIP: You can find out your current version and build date (this is when the version was released, not the date you installed) by running

```bash
sleephistory -v
```

---

1. Enter your local copy of this repo and open a terminal

2. ```bash
   git pull
   ```

3. Now just run *installer.sh*, it will ask you if you want to install it locally or global, if you don't remember how it was installed, you can check it beforehand by running

```bash
$ which -a sleephistory
/home/<your-username>/.local/bin/sleephistory # this means sleephistory 
# is locally installed
/usr/local/bin/sleephistory # this means sleephistory is installed 
# globally.
# If both appear like here you have installed it both locally and global.
# You may want to remove one version of it by running the uninstaller to
# avoid confusion
```

## Uninstall

To uninstall you can run the uninstall script.

```bash
./uninstaller.sh
```

### Cache

Whether you install sleephistory or not, the moment you start the program it creates a cache in your users .cache directory:

```bash
$ ls -l ~/.cache/sleephistory/
total 25
-rw-rw-r-- 1 user user  2814 Sep 28 21:07  boot_ids
drwxrwxr-x 2 user user  4096 Sep 21 20:01  sessions
```

- The uninstall script does NOT touch this directory.

- This directory should NOT be removed, as it makes sleephistory persistent to journal rotations.

- Only remove this cache if you are okay with losing this persistence. 

- [Learn more about caching](#caching)

### Local installation files

The local installation creates the following files:

```bash
~/.local/bin/sleephistory # the binary itself
~/.local/share/icons/sleephistory.png # the app icon
~/.local/share/applications/sleephistory.desktop # this makes it a searchable, startable app
```

### Global installation files

The global installation creates the following files:

```bash
/usr/local/bin/sleephistory # the binary itself
/usr/share/pixmaps/sleephistory.png # the app icon
/usr/share/applications/sleephistory.desktop # this makes it a searchable, startable app
```

# 4. Usage

sleephistorys usage manual is documented in its help page. Just run:

```bash
sleephistory -h
```

The TUI specific help page can be reached by 

1. starting `sleephistory` (This starts sleephistory's interactive TUI mode)

2. Pressing `:h` and then pressing Enter to send (`:` starts VIM command mode, `h` stands for help)

## TUI Examples

Starting normally:

- By using the Desktop Shortcut

- In Terminal:

```bash
sleephistory
```

Starting in monochrome color mode:

```bash
sleephistory -c0
```

## TUI Navigation

### Event Modes

You can switch between different views using shortcut keys:

- Press `v` to toggle between **Day View** and **Week View**.
- Press `t` to toggle in and out of **Session View**.

```text
       ┌─────────────┐               ┌─────────────┐
       │  Day View   │ ◄─── [v] ───► │  Week View  │
       │  (Default)  │               │             │
       └──────┬──────┘               └──────┬──────┘
              │                             │
             [t]                           [t]
              │                             │
              ▼                             ▼
       ┌───────────────────────────────────────────┐
       │               Session View                │
       └───────────────────────────────────────────┘
```

### Graph Modes

Press `g` to cycle through graph modes, or use VIM shortcuts (`g0`–`g3`) to jump directly to a layout:

```text
       ┌─── [g] ───► [g1] Horizontal ─── [g] ───► [g2] Vertical ─── [g] ────────────┐
       │                                                                            │
       ▼                                                                            ▼
┌──────────────┐          ┌──────────────┐          ┌──────┬──────┐          ┌──────────────┐
│   Log View   │          │   Log View   │          │ Log  │Graph │          │  Graph View  │
│  (Full View) │          ├──────────────┤          │      │      │          │  (Full View) │
│              │          │    Graph     │          │      │      │          │              │
└──────────────┘          └──────────────┘          └──────┴──────┘          └──────────────┘
  [g0] No Graph                                                              [g3] Graph Only
       ▲                                                                            │
       └────────────────────────── [g] ◄────────────────────────────────────────────┘
```

## CLI Examples

- Here are just some examples, there are way more options and permutations of these possible.

History of current session

```bash
sleephistory 0
```

History of last session, in markdown format

```bash
sleephistory 1 -md
```

History of a specific date THIS year

```bash
sleephistory 20.5 # 20th may this year
```

History of a specific Calendar week in a specific year

```bash
sleephistory cw10.25 # calendar week 10 of 2025
```

History of the session 8 sessions before the current session in reduced color mode:

```bash
sleephistory 8 -c1 # c1 enforces basic color mode (8 colors)
```

Piping the markdown output to pandoc for better plain formating:

```bash
sleephistory cw30 -md | pandoc -t plain
```

# 5. Internal Architecture

## Backend

SleepHistory works by using `journalctl` in the backend, which is part of **systemd**.

This means that SleepHistory only works on Linux Computers which use the systemd init system.

The backend uses `journalctl` by just calling it and interpreting it's output. The following commands will be called when using sleephistory, requiring atleast **systemd version 237** or greater.

#### Overview of `journalctl` Invocations

All commands are built dynamically using a common base:

`string base_cmd = journalctl --output=json -b <session_id> [--since="<timestamp>"] 2>/dev/null`

| **No.** | **Invocation (Composition)**                                                   | **Reason**                                                                                | **Parameters Used**                                                                                           | **Minimum systemd Version**                                          |
| ------- | ------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------- |
| 0       | `journalctl --output=json -b <session_id> [--since="<timestamp>"] 2>/dev/null` | Gets log in json format from the requested boot number.                                   | `-b` (boot selection), `--output=json` (gives output as json), `--since` (optimization (only loads new logs)) | systemd 186 (-b), systemd 206 (--output=json), systemd 195 (--since) |
| 1       | `base_cmd + " -n 1"`                                                           | Existence check: Verifies if logs exist for the specified boot. Retrieves last log entry. | `-n 1` (last line)                                                                                            | systemd 205                                                          |
| 2       | `base_cmd + " \| head -n 1"`                                                   | Retrieves the first log entry of the boot process (start time).                           | `\| head -n 1` (piping)                                                                                       | systemd 205                                                          |
| 3       | `base_cmd + " -kg '^(PM:.*(suspend\|hibernation))'"`                           | Searches for specific PM events (suspend/hibernation) via regex.                          | `-k` (kernel), `-g` (grep)                                                                                    | systemd 220 (-k), systemd 237 (-g)                                   |

### Some explantions:

```bash
$ journalctl --output=short-unix
1772754068.337526 …
```

shows events with unix timestamp.

```bash
$ journalctl --output=short-unix -b -X
```

The arg `-b` allows us to show the output of the X't last session. e.g.

`-b -1` shows the entire journal of last session. This means everything that happend before you shutdown your computer last time.

### Identifying time period of a session

#### Powered On:

This command shows when computer was turned on and which kernel was used.

```bash
$ journalctl --output=short-unix -b -1 | head -1
1772549491.298203 ccmain kernel: Linux version 6.8.0-101-generic …
# effizienter ist es mit -n als arg:
$ journalctl --output=short-unix -b -1 -n +1
```

#### Powered Off:

```bash
$ journalctl --output=short-unix -b -1 -n 1
1772754068.834443 ccmain kernel: PM: suspend entry (deep)
$ journalctl --output=short-unix -b -2 -n 1
1772549438.955812 ccmain kernel: sysrq: Emergency Remount R/O
$ journalctl --output=short-unix -b -3 -n 1
1772233491.376670 ccmain systemd-journald[900]: Journal stopped
# -n 1 is the same as piping it with tail -1, but better performance as 
# journalctl does the filtering. 
$ journalctl --output=short-unix -b -3 | tail -1
1772233491.376670 ccmain systemd-journald[900]: Journal stopped
```

Here are shown three different situations:

`-b -1`: Here the last journal was from going to sleep. This means the computer got shutdown hard (e.g. by unplugging it from a power source while it was sleeping)

`-b -2`: Here the last journal was a sysrq sequence. Also an unclean shutdown as this means the user shutdown the machine using the sysrq sequence.

`-b -3`: This is a **normal shutdown**. Journal was closed properly `Journal stopped`

### Sleep

Sleep can be identified this way:

```bash
$ journalctl --output=short-iso --until "now" -kg '^PM:.*suspend' 
2026-03-07T02:35:42+01:00 ccmain kernel: PM: suspend entry (deep)
2026-03-08T10:49:59+01:00 ccmain kernel: PM: suspend exit
2026-03-08T13:01:40+01:00 ccmain kernel: PM: suspend entry (deep)
2026-03-08T22:37:45+01:00 ccmain kernel: PM: suspend exit
```

## Parsing

We parse using the `json` option:

```bash
journalctl --output=json …
```

## Logging/Debugging

Logging is implemented by using the `LOG_DEBUG()` defined in debugging.h

It gets included and compiled in the binary if `DEBUG_MODE` is true. If not, every line including `LOG_DEBUG()` gets removed by the preprocessor and has no effect on performance.

If the binary gets compiled in debug mode it should be called by redirecting `cerr` in a logfile (or else it affects the TUI).

```bash
$ sleephistory 2> sleephistoryLog.log
```

With `tail -f` it can be then read while using sleephistory

```bash
$ tail -f sleephistoryLog.log
[DEBUG 2026-04-16 14:32:19] starting sleephistory in TUI Mode.
```

## Caching

SleepHistory uses caching to MASSIVELY improve performance and survive journal rotations. It should not be used with disabled cache, although it's possible to disable caching.

Caching requires:

1. A ~/.cache directory which is accessible by executing user
2. Read and Write permission in ~/.cache/sleephistory/
3. ~/.cache/sleephistory/donotcache to NOT exist.

That's right, caching can be DISABLED by creating an empty _donotcache_ file in sleephistory's cache directory.

```bash
$ touch ~/.cache/sleephistory/donotcache
```

# 6. License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.