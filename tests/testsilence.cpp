/**
 * Silence for every test and bench process of this project.
 *
 * The guard dialog is a KMessageDialog since #66, and that build plays a system
 * sound on every show: showEvent() calls beep(), which reaches the
 * `messageWarning` event of plasma_workspace.notifyrc and plays
 * `dialog-warning` through libcanberra — inside our own process, so the stream
 * carries the name of the test binary. One librarytest run makes fourteen of
 * them, in the room the user sits in (measured; „Prüfbericht Testklänge“).
 *
 * libcanberra reads CANBERRA_DRIVER when it opens its context, which is the
 * moment a sound is first played and therefore long after this file has run.
 * Pointing it at the installed null driver keeps everything above the audio
 * device untouched: dialog, buttons, modality, focus and even the KNotification
 * event behave exactly as they do for a user. No test measures sound.
 *
 * Deliberately not KMessageDialog::setNotifyEnabled(false) in the application:
 * the sound is KDE platform behaviour and stays in the product (user
 * decision of 04.08.2026, SPEC 9).
 *
 * The second channel is the session bus, and it carries the same fault one
 * storey up (issue #126): capturetest walks the rescue path of SPEC 2.5 on
 * purpose, RecordingWindow calls KNotification::event() there, and none of the
 * variables ctest sets separates the bus — so the check sent a real "Recording
 * not saved" into the user's desktop, naming a /tmp path and promising a
 * recording would not be deleted. Indistinguishable from the fault it is
 * written for, which is the point of a good message and the problem here. The
 * CI never showed it because the CI has no bus at all.
 *
 * Pointed at nothing rather than unset: with the variable removed, D-Bus may
 * autolaunch a bus of its own and the run would find one (the same reasoning
 * commandlinetest writes over its own `kein-bus`). A KNotification on a bus
 * with no notification server sends nothing at all — not a failed call, not a
 * warning (CLAUDE.md, finding 37) — which is what makes one line here the whole
 * fix for every call site a check can reach: the three in RecordingWindow, the
 * two in GlobalShortcuts, and any written next month. The sixth, in main.cpp,
 * belongs to the daemon alone and no test binary links it. The application is
 * untouched either way; it keeps its own bus and goes on notifying, measured
 * against a stand-in owning the name — two calls with it, none without.
 *
 * commandlinetest is the one set that needs a session bus — it reads back the
 * name the built daemon announces — and it brings one of its own through
 * dbus-run-session. Its ctest ENVIRONMENT says so, and it is the only place
 * DENKZETTEL_TEST_SESSION_BUS is set.
 *
 * Nobody includes this file. Its object is linked into every executable under
 * tests/ by the link_libraries() line in tests/CMakeLists.txt, and the call
 * below runs before main() — so a bench or test written next month is silent
 * without its author ever learning that this file exists, and it is silent
 * even when started by hand without ctest.
 */

#include <QByteArray>

namespace
{

int silence()
{
    qputenv("CANBERRA_DRIVER", "null");

    if (!qEnvironmentVariableIsSet("DENKZETTEL_TEST_SESSION_BUS")) {
        // A path under /nonexistent, which is not a directory on any machine
        // this builds on, so the address can never resolve to a socket.
        qputenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/nonexistent/denkzettel-tests-have-no-bus");
    }

    return 0;
}

// Only the initialisation matters; the value is never read. An int and not an
// object with a constructor on purpose: a non-POD global static brings an
// unclear initialisation order and a destructor run at exit, neither of which
// is wanted for setting one environment variable — and clazy says so
// (non-pod-global-static, the finding that turned the CI red on 04.08.2026).
const int silenceApplied = silence();

}
