/**
 * Vorprüfung #83, Bearbeiter B — eine Frage, gegen die SPEC 3.1 steht.
 *
 * AK 6 von #83 sichert zu: „Die Flächenfarbe folgt weiterhin dem Farbschema."
 * Die Messgrundlage des Issues (native-ak2-kontrast.txt) prüft das über
 * 20 Farbschemata — aber unter **einem** Desktop-Theme (`default`).
 * SPEC 3.1 sagt dagegen, gemessen am 01.08.2026: „Von den acht auf der
 * Kundenmaschine installierten Themes richtet nur `default` seine Füllfarbe am
 * Farbschema aus; ein Fenster in Theme-Farben stünde bei sieben von acht dunkel
 * auf dunkel."
 *
 * Diese Sonde dreht die Achse um: **ein** Schema, alle installierten Themes.
 * Gemessen wird der Pixel in der Mitte der von `framePixmap()` gezeichneten
 * Fläche gegen `QPalette::Window` des Schemas.
 *
 * Aufruf: themefarbe   (offscreen genügt — der Weg ist laut Messung des
 *         Issues plattformunabhängig)
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QDir>
#include <QGuiApplication>
#include <QPalette>
#include <QImage>
#include <QTextStream>
#include <QtMath>

namespace {

QString rgb(const QColor &c)
{
    return QStringLiteral("%1,%2,%3").arg(c.red(), 3).arg(c.green(), 3).arg(c.blue(), 3);
}

qreal luminance(const QColor &c)
{
    const auto channel = [](qreal v) {
        return v <= 0.03928 ? v / 12.92 : qPow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) + 0.0722 * channel(c.blueF());
}

qreal contrast(const QColor &a, const QColor &b)
{
    const qreal la = luminance(a);
    const qreal lb = luminance(b);
    return (qMax(la, lb) + 0.05) / (qMin(la, lb) + 0.05);
}

}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    // Das Schema kommt über die Anwendungspalette, nicht über eine übergebene
    // .colors-Datei — KSvg färbt seine Grafik nach der Palette der Anwendung.
    // Gesetzt wird es deshalb von außen über XDG_CONFIG_HOME + kdeglobals, wie
    // in `pruefen.sh` der Abnahme-Untersuchung (Zeilen 85-88).
    const QColor window = QGuiApplication::palette().color(QPalette::Window);
    const QColor windowText = QGuiApplication::palette().color(QPalette::WindowText);

    out << "=== #83 AK 6 gegen SPEC 3.1: ein Schema, alle Themes ===\n";
    out << "Plattform : " << QGuiApplication::platformName() << "\n";
    out << "PLATFORMTHEME : " << qEnvironmentVariable("QT_QPA_PLATFORMTHEME") << "\n";
    out << "XDG_CONFIG_HOME: " << qEnvironmentVariable("XDG_CONFIG_HOME") << "\n";
    out << "Palette   : Window " << rgb(window) << "   WindowText " << rgb(windowText) << "\n\n";

    QStringList themes;
    for (const QString &wurzel : {QStringLiteral("/usr/share/plasma/desktoptheme"),
                                  QDir::homePath() + QStringLiteral("/.local/share/plasma/desktoptheme")}) {
        const QDir dir(wurzel);
        for (const QString &name : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            if (!themes.contains(name)) {
                themes << name;
            }
        }
    }

    out << QStringLiteral("%1|%2|%3|%4|%5|%6\n")
               .arg(QStringLiteral("Desktop-Theme"), -22)
               .arg(QStringLiteral("Fläche gezeichnet"), -18)
               .arg(QStringLiteral("Alph/min"), -8)
               .arg(QStringLiteral("= Window?"), -10)
               .arg(QStringLiteral("dRGB"), -5)
               .arg(QStringLiteral("Text auf Fläche"), -16);
    out << QString(90, u'-') << "\n";

    for (const QString &name : std::as_const(themes)) {
        KSvg::ImageSet set(name, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg rahmen;
        rahmen.setImageSet(&set);
        rahmen.setImagePath(QStringLiteral("dialogs/background"));
        rahmen.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        rahmen.resizeFrame(QSizeF(600, 174));

        if (!rahmen.isValid()) {
            out << QStringLiteral("%1| keine dialogs/background-Grafik\n").arg(name, -22);
            continue;
        }

        const QImage bild = rahmen.framePixmap().toImage();
        const QColor mitte = bild.pixelColor(bild.width() / 2, bild.height() / 2);
        // Kleinste Deckung über die ganze Fläche — nicht nur in der Mitte.
        int minAlpha = 255;
        for (int y = 20; y < bild.height() - 20; y += 7) {
            for (int x = 20; x < bild.width() - 20; x += 7) {
                minAlpha = qMin(minAlpha, bild.pixelColor(x, y).alpha());
            }
        }
        const int abstand = qMax(qMax(qAbs(mitte.red() - window.red()), qAbs(mitte.green() - window.green())),
                                 qAbs(mitte.blue() - window.blue()));

        out << QStringLiteral("%1|%2|%3|%4|%5|%6\n")
                   .arg(name, -22)
                   .arg(rgb(mitte), -18)
                   .arg(QStringLiteral("%1/%2").arg(mitte.alpha()).arg(minAlpha), -8)
                   .arg(abstand == 0 ? QStringLiteral("gleich") : QStringLiteral("ABWEICHUNG"), -10)
                   .arg(abstand, -5)
                   .arg(QStringLiteral("%1:1").arg(contrast(mitte, windowText), 0, 'f', 2), -16);
    }

    out << "\nLesart: Spalte „= Window?" << "\" vergleicht die vom Theme gezeichnete Fläche mit\n"
           "der Schemafarbe `Window`. Nur wo sie gleich ist, trägt AK 6 von #83.\n"
           "Die letzte Spalte rechnet deckend (bester Fall); durchscheinend liegt sie tiefer.\n";
    return 0;
}
