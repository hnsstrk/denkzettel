# Denkzettel — Specification

Derived from three design interviews and the wireframes (`wireframes/`). This
specification is the basis for the build and is binding.

## 1. Goal

Denkzettel is a quick-capture tool for KDE Plasma (Wayland): a global shortcut
opens a centred input window, a thought is typed or spoken and is saved — zero
ceremony. A background service classifies with AI, assigns tags, builds a
search index and lays suggestions before the user for confirmation: thematic
bundles for the Obsidian vault and tasks for Taskwarrior. Denkzettel is
pass-through storage — the plain-text place stays Obsidian, the task system
stays Taskwarrior.

Guard rails (from the concept, not negotiable):

- The capture path is never burdened with UI — speed is the core.
- Transfers happen only after confirmation ("report, do not heal").
- The application must not overflow; a full-export rescue path always exists.

## 2. Architecture

### 2.1 Process model — a refinement of the concept

The concept names "two components" (capture window, background service). This
spec refines that: **one process at runtime** (`denkzetteld`), in which the
capture and recording windows live as pre-instantiated, hidden windows.

Reason: the service runs permanently anyway (tray, shortcuts, analysis). A
window kept ready appears on a shortcut press without a process start — it
cannot be done faster, and speed is core goal no. 1. The separation
capture ↔ service is preserved as a **module boundary in the code** (its own
directory, no dependency of the capture module on analysis code).

The **D-Bus coupling** decided in the third interview stays real: the process
exports `io.github.hnsstrk.denkzettel` as an external interface (see 2.3) —
usable from the CLI, from scripts and to enforce a single instance.

### 2.2 Modules

| Module | Task |
|---|---|
| `capture` | Text capture window, recording window (voice note) |
| `store` | SQLite access, data model, FTS index, audio file management |
| `analysis` | AI pipeline: classification, tags, embeddings, clustering, task extraction |
| `transcribe` | whisper.cpp as a subprocess, job queue |
| `proposals` | Suggestion generation and execution (Obsidian export, Taskwarrior) |
| `ui` | Library, suggestion review, settings |
| `shell` | Tray (KStatusNotifierItem), KGlobalAccel registration, KNotifications, D-Bus adaptor |

### 2.3 D-Bus interface `io.github.hnsstrk.denkzettel`

Service name `io.github.hnsstrk.denkzettel`, object `/Daemon`, interface
`io.github.hnsstrk.denkzettel.Daemon`.

| Method | Effect |
|---|---|
| `ShowCapture()` | Show the capture window (also the target of the KGlobalAccel action) |
| `ShowRecorder()` | Show the recording window, recording starts at once |
| `AddNote(text) → id` | Create a note without UI (CLI/scripts) |
| `AnalyzeNow()` | Kick off an analysis run |
| `ShowLibrary()`, `ShowProposals()` | Open windows |
| `Quit()` | Quit |

A second process start recognises the taken D-Bus registration and calls
`ShowCapture()` instead (single instance).

