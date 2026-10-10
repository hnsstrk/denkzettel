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

Measured cases in which something looked like evidence and was none. **The full
entries, each with the measurement that carries it, are in
`docs/findings.md`** — this is the index, one line per finding.

**How to use it.** Before you build a check, a probe, a picture or a report,
read the group that matches what you are about to measure and open the entries
it names. Before you report a proof, read the ★ lines: they hold for every
task, whatever its subject. An index line is not the finding — it is there to
make you look, and it has never been enough to keep anybody from the mistake.

**Whoever draws a new lesson appends it to `docs/findings.md` and adds its line
here.** Numbers are permanent: the repository cites them 285 times, so a
correction is written into the entry it belongs to and nothing is renumbered.

### ★ Holds for every task

- ★ **10** — a comparison can be wrong on both sides and report "correct"; hold one side against a value set from outside
- ★ **11** — a red result in a reconfigured build directory proves nothing; `rm -rf build` before you believe it
- ★ **29** — a check that clears up before it looks measures its own tidying
- ★ **34** — input ordered so two rival rules agree measures neither
- ★ **35** — a mutation probe reddens on the function's **first** assertion, rarely the one the criterion asks about
- ★ **47** — a probe run outside `ctest` reddens for a reason that is not the mutation; take the set's ENVIRONMENT with you
- ★ **48** — a comment calling a test set a guard does not say the set can reach the thing; name the build target that carries the other side
- ★ **50** — a header comment is a statement of intent, not a measurement; quote it as a source, never as a result
- ★ **55** — every agent writes into the same scratchpad; give every run its own subdirectory and unmistakable file names
- ★ **56** — a review measures a **commit**, not a worktree; build from `git archive`, and never `--amend` a branch under review
- ★ **62** — name both ends of an assurance before measuring either; a run that asks only the end you built says yes
- ★ **65** — the diagnosis starts in this project's own written record, which holds measurements and not intentions
- ★ **85** — every agent gets its own worktree, the lead included; `git commit` writes to whatever HEAD says at that moment
- ★ **92** — a build in a pipe hands its exit code to the pipe, and `ctest` then measures the previous binary
- ★ **93** — a control that cannot come out different is not a control; what you delete for it has to be in the baseline
- ★ **101** — a search cut off by `head` answers "does this occur?" with "I did not see it"; count before you read
- ★ **110** — a lint baseline taken while the files are already rewritten measures a mixed state

### Tests, mutation probes, assertions

- **5** — a test set that does not choose its theme may be testing on one that knows no difference
- **42** — `KSharedConfig` names its file after the application, and a QTest binary is not the application; a tidy-up line at the end of a case runs only when the case passes
- **54** — a stand-in answering every call the same does not weaken a check, it builds the degenerate case
- **58** — `QString::arg()` fills by the lowest free number, so deleting one placeholder rewires the rest
- **61** — two actions with the same wording: a lookup by wording picks the wrong one and the red lands next door
- **63** — a check that walks the story's one road is green over what the other road leaves behind
- **70** — a mutation cutting one line out of a multi-line call breaks the configure, and the old rules stay in the build directory
- **87** — a probe of a schema typed beside the product measures the schema it typed
- **88** — a check reading a value back through the object it wrote through measures memory, not the disk
- **97** — a negative assertion behind an exact comparison of the same value can never go red
- **98** — a restore by `mv` is no restore as far as `make` is concerned; `touch` it and read the recompile back
- **108** — a table counting fault patterns counts knock-on damage as a pattern
- **109** — `git checkout` as a probe's restore destroys unversioned work and drags a foreign branch's files in

### Build, linter, packaging, catalogues

