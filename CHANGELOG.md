# Changelog

All notable changes to Denkzettel are recorded in this file — written from the
user's point of view, structured after
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). The source is the
closed issues of the respective sprint milestone; purely technical entries stay
out, every change to the database schema is always named. Version numbering
follows 0.x SemVer (decided on 2026-08-02; visible since #61 via
`denkzetteld --version`).

## [Unreleased]

### Changed

- **The interface speaks English.** The source language of the application is
  now English; German lives on as a maintained translation in
  `po/de/denkzettel.po` and is installed along with the program, so a German
  session sees exactly the wording it saw before. `po/Messages.sh` refreshes the
  template after every change to a string.
- **Dates and times follow the locale.** The two date formats were wired to the
  German arrangement, which showed English-speaking users "Tue, 28. July" and
  "10.07.2026". They are now derived from the locale — "Tue, July 28" and
  "7/10/2026" under en_US, unchanged under de_DE.
- **Documentation and issues are in English.** `README.md` is rewritten for
  GitHub and carries the note that the interface is English; `README.de.md` is
  the German counterpart. `SPEC.md`, this changelog, the working instructions
  and the CI workflow follow, as do the titles and texts of the open issues.
  The screenshots exist in both languages under `docs/images/`.

**No change to the database schema.**

## [0.4.0] — 2026-08-11

Sprint 10, in a single evening. **Four faults**, two of them from the
user's review after sprint 9 — and one that nobody could have seen, because
it only occurred in the release build.

### Fixed

