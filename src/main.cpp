#include "capture/capturewindow.h"
#include "platform/systemfonts.h"
#include "shell/appidentity.h"
#include "shell/daemonservice.h"
#include "shell/firstrun.h"
#include "shell/globalshortcuts.h"
#include "shell/trayicon.h"
#include "store/store.h"
#include "transcribe/transcriber.h"
#include "ui/librarywindow.h"

#include <KConfigGroup>
#include <KDBusService>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QApplication>
#include <QIcon>

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

    return app.exec();
}
