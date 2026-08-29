#include "capture/capturewindow.h"
#include "platform/systemfonts.h"
#include "settings/settingsdialog.h"
#include "shell/appidentity.h"
#include "shell/daemonservice.h"
#include "shell/firstrun.h"
#include "shell/globalshortcuts.h"
#include "shell/trayicon.h"
#include "store/store.h"
#include "transcribe/transcriber.h"
#include "ui/librarywindow.h"
#include "ui/timestampformat.h"

#include <KConfigGroup>
#include <KDBusService>
#include <KLocalizedString>
#include <KNotification>
#include <KSharedConfig>

#include <QApplication>
#include <QIcon>
#include <QLocale>

#include <optional>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Plasma does not pass a font change on to a running Qt Widgets
    // application — measured 24.08.2026: with the KDE platform theme loaded, a
    // font raised in kdeglobals from 10 pt to 16 pt never arrived. The daemon
    // keeps its two windows for the whole session (SPEC 2.1), so without this
    // they would carry the old font until the next login (issue #68).
    platform::followSystemFonts(&app);

    // Name, version, organisation domain and desktop file in one place —
    // shell/appidentity.cpp says why they may not be set anywhere else.
    registerApplicationIdentity();

    // Before KDBusService below, and that order carries the whole option
    // handling: with a running daemon the single-instance switch hands a second
    // start over to the first process, so --version would open a capture window
    // there instead of writing a line here — and return 0 while doing it
    // (measured, pre-check for #61, F2/F3). Without a
    // reachable session bus KDBusService ends the process with 1, so the
    // version would not appear in the automated run either.
    processCommandLineArguments(app);

    app.setQuitOnLastWindowClosed(false);
    // The bundled copy covers runs from the build directory, before the icon
    // is installed into any theme (issue #43).
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("denkzettel"),
                                       QIcon(QStringLiteral(":/icons/denkzettel.svg"))));

    // KDBusService builds the bus name from the reversed organization domain
    // plus the application name (see kdbusservice.h): hnsstrk.github.io and
    // denkzettel give io.github.hnsstrk.denkzettel, which SPEC 2.3 fixes as the
    // name. Both values come from registerApplicationIdentity() above, so
    // nothing has to be renamed around the registration any more (issue #112).
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    KDBusService service(KDBusService::Unique);

    Store store(Store::defaultDatabasePath());
    if (!store.open()) {
        qCritical("Opening the database failed: %s", qPrintable(store.lastError()));
        return 1;
    }

    // The cleanup check of SPEC 2.5, and here is the only place it may stand:
    // a recording writes its file before the note that points at it exists, so
    // the sweep has to be over before the first window can record. Nothing
    // else in this process touches that directory yet.
    store.sweepOrphanedAudio();

    // Built before the first window and started at the end of this function:
    // it listens on Store::noteAdded for the audio notes to come, and its
    // start() picks up what an earlier run left in the queue (SPEC 12).
    Transcriber transcriber(&store);

    CaptureWindow capture(&store);

    // SPEC 2.3: a second process start must surface the capture window in the
    // running instance instead of just exiting.
    QObject::connect(&service, &KDBusService::activateRequested, &capture,
                     [&capture](const QStringList &, const QString &) {
                         capture.showCapture();
                     });

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    LibraryWindow library(&store);

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    TrayIcon tray;
    QObject::connect(&tray, &TrayIcon::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&tray, &TrayIcon::libraryRequested, &library, &LibraryWindow::showLibrary);
    // The settings are free-standing and belong to no window, so nothing is
    // handed over here — the dialog finds the standing one itself or builds a
    // new one (SPEC 13, issue #16).
    QObject::connect(&tray, &TrayIcon::configureRequested, &app, &SettingsDialog::showSettings);

    // The error path of the transcription reaches the user here and nowhere
    // else (SPEC 10 and 12, issue #24). Both edges ask the same question of the
    // database rather than each carrying its own answer: **the state has to
    // stand where the queue stands.** A success that simply cleared the state
    // would make the icon quiet again while another note lies given up on in
    // the queue, and only the next start of the service would bring it back.
    //
    // Asked on both signals and not on failed(): a first attempt is followed
    // by a second one, so pausedTranscribeJob() still answers nothing there,
    // and a state raised for it would clear itself a moment later.
    const auto showWhereTheQueueStands = [&store, &tray] {
        const std::optional<TranscribeJob> givenUp = store.pausedTranscribeJob();
        tray.setTranscriptionError(givenUp.has_value() ? givenUp->lastError : QString());
    };
    QObject::connect(&transcriber, &Transcriber::paused, &tray, showWhereTheQueueStands);
    QObject::connect(&transcriber, &Transcriber::transcribed, &tray, showWhereTheQueueStands);

    // And the loud channel beside it (SPEC 10 and 14, issue #115): a voice note
    // whose transcription has finally failed is the case the user otherwise
    // notices nothing of for weeks, so it says so — once per note, because
    // paused() is emitted where the attempts are used up and failed() is the
    // one that fires per attempt.
    //
    // Armed only after start() below, and that is the whole of acceptance
    // criterion 3: start() emits paused() for a job that was already given up
    // on before this process began, so that the tray stands where the queue
    // stands after a restart. Announced, it would greet every login with last
    // week's failure. A note that is gone says nothing either — deleting it
    // takes its job row with it (ON DELETE CASCADE), and there is nothing left
    // to tell the user about.
    bool announceGivingUp = false;
    QObject::connect(&transcriber, &Transcriber::paused, &app,
                     [&store, &announceGivingUp](qint64 noteId, const QString &reason) {
                         if (!announceGivingUp) {
                             return;
                         }
                         const std::optional<Note> note = store.note(noteId);
                         if (!note.has_value()) {
                             return;
                         }
                         // The moment it was recorded is what names the note:
                         // that is the handle the library lists it under (SPEC
                         // 9), and one without a transcript has no other. It is
                         // written in the form the library writes it in, out of
                         // the same function — a second form of the same
                         // timestamp would be a second thing to look for.
                         KNotification::event(
                             KNotification::Warning,
                             i18n("Transcription failed"),
                             i18n("The voice note of %1 could not be transcribed: %2",
                                  library::entryTimestamp(note->createdAt, QLocale()),
                                  reasonWithoutDirectories(reason)));
                     });

    DaemonService daemon(&store);
    QObject::connect(&daemon, &DaemonService::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&daemon, &DaemonService::libraryRequested, &library, &LibraryWindow::showLibrary);
    QObject::connect(&daemon, &DaemonService::quitRequested, &app, &QApplication::quit);
    if (!daemon.registerOnSessionBus()) {
        qWarning("Exporting io.github.hnsstrk.denkzettel.Daemon failed; the D-Bus entry points are unavailable.");
    }

    // Store::open() above has created data directory and database; this
    // completes the first start of SPEC 2.5.
    KConfigGroup general(KSharedConfig::openConfig(), QStringLiteral("General"));
    const bool firstRun = runFirstStart(general);

    GlobalShortcuts shortcuts;
    QObject::connect(&shortcuts, &GlobalShortcuts::captureRequested, &capture, &CaptureWindow::showCapture);
    const QList<ShortcutOwner> conflicts = shortcuts.registerCaptureShortcut();
    if (firstRun && !conflicts.isEmpty()) {
        notifyShortcutConflict(conflicts);
    }

    // Last, and after the first start above: the queue may hold a job from a
    // run that was killed, and working it off is the same road as a fresh one.
    transcriber.start();
    // Everything given up on from here on happened while the user was watching,
    // and only that is worth a notification (issue #115).
    announceGivingUp = true;

    return app.exec();
}
