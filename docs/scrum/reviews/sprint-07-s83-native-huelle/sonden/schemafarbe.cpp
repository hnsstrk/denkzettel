/**
 * Messsonde 4 zu #83 — folgt die Flächenfarbe dem Farbschema? (AK 8)
 *
 * Unter dem Desktop-Theme `default` soll die Fläche der Hülle die Farbe
 * `Window` des eingestellten Farbschemas tragen, Toleranz ein Zählschritt je
 * Kanal. Gemessen wird über **alle** installierten Farbschemata, je Schema mit
 * einem eigenen Prozess — ein Farbschema wird beim Start der Anwendung
 * aufgelöst, ein laufender Prozess kann es nicht wechseln.
 *
 * Die Toleranz ist kein Nachlass: Die eigene Messgrundlage der Story weicht bei
 * 3 von 19 Schemata um einen Schritt ab (`native-ak2-kontrast.txt`).
 *
 * Aufruf: schemafarbe [Themename]   (das Farbschema kommt aus der Umgebung,
 *         die `pruefen.sh` je Lauf über ein eigenes XDG_CONFIG_HOME setzt)
 */

#include <KColorScheme>

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QImage>
#include <QTextStream>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QString theme = app.arguments().value(1, QStringLiteral("default"));

    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    frame.setColorSet(KSvg::Svg::Window);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(600, 174));

    if (!frame.isValid()) {
        out << "ungültig\n";
        return 1;
    }

    const QImage image = frame.framePixmap().toImage();
    const QColor fill = image.pixelColor(image.width() / 2, image.height() / 2);
    const KColorScheme scheme(QPalette::Active, KColorScheme::Window);
    const QColor window = scheme.background(KColorScheme::NormalBackground).color();

    const int worst = qMax(qAbs(fill.red() - window.red()),
                           qMax(qAbs(fill.green() - window.green()),
                                qAbs(fill.blue() - window.blue())));

    out << QStringLiteral("%1  Fläche %2,%3,%4 / Alpha %5   Window %6,%7,%8   Abweichung %9  %10\n")
               .arg(qEnvironmentVariable("DENKZETTEL_SCHEMA", QStringLiteral("(unbenannt)")), -28)
               .arg(fill.red(), 3)
               .arg(fill.green(), 3)
               .arg(fill.blue(), 3)
               .arg(fill.alpha(), 3)
               .arg(window.red(), 3)
               .arg(window.green(), 3)
               .arg(window.blue(), 3)
               .arg(worst, 3)
               .arg(worst <= 1 ? QStringLiteral("innerhalb der Toleranz")
                               : QStringLiteral("**ÜBER DER TOLERANZ**"));

    return worst <= 1 ? 0 : 2;
}
