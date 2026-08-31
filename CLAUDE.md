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
Measured again on 2026-08-29, now over **six** pictures in two languages: with
that invocation all six come out byte-identical, without it the **three that
carry the hull** do not — the capture and the recording window in English and
the capture window in German, while the library, which draws no theme graphic,
is unaffected. And the picture is only evidence once the readback says the file
compared is this run's own: a random marker written into the target directory
beforehand, and a timestamp inside the run's own window (finding 55).

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

    **And the quiet half of the same trap**, measured 2026-08-29 on #47: there
    the session program did run, and the daemon *inside* it did not — the path
    handed in was empty, because `readlink -f` on a path built out of `..`
    segments hands back nothing. Nothing hung then; the run finished and
    reported `isScriptLoaded b false` and zero calls on the bus, which is
    **exactly** the right answer for the case it was measuring, the setting
    switched off. A hang is read as a fault, a plausible answer is not. So a
    nested run needs one line that says its subject was **there** — here the
    `method return` of the `ShowCapture` call, printed beside the result — and
    that line belongs in the output whether the run comes out empty or full.

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

27. **A player that was never started looks stopped.** Measured 2026-08-29 on
    #26: the picture delivered as evidence for "the player is muted **and
    stopped** while editing" was taken from a player that had never played —
    it read `0:00 / 0:41`, which is what a paused, a stopped and an untouched
    player all look like. The same trap in a check: an assertion on
    `StoppedState` that is not preceded by an asserted `PlayingState` is green
    whether the code stops anything or not. The evidence is the transition, so
    assert the loud state first — and let the counter-run come out different:
    without `stop()` the same picture shows the pause symbol, a bar at 35% and
    `0:14 / 0:41`. A file long enough not to end by itself is part of it; a
    player that finishes on its own passes the assertion without the code
    doing anything.

28. **`QT_QPA_PLATFORMTHEME=kde` selects the platform theme, not a font —
    without it the run measures a different style.** Measured 2026-08-29 on
    #26: with the variable, `QApplication::style()->objectName()` is `breeze`
    (`Breeze::Style`) and `PM_DefaultFrameWidth` is 0; without it, `fusion`
    and 1. The substitute font that rule 2 names is one consequence among
    several — `QIcon::fromTheme()` hands back an empty icon without a
    resolvable theme, and the platform integration replaces a built
    `QMessageBox` with a substitute dialog of its own (both already noted in
    `tests/CMakeLists.txt`). So **every** number that comes out of the style is
    affected — `pixelMetric`, `sizeHint`, `frameWidth` and any geometry
    assertion built on them — including in a run that never draws anything.
    The proof is the style name read back inside the run, not the variable
    being set: the variable can be set and the plugin still missing. And
    `frameWidth() == 0` alone still only says "no space reserved": that Breeze
    draws nothing there was settled by counting border pixels — 0 of 240 differ
    from the middle for a plain `QFrame`, 240 of 240 for `QFrame::Box` and for
    a `QTextBrowser` of the same shape.

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

31. **`gh issue view <n> --comments` prints nothing at all for an issue that
    has none, and exits 0.** The flag shows the comments and *only* the
    comments, so a blank screen means "no comments" and looks exactly like a
    call that failed, an expired login or a network that was not there.
    Measured 2026-08-29 on #112: the command came back empty four times over —
    sandboxed and not, with and without `--repo`, redirected into a file of 0
    bytes — while the issue had a full body and `gh api` answered normally. The
    first step of every issue here is reading the body and every comment, and
    that step's output cannot say whether it read anything. Read the issue with
    `gh issue view <n>`: it prints the body and a `comments: N` line, and that
    number is the readback which says whether `--comments` has anything to
    show.

32. **A queue is idle between two jobs, and a check that waits for idle stops
    there.** Measured 2026-08-29 on #113: after a run had been given up on, the
    check waited for `Transcriber::isBusy()` to go false and then counted what
    the given-up runs had left behind. `endJob()` sets the step to Idle and
    posts the next take through the event loop, so `isBusy()` is false in that
    gap — the wait returned after two of the three runs, and every assertion
    behind it was taken while the queue went on working underneath. It came out
    as a wrong count; it could as easily have come out green, because a working
    directory that the next job has not created yet is a directory that is not
    there. Wait for the number of ends the queue can still produce — here three
    failures, one attempt left on the first note and two on the second — and use
    the idle state only after that number is in.

33. **A green `makepkg` on the development machine says nothing about
    `depends` — every package it would name is already installed there.**
    Measured 2026-08-29 on #41: `makepkg -e --nobuild` printed its two
    dependency-check lines and nothing else, both with `kxmlgui` in `depends`
    and with that line deleted. makepkg asks `pacman -T`, which
    answers "satisfied" for a name that is installed and stays silent about a
    name that is not written down at all — so the run that builds the package
    is exactly the run in which a forgotten dependency cannot show up. The
    package builds, installs, starts, and fails on the first machine that does
    not happen to have the library. Two checks do carry, and neither needs a
    clean chroot: whether every declared name resolves to a package
    (`pacman -Ssq "^name$"`, which catches a typo), and whether the package
    owning each `DT_NEEDED` entry of the built binary stands in the dependency
    closure of what `depends` declares (`readelf -d` plus `ldd`, `pacman -Qoq`
    and `pactree -u` — namcap's check, done by hand because namcap is not
    installed here). Both were run against a control that came out different:
    with `kxmlgui` and `kstatusnotifieritem` struck from the list the second
    check named exactly those two libraries as uncovered, and a deliberately
    misspelt `kxmlgu1` was the only name the first one rejected. **What neither
    of them reaches** is a runtime dependency that is not linked — `ffmpeg` as
    a program, `libplasma` as theme data, `breeze-icons` as icons. Those are
    read out of the code and out of SPEC 15, and a machine that has them
    installed can never contradict the list.

    **And the closure has to resolve virtual provisions itself, or it invents
    uncovered libraries.** Measured 2026-08-30 on #37 and again in its review:
    the check named `libGLX.so.0` and `libOpenGL.so.0` as uncovered, on the
    changed state and on the unchanged one alike, which read like a package
    that would not start on a foreign machine. It is right that `pactree -u`
    does not list `libglvnd` — `qt6-base` depends on the **virtual** name
    `libgl`, which `libglvnd` provides together with `libGLX.so=0-64` and
    `libOpenGL.so=0-64`, and `pactree` prints the virtual name without
    resolving it to a provider. `libgl` does stand in the closure, the package
    is sound, and the finding was a fault of the check. Whoever runs this check
    resolves each uncovered library's owner against the provides table
    (`pacman -Qi <provider> | grep -i provides`, or `pacman -Sddp`) before
    reporting it — namcap does exactly that, which is why it does not raise it.

34. **A check whose input is ordered so that two rival rules agree measures
    neither of them.** Measured 2026-08-29 on #10: the parser narrows two date
    boundaries of the same direction to the tighter one, and the case handed it
    `vor:2026-06 vor:2026-01` — where the tighter one happens to be the last
    one read as well. Taking the rule out and letting the last value simply
    overwrite the first left the case **green**, because both rules produce
    2026-01-01 for that input. With the narrower boundary written first
    (`vor:2026-01 vor:2026-06`) the same probe came out red at once. The trap
    is not the assertion, it is the order of the input: whenever a rule picks
    between several values, arrange them so that the rule's answer is **not**
    the one the obvious wrong implementation would give — first, last, largest,
    the only one there is.

35. **A mutation probe goes red on the first assertion of the function, and
    that is rarely the case the acceptance criterion asks about.** `QCOMPARE`
    aborts its test function, so a probe reports exactly one failure per
    function and says nothing about everything below it. Measured 2026-08-29
    on #114: the criterion demanded a case that comes out different before and
    after — a note dated exactly on the boundary. With the old reading of
    `nach:` restored, `storetest` did stand red, but on the two **month**
    cases four and six lines further up; the day-exact pair the criterion was
    about was never reached, and reporting that red as its proof would have
    proved a neighbour. Only with those earlier expectations set to the old
    values by hand did the probe reach `nach:2026-06-15` (`2026/06/16`
    against `2026/06/15`) and `nach:2026-07-15` (one note against two), and a
    third pass was needed for `nach:9999-12-31`, which came out `+10000/01/01`
    — the shift into the year 10000 the issue described. A probe proves the
    case it actually reached, so walk the earlier assertions out of the way
    until the one under test is the one that fails.