- **16** — a check reading an installed path says nothing about the source tree; stage into a `DESTDIR` of the run's own
- **33** — a green `makepkg` here says nothing about `depends`; resolve virtual provisions yourself or the check invents uncovered libraries
- **57** — a staging directory installed from a stale build serves the old catalogue; build, install, then read back
- **96** — a linter run on this machine counts the toolchain, not the change; compare against the unchanged state on the same clang
- **99** — a branch whose base predates a signature change carries old calls into a merge git reports no conflict for
- **52** — compare two catalogues by their set of msgids against the common ancestor, and **name** every departure
- **80** — `msgcomm --unique` is symmetric and `--no-wrap` does not unwrap a multi-line msgid; neither usual method states a direction
- **83** — `msgcomm` counts an obsolete `#~` entry as present; `msgattrib --no-obsolete` on both sides first
- **100** — a message count does not see a glued-on sentence; the multiset of placeholders does

### Pictures and UI checks

- **2** — `KWindowShadow::create()` reports `true` for one image eight times over, and offscreen is always `false`
- **4** — two live `KSvg::ImageSet` of the same theme share their lookup paths, selectors included; take a **different** theme for the second version
- **6** — with the session locked, `spectacle -f` delivers a black image with return code 0
- **7** — a fullscreen window as a backdrop covers what you wanted to measure
- **8** — a sandbox without `kdeglobals` colours the theme graphic differently from the Qt palette
- **9** — `show()` instead of `showCapture()` delivers a window without a shadow
- **12** — one window is not enough for a fault living in a shared table; ask for it twice
- **13** — `spectacle -m` in a nested `--virtual` session delivers an empty white picture with return code 0
- **14** — a hidden window cannot show a fault that only the layout produces
- **19** — a KSvg graphic changes 100 ms after the call; let the event loop run before the picture
- **28** — `QT_QPA_PLATFORMTHEME=kde` decides the **style**, so every number out of it; read the style name back inside the run
- **38** — `KColorScheme` never reads the application palette; give the run its own `kdeglobals`
- **39** — a picture that looks right does not say the rest of the window is unchanged; the difference against the unchanged build does
- **43** — Breeze animates the radio dot, so a `grab()` in the same turn draws the state it starts from
- **51** — an item view hands back the value it is not showing; ask `columnWidth()` beside it, or look
- **60** — a widget told to ignore its width wish can get 0 px, and `elidedText()` then returns empty
- **64** — the bounding box of a picture difference is not the rectangle of the widget that changed
- **66** — a window with a clock in it differs from itself between two runs
- **79** — a hidden label keeping its last sentence answers the same for "nothing to say" and "hid it by mistake"
- **84** — a picture runner resizing a page by a number of its own measures that number
- **90** — a geometry read in a roomy window is not the size the widgets need; read it where the content overflows
- **95** — a tray item in `NeedsAttention` pulses, so a grab can land in a phase that stands still

### D-Bus, shortcuts, tray, notifications

- **1** — `KGlobalAccel::setGlobalShortcut()` returns `true` even with the daemon unreachable
- **15** — `QDBusConnection::connect()` returns `true` for a signal that does not exist; introspect first
- **18** — `emits-change` in an introspection is a promise, not a signal; check with `dbus-monitor`
- **20** — `shortcut()` answers an empty list for a wrong action id exactly as for an empty component; `IsDefault` returns keys it never set
- **36** — a tray item without a `StatusNotifierWatcher` exports nothing, and never on the daemon's own bus name
- **37** — a `KNotification` on a bus with no notification server sends nothing at all
- **49** — the shortcut service is `kwin_wayland` itself; a component whose desktop file the run cannot reach is dropped whole
- **67** — two desktop actions with the same `Exec` line are one action, and every readback says the registration is fine
- **68** — a launcher on the offscreen platform puts a different signature on the wire
- **69** — a bus-activated process inherits the environment the bus was started with
- **71** — a running compositor holds the desktop file as it stood at login; `kbuildsycoca6` does not reach it
- **72** — a stored `none` beats the default, so two actions of one component differ on identical code
- **74** — `busctl` with the wrong signature answers with an error that a `grep` swallows
- **91** — a `KNotification` with an event id of its own sends nothing without a reachable `notifyrc`, so never from a build directory
- **102** — waiting for a bus name **with a call** activates and starts the installed copy; wait with `busctl --user list`
- **103** — `CloseNotification` after an `ActionInvoked` comes with and without the action; measure an activation by its effect

