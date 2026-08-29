# Denkzettel

[![Build and Tests](https://github.com/hnsstrk/denkzettel/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/hnsstrk/denkzettel/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A quick-capture scratchpad for KDE Plasma. Press `Meta+N`, type, press
`Ctrl+Enter` — the note is stored and the window is gone. No file name, no save
dialog, no question where to put it.

🇩🇪 [Deutsche Fassung](README.de.md)

> **The interface is English.** A German translation ships with the source
> (`po/de/denkzettel.po`) and is installed with the program, so a German
> session gets German without any further setup.

![The capture window: a text field with a half-written note, below it the hint "Esc discards · Ctrl+Enter saves"](docs/images/erfassungsfenster.png)

I built Denkzettel because things keep occurring to me while I work that belong
somewhere else: an idea, an open question, a command line I want to keep. If I
have to create a file for that first, the thought is gone. Whatever turns out to
be worth keeping moves into my Obsidian vault later.

I do not write the code myself: **Denkzettel is developed with Claude Code**, I
set the goals, the priorities and the acceptance —
[How this project is run](#how-this-project-is-run).

## What it does

- **Capture with one key press**, without a file name and without a dialog.
  `Meta+N` is registered as a global shortcut through KGlobalAccel; if another
  component already holds it, the first start says so in a notification instead
  of failing silently.
- **A library of all notes**, grouped by day like an inbox: Today, Yesterday,
  This week, Last week, Older. `F2` opens the editor, `Del` deletes with five
  seconds of undo, `Ctrl+Enter` saves an edit and `Esc` cancels it. Leaving an
  edit with unsaved changes asks first.
- **Full-text search** over a SQLite FTS5 trigram index. It matches inside
  words, not just at their start — `grafieren` finds `fotografieren` — and it
  folds diacritics, so `bucher` finds `Bücher`. Terms of one or two characters
  cannot be in a trigram index and fall back to a substring comparison, so `KI`
  finds something instead of nothing. `ß` is not folded; that is a limit of the
  tokenizer and it is covered by a test that says so.
- **Icons and labels come from the system**, and a colour-scheme change is
  picked up while the program runs.
- **The capture window wears the shell of the desktop theme** — rounding,
  outline, shadow, the frame of the input field and its focus layer are drawn
  from the theme, not from built-in values.
- **Runs in the background**, sits in the system tray, starts with the session.
- **Everything stays local** in one SQLite file. Nothing leaves the machine.

![The library: on the left the note list grouped by day, on the right the reading pane](docs/images/bibliothek.png)

### Not built yet

The specification describes considerably more than the program does today. What
is written down and not built:

- **Voice notes** with recording window and transcription (whisper.cpp,
  WhisperX). The tray entry exists and is disabled.
- **AI analysis** — classification, tags, a category sidebar, Ollama and
  OpenAI-compatible providers. The tray entry exists and is disabled.
- **Suggestions** — embeddings, topic clustering, bundle and task proposals with
  a review UI. The tray entry exists and is disabled.
- **Export** to Obsidian and Taskwarrior, and a full export as a way out.
- **A settings dialog.** There is none; what can be configured is configured in
  `denkzettelrc` or not at all.
- **Search operators** (`tag:`, date ranges and the rest of SPEC 6). The search
  takes plain terms today and combines them with AND.

What is currently being worked on is in the
[issues](https://github.com/hnsstrk/denkzettel/issues); the binding
specification is [`SPEC.md`](SPEC.md).

## Requirements

- CMake 3.20 or newer, a C++20 compiler, `extra-cmake-modules`
- Qt 6.7 (DBus, Multimedia, Widgets, Sql) — QtMultimedia needs the **ffmpeg**
  backend (`qt6-multimedia-ffmpeg`); its package dependency is a virtual one
  with two providers, and the gstreamer provider writes a different set of
  formats than the Opus in OGG the voice note is recorded in
- KDE Frameworks 6: ColorScheme, Config, CoreAddons, DBusAddons, GlobalAccel,
  I18n, Notifications, StatusNotifierItem, Svg, WidgetsAddons, WindowSystem
- `ffmpeg`, the program — the transcription converts a recording into a
  temporary 16 kHz mono WAV with it before whisper.cpp sees it, and the test
  run does the same
- gettext (`msgfmt`) for the message catalogues
- AppStream (`appstreamcli`) — the configuration stops without it: the test run
  validates the AppStream description a software centre reads
- libplasma at runtime — it ships the desktop themes the capture window draws
  its shell from — plus Breeze icons
- For the transcription only, and only at runtime: `whisper-cpp` with a GGML
  backend (`ggml-vulkan`, or `ggml-hip` on ROCm) and a model under
  `~/.local/share/denkzettel/models/`. Neither is needed to build or to test —
  the test run puts a program of its own in whisper-cli's place. A run is given
  up on after five minutes, counted over the whole job: Denkzettel takes down
  short notes and is no audio recorder — the recording itself already stops at
  fifteen minutes. Both paths are settings in `denkzettelrc`:

  ```ini
  [Transcription]
  FfmpegProgram=/usr/bin/ffmpeg
  WhisperProgram=/usr/bin/whisper-cli
  ModelPath=/home/you/.local/share/denkzettel/models/ggml-small.bin
  ```

On Arch and derivatives:

```sh
sudo pacman -S --needed cmake extra-cmake-modules gettext ffmpeg qt6-base \
    qt6-multimedia qt6-multimedia-ffmpeg \
    kcolorscheme kconfig kcoreaddons kdbusaddons kglobalaccel ki18n \
    knotifications kstatusnotifieritem ksvg kwidgetsaddons kwindowsystem \
    kxmlgui libplasma breeze-icons appstream
```

## Build and install

```sh
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

Where no terminal can ask for the password, the same step through a graphical
dialog — `pkexec` needs absolute paths for both the program and the build
directory:

```sh
pkexec /usr/bin/cmake --install "$PWD/build"
```

The prefix `/usr` is not cosmetic. Plasma's shortcut service only finds the
action when the desktop file lies system-wide, and the autostart entry is only
read from `/etc/xdg/autostart`, which is where the prefix `/usr` puts it.

Afterwards start `denkzetteld` once or log in again; from then on the autostart
entry does it.

### As a package (Arch and derivatives)

`packaging/PKGBUILD` builds the same state as a package, out of the git tag of
the version it names:

```sh
cd packaging
makepkg -si
```

`makepkg` fetches the missing build dependencies itself, runs the test set, and
`-i` hands the finished package to `pacman`. The prefix is `/usr`, so the
autostart entry lands in `/etc/xdg/autostart` and Plasma's shortcut service
finds the desktop file — the same two conditions the source installation above
has. What the package brings besides the program: the desktop entry, the
autostart entry of the same name, the AppStream description, both icons and the
German catalogue. There is no `.notifyrc` of Denkzettel's own — the warning
sound of the guard dialog is Plasma's `messageWarning` event (SPEC 9).

`whisper-cpp` and a GGML backend stay **optional** and are named as
`optdepends`: without them a voice note keeps its recording and stays playable,
and the reason stands in the job line of the database and in the journal
(`journalctl --user -t denkzetteld`).

`pkgver` is the second place the version number lives. It follows
`project(denkzettel VERSION …)` of the root `CMakeLists.txt` and the tag of the
same number — the two change together, or `makepkg` fetches a different release
than the one the working copy holds.

### Updating from an older version

The application id is `io.github.hnsstrk.denkzettel` — the desktop entry, the
AppStream component and the shortcut component in Plasma all read it. Installing
writes the new files **beside** the old ones instead of replacing them, and the
two that stay behind have to go:

```sh
sudo rm -f /usr/share/applications/org.denkzettel.Denkzettel.desktop \
           /etc/xdg/autostart/org.denkzettel.Denkzettel.desktop
```

Skip that and the session reads **two** autostart entries, because XDG shadows
them by file name and the names now differ. It starts `denkzetteld` twice, the
second start is handed to the running one as an activation, and that is bound to
the capture window — **so the capture window pops up at every login.** Beside
that, two components then hold `Meta+N` and the shortcut settings list
*Denkzettel* twice. The key press itself keeps working throughout: both entries
carry the same `Exec=denkzetteld` and the same action, and whichever of the two
the press reaches ends at the same window. The conflict warning stays silent —
it is only ever shown on a first start, and an installation being updated has
that behind it.

Both symptoms are gone with the two files. The group
`[services][org.denkzettel.Denkzettel.desktop]` stays behind in
`~/.config/kglobalshortcutsrc`, and it can stay: measured on 28.08.2026 in a
session of its own, with the desktop entry unreachable the shortcut service
loads no component from it at all and `Meta+N` has exactly one holder, the new
one. Editing that file by hand is pointless anyway — the running service writes
it back.

A key sequence changed by hand does not travel with the rename: it stands in the
old component's group, and the new one starts on `Meta+N` again.

The D-Bus service name changed with it — scripts calling `org.denkzettel.Daemon`
need one line altered, see [Everyday use](#everyday-use) below.

## Everyday use

`Meta+N` opens the capture window, `Ctrl+Enter` saves, `Esc` discards. The
library is reached from the tray icon.

`denkzetteld --version` says which version is running, `denkzetteld --help`
lists the switches. Both answer while the service is already running.

For scripts there is a D-Bus interface, `io.github.hnsstrk.denkzettel` at
`/Daemon`:

```sh
qdbus6 io.github.hnsstrk.denkzettel /Daemon AddNote "text of the note"
qdbus6 io.github.hnsstrk.denkzettel /Daemon ShowCapture
qdbus6 io.github.hnsstrk.denkzettel /Daemon ShowLibrary
qdbus6 io.github.hnsstrk.denkzettel /Daemon Quit
```

`AddNote` returns the id of the new note, or 0 when nothing was stored.

**Up to version 0.7.0 the service was called `org.denkzettel.Daemon`.** The
daemon now registers this one name and nothing else, so a script written against
the old one gets an error from `qdbus6` or `dbus-send` — and in a background job
nobody reads it. What such a script has to change is the service name; the
object path `/Daemon` and all four method names stay as they are. Where the call
names the interface as well, as `dbus-send --dest=… /Daemon <interface>.<method>`
does, that becomes `io.github.hnsstrk.denkzettel.Daemon`.

The notes live in `~/.local/share/denkzettel/denkzettel.db`. If an update
changes the schema, the existing database is migrated on the first start; what
changes is recorded in the [changelog](CHANGELOG.md).

## Contributing

Bug reports and ideas are welcome as an
[issue](https://github.com/hnsstrk/denkzettel/issues). Whoever wants to
contribute code starts here.

### Build and test

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
```

The tests run offscreen and need no running Plasma session.

### Linters

Two targets that run on demand and change nothing:

```sh
cmake --build build --target lint-tidy    # clang-tidy
cmake --build build --target lint-clazy   # clazy, Qt semantics
```

Both see `src/` and `tests/` only and stand at **zero findings**. Where a
finding deliberately stays, a `NOLINT` with its reason stands next to it. Known
gap: clazy checks `tr()`, this project uses `i18n()`.

### Translations

The source language is English: the `msgid` is the string standing in the code,
and every visible string goes through `i18n()` or one of its relatives. German
lives in `po/de/denkzettel.po`.

After adding, removing or rewording a visible string, run the extraction from
the project root. It rebuilds `po/denkzettel.pot` from `src/` and merges it into
every catalogue under `po/<lang>/`:

```sh
./po/Messages.sh
```

A new language needs one directory and one file, and no change to the build:

```sh
mkdir -p po/fr
msginit --input=po/denkzettel.pot --locale=fr --output-file=po/fr/denkzettel.po
msgfmt --statistics -o /dev/null po/fr/denkzettel.po   # what is still missing
```

`ki18n_install(po)` in the root `CMakeLists.txt` reads the language from the
*directory* name and the domain from the file name, so it picks the new
catalogue up on the next configure run and installs it as
`share/locale/fr/LC_MESSAGES/denkzettel.mo` — the path
`KLocalizedString::setApplicationDomain("denkzettel")` looks in. Hence the
layout `po/<lang>/denkzettel.po`; a `po/fr.po` would install a catalogue named
after the domain `fr` and no session would ever find it.

The test suite reads the source strings and not a catalogue: `LANGUAGE=en_US`
in `tests/CMakeLists.txt` keeps an installed German catalogue from reaching the
checks that compare English wording.

### Screenshots

The pictures in both READMEs come from `readmeshots`, which is built with the
test suite but deliberately kept out of `ctest`: a broken screenshot writer must
not turn the suite red. It is built all the same, because a runner nobody
rebuilds ages unnoticed and then writes plausible pictures of an *old* state
with a fresh timestamp.

One run writes **one** language set, and three things in the picture have three
different sources: the interface strings come from the message catalogue
(`LANGUAGE`), the timestamps come from `QLocale` (`LANG`/`LC_ALL`), and the
invented notes follow the locale as well. All three have to be set together, or
the result is the worst kind of picture — one that looks plausible and shows an
English window with German dates in it.

The runner also throws away its own configuration directory, so that the
pictures show the state as shipped and not the window size somebody has stored.
That has a second consequence: without a `plasmarc` it falls back to the default
(light) desktop theme while it sets a dark palette itself, and the note text
then stands light on light. So point it at a matching theme.

```sh
cmake --build build --target readmeshots

conf=$(mktemp -d)
printf '[Theme]\nname=breeze-dark\n' > "$conf/plasmarc"

# English — the source language, no catalogue involved
env -u LANGUAGE LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
    XDG_CONFIG_DIRS="$conf:/etc/xdg" \
    QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
    build/bin/readmeshots docs/images

# German — the catalogue has to be findable at runtime. Installed into a
# throwaway root with DESTDIR, nothing is written outside it.
dest=$(mktemp -d)
DESTDIR="$dest" cmake --install build

env LANGUAGE=de LANG=de_DE.UTF-8 LC_ALL=de_DE.UTF-8 \
    XDG_DATA_DIRS="$dest/usr/share:/usr/share" XDG_CONFIG_DIRS="$conf:/etc/xdg" \
    QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
    build/bin/readmeshots docs/images/de
```

The check that turns the second call into evidence is the same call **without**
`XDG_DATA_DIRS`: the window has to come out English then. If it does not, the
catalogue was never what made the difference. One trap in that check: the line
`-h, --help  Zeigt Hilfe …` is German either way — it comes from Qt's own
catalogue, not from this project's, and proves nothing.

`QT_QPA_PLATFORMTHEME=kde` has to be set: without it a substitute font takes the
place of the real one and gets the proportions wrong. The runner works
deterministically — two runs in a row deliver byte-identical files. The notes
shown in the pictures are invented.

Both pictures are produced offscreen. That shows geometry, typesetting, colour
roles and the frame the desktop theme draws — it does **not** show the shadow
and the blur behind the capture window, which the compositor contributes and
which no offscreen run has.

#### Hull, shadow and title bar

What the compositor draws needs a compositor, and it is never taken from the
session someone is working in: their notes are personal data and this
repository is public. A nested `kwin_wayland` on a bus of its own does it,
with a throwaway `HOME` whose database starts out empty:

```sh
sand=$(mktemp -d)
mkdir -p "$sand/.config"
printf '[Theme]\nname=breeze-dark\n' > "$sand/.config/plasmarc"
printf '[General]\nColorScheme=BreezeDark\n' > "$sand/.config/kdeglobals"

dest=$(mktemp -d); DESTDIR="$dest" cmake --install build      # for the catalogue

cat > "$sand/run.sh" <<'SCRIPT'
#!/bin/sh
"$PWD/build/bin/denkzetteld" &
sleep 6
dbus-send --session --dest=io.github.hnsstrk.denkzettel /Daemon io.github.hnsstrk.denkzettel.Daemon.ShowLibrary
sleep 4
spectacle -a -b -n -o "$SHOT"
sleep 2
SCRIPT
chmod +x "$sand/run.sh"

env SHOT="$PWD/docs/images/reviews/shot.png" \
    HOME="$sand" XDG_CONFIG_HOME="$sand/.config" \
    XDG_DATA_HOME="$sand/.local/share" XDG_CACHE_HOME="$sand/.cache" \
    XDG_DATA_DIRS="$dest/usr/share:/usr/share" LANGUAGE=de LANG=de_DE.UTF-8 \
    dbus-run-session -- kwin_wayland --virtual --width 1200 --height 800 \
    --no-lockscreen -- "$sand/run.sh"
```

`--virtual` keeps the nested session off the screen, and the private bus keeps
the daemon inside it: `KDBusService::Unique` would otherwise hand the start over
to the one already running. `spectacle -a` takes the window and not the whole
output — a full-screen grab of the virtual output comes out black.

That is the run [`docs/images/reviews/bibliothek-fenstertitel.png`](docs/images/reviews/bibliothek-fenstertitel.png)
comes from, the picture that shows the title bar reading "Bibliothek —
Denkzettel" and not the application name twice.

### How this project is run

Denkzettel is developed with AI. Claude Code writes the production code, the
tests and the checks. Goals, priorities, approvals and acceptance are mine. Most
commits therefore carry a `Co-Authored-By: Claude` line.

The backlog is the [issues](https://github.com/hnsstrk/denkzettel/issues) with
their acceptance criteria; the binding specification is [`SPEC.md`](SPEC.md).
Until August 2026 an extensive process apparatus of sprint records and review
reports stood beside it. It has been removed: in the end ten lines of report
stood against every line of code, and most findings concerned the checking
rather than the product. What remains are the four rules that actually found
faults in the program — they are in [`CLAUDE.md`](CLAUDE.md).

Every push to `main` and every pull request trigger a build and test run
([`.github/workflows/ci.yml`](.github/workflows/ci.yml)). It runs in an Arch
container and fails on every build error, every compiler warning, every red test
and **every linter finding**. Since 2026-08-11 it builds and checks **two build
types**, `Debug` and `Release`: Qt sets `QT_NO_DEBUG` for every build type
except `Debug`, and under it `Q_ASSERT` leaves its condition unchecked — a
difference a run with a single build type cannot see (#99). What the run does
**not** cover stands at the top of the file: it has no graphical session,
installs nothing and produces no pictures. Checking the installed state and
checking the pictures stay manual work.

### Directory layout

```
src/capture     capture window
src/store       SQLite access, schema, full-text index
src/ui          library
src/shell       tray, global shortcuts, D-Bus
src/analysis    AI pipeline (reserved by SPEC 2.2, still empty)
src/transcribe  Whisper backends (reserved by SPEC 2.2, still empty)
src/proposals   suggestion generation and execution (reserved by SPEC 2.2, empty)
tests/          unit tests and the screenshot runner
po/             message catalogues (German)
icons/          application and tray icons
desktop/        the desktop entry
packaging/      the PKGBUILD for Arch and derivatives
cmake/          helper modules for the lint targets
wireframes/     the binding drawings
docs/           the images used in this file
recherche/      dated research notes behind design decisions
third_party/    foreign code (spellfix from SQLite)
```

## The name

*Denkzettel* is German for a reminder — and someone who is handed one does not
forget the matter in a hurry. Checked on 2026-07-31 against some 400 existing
note apps as well as AUR, crates.io, PyPI, Flathub and GitHub: free.

## License

[MIT](LICENSE). Take the code, build on it, sell it for all I care — the
copyright notice just has to come along.

Two things about that: `third_party/spellfix/spellfix.c` comes from SQLite and
is public domain. And whoever passes Denkzettel on as a finished program has to
observe the terms of Qt and the KDE Frameworks, which are linked in dynamically
— those are LGPL.
