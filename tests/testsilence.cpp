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
 * tests/ by the link_libraries() line in tests/CMakeLists.txt, and the static
 * object below runs before main() — so a bench or test written next month is
 * silent without its author ever learning that this file exists, and it is
 * silent even when started by hand without ctest.
 */

#include <QByteArray>

namespace
{

struct SilentAudio {
    SilentAudio()
    {
        qputenv("CANBERRA_DRIVER", "null");
    }
};

const SilentAudio silentAudio;

}
