#include "capture/capturewindow.h"
#include "shell/daemonservice.h"
#include "shell/firstrun.h"
#include "shell/globalshortcuts.h"
#include "shell/trayicon.h"
#include "store/store.h"
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

    app.setOrganizationDomain(QStringLiteral("denkzettel.org"));
    app.setApplicationName(QStringLiteral("denkzettel"));
    // Wayland uses the desktop file as the application id, and KGlobalAccel
    // names its component after it (SPEC 2.4) — the file itself is installed
    // from desktop/.
    app.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));
    app.setQuitOnLastWindowClosed(false);
    // The bundled copy covers runs from the build directory, before the icon
    // is installed into any theme (issue #43).
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("denkzettel"),
                                       QIcon(QStringLiteral(":/icons/denkzettel.svg"))));

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // KDBusService builds the bus name from the reversed organization domain
    // plus the application name (see kdbusservice.h). SPEC 2.3 fixes that name
    // to org.denkzettel.Daemon, so the application name has to be "Daemon" for
    // the registration and is restored right after: config and data paths
    // derive from it.
    app.setApplicationName(QStringLiteral("Daemon"));
    KDBusService service(KDBusService::Unique);
    app.setApplicationName(QStringLiteral("denkzettel"));

    Store store(Store::defaultDatabasePath());
    if (!store.open()) {
        qCritical("Opening the database failed: %s", qPrintable(store.lastError()));
        return 1;
    }

    CaptureWindow capture(&store);

    // SPEC 2.3: a second process start must surface the capture window in the
    // running instance instead of just exiting.
    QObject::connect(&service, &KDBusService::activateRequested, &capture,
                     [&capture](const QStringList &, const QString &) {
                         capture.showCapture();
                     });

    LibraryWindow library(&store);

    TrayIcon tray;
    QObject::connect(&tray, &TrayIcon::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&tray, &TrayIcon::libraryRequested, &library, &LibraryWindow::showLibrary);

    DaemonService daemon(&store);
    QObject::connect(&daemon, &DaemonService::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&daemon, &DaemonService::libraryRequested, &library, &LibraryWindow::showLibrary);
    QObject::connect(&daemon, &DaemonService::quitRequested, &app, &QApplication::quit);
    if (!daemon.registerOnSessionBus()) {
        qWarning("Exporting org.denkzettel.Daemon failed; the D-Bus entry points are unavailable.");
    }

    // The application name is back to "denkzettel" here, so the configuration
    // is denkzettelrc and not Daemonrc. Store::open() above has created data
    // directory and database; this completes the first start of SPEC 2.5.
    KConfigGroup general(KSharedConfig::openConfig(), QStringLiteral("General"));
    const bool firstRun = runFirstStart(general);

    GlobalShortcuts shortcuts;
    QObject::connect(&shortcuts, &GlobalShortcuts::captureRequested, &capture, &CaptureWindow::showCapture);
    const QList<ShortcutOwner> conflicts = shortcuts.registerCaptureShortcut();
    if (firstRun && !conflicts.isEmpty()) {
        notifyShortcutConflict(conflicts);
    }

    return app.exec();
}