### Processes, sessions, isolation

- **3** — `activateWindow()` does not fetch the focus back under Wayland; close the window lying on top
- **21** — a nested session with its own `HOME` separates configuration and nothing the system directories carry
- **23** — `kwin_wayland` hangs without a word when the program behind `--` is not executable; and a plausible answer is read as a result
- **30** — `pgrep -f <path>` finds the shell running the check; identify a process by what the kernel says
- **75** — a second daemon out of another build directory runs beside the real one for hours without a hint
- **86** — a `QTemporaryDir` destructor that runs is no proof; a library static writes the path back after `main()`
- **46** — a permission check run as root proves nothing; `isWritable()` can only ever say yes there

### Store, configuration, schema

- **22** — `KDirWatch` swallows an overwrite in place inside the same second; replace the file, do not rewrite it
- **81** — a shared configuration file's mtime names its last writer, and a self-renewing value has no age at all
- **89** — `Settings::self()` reads the file once, so a second dialog shows what the first one saw

### Foreign services, network, models

- **17** — `GGML_BACKEND_PATH` does not switch a backend off, it adds one; read in the source that the lever is connected
- **24** — `QAudioBufferInput` constructed with a format refuses every buffer and reports it nowhere
- **26** — `QMediaRecorder` invents a file name when the output is a directory
- **40** — a unit test written from the documentation asserts the case the code never reaches; one live run per error value
- **41** — a stand-in that fails only the first time cannot show a repeat that is not yours
- **45** — Ollama takes the reasoning out of the answer, so this backend cannot show the case the robustness is for
- **76** — how long a thinking model takes is no property of the note; four seconds-values are not a frequency
- **77** — `XDG_DATA_DIRS` cannot take a service off a session bus; the two ways of opening a wallet call two different methods
- **78** — `QDBusReply<bool>` hands back its default when the call failed, so `false` means two things
- **82** — `QTcpServer::newConnection` counts sockets, and Qt opens one carrying no request after the answer
- **105** — an error body carries an account identifier no documentation names; the redaction stands before the first run
- **107** — `QTcpSocket::flush()` does not block, and the stand-in's closing form decides the request count

### Reports, numbers, reasoning

- **25** — Qt's own warnings do not reach a pipe outside QTest; a grepped run is mute, not quiet
- **27** — a player that was never started looks stopped; the evidence is the transition
- **31** — `gh issue view --comments` prints nothing for an issue without comments and exits 0
- **32** — a queue is idle between two jobs, and a check waiting for idle stops there
- **44** — a shell check parsing a file is fooled by a comment in it, and a swallowed error turns a count of zero into an answer
- **53** — `KColorUtils::contrastRatio()` is more generous than WCAG throughout; name the scale beside the number
- **59** — a check searching a file finds the value in the file's own comment; look for the statement
- **73** — a prompt to the user inside a running tool call reaches them only after it ends
- **94** — a counter kept current by a signal cannot measure the growth of the run that fires it
- **104** — a fault report reading the mechanism out of the code names the wrong state
- **106** — a sentence is true or false at the place where it stands, not by itself

## Before handover

Start the built state and walk the story's main path once yourself. For images
`QT_QPA_PLATFORM=offscreen`, `QT_QPA_PLATFORMTHEME=kde` and `QT_SCALE_FACTOR`
at the user's scaling (**1.5**).

After starting the daemon, look into the journal
(`journalctl --user -t denkzetteld -n 20`) — silent faults of third-party
services stand there and nowhere else.

