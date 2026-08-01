// Achse 3 / Gestaltung: Woher kaeme die Huelle eines rahmenlosen Fensters?
// Geprueft wird die Huelle, die Plasma-Popups und KRunner benutzen —
// dialogs/background aus dem Desktop-Theme des Nutzers.
//
// Drei Fragen, die die Zeichnung tragen muss:
//  1. Wie gross sind die Raender, die das Theme vorgibt? (Innenabstand)
//  2. Bringt das Theme Schattenteile mit? (Schatten-Zusicherung)
//  3. Faerbt sich die Huelle nach dem aktiven Farbschema? (passt Text darauf?)
//
// Aufruf: themehuelle-probe <Zielverzeichnis> <Pfad zur .colors> <Theme-Name> …

#include <KColorScheme>
#include <KSharedConfig>

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include <cstdio>
#include <map>

namespace
{
/** Haeufigste Farbe der Flaechenmitte — die Fuellfarbe der Huelle. */
QColor fuellfarbe(const QPixmap &huelle)
{
    const QImage bild = huelle.toImage();
    std::map<QRgb, int> zaehler;
    for (int y = bild.height() / 3; y < 2 * bild.height() / 3; ++y) {
        for (int x = bild.width() / 3; x < 2 * bild.width() / 3; ++x) {
            ++zaehler[bild.pixel(x, y)];
        }
    }
    QRgb haeufigste = 0;
    int meiste = 0;
    for (const auto &[wert, anzahl] : zaehler) {
        if (anzahl > meiste) {
            meiste = anzahl;
            haeufigste = wert;
        }
    }
    return QColor::fromRgb(haeufigste);
}
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("Aufruf: themehuelle-probe <Zielverzeichnis> <Pfad zur .colors> <Theme-Name> …\n");
        return 2;
    }

    QGuiApplication app(argc, argv);
    const QString directory = QString::fromLocal8Bit(argv[1]);
    const QString schemaPfad = QString::fromLocal8Bit(argv[2]);
    const QString schemaName = schemaPfad.section(QLatin1Char('/'), -1).section(QLatin1Char('.'), 0, 0);
    const QSizeF fenster(600, 155); // Groesse des Capture-Fensters

    app.setPalette(KColorScheme::createApplicationPalette(
        KSharedConfig::openConfig(schemaPfad, KConfig::SimpleConfig)));
    printf("Farbschema der Anwendung: %s  (Fenstergrund %s)\n\n",
           qPrintable(schemaName),
           qPrintable(app.palette().color(QPalette::Active, QPalette::Window).name()));

    for (int i = 3; i < argc; ++i) {
        const QString themeName = QString::fromLocal8Bit(argv[i]);

        KSvg::ImageSet imageSet(themeName, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg frame;
        frame.setImageSet(&imageSet);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.resizeFrame(fenster);

        const QPixmap huelle = frame.framePixmap();
        printf("%-24s Raender l/r/o/u = %.0f/%.0f/%.0f/%.0f px   Schattenteile: %-5s Fuellfarbe %s\n",
               qPrintable(themeName),
               frame.marginSize(KSvg::FrameSvg::LeftMargin),
               frame.marginSize(KSvg::FrameSvg::RightMargin),
               frame.marginSize(KSvg::FrameSvg::TopMargin),
               frame.marginSize(KSvg::FrameSvg::BottomMargin),
               frame.hasElementPrefix(QStringLiteral("shadow")) ? "ja" : "nein",
               qPrintable(fuellfarbe(huelle).name()));

        // Auf kariertem Grund, damit Rundung und Durchscheinung sichtbar werden.
        QPixmap blatt(huelle.size());
        QPainter maler(&blatt);
        for (int y = 0; y < blatt.height(); y += 12) {
            for (int x = 0; x < blatt.width(); x += 12) {
                const bool hell = ((x / 12) + (y / 12)) % 2 == 0;
                maler.fillRect(x, y, 12, 12, hell ? QColor(210, 210, 214) : QColor(184, 184, 190));
            }
        }
        maler.drawPixmap(0, 0, huelle);
        maler.end();
        blatt.save(QStringLiteral("%1/achse3-huelle-%2-%3.png").arg(directory, themeName, schemaName));
    }

    return 0;
}