36. **A tray item without a `StatusNotifierWatcher` on the bus exports nothing,
    and the item is never on the daemon's own bus name.** Measured 2026-08-29
    on #24, where the run had to show what the tray announces while the built
    daemon runs. Both halves look identical from outside — `UnknownObject` for
    `/StatusNotifierItem` — and both read like a tray icon that was never
    built:

    - With no watcher registered, `KStatusNotifierItem` falls back to a legacy
      `QSystemTrayIcon` and exports no D-Bus object at all. Nothing in the
      daemon's output says so. A stand-in for plasmashell is therefore part of
      the run: a `org.kde.StatusNotifierWatcher` at `/StatusNotifierWatcher`
      that takes `RegisterStatusNotifierItem` and answers
      `IsStatusNotifierHostRegistered` with `true` — forty lines of
      python-gi, and the item goes into the real protocol at once.
    - The item then registers on a **connection of its own**, not on
      `io.github.hnsstrk.denkzettel`. `gdbus introspect --recurse` on the
      daemon's name lists `/Daemon`, `/MainApplication` and the KDBusService
      path, and no item — the address is the unique name the watcher was handed
      in the registration, and the watcher's log is the only place to read it.

    And the reading has to fall **between** the two states, not after them: the
    first attempt asked for `Status` once the job was done and got `Active`,
    which is also what a daemon that never reported anything answers (finding
    27's family). With a stand-in for `whisper-cli` that sleeps three seconds,
    the same run reads `NeedsAttention` at the start and `Active` after the
    transcript — different twice on the same binary.

37. **A `KNotification` on a bus with no notification server sends nothing at
    all — not a failed call, not a warning, nothing.** Measured 2026-08-29 on
    #115 in a `dbus-run-session`: the same binary that writes one `Notify` with
    a stand-in on `org.freedesktop.Notifications` wrote **zero** with the name
    unowned — `dbus-monitor "interface='org.freedesktop.Notifications'"`
    recorded not one call, and the daemon's own log said nothing about it. The
    output is identical to the build that never had the feature, so a run
    watching for a notification is quiet for a reason that has nothing to do
    with the code. This is finding 36's family for the other channel, and
    finding 21's for the road it takes: `XDG_DATA_DIRS` still points at
    `/usr/share`, where `org.kde.plasma.Notifications.service` maps that name
    to `/usr/bin/plasma_waitforname` — the bus dutifully activates a program
    that only waits for the name to appear, and the call stays in that wait. So
    a stand-in owning the name is part of the run, and it is what the "already
    activated" line in the daemon's output is really reporting.

38. **`KColorScheme` never reads the application palette — a runner that sets
    its palette by hand measures two sources at once.** Measured 2026-08-29 on
    #77, where the mark under a hit is `NeutralBackground` of the View set.
    `readmeshots` throws its configuration directory away and then sets a dark
    palette in code; in such a run `QApplication::palette()` says
    `Base #141618` and `Text #fcfcfc`, while
    `KColorScheme(QPalette::Normal, KColorScheme::View)` hands back the
    built-in **light** defaults — background `#ffffff`,
    `NeutralBackground #fef1ea`, `NormalText #232629`. A picture taken that way
    shows a near-white mark in a dark window and reads as a fault of the
    product; it is a fault of the runner. That is finding 8 one storey further
    down: not the theme graphic this time, but every colour role a widget asks
    for. What works is a `kdeglobals` of the run's own — copy a `.colors` file
    out of `/usr/share/color-schemes/` into the throwaway `XDG_CONFIG_HOME`
    **before** `QApplication`, and the platform theme and `KColorScheme` read
    the same file: `#3c1f05` on `#fcfcfc` under BreezeDark, `#fef1ea` on
    `#232629` under BreezeLight. And read the colour back inside the run, the
    way finding 28 reads the style name back.

39. **A picture that looks right does not say the rest of the window is
    unchanged — the difference against the unchanged build does.** Measured
    2026-08-29 on #77: the lines carrying a mark were laid out through
    `QTextLayout` while every other line went on through
    `QPainter::drawText()`, and the marked line came out **one device pixel
    higher** than its neighbours. Nothing in the picture said so — the mark sat
    on the right letters and the row looked like the rest. The pixel difference
    against the same picture taken from the unchanged build named it in one
    step: not the mark alone but the **whole** line differed, x 20 to 425, and
    moving the new picture down by one row cut the summed channel difference
    over that line from 477,435 to 147,493. With the type going through the
    same `drawText()` in both cases and only the mark's ground and its clipped
    second pass added, the difference shrank to exactly the two mark
    rectangles and nothing else. Whoever changes how something is drawn
    compares pictures: what the change was not meant to touch has to come out
    **bit for bit** as it was.

40. **A unit test written from the documentation asserts the case the code
    never reaches.** Measured 2026-08-29 on #13: `QNetworkRequest::
    setTransferTimeout()` is documented to abort the reply with
    `OperationCanceledError`, the error mapping was written for that value, and
    the test case handing that value in was green. Against a real Ollama with
    the limit at 5 ms the reply came back as **`TimeoutError`** — so the
    timeout branch had never once fired, and a bitten limit reached the user as
    "Ollama could not be reached: Zeitüberschreitung", the transport sentence
    over a limit of our own. The check was green because it and the code read
    the same documentation; nothing in the pair could contradict it. This is
    finding 17's family — the lever the source says is connected — one storey
    up: there it was a variable that did not switch anything off, here it is a
    return value the library does not produce. **A mapping of foreign error
    values gets one live run per value that a real service can actually
    deliver**, and the value goes into the test from that run, not from the
    documentation.

41. **A stand-in that fails only the first time cannot show a repeat that is
    not yours.** Measured 2026-08-29 on #13: the proof that the provider
    retried once used a stub which closed the first connection per endpoint and
    answered the second. It saw two requests, the run succeeded, and that was
    reported as "the retry is proven". It was — and the number was wrong.
    Against a stub that closes **every** connection the same call makes
    **three** requests, because `QNetworkAccessManager` repeats a closed
    connection by itself; the first stub had ended the experiment before the
    third attempt could happen. The same shape hid the timeout: 30 s became 60,
    and nothing in the successful run said so. A stand-in built to be survived
    measures the first failure and stops. **Whoever counts attempts makes the
    fault permanent** — closes every time, never answers — and reads the total
    off the stand-in, not off the client's result.

42. **`KSharedConfig::openConfig()` names its file after the application, and
    a QTest binary is not the application.** Measured 2026-08-29 on #16: the new
    `settingstest` pressed Apply on a `KConfigDialog` and read the result back
    out of `denkzettelrc` — while the dialog had written `settingstestrc`,
    because QTest fills `QCoreApplication::applicationName()` from the binary
    and the skeleton takes its file name from there. The case stood **red over
    a program that was doing exactly the right thing**; the same setup the other
    way round — an assertion that a key is *absent* — would have stood green
    for ever, on a file nobody ever wrote. So a check of a configuration file
    sets `setApplicationName()` to the name the daemon registers, before the
    first `KSharedConfig` call. **The second half of the same run:** with the
    file name put right the case was green once and red on the next run. It
    writes what it asserts, so the second run set the widgets to the values that
    already stood in the file — no change, and Apply stays grey. In the other
    order the same case would have been **green over a write that never
    happened**. A check that writes into a configuration therefore deletes it
    first, and that deletion carries a guard on `XDG_CONFIG_HOME`: the same line
    without one deletes the file of whoever runs the check. **And the tidy-up line at
    the end of a case only runs when the case passes — which is exactly when it
    is not needed.** Measured 2026-08-29 on #119: the new case was the first in
    `aitest` to write the shared `[AI]` group, it died on its own assertion,
    and the model name it had set stayed in the group — which took
    `everyNoteGetsItsOwnVector()` red with it four hundred lines further down,
    a red saying nothing about the code under it. The deletion in
    `initTestCase()` does not reach this: it runs before the first case, and
    the leak happens **between** two cases of one run. So the group goes away
    through a `qScopeGuard`, which runs on the abort path as well — measured in
    review: a case that dies on `QCOMPARE` still leaves the group clean, and
    the next case, which reads its default out of that very group, passed in
    the same failing run. This is finding 29's mirror image: there the tidying
    runs too early and hides the fault in the check that is measuring, here it
    does not run at all and the damage lands on a case that has nothing to do
    with it.

43. **Breeze animates the dot in a radio button, so a `grab()` in the same turn
    draws the state the animation starts from.** Measured 2026-08-29 while
    reviewing #16: the picture showed „Ollama" checked while `choice()` read
    back 2 (OpenAI) in the same run and the three button states were 0/0/1.
    `QTest::qWait(400)` before the grab and the picture agrees. This is
    finding 19's family for widget animations rather than for KSvg — and it was
    only visible because a number was read back beside the picture. A picture
    on its own would have been reported as a fault of the product, or worse,
    have hidden a real one.

44. **A shell check that parses a file can be fooled by a comment in it, and
    `set -e` then ends the run mid-way with a plausible answer.** Measured
    2026-08-29 on #36, running finding 33's dependency-hull check by hand: an
    apostrophe inside a PKGBUILD comment („somebody else's machine") made the
    name extraction invent the packages `s` and `machine.`, `pactree` failed on
    them, and the loop died where it stood — with `kxmlgui` reported as
    UNCOVERED although it stands in `depends`. The output looked like a finding
    and was an artefact of the parser. Strip the comments first
    (`grep -v '^ *#'`), and treat a check that names something you know to be
    there as broken until proven otherwise.
45. **Ollama takes the reasoning out of the answer, so the backend at hand
    cannot show the case a robustness against it is built for.** Measured
    2026-08-29 on #14 with Ollama 0.32.15 and `qwen3:8b`: the classification of
    SPEC 7.2 has to survive a thinking block, and against this server not one
    ever arrived. `message.content` carried the bare JSON object every time,
    while the reasoning — 8,184 characters for one note — stood in a
    `thinking` field of its own; the same on `/api/chat` with `think` true and
    with `think` false, and on `/api/generate`, where it is `response` beside
    `thinking`. So a live run against this machine says nothing at all about
    the parser: with the block stripping deleted it would have come out
    identical, which is the first rule of the verification stance. The
    robustness is still owed — SPEC 7.1 puts openrouter and OpenAI beside
    Ollama, and those hand the text through as the model wrote it — and it is
    checked against the stand-in, where the block is written by hand. That is
    finding 40's family with the opposite ending: there the documented value
    never occurred and the code was wrong, here the expected shape never
    occurs and the code is right. Both times the question is the same, and it
    is asked **per backend**: does this server produce the case at all, and
    what does it produce instead?

46. **A permission check run as root proves nothing — for root everything is
    writable.** Measured 2026-08-29 on #75, whose criterion is that a
    non-writable vault path is reported when it is set. `QFileInfo::isWritable()`
    asks the *effective* user, and the CI runs in an Arch container as root, so
    the case would have stood green over a program that happily stored the
    rejected folder. Measured by hand as uid 1000 with `chmod 0500` it comes out
    the other way: the message turns red and the field falls back to the last
    accepted folder. **`isExecutable()` is not the same case, and that was
    measured too**: on Linux X_OK is granted to uid 0 only where at least one
    execute bit is set, so a file at 0644 is executable for nobody. Measured
    2026-08-29 on #27 under `unshare -r` as uid 0 — the whole `settingstest`
    came out 8 passed, 0 failed with a case that rejects exactly such a file,
    while a file at 0000 *is* writable in that same namespace. The rule is
    therefore about the checks root overrides — `isWritable()`, `isReadable()`,
    reaching into a directory — and not about every mode bit. Where it bites:
    either the check drops privileges for the run, or the
    case stays out of the set and is measured by hand — but it never stands in
    a set that runs as root, where it can only ever say yes.

47. **A mutation probe run outside `ctest` goes red for a reason that is not the
    mutation.** Measured 2026-08-29 on #15: `shelltest` was to show that a tray
    entry connected to nothing fails its check, and the binary was started by
    hand. It did fail — with "the tray menu carries no entry for an analysis
    run", because without the `LANGUAGE=en_US` that `tests/CMakeLists.txt` sets
    on that set, the installed German catalogue answers `i18n("Analyze now")`
    with „Jetzt analysieren" and the check looks for the English wording. The
    **unchanged** binary failed exactly the same way, so the red said nothing
    at all. With the three variables of the test property in place the same
    binary came out green unchanged and red with the connection taken away, on
    the assertion the probe was aimed at (`requested.count()` 0 against 1).
    This is finding 35's family from the other end: there the probe reached the
    wrong assertion, here it reached the right set in the wrong environment. So
    a probe is run the way `ctest` runs it — `ctest -R <set>`, or for a single
    function the ENVIRONMENT of its `set_tests_properties` set by hand — and
    its red is read on the assertion it was aimed at, never on the first one
    that happens to fail.

48. **A comment that presents a test set as a guard does not say the set can
    reach the thing.** Measured 2026-08-29 on #15: `analysisscheduler.h` called
    itself the one place the trigger names and the bounds stand, and `aitest`
    claimed to pin "the three choices, which settings.cpp declares" — while both
    values stood a second time in `settings.h`/`settings.cpp`, and the set reads
    only the constants of its own library. `choice("AfterSaving")` renamed to
    `"Immediately"` and the floor moved from 5 to 15: **all fourteen test sets
    stayed green**, while the scheduler would have been reading a trigger nobody
    can set any more. The same fault in #16, where a comment presented `aitest`
    as the guard of defaults out of a library `denkzettelsettings` does not even
    link. Whoever writes that two sides have to agree names the build target
    that carries the other side — where the set does not link it, the guard is
    an assertion and nothing else. What answered it here was not a check but one
    source, because no check could: `aitest` cannot link `denkzettelsettings`,
    which links the analysis library and not the other way round — the literal
    put back into `settings.cpp` leaves all fourteen sets green today as well.
    **The guard is then the build, and a guard is read back like any other
    value**: renaming the constant at its one place stops the compiler in
    `settings.cpp:56` with "is not a member of `analysis`", which is what says
    the other side really is built from it.

49. **The shortcut service is `kwin_wayland` itself, and a component whose
    desktop file the run cannot reach is dropped whole.** Measured 2026-08-29
    on #74, where the readback of SPEC 2.4 had to come out different once.
    Two halves, both of which cost a run:

    - `/usr/lib/kglobalacceld` started under a bare `dbus-run-session` **exits
      at once and writes nothing** — not a line into the pipe, and the name
      never appears on the bus (finding 25's family: what it has to say goes to
      the journal). In a nested `kwin_wayland`, `busctl --user list` shows
      `org.kde.kglobalaccel` owned by **kwin_wayland**, so starting a daemon
      beside it changes nothing and killing that daemon changes nothing either
      — a run built as "with and without kglobalacceld" comes out identical
      twice and proves nothing.
    - The lever that does work is the component. kglobalacceld resolves a
      component name ending in `.desktop` through the desktop file and creates
      **no component at all** without one. Same binary, same service, one
      argument apart: with `io.github.hnsstrk.denkzettel` (installed under
      `/usr/share/applications`, finding 21's road) the write arrives —
      `isComponentActive` true, the readback hands back `Meta+Shift+K`, and a
      **second process** in the same session still reads it. With a made-up
      name the readback is empty for both actions, `isComponentActive` is
      false, and the page reports the failure. So a run that has to make a
      global shortcut fail does it by taking the desktop file away, not by
      taking the service away.

    And finding 20's trap stood in every one of those runs, on the same
    service: `setDefaultShortcut()` (the `IsDefault` flag) returned **true**
    while the readback stayed empty, next to a `setShortcut(..., NoAutoloading)`
    — which carries `SetPresent` — that really landed.

50. **A header comment is a statement of intent, not a measurement — findings
    15 and 18 hold for library documentation as much as for D-Bus.** Measured
    2026-08-29 on #74: `kkeysequencewidget.h:296` says the component name has to
    be set or the application's own registered shortcut reports a conflict with
    itself. It does not. With our own component holding the sequence,
    `isGlobalShortcutAvailable` answered **0** for an empty name, for our own,
    and for a foreign one alike, while a free sequence answered 1 in all three
    — `Component::isShortcutAvailable` resolves the own name to
    `shortcutContext("default")`, and that context holds the key. The line is
    the documented way to build it and stays; only the reason beside it was
    wrong.

    **The way the error travelled is the finding.** The UX pass read it in the
    header and wrote it into a decision, the implementer reported it as checked,
    the lead carried it into two briefs — three stations, none of them measured,
    because all three had read the same source and could not contradict each
    other (finding 40). What an interface says about itself is an intention;
    what it does is the measurement. Quote a header comment as a *source*, never
    as a *result*.

51. **An item view hands the value back that it is not showing.** Measured
    2026-08-29 on #18: the category column is a `QTreeWidget` of two columns,
    label left and counter right. Every counter was set and every readback said
    so — `item->text(1)` answered `3`, `0`, `0` for the right rows — and the
    picture showed a column of headings with no numbers at all. `QTreeView`
    switches `stretchLastSection` on by itself, and that **beats** the resize
    mode set on the section: with the sidebar 158 px wide the counter column
    took 100 px (the style's own `minimumSectionSize`), the label column was
    left 79, the labels elided to "CLI com…" and the numbers stood outside the
    viewport. `setStretchLastSection(false)` plus a `setMinimumSectionSize()` of
    its own put it right — 136 and 22. The model readback proves the value is
    **set**, never that it is **visible**; that is finding 27's family for
    layout rather than for state, and only the image told the two apart. Ask
    `columnWidth()` beside the value, or look.

52. **Two catalogues are compared by their set of msgids and against the common
    ancestor, never by two numbers from two states.** Measured 2026-08-29 on
    #17 — as the mistake: `msgfmt --statistics` fell from "139 on `main`" to
    136, that was read as a loss, and a finding was written about it. Both
    numbers came from different states; `main` had moved on by several merges
    between the branch point and the comparison, and the difference was their
    gain, not the branch's loss. Measured at the common ancestor the same thing
    reads 147 → 150, the three additions being the story's own strings and
    nothing gone at all. A difference between two numbers does not say **which**
    message is missing, and without that it cannot be read: take
    `git merge-base`, turn both catalogues into sets of msgids with
    `msgattrib --no-obsolete --no-wrap`, and **name** every departure — then
    hold each one against `grep -rn` in `src/`. And `grep '^msgid '` alone is
    not enough for that: a msgid xgettext has wrapped (`msgid ""` with the text
    beneath it) does not appear in its output.

53. **`KColorUtils::contrastRatio()` uses the WCAG weights through a different
    transfer function, and comes out more generous throughout.** Measured
    2026-08-29 on #116: the channel weights are the same — pure primaries come
    out identical on both (red on white 3.9985 against 3.9985), and that is
    exactly what hides the difference. `KColorUtils::luma()` is a plain
    `pow(v, 2.2)` per channel (identical to six decimal places at every point
    of the grey ramp), WCAG uses the kinked sRGB curve; on dark colours KDE
    therefore reads higher — `#232629` on `#fef1ea` gives 14.69 against 13.75.
    Over the eighteen installed schemes the KDE value is above the WCAG value
    in **all** eighteen, the worst type contrast reads 4.79 against 4.55, and
    the closest case is CachyOSNord with 4.787 against 4.552 — five hundredths
    above the promise instead of twenty-nine. From which follows that two pairs
    of numbers for the same change can **both** be right when they are computed
    on different scales: the implementer's 4.94 → 4.79 and the issue's
    4.63 → 4.56 are the same measurement, once with `contrastRatio()` and once
    by WCAG, and neither refutes the other. **Whoever writes a contrast
    threshold names the scale beside the number, and where an accessibility
    criterion is to be met, computes the WCAG value beside it** — a 4.5 : 1
    written with `contrastRatio()` is not the 4.5 : 1 the guideline means.

54. **A stand-in that answers every call with the same value does not weaken a
    check — it builds a state the check is not about.** Measured 2026-08-29 on
    #29: the embedding mock had one default vector, so every note of a run came
    out identical, similarity 1.0, and any case with three or more notes
    silently grew a cluster and a bundle call on top of what it was testing.
    One case then stood at 4 prompts against 3 — measured five times out of
    five with the mock's differing vectors taken away, and five green with them
    back, so it is deterministic and not the flake the first report of it
    claimed. That claim was checked because a list entry naming an observation
    the next reader cannot reproduce damages the list. A
    stand-in's constant answer is an input like any other: give it values that
    differ, or the check runs against a corpus the product would never see.
    Where the code computes a **relation** out of the answers — similarity,
    distance, ranking, grouping — a constant stand-in is not a placeholder but
    an instruction, and it instructs the degenerate case.

55. **Every agent of a team writes into the same scratchpad, and an obvious
    file name is overwritten by whoever writes it next.** Measured 2026-08-29
    while reviewing #29: a `lint-tidy` run redirected into
    `$SCRATCHPAD/tidy.log` was read back and named 58 files of a **foreign**
    worktree, none of its own — `suggester.cpp` and `modelanswer.cpp`, the
    files under review, appeared nowhere; two hours later the same path named a
    **third** worktree. The directory held 800 entries from runs on nine other
    issues, under one session directory for the whole team. The exit code stays
    the running process's own, so the **verdict** was right and the
    **evidence** belonged to somebody else — the worse half to get wrong,
    because the authoritative-looking number is the one that survives. Not a
    build-system fault: `USES_TERMINAL` was suspected and cleared, a redirected
    custom target writes to its own file. **Worse than swapped logs**: the same
    directory holds the `.orig` copies a mutation probe makes before it mutates
    and plays back afterwards — two agents with the same name, and one restores
    the **foreign** source over its own tree. What carries: every run writes
    into a subdirectory of its own, and every scratchpad file carries a name
    only this run can have — the issue number, the worktree hash — never a
    generic `build.log`, `out.txt`, `probe.cpp`. And read the readback: a log
    used as evidence has to name **your own** path, the way finding 28 reads
    the style name back.

56. **A review measures a commit, and a worktree is not a commit.** Measured
    2026-08-29 on #21: the reviewer's verdict was written against `99a0bcd`,
    the implementer was then sent the findings, and by the time a second
    measurement was needed the same worktree stood eight files and 203 lines
    further on — the fixes for the very findings under discussion. The first
    re-run would have built that newer state and reported it under the old
    commit's name, and nothing in the output says which state was built. This
    is the lead's error before it is anybody's: whoever asks for a second look
    while the branch is being worked on has to say **which** state it is on.
    What carries: a review builds from `git archive <commit>` into a directory
    of its own, and its report names the commit it holds for — not the branch,
    which has moved by the time the report is read.

57. **A staging directory installed from a stale build serves the old
    catalogue.** Measured 2026-08-29 on #21: the `.mo` is compiled by the
    **build**, not by `cmake --install`, so a readback taken after editing the
    `.po` and installing to a `DESTDIR` named the **previous** wording — which
    reads exactly like a translation nobody wrote, or like a string that never
    reached the catalogue. Build first, then install, then read back. This is
    finding 16's neighbour: there a check read an installed path and said
    nothing about the source tree, here it reads an installed path that is
    older than the source tree.

58. **`QString::arg()` fills by the lowest free number, so a mutation that
    deletes one placeholder rewires the rest.** Measured 2026-08-29 on #117:
    the probe was to take the date out of the classification prompt and deleted
    only the sentence carrying `%2`. `arg()` then filled `%1` and `%3`, the
    date landed where the note text belongs, the note fell out of the prompt
    altogether — **two unrelated cases went red and the case under test
    passed**, on a prompt that carried the date in the note's place. A red for
    a reason that is not the mutation, and it looked exactly like a finding.
    With the placeholders renumbered so that nothing but the date is gone, the
    same probe comes out 51 passed, 1 failed, on the case it was aimed at. This
    is finding 35's family from a third side: there the probe reached the wrong
    assertion, in 47 the wrong environment, here it made a second change nobody
    wrote down. **Whoever mutates a format string renumbers its placeholders
    with it, and reads the produced string back once before believing the
    run.**

59. **A check that searches a file for a value finds it in the file's own
    comment.** Measured 2026-08-29 on #47: the KWin script filters on the
    application id, and a rename leaves the filter matching nothing while
    everything else reports success (that is what #112 did to the spike of
    #50). The guard against it read the installed script and asked whether it
    contained `QGuiApplication::desktopFileName()`. It always did — the id
    stands in the script's own comment and in the bus name it calls back on as
    well, so the control run with the constant blunted to the retired
    `org.denkzettel.Denkzettel` came out **green**. Against the whole
    assignment (`var OWN_CLASS = "…";`) the same control reads `isScriptLoaded
    b false`, zero calls on the interface and one line in the journal naming
    the file. Whatever a check looks for in a file, look for the **statement**
    and not the token — a value that occurs three times in a file is a value
    the check cannot tell apart.

60. **A widget told to ignore its own width wish can be given none at all, and
    `elidedText()` then returns an empty string.** Measured 2026-08-29 on #47:
    the origin line carried `QSizePolicy::Ignored` so that a long window title
    could not push the head row's minimum width — the price is that when the
    row is over budget the line gets 0 px and disappears **without a word**. At
    714 px window width that is what happened, and the case looked like a value
    that never arrived; at 900 px the same note read `· Konsole …`. And the
    room a design was measured against is read back in the **built** window,
    not taken from the drawing: the UX pass reckoned with 256 logical px free
    in that row, the built window gave the line 57 to 74 px, because the
    reading pane is 440 px of the 900 and the timestamp takes 148 to 165 of
    what is left. That measurement overturned the decision it was checking —
    the origin moved into a line of its own. Whoever builds to a measured
    number measures it again where it is spent, and reports it before the
    picture looks finished.

61. **Two actions with the same wording in one window: a lookup by wording
    picks the wrong one, and the red lands in a neighbouring case.** Measured
    2026-08-29 on #47: the origin removal got an "Undo" of its own beside the
    deletion's, both added to the window with `addAction()`. `librarytest`'s
    `actionNamed()` walks `window.actions()` and takes the first hit, so
    `takesUpTheWaitingNoteWhenTheDeletionIsUndone` — a case that has nothing to
    do with the new feature — triggered the new action, which did nothing, and
    failed on a note count. Both readings of that red are wrong: it is neither
    a broken deletion nor a broken test. What carries: an action that is only
    reached through one button does not go into `window.actions()` at all.

62. **A guarantee that hangs on one end is measured at one end, and the run
    says yes.** Measured 2026-08-29 on #47, by the review: the switch of
    SPEC 13 bars the *source* — with it off the KWin script is never loaded —
    and the nested run showed exactly that, `isScriptLoaded b false` and zero
    calls on the bus, twice over, with a control that came out different. Every
    one of those numbers was right, and the assurance was still open at the
    other end: the receiving method sat on the session bus and took a value
    from anybody who called it, switch or no switch, so on that side the two
    states were indistinguishable. The same shape in the same class: the load
    was read back with `isScriptLoaded`, the **unload** was read back with
    nothing. **Name both ends of an assurance before measuring either** — what
    produces the data and what accepts it, what switches a thing on and what
    switches it off — and read each of them back on its own. A run that only
    ever asks the end you built is finding 40's family with the roles swapped:
    there the check and the code read the same documentation, here they measure
    the same half.

63. **A check that walks the story's one road is green over the state the other
    road leaves behind.** Measured 2026-08-29 on #47:
    `takesTheOriginOffANoteAndPutsItBack()` removed the origin, undid it and
    asserted the value was back — and it was, until the next save. `saveEdit()`
    wrote the copy of the note the editor was opened with, `updateNote()` writes
    every column, and so the restored value went away again without a word. The
    case could not see it because it never opened the editor. Two things follow,
    and the second is the one that costs: whatever a story writes gets asserted
    **after every other road that writes the same row**, and a whole-row write
    from a snapshot is the fault itself — not a fault of the column the story
    added, but one waiting for every column any later story adds beside it. The
    lazy fix and the root-cause fix were the same line: read the row back
    instead of keeping a copy.

64. **The bounding box of a picture difference is not the rectangle of the
    widget that changed.** Measured 2026-08-29 while widening the level meter:
    the difference between the unchanged and the changed picture came out
    `x 92..806, y 59..88`, and read as finding 39 asks it to be read, that is a
    change confined to one row and nothing else. It is not the meter. The meter
    stands at device `x 75..806`; the box begins seventeen pixels further right
    because the **first bar is drawn identically in both builds** and only the
    second one moves, the gap having gone from 3 logical pixels to 4. A box that
    starts inside the widget looks exactly like a widget that has moved right,
    and the picture cannot tell the two apart. What told them apart was the
    widget's geometry read back inside the run — `meter->x()`, `width()`, times
    the device pixel ratio — held against the box. So a difference is read
    against a rectangle **set from outside** (finding 10), and "the difference
    lies inside the changed widget" is a statement about two numbers, only one
    of which the picture carries.

65. **The diagnosis starts in this project's own written record, not in a
    measurement — and the record here is evidence, not intention.** Measured
    2026-08-29 on #125: the customer reported that `Meta+Shift+N` does nothing.
    An hour went into measuring the D-Bus channel — the component is active,
    the sequence is registered, the active keys equal the defaults, no foreign
    component claims them, and a control run proved the channel carries
    signals; five minutes of real key presses produced **zero**, which was
    read as "the key never reaches the service". The opposite conclusion.
    SPEC 2.4 has said since 2026-08-01 what a key press really does: with an
    installed application the component name ends in `.desktop`, so kglobalacceld
    treats it as a service action component and **starts the desktop action of
    the same name instead of sending a D-Bus signal**. Every measurement had
    taken a road the key does not take. **The distinction that makes this a
    rule:** finding 50 says a library's own words are an intention and have to
    be measured — but `SPEC.md`, the issue comments and the vault entries of
    this project record **measurements that were already made here**, each with
    its date and its finding. Reading them is not trusting a promise, it is
    reading a result. So before diagnosing a mechanism this project has already
    written about — shortcuts, the bus, the compositor, the store — read its
    section first, and let the measurement start where the record stops. The
    customer's sentence that opened it: „Wir haben die Doku doch, damit du sie
    auch nutzt."
66. **A window with a clock in it differs from itself between two runs, and a
    picture comparison reads that as the change.** Measured 2026-08-29 while
    making the level meter more sensitive: the eight before-and-after pairs came
    out clean except the last, which showed 133 differing pixels at device
    `x 861..870` — outside the meter, and therefore exactly what finding 39 says
    must not happen. It is the running time, `0:00` against `0:01`: it counts
    the frames the encoder has taken, and how many that is by the eighth grab
    depends on how fast the machine got through the seven before it. Nothing in
    the code moved. So a picture pair from a window that shows a time is
    compared with that field named beforehand as a region the run does not
    control — and the way to tell the two apart is to crop it and **look**, not
    to widen the tolerance.

67. **Two desktop actions with the same `Exec` line are one action, and every
    readback the shortcut service offers says the registration is fine.**
    Measured 2026-08-29 on #125, as the fault: `Meta+Shift+N` reached nothing
    for four weeks while the component was active, `show-recorder` held
    `Meta+Shift+N`, no foreign component claimed the sequence, and
    `invokeShortcut` delivered `globalShortcutPressed` with a control that came
    out different. All of it was true and none of it was the key's road: a real
    press starts the **desktop action** through an `ApplicationLauncherJob`
    (SPEC 2.4), both actions carried `Exec=denkzetteld`, and the started process
    handed itself to the running service as a plain activation — which shows the
    capture window. `Meta+N` "worked" because its target is what happens anyway,
    which is the worst kind of green: the one shortcut that could be checked by
    hand was the one the fault could not touch. Whatever a key press does, walk
    **the launcher's** road — `KService::serviceByDesktopName`, the
    `KServiceAction` of that name, `KIO::ApplicationLauncherJob` — and read back
    what lands on the bus, not what the shortcut service holds.

68. **A launcher on the offscreen platform puts a different signature on the
    wire.** Measured 2026-08-29 on #125: `DBusActivationRunner::startProcess()`
    appends the platform-data map only under X11 or Wayland
    (`KWindowSystem::isPlatformWayland()`), so with `QT_QPA_PLATFORM=offscreen`
    the call goes out as `ActivateAction(sav)` while `KDBusService`'s adaptor is
    `sava{sv}` — D-Bus dispatches by exact signature, and both presses came back
    `No such method 'ActivateAction'`. On the fixed build. Read as the result it
    looks like, that is a fix that does not work; the same run inside a nested
    `kwin_wayland --virtual` came out right for both actions. Offscreen is not a
    neutral platform for anything that asks **which** platform this is, and the
    proof is the platform name read back inside the run, next to the call
    (finding 28's family for the launcher rather than the style).

69. **A bus-activated process inherits the environment the bus was started
    with.** Measured 2026-08-29 on #125, proving that a key press starts the
    daemon when none is running: the nested compositor came up *after*
    `dbus-run-session`, so the activated daemon found no platform plugin and
    died with signal 6 — `jobErrorText=Process … received signal 6`, which reads
    like a broken service file. `UpdateActivationEnvironment` on
    `org.freedesktop.DBus` is what a real session does and what the run has to
    do too. And the control that makes the run evidence is the file itself: with
    the service file staged, the name has no owner before the press and one
    after it, with `Recording failed` arriving from the cold-started daemon;
    without it, the same press answers `The name … was not provided by any
    .service files` and nothing happens.

70. **A mutation that cuts one line out of a multi-line call breaks the
    configure, and the build directory keeps the rules the probe was meant to
    remove.** Measured 2026-08-29 on #125: `grep -v` on the first line of a
    two-line `install(FILES … DESTINATION …)` left a dangling argument, `cmake
    -B` failed with its output redirected, `ctest` ran against the **previous**
    configuration and reported `Passed` — a green that said nothing. Cutting the
    whole call made the same check red on the right message. This is finding
    58's family for build files: read the exit code of every step the mutation
    depends on, and mutate the whole statement. **Its neighbour in the same
    check:** `get_filename_component(… NAME_WE)` cuts at the **first** dot, so a
    reverse-DNS file name gives `io` — the check went red over a correct file
    until it used `NAME` and stripped the suffix itself.

71. **A running compositor holds the desktop file as it stood at login, and
    `kbuildsycoca6` does not reach it.** Measured 2026-08-29 on #125: after
    installing to `/usr`, `kbuildsycoca6 --noincremental` and a restart of the
    daemon — the checksum of the running file compared against the one on disk
    — the key still did nothing, while a check program built for the purpose
    took the D-Bus road at once. Same machine, same desktop file, same minute:
    the real key press put **0** `ActivateAction` on the bus and started one
    systemd unit `app-…@<hash>.service` with `ExecStart=/usr/bin/denkzetteld`;
    the check program, walking exactly the three steps of the launcher
    (`KService::serviceByDesktopName`, the `KServiceAction` of that name,
    `KIO::ApplicationLauncherJob`), put **2** on the bus with `show-recorder`
    and started no unit at all. A second check program read
    `DBusActivatable=true` out of the installed file in a fresh process, so
    the file on disk was right. What differs is the **age of the process**: the
    shortcut service is `kwin_wayland` itself (finding 49), it read the desktop
    file at login before `DBusActivatable` stood in it, and holds that reading.
    That the unit name and the `ExecStart` are precisely what `KProcessRunner`
    writes on its forking road is what says the service does go through
    `KProcessRunner` and only picks the **wrong** runner. So a changed desktop
    file reaches the shortcut service only in a new session, and the
    counter-check came out different: after a restart of the machine the same
    key press took the D-Bus road. Whatever a long-lived service read once,
    measure it in a process of **its** age — a fresh one answers for itself and
    for nobody else.

72. **A stored `none` beats the default, and two actions of one component then
    behave differently on identical code.** Measured 2026-08-29 on #125: after
    the restart the daemon reported the shortcut was not ready, and the
    readback said `Meta+N` had one holder, `show-capture`, while
    `Meta+Shift+N` had **none**. The component's group in `kglobalshortcutsrc`
    carried `show-recorder=none`, and for `show-capture` no line at all.
    `registerShortcut()` deliberately uses the autoloading
    `KGlobalAccel::setGlobalShortcut()` (SPEC 2.4), which writes the default
    only at the very first registration and afterwards restores the stored
    value — so a missing line lets the default land and a line reading `none`
    does not, in every new session. The difference between a working and a dead
    shortcut therefore stood in a configuration file, and no readback of the
    registration the code performs could have shown it. Where the `none` came
    from is **not established**. What overwrites it is a registration with
    `NoAutoloading`, which carries `SetPresent` (findings 20 and 49); the
    customer set the sequence on the application's own shortcut settings page,
    and `Meta+Shift+N` opens the recording window on the installed state. Where
    a default can be overruled by a stored value, read the store beside the
    code — two actions with the same code are not two runs of the same case.

    **Two corrections from 2026-08-31, when #142 settled where the `none` came
    from: our own registration writes it, at every login, because the desktop
    file declares no `X-KDE-Shortcuts`.** With that the mechanism above stands
    unchanged and the sentence "not established" is retired. But **the dating
    does not hold, and the entry cannot be reproduced as written**: an hourly
    backup of the customer's real `kglobalshortcutsrc` shows no group of ours
    in any of 36 snapshots from 24.08. to 29.08. 23:02, and the group appears
    for the first time on 30.08. 08:03. So on the day this was measured, that
    `none` was not in the file this entry names. The likeliest reading is that
    it was read out of one of the dozens of isolated sessions running that day,
    each with an `XDG_CONFIG_HOME` of its own — finding 21 in its purest form,
    and unproven either way. Whoever cites this entry cites the mechanism, never
    the date; and whoever measures a user's configuration says **which** file
    the run actually opened (finding 54).

73. **A prompt to the user inside a running tool call reaches them only after
    the call has ended.** Measured 2026-08-29 on #125: a bus capture was to
    show whether a key press puts anything on the wire, and the line asking for
    the press stood in the same call as the capture, which ran for 25 seconds —
    the user read it once the capture was over. It came back empty for the
    working key as well, and was nearly reported as a finding; it could not
    have come out any other way, which is the first rule of the verification
    stance. What carries: the capture runs in the background, the request goes
    to the user before it starts, and the reading happens after their answer.

74. **`busctl` called with the wrong signature answers with an error, and a
    `grep` over the output swallows it.** Measured 2026-08-29 on #125:
    `getGlobalShortcutsByKey` takes **one** `i`, not `ii`; called with `ii` the
    service answers `No such method … (signature 'ii')`, and because the output
    ran through a `grep` for the application name, what came back was empty and
    read as "no holder for this key" — the same shape as finding 31, where a
    command that prints nothing looks like an answer. Look at the raw output
    before filtering it, and read the interface with `busctl introspect` before
    calling it.

75. **A second daemon out of another build directory runs beside the real one
    for hours, and nothing says so.** Measured 2026-08-29 while tidying up:
    `pgrep -x denkzetteld` found **two** processes. One was the expected one;
    the other had an `exe` pointing into a build directory outside the
    installation, with `(deleted)` behind it, a start time over seven hours
    back, an `XDG_DATA_HOME` of its own, and `systemd` as its parent — it had
    been orphaned when the shell that started it ended. It did **not** hold the
    bus name, read back with `busctl --user list`, so the notes went to the
    right store; but that is the whole trap. `KDBusService::Unique` decides who
    owns the name, and whoever does not own it is there all the same: it holds
    files open, it listens on the same signals, and `pgrep` finds it. So a
    measurement "on the running service" can be taken on the wrong process for
    hours without a hint. This is finding 30's family — a process is what the
    kernel says it is — and finding 1's, where the running process is not the
    installed state. What carries: `busctl --user status <name>` answers who
    the service **is**, `readlink /proc/<pid>/exe` gets asked for **every**
    process found and not only the first, and a run that starts a daemon ends
    it again, or it outlives the shell that started it.

76. **How long a thinking model takes is no property of the note, so a limit
    near that range fails by a coin toss — and a before/after run against the
    real server comes out green on the broken build.** Measured 2026-08-30 on
    #121, and each of the three parts cost a run:

    - **The reproduction.** The first walk put a 316-word note through the
      **unfixed** daemon against a cold Ollama and came back
      `analysiert | persoenlich | 0 attempts` — which reads as "the bug is not
      here". Four runs of the same note through the same prompt on the same
      server then took 18.1 s, 46.9 s, 22.5 s and 45.7 s, two of them over the
      30 s of SPEC 7.1 and two under it. Nothing about the note decides which.
      So a fault that hangs on a limit is not shown by a run against a live
      model, and the control has to be a stand-in that takes the same time
      **every** time — here one that sends nothing for 45 s unstreamed and a
      chunk a second streamed, which is what the real server was measured to
      do. Against that the same pair came out different at once:
      `neu | 1 attempt | "Ollama did not answer within the time limit."`
      against `analysiert | todos | 0 attempts`.
    - **And four seconds-values are not a frequency.** The review's own five
      runs on another note reached 5.2 to 15.0 s and broke the limit **not
      once**, which reads like a contradiction and is none: the throughput was
      constant to 1.2 % (92.5 to 93.6 tokens per second, `prompt_eval_count`
      identical, `load_duration` zero) while the token count ran from 477 to
      1,385. The time is a linear reading of the reasoning tokens and of
      nothing else, so both series lie on one line and the seconds are a
      snapshot of one note on one machine. What generalises is the rate — at
      93 tok/s the 30 s hold about 2,800 output tokens — and that is what
      belongs in a binding text; the seconds may stand beside it as an example,
      never as a measure.
    - **A shortened prompt is a different measurement.** The first number
      written into the commit message, 34.8 s, came from a probe that sent an
      abbreviated version of `classificationPrompt()`. The real prompt, with
      its schema and its rules, makes the same model reason **less**. A probe
      that rebuilds a prompt by hand measures the prompt it rebuilt (finding
      58's family for the input rather than the placeholders); the review's
      probe linked `libdenkzettelanalysis` and called the function.
    - **A lever is read back, or the run measures nothing** — and this is the
      part that was reported wrong before the review caught it. `ollama stop`
      is not a cold start: it frees the graphics card, the model file stays in
      the page cache. But `posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED)` on the
      blob does **not** get it out either, and the numbers reported from it —
      2.3 s and 6.5 s — were warm loads wearing a cold label, indistinguishable
      from the 1.96 s a plain reload measures. `mincore()` over an own `mmap`
      says so in one line: every page resident before the call and after it, on
      two models — 1275727 of 1275727 for `qwen3:8b` and 1305644 of 1305644 for
      `granite4.1:8b`, which was not even loaded — while the same tool on a file
      of the run's own on the same btrfs went 153600/153600 → **0**/153600. The
      lever works, the object refuses it — most likely because the running
      service holds the blobs mapped. Whoever drops a cache reads the cache
      back, the way finding 28 reads the style name back.

      **And the control has to be able to fail.** The review's first control
      file lay under `/tmp`, which is tmpfs — there the pages *are* the file
      and `DONTNEED` can never discard anything. That run looked exactly like a
      confirmation of the finding and was a property of the file system.

      What carried in the end needed no lever at all: on a file measured
      **empty** by the same readback, cold throughput is 5.9 GB/s, so the
      5.2 GB blob is read in under a second and an 18 GB model in about 3.1 s.
      The conclusion the numbers were for — the load is the smaller half — came
      out stronger for being reckoned instead of staged.
77. **`XDG_DATA_DIRS` cannot take a service away from a session bus, and the
    two ways of opening a wallet call two different D-Bus methods.** Measured
    2026-08-30 on #37, four things in one run and each of them looked like a
    result:

    - `<standard_session_servicedirs/>` appends `/usr/share/dbus-1/services`
      **whatever `XDG_DATA_DIRS` says**. The mirror of finding 21 is built,
      `XDG_DATA_DIRS` points at it alone, and `dbus-run-session` still
      activates the service out of `/usr/share` — a run meant to measure "no
      wallet on this session" measured the installed one. What works is a bus
      configuration of the run's own: `dbus-run-session --config-file=…` with a
      single `<servicedir>`. The readback is `busctl --user list`, which then
      does not even list the name as activatable — but **without**
      `--acquired=no`, which hides activatable names and reports nothing for a
      service that is very much reachable.
    - `KWallet::Wallet::isEnabled()` answers **true** on a session with no
      wallet service at all — it reads `kwalletrc` and nothing else. A guard
      built on it lets every request through.
    - `Wallet::LocalWallet()` and `Wallet::NetworkWallet()` are **both**
      blocking D-Bus calls (`localWallet` and `networkWallet`, both in
      `kf6_org.kde.KWallet.xml`) and **both answer the empty string when no
      wallet service is reachable** — that, and not a difference between the
      two functions, is what makes the empty-name check in
      `KeyStore::openWallet()` carry. With a service reachable the first of the
      two costs **one activation of the wallet service** and every one after it
      is negligible — that is the part that holds across machines, and the
      numbers are not: 70 to 181 ms for the first call and 0 to 3 ms for the
      second here, 1422 ms and 7 to 12 ms on the machine beside this one, where
      `ksecretd` had to come up cold. A span measured on one machine is the
      entry that damages the list; what to write down is the shape.
      `LocalWallet()` answered `kdewallet` in every state measured. What
      `NetworkWallet()` answers depends on the state behind the service and is
      not to be relied on: on a session whose Secret Service held a collection
      it answered `kdewallet` too, on one whose did not — `kwalletd6` logging
      `Error reading label: … 'org.freedesktop.Secret.Collection' was not
      found` — it answered empty, four readings in two processes and in both
      call orders. The first version of this entry said the empty string was a
      property of `NetworkWallet()` on KWallet 6.29; a second measurement on
      another session state contradicted it. **A statement about the return
      value of either function is only one once it says what was reachable.**
    - `openWallet(…, Synchronous)` calls `open` on the bus and
      `openWallet(…, Asynchronous)` calls `openAsync`. A stand-in that
      implements only one of them answers the other with `UnknownMethod`, so
      the mutation probe "synchronous instead of asynchronous" came back in
      1 ms and looked exactly like the fixed code. With the stand-in holding
      **both** methods open, the same probe never returns from the call at all,
      while the built code answers in 51 ms with 4042 event-loop ticks in
      twenty seconds. Whoever mutates which call is made makes sure the other
      end can receive it — otherwise the control measures a missing method.

    And the wallet case has a fifth: in a headless session `kwalletd6` asks the
    user before it opens anything (`Using kwallet without parent window!`, then
    silence), so **no** run without a compositor can show a key arriving in a
    real wallet. What carries is a stand-in on `org.kde.kwalletd6` speaking
    `org.kde.KWallet` — a hundred lines of python-dbus — which puts the whole
    road from the code down to the wire under test and keeps the entries in a
    file the run can read without asking our own code (finding 62). Two things
    about that stand-in, both of which cost a run: it has to **load** its state
    file rather than write an empty one at startup, or a run that restarts it
    to change its behaviour reads back an empty wallet and reports the deletion
    it was measuring as done (finding 29's family, the harness tidying away its
    own evidence); and a probe built against the newer interface does not
    compile against the older one, so the "before" side of a comparison keeps
    running the **previous** binary while the build fails beside it
    (finding 11's family — read the exit code of the build, not only of the
    run).

78. **`QDBusReply<bool>` hands back its default when the call failed, so every
    KWallet question that answers `bool` says `false` for "it is not there" and
    for "the wallet did not answer" alike.** Measured 2026-08-30 in the review
    of #37, against a wallet that opens and then refuses every call — the state
    after `kwalletd6` dies with the handle still held. `hasFolder()`,
    `hasEntry()` and `setFolder()` all read `true` on a healthy store and
    `false` on a broken one, which is exactly what they read for an empty one:
    a read then hands out an empty key **with no error**, and a removal reports
    success while the secret provably stays in the wallet — read back from the
    wallet's own side, never from the code that claimed to have deleted it
    (finding 62). Only **three** methods of `KWallet::Wallet` carry an error
    channel of their own — `entriesList(bool *ok)`, `mapList(bool *ok)` and
    `passwordList(bool *ok)` (`kwallet.h:412`, `:426`, `:441`); everything else
    answers `bool`, or one collecting non-zero `int` for everything that went
    wrong. Three of how many is left out on purpose: counted as declarations
    without the signals and the constructors it is 35, by distinct name 33, and
    two readers of the same header came to 35 and 39 — a number nobody
    reproduces the same way carries nothing, and the three named ones carry the
    whole statement. `passwordList(&ok)` on the same binary reads `ok=true`
    against the healthy store and `ok=false` against the broken one, and it is
    the only call that tells them apart. Its `ok` is not quite "did the wallet
    answer" either: the header (`kwallet.h:432–434`) says it is set false **also**
    when an entry in the folder was not written as a password. That direction is
    the harmless one — an error rather than a silent empty answer — but it is
    what the interface promises, and the narrower sentence is the kind the next
    reader does not check. The rule is more
    general than KWallet: **wherever a foreign interface answers a question
    with a plain `bool` or a bare list, ask what it answers when the call
    itself fails** — and if that is the same value, the guarantee built on it
    is not one. The neighbouring trap is the folder: with the return value of
    `setFolder()` unusable for the same reason, letting it fail silently leaves
    the wallet's **global** folder current, and the next `passwordList()` reads
    entries that are not ours.

79. **A hidden label that keeps its last sentence answers the same thing for
    "nothing to say" and for "said it and hid it by mistake".** Measured
    2026-08-30 on #69: the correction line above the library list was hidden
    when there was nothing to report but its text was left standing, and the
    readback beside the picture then printed
    `line=0 text="Ergebnisse für „prüfen“"` for the spelling variant — a line
    that reads like a fault and is none, and that would have read exactly the
    same had the line been wrongly hidden over a real correction. Only
    `isVisible()` told the two apart, and a picture cannot show a widget that
    is not drawn. Clearing the text with the hiding made the same readback
    `line=0 text=""`, which comes out **different** from the case that has
    something to say. This is finding 51 from its other side: there a value
    was set and not visible, here it is visible in the readback and not on the
    screen. Whatever a check reads off a widget it may find hidden, put the
    widget into **one** state per case — or the readback carries two.

80. **`msgcomm --unique` answers symmetrically, and `--no-wrap` does not put a
    multi-line msgid on one line — so both usual ways of comparing two
    catalogues answer the same thing to "was something lost?" and "was
    something added?".** Measured 2026-08-30 on #32, as a disagreement between
    two readings of the same catalogue that were both partly wrong.

    - **`--unique` is `--less-than=2`** (its own `--help` says so), so it hands
      back the symmetric difference and cannot, on its own, tell an addition
      from a departure. A report that names only that option has not stated a
      direction, whatever number stands beside it. **The refinement does
      carry**, and that was measured rather than argued: the difference
      intersected back against each side with `--more-than=1` came out
      **0 departures** on the real comparison and **2** on a control from which
      one message had actually been removed. Different twice, so the procedure
      discriminates — but the count includes the header entry, which is why
      finding 52 asks for the departures to be **named** and not counted.
    - **The other method is blind where it matters most.**
      `grep '^msgid ' | sort -u` plus `comm` is the obvious way and it collapses
      exactly the entries finding 52 already warned about: a msgid carrying an
      embedded newline is written as `msgid ""` with the text beneath it **even
      under `--no-wrap`**, which suppresses wrapping at the column and nothing
      else. Three of this project's messages look like that, so four lines read
      `msgid ""` — the header and three real messages — and `sort -u` makes one
      of them. Control: one of those three deleted for real (`msgfmt` 202 → 201),
      and `comm` reported **0** departures while the msgcomm procedure reported
      it. A whole class of messages can leave the catalogue without that check
      saying a word.
    - **And the totals from either method are not the message count.** The two
      readings of this branch were 194 → 203 and 190 → 199; both are right about
      the change (+9) and neither is the number of messages. `msgfmt
      --statistics` is: **193 → 202**, and it counts no header. Where a
      catalogue is reported on, the total comes from `msgfmt` and the departures
      come from a set comparison whose entries are read.

81. **The modification time of a shared configuration file names its last
    writer, never the writer of the value you are reading — and a value that
    renews itself has no age at all.** Measured 2026-08-31 on #142, and it sent
    the diagnosis to the wrong application twice. The customer reported both
    global shortcuts dead and suspected a dictation tool he had just installed;
    `~/.config/kglobalshortcutsrc` held `show-capture=none` and
    `show-recorder=none`, and its mtime fell in **the same minute** as that
    tool's start. Two facts, one minute apart, and the wrong conclusion. The
    file is written by kglobalacceld for **every** component: `writeSettings()`
    opens with `config.deleteGroup()` and writes the whole component back, so
    any foreign registration restamps our entries without touching their
    content. The tool was cleared three times over, and only the third way
    needed no reading of anybody's source: its own source never names
    `kglobalshortcutsrc`, `kwriteconfig` or `kreadconfig`, it calls neither
    `setForeignShortcut` nor `stealGlobalShortcutSystemwide`, and the package
    log has it installed at **30.08. 11:22:58** while an hourly backup carries
    the first `none` at **30.08. 08:03** — three hours and nineteen minutes
    before the suspect reached the machine, and a day and a half before the
    start that restamped the file and raised the suspicion. The same backup
    shows **no** group of ours at all in 36 snapshots from 24.08. to 29.08.,
    so the value has no earlier occurrence in that file to argue about.
    What actually wrote the value was our own start, at every login, out of a
    missing `X-KDE-Shortcuts=` in the desktop file. **The general rule:** where
    several producers write one file, its timestamp answers a question nobody
    asked. What carries is the history — a backup, a snapshot, a version — and
    a value the system regenerates each session has no first occurrence in the
    file at all, only in the code that regenerates it. Ask what **rewrites**
    the file before reading its age as evidence.

    **And the reason our entry can be regenerated wrong at all is that the two
    writers of that one file are asymmetric.** `Component::writeSettings`
    (`component.cpp:373-377`) writes three fields — active keys, **default
    keys** and friendly name — so an ordinary component's default stands on
    disk and survives anything: `Edit Tiles=Meta+T,Meta+T,…`.
    `KServiceActionComponent::writeSettings` (`kserviceactioncomponent.cpp:132`)
    writes the active keys and **nothing else**, because a service action
    component's default is meant to come out of `X-KDE-Shortcuts` afresh every
    session. Without that key the default is neither stored nor recoverable,
    and the same file that would have carried another component safely through
    carries ours as a bare `none`. Which half of a config file is authoritative
    can differ **per writer within one file** — read the writer of your own
    entries, not the neighbouring line that looks like it.

    **And the neighbouring trap, which cost the same run:** the value looked
    corrupted, and was not. Beside our two lines stood
    `dictation=Meta+Ctrl+,none,OpenWhispr dictation`, whose active field reads
    as a truncated `Meta+Ctrl+` — so the file looked damaged and a damaged file
    looked like the cause. It is a **modifier-only** binding, and
    `QKeySequence::toString()` writes exactly that: `encodeString` appends the
    key name unconditionally after its `+`, and `keyName(0, …)` returns empty
    because every one of its branches tests a truthy key. The discriminator was
    the byte that was **not** there: KConfig escapes an embedded comma as `\,`
    in `serializeList` with no path around it, and `cat -A` showed no
    backslash — which refutes the comma reading rather than supporting it.
    A field separator that can also be payload is read with `cat -A`, and the
    absent escape is the evidence, not the plausible-looking string.

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

Changelog line, version in `CMakeLists.txt`, `pkgver` in `packaging/PKGBUILD`,
tag `vX.Y.Z`, close issues and milestone.

`pkgver` is the one copy of the version number outside `CMakeLists.txt`
(SPEC 15.1 allows the program only one source, and this is not one the program
reads). It has to move with the tag, because the PKGBUILD's `source` fetches
`v$pkgver` — left behind, `makepkg` silently builds the previous release.

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
