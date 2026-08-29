# Spike #50 — the title of the previously active window under Plasma/Wayland

**Question (T8/#50):** can the daemon find out, under Plasma on Wayland, which
application was active *before* the capture window took the focus — and at what
price? The answer decides whether the context stamp (#47) can be built.

**Answer: yes, over one route — a KWin script loaded at runtime over the session
bus.** Measured on KWin 6.7.4, in a nested `kwin_wayland` session with a
throwaway `HOME`. The two obvious other routes do not carry.

## The route that carries

KWin runs the script, KWin keeps the answer, and Denkzettel is told it once —
when a window of Denkzettel's own takes the focus. No privileges, no portal, no
new build dependency: `org.kde.KWin` sits on the ordinary session bus and the
project already links `Qt6::DBus`.

```sh
busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting \
    loadScript ss /usr/share/denkzettel/origin.js denkzettel-origin
busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting start
busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting \
    isScriptLoaded s denkzettel-origin        # b true
```

`loadScript` returns an id (`i 0`) for a file KWin never runs as well, so the
value proves nothing on its own — `isScriptLoaded` is the one that answers, and
the arriving report is the second answer.

The script — [`origin.js`](origin.js) — hangs on `workspace.windowActivated`,
keeps the last foreign window in a variable and sends caption and
`resourceClass` with `callDBus` at the moment a window with
`resourceClass == "io.github.hnsstrk.denkzettel"` becomes active — the
application id of issue #112, read out of the run below and not derived.

## What came out

[`run.sh`](run.sh) is the whole measurement: nested session, three foreign
windows with titles of its own making, `denkzetteld`, two captures.
[`sink.py`](sink.py) stands in for the daemon.

```
11:32:41  sink up
11:33:06  Origin(caption='Fenster C', app='org.kde.kdialog')
11:33:16  Origin(caption='Fenster D', app='org.kde.kdialog')
```

Three switches between foreign windows (A → B → C) before the first capture
produced **nothing**. Then one line per capture, each naming the window the
capture window took the focus from. The titles are set from outside by the
runner, so the two lines can only come from KWin.

The decisive edge case is settled by the unfiltered run of the same route,
which reports every activation instead of filtering. KWin's script output
reaches `kwin-stderr.txt` only with `QT_FORCE_STDERR_LOGGING=1`; without it a
`print()` goes to the journal (`journalctl --user -t kwin_wayland`) and the
file stays empty:

```
js: DZPROBE activated caption=Fenster C resourceClass=org.kde.kdialog
js: DZPROBE activated caption=Denkzettel resourceClass=io.github.hnsstrk.denkzettel
js: DZPROBE activated caption=Fenster D resourceClass=org.kde.kdialog
js: DZPROBE activated caption=Denkzettel resourceClass=io.github.hnsstrk.denkzettel
```

The capture window **does** take the activation — a one-shot query at capture
time would therefore always deliver Denkzettel itself. Only something that was
already listening before the window came up knows the answer. And this is where
`OWN_CLASS` is read off: `io.github.hnsstrk.denkzettel`, the desktop file name
`src/shell/appidentity.cpp` sets, which Qt hands to Wayland as the `app_id`.

**The counter-check.** Same route, same script, but no foreign window ever
active — `denkzetteld` alone in the session, two captures:

```
11:34:13  sink up
11:34:26  Origin(caption='', app='')
11:34:31  Origin(caption='', app='')
```

Two empty strings, not something plausible. The route says "nothing here" when
there is nothing, which is what makes the two lines above evidence.

## The two routes that do not carry

**The Wayland protocol.** KWin 6.7.4 advertises neither
`org_kde_plasma_window_management` nor `ext_foreign_toplevel_list_v1` nor
`zwlr_foreign_toplevel_management_v1` to an ordinary client — measured with
`wayland-info` in the nested session (65 globals) and in a normal Plasma session
(66 globals). What the task manager uses, it gets over a socket of its own.
Without one of those globals there is nothing for a client to bind, and the
protocol XML would have to be installed on top (`plasma-wayland-protocols` is
not on the system).

**KWin's KRunner interface**, `/WindowsRunner` with `org.kde.krunner1.Match`,
lists windows without any script and returns caption plus uuid; the uuid then
goes into `org.kde.KWin.getWindowInfo`. But that answer carries **no `active`
field and no stacking order** — its keys are `activities, caption,
clientMachine, desktopFile, desktops, excludeFromCapture, fullscreen,
hasTransientParent, height, keepAbove, keepBelow, layer, localhost,
maximizeHorizontal, maximizeVertical, minimized, noBorder, pid, resourceClass,
resourceName, role, skipPager, skipSwitcher, skipTaskbar, type, uuid, width, x,
y`. It also returns every window twice (matched by title and by desktop file).
So it can say *which windows exist*, never *which of them is or was in front*.

## What it costs

- **One installed file**, the script, at an absolute path KWin can read. No new
  library, no new `find_package`.
- **A dependency on KWin's scripting API.** `workspace.windowActivated` and
  `window.resourceClass` carry these names since KWin 6; under KWin 5 they were
  called `clientActivated` and the same rename can happen again at KWin 7. The
  breakage is silent: a script whose signal does not exist loads and reports
  nothing — the same shape as finding 15 in `CLAUDE.md`. `OWN_CLASS` breaks the
  same way, and it already did: the spike was written before issue #112 renamed
  the application, the constant kept the old id, and the run went through
  `isScriptLoaded  b true`, two `ShowCapture` calls and an empty `sink.log`
  without a single complaint. Whatever is built on this needs the state read
  back (`isScriptLoaded`) **and** a check that something actually arrives.
- **The script lives in the KWin process.** A `kwin --replace` or a KWin crash
  drops it and the daemon has to load it again; a `QDBusServiceWatcher` on
  `org.kde.KWin` is the place for that. Not measured in this spike.
- **Nothing to switch off but the script itself.** With the setting off, the
  daemon does not load the script and KWin sends nothing at all — no filtering
  after the fact, nothing determined in the first place.

## Running it

```sh
docs/spikes/50-active-window/run.sh build/bin/denkzetteld
```

Never against the session you are working in: window titles are personal data
and this repository is public. The script opens a nested session with a
throwaway `HOME` of its own, and the windows it measures are its own invention.
