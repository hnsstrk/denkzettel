# Denkzettel — working instructions for Claude Code

Quick-capture tool for KDE Plasma (Wayland), C++/Qt6/KF6, CMake, QTest.

`SPEC.md` is binding. The backlog is the GitHub issues with their acceptance
criteria — they say when a story is done.

**An issue also says whether it may be started, and it says it in its comments
as often as in its body.** Read every comment, oldest to newest, before the
first line of code: the newest one beats the body, and it may have retired an
approach that was built and measured and failed. Then look for the sentences
that mean stop — "Not ready", "before it is pulled", "a second estimate is
due", a decision the text reserves for the customer, a visible change with no
drawing. Measured on 28.08.2026: #87 was built although its own text said "Not
ready, and without the user's decision not even checkable in advance", and #19
was pulled with its second estimate outstanding. Both were readable in ten
seconds and neither was read.

## How code gets written

The ponytail plugin is enabled here (`.claude/settings.json`) and loads its
"lazy senior dev" ruleset at every session start: build it only if it has to
exist, standard library and native platform before custom code, no abstraction
nobody asked for, the shortest diff you actually understand. That ruleset is
loaded, not repeated here.

**Where it collides with this file, this file wins.** Three places where it
does:

1. **Tests.** ponytail wants one runnable check behind every non-trivial piece
   of logic. Here the first question is the one below — *would the user notice
   this fault while using the program?* — and only what breaks silently gets a
   test. Do not write back the 88 test cases cut on purpose in `45df0dc` and
   `597ecc9`. Where a check is warranted it is a function in an existing QTest
   set, not a script beside it: the CI allows no skipped test set.
2. **Evidence.** "Shortest working diff" is about the solution, never about the
   proof. The installed state, an image of your own, a freshly built runner —
   ponytail exempts understanding the problem and the calibration real hardware
   needs, and that is what those rules are.
3. **Reporting.** ponytail caps explanations at three short lines; the user asks
   for a closing report of what changed and what was found and left alone. An
   explicit request outranks the cap, and ponytail says so itself.

Deliberate simplifications with a known ceiling carry a `ponytail:` comment
naming the ceiling and the upgrade path — in English, like every comment here.

## Build, verify, install

```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug && cmake --build build
ctest --test-dir build
cmake --build build --target lint-tidy      # or lint-clazy
```

Installing needs the user's password through a graphical dialog:
`pkexec /usr/bin/cmake --install <project-path>/build`

## What gets verified — and what does not

**A test only stays where the eye cannot reach** (the user's decision,
2026-08-11). Denkzettel is a small tool, not rocket guidance, and the user
looks at the result themselves — colors, spacing, lines, font sizes and whether
a window opens are things they judge better and faster than a pixel comparison.

A test set is therefore only justified for what **breaks silently**: schema
migrations and data loss, the search index, error paths, character encoding,
return values of third-party services, differences between build types.
Everything that can be looked at gets looked at.

Before every new test set, the question: *would the user notice this fault
while using the program?* If yes, the image is the evidence and not a test.

## The four rules that have found faults

Everything else is judgement. Not these four — each of them has uncovered at
least one fault in this project that the user would otherwise have found.

**1. Verification happens on the installed state, and installing does not mean
running.** After `cmake --install` a running service keeps the deleted old file
alive; conversely `KDBusService::Unique` hands the start of a debug build on to
the running service. Both times you verify the wrong state without noticing.
So: stop the service, start it again, then
`readlink /proc/$(pgrep -x denkzetteld)/exe` — without `(deleted)`. Whoever
wants to verify the debug state stops the installed service first.
`readlink` says *which file*, only a checksum says *which state*.

**2. A UI review without an image of your own has not been conducted.** Tests
do not replace the image check, and images do not replace the tests:

> Bei Bewegungen ist der Weg der Prüfgegenstand, nicht das Ziel.
> Bei Zuständen ist das Bild der Prüfgegenstand, nicht die Zusicherung.
>
> (With movements, the path is the object of the check, not the destination.
> With states, the image is the object of the check, not the assertion.)

For image runs `QT_QPA_PLATFORMTHEME=kde` has to be set, otherwise a substitute
font distorts the proportions. An image that serves as evidence runs with
`QT_SCALE_FACTOR` at the user's scaling.

**3. An image produced offscreen does not show what the user sees.** It proves
geometry, typesetting and color roles — not hull, rounding, outline, shadow or
decoration. Those are drawn by the theme and the compositor, and offscreen both
of them lack their basis. Where an acceptance criterion claims something about
theme or compositor, the image belongs in a session of its own — a nested
`kwin_wayland` with its own `HOME`, its own color scheme and invented notes.

