# Changelog

All notable changes to Denkzettel are recorded in this file — written from the
user's point of view, structured after
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). The source is the
closed issues of the respective sprint milestone; purely technical entries stay
out, every change to the database schema is always named. Version numbering
follows 0.x SemVer (decided on 2026-08-02; visible since #61 via
`denkzetteld --version`).

## [Unreleased]

### Added

- **A topic becomes a suggestion, and a note that asks for something becomes a
  task.** The analysis run has a third step. Out of every topic of at least
  three notes the local model names the subject and says which of the notes
  really belong to it — it may drop one that does not fit — and out of that
  comes an open suggestion carrying that title and the collective note as
  Markdown: the topic as the heading, a section per day, and **the notes as
  they were typed**. The text is not handed to the model to rewrite; what is
  exported later is what was written. Beside them, every note the
  classification found a task in becomes a task suggestion with exactly the
  fields the note gave — description, project, tags, due date, priority, and
  none that were not there. **Nothing is carried out**: no note is exported and
  no task is added anywhere; the suggestions wait for the review, which is
  still to come (#30, #31). A suggestion put off with "later" is not buried —
  its notes go back into the pool and are grouped again in the next run, and
  the fresh suggestion takes the old one's place rather than standing beside
  it. Notes with a suggestion still open are left alone until it is answered.
  **Schema version 6** brings the tables `proposals` and `proposal_notes`;
  deleting a note takes its references with it, and deleting a suggestion takes
  them too, while the notes stay (#29).
- **Voice notes: `Meta+Umschalt+N` opens a window that is already recording.**
  No start button — the recording runs from the moment the window stands there
  (SPEC 4), with a red dot, a level meter of seven bars and the running time.
  `Ctrl+Enter` saves the recording as an audio note and the transcription queue
  takes it from there; `Esc` throws it away together with its file. At fifteen
  minutes the recording **ends the way `Ctrl+Enter` ends it** rather than being
  discarded, and from minute fourteen the running time turns red and the footer
  says when it will end. The level comes off the same buffers that go into the
  file, so no second stream is opened on the microphone. The window is reachable
  three ways, like the capture window: the shortcut, the tray entry, and the
  D-Bus method `ShowRecorder()`. **A recording that cannot be stored is not
  thrown away**: the note is only created once the file is closed, and if the
  database refuses it, the recording is moved to `rescued/` beside the audio
  directory and the message names that path. The cleanup check reads `audio/`
  and would otherwise take the file for an orphan and delete it at the very
  next start; nothing in Denkzettel ever deletes anything under `rescued/`
  (#21).

- **Notes get a vector, and notes about one topic find each other.** The
  analysis run has a second step: every analysed note is turned into an
  embedding by the local Ollama, and a note the user has edited is embedded
  again. Notes whose vectors point in a similar direction are chained into
  topics — A and B, B and C, and A and C need not be similar for all three to
  belong together — and a topic of at least three notes is what a suggestion
  will be made from. How many notes that takes is the setting "Bundle
  threshold" of the export page. A note the local model refuses twice is left
  alone and reported, while an Ollama that cannot be reached at all costs no
  note an attempt — it is a pause, and the vectors are made up for as soon as
  it answers again. The connection test on the settings page now says what an
  unreachable Ollama costs: no topic bundles, while the classification keeps
  running through the chosen provider. **Schema version 5** brings the table
  `embeddings` with one vector per note; deleting a note deletes it with it.
  What a topic becomes is still to come — that is the collective note (#29)
  (#28).
- **The library gets a category column, and the reading pane shows what the
  analysis found.** Left of the note list stand "Alle" and the five categories
  of the search, each with the number of notes behind it, counted in the
  database rather than off the list beside it. **A click writes `kat:ideen`
  into the search field** instead of filtering quietly beside it — so the
  column is the place where the search language is picked up, and a search
  already standing is added to rather than thrown away. The mark in the column
  follows the field: delete the word and the mark goes with it. The counters
  stay where they are while the list narrows — the column says what is there,
  not what is showing. Under the note text the category
  stands as a filled pill and the tags as outlined ones — a value out of a fixed
  list looks different from several free ones, and with neither of them there
  the row is not there either, with no empty strip left behind. The list rows
  keep their two lines of text and get no pills: they already carry the search
  mark. One entry beyond the five: **„Nicht eingeordnet"** collects the notes
  whose classification has been given up on, so that they can be reached and
  dealt with; it stays out of sight while there are none (#18, and #63, which
  went into it).
- **Notes get a category and tags from the local model.** One call per note
  yields one of the five categories the search already knows (`todos`, `ideen`,
  `cli`, `persoenlich`, `software`), one to four tags in lower case, and for a
  note that asks for something to be done the fields of a task. A note whose
  classification fails twice is left alone and reported, and that count
  survives a restart of the service. **Schema version 4** brings the column
  `notes.task` for the task fields; everything else the classification writes
  had its place since version 1. Nothing sets a run going yet — that is the
  trigger and the budget (#15) — and nothing shows the category and the tags
  yet, which the library brings (#18) (#14).
- **The classification runs by itself now.** When it does is the setting on the
  "Analysis" page: at once after a note is saved or its transcript arrives, at
  an interval (half an hour by default), or only when asked — and asked means
  the tray entry "Jetzt analysieren", which no longer stands greyed out, or
  `AnalyzeNow()` on the D-Bus interface. A switch in the settings takes effect
  at once and not at the next start of the service. One run takes at most 50
  notes and the rest follow in the next one, so a large library does not tie
  the machine up for hours; a note written while a run is going is taken up
  when that run ends. A note whose classification has failed twice is written
  into the log — `journalctl --user -t denkzetteld` — and not into the tray
  tooltip, which the transcription holds and where the message would stand for
  ever, without a place to work it off; it moves there once the library has an
  entry for unclassified notes (#18) (#15).
  trigger and the budget (#15); what the classification writes is shown by the
  library above (#18) (#14).
- **Denkzettel has settings.** The tray menu now carries "Configure
  Denkzettel…", and behind it stands a dialog with a page list. Two pages so
  far: "AI provider" — which backend answers, the language and embedding model,
  the Ollama address, and a "Test connection" button that reports the latency of
  one chat and one embedding call or the reason there is none — and "Analysis",
  which decides whether a run starts at once after saving, at an interval or
  only on demand. What is set there is kept in `~/.config/denkzettelrc` and
  survives a restart (#16).
- **A third settings page: "Export".** It holds the vault folder the collective
  notes are written into — a field with a folder chooser beside it, empty until
  it is set — and the three thresholds behind the overflow warning: at how many
  notes not transferred yet (200), from what age of the oldest one (30 days),
  and from how many notes on one topic a collective note is proposed (3). A
  folder that is not there or cannot be written to is reported by name the
  moment it is set — and again whenever the page is opened, so a vault that has
  been moved in the meantime does not sit there unremarked. The field keeps the
  folder that was accepted last (#75).
- **A third settings page: "Voice notes."** It holds the two values the
  transcription has so far only read out of the configuration file — the model
  size and the path to `whisper-cli`. All five sizes stand in the list, and one
  whose model is not on disk is shown greyed with the note that it is not
  downloaded, so nothing can be chosen that would only fail at the next
  recording. A path that names no executable program is reported on the page
  itself and is not written; the stored one stays. Both take hold while the
  daemon runs — no restart (#27).
- **Both global shortcuts are changeable in the settings.** The page "Kürzel"
  shows what the shortcut service really holds for "Notiz erfassen" and
  "Sprachnotiz aufnehmen", and takes a new key sequence for either. A change
  takes effect on Apply or OK, and is read back from the service afterwards:
  did the service keep something else than what was asked for, the field goes
  back to what it really holds and a red line underneath says so, instead of
  showing a shortcut that no key press finds. A sequence already taken is
  reported by the input field itself, in the same dialog KDE uses everywhere
  else. The key press for the voice note is still without effect — that is the
  recorder itself (#21) — and the messages about a failed registration name the
  sequence that was really registered instead of spelling out "Meta+N" (#74).
- **Denkzettel says what it is missing.** At every start it looks whether
  `ffmpeg`, `whisper-cli` and `task` can be started here and whether Ollama
  answers, and names in the tray tooltip whatever cannot — "Not available:
  ffmpeg, Ollama". Asking Ollama costs nothing: it is asked for its list of
  models, which loads none, and not for a sample answer — that is what the
  "Test connection" button is for, and paying its price at every login would
  add seconds to the start for one line of tooltip. Only the lack is named;
  everything being there is the ordinary case and says nothing, and the tray
  icon is not set apart for it, so the error state stays what it was, the sign
  of a transcription that has failed. A file that lies in the right place
  without being executable counts as missing, which is the case that used to
  turn up only at the moment the function was wanted. The settings say it a
  second time where it belongs: the page "Voice notes" for `ffmpeg`, "Export"
  for `task`, and "AI provider" has had its "Test connection" button all along.
  What is named is the program, not the package it comes in — which package
  that is depends on the distribution (#17).

### Changed

- **The model is set by its size, not by a file name.** The key `ModelPath` in
  the `[Transcription]` group of `~/.config/denkzettelrc` is replaced by
  `ModelSize` (`tiny`, `base`, `small`, `medium`, `large-v3`, default `small`);
  the file below `~/.local/share/denkzettel/models/` follows from it. **The
  first start takes the old key over**: a path ending in `ggml-<size>.bin`
  becomes that size and `ModelPath` goes. A path that names anything else stays
  in the file untouched — the size falls back to `small`, and the page "Voice
  notes" reports it with the old path in the sentence until the settings are
  applied once. Nobody's hand-set path disappears without being shown (#27).

## [0.7.0] — 2026-08-28

**The window follows what the session does**, the version becomes visible, and
the library stops re-reading a configuration file once per row. Six issues, all
of them measured rather than assumed.

### Fixed

- **The capture window now follows the blur.** Whoever switched the desktop
  effects off while the program ran kept the translucent variant of the theme
  graphic over a compositor that no longer blurred anything — with themes whose
  graphic fills almost nothing, that went as far as illegibility. The window now
  changes to the opaque variant without a restart, and back when the effect
  returns (#93).
- **The library was slow at a size nobody had measured.** It read `kdeglobals`
  anew for every row of the list — 20,005 times per list build, 3.7 s for
  20,000 notes, and already 0.38 s per keystroke at 2,000. Now 0.13 s. The
  guarantee that both windows follow a change of the system font while they run
  stays untouched (#110).

### Added

- **"Über Denkzettel" in the tray menu**, above "Beenden": name, version,
  short description, license and copyright holder, in the platform dialog KDE
  brings for it. A second click brings it forward when it is covered (#87).
- **AppStream metainfo**, so software centres can show what Denkzettel is —
  and so the check for it can fail. It could not before: the test looked below
  `/usr` and found nothing there without a real installation (#73).

### Changed

- **The application identity is now `io.github.hnsstrk.denkzettel`.**
  Reverse-DNS naming asks for a domain the project controls, and
  `denkzettel.org` is not one. **When updating from an older version, delete the
  three files of the old name** — otherwise two autostart entries stand in
  `/etc/xdg/autostart` and the capture window pops up at every login. Both
  READMEs say how, first thing in the section on updating (#109).
- **The result list keeps no upper bound.** Measured at 20,000 notes: a term
  matching every note costs 120 ms and 26 MiB, the same corpus with 50 hits
  0.4 ms. A limit would have bought some 95 ms per keystroke and cost a rule to
  learn (#78).
- **whisper.cpp comes from the `whisper-cpp` package now**, not the AUR, with
  `ggml-vulkan` beside it — 371 ms against 409 ms for ROCm at 52 MB instead of
  1.2 GB. Nothing of the voice notes themselves is built yet; this settles the
  ground they stand on (#19).

### For those building it

`kxmlgui`, `appstream` and `qt6-multimedia-ffmpeg` are new dependencies; both
READMEs carry them. The list "Runs that prove nothing" in `CLAUDE.md` grew from
15 entries to 22 — every one of them a measurement from this sprint that looked
like evidence and was none.

## [0.6.0] — 2026-08-24

Sprint 12. **Everything the user sees in the two windows**, and the one crash in
the stock. Ten of the eleven items of the package; the eleventh is recorded
below.

### Changed

- **Every note in the library now shows its full date and time.** Before, only
  notes from today or yesterday carried a time of day, and the week groups
  showed a weekday instead of a date — a note in "Older" carried no time of
  day at all, anywhere. The list now shows date and time for every note, and
  the reading pane adds the weekday and seconds; both follow the arrangement
  of the system language and keep the four-digit year (#108).
- **The note text sits closer to the edge of its field.** It kept four points
  of spacing that nothing drew, on top of the border the desktop theme draws
  around the input area — so it began nine to ten points right of the
  application name above it, where only six of them were visible as a frame.
  The text now begins on the inner edge of that frame, and under a theme that
  draws no frame at all both edges fall together. The resting window is eight
  points shallower because of it; the field still holds the five lines the
  specification promises (#81).

- **The application name in the capture window is no longer dimmed.** At rest —
  before anything is typed — the window showed nothing but muted texts: the
  name, the prompt in the field and the hint below it all carried the colour
  meant for placeholders. Every one of them was the right colour, and the
  window still looked as though it were ignoring the colour scheme, because
  nothing in it stood in the scheme's ordinary text colour. The name is the
  heading of the window and now says so; the hint below stays dimmed (#84).

- **Under some desktop themes the note was harder to read than the prompt it
  replaces.** The theme's own muted colour carried more contrast than its
  normal one, so whoever began to type saw their note worse than the words
  asking for it — measured over a light desktop background at 1.91 : 1 against
  the prompt's 4.72 : 1. Where that happens the note now takes the more legible
  of the two colours the theme holds. Where a theme ranks its two writings the
  right way round, nothing changes. This makes the note the louder of the two;
  it cannot make it legible where the theme offers nothing better (#97).

### Fixed

- **A change of the system font reaches the running windows.** Plasma does not
  pass a font change on to an application built with Qt Widgets, and Denkzettel
  keeps its two windows for the whole session — so until now a font changed in
  the system settings only took hold at the next login. Both windows now follow
  it: the note field, the note list and the small texts around them, without a
  restart (#68).
- **A capture window that is open shrinks back again.** Anyone who wrote eight
  lines, discarded them and immediately started the next note sat in front of a
  window that had stayed too large — and it stayed too large after Esc and a
  second `Meta+N` as well, and after a change of the interface font. Only a
  window nobody was looking at shrank back correctly, which is also why the
  check that was supposed to hold this stood green for so long (#79).

- **The capture window no longer crashes when a desktop theme has been
  removed.** Anyone who set a desktop theme and later uninstalled its package
  had a configuration naming a theme that no longer exists; the first `Meta+N`
  then killed the service — sometimes on the spot, sometimes one window later,
  because the damage was done to memory that something else used afterwards.
  The window now checks the name before handing it on and falls back to the
  standard theme with a line in the journal. The fault itself is in the KDE
  library `KSvg`; this is the guard against it (#107).

### Known limitations

- **Switching the blur effect off while the program runs still does not reach
  the capture window.** It keeps the see-through variant of its graphic until
  the service is restarted, and under themes whose graphic covers almost
  nothing that costs legibility. The way this was to be built does not exist:
  KWin's effect interface carries no signals at all, only methods, so there is
  nothing to listen to. Measured, and recorded with the way that remains
  (#93).
- **Where a theme sets its two writings the wrong way round, the note is now
  the louder of the two but still not legible** — the window can only hand out
  what the theme holds (#97).

**No change to the database schema.**

## [0.5.0] — 2026-08-24

Sprint 11. The application changes its source language. Nothing a German
session shows changes with it — the wording it had is now a maintained
translation that ships with the program.

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
  The screenshots exist in both languages under `docs/images/`. Both READMEs
  name the way a further language is added — one directory, one file, no change
  to the build.

### Fixed

- **The library no longer names the application twice in its title bar.** It
  read "Denkzettel — Bibliothek — Denkzettel": the window set the name itself
  and the window decoration appended it a second time out of the application
  data. The window now says only what it is, and the title bar reads
  "Bibliothek — Denkzettel".

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
