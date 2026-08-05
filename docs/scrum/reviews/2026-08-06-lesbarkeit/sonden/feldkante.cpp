/*
 * Dritte Messsonde vom 06.08.2026, zum Erfassungsfenster (Issue #100):
 * Welche Kante bekäme das Textfeld, wenn sie aus derselben Quelle käme wie
 * die Hülle?
 *
 * Der Maßstab des Kunden ist KRunner. KRunners Eingabefeld ist ein
 * `KSvg.FrameSvgItem` mit `imagePath: "widgets/lineedit"`, `prefix: "base"`
 * (/usr/lib/qt6/qml/org/kde/plasma/components/TextField.qml:187–191). Die
 * Sonde zeichnet genau diese Grafik je Desktop-Theme und misst, was
 * dabei herauskommt:
 *
 *   1. Bringt das Theme `widgets/lineedit` überhaupt mit, oder fällt KSvg auf
 *      `default` zurück?
 *   2. Welche Fläche und welche Kante zeichnet der Vorsatz `base`?
 *   3. Wie stark heben sie sich von der Hülle ab, die dasselbe Theme über
 *      `dialogs/background` zeichnet — also von dem, worauf sie liegen?
 *   4. Was trüge der Notiztext danach: Er stünde nicht mehr auf der Hülle,
 *      sondern auf der Feldfläche. Beide Kontraste stehen daneben.
 *
 * Schreibt nichts, installiert nichts, liest keine Kundeneinstellung außer
 * dem Farbschema, das die Anwendungspalette ohnehin trägt.
 *
 * Aufruf: feldkante <Zielverzeichnis|-> <theme> [<theme> …]
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QFile>
#include <QTextStream>

#include <cmath>
#include <memory>

namespace
{
QTextStream out(stdout);

double channel(int c)
{
    const double v = c / 255.0;
    return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
}

double luminance(const QColor &c)
{
    return 0.2126 * channel(c.red()) + 0.7152 * channel(c.green()) + 0.0722 * channel(c.blue());
}

double contrast(const QColor &a, const QColor &b)
{
    const double la = luminance(a);
    const double lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

/** Die Fläche über einem Grund zusammengesetzt — das, was das Auge sieht. */
QColor over(const QColor &top, const QColor &ground)
{
    const double a = top.alpha() / 255.0;
    return QColor(qRound(a * top.red() + (1 - a) * ground.red()),
                  qRound(a * top.green() + (1 - a) * ground.green()),
                  qRound(a * top.blue() + (1 - a) * ground.blue()));
}