**Never a capture of the session the user is working in** (their instruction of
2026-08-24). Their notes are personal data, this repository is public, and a
picture that has been taken cannot be untaken: it stands in the transcript
before anybody decides whether it belongs in a file. The isolated run is not the
careful variant, it is the only one.

**4. An image is only evidence once its runner has been freshly built.** An
outdated runner writes plausible images of an *old* state with a fresh
timestamp. Before every image used as evidence:
`cmake --build build --target readmeshots`. And the bare call is not enough:
take the invocation from the README section "Screenshots", which points
`XDG_CONFIG_DIRS` at a throwaway `plasmarc`. Without it the runner finds no
color scheme, draws a light theme shell under the dark palette it sets itself,
and the picture looks like a fault of the product — that is finding 8 below.
Measured on 2026-08-24: with that invocation all four committed pictures come
out byte-identical, without it none of the two capture-window ones does.

## Verification stance

- **Before every step whose result goes into a report, ask what it would output
  if its subject were missing.** If the answer is the same output, the step
  carries nothing. A test setup in which the fault cannot even occur is not a
  test.
- **No process fetches the focus back for itself under Wayland.** Whoever
  builds a check with a window switch closes the window lying on top — then the
  compositor gives the focus back by itself.
- **Where the evidence is kept** (the user's decision, 2026-08-28). Older
  acceptance criteria ask for it "under version control in
  `docs/scrum/reviews/`". That directory is gone with the scrum apparatus and
  is not coming back. The wording of a mutation probe, a measurement or a
  verification run goes into the project page in the user's Obsidian vault, and
  its result into the commit message. What stays in the repository is what the
  next reader of the code needs: images that carry a finding, under
  `docs/images/reviews/`.

## Runs that prove nothing

Measured cases in which something looked like evidence and was none. Check
against them before you report a proof, and **extend the list** with every new
find.

1. **`KGlobalAccel::setGlobalShortcut()` returns `true` even when the daemon is
   not reachable.** Only reading the value back from the service proves
   anything.
2. **`KWindowShadow::create()` reports `true` even for the same image eight
   times over instead of eight tiles.** Offscreen it is **always** `false` —
   there neither `true` nor `false` proves anything.
3. **`activateWindow()` does not fetch the focus back under Wayland.** No
   process can assign itself the focus. An Alt+Tab cannot be triggered; the way
   over the window lying on top stands above.
4. **Two live `KSvg::ImageSet` of the same theme share their lookup paths.**
   Otherwise every comparison of two versions of the same graphic runs against
   itself — and is green. For the second version take a **different** theme.
   `ImageSet(name, …)` looks its private up in a table keyed by that name and
   reference-counts it, so **the selectors travel with it too**: a set built
   later inherits what an earlier one selected. Measured 2026-08-28 on #93 —
   `setSelectors({"opaque"})` set only in the no-blur case stayed behind when
   the blur came back, and the window kept the opaque graphic. Whatever a set
   selects gets selected on **every** branch, the empty one included.
5. **A test set that does not choose its theme may well be testing on one that
   does not know the difference.** Choose the object of the check so that the
   choice changes anything at all.
6. **With the session locked, `spectacle -f` delivers a black image with return
   code 0.** Query the lock state beforehand and abort instead of measuring.
7. **A fullscreen window as a backdrop for the check covers what you want to
   measure** — the compositor puts it over the capture window.
8. **A sandbox without `kdeglobals` colors the theme graphic differently from
   the Qt palette** — the image then looks like a fault of the product.
9. **`show()` instead of `showCapture()` delivers a window without a shadow** —
   it is only bound in `present()`.
10. **A comparison can be wrong on both sides and report "correct".** If you
    compare two quantities that the same fault shifts together, you measure
    nothing. Hold at least one side against a value **set from outside**.
11. **A red result in a build directory that has been reconfigured proves
    nothing.** Measured 2026-08-24: `librarytest` died with SIGSEGV in
    `NoteListModel::insertNote` in six runs out of six — on the changed and on
    the unchanged state alike, which looked like proof that the fault was older
    than the change. The same commit built clean in a fresh directory passed on
    the first try. Before a red result becomes a finding: `rm -rf build`, build
    again, and only then believe it.
12. **One window is not enough for a fault that lives in a shared table.**
    Measured 2026-08-24 on #107: with an unresolvable desktop theme in
    `plasmarc` the **first** capture window comes up without a murmur; only the
    second one dies, because what the first leaves behind is a freed pointer in
    KSvg's table of image sets. A check that builds one window stands green over
    the unfixed bug. Whatever a library keys by name and hands out again gets
    asked for **twice** in the check.
13. **`spectacle -m` in a nested `--virtual` session delivers an empty white
    picture with return code 0.** Measured 2026-08-24, beside the black one the
    README already names for `-f`. `-a` is the only one that carries anything
    there — and it refuses with "No active window" if the window under test has
    not taken the activation, which a frameless one does not always do.
14. **A hidden window cannot show a fault that only the layout produces.**
    Measured 2026-08-24 on #79: emptied from eight lines the window goes back
    to five while it is hidden and stays at eight while it is shown, because
    only an activated layout writes a minimum onto the window that `resize()`
    is then clamped by. The deleted `windowFollowsTheTextHeight()` asserted
    exactly the right thing and was green for years, because it never showed
    the window. Whatever depends on the layout gets checked after
    `showCapture()` and `qWaitForWindowExposed()`.

15. **`QDBusConnection::connect()` returns `true` for a signal that does not
    exist.** Measured 2026-08-24 on #93: the window subscribed to
    `effectLoaded` and `effectUnloaded` on `org.kde.kwin.Effects`, the call
    reported success, and nothing ever arrived — the interface of KWin 6.7 has
    **no signals at all**, only methods. The subscription looked built and was
    a hole. Whatever a bus hands out, introspect the interface first
    (`busctl introspect`), and read the state back afterwards.

16. **A check that reads an installed path proves nothing about the source
    tree.** Measured 2026-08-28 on #73: the ECM `appstreamtest` validates the
    `/usr` path out of `install_manifest.txt`, which a run without root never
    writes — so with no metainfo at all it printed `Could not find …` and
    reported `Passed`, and with a deliberately broken file it passed just the
    same. Once somebody does install to `/usr`, it validates the **installed**
    copy, which can lag behind the source. A check of the installed state
    belongs on a staging directory the run creates itself (`DESTDIR`), or it
    measures the wrong file or none.

17. **`GGML_BACKEND_PATH` does not switch a backend off — it adds one.**
    Measured 2026-08-28 on #19: the plan was to prove the Vulkan backend by
    running whisper-cli a second time with that variable pointing at an empty
    directory. `ggml-backend-reg.cpp` reads it only **after**
    `ggml_backend_load_best("vulkan", …)` has already loaded the backend from
    the default path, so the control run loaded Vulkan just the same and took
    the identical 371 ms. Read as the comparison it was meant to be, "no
    difference" would have said the backend does nothing. The control that
    works is `whisper-cli -ng` — 2633 ms on the CPU. Before a run is used to
    switch something off, read in the source that the lever is connected.

18. **`emits-change` in an introspection is a promise, not a signal.**
    Measured 2026-08-28 on #93: `busctl introspect org.kde.KWin /Effects` marks
    the properties `activeEffects`, `listOfEffects` and `loadedEffects` as
    `emits-change`, which reads like the way out of finding 15. In a nested
    session `blur` was unloaded — `isEffectLoaded` went from `true` to `false`,
    `loadedEffects` no longer held it — and `dbus-monitor` recorded **not one**
    `PropertiesChanged` in that window, only the bus daemon's own name signals.
    The annotation is written by the adaptor, the emission is written by the
    service, and here the second half is missing. What the introspection
    promises is checked with `dbus-monitor` before anything is hung on it.

19. **A KSvg graphic changes 100 ms after the call, not within it.**
    Measured 2026-08-28 on #93: `ImageSet::setSelectors()` writes the
    selectors and then only calls `scheduleImageSetChangeNotification()` —
    a 100 ms timer, after which `discoveries.clear()` runs and
    `imageSetChanged` is emitted, and only that makes every attached `Svg`
    re-resolve its file. Whoever calls `reloadDesktopTheme()` and grabs the
    window in the same turn measures the **old** graphic and reads it as proof
    that the switch does not work; a check without a running event loop never
    sees the new one at all. Let the loop run before the picture.

20. **`org.kde.KGlobalAccel.shortcut()` answers an empty list for a wrong
    action id, exactly as it does for a component that holds nothing.**
    Measured 2026-08-28 on #109: the action id is a list of **four** strings
    (component id, action id, component display name, action display name);
    called with two, the service returns `ai 0` and nothing says why. The
    neighbouring trap in the same interface: `setShortcut` with the flag
    `IsDefault` (8) returns the keys handed to it while the **active** shortcut
    stays empty — the return value looks like a registration and is none. Only
    `SetPresent` (2) sets what a key press finds. Prove a readback by making it
    come out **different** at least once.

21. **A nested session with its own `HOME` looks isolated and is not — the
    installed program lies beside it and the run finds it there.** A throwaway
    `HOME` separates *configuration*: `XDG_CONFIG_HOME`, `XDG_DATA_HOME`,
    `XDG_CACHE_HOME`. It separates nothing that the system directories carry,
    and `XDG_DATA_DIRS` keeps pointing at `/usr/share`, `XDG_CONFIG_DIRS` at
    `/etc/xdg` — where this project's own installation lies. So every run that
    measures **what an installation does** is measuring the installed copy as
    much as the one it staged: desktop and autostart entries, D-Bus service
    files, KService and MIME data, icon themes, message catalogues, KPackage
    and KCM plugins.

    Measured 2026-08-28 on #109: the run was to show what the shortcut service
    does once the desktop entry of the old name is gone, so that entry was left
    out of the staging root — while `XDG_DATA_DIRS` still ended in
    `/usr/share`, where the installed copy of that very file lies. The service
    resolved it from there, and the run "proved" that a stale component
    survives without its desktop entry. It does not: with the entry truly
    unreachable, `getGlobalShortcutsByKey` returns **no** holder for the key,
    although the group still stands in `kglobalshortcutsrc`. The opposite
    conclusion, from a run that could not have come out any other way.

    Before such a run, name the paths the object of the check can arrive
    through, and take it away from **every** one of them. Never by deleting it
    from the machine the user works on — mirror the system directory instead:
    symlink every entry of `/usr/share` into a directory of the run's own,
    rebuild the one subdirectory that holds the file by hand, and leave that
    file out. Dropping `/usr/share` from the list altogether is no shortcut,
    the session dies without its GSettings schemas. And the control that makes
    the run evidence is the same one as everywhere: it has to come out
    **different** once — with the file reachable and without.

22. **`KDirWatch` swallows an overwrite in place inside the same second — a
    replacement it always reports.** For a watched file it compares
    `qMax(st_ctime, st_mtime)` in whole seconds, and beside it `st_ino` and
    `st_nlink` (`kdirwatch.cpp:1279–1281`, `kdirwatch_p.h:111`). So a new inode
    is a change whatever the clock says, and only a rewrite of the same inode
    within one second goes unreported. Measured 2026-08-28 on #110: `kdeglobals`
    rewritten in place twice inside one second — inotify logged `MODIFY` four
    times over (`QT_LOGGING_RULES=kf.coreaddons.kdirwatch.debug=true`), `dirty`
    was never emitted, and the check stood red with the fix in place and red
    without it, so its red said nothing about the code. The same check writing
    through `QSaveFile` runs in 80 ms and needs no waiting at all. **This is no
    lost save:** KConfig replaces the file, so what Plasma writes always
    arrives. Whatever a check rewrites for a file watch to notice gets replaced,
    not overwritten — that is the road the real writer takes anyway.

23. **`kwin_wayland` hangs without a word when the program behind its `--` is
    not executable.** Measured 2026-08-28 on #50: the nested session came up
    complete — the socket was there and `wayland-info` against it listed all 65
    globals — but the measurement never started, and every step after it timed
    out. Nothing in the output says so; three runs in a row looked like two
    nested compositors getting in each other's way. Before every nested run:
    `chmod +x` on the session program, and if nothing comes back, ask the
    compositor over its socket whether it is up rather than guessing.

24. **`QAudioBufferInput` constructed with a format refuses every buffer, and
    reports it nowhere.** Measured 2026-08-28 on #20 with Qt 6.11.2:
    `sendAudioBuffer()` returned `false` for all ten buffers of the run,
    `readyToSendAudioBuffer()` never fired once, `QMediaRecorder` stayed in
    `RecordingState` without an error, and what stood on disk afterwards was a
    valid 127-byte OGG header with return code 0. The default constructor takes
    the format from the first buffer and works — and `QAudioBufferInput` has no
    `setFormat()`, so construction is the only place the trap can be laid.
    **Reproducing it needs a format that is already finished:** written as a
    member initialiser, `QAudioBufferInput m_input{m_format}` does *not* trip
    it, because `m_format` is still empty at that point and the setters only
    run in the constructor body — a mutation probe built that way passes and
    proves nothing. So: a recording is proven by the duration and the codec of
    the file, never by the file existing.

25. **Qt's own warnings do not reach a pipe — outside QTest.** Measured
    2026-08-28: for an ordinary Qt program with stderr not a terminal (every
    `2>&1 |`, every redirect into a log) the default message handler writes to
    the journal instead — 0 hits through a pipe, 0 into a file, 4 in the
    journal, and 1 back on stderr with `QT_FORCE_STDERR_LOGGING=1`. A run
    grepped for a warning then looks clean because it is mute, not because it
    is quiet. **QTest binaries are not affected**: QTest installs a message
    handler of its own, and the same probes came through identically with and
    without the variable (3 to 3). The rule is for `denkzetteld` and helper
    programs, not for `ctest` runs.

26. **`QMediaRecorder` invents a file name when the output is a directory.**
    Measured 2026-08-28 on #20: to force an encoder error the check put a
    directory where the file should go — `audio/<timestamp>.ogg/`. The muxer
    neither refused nor complained; it wrote `record_0001.ogv` **inside** that
    directory and reported a running recording. Same make as finding 24: a file
    comes into being, the return value is right, and what was checked is not
    what was meant. `outputLocation()` is what was ordered, `actualLocation()`
    is what was written — `AudioRecorder::removeFile()` therefore deletes the
    second, or a cancelled recording would stay on disk under a name nobody
    looked for.

29. **A check that clears up before it looks measures its own tidying.**
    Measured 2026-08-28 on #22: three cases asserted that no temporary working
    directory survives a transcription job, and the cleanup was deliberately
    taken out to see them go red. Two did; the third stayed green, because it
    called `Transcriber::start()` one line earlier — and that sweeps abandoned
    directories away before it takes up the queue. The same trap sits in
    `QTemporaryDir`: it deletes what it holds in its **destructor**, so a
    leftover check made after the owning object has gone is green whatever the
    code does. Both were only visible because the check was run against the
    unfixed state. Assert what has to be gone **before** anything that could
    remove it — and while its owner is still alive.

30. **`pgrep -f <path>` finds the shell that is running the check.** Measured
    2026-08-29 on #22: a run was to show that a child process dies with its
    daemon, and it looked up the daemon with `pgrep -f` on its path and took
    the first hit. That hit was the harness's own shell, whose command line
    holds that path because the command does — so the signal went to the shell,
    the real daemon lived on, and the run reported "the child survived its
    daemon", the exact opposite of what the code does. With the parent taken
    from the child instead (`ps -o ppid=`), the same run came out the other way
    round on the same binary. A process is identified by what the kernel says
    it is — `pgrep -x`, `readlink /proc/<pid>/exe`, or the parentage — never by
    a string that the measuring command itself contains.

**The common denominator** is every time the first rule of the verification
stance: the step would have delivered the same output if its subject had been
missing.

## Before handover

Start the built state and walk the story's main path once yourself. For images
`QT_QPA_PLATFORM=offscreen`, `QT_QPA_PLATFORMTHEME=kde` and `QT_SCALE_FACTOR`
at the user's scaling (**1.5**).

After starting the daemon, look into the journal
(`journalctl --user -t denkzetteld -n 20`) — silent faults of third-party
services stand there and nowhere else.

## UI review

The yardstick is `wireframes/Denkzettel Wireframes.dc.html` as the project's UI
reference and the KDE Human Interface Guidelines (develop.kde.org/hig) —
Denkzettel is a Qt6/KF6 app for KDE Plasma. The check points come from the
wireframe, not from memory: every drawn area produces exactly one check
question, the division of space included. Images that carry a finding go
under `docs/images/reviews/` — the directory is created with the first one.

## When nothing moves forward

The same fault twice without new insight: stop and report. A contradiction
between issue and SPEC, or a decision only the user can make: ask instead of
guessing.

## Closing out

Changelog line, version in `CMakeLists.txt`, tag `vX.Y.Z`, close issues and
milestone.

Whoever draws a lesson writes it in here. The next session does not read a log.

**The user schedules the installation to `/usr`** — it needs their password.

## Publication

The repository is **public**. What stands in issues and commits is published.
Permitted are quotes from the user and measurements; not permitted are system
details (host names, kernel versions, paths outside the project, internals of
the home network) and personal data.

Pushing happens after every completed block of work, without asking. Every push
to `main` triggers a public build and test run (`.github/workflows/ci.yml`); it
fails on every compiler warning, every red test and every linter finding.
Whoever pushes checks afterwards — **on the run of their own commit**, not on
the topmost one in the list:

```
gh run list --commit $(git rev-parse HEAD) --json status,conclusion \
    --jq '.[]|[.status,.conclusion]|@tsv'
```

Only `completed` **and** `success` counts as having looked. Conversely, the green
badge is no substitute for verification on the installed state — the run does
not reach it.

Code, comments and UI strings in English; every visible string goes through
`i18n()`, German is produced in `po/de/denkzettel.po`. Commit subject lines in English.
