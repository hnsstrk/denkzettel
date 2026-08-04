/**
 * Silence for every test and bench process of this project.
 *
 * The guard dialog is a KMessageDialog since #66, and that build plays a system
 * sound on every show: showEvent() calls beep(), which reaches the
 * `messageWarning` event of plasma_workspace.notifyrc and plays
 * `dialog-warning` through libcanberra — inside our own process, so the stream
 * carries the name of the test binary. One librarytest run makes fourteen of
 * them, in the room the customer sits in
 * (docs/scrum/reviews/2026-08-04-testklaenge.md).
 *
 * libcanberra reads CANBERRA_DRIVER when it opens its context, which is the
 * moment a sound is first played and therefore long after this file has run.
 * Pointing it at the installed null driver keeps everything above the audio
 * device untouched: dialog, buttons, modality, focus and even the KNotification
 * event behave exactly as they do for a user. No test measures sound.
 *
 * Deliberately not KMessageDialog::setNotifyEnabled(false) in the application:
 * the sound is KDE platform behaviour and stays in the product (customer
 * decision of 04.08.2026, SPEC 9).
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

int silenceAudio()
{
    qputenv("CANBERRA_DRIVER", "null");
    return 0;
}

// Only the initialisation matters; the value is never read. An int and not an
// object with a constructor on purpose: a non-POD global static brings an
// unclear initialisation order and a destructor run at exit, neither of which
// is wanted for setting one environment variable — and clazy says so
// (non-pod-global-static, the finding that turned the CI red on 04.08.2026).
const int silenceApplied = silenceAudio();

}
