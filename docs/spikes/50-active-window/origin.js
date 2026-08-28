// Spike #50 — the window Denkzettel's capture window took the focus from.
//
// KWin keeps the answer, Denkzettel is told it once: when a window of
// Denkzettel's own takes the focus, the script sends the caption and the
// application id of the window that was active before it. Every other window
// switch stays inside KWin — the daemon never sees it.
//
// Loaded over the session bus, no privileges:
//   busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting \
//       loadScript ss <absolute path to this file> denkzettel-origin
//   busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting start
//
// In the product the destination below becomes the daemon itself; here it is
// sink.py, which writes down what arrives.

var OWN_CLASS = "org.denkzettel.Denkzettel";

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
    // here" rather than something plausible.
    callDBus("org.denkzettel.Spike", "/Sink", "org.denkzettel.Spike", "Origin",
             previous ? String(previous.caption) : "",
             previous ? String(previous.resourceClass) : "");
});
