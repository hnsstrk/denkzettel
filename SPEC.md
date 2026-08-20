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
exports `org.denkzettel.Daemon` as an external interface (see 2.3) — usable
from the CLI, from scripts and to enforce a single instance.

### 2.2 Modules

| Module | Task |
|---|---|
| `capture` | Text capture window, recording window (voice note) |
| `store` | SQLite access, data model, FTS index, audio file management |
| `analysis` | AI pipeline: classification, tags, embeddings, clustering, task extraction |
| `transcribe` | Whisper backends (whisper.cpp, WhisperX) as subprocesses, job queue |
| `proposals` | Suggestion generation and execution (Obsidian export, Taskwarrior) |
| `ui` | Library, suggestion review, settings |
| `shell` | Tray (KStatusNotifierItem), KGlobalAccel registration, KNotifications, D-Bus adaptor |

### 2.3 D-Bus interface `org.denkzettel.Daemon`

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
  `kde.org` —, and the service then registers as `org.kde.Daemon`.
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
  otherwise). The `Exec` line starts `denkzetteld`; the single-instance branch
  from 2.3 turns that into the call that shows the window.
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
- **The component name hangs on the desktop file name (discovered condition,
  finding 2026-08-04, issue #61):** Denkzettel reads
  `QGuiApplication::desktopFileName()` and appends `.desktop` — the component
  name hangs on that, and under Wayland the application id as well.
  `KAboutData::setApplicationData()` overwrites the property with its default
  `org.kde.<component name>`; the shortcuts then ran under a component to which
  no installed desktop file belongs. Denkzettel therefore sets the name on the
  `KAboutData` object before the registration (see 2.3).
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
  is permissible in the sense of the loop conventions.

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
  `WindowText`, **not** the role for entry fields, and **application name and
  footer** `PlaceholderText`. The **placeholder text** of the empty field
  belongs to the muted class as well — three places, not two.
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
   **An *unknown theme name* is something else, and the guarantee about it is
   refuted** (2026-08-12): it said here that KSvg falls back to `default` in
   that case, so the case is harmless. Measured, it does not fall back but
   crashes — checked three times, each with the same `plasmarc`:

   | `[Theme] name` in `plasmarc` | Result |
   |---|---|
   | theme that is not on the data path | **SIGSEGV** in `KSvg::ImageSet` |
   | key missing entirely | fallback to `default`, everything as guaranteed |
   | existing theme (`breeze-dark`) | everything as guaranteed |

   The situation arises without anyone's doing: whoever sets a desktop theme
   and later removes its package has exactly this `plasmarc`. Open as
   issue #107; until it is fixed this item guarantees the *missing* key alone,
   not the name that points into the void.
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
   **Limit, named rather than covered up:** this value is obtained **once when
   the window is created**. Whoever switches the
   blur off **at runtime** while the service is running keeps getting the
   translucent variant until the restart — item 4 holds for the state at the
   start, not for a switch after it. The theme switch is **not** affected by
   this; the watch on `plasmarc` catches that one. Whether the limit is closed
   or written down is open (issue #93).
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
- Upper bound 15 minutes (protection against a forgotten recording); hint in
  the time display from minute 14.

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
      analysis_last_error TEXT NULL)
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
- Audio lies as a file under `audio/` (name = the note's ISO timestamp), the DB
  holds the reference. Deleting a note deletes tags, embedding, FTS entry,
  `proposal_notes` references and the audio file in one transaction plus a
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
| `kat:todos` | Category (alle, todos, ideen, cli, persoenlich, software) |
| `typ:text` / `typ:audio` | Note type |
| `vor:2026-07` / `vor:2026-07-15` | created before a date (month or day) |
| `nach:2026-06` | created after a date |
| `"exact phrase"` | Phrase search (FTS5 phrase) |
| free text | FTS5 full text (ANDed terms) |

- All components are **ANDed**; no OR, no brackets (v1).
- Unknown `xyz:` prefixes are treated as full text (no error).
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
- **openrouter.ai**: OpenAI-compatible API, API key from KWallet — `chat` only.
- **OpenAI**: by platform API key (see 7.5) — `chat` only.

**Embeddings always come from Ollama in v1** (refinement after the finding of
the estimation session 2026-07-31: openrouter offers no embedding endpoint).
The LLM provider is freely selectable, the embedding model runs locally —
uniform, free of charge, and the cluster threshold (7.3) stays bound to one
model. Without a reachable Ollama, Denkzettel degrades visibly:
classification through the chosen provider keeps working, topic bundles are
dropped (hint in the settings and the tray tooltip).

All calls through Qt Network, asynchronous, with a timeout (30 s) and one
retry. "Test connection" in the settings makes one mini `chat` call and (with
Ollama) one `embed` call each and shows the latency or the error.

### 7.2 Analysis run

Trigger according to the setting: **at once** (after saving/transcription),
**periodically** (interval, default 30 min) or **on demand** (tray/D-Bus).
A run processes all notes with `state != 'analysiert'` as well as — for step 2
only — those with `needs_reembed = 1`; at most 50 notes per run (budget,
section 14), the rest follow in the next run:

1. **Classification + tags** (one LLM call per note, JSON schema:
   `{category, tags[], is_todo, task?}`): category from a fixed list (TODOs,
   Ideen, CLI-Befehle, Persönlich, Software-Ideen), 1–4 tags in lower case.
   For `is_todo=true` the same call extracts the task fields
   (`description, project, tags, due, priority` — `due`/`priority` only on a
   clear signal, otherwise null).
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
  before the LLM: it names the topic, may remove obvious outliers (sanity
  check) and generates the collective note (see 8.1). Result: one `bundle`
  suggestion.
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
  An entry shows the timestamp, the first line as the subject, the following
  text as a preview and tag chips; voice notes additionally ▶ and the duration.
  The timestamp follows the group: in Today/Yesterday the time of day, in the
  week groups the weekday and date, in Older the absolute date; in the detail
  pane the full form. The structure is fixed — no toggle, no collapsible groups
  (wireframes 3a/3b).
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
  without it there stands nothing but a time of day without a day under "Today"
  and "Yesterday" — the timestamp rule above presupposes the head. The price is
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
- Detail view: **read and edit view** (decision of the third interview — above
  all for faulty transcripts). Editing keeps category/tags and `state`, but
  sets `needs_reembed = 1` — the next analysis run renews the embedding only
  (7.2), since it ages with the text. Delete action with a 5-second undo
  (addition of the spec, not in the concept: purely client-side delayed
  deletion, no soft-delete state in the DB).
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
  | Quit | `application-exit` | active | — |

  **"Quit" stands set apart in the last group** and never beside the most
  frequent entry: it ends the service and with it the shortcut. Until the
  settings dialog (#16) exists, "Configure Denkzettel…" is **no entry at all**
  — a permanently greyed-out one does not explain to the user why it is grey
  (KDE HIG).
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

## 11. Overflow guard

- Two criteria, both configurable: **count** of unexported notes (default 200)
  OR **age** of the oldest unexported note (default 30 days).
- On crossing: tray reminder + notification; the next analysis run generates
  bundles preferentially (including "Miscellaneous", 7.3). **Never** an
  automatic export.

## 12. Transcription

- Job queue (`transcribe_jobs`), worked off serially (one GPU); survives
  restarts (queue in the DB).
- **whisper.cpp** (default): Vulkan build from the AUR (`whisper.cpp-vulkan`
  or similar; check the package name at implementation time). Called as a
  subprocess: audio through `ffmpeg` to 16 kHz mono WAV (temporary), then
  `whisper-cli -m <model> -f <wav> -l de -oj` → JSON transcript.
  Model size configurable (default `small`, choice tiny–large-v3; download of
  the GGML models at first use with progress, kept under
  `~/.local/share/denkzettel/models/`).
- **WhisperX (ROCm/GPU)**: configurable call path, subprocess with
  `--language de`, JSON evaluation, without diarisation. **Precondition**
  (finding of the estimation session 2026-07-31): WhisperX is not yet installed
  on the development machine — the PoC of the RPG audio project ran with
  whisper.cpp. The hookup is built/accepted only once the installation exists
  (it comes into being in the RPG audio project); until then whisper.cpp is the
  only active route.
- Error path: 2 failed attempts → the job pauses, tray error state, the note
  stays visible/playable as an `audio` note without a transcript (nothing is
  lost).

## 13. Settings (dialog)

Page list according to the concept: **AI provider** (provider choice, LLM and
embedding model, test connection), **Analysis** (at once/periodically with an
interval/on demand), **Export** (vault path with a folder chooser, overflow
thresholds count + days, bundle threshold), **Voice notes** (backend
whisper.cpp/WhisperX, model size, WhisperX path), **Shortcuts**
(KKeySequenceWidget for both shortcuts).

## 14. Error handling and loop discipline

The periodic analysis run is a loop in the sense of the loop conventions:

- **goal met**: all notes `analysiert`, suggestions generated → the run ends.
- **budget**: one run processes at most 50 notes (the rest in the next run).
- **stalled**: the same error 2× → skip the affected note, report it.
- **needs a human**: all transfers require confirmation anyway.

Reporting channels: tray state + tooltip (quiet), KNotification (important), log
file `~/.local/share/denkzettel/denkzettel.log` (details, with rotation).

## 15. Build, dependencies, packaging

- **CMake** + ECM (Extra CMake Modules), C++20.
- Qt 6: Widgets, Sql, Network, Multimedia, **DBus** (not only for the single
  instance and the service interface: the capture window asks KWin over it
  whether this session blurs at all — 3.2, item 9).
  KF6: KGlobalAccel,
  KConfig, KNotifications, KStatusNotifierItem, KWallet (framework: KWallet),
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
- Packaging: for now a local `cmake --install` — with
  `-DCMAKE_INSTALL_PREFIX=/usr`, because only then does the XDG autostart entry
  land in `/etc/xdg/autostart`; under the CMake default `/usr/local` no Plasma
  session reads it (sprint 2 finding, issue #6). PKGBUILD/AUR after
  stabilisation.

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
  session bus (condition in 2.3). An about dialog is a story of its own (#87).
- **Unknown switches are rejected** (return ≠ 0). The start without arguments
  stays the start of the service — both `Exec=` lines of the desktop file call
  without an argument.

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
   player in the library; WhisperX hookup.
5. **M5 suggestions**: embeddings + clustering, bundle and task suggestions,
   review UI, Obsidian and Taskwarrior execution, overflow guard,
   full export.
6. **M6 provider expansion**: openrouter, OpenAI by API key (per 7.5), KWallet.
7. **M7 polish**: icon states, notification fine-tuning, logging, PKGBUILD.

Every milestone ends with the manual checklist (16) and a runnable state.
