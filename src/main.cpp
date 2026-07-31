#include "capture/capturewindow.h"
#include "shell/daemonservice.h"
#include "shell/trayicon.h"
#include "store/store.h"

#include <KDBusService>
#include <KLocalizedString>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationDomain(QStringLiteral("denkzettel.org"));
    app.setApplicationName(QStringLiteral("denkzettel"));
    app.setQuitOnLastWindowClosed(false);

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

    TrayIcon tray;
    QObject::connect(&tray, &TrayIcon::captureRequested, &capture, &CaptureWindow::showCapture);

    DaemonService daemon;
    QObject::connect(&daemon, &DaemonService::captureRequested, &capture, &CaptureWindow::showCapture);
    if (!daemon.registerOnSessionBus()) {
        qWarning("Exporting org.denkzettel.Daemon failed; the D-Bus entry points are unavailable.");
    }

    return app.exec();
}
