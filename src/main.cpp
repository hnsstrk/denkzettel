#include "shell/trayicon.h"

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
    // derive from it once the store lands.
    app.setApplicationName(QStringLiteral("Daemon"));
    KDBusService service(KDBusService::Unique);
    app.setApplicationName(QStringLiteral("denkzettel"));

    TrayIcon tray;

    return app.exec();
}