- **A new note now appears in an open library immediately.** Anyone who had the
  library standing open and captured something with `Meta+N` did not see the
  note until the next time the window was opened. The list now listens to the
  store instead of to the window, so it holds for every path — including
  `AddNote()` over D-Bus. If a deletion grace period is running, the new note
  waits out its five seconds instead of consuming the undo that the library is
  still offering. The note being read stays in place, the scrollbar does not
  jump, and while a search is running the note only enters the list if it
  matches (#105)
- **The library's groups can be told apart.** "Today", "Yesterday", "This week"
  now stand in the application's text size and in bold, and the line sits next
  to the label instead of running the full width — the shape KDE uses for
  section headers. Until now the group boundary differed from the note boundary
  in a single feature: 18 device pixels per side, 4.3% of the width, to be
  compared across a distance of 109 to 159 pixels. The header was also set in
  the smallest font of the list and was therefore smaller than the note text it
  heads (#104)
- **The input field shows when the keyboard goes there.** It now carries the focus
  state of its theme graphic as long as the window is the active one. Under
  five of the eight desktop themes the field was practically invisible at rest
  (1.00 to 1.14 : 1 against the background); with the focus layer it reaches up
  to 4.66 : 1. A window without the keyboard shows no edge — that is the other
  half of the information (#102)
- **A category is no longer lost in the release build.** The fault sat in the
  test harness, not in the program: `Q_ASSERT` leaves its condition unchecked
  under `QT_NO_DEBUG`, and Qt sets that for every build type except `Debug`.
  Anyone building an optimized package from the source got test cases reporting
  a data loss that only the test environment had caused. The public run
  therefore builds both build types from now on, and the linter reports this
  class of fault itself from here on (#99)

### Known limitations

- In the **resting** state — when another window has the focus — the input
  field falls back to 1.00 to 1.14 : 1 under the same five themes. That is
  deliberate: a window the keyboard does not go to should not claim otherwise.
- The images in the README still show the state of 2026-08-04.

## [0.3.0] — 2026-08-11

Sprint 9, accepted by the user on 2026-08-11 on the installed state. **Two
findings from their review after sprint 8**, both about legibility. The first
is cured, the second by half: the notes can be told apart, the groups cannot
yet.

### Changed

- **The capture window shows where you are typing.** The text area takes
  surface and edge from the desktop theme's graphics (`widgets/lineedit`) — the
  same source as the window shell, one layer deeper. Up to here the window was
  one continuous block of color in which the input area could not be made out.
  In the session the field stands out from the shell by 1.79 : 1 under
  `default` — KRunner's field sits at 1.41 : 1 (#100)
- **The note list separates entries and groups.** Between two notes of the same
  group there is a hairline indented to the text edge, above every group header
  except the first the same line across the full width. Same color, different
  length — the ranking note/group comes out of the extent of the stroke. No
  measurement of the list changes because of it (#101)

### Known limitations

- **Where one group ends and the next begins is still hard to see.** The line
  above the group header runs the full width, the one between two notes is
  indented — both have the same color and the same weight, and the difference
  in length alone does not carry the ranking far enough. Recorded as #104;
  there the design question is left open instead of prescribing a means
- **A newly captured note does not appear in an already open library** — the
  list only reads again the next time the window is shown. Recorded as #105
- **Under five of the eight desktop themes examined the input field stays
  invisible** — there the theme graphic draws no more than a hint (opacity 15
  of 255). On the default setting the fallback `default` takes hold, and there
  it carries. The cure is recorded as #102: all eight themes carry a focus
  state with a visible edge that would make the field visible under all of them

### Note

The images in the README still show the state of 2026-08-04 — the screenshot
runner has produced an unusable image since the native shell (#96).

## [0.2.0] — 2026-08-05

Accepted on 2026-08-05 (sprints 6 to 8; the acceptance lay with the product
owner, the user sees the overall result afterwards). **The first release
with a version number** — up to here there was none, because the number did not
reach the application.

### Added

- **The capture window is a native Plasma overlay.** Rounding, outline and
  shadow come from the desktop theme, not from built-in values; the background
  behind it is blurred as with KRunner and the notifications. Measured, the
  surface is **pixel-identical** with KRunner (#83)
- **The type comes from the same source as the surface.** If the desktop theme
  brings its own colors, they apply — for the note text and for the muted
  texts. Under `breeze-light` the contrast of the note text rises from 1.1 : 1
  to 13.4 : 1 (#85)
- **`denkzetteld --version` and `--help`.** Both answer even while the service
  is running (#61)
- **Tooltips with keyboard shortcuts** on "Edit", "Delete" and "Undo" in the
  library (#72)

### Fixed

- **A click on a clipped row now selects that row.** Previously the view
  scrolled and marked the neighbouring row while the reading pane showed the
  clicked note — selection and display drifted apart (#71)
- **The first note of a group brings its day header into view.** Previously a
  note from yesterday read "08:00" and nothing said which day it was from (#70)

### Changed

- The linter threshold stands at zero and is checked on every public run; 88
  findings are cured, 37 left standing with a reason (#76)

**No change to the database schema.**

## [Unversioned] — sprints 4 and 5

State from sprints 4 and 5, accepted on 2026-08-02 (user acceptance). These
acceptances did not get a version number — the rule only takes effect from #61,
and numbering is not applied retroactively.

### Added

- **Editing notes:** in the library, "Edit" or F2 opens the editor; saving
  (Ctrl+Enter) and cancelling (Esc) with a prompt on unsaved changes —
  nothing is silently discarded or written. The edited note stays visible in
  the result list even when it no longer matches the search term (#11)
- **Tray menu reworked:** all entries in German ("Notiz erfassen" instead of
  "Capture öffnen") and with icons; "Beenden" stands set apart at the end,
  separated from the working paths (#60)
- **Icons in the library:** "Edit", "Delete", "Save", "Cancel" and "Undo"
  now carry icons from the system theme; so does the prompt before unsaved
  changes, including a warning icon (#66, #67)

### Changed

- The menu entry displays the keyboard shortcut Meta+N; the rename applies
  everywhere, including in the system settings (shortcuts) and in the
  application launcher (#60)
- The guard dialog is switched to the KDE dialog style; "Save" remains the
  pre-selected answer (#66)

### Fixed

- A click on a visible note of another day group no longer makes the list
  jump (#57)
- Timestamps and hints in the library now follow a change of color scheme
  without a restart (#58)

## [0.1.0] — 2026-08-02

First accepted overall state, worked out in sprints 1–3
(Git tag `sprint-03-abschluss`).

### Added

- **Capture window:** one keypress (Meta+N) opens a slim input field, Enter
  saves, Escape discards — no file names, no dialogs (#4, #5, #42)
- **Permanent local storage:** notes live in an SQLite database in the user
  profile — no cloud requirement, nothing leaves the machine (#3)
- **Library:** window with note list and reading pane; deleting notes with
  an undo path (#7)
- **Inbox structure:** the note list groups by Today · Yesterday · This
  week · Last week · Older, the first line of each note serves as the
  subject (#46)
- **Full-text search:** the library's search field finds parts of words
  even in the middle of a word and forgives missing umlauts — "bucher"
  finds "Bücher", "grafieren" finds "fotografieren" (#8)
- **Tray icon** with an icon of its own; left click opens the menu just as
  the right click does (#2, #43, #44)
- **Autostart:** the background service starts with the login; on first
  start the keyboard shortcut is set up (#6)

### Changed

- **Database schema at version 2** for the full-text search. An existing
  stock of notes is migrated once on the first start; all notes are
  preserved (#8, #9)

### Fixed

- Small texts in the capture window now follow a theme change instead of
  staying in the old color (#54)