- **The bus name hangs on the organisation domain (discovered condition,
  finding 2026-08-04, issue #61):** KDBusService assembles it out of the
  reversed domain and the application name. `KAboutData::setApplicationData()`
  overwrites both fields with its own defaults — the domain with
  `kde.org` —, and the service then registers as `org.kde.denkzettel`.
  Measured on the name actually registered, not derived from the header.
  **Denkzettel therefore sets the domain and the desktop file name on the
  `KAboutData` object before it is registered, and in no second place** — two
  setters would have decided the question by line order. No return value
  reports the breakage.
- **The arguments are evaluated before the single-instance branch (discovered
  condition, finding 2026-08-04):** if the service is already running, the
  branch hands a second start on to it; an evaluation of `--version` sitting
  behind it would then open a capture window and return **0** without printing
  anything. Without a reachable session bus KDBusService additionally ends the
  process with 1. Both make the order a condition, not a matter of taste.

### 2.4 Global shortcuts

- `Meta+N` → `ShowCapture()` · `Meta+Shift+N` → `ShowRecorder()`
- Registration through **KGlobalAccel** (KF6); the shortcuts appear in the
  Plasma system settings and can be changed there as in the application settings.
- Occupancy checked on the development machine (2026-07-31, including multiple bindings): both free.
- **Conflict detection (T1 finding):** a KGlobalAccel registration can fail
  invisibly — the entry comes into being and `invokeShortcut` works, but the
  real key press keeps going to the existing owner. At first start and when a
  shortcut is changed Denkzettel checks the sequence against the existing
  occupancy (including multiple bindings) and reports conflicts visibly
  instead of failing silently.
- **Triggering through desktop actions (discovered condition, finding
  2026-08-01):** with the application installed the component name ends in
  `.desktop`; kglobalacceld thereby treats the component as a *service action
  component* and, on the key press, starts the **desktop action** of the same
  name in the `.desktop` file (ApplicationLauncherJob) instead of sending a
  D-Bus signal to the running process. If the group is missing, the service
  logs an error and gives up — the registration is in place, `isActive` is
  true, and the key press fizzles out. Hence: **per shortcut the desktop file
  declares a group `[Desktop Action <action id>]` with an `Exec` line of its
  own, and the id stands in `Actions=`.** The action id is at the same time
  the `objectName` of the `QAction` and must be a valid XDG identifier
  (letters, digits, hyphen — no underscore; `desktop-file-validate` rejects it
  otherwise).
- **The action name is what tells the two shortcuts apart, and it only arrives
  over D-Bus activation (discovered condition, finding 2026-08-29, issue
  #125):** both `Exec=` lines are the same start line without an argument, so
  the started process carries no mark of which key was pressed — it reaches the
  running service through the single-instance branch of 2.3 as a plain
  activation, and that branch shows the capture window. `Meta+N` therefore
  worked reliably because its target is what happens anyway, and `Meta+Shift+N`
  could reach the recorder through no key at all. What carries the name is the
  XDG road: the desktop entry declares **`DBusActivatable=true`**, KIO's
  `ApplicationLauncherJob` then calls
  `org.freedesktop.Application.ActivateAction(<action id>)` on the bus name of
  2.3 instead of running the `Exec` line, and `KDBusService` hands that name on
  as `activateActionRequested`, where `main()` picks the window. The `Exec`
  lines stay as they are (15.1): they are the road taken when nothing answers
  on the bus, and the key is mandatory for a desktop action.
- **What the D-Bus road needs beside the key (measured 2026-08-29):** the
  desktop file name has to carry at least two dots — `DBusActivationRunner`
  refuses a shorter one — the bus name has to equal the file name without
  `.desktop`, which 2.3 already lays down, and a D-Bus service file
  `io.github.hnsstrk.denkzettel.service` has to map that name to the installed
  program. Without the third one the launcher's call reaches a bus name nobody
  owns whenever the daemon is not running, and the key press fails with
  `ServiceUnknown` and nothing on screen.
- **A changed desktop file reaches the shortcut service only in a new session
  (measured 2026-08-29, issue #125):** under Plasma 6 that service is
  `kwin_wayland` itself (finding 49 in `CLAUDE.md`); it reads the desktop file
  at login and holds that state. Installing, `kbuildsycoca6 --noincremental`
  and a restart of the daemon do not reach it — on the same file in the same
  minute a freshly started process takes the D-Bus road of the paragraph above
  while the real key press still runs the `Exec` line. So whoever changes the
  desktop file verifies the key press after a new session, and a user who
  updates the package keeps the old behaviour until their next login.
- **Reading the registration back (discovered condition, finding 2026-08-01;
  retro resolution B5):** `KGlobalAccel::setGlobalShortcut()` cannot report a
  failure of the service — the call sends off its D-Bus message without
  reading the answer, and returns `true` even when kglobalacceld has kept
  nothing. Denkzettel therefore asks the service after every registration
  which sequence it holds for the action, and checks at the same time whether
  the desktop file declares the matching group. If either of the two is
  missing, Denkzettel reports it visibly — **at every start, not only at the
  first**: unlike a conflict, this failure leaves no working shortcut behind
  at all. The message names the affected shortcut and one executable step.
  **This holds per shortcut:** `Meta+Shift+N` goes through the same check as
  `Meta+N` — without it, it repeats that shortcut's failure, and "visible in
  the system settings" is precisely the state a silently failed shortcut
  produces.
- **A stored `none` beats the default (measured 2026-08-29, issue #125):**
  `registerShortcut()` uses the autoloading `KGlobalAccel::setGlobalShortcut()`,
  which writes the default only at the very first registration and afterwards
  restores what stands in `kglobalshortcutsrc`. A line `<action id>=none` there
  therefore wins against the default in every new session, while a missing line
  lets the default land — two actions of one component then behave differently
  on identical code, and the difference is not in the code. Only a registration
  with `NoAutoloading` overwrites it, and the application's own shortcut
  settings page writes that way.
- **The component name hangs on the desktop file name (discovered condition,
  finding 2026-08-04, issue #61):** Denkzettel reads
  `QGuiApplication::desktopFileName()` and appends `.desktop` — the component
  name hangs on that, and under Wayland the application id as well.
  `KAboutData::setApplicationData()` overwrites the property with its default
  `org.kde.<component name>`; the shortcuts then ran under a component to which
  no installed desktop file belongs. Denkzettel therefore sets the name on the
  `KAboutData` object before the registration (see 2.3).
- **The application id is `io.github.hnsstrk.denkzettel` (decision 2026-08-28,
  issue #109):** desktop entry, AppStream component id and Wayland application
  id all read that, and the shortcut component is
  `io.github.hnsstrk.denkzettel.desktop`. Reverse-DNS naming asks for a domain
  the project controls and `denkzettel.org` is not one; the
  `io.github.<user>.<app>` form is what a project hosted on GitHub uses and what
  Flathub requires. **The bus name follows the id (decision 2026-08-29, issue
  #112):** the organisation domain in the `KAboutData` is `hnsstrk.github.io`,
  which reversed and followed by the application name `denkzettel` gives the
  service `io.github.hnsstrk.denkzettel` of 2.3. The old name
  `org.denkzettel.Daemon` is withdrawn and registered nowhere beside it — the
  customer confirmed on 2026-08-29 that no script calls it. **A rename of the
  id is not free:** installing writes the entries of the new name beside those
  of the old one instead of replacing them, so until the old ones are deleted
  the session reads two autostart entries and starts the daemon twice — the
  second start arrives as an activation and shows the capture window (2.3), at
  every login. Two shortcut components then hold `Meta+N` as well, and
  `foreignShortcutOwners()` reports the older of the two as a foreign owner —
  unseen, because `main()` only notifies about a conflict on the first start.
  The key press keeps working, both entries carry the same `Exec` and the same
  action. Only deleting the entries of the old name ends it; the group left in
  `kglobalshortcutsrc` is inert once its desktop entry is gone (measured
  2026-08-28), and the running service writes that file back anyway.
- **The application id is not a command line value (decision 2026-08-05):**
  `KAboutData::setupCommandLine()` would bring the option `--desktopfile
  <file name>` with it, which overwrites precisely this value at runtime.
  Denkzettel does not register it; it thereby falls under the rejected
  switches (section 15).

### 2.5 Autostart and first start (addition from the estimation session)

- `denkzetteld` starts with the Plasma session through an **XDG autostart
  entry** (the package installs the .desktop file; deactivation through the
  Plasma system settings). Without a running service there would be no
  shortcuts — autostart is a basic function, not polish.
- **First start**: creates the data directory, the DB (current schema version)
  and the default configuration; detects the optional tools (ffmpeg,
  whisper-cli, task, Ollama reachability) and shows what is missing in the
  settings (cf. section 15).
- **Cleanup check**: at service start, audio files without a DB reference
  (aborted recordings, interrupted deletions) are removed and noted in the log
  — an unambiguous, harmless, recurring case, and therefore self-healing that
  is permissible in the sense of the loop conventions. There is a third road to
  the same state, and it is the one case where the sweep would be the second
  half of a data loss: a recording that finished while the note it belongs to
  never reached the database. In the data it cannot be told from the harmless
  two — a file, no row, nothing beside it — so it is barred at its source and
  not here: whatever saves a recording reports a failed save to the user in the
  same breath (§14), naming the file, and the recording window is where that
  belongs.
- **And naming it is not enough** (finding of the review of #21, 29.08.2026):
  the message told the user the recording "will not be deleted", while the
  sweep above removes it at the very next service start, because nothing points
  at it. The sentence was true only for a user who acts before restarting. So
  the file is **moved out of the sweep's reach** — into `rescued/` beside
  `audio/`, which the sweep does not read, since it lists the files of `audio/`
  and no subdirectory — and the message names **that** path. That is what
  "barred at its source" means here: not a promise about the sweep, but a place
  the sweep does not look. Denkzettel never deletes anything under `rescued/`;
  what happens to it is the user's decision, and the directory exists only once
  something has gone wrong.

## 3. Capture window (text)

- Frameless window, immediate focus, always in the foreground.
- **Focus mechanics (T1 finding, issue #1):** before every showing the window
  is mapped anew — `hide()` destroys the Wayland surface, `show()` creates a
  fresh toplevel which regularly receives the focus from the compositor. The
  XDG activation token route demonstrably does not carry (KGlobalAccel
  delivers no token, timestamp always 0) and is not built.
- **Centring (design decision after T1):** KWin default placement — Plasma 6.7
  centres by default, verified on the development machine. A Wayland client
  cannot position itself; if the user's placement policy differs, layer shell
  (overlay, `AnchorNone`, `KeyboardInteractivityOnDemand`) is the measured
  fallback route — documented, not built in v1.
- Content: application name small, multi-line text field (placeholder
  "Capture a thought…"), footer "Esc discards · Ctrl+Enter saves".
- **Growing along**: starting height ~5 lines (sprint 1 acceptance: 3 were too
  few for the user), grows with the text up to ~8 lines, then a scrollbar.
- **And shrinking back** (issue #79). The way back belongs to the growing: a
  window emptied by `Ctrl+Enter` or `Esc` stands at the resting height again,
  and so does one whose font has been made smaller. On a **shown** window that
  needs a step of its own, and the reason is measured: on activation the layout
  writes its total minimum onto the window, `resize()` is clamped by it, and
  the minimum of the eight-line state stands until the next activation — 244 px
  against a hint of 190. `adjustHeight()` therefore activates the layout anew
  before it resizes. On a hidden window the fault cannot occur, which is why a
  check for it has to show the window.
- Ctrl+Enter: save the note (`store`), hide the window, clear the field.
  Esc: discard, hide the window. Loss of focus: the window stays (no data loss
  through an accidental click beside it).
- **The height is recalculated when the font changes as well** — not only when
  the text changes (issue #56). The number of lines stays the same when the
  font is switched, the line spacing does not; without this rule a standing
  window falls back from five lines to three, that is to precisely the state
  the sprint 1 acceptance rejected.
- No button, no menu, no formatting.

### 3.1 Hull from the desktop theme (issue #55, wireframe 4a/4b)

The frameless window wears the **rounding, border, opacity and shadow of its
desktop theme**, drawn out of `dialogs/background` — the same graphic Plasma
builds its popups and KRunner from. The library and the dialogs are untouched
by this: they are decorated windows and get their hull from KWin.

**Three registrations with the window system belong to the hull, not one:**
`KWindowShadow`, `KWindowEffects::enableBlurBehind` and
`KWindowEffects::enableBackgroundContrast` — the same three that
`libPlasmaQuick` makes for Plasma's own overlays. The conditions stand in 3.2.

**Form and colour come from the theme** (user decision 2026-08-04, issue #83:
„dann eine native Plasma-Überlagerung ohne Anpassungen" — then a native Plasma
overlay without adaptations). What is drawn is the theme's own graphic, **in
one piece** (`FrameSvg::framePixmap()` at the pixel ratio of the window) — no
alpha mask, no fill colour of our own, no outline drawn by ourselves.
**What is guaranteed is where the colour comes from, not its contrast figure.**
A contrast figure holds for one colour scheme, one selection path and one named
ground; none of the three belongs to the code.

**Limit, named and accepted:** of the eight themes installed on the user's
machine only `default` aligns its fill colour with the colour scheme. Under
`CachyOS-Nord-round`, `Iridescent-round` and the three `cachyos-emerald`
themes the hull covers almost nothing (down to 2.7 %), and what is supposed to
carry the text there is the contrast effect of the compositor — which on this
state is **not present** (3.2, item 10). There the specification guarantees no
legibility.

- **Two surfaces, both out of the theme** (user decision 2026-08-06,
  issue #100). The **window surface** is drawn by `dialogs/background`, the
  **text field** by `widgets/lineedit` with the prefix `base` — the same
  source, one graphic deeper, and the same one KRunner's entry field is drawn
  from. Neither of the two necessarily covers; "closed" here means complete,
  not opaque. Under the five weak themes above, `widgets/lineedit` draws no
  more than a breath (opacity 15 out of 255 against 255 under `default`);
  there the field stays practically invisible, and there is nothing to be
  healed about that without giving up #83.
- **The field has two states, not one** (issue #102). The prefix `base` is the
  **resting** state; as long as the window is the active one, the prefix
  **`focus`** of the same graphic lies on top of it — Plasma's own layering,
  in the graphic `hint-focus-over-base`. The layer claims no border of its own
  (0.1 px against 6 px for `base`) and therefore shifts no inner spacing out
  of #100.
  **The condition is `hasFocus() && isActiveWindow()`**: a window that has not
  got the keyboard shows no focus edge. That is the other half of the
  guarantee "loss of focus: the window stays" from section 3 — it stays put
  and says at the same time that nothing is arriving there just now.
  **The focus edge takes the accent colour of the colour scheme** (SVG class
  `ColorScheme-ViewFocus`); on the user's setting that is green. That is the
  KDE convention and no choice of the build.
  **Limit:** in the resting state the field falls back to practically
  invisible under the five weak themes — the same limit as above, now for one
  state instead of for the field as a whole.
- **The outline** is no line of its own. The theme graphic draws the same
  colour at its edge as in the surface and differs only in opacity; the edge
  becomes visible only because the hull lets the ground through.
- **The text comes from the same source as the surface** (user decision
  2026-08-04, issue #85), and that holds for **both** text classes. If the
  desktop theme brings a `colors` file of its own, its colours apply:
  `ForegroundNormal` for the note text, `ForegroundInactive` for the muted
  class. If it brings none, the colour scheme applies — then **note text**
  `WindowText`, **not** the role for entry fields, and the **footer**
  `PlaceholderText`. The **placeholder text** of the empty field belongs to the
  muted class as well.
- **The application name carries the note text's colour** (user decision
  2026-08-24, issue #84). It is the heading of the window and not a
  placeholder; before this the window showed nothing but muted writing until
  something was typed, and looked as though it were ignoring the scheme even
  where every colour was right. The muted class therefore has two places, not
  three: footer and placeholder text.
- **The note text is never the quieter of the two writings that share the
  field** (user decision 2026-08-24, issue #97). Under some themes the theme's
  own muted colour carries more contrast than its normal one, and the
  placeholder then reads better than the note that replaces it — measured over
  a light ground 1.91:1 against 4.72:1 under `cachyos-emerald-color`. Where
  that happens the note takes the muted colour, which is the more legible of
  the two the theme holds.

  This is a **deliberate exception** to the rule above, and the only one: the
  colour still comes from the theme, only the choice between its two writings
  is the window's. It takes hold **only** where the note is the quieter one —
  a window that always lifted would break the ordinary case to heal the
  exception. Judged on the poorer of two grounds, because the window lets the
  screen behind it through and the ranking otherwise depends on the user's
  wallpaper.

  **Its limit:** it makes the note the louder of the two, not legible. Where it
  takes hold, both writings are far below 4.5:1 — under `cachyos-emerald-color`
  the note ends at 3.69:1. What the theme does not hold, the window cannot
  hand out.
- **The text cursor follows the type, not the colour scheme.** Qt draws it in
  the text colour of the entry field; as soon as that comes from the theme, it
  goes along. **That is intended and must not be "healed back":** under
  `breeze-light` with a dark scheme the scheme colour would be invisible on
  the light surface. For the **selection** the palette continues to apply.
- **Inner spacings** (12 at the sides, 10 at the top, 8 at the bottom) apply
  **in addition** to the border the theme claims for itself — with Breeze the
  content starts 16 px from the window edge, with an 8 px theme 20 px. Above
  the footer there is more air (12) than below the application name (8); since
  the separating line was dropped, this difference is the whole structure.
  **For the note text the border of the field is added** — the strip that
  `widgets/lineedit` claims for itself (about 6 px all round under all eight
  installed themes). The text moves inwards accordingly and the text area
  grows by twice that border; **application name and footer stand unchanged**,
  because the field surrounds the text area alone. Here too the guarantee is
  relative: against the border the graphic reports, not against the number 6 —
  four of the eight themes report 5.99999.

**Rounding and border are not numbers of this specification.** They belong to
the theme. What is guaranteed is **relative**: with two desktop themes with
different borders, border measure and corner form differ accordingly. The
corner form comes from the corner tiles of the theme and is **not** to be
derived from the border measure — two themes can be curved differently at the
same border.

### 3.2 Conditions of the hull (discovered during the build)

They stand here because an implementation without them draws a hull that looks
right and is wrong. Most of them contradict what the construction suggests, and
**not a single one reports its failure through a return value.**

1. **KSvg does not find the desktop theme by itself.** The name that is set
   stands in `plasmarc` under `[Theme] name`; without being handed it,
   `KSvg::ImageSet` stays on `default`, which is what the file says as well.
   The application reads it itself.
2. **A `FrameSvg` follows only a fresh `ImageSet`.** Renaming the set has no
   effect, setting the image path again has no effect, assigning the same set
   again has no effect. At every theme switch a new set comes into being; the
   old one falls away only once all frames point at the new one.
3. **The theme switch is delivered through `KDirWatch` on `plasmarc`, not
   through `KConfigWatcher`.** The latter only reports when the *writer* has
   used `KConfig::Notify` — the guarantee would then hang on the discipline of
   a foreign program. KConfig replaces the file when writing, which is why a
   `QFileSystemWatcher` would lose its watch in the process.
4. **Without a desktop theme no hull, but a usable window.** Outside a Plasma
   session `dialogs/background` is missing. The window then stays opaque and
   operable — no crash, no transparent surface.
   **An *unknown theme name* falls back to `default`, and Denkzettel is what
   makes it do so** (issue #107, 2026-08-24). The fallback KSvg has of its own
   may not be walked into: `KSvg::ImageSet` keys its shared private by the name
   it is **given** and removes it again by the name it has **resolved** (ksvg
   6.29, `imageset.cpp` and `private/imageset_p.cpp`). A name with nothing
   behind it resolves to `default`, so the destructor takes the wrong key out
   of the table and leaves the given one pointing at freed memory; the next set
   built under that name references it and writes through it. That is why the
   place of the crash is not the place of the fault — the first window comes up
   and the second one dies, or an allocation after it does. Denkzettel
   therefore resolves the name itself before it hands it over, and both roads
   into the theme come past that check: the constructor and the watch on
   `plasmarc` (item 3).

   | `[Theme] name` in `plasmarc` | up to #107 | since #107 |
   |---|---|---|
   | theme that is not on the data path | **SIGSEGV** in `KSvg::ImageSet`, at the *second* set of that name | hull of `default`, window usable |
   | key missing entirely | fallback to `default`, everything as guaranteed | unchanged |
   | existing theme (`breeze-dark`) | everything as guaranteed | unchanged |

   The left-hand column is measured on the state before the fix
   (`capturetest`, `survivesAnUnresolvableDesktopTheme`): SIGSEGV in
   `QString::operator=` on the freed private, and under AddressSanitizer a read
   through a dangling `d` in the second `ImageSet` constructor. The situation
   arises without anyone's doing: whoever sets a desktop theme and later
   removes its package has exactly this `plasmarc`.
5. **The shadow is bound anew after every re-showing.** Before every showing
   the window is mapped anew (above), and the Wayland surface disappears in the
   process; a shadow bound once in the constructor would be gone after the
   first hiding. The binding therefore belongs after `show()`.
6. **The blur takes effect only if it is registered immediately after
   `show()`** — a second later the ground stays sharp, with a mask region as
   without one. `enableBlurBehind()` is `void`, so there is no return value
   that would report the failure; it becomes visible solely in the image from a
   logged-in session. The registration therefore stands beside the shadow
   binding.
   **And it must not be given a `nullptr` window:**
   `enableBlurBehind(nullptr, …)` crashes under Wayland (SIGSEGV), while the
   same call returns offscreen.
7. **The pixel ratio of the window is not yet settled after `show()`.** Under
   Wayland Qt first reports 2 and about a second later the real value,
   delivered as `QEvent::DevicePixelRatioChange` **without an accompanying
   `QEvent::Resize`**. A `KSvg::FrameSvg` does not follow the screen by itself
   anyway — its ratio is 1 after construction, whatever scaling applies.
   Whoever redraws the hull only in `resizeEvent()` draws wrong permanently.
   **Offscreen the case does not occur.**
8. **Whether the contrast effect is registered is said by the theme.** The four
   values stand in the group `[ContrastEffect]` of the theme's `metadata.desktop`
   file; if the group is missing — as with `default` — nothing is registered.
   They are read directly and not through `Plasma::Theme`: that class sits in
   libPlasma, which would pull QtQuick along (build decision on #83).
9. **Without a session with blur the selection path `opaque` applies.**
   Otherwise the window would draw the translucent variant of the theme graphic
   without the compositor that makes it carry — item 4 promises the opposite.
   The condition does **not** hang on
   `KWindowEffects::isEffectAvailable(BlurBehind)`: that value is sluggish and
   returns `false` in the user's session **before** the first registration of
   our own, `true` afterwards — hung on that, the window would switch to opaque
   of all times at the start. Denkzettel asks KWin itself instead
   (D-Bus, `org.kde.kwin.Effects.isEffectLoaded("blur")`) and answers "no"
   without asking where there is no compositing window system at all.
   **The value is asked again when the session changes it** (issue #93,
   2026-08-28): whoever switches the blur off while the service is running gets
   the opaque variant on the standing window, without a restart and without
   closing it, and switching it back on brings the translucent one back — item 4
   holds for a switch after the start as well, not only for the state at it.
   Measured in a nested session under `default`, one process, two windows, the
   theme name unchanged throughout: hull pixel alpha 216 → **255** → 216, and
   the resolved file `dialogs/background.svgz` →
   `opaque/dialogs/background.svgz` → back. On the unchanged state the same run
   switches the effect just the same and moves nothing: 0 of 240.300 pixels,
   with a start picture byte-identical to the changed run's — so the runner did
   grab a window, and only the code told the two runs apart. The switch in the
   system settings writes `[Plugins] blurEnabled` into `kwinrc` and tells KWin
   over D-Bus; a `KDirWatch` on that file carries the change to the standing
   window, exactly as item 3 carries the theme switch.
   Four measured properties of that road:
   **the file announces the change and does not answer it** — its value says
   what the user *wants*, not whether anything blurs: outside a Plasma session
   the same `blurEnabled=true` stands in it and nothing is blurred, which is
   the case item 4 is written for. KWin is therefore asked again, and the hull
   is rebuilt only when the answer has changed. That the file is not even a
   reliable account of KWin's own state is the smaller reason beside it,
   measured all the same: `reconfigure()` after `blurEnabled=false` leaves the
   effect loaded;
   **there is no race** — `KDirWatch` delivers the change 500 ms after the
   write, KWin has switched over after 20 ms;
   **the selectors have to be set on both branches**, because
   `KSvg::ImageSet` keys its private by the theme name and every live set of
   that name shares it, the selectors included — set only in the opaque case,
   they stay behind and the window never comes back (`CLAUDE.md`, finding 4);
   **and the new graphic arrives about 100 ms later**, on KSvg's own
   notification timer, not within the call (`CLAUDE.md`, finding 19).
   A signal of KWin's would be the shorter road and there is none:
   `org.kde.kwin.Effects` carries no signals at all, and the
   `PropertiesChanged` its `emits-change` annotation promises for
   `loadedEffects` never reaches the bus (`CLAUDE.md`, findings 15 and 18).
   **What this does not cover:** the hull follows both ways, the blur
   *registration* of item 6 is not renewed with it. Whether KWin still blurs
   behind a window that was already standing when the effect came back is not
   measured — the next showing settles it in any case, because `present()`
   registers anew after every mapping.
10. **The contrast effect has no recipient on this state.** KWin 6.7.3 loads
    **no** effect with "contrast" in its name; `blur` on the other hand is
    loaded. The registration from item 8 thereby goes into the void.
    **Affected are the three themes that ask for the effect:**
    `cachyos-emerald`, `cachyos-emerald-color` and `Iridescent-round`. Their
    hull covers almost nothing; there the text stands on the desktop
    background and on nothing else. They are built on the compositor darkening
    the ground — and that is precisely what it does not do on this state.
    **`cachyos-emerald-light` is a case of its own and not part of this
    condition.** It asks for **no** contrast effect, so Denkzettel registers
    none there either, per item 8. Its dark theme text on a translucent hull
    would therefore stay as it is **even on a KWin with the contrast effect
    loaded** — over a white ground 14.32:1, over a black one 1.37:1, and **no
    choice of text colour rescues both directions**.
11. **The precedence rule for the text colours stands in exactly one place.**
    The two sources from 3.1 move at different moments — the theme at the theme
    switch, the scheme at the palette switch. An implementation that writes the
    theme colour only at the theme switch is right at the acceptance and wrong
    after the first scheme switch, and **silently** so: no return value, no
    event, no red test assertion. Both occasions therefore run through the same
    function.
12. **KSvg knows no counterpart to `ForegroundInactive`.** The enumeration
    `KSvg::Svg::StyleSheetColor` carries `Text`, `Background`, `Highlight`,
    `HighlightedText` and three signal colours per colour set. The note text
    therefore comes through `KSvg::Svg::color(Text)` with `colorSet(Window)`
    set — that **is** already the rule from 3.1 —, the muted class on the other
    hand from the theme's `colors` file, read directly like the group from
    item 8.

## 4. Recording window (voice note)

- Same make as capture; the **recording runs from the moment the window
  opens** — no start button.
- Content: recording indicator (red dot), simple level meter, running time
  display, same footer.
- Ctrl+Enter: stop the recording, save the audio, create a note of type
  `audio`, enqueue a transcription job, hide the window. Esc: discard the
  recording (delete the file).
- Technology: QtMultimedia (PipeWire backend), format **Opus in OGG**
  (`audio/*.ogg`), mono, 48 kHz — small and directly playable by Qt.
- **The file is named after the note's ISO timestamp with the colons of the
  hour replaced by hyphens** — `2026-08-28T22-10-48.526.ogg` for a note stored
  as `2026-08-28T22:10:48.526` (user decision 2026-08-28, issue #20). Not a
  cosmetic departure from the database: FAT and exFAT forbid `:` in a file
  name, and the full export of section 8.3 copies the audio files to wherever
  the user points it — a USB stick is the case that path exists for. Milliseconds
  stay, and settle the only collision two recordings could have.
- Upper bound 15 minutes (protection against a forgotten recording); hint in
  the time display from minute 14.
- **At the upper bound the recording is saved, not discarded** (decision
  29.08.2026, issue #21). The bound exists against a recording forgotten while
  running, not against the fifteen minutes somebody has just spoken; throwing
  them away would be the one data loss the program causes on its own. So minute
  15 ends the recording the way Ctrl+Enter does — note, transcription job,
  window closed — and the hint from minute 14 says so instead of threatening.

## 5. Data storage

### 5.1 SQLite (one DB: `~/.local/share/denkzettel/denkzettel.db`)

```sql
notes(id INTEGER PK, created_at TEXT ISO8601, type TEXT 'text'|'audio',
      content TEXT,            -- text or transcript
      audio_path TEXT NULL,    -- relative to audio/, only type='audio'
      audio_duration_s INTEGER NULL,
      category TEXT NULL,      -- AI category, NULL = unanalysed
      state TEXT 'neu'|'transkribiert'|'analysiert',
      needs_reembed INTEGER NOT NULL DEFAULT 0,  -- after an edit (section 9)
      analysis_attempts INTEGER NOT NULL DEFAULT 0,  -- error counter 7.2
      analysis_last_error TEXT NULL,
      task TEXT NULL,          -- JSON of the task fields of 7.2, schema
                               -- version 4; NULL means the note is no task
      origin TEXT NULL,        -- window title at capture time (opt-in, §13)
      origin_app TEXT NULL)    -- and the application id beside it
tags(note_id FK, tag TEXT)
embeddings(note_id FK PK, model TEXT, vector BLOB)  -- float32 array
proposals(id INTEGER PK, kind TEXT 'bundle'|'task', created_at TEXT,
          status TEXT 'offen'|'zurueckgestellt',
          payload TEXT JSON)   -- bundle: title+Markdown; task: fields
proposal_notes(proposal_id FK, note_id FK)
transcribe_jobs(note_id FK PK, enqueued_at TEXT, attempts INTEGER,
                last_error TEXT NULL)
meta(key TEXT PK, value TEXT)  -- schema version and the like
```

- Full-text index: **FTS5** table `notes_fts(content)`, kept in sync by
  triggers. It holds **no text of its own** but points at the notes table with
  `content='notes'`, `content_rowid='id'` (schema version 2, issue #8) — the
  note text exists exactly once. From that follows a condition without which
  the index quietly falls into disrepair: the triggers for updating and
  deleting must hand FTS5 the **old** text (`'delete'` command with
  `old.content`). With the new text the old words stay findable, and neither an
  error nor FTS5's `integrity-check` shows that — only a search for the old
  word does (see `StoreTest::keepsSearchIndexInSync()`).
- **The origin is two columns, and that is deliberate too** (issue #47): the
  window title is what the user reads, the application id is what the
  classification of §7 can key on — a note from a terminal is probably a
  command-line note whatever its title says. They are two different facts about
  one moment, not two spellings of one, so the argument against `is_todo` does
  not apply. Both are NULL while the setting of §13 is off, and both are NULL
  when nothing could be determined; the story that brings them brings its own
  migration (decision E2).

- **There is no `is_todo` column, and that is deliberate** (issue #14,
  29.08.2026): it would be exactly `task IS NOT NULL`, and two columns whose
  truth has to agree are the first place that drifts apart. So the presence of
  the task fields *is* the statement that the note is a task — an answer that
  calls a note a task without saying what is to be done carries no task, and
  the classification of that note stands all the same.

- Audio lies as a file under `audio/` (name = the note's ISO timestamp with the
  colons replaced, section 4), the DB holds the reference. Deleting a note
  deletes tags, embedding, FTS entry, `proposal_notes` references, an
  outstanding transcription job and the audio file in one transaction plus a
  file system cleanup.

### 5.2 Settings and secrets

- Settings: **KConfig** (`~/.config/denkzettelrc`).
- API keys (openrouter, OpenAI): **KWallet** — never in plain text in config
  files. (OAuth tokens only if the later Codex app server additional path from
  7.5 is ever built.)

## 6. Search

Full-text search through FTS5 with **operators in the search field** (decision
of the third interview). Extent of the syntax (this answers open question 3 of
the concept):

| Operator | Meaning |
|---|---|
| `tag:backup` | Notes with the AI tag `backup` |
| `kat:todos` | Category (todos, ideen, cli, persoenlich, software) |
| `typ:text` / `typ:audio` | Note type |
| `vor:2026-07` / `vor:2026-07-15` | created before a date (month or day) |
| `nach:2026-06` | created from a date on (month or day) |
| `"exact phrase"` | Phrase search (FTS5 phrase) |
| free text | FTS5 full text (ANDed terms) |

- All components are **ANDed**; no OR, no brackets (v1).
- Unknown `xyz:` prefixes are treated as full text (no error).
- **`nach:` includes the day it names, `vor:` excludes it** (user decision
  2026-08-29, issue #114): `nach:2026-06-15` finds the notes of the 15th of
  June itself, `nach:2026-06` those from the 1st of June on, while
  `vor:2026-06-15` stops before that day. Everyday language beat symmetry
  here. A note of the named day therefore belongs to `nach:` and not to
  `vor:`, and the two split the corpus at that day instead of leaving it out
  of both, as the earlier excluding reading of `nach:` did.
- **`tag:` and `kat:` fold ASCII case and nothing else.** Both compare the
  stored value `COLLATE NOCASE`, which knows `a`/`A` and no other alphabet:
  `tag:BACKUP` finds `backup`, `tag:BÜCHER` does **not** find `bücher`. The
  full text is tolerant exactly where these two are not — „bucher" finds
  „Bücher" through `remove_diacritics 1` below. Accepted, because nothing the
  application stores by itself runs into the ceiling: the analysis run writes
  tags in lower case and the categories are the ASCII values of the table
  above (7.2). Lifting it needs a folding of its own, like the ß/ss story S30.
- The parser is a pure function `QString → SearchQuery` — unit-testable.
- FTS5 tokenizer: **`trigram remove_diacritics 1`** (user decision
  2026-08-01, issue #8). A search term finds **parts of words at any
  position**: „grafieren" finds „fotografieren", „bahn" finds „Straßenbahn",
  „sprech" finds „Besprechung". „bucher" finds „Bücher" — umlaut tolerance
  remains a core requirement and is preserved with `remove_diacritics 1`.
  - **Prefix search is thereby no longer a decision of its own.** It is
    contained in the substring behaviour. The query appends **no** `*`:
    measured on the system, `"foto"` and `"foto"*` are identical with the
    trigram tokenizer — it produces nothing but complete three-character
    tokens, and a prefix character can extend nothing about those. A `prefix=`
    index is not created either (resolution E2).
  - **Price, measured at 20,000 notes:** the trigram index is about **six
    times** as large as a `unicode61` index (1.8 MiB → 10.9 MiB) and thus a
    good three times as large as the raw text itself. For the expected corpus
    that is bearable; at six-figure note counts it would have to be assessed
    anew.
  - **Limit (finding of issue #8, SQLite 3.53.4):** the tokenizer removes
    diacritics. `ß` carries none — it is a letter of its own and stays. That
    is why „strassenbahn" does **not** find „Straßenbahn", and „grosse" does
    not find „Größe". Holds for `unicode61` as for `trigram`; folding ß/ss
    demands a tokenizer of its own and is a story of its own (S30).
- **Search terms of fewer than three characters (decision of issue #8):** a
  trigram index cannot contain them by construction — a trigram is three
  characters long. Such terms are therefore **compared as substrings directly
  against `notes.content`** (`LIKE '%…%'`), the rest still through the index;
  both routes are ANDed. Reason: „KI", „PO" or „ad" are real search terms, and
  a search that stays wordlessly empty over them would be a fault nobody
  recognises as one. The alternative route — a hint in the empty state — would
  turn a pure implementation limit into a rule the user has to learn.
  **Cost measured** (20,000 notes): 3 ms per query, less than the index query
  itself (9 ms) — the objection of the full table scan does not carry at this
  order of magnitude.
  - Limit of this route: it ignores upper and lower case for ASCII only („ki"
    finds „KI"), but folds no diacritics („u" does not find „ü") and no case
    beyond that („ü" does not find „Ü"). Affects terms of one or two
    characters exclusively.
- **No upper bound on the result list** (user decision 2026-08-28, measured in
  issue #78). At the 20,000 notes this section sizes the index for, a term that
  matches every note costs **120 ms** in `Store::search()` and **26 MiB** for
  the list; the same corpus with 50 hits costs 0.4 ms. Fetching the rows is
  where the time goes — the same query with `LIMIT 200` takes 26 ms — so a
  limit would buy some 95 ms per keystroke and cost a number, a hint line, an
  entry in the drawing and a rule the user has to learn. The 3.7 s the window
  needed at that size were not the unbounded list: they were one re-read of
  `kdeglobals` per row out of `NoteListDelegate::sizeHint()` (issue #110). The
  bench that produced these figures is `tests/searchbench.cpp`; it builds its
  own corpus in a temporary directory and is deliberately not an `add_test()`.
- The result list keeps the order of the library (newest first, 9.) instead of
  the FTS5 relevance sorting — only that way does it carry the library's day
  groups.
  - **BM25 was checked on 2026-08-04 and rejected** (user decision). The
    reason is **not**
    the trigram tokenizer — the assumption that a search term falling apart
    into trigrams distorts the formula is refuted: FTS5 sums BM25 over
    **phrases**, not over tokens, and the query is built phrase by phrase.
    BM25 was rejected for three other reasons:
    1. **With short notes it degenerates into "shortest note first".** A term
       being searched for stands there almost always exactly once (`f=1`); with
       a single search term the IDF share cancels out, and what remains is the
       length normalisation. A one-line note in which "backup" occurs in
       passing would stand before the detailed note about backups. `k1` and `b`
       are **not adjustable** in FTS5 — there is no control against it.
    2. **Two search routes would have no rank at all:** terms of one or two
       characters run through `content LIKE` and do not stand in the FTS index;
       pure filter searches (`tag:`, `kat:`, `vor:`) contain no full-text term.
    3. **The sorting is not a property of the search but the precondition of
       the list display.** The row building groups, it does not sort; input
       ordered by rank produces day heads several times over and in changing
       order. On top of that the timestamp of each entry hangs on its group
       (9., drawing 3b): without a head every hit from today and yesterday
       would lose its day. And since a search runs on every key press, the list
       would jump while the user is still typing.
  - **What is missing instead is the legibility of the hit**, not its rank: the
    term found is not highlighted in the list, and the place it was found in
    often lies in the part that was cut off (note 2 of the S6 review,
    sprint 3). A story of its own.

## 7. AI pipeline

### 7.1 Provider abstraction

Interface `AiProvider` with two capabilities: `chat(prompt) → text/json` and
`embed(text) → vector`. Implementations:

- **Ollama** (local/own URL): `/api/chat` + `/api/embed`.
  Defaults: LLM `qwen3:8b`, embedding `bge-m3` (multilingual).
- **openrouter.ai**: OpenAI-compatible API, API key from KWallet.
- **OpenAI**: by platform API key (see 7.5).

**Both capabilities are selectable per provider** (customer decision
29.08.2026, issue #130). Until that date this section bound embeddings to
Ollama for the whole of v1, on the ground that openrouter offered no embedding
endpoint — measured on 2026-07-31 and true then. It is not true now:
openrouter lists 34 embedding models beside its chat models. The restriction
therefore falls with its reason, and it falls for the same purpose the customer
names for the whole provider choice: **a machine without the compute to run a
model locally, or the wish to use a distinctly stronger one.** Ollama stays the
default; nothing about a local installation changes.

What that costs, and it is owed before the switch rather than after (issue
#130): vectors of two different models are not comparable, so a change of the
embedding model invalidates the stored corpus and the cluster threshold of 7.3
has to be calibrated again; and the note text leaves the machine, which the
local route was the one way to avoid. Whoever offers the choice states both
where it is made.

Without a reachable provider for embeddings, Denkzettel degrades visibly:
classification through the chosen provider keeps working, topic bundles are
dropped (hint in the settings and the tray tooltip).

All calls through Qt Network, asynchronous, with one retry and **two limits per
call**: 30 s in which nothing at all arrives, and 5 minutes for the whole call
(decision 30.08.2026, issue #121).

**The 30 s measure silence.** The chat call streams and its lines are put back
together before anybody sees them, so 7.2 still reads one JSON document per
note. Unstreamed, the limit covered the model load and the whole generation in
one budget, and the first note after a start paid one of its two attempts for a
server that was merely cold. Measured on 30.08.2026 against Ollama 0.32.15 and
`qwen3:8b`: reading the 5.2 GB blob at 5.9 GB/s of cold throughput takes under
a second, an 18 GB model about 3.1 s.

**The load is the smaller half, and what decides the time is how many tokens
the model reasons for.** At a throughput measured constant to 1.2 % — 92.5 to
93.6 tokens per second over five runs — 30 s hold about **2,800 output
tokens**, and a reasoning of that length is ordinary for a thinking model
rather than exceptional. Identical input produced 477 to 1,385 tokens in those
five runs and 1,466 to 4,153 in four runs of another note — both series
`eval_count`, read off the answer. The second took 18.1 s to 46.9 s of wall
time, and the same rate accounts for it once prompt evaluation and transfer are
added: 1,466 tokens are 15.8 s of generation, 4,153 are 44.7 s. The conversion
rate is what carries here; the seconds are a snapshot of one note on one
machine and say nothing about how often the limit is reached.

**The 5 minutes bound the call**, because the limit above no longer does: a
server sending one byte every 29 s would hold an analysis run for ever, and
nothing else ends one — 7.2 carries the interval between runs, not a bound on a
call. Exceeded, the request is aborted, and what that costs depends on which
step was calling — the two are not the same and 7.2 decides it, not this
section. **Classification counts one attempt**, as it does for every failure:
the chat answer carries no failure value, so the run goes on to the next note
with this one a step nearer its limit. **Embedding counts none**: the failure
is an unreachable backend, and there the run stops instead, on the ground that
the next note would fare exactly the same. Either way the call ends, which is
the point — a run has to end, and a hanging call reaches no reporting channel
at all (§14). The bound is deliberately generous: healthy calls were measured
up to 46.9 s, and a tight one would repeat issue #121 one storey up.

**It is §12's number and not §12's rule.** Transcription allows five minutes
per **job**, and the ground there is the product — Denkzettel is no audio
recorder. Here it is five minutes per **call**, and the ground is the loop
discipline of 7.2: turn a silent hang into an error the attempt counter can
deal with. Same number, two reasons, and neither follows from the other.

"Test connection" in the settings makes one mini `chat` call and (with Ollama)
one `embed` call each and shows the latency or the error.

### 7.2 Analysis run

Trigger according to the setting: **at once** (after saving/transcription),
**periodically** (interval, default 30 min) or **on demand** (tray/D-Bus).
A run processes all notes with `state != 'analysiert'` as well as — for step 2
only — those with `needs_reembed = 1`; at most 50 notes per run (budget,
section 14), the rest follow in the next run:

1. **Classification + tags** (one LLM call per note, JSON schema:
   `{category, tags[], is_todo, task?}`): category from the fixed list of
   section 6 — `todos`, `ideen`, `cli`, `persoenlich`, `software` —, 1–4
   tags in lower case. For `is_todo=true` the same call extracts the task
   fields (`description, project, tags, due, priority` — `due`/`priority`
   only on a clear signal, otherwise null).
   **A due date is read from the note's own day, and it is checked afterwards**
   (decision 29.08.2026, issue #117). The prompt names the `created_at` of the
   note — the day it was written, not the day of the analysis run: a note from
   the day before yesterday saying "morgen" means the day after the day before
   yesterday, and a run catching up on a week of notes would otherwise date
   them all to itself. The answer is then held against that day: a `due` before
   it, or more than a year after it, is dropped and the field stays null.
   Both halves are needed, and the reason is a measurement — asked without a
   day in the prompt, qwen3:8b answered a note containing the word "Morgen"
   with a `due` in 2023, well-formed, out of its training data and past every
   format check. The prompt is a request to a model and assures nothing about
   the answer; sections 7.4 and 8.2 carry this field into a real task list,
   where nothing about the value says it was guessed.

   **What the classifier writes is what the user types** (user decision
   2026-08-29, issue #114): the category values are the ones section 6 offers,
   without umlaut and without hyphen, so the search stays a literal
   comparison. The readable label for the sidebar, the chips and the detail
   view is a matter of the user interface and is made there (M3); what stands
   in `notes.category` is the short form.
2. **Embedding** (one `embed` call per note) → `embeddings` table; resets
   `needs_reembed`.
3. **Clustering + suggestion generation** (7.3/7.4).

Error handling follows the loop conventions: failed attempts are counted
persistently (`notes.analysis_attempts`/`analysis_last_error`, survives daemon
restarts; success resets them). From the second failure on, the note is skipped
and the error is reported in the tray tooltip + log — no endless retrying, no
self-healing.

### 7.3 Topic clustering (answers open question 1)

- Basis: cosine similarity of the embeddings of all **unexported, analysed**
  notes (brute-force pairwise comparison; at an overflow threshold of ~200
  notes that is ≤ 20k comparisons — uncritical, no vector DB).
- Method: single-linkage chaining — pairs of notes with a similarity ≥
  **0.60** (internal constant, calibratable, no user setting) land in the same
  cluster.
- Clusters with ≥ **bundle threshold** notes (setting, default **3**) are laid
  before the LLM: it names the topic and may remove obvious outliers (sanity
  check). Result: one `bundle` suggestion.
- **The Markdown of the collective note is built by the program, not by the
  model** (decision 29.08.2026, issue #29). An earlier wording had the LLM
  generate it, and that was wrong twice over: section 8.1 prescribes the form
  down to the frontmatter and the chronological `## <YYYY-MM-DD>` sections, so
  a generated note could only ever agree with it by accident — and a model that
  writes the note writes the note **texts** too. Section 7.2 settles that the
  other way: what the classifier writes is what the user types. Whoever lets
  the model reformulate here changes what the export of 8.1 hands back to the
  user, in the one place where the original is then deleted.
- **Notes without a cluster** simply stay in the corpus. If the age threshold
  of the overflow guard is crossed, the service additionally generates a bundle
  "Miscellaneous from <period>" out of the oldest notes without a cluster —
  likewise only as a suggestion, every note deselectable.
- Notes already suggested but deferred (`status = 'zurueckgestellt'`) are
  clustered again in the next run — a "Later" only postpones, it hides nothing
  permanently.

### 7.4 Task suggestions

For every note classified as a TODO, a `task` suggestion comes into being with
the extracted fields. No automatic `task add` — execution only after
confirmation in the review (section 9).

### 7.5 OpenAI hookup

The decision of the third interview (OAuth "Sign in with ChatGPT" from the
start) stood expressly under reservation of research. The research
(2026-07-31, complete with sources:
`recherche/2026-07-31-openai-oauth-machbarkeit.md`) triggered the reservation:

- **"Sign in with ChatGPT" is a pure identity procedure** (beta, six curated
  partners, no public self-registration). The application would receive name,
  e-mail address and profile picture — expressly no tokens and no model access,
  according to the OpenAI documentation.
- The unofficial **Codex OAuth subscription route** (Codex CLI, OpenClaw) can
  be inspected technically, but was never released for third-party apps (ToS
  grey area, left unanswered by OpenAI several times since 12/2025) — and it
  delivers chat endpoints only: **no embeddings**, which Denkzettel's AI
  architecture (7.1/7.3) necessarily needs.

**Consequence for v1: OpenAI by a manual platform API key** (kept in KWallet),
on equal footing beside Ollama and openrouter. The settings page shows a short
hint text with OpenAI, explaining why there is no "Sign in with ChatGPT". A
subscription route through the Codex app server (JSON-RPC/stdio) remains
conceivable as an optional later additional path, but is not built for v1.

## 8. Transfers

### 8.1 Obsidian export (bundles)

- Target note: **one collective note per topic** in `<vault>/_INBOX/`, file
  name `Denkzettel <topic> <YYYY-MM-DD>.md`.
- Structure: vault-conformant frontmatter (`type`, `tags`, `created` — verify
  against the vault CLAUDE.md conventions when building), `# <topic>`, then
  `## <YYYY-MM-DD>` sections with the notes as paragraphs/bullets
  (chronological), like the Markdown preview in the wireframe.
- Voice notes export their **transcript**; the audio file is deleted on export
  (decision of the concept: audio lives only as long as the note).
- After a confirmed export: delete the notes in one transaction (including
  audio, tags, embeddings, FTS) and **remove the suggestion together with its
  `proposal_notes` references** — accepting and discarding end the same way,
  except that accepting executes beforehand. An "accepted" state does not exist
  (pass-through storage, no history of suggestions).

### 8.2 Taskwarrior

- Execution by `QProcess`: `task add <description> project:<p> +tag1 +tag2
  due:<d> priority:<p>` (only populated fields), afterwards, with a longer note
  text, `task <uuid> annotate <full text>` (UUID from the `task add` output).
- Error case (task binary missing, exit ≠ 0): the suggestion stays open, the
  error is shown on the card — nothing is lost.
- After success: delete the note and remove the suggestion (as in 8.1).

### 8.3 Full export (rescue path)

- Menu item in the library: exports **all** notes as a folder
  `denkzettel-export-<date>/` with one `.md` per note (ISO name, frontmatter
  with category/tags/type) plus an `audio/` subfolder with the original files.
- Purely reading — the corpus stays unchanged. No AI call needed.

## 9. Library and suggestion review

- **Library** (window): sidebar with AI categories + counters, chronological
  note list, structured into day groups like an inbox (**Today · Yesterday ·
  This week · Last week · Older**; "week" is the calendar week, its start
  follows the locale of the system — `QLocale::firstDayOfWeek`, Monday in
  Germany), newest first within the groups.
  An entry shows the timestamp, the first line as the subject and the following
  text as a preview; voice notes additionally ▶ and the duration.
  **No chips in the list row** (UX decision 2026-08-29, issue #18, retiring what
  wireframes 2b and 3a draw): the row is two lines of text high and has carried
  the search mark since issue #77, so pills would be the third area style on it
  and compete with mark and selection. What a category is for finding again is
  the column, not a pill per row.
  The timestamp does not follow the group: an entry shows date and time in
  every one of the five groups, in the arrangement the locale gives them and
  with a four-digit year, without seconds. The detail pane carries the same
  date and time plus the weekday, again the same form whichever group the note
  falls into — "Today" and "Yesterday" stay group heads and appear nowhere
  else (issue #108). **The timestamp carries seconds nowhere**, the detail
  pane included (issue #124, retiring the half of #108 that put them there):
  a note is a thought written down in passing and not a measurement, and the
  head row of the detail pane is the one row where the space is short — the
  reading pane is 440 px of a 900 px window, 168 of them taken by the two
  buttons. Without the seconds the timestamp label measures 130 logical px
  under de_DE instead of 145, and 133 instead of 148 under en_US (measured in
  the built window, 2026-08-30). The running time of a voice note is not a
  timestamp and keeps its seconds — "▶ 0:41" is a length, not a time of day.
  The structure is fixed — no toggle, no collapsible groups (wireframes
  3a/3b).
  **Separation is by two hairlines of one colour. The note boundary carries its
  line on the edge of the row, the group boundary carries it beside the label
  of its head, and the head carries a rank of type of its own on top of that**
  (wireframe 3a, user decisions 2026-08-06 on issue #101 and 2026-08-11 on
  issue #104):
  - Between two consecutive notes of the **same** group a line inset to the
    text edge (12 px left and right, the same 12 px at which the timestamp and
    the head text begin). None under the last note of a group, none under a
    head, and **none at an edge of the selected row** — a second separation
    there would compete with the selection mark.
  - Beside the label of **every** group head a line on half the height of the
    head type, beginning 8 px after the end of the label and running out to the
    right text edge (the same 12 px). That is the form of
    `Kirigami.ListSectionHeader`. It stands even when the note above it is the
    selected one: the exception above applies to the entry line alone, and this
    line does not lie at the edge where the two rows touch at all.
  - The **group head is set in the text size of the application**
    (`QFontDatabase::GeneralFont`) and **bold**, the note text stays normal.
  The difference between the two boundaries is thereby **categorical** — a
  different place for the line **and** a different rank of type, and neither of
  the two is a degree of the other. A version in which the extent of the lines
  alone carried the ranking was rejected by the user at the sprint 9
  acceptance: „Ich sehe nicht direkt wo eine endet und die andere anfängt."
  (I do not see directly where one ends and the next begins.)
  The colour is **not a palette role** but the mixture of list ground and text
  colour in the ratio `KColorScheme::frameContrast()` — the method with which
  Kirigami colours its separators. Alternating row colours are no good for
  that: over 18 schemes they stand out between 1.00 : 1 and 1.21 : 1, and at
  every group boundary at 1.00 : 1.
  **The thickness is a measure in device pixels** (discovered during the
  implementation): whole device pixel rows, at least one, rounded and not
  truncated. Filled as a logical rectangle, under odd scalings the same
  line came out one device pixel thick above one entry and two above the next —
  and then the thickness said the opposite of what the structure says. A guarantee about the
  **position** of the upper edge does not exist, by contrast: uniformity is
  carried by the whole-number height alone.
  **Condition, discovered during the implementation:** where the label
  together with its 8 px of clearance fills the text width, **the head line is
  dropped**. Without the clearance it would read as an underscore of the last
  letter. The case is not reachable in the list — the longest label measures
  87 px, the narrowest list leaves 175 px — but the rule stands, because a
  guarantee must not reach further than its evidence.
  **Condition, discovered during the implementation: the view does not redraw
  the upper neighbour of a selection change of its own accord.** It paints only
  the stretch between the old and the new selection, and the row above both
  ends lies outside it — without an express registration a line stays standing
  there or one is missing (issue #101). A still image does not show that:
  `grab()` redraws every row.
  The groups are recalculated when the list is built and at every window
  activation — there is no midnight timer (wireframe 3b).
  **Regrouping happens only if the calendar day is a different one than at the
  last build** (discovered during the implementation): regrouping resets the
  model and restores the selection, which scrolls the list to it — without a
  change of day an Alt-Tab threw the reader back onto their selection
  (issue #59). The calendar day suffices as a condition, because all four group
  boundaries are day boundaries.
  **A note that comes into being while the library is open appears in the list
  without any further doing** (user finding 2026-08-11, issue #105).
  The report comes from the store, not from the capture window: the route
  through D-Bus (`AddNote`, 2.3) writes without any window at all, and both
  routes go through the same door. Three conditions, discovered during the
  implementation:
  **(a)** If a deletion period is running, the new note waits for its end —
  the deleted note is still in the store until then and would come back on
  re-reading into a list that counts it away. The period is **not** shortened
  for this (unlike with the search term, 6, which the user types with the
  report in view): what the user writes in another window must not use up the
  undo that is still being offered here.
  **(b)** The reading position stays: the row at the upper edge keeps its
  place, the note being read keeps its selection. Excepted is the list that
  stands **at the beginning** — there the beginning is the place of the new
  note, and there it has to be seen.
  **(c)** If a search term stands in the field, it applies to the new note as
  well; it comes into the list only if it matches.
  If the selection jumps **by key** across a group boundary, the list brings
  the head of the new group into view (wireframe 3b, case 4) — **and likewise
  when the selection reaches the first note of a group by key without crossing
  a boundary** (issue #70, user decision 2026-08-04): whoever goes upwards from
  the second to the first note would otherwise never get to see the head, and
  without it nothing says which of the five groups the entry stands in — the
  entry names its own date and time, never the group (issue #108). The price is
  that the list moves further in this case than before. It still holds for both
  cases: if head and selection do not fit into the view together, the head
  stays outside.
  **A mouse click does not move the list at all** — it neither fetches the head
  nor scrolls up to the selection: whoever points expects the place they
  pointed at to stay, and scrolling forward would tear it away from under their
  pointer (issue #57).
  **That the scrolling up to the selection falls under this as well was
  discovered during the implementation:** a click on a partly visible row moved
  the view by one row height, and because the view determines its selection
  only afterwards, out of the rectangle remembered at the press, it marked the
  neighbouring row (issue #71).
  The price, expressly: a partly visible row stays partly visible after the
  click. Making it fully visible would mean pulling it away from under the
  pointer — that is the fault itself.
  **Condition, discovered during the visual check: "does not move the list at
  all" holds for the press, not for the second afterwards.**
  `QAbstractItemView` starts a **delayed autoscroll** at the mouse press and
  does bring the partly visible row into view about half a second later; the
  mark stays on the clicked row in the process. Whether the after-run stays or
  is switched off is open (issue #89); until the decision falls, the condition
  stands here — a sentence that guarantees more than the build holds is a trap,
  whichever way it turns out.
  The day is not lost in the process — the detail pane carries the full
  timestamp.
  In addition the search field (section 6) and the button "Suggestions" with a
  badge.
- **The category column** (wireframe 1b, issue #18): "All" with the number of
  all notes, then the five categories of section 6 in the order they stand
  there, each with the number of notes carrying it.

  **A click writes `kat:<short form>` into the search field** (UX decision
  2026-08-29): the column is not a second way of filtering beside the search,
  it **is** the search, written out — so it is the place where the language of
  section 6 is learnt, by whoever never went looking for a help text. From that
  follow four things, and they are the whole behaviour:
  - **The click sets the text *and* searches.** A word standing in the field
    over a list that still shows the old result reads as a fault.
  - **A search already standing is added to, not replaced.** An existing `kat:`
    is overwritten — two would ask for a note with two categories — and
    everything else stays: whoever typed `nach:2026-06` and then clicks a
    category wants both.
  - **"All" is the absence of a `kat:`**, not a value of its own: a click on it
    takes the operator out of the field and leaves the rest standing.
  - **The mark of the column follows the field, never the other way round.**
    Deleting `kat:software` by hand takes the mark off that entry, typing it
    puts it there; a `kat:` no entry carries, or two of them, leave the column
    marking nothing. Otherwise two truths would stand beside each other.

  The counters count the **whole** library and not the list beside them, so
  they say what a click would find and not what is already found — they do not
  move while the list narrows.
  - **The readable label is made in the window** (7.2): the database keeps the
    short form, the column writes „TODOs · Ideen · CLI-Befehle · Persönlich ·
    Software-Ideen". The list is fixed. A category value the list does not know
    — which only an older database can carry, the classifier of 7.2 writing
    none — is counted under "All", gets no entry of its own and stays reachable
    through the search: an entry generated from the contents would give the
    column a shape that changes with them.
  - **Two entries beyond the five, in the order a note travels through them:
    "Wartet auf Analyse" and below it "Nicht eingeordnet"** (issue #133, UX
    decision 30.08.2026). The first holds the notes the analysis has not
    reached yet, the second those whose classification attempts are used up
    (7.2, `analysis_attempts` at `Store::analysisAttemptLimit`). The second is
    where such a note can be dealt with; without it the tray message of 10
    would report a state the window offers no way into.

    **Both carry "no category" as their first condition**, and that is what
    makes the column add up: a note with a category stands in its category
    entry, a note without one in exactly one of these two. What "Alle" names is
    therefore the sum of the entries below it, for every row the database can
    hold — the column promised that sum from the first drawing (wireframe 1b
    adds five counters to "Alle 128") and did not keep it until then. A note
    carrying a category **and** used-up attempts is counted under its category
    and in neither of the two: it is not uneingeordnet, whatever its attempt
    counter says. `Classifier::start()` still reports it through `paused()`,
    which asks nothing about the category — the two sets are deliberately not
    the same any more.

    Neither entry is **shown while it counts nothing**, unless it is the
    selected one — a state that is not there gets no permanent line, and an
    analysis that has caught up leaves the column as the five categories.
    **They are the two entries that write nothing into the field**, because the
    search language of section 6 has no operator for "the classification was
    given up on" or "the analysis has not been here yet"; a word of their own is
    a change to that language and is the customer's to make. So each carries a
    condition of its own, and the field cannot contradict it: a word typed while
    one of them is chosen narrows **within** that entry, and only a `kat:` moves
    the mark away.
  - The column is **switched off while a note is being edited**, for the reason
    the search field is: it rebuilds the list under the editor.
- Detail view: **read and edit view** (decision of the third interview — above
  all for faulty transcripts). Editing keeps category/tags and `state`, but
  sets `needs_reembed = 1` — the next analysis run renews the embedding only
  (7.2), since it ages with the text. Delete action with a 5-second undo
  (addition of the spec, not in the concept: purely client-side delayed
  deletion, no soft-delete state in the DB).
- **Category and tags in the reading pane** (wireframe 2b, UX decision
  2026-08-29 on issue #18): a row of pills under the note text, the category
  first and then the tags. **Told apart by the filling, not by the shape** —
  the category filled, a tag outlined, both in the colour the separator lines
  of the list are mixed in (see above), so a value out of a fixed list looks
  different from several free ones without a second shape.
  Without a category and without tags the row is **hidden** — no placeholder
  and no reserved height; until the analysis run of M3 has been through the
  library that is the normal case.
  **The edit state keeps its label row** (wireframe 2a): there the two are
  fields, not marks, and they become editable later. "One appearance" therefore
  holds within the read state — two states, two appearances, which is the
  difference between showing and entering.
- **Conditions of the edit state** (S8; the last two discovered during the
  implementation):
  - Unsaved changes are **never** written or discarded **without asking**. A
    change of selection, closing the window, Esc and "Cancel" all lead to the
    same dialog with **Save · Discard · Cancel**. That deliberately differs
    from the capture window, where Esc discards silently (3): there stands a
    draft that was never saved, here a note that has already been saved.
    "Cancel" is in the dialog as well because the button and the shortcut are
    the same action — a misclick on it is precisely the case the dialog is
    framed against.
  - The saved note **stays standing in the running result list**, even if its
    new text no longer matches the search term (6); only the next change of the
    search term reads the store anew. Otherwise the note would disappear under
    the hand that had just corrected it.
  - The **search field is switched off** meanwhile. A search rebuilds the list;
    the note under the editor can fall out of it in the process, and then the
    dialog has no row left to take the selection back to.
  - **Make of the dialog (#66):** the guard is a **`KMessageDialog`** of type
    `WarningTwoActionsCancel` with `KStandardGuiItem` icons; **the default
    answer is "Save"**. The reason is a condition discovered during the build:
    under the KDE platform integration (`QT_QPA_PLATFORMTHEME=kde`) the system
    answers a constructed `QMessageBox` with a **message window of its own
    including button objects of its own** — it takes over the labels, roles and
    order, but nothing that is set on the `QPushButton` afterwards (icons,
    default/escape button). A `KMessageDialog` is an ordinary `QDialog` and
    stays our own. From that follows for the check: the dialog test measures
    the dialog the application **shows**
    (`QApplication::activeModalWidget()`), with the platform theme set — a test
    without a platform theme measures a dialog no KDE session user sees.
  - **Conditions of this make, all discovered during the build:**
    - `KMessageDialog` knows **no secondary text** (`informativeText`);
      question and explanation stand in one text, separated by a blank line.
    - The answer roles are `Yes` · `No` · `Reject` instead of
      `Accept` · `Destructive` · `Reject`. **What is guaranteed is the
      meaning** (Save writes and carries out the action, Discard carries it out
      without writing, Cancel stays in the editor), not the role and not the
      order.
    - The default answer **follows the focus**: with buttons capable of making
      themselves the default, the change of focus makes the focused button the
      default. The KDE make gives focus and default to "Cancel" when the dialog
      becomes visible; "Save" must therefore receive focus *and* default
      **after** the showing, and a guarantee about it is only valid once it is
      measured on the **visible** dialog.
    - If the dialog is shown by hand, it is **no longer made modal** by a later
      `exec()`; the modality is then to be set oneself.
    - The **warning icon** (`dialog-warning`) is set **expressly**.
      `KMessageDialog::setIcon()` does promise to choose one by dialog type
      when the icon is empty — measured, none comes, and the dialog then
      carries no image label at all. A dialog about impending data loss is the
      core case of the warning icon (design decision 2026-08-02; drawing 2a,
      state C brought up to date).
    - **The make sounds:** `showEvent()` reports the KNotification event
      `messageWarning` at every showing, to which `plasma_workspace.notifyrc`
      assigns the system sound `dialog-warning`. That is KDE platform standard
      and **stays that way**: volume and muting are governed by the user in the
      system. `KMessageDialog::setNotifyEnabled(false)` would switch the sound
      off; **precisely that is deliberately left undone** (user decision
      2026-08-04), so that nobody later takes it for an oversight and removes
      it. Silent are the test and image runners alone: they steer libcanberra
      onto the null driver before `main()` (`tests/testsilence.cpp`).
- If the note is part of an **open suggestion**, editing or deleting discards
  that suggestion (its preview would be out of date); the next analysis run
  generates it anew on the current state.
- With voice notes: audio player (play/pause, progress, time) above the
  transcript.
- **Suggestion review**: list of open suggestions of both kinds. Bundle card:
  title, note list with deselect checkboxes, Markdown preview, target
  "→ Obsidian _INBOX". Task card: editable fields (description, project, tags,
  due, priority), annotation preview, target "→ Taskwarrior". Actions per card:
  **Accept · Later · Discard** (Discard deletes only the suggestion, never
  notes).

## 10. Tray and notifications

- **KStatusNotifierItem**, permanent. **One** menu, in three groups; wording
  and icon names are binding (wireframe 5a, issue #60). The icons come without
  exception from the icon theme: only an icon from the theme carries a name,
  and what goes over the tray protocol is the **name**, not the image.

  | Entry | Icon name | State | Shortcut hint |
  |---|---|---|---|
  | Capture note | `document-edit` | active | Meta+N |
  | Record voice note | `audio-input-microphone` | inactive until M4 | Meta+Shift+N from M4 |
  | *— separator —* | | | |
  | Open library | `view-list-text` | active | — |
  | Analyze now | `system-run` | inactive until M5 | — |
  | Suggestions (counter) | `tools-wizard` | inactive until M5 | — |
  | *— separator —* | | | |
  | Configure Denkzettel… | `configure` | **only with #16** | — |
  | About Denkzettel | `help-about` | active | — |
  | Quit | `application-exit` | active | — |

  **"Quit" stands set apart in the last group** and never beside the most
  frequent entry: it ends the service and with it the shortcut. Until the
  settings dialog (#16) exists, "Configure Denkzettel…" is **no entry at all**
  — a permanently greyed-out one does not explain to the user why it is grey
  (KDE HIG).
- **"About Denkzettel" opens `KAboutApplicationDialog`** with the global
  `KAboutData` (issue #87) and stands in the last group, above "Quit". That is
  the place, not the library window: the library is one route among several of
  equal rank (2.1), and a statement about the whole application must not hang
  off one of them. The daemon has no menu bar and no main window, so the tray
  menu is the only application-wide menu there is. The dialog is not written by
  hand — `KAboutApplicationDialog` out of **KXmlGui** is the same dialog every
  KDE application shows, and it is the one thing this project uses out of that
  framework.
- **The shortcut hint is a hint.** Meta+N stands as text on the entry and must
  not duplicate the registration with KGlobalAccel; the shortcut of the menu
  action therefore carries `Qt::WidgetShortcut` and reaches only the window of
  its menu — and it has none, plasmashell draws it.
- **A left click on the tray icon opens the same menu as the right click**
  (`ItemIsMenu`). That deliberately differs from the KDE standard, which
  foresees a main action for the left click: Denkzettel has no main window but
  several routes of equal rank, and the research on the KDE behaviour was laid
  before the user. User decision of 2026-08-01, documented in issue #44,
  confirmed on 2026-08-02 after being laid before them again.
- **Discovered condition (issue #60): separate menus for
  left and right click are not to be had under Plasma/Wayland.** They would
  mean `ItemIsMenu=false` plus a menu of our own in the
  `activateRequested` handler; Denkzettel would then have to draw that menu
  itself. As a popup it is created and closed again two milliseconds later — a
  `Qt::Popup` needs a parent surface and an input grab under Wayland, and a
  client with nothing but a tray icon has neither. As an ordinary window it
  stays standing, but the desired position is discarded and KWin puts it in the
  middle of the screen. It therefore stays at one menu; the user decided on the
  fallback on 2026-08-02. Of the
  three HIG deviations named in wireframe 5a, only **A1** thereby remains
  (left click opens a menu); A2 (two different menus) and A3 ("Quit" only
  through the right click) fall away without replacement, because the second
  list does not exist.
- **Second discovered condition (same place): the icon names reach Plasma only
  as long as an icon theme is resolvable.** Without a platform theme the search
  paths contain nothing but the Qt resource, `QIcon::fromTheme()` delivers an
  empty icon without a name, and the menu would arrive unillustrated. Under
  Plasma the theme is there; for test runs `QT_QPA_PLATFORMTHEME=kde` is
  therefore a precondition, not decoration.
- Icon states: normal · "suggestion waiting" (badge) · error state (analysis/
  transcription error, the tooltip names the cause).
- Icons in v1 out of the Breeze stock (application and tray icon derived, badge
  drawn as an overlay) — no graphics work of our own.
- **KNotification** on: a new package of suggestions, an overflow reminder, a
  repeated error. No notification for routine runs.
- **A transcription that has finally failed is one of those repeated errors**
  (customer decision 29.08.2026, issue #115). Where the attempts of §12 are
  used up, **one** notification goes out — once per note and not once per
  attempt, and not again at the next start for a job that was already given up
  on: the tray state carries that, the notification is for the moment it
  happens. Its text names the note by the moment it was recorded and the reason
  it failed, and carries **no path**: the program path is configurable (§12)
  and may lie in the user's home, and a notification travels further than a
  tooltip because it can be forwarded. What the program said last and the paths
  stay in the log (§14).

## 11. Overflow guard

- Two criteria, both configurable: **count** of unexported notes (default 200)
  OR **age** of the oldest unexported note (default 30 days).
- On crossing: tray reminder + notification; the next analysis run generates
  bundles preferentially (including "Miscellaneous", 7.3). **Never** an
  automatic export.

## 12. Transcription

- Job queue (`transcribe_jobs`), worked off serially (one GPU); survives
  restarts (queue in the DB).
- **whisper.cpp** (default): the package `whisper-cpp` from Arch `extra`, not
  the AUR — since 1.9.1-2 it is the only one, it replaces `whisper-cpp-rocm`
  and `whisper-cpp-vulkan`. The GPU backend is a package of its own that ggml
  loads at runtime (`ggml_backend_load_all`), so the choice between the two
  costs a package installation and neither a rebuild nor a line here:
  `ggml-vulkan` (52 MB installed) is the one set up, `ggml-hip` (1.2 GB) the
  way back. Measured 2026-08-28 on the RX 7900 XTX, model `small`, 7.3 s of
  audio: Vulkan 371 ms, HIP/ROCm 409 ms, CPU 2633 ms.
  Called as a subprocess `/usr/bin/whisper-cli`. **The program path is
  configurable** — the CI has no graphics card and does not transcribe there;
  what it checks of the queue and the error path, it checks with a program of
  its own put in that place.
  Audio goes through `ffmpeg` to 16 kHz mono WAV (temporary), then
  `whisper-cli -m <model> -f <wav> -l de -oj` → JSON transcript. That
  conversion stays explicit although this package reads Opus by itself: it does
  so only because the packager linked it against libavformat, and a build
  without that option refuses Opus-in-OGG — the recording format of §4 —
  outright (both measured 2026-08-28; counter-checked with Vorbis-in-OGG, which
  the same build does read, so the refusal is the codec and not the container).
  Model size configurable (default `small`, choice tiny–large-v3; download of
  the GGML models at first use with progress, kept under
  `~/.local/share/denkzettel/models/`. The SHA-1 per model stands in the
  upstream `models/README.md` — that is what an aborted download is recognised
  by, no checksum of our own).
- **A second backend is not built** (decision 2026-08-29). Up to this date
  WhisperX stood here as a configurable second route, blocked since 2026-07-31
  on an installation that was to come about in another project. Two things
  retired it: the entry itself ruled out diarisation, which is the one thing
  WhisperX does that whisper.cpp does not; and the Vulkan measurement above
  puts whisper.cpp at 371 ms for 7.3 s of audio at 52 MB of dependencies. What
  stays from the story is its settings page, which now holds the model size and
  the path to `whisper-cli` (§13) — the two values the single route does have.
  A second backend needs a new decision, not a resumed one — and that decision
  has been taken: **transcription through an external provider is an option**
  (customer decision 29.08.2026, issue #131). whisper.cpp stays the default and
  the only route that needs no account. All three providers of 7.1 offer
  speech-to-text: OpenAI `POST /v1/audio/transcriptions` (25 MB per file),
  openrouter `POST /api/v1/audio/transcriptions` (19 transcription models,
  no documented size limit), ElevenLabs `POST /v1/speech-to-text` (up to 3 GB
  and ten hours, with diarisation) — the endpoints probed on 2026-08-29 against
  an invented route as the control, which answered differently in each case.
  ElevenLabs is a fourth provider that 7.1 does not know and that would exist
  for this one capability.

  What the option costs, and it belongs beside it: the audio leaves the machine,
  the key needs the storage of 7.5, and the five-minute bound below is measured
  against a local run and has to be restated for a route whose time is spent on
  the network.
- **Upper bound 5 minutes per transcription run** (customer decision
  29.08.2026), and it is the whole job: the clock starts where the job is taken
  out of the queue and stops where the job ends, not on a stretch without
  output — whether `whisper-cli` writes anything at all while it recognises is
  unmeasured. Exceeded, the child process is killed, the job records the reason
  in `last_error` and the queue goes on to the next one. Same ground as the
  15-minute bound of §4: Denkzettel takes down short notes and is no audio
  recorder.
- Error path: 2 failed attempts → the job pauses, tray error state **and one
  KNotification** (§10, §14), the note stays visible/playable as an `audio`
  note without a transcript (nothing is lost).
- **A missing model is not a failed attempt** (decision 29.08.2026, issue #23).
  It is not a job that went wrong but a precondition not yet met — the note did
  nothing wrong, and the model may be arriving at this very moment, because the
  size can be chosen in the settings while its download is still running. The
  job stays in the queue, the counter stays where it is, and the tooltip says
  what is missing and where to get it. Without that rule the two attempts of a
  note can be spent on the gap between choosing a size and its file arriving —
  and at 1.5 GB for `medium` that gap is minutes, after which the note stands
  in the error state with the model long since in place.

## 13. Settings (dialog)

Page list according to the concept: **Capture** (see below), **AI provider**
(provider choice, LLM and embedding model, test connection), **Analysis** (at once/periodically with an
interval/on demand), **Export** (vault path with a folder chooser, overflow
thresholds count + days, bundle threshold), **Voice notes** (model size,
path to `whisper-cli`), **Shortcuts**
(KKeySequenceWidget for both shortcuts).

**Capture** stands first, and it is the page for the privacy switch of §5.1:
„Herkunft der Notiz mitspeichern" (store the note's origin), **off by
default**, with the sentence that answers what is kept — „Gespeichert werden
Name der Anwendung und Fenstertitel im Moment der Erfassung. Beide stehen in
der Detailansicht und werden dort zusammen gelöscht." **Both are stored and
both are shown**, the application before the title. Between the two the
sentence went the other way for a few hours, and the customer's own case turned
it back: a note captured out of a terminal read back the title the terminal
happened to carry, with nothing saying it was a terminal. The application id is
resolved to a name through its desktop file at **display** time — a name frozen
at capture would be a third fact beside the two columns 5.1 carries on purpose
— and where no desktop file answers, the raw id stands, because
`com.example.terminal` still says which program it was (correction of
29.08.2026, issue #47, after the customer measured it on the installed
build).
Icon
`document-edit`, the same name the tray gives „Notiz erfassen", because it is
the same action. It is deliberately **not** a line on the Analysis page,
although the origin feeds the classification: a switch against invisible data
collection whose own page nobody opens is the collection it was built against.
Findability is its purpose (UX decision 29.08.2026).

## 14. Error handling and loop discipline

The periodic analysis run is a loop in the sense of the loop conventions:

- **goal met**: all notes `analysiert`, suggestions generated → the run ends.
- **budget**: one run processes at most 50 notes (the rest in the next run).
- **stalled**: the same error 2× → skip the affected note, report it.
- **needs a human**: all transfers require confirmation anyway.

Reporting channels: tray state + tooltip (quiet), KNotification (important), log
file `~/.local/share/denkzettel/denkzettel.log` (details, with rotation).

**A transcription that has finally failed uses all three**, and that is what
the split means in practice (§10, §12): the tray state and the tooltip stand as
long as the queue holds the job that was given up on, a restart included; one
KNotification goes out at the moment the attempts are used up, once per note;
and the log keeps the full reason, including the program path and the last line
the program wrote, which neither visible channel carries.

## 15. Build, dependencies, packaging

- **CMake** + ECM (Extra CMake Modules), C++20.
- Qt 6: Widgets, Sql, Network, Multimedia, **DBus** (not only for the single
  instance and the service interface: the capture window asks KWin over it
  whether this session blurs at all — 3.2, item 9).
  KF6: KGlobalAccel,
  KConfig, KNotifications, KStatusNotifierItem, KWallet (framework: KWallet),
  **KXmlGui** (`KAboutApplicationDialog` — the about dialog behind the tray
  entry of the same name, section 10; nothing else of that framework is used),
  **KDBusAddons** (KDBusService/single instance), **KWidgetsAddons**
  (KMessageWidget — messages inside the window; KMessageDialog together with
  KStandardGuiItem — the guard dialog, section 9), **KWindowSystem**
  (KWindowConfig — window size across sessions; a Wayland client does not set
  the position itself, see section 3; **KWindowShadow** — the shadow of the
  capture window, section 3.1; **KWindowEffects** — the two other
  registrations of the hull, blur and contrast effect,
  section 3.1/3.2), **KSvg** (`FrameSvg` and
  `ImageSet` — the hull out of `dialogs/background` of the desktop theme),
  **KCoreAddons** (`KDirWatch` — the watch on `plasmarc`, through which a theme
  switch reaches a standing window; why not `KConfigWatcher` stands in 3.2),
  **KColorScheme** (`KColorScheme::frameContrast()` — the ratio in which the
  library list mixes its separator lines, section 9),
  KI18n (source language **English**; every visible string goes through
  `i18n()`, `KLocalizedString::setApplicationDomain()` names the catalogue and
  `ki18n_install(po)` installs it. German is a maintained translation:
  `po/de/denkzettel.po` carries the wording the interface had before the source
  language changed, and `po/Messages.sh` refreshes the template out of the
  sources after every change to a string). UI running texts (placeholders,
  hints, dialogs) address the user in the **imperative** — the form of the KDE
  HIG: "Select a note on the left to read it.", "Press Meta+N to capture a
  thought.", "Change the search term or clear the field." The German catalogue
  carries the counterpart of that rule, the impersonal infinitive form („Zum
  Lesen links eine Notiz auswählen." — design decision 2026-07-31, design
  assignment S8). That one form of address applies to the whole application
  instead of being chosen per window; the imperative came with the switch of
  the source language.
- **Minimum versions:** the general lower bound for ECM and the KF6 components
  lies at **6.0.0**. One exception carries its version itself:
  **KColorScheme is looked up with a `find_package` call of its own and the
  minimum version 6.20**, because `KColorScheme::frameContrast()` was only
  added there. The reason for the separate call is measured (issue #101, user
  decision 2026-08-07): the one number `KF_MIN_VERSION` feeds **both**
  `find_package` calls of the root `CMakeLists.txt`, so a 6.20 at this place
  would raise the lower bound for ECM and all ten components along with it —
  for a distribution with an older KF6, without need. The lower bound rises
  where the function sits.
- Runtime dependencies: `ffmpeg` (audio conversion), optionally
  `whisper.cpp` (Vulkan) and `task` (Taskwarrior) — both are detected at
  runtime; if one is missing, only the affected functions are deactivated
  (with a hint in the settings), the application stays usable.
- Packaging: a PKGBUILD under `packaging/` (issue #41), and beside it a local
  `cmake --install` for development. Both configure with
  `-DCMAKE_INSTALL_PREFIX=/usr`, because only then does the XDG autostart entry
  land in `/etc/xdg/autostart`; under the CMake default `/usr/local` no Plasma
  session reads it (sprint 2 finding, issue #6). `pkgver` moves with the
  version in `CMakeLists.txt` and with the tag — `source` fetches `v$pkgver`,
  so a forgotten bump builds the previous release without a word. The AUR
  submission is still outstanding.

### 15.1 Version rules and command line (issue #61)

- **A single source:** the number stands in `project(denkzettel VERSION …)` of
  the root `CMakeLists.txt` and nowhere else. From there `src/CMakeLists.txt`
  hands it on to the code as the compile definition `DENKZETTEL_VERSION`, which
  enters it into `KAboutData`. A second copy in a source file would be the one
  that goes out of date unnoticed.
- **Scheme 0.x SemVer**, as long as the application stands before 1.0:
  - **MINOR** (`0.1.0` → `0.2.0`) with **every user acceptance**.
  - **PATCH** (`0.2.0` → `0.2.1`) for unscheduled fixes between two
    acceptances.
  - **Every schema migration of the database forces at least MINOR** — even
    when nothing else is added. A corpus that no longer fits the previous
    version is no small matter.
- **The tag is the seal:** `vMAJOR.MINOR.PATCH` on the accepted state. The
  increase and the tag stand in the closing procedure (`CLAUDE.md`) and follow
  the acceptance; they do not precede it.
- **The number becomes visible through `denkzetteld --version`** — output
  `denkzettel <number>`, return 0, with a running service as well and without a
  session bus (condition in 2.3) — and in the running application through the
  tray entry "About Denkzettel" (#87, section 10). `KAboutData` carries beside
  the number the short description, the licence (MIT) and the copyright holder
  — both taken word for word from `LICENSE` in the project root; the dialog
  reads all four out of it and out of no second place. Authors stay empty:
  without them the dialog leaves out a tab, and there is nobody to list beside
  the holder.
- **Unknown switches are rejected** (return ≠ 0). The start without arguments
  stays the start of the service — both `Exec=` lines of the desktop file call
  without an argument, the one of the second desktop action included. That
  sentence was written when there was one action; it stays true with two,
  because what tells the two apart is not the command line but the action name
  of the D-Bus activation (2.4, issue #125). Denkzettel has no switch that
  chooses a window, and a start line anybody can type is not one of its
  interfaces.

## 16. Test strategy

- **Unit (QTest)**: search operator parser, clustering (with synthetic
  vectors), prompt/JSON schema processing (provider mocked), export formatter
  (collective note Markdown), building the Taskwarrior command line, file name
  and path logic.
- **Integration**: the store layer against a real SQLite (temp file), FTS
  triggers, deletion transaction including the audio file.
- **Manual (checklist per milestone)**: the shortcut route under Wayland, focus
  behaviour, recording with a real microphone, a Whisper run on the 7900 XTX,
  export into a **test vault** (never the real one), Taskwarrior against a
  **`TASKDATA` test directory of its own** (never the production corpus).
- **Migration test**: as soon as the first real schema migration exists, a test
  checks the upgrade of an existing DB from version n to n+1.
- **Condition for icon and dialog guarantees** (discovered at #60, confirmed at
  #66/#67): tests that guarantee icon names or the appearance of a message
  dialog run with `QT_QPA_PLATFORM=offscreen` **and
  `QT_QPA_PLATFORMTHEME=kde`. Without the platform theme**
  `QIcon::fromTheme()` resolves nothing and delivers an icon **without a name**
  — the guarantee is then red without anything missing in the build —, and the
  platform integration does not build the message dialog the way a session user
  sees it (section 9). This holds for `shelltest` and `librarytest`. **It does
  not replace a Plasma session:** the image evidence on the installed state
  remains.
- **What cannot be demonstrated offscreen by construction** (measured at #55):
  the **shadow** of the capture window. Without a compositor there is nobody
  for `KWindowShadow::create()` to hand the tiles to — the return value is
  **always** false there, and that is no fault of the code. `QWidget::grab()`
  never shows it either, because it lies outside the widget. An image produced
  offscreen without a shadow is therefore **no test finding**. The evidence is
  the image from the Plasma session; there has been no substitute in the test
  since the test cut, and the guarantee that the shadow lies there again after
  **every** re-showing (3.2, item 5) is carried by the visual check alone
  anyway.
- **And the blur, sharper than the shadow** (measured at #83): a **window
  capture** does not show it either. `spectacle -a` delivers, like
  `QWidget::grab()`, the window's own surface; what lies behind the hull stands
  in neither of the two. It can only be demonstrated on a capture of the
  **composited image** over a known pattern, in an A/B comparison against a run
  without the registration. `enableBlurBehind()` and
  `enableBackgroundContrast()` are `void`, and the one available return value —
  `isEffectAvailable()` — gives a false answer before the first registration (3.2, item 9).
  Whether the contrast effect **takes effect** at all on the state checked is
  unmeasured: `org_kde_kwin_contrast_manager` no longer stands in the
  compositor's list of globals. The call is demonstrated, the effect is not.
- **What likewise does not occur offscreen: the jump of the pixel ratio**
  (3.2, item 7). Offscreen it stays at a `Resize`, and the only known fault
  mechanism is thereby systematically invisible. Evidence about it comes from
  the logged-in session and **without** `QT_SCALE_FACTOR` — under Wayland the
  variable multiplies with the session scaling (1 → 1.6; 1.6 → 2.56),
  offscreen it does not.
- **An image comparison of whole windows across platform boundaries is no
  means of checking** (measured at #83): the **hull** is byte-identical
  offscreen and under Wayland at the same pixel ratio, a **grabbed window** is
  not — the type rasterisation differs (1,587 of 154,440 pixels, all of them in
  the text area; cause Fontconfig, not KSvg).
- **A reconstruction of the scaling in the same process is no test setup**
  (discovered at #101): drawing the list through a painter onto an image with
  `devicePixelRatio` 1.6 measures something other than a session under
  `QT_SCALE_FACTOR=1.6`. The reconstruction showed the fault at **1.25** and
  **not at 1.6** — where the real scaling shows it. The row heights of a scaled
  session are not those of an unscaled one; the reconstruction measured row
  positions that do not exist at all. **A test assertion that holds precisely
  where the fault sits is worse than none.** Whoever wants to check scaling
  registers the same test function a second time, with `QT_SCALE_FACTOR` in the
  environment — and only the functions that say something under scaling. Pixel
  guarantees in logical points do not belong in it; they would otherwise check
  the scaling instead of the matter. **The scene has a say as well:** the fault
  hangs on the position in the raster, and the first run with real scaling was
  green because the scene knew too few positions.
- **States that cannot be produced in the test process itself need a process of
  their own** (discovered at #55, AC 8): the window without a desktop theme
  cannot be produced through an invented theme name — KSvg falls back to
  `default` in the process, and the test checked a case that cannot occur at
  all. For this guarantee `capturetest` restarts itself with a trimmed
  `XDG_DATA_DIRS`.
- **No guarantee hangs on a name only this machine knows** (discovered at #55).
  The themes of the official KDE stock all carry a 4 px border; every wider one
  on the development machine comes out of a CachyOS package, and a build place
  that installs only the KF6 parts of this project has **no** desktop theme at
  all — `ksvg` does not hang on `libplasma`. Whoever hangs a guarantee on a
  particular theme checks something else or nothing elsewhere. Whoever hangs it
  on test themes shipped along demonstrates only that the code reads *our* SVG.
- AI quality (classification/clustering) is not tested automatically — the
  suggestion review is the human control instance.

## 17. Milestones

1. **M1 capture core**: daemon, tray, KGlobalAccel, text capture, SQLite store.
   *Usable: holding on to thoughts.*
2. **M2 library + search**: window, list, detail (read/edit/delete),
   FTS + operators.
3. **M3 AI basis**: provider abstraction (Ollama), classification + tags,
   categories sidebar, settings pages AI/analysis.
4. **M4 voice notes**: recording window, queue, whisper.cpp backend,
   player in the library, settings page for voice notes.
5. **M5 suggestions**: embeddings + clustering, bundle and task suggestions,
   review UI, Obsidian and Taskwarrior execution, overflow guard,
   full export.
6. **M6 provider expansion**: openrouter, OpenAI by API key (per 7.5), KWallet.
7. **M7 polish**: icon states, notification fine-tuning, logging, PKGBUILD.

Every milestone ends with the manual checklist (16) and a runnable state.