QString show(const QColor &c)
{
    return QStringLiteral("%1,%2,%3/a%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

QImage render(KSvg::FrameSvg &frame, const QSize &size)
{
    frame.resizeFrame(size);
    QImage picture(size, QImage::Format_ARGB32_Premultiplied);
    picture.fill(Qt::transparent);
    QPainter painter(&picture);
    frame.paintFrame(&painter);
    painter.end();
    return picture;
}

/** Der äusserste Bildpunkt, an dem die Grafik noch etwas zeichnet. */
QColor firstOpaqueFromLeft(const QImage &picture, int y)
{
    for (int x = 0; x < picture.width() / 2; ++x) {
        const QColor c = picture.pixelColor(x, y);
        if (c.alpha() > 0) {
            return c;
        }
    }
    return {};
}

/** Der kräftigste Bildpunkt der linken Kante — die Konturfarbe. */
QColor strongestOnLeftEdge(const QImage &picture, int y)
{
    QColor best;
    int bestAlpha = -1;
    for (int x = 0; x < picture.width() / 2; ++x) {
        const QColor c = picture.pixelColor(x, y);
        if (c.alpha() > bestAlpha) {
            bestAlpha = c.alpha();
            best = c;
        }
        if (c.alpha() > 0 && x > 0 && picture.pixelColor(x + 1, y).alpha() < c.alpha()) {
            break;
        }
    }
    return best;
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    if (argc < 3) {
        out << "Aufruf: feldkante <Zielverzeichnis|-> <theme> [<theme> …]\n";
        return 2;
    }
    const QString directory = QString::fromLocal8Bit(argv[1]);

    out << "Gerechnet wird deckend über einem benannten Grund, weil beide Grafiken\n"
           "durchscheinen können. Grund für die Hülle: mittleres Grau 128,128,128\n"
           "(derselbe benannte Grund wie in den Belegen zu #85). Grund für das Feld:\n"
           "die zusammengesetzte Hülle desselben Themes — dort liegt es ja.\n\n";

    const QColor namedGround(128, 128, 128);

    for (int i = 2; i < argc; ++i) {
        const QString theme = QString::fromLocal8Bit(argv[i]);

        auto set = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));

        KSvg::FrameSvg hull;
        hull.setImageSet(set.get());
        hull.setImagePath(QStringLiteral("dialogs/background"));
        hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        hull.setColorSet(KSvg::Svg::Window);

        KSvg::FrameSvg field;
        field.setImageSet(set.get());
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.setColorSet(KSvg::Svg::View);

        out << "############ " << theme << "\n";
        if (!hull.isValid()) {
            out << "  dialogs/background fehlt — übersprungen\n\n";
            continue;
        }
        if (!field.isValid()) {
            out << "  widgets/lineedit nicht auflösbar — übersprungen\n\n";
            continue;
        }

        out << "  eigene lineedit-Grafik: "
            << (QFile::exists(QStringLiteral("/usr/share/plasma/desktoptheme/%1/widgets/lineedit.svgz").arg(theme))
                        || QFile::exists(QStringLiteral("/usr/share/plasma/desktoptheme/%1/widgets/lineedit.svg").arg(theme))
                    ? "ja"
                    : "nein (Rückfall auf default)")
            << "\n";

        const QImage hullPicture = render(hull, QSize(600, 174));
        const QColor hullRaw = hullPicture.pixelColor(300, 87);
        const QColor hullOnGround = over(hullRaw, namedGround);

        const QImage fieldPicture = render(field, QSize(560, 60));
        const QColor fieldRaw = fieldPicture.pixelColor(280, 30);
        const QColor fieldOnHull = over(fieldRaw, hullOnGround);
        const QColor edgeRaw = strongestOnLeftEdge(fieldPicture, 30);
        const QColor edgeOnHull = over(edgeRaw, hullOnGround);
        const QColor firstRaw = firstOpaqueFromLeft(fieldPicture, 30);

        // Ränder, die die Grafik für sich beansprucht — das Maß, um das der
        // Text nach innen rückte.
        qreal l = 0;
        qreal t = 0;
        qreal r = 0;
        qreal b = 0;
        field.getMargins(l, t, r, b);

        out << "  Hülle   Mitte roh " << show(hullRaw) << "  über Grau " << show(hullOnGround) << "\n";
        out << "  Feld    Mitte roh " << show(fieldRaw) << "  über Hülle " << show(fieldOnHull) << "\n";
        out << "  Kante   roh       " << show(edgeRaw) << "  über Hülle " << show(edgeOnHull)
            << "  (erster deckender Punkt " << show(firstRaw) << ")\n";
        out << QStringLiteral("  Ränder der Feldgrafik: links %1 oben %2 rechts %3 unten %4\n")
                   .arg(l)
                   .arg(t)
                   .arg(r)
                   .arg(b);
        out << QStringLiteral("  Feldfläche gegen Hülle : %1:1\n")
                   .arg(contrast(fieldOnHull, hullOnGround), 0, 'f', 2);
        out << QStringLiteral("  Feldkante  gegen Hülle : %1:1\n")
                   .arg(contrast(edgeOnHull, hullOnGround), 0, 'f', 2);

        // Was der Text danach trüge. Die beiden Farben kommen aus derselben
        // Quelle wie die Flächen (Entscheidung #85): der Textfarbe des
        // Theme-Farbsatzes für Fenster beziehungsweise für Ansichten.
        KSvg::Svg colours;
        colours.setImageSet(set.get());
        colours.setImagePath(QStringLiteral("dialogs/background"));
        const QColor windowText = colours.color(KSvg::Svg::Text);
        const QColor viewText = colours.color(KSvg::Svg::ViewText);
        out << QStringLiteral("  Text auf der Hülle heute (Window-Text %1)      : %2:1\n")
                   .arg(show(windowText))
                   .arg(contrast(windowText, hullOnGround), 0, 'f', 2);
        out << QStringLiteral("  Text auf der Feldfläche danach (View-Text %1)  : %2:1\n")
                   .arg(show(viewText))
                   .arg(contrast(viewText, fieldOnHull), 0, 'f', 2);
        out << "\n";

        if (directory != QLatin1String("-")) {
            QImage sheet(600, 174, QImage::Format_ARGB32_Premultiplied);
            sheet.fill(namedGround);
            QPainter painter(&sheet);
            painter.drawImage(0, 0, hullPicture);
            painter.drawImage(20, 45, fieldPicture);
            painter.end();
            sheet.save(QStringLiteral("%1/feldkante-%2.png").arg(directory, theme));
        }
    }

    return 0;
}
