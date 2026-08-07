// Kleine Nebensonde: In welcher Gruppe und unter welchem Schlüssel liest
// `KColorScheme::frameContrast()` seinen Wert, und cacht es?
//
// Anlass: Alle 18 installierten Schemata tragen denselben Wert 0,20 — die
// Voreinstellung. Ein Gegenversuch mit einem ausgedachten Wert ergab weiterhin
// 0,20. Bevor daraus ein Befund wird, muss geklärt sein, ob die Abfrage
// woanders liest oder ihren ersten Wert festhält.
//
// Aufruf: kontrastwert

#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QApplication>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <cstdio>

int main(int argc, char **argv)
{
    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);

    const QString path = sandbox.filePath(QStringLiteral("Probe.colors"));
    {
        KSharedConfigPtr file = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
        KConfigGroup(file, QStringLiteral("General")).writeEntry("frameContrast", 0.45);
        KConfigGroup(file, QStringLiteral("KDE")).writeEntry("frameContrast", 0.55);
        file->sync();
    }

    KSharedConfigPtr file = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    printf("Erster Aufruf überhaupt, mit Schemadatei: %.4f\n", KColorScheme::frameContrast(file));
    printf("  gegengelesen [General] frameContrast : %.4f\n",
           KConfigGroup(file, QStringLiteral("General")).readEntry("frameContrast", -1.0));
    printf("  gegengelesen [KDE] frameContrast     : %.4f\n",
           KConfigGroup(file, QStringLiteral("KDE")).readEntry("frameContrast", -1.0));
    printf("Ohne Schemadatei                       : %.4f\n", KColorScheme::frameContrast());

    // Und der Weg, den ein Programm im Betrieb nimmt: die Anwendungs-
    // konfiguration. Beide Gruppen werden beschrieben, damit die Ausgabe
    // zeigt, welche gilt.
    {
        KSharedConfigPtr globals = KSharedConfig::openConfig();
        KConfigGroup(globals, QStringLiteral("General")).writeEntry("frameContrast", 0.30);
        KConfigGroup(globals, QStringLiteral("KDE")).writeEntry("frameContrast", 0.40);
        globals->sync();
    }
    printf("\nNach Eintrag in die Anwendungskonfiguration ([General] 0,30 · [KDE] 0,40):\n");
    printf("Ohne Schemadatei                       : %.4f\n", KColorScheme::frameContrast());

    printf("\nTragen die installierten Schemadateien den Schlüssel überhaupt?\n");
    for (int i = 1; i < argc; ++i) {
        const QString candidate = QString::fromLocal8Bit(argv[i]);
        KSharedConfigPtr scheme = KSharedConfig::openConfig(candidate, KConfig::SimpleConfig);
        const double general = KConfigGroup(scheme, QStringLiteral("General")).readEntry("frameContrast", -1.0);
        const double kde = KConfigGroup(scheme, QStringLiteral("KDE")).readEntry("frameContrast", -1.0);
        if (general >= 0 || kde >= 0) {
            printf("  %s: [General] %.4f · [KDE] %.4f\n", qPrintable(candidate), general, kde);
        }
    }
    printf("  (nur Dateien mit dem Schlüssel stehen oben; keine Zeile heißt: keine trägt ihn)\n");

    return 0;
}
