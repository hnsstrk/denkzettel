// Sonde zur Vorprüfung von Issue #61 — kein Produktivcode.
//
// Misst die drei Wege, auf denen eine Versionsnummer in `denkzetteld --version`
// landen könnte, und was sie unterwegs mit den Eigenschaften anstellen, die
// src/main.cpp heute von Hand setzt (SPEC 2.3 Busname, SPEC 2.4 Kürzel-
// Komponente).
//
// Der Modus kommt aus der Umgebungsvariablen DZ_MODUS:
//   roh     — nur QApplication, so wie main.cpp es heute tut
//   kf6     — KAboutData + setupCommandLine + parser.process()
//   kf6lax  — KAboutData + setupCommandLine + parser.parse() (nachsichtig)
//
// Gebaut wird gegen dieselbe Definition, die eine Umsetzung setzen könnte:
// DENKZETTEL_VERSION als Präprozessorzeichenkette aus PROJECT_VERSION.

#include <KAboutData>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>

static void zeigeEigenschaften(const char *wann)
{
    QTextStream aus(stdout);
    aus << "  [" << wann << "]\n"
        << "    applicationName ...... '" << QCoreApplication::applicationName() << "'\n"
        << "    applicationVersion ... '" << QCoreApplication::applicationVersion() << "'\n"
        << "    organizationDomain ... '" << QCoreApplication::organizationDomain() << "'\n"
        << "    applicationDisplayName '" << QGuiApplication::applicationDisplayName() << "'\n"
        << "    desktopFileName ...... '" << QGuiApplication::desktopFileName() << "'\n";
    aus.flush();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTextStream aus(stdout);
    aus << "DENKZETTEL_VERSION (Präprozessor) = '" << QStringLiteral(DENKZETTEL_VERSION) << "'\n";
    aus << "QCoreApplication::arguments() = " << QCoreApplication::arguments().join(QLatin1Char('|')) << "\n";

    // Genau die Zeilen aus src/main.cpp:21-33, unverändert übernommen.
    app.setOrganizationDomain(QStringLiteral("denkzettel.org"));
    app.setApplicationName(QStringLiteral("denkzettel"));
    app.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    zeigeEigenschaften("nach den Zeilen aus main.cpp");

    const QByteArray modus = qgetenv("DZ_MODUS");
    if (modus == "roh") {
        aus << "Modus roh: keine Optionsauswertung. Der Dienst liefe jetzt weiter.\n";
        return 0;
    }

    KAboutData about(QStringLiteral("denkzettel"),
                     QStringLiteral("Denkzettel"),
                     QStringLiteral(DENKZETTEL_VERSION));
    KAboutData::setApplicationData(about);
    zeigeEigenschaften("nach KAboutData::setApplicationData");

    QCommandLineParser parser;
    const bool eingerichtet = about.setupCommandLine(&parser);
    aus << "setupCommandLine() = " << (eingerichtet ? "true" : "false") << "\n";
    aus << "bekannte Optionen: " << parser.helpText().split(QLatin1Char('\n')).join(QLatin1Char('/')) << "\n";
    aus.flush();

    if (modus == "kf6lax") {
        const bool ok = parser.parse(QCoreApplication::arguments());
        aus << "parse() = " << (ok ? "true" : "false")
            << "  Fehler: '" << parser.errorText() << "'\n";
        aus << "isSet(version) = " << (parser.isSet(QStringLiteral("version")) ? "true" : "false") << "\n";
        aus.flush();
        if (parser.isSet(QStringLiteral("version"))) {
            aus << QCoreApplication::applicationName() << ' ' << QCoreApplication::applicationVersion() << '\n';
            aus.flush();
            return 0;
        }
        zeigeEigenschaften("nach parse()");
        return 0;
    }

    parser.process(app);   // beendet den Prozess bei --version/--help selbst
    about.processCommandLine(&parser);
    zeigeEigenschaften("nach process()");
    aus << "process() hat nicht beendet — es war weder --version noch --help.\n";
    return 0;
}
