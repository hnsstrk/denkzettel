// Sonde zur Vorprüfung von Issue #61 — kein Produktivcode.
//
// Frage: Welchen Namen meldet KDBusService an, wenn KAboutData::setApplicationData
// vor ihm gelaufen ist? Gemessen wird der tatsächlich angemeldete Busname,
// nicht die Ableitung aus der Kopfdatei.
//
// DZ_MODUS=ohne   — die Reihenfolge von src/main.cpp heute
// DZ_MODUS=mit    — KAboutData davor, so wie eine naive Umsetzung es täte
// DZ_MODUS=heilung— KAboutData davor, aber Domäne und Desktop-Datei nachgesetzt

#include <KAboutData>
#include <KDBusService>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QTextStream>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextStream aus(stdout);

    app.setOrganizationDomain(QStringLiteral("denkzettel.org"));
    app.setApplicationName(QStringLiteral("denkzettel"));
    app.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));

    const QByteArray modus = qgetenv("DZ_MODUS");
    if (modus != "ohne") {
        KAboutData about(QStringLiteral("denkzettel"), QStringLiteral("Denkzettel"),
                         QStringLiteral(DENKZETTEL_VERSION));
        if (modus == "heilung") {
            about.setOrganizationDomain(QByteArrayLiteral("denkzettel.org"));
            about.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));
        }
        KAboutData::setApplicationData(about);
    }

    aus << "Modus: " << QString::fromLatin1(modus) << '\n'
        << "  organizationDomain vor KDBusService: '" << QCoreApplication::organizationDomain() << "'\n"
        << "  desktopFileName    vor KDBusService: '" << QGuiApplication::desktopFileName() << "'\n";
    aus.flush();

    // Die Namensakrobatik aus src/main.cpp:35-42, wörtlich.
    app.setApplicationName(QStringLiteral("Daemon"));
    KDBusService service(KDBusService::Unique);
    app.setApplicationName(QStringLiteral("denkzettel"));

    const QStringList namen = QDBusConnection::sessionBus().interface()->registeredServiceNames().value();
    aus << "  angemeldete Busnamen (gefiltert): ";
    for (const QString &n : namen) {
        if (n.contains(QLatin1String("denkzettel"), Qt::CaseInsensitive)
            || n.contains(QLatin1String("Daemon"), Qt::CaseInsensitive)) {
            aus << n << ' ';
        }
    }
    aus << "\n  SPEC 2.3 verlangt: org.denkzettel.Daemon\n";
    aus.flush();

    QTimer::singleShot(0, &app, &QCoreApplication::quit);
    return app.exec();
}
