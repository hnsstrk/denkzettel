// The window Denkzettel's capture window took the focus from (SPEC 5.1, 13).
//
// KWin keeps the answer, Denkzettel is told it once: when a window of
// Denkzettel's own takes the focus, this script sends the caption and the
// application id of the window that was active before it. Every other window
// switch stays inside KWin — the daemon never sees it, and that is the guard
// rail of issue #47 in its construction rather than as a promise.
//
// It is loaded by OriginWatcher over the session bus and only while the
// setting „Herkunft der Notiz mitspeichern" is on. Switched off, the script is
// unloaded and KWin sends nothing at all.
//
// A single query at capture time cannot do this: the capture window takes the
// activation itself, so asking KWin then always answers "Denkzettel". Measured
// in the spike of issue #50, whose README carries the two roads that do not
// carry.

// Measured, not derived: KWin reports this window's resourceClass as the
// Wayland app_id, which Qt takes from the desktop file name set in
// src/shell/appidentity.cpp. Whoever moves that name moves this string with
// it, and a mismatch is silent — the filter simply never matches, the script
// loads, isScriptLoaded says true and nothing ever arrives (issue #112, and
// CLAUDE.md finding 15). OriginWatcher::loadScript() therefore holds this file
// against the application's own desktop file name before it loads it.
var OWN_CLASS = "io.github.hnsstrk.denkzettel";

var current = workspace.activeWindow;
var previous = null;

function isOwn(w) {
    return w !== null && String(w.resourceClass) === OWN_CLASS;
}

workspace.windowActivated.connect(function (window) {
    if (window === current) {
        return;
    }
    if (current !== null && !isOwn(current)) {
        previous = current;
    }
    current = window;

    if (!isOwn(current)) {
        return;
    }
    // No window before ours: both strings stay empty. The route says "nothing
    // here" rather than something plausible, and a note then carries no origin
    // — which is the same picture as a note taken with the setting off.
    callDBus("io.github.hnsstrk.denkzettel", "/Origin",
             "io.github.hnsstrk.denkzettel.Origin", "Report",
             previous ? String(previous.caption) : "",
             previous ? String(previous.resourceClass) : "");
});
