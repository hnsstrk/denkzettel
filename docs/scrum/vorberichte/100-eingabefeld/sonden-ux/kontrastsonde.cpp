/*
 * Herkunft von `frameContrast` (Zeichnung 3a, Zeilen 553 und 565).
 *
 * Die Zeichnung nennt den Wert „frameContrast des Farbschemas". Geprüft wird
 * hier, woher `KColorScheme::frameContrast()` ihn tatsächlich nimmt: mit einer
 * .colors-Datei als Konfiguration, ohne Argument, und gegen den Wert, den die
 * Sonde des Lesbarkeits-Reviews aus der Gruppe [General] der .colors-Datei las.
 *
 * Aufruf: kontrastsonde <Pfad zur .colors> …
 */
#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QApplication>
#include <QFileInfo>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    out << "Voreinstellung ohne Konfiguration: KColorScheme::frameContrast() = "
        << KColorScheme::frameContrast() << "\n\n";
    out << QStringLiteral("%1 %2 %3 %4\n")
               .arg(QStringLiteral("Schema"), -24)
               .arg(QStringLiteral("frameContrast()"), 16)
               .arg(QStringLiteral("[General]"), 11)
               .arg(QStringLiteral("Schluessel da?"), 15);

    int mitSchluessel = 0;
    int zeilen = 0;
    for (int i = 1; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
        const KConfigGroup general(scheme, QStringLiteral("General"));
        const bool da = general.hasKey("frameContrast")
            || KConfigGroup(scheme, QStringLiteral("KDE")).hasKey("frameContrast");
        if (da) {
            ++mitSchluessel;
        }
        ++zeilen;
        out << QStringLiteral("%1 %2 %3 %4\n")
                   .arg(QFileInfo(path).baseName(), -24)
                   .arg(KColorScheme::frameContrast(scheme), 16, 'f', 3)
                   .arg(general.readEntry("frameContrast", 0.20), 11, 'f', 3)
                   .arg(da ? QStringLiteral("ja") : QStringLiteral("nein"), 15);
    }
    out << "\n" << mitSchluessel << " von " << zeilen
        << " Schemata tragen den Schluessel (in [General] oder [KDE]).\n";
    return 0;
}