**A wiring in `main.cpp` can be driven, and "the user's daemon must not be
stopped" is not a reason not to** (measured 2026-09-04 while reviewing #31).
Finding 1 is about the **installed** state; a daemon of your own on a session
bus of its own is a different process and touches nothing. The recipe, which
came out different with the `connect` and without it (a window count of 0 → 1
against 0 → 0):

- nested `kwin_wayland --virtual` under `dbus-run-session`, with a throwaway
  `HOME` and its own XDG directories
- `QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1`, and the a11y bus plus
  `at-spi2-registryd` started **by hand** — activation through the systemd
  unit fails on such a bus
- the window opened over D-Bus, the button then pressed through AT-SPI
  (`do_action(0)`)
- `readlink /proc/<pid>/exe` for **every** process found, to say which binary
  answered (findings 30 and 75)

So a report that says "not run, because the daemon may not be stopped" is
saying the wrong thing. Say "not run" and why it was not worth the run, or run
it.

**The tray menu is reachable the same way, and one kind of button is not.**
Measured 2026-09-04 while reviewing #34, where the question was whether the
overflow line disappears once an export empties the library. The road that
works: the tray entry over `com.canonical.dbusmenu`, `Event(6, "clicked")` —
which is what plasmashell sends on a click — then the window's own buttons over
AT-SPI. It came out different twice on the same road: with the `connect` the
tooltip loses the overflow part three seconds after Accept, without it the part
stays while the export happens all the same.

**What cost two runs is the other kind of button.** The library's "Delete"
button is **never disabled** — only the `QAction` behind it is
(`librarywindow.cpp:1124-1125` against `:2242`), and that action needs a
**current** index. AT-SPI's `select_child` sets the selection, not the current
index, so `do_action` answers `True`, nothing happens, and the store still
holds both notes. Both runs looked identical and measured nothing. Whenever a
button only forwards to a `QAction`, press it and then **read the state back**
— the return value of `do_action` is not one.

## UI review

The yardstick is the KDE Human Interface Guidelines (develop.kde.org/hig) —
Denkzettel is a Qt6/KF6 app for KDE Plasma — together with what SPEC and the
issue's own acceptance criteria lay down.

**`wireframes/Denkzettel Wireframes.dc.html` is a rough guide and nothing more**
(the user's correction, 03.09.2026). It says which areas exist and roughly
where; it does not say what is correct. So it supplies check questions — every
drawn area produces one, the division of space included, and asking them from
the drawing beats asking them from memory — but a departure from it is not a
finding on its own. A drawing that has fallen behind the built state is a
drawing that has fallen behind, not a defect in the product; where it matters,
correct it with a dated addendum, and never report "differs from the wireframe"
as a fault without saying what is actually wrong with the result.

Images that carry a finding go under `docs/images/reviews/` — the directory is
created with the first one.

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

Whoever draws a lesson writes it into `docs/findings.md` and adds its line to
the index above. The next session does not read a log.

**The user schedules the installation to `/usr`** — it needs their password.

## Publication

The repository is **public**. What stands in issues and commits is published.
Permitted are quotes from the user and measurements; not permitted are system
details (host names, kernel versions, paths outside the project, internals of
the home network) and personal data.

**A follow-up commit does not unpublish anything.** Measured 2026-10-10 on
#40, whose review images showed the nested session's launcher bar with a
third-party icon in it. Cropping the files afterwards left four blobs standing
in the branch's earlier commits, and an ordinary merge would have made every
one of them reachable from `main` — the next push publishes what is reachable,
not what the tree shows. The branch had never been pushed, which was the only
reason the choice still existed: it went to `main` as **one squashed commit**
whose tree carries the cropped images, the branch was deleted afterwards, and
the readback is `git rev-list --objects --all` coming back empty for every one
of the four blobs. So: whoever finds that a commit carries something that must
not be published asks first whether the branch has been pushed, and squashes if
it has not. If it has, the decision is the user's.

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
