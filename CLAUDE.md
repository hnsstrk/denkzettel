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
