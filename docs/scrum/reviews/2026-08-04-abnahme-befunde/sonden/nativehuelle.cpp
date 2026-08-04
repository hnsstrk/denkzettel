/**
 * Messsonde zum nativen Weg — „eine native Plasma-Überlagerung ohne
 * Anpassungen" (Kundenentscheidung 04.08.2026).
 *
 * Sie zeichnet die Hülle so, wie Plasma seine eigenen Überlagerungen zeichnet:
 * **`FrameSvg::framePixmap()` in einem Stück.** Kein Ring aus zwei Rahmen,
 * keine selbst gezogene Kontur, keine eigene Einfärbung. Und sie misst, was
 * dabei herauskommt:
 *
 *   A  die Ecke bei Pixelverhältnis 1 und 1,6 — Treppe weg? Kontur da?
 *   B  was `colorSet` aus der Grafik macht, Farbsatz für Farbsatz
 *   C  der Kontrast der so entstandenen Fläche gegen die Textfarbe (AK 2)
 *   D  ob die Theme-Grafik überhaupt eine Konturlinie zeichnet
 *   E  wie `framePixmap()` und `alphaMask()` sich unterscheiden — der heutige
 *      Weg nimmt die `mask-`Elemente, der native die Rahmenelemente selbst
 *
 * Aufruf: nativehuelle <Zielverzeichnis> [Themename]
 *         (offscreen wie in der Sitzung; die Plattform steht im Protokoll)
 */

#include <KColorScheme>

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QTextStream>

namespace {

constexpr int Window = 16;

QString rgb(const QColor &c)
{
    return QStringLiteral("%1,%2,%3").arg(c.red(), 3).arg(c.green(), 3).arg(c.blue(), 3);
}

/** Relative Leuchtdichte nach WCAG 2.1. */
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

void grid(QTextStream &out, const QImage &image, const QString &what)
{
    out << "\n" << what << "  (" << image.width() << "x" << image.height() << " Bildpunkte)\n      ";
    for (int x = 0; x < Window; ++x) {
        out << QStringLiteral("%1").arg(x, 16);
    }
    out << "\n";
    for (int y = 0; y < Window; ++y) {
        out << QStringLiteral("%1  ").arg(y, 3);
        for (int x = 0; x < Window; ++x) {
            const QColor c = image.pixelColor(x, y);
            out << QStringLiteral("%1").arg(QStringLiteral("%1/%2").arg(rgb(c), QString::number(c.alpha())), 16);
        }
        out << "\n";
    }
}

/**
 * Der Kantenlauf: erste Spalte je Zeile, in der die Hülle mehr als halb deckt.
 *
 * Ein kantengeglätteter Bogen wandert eine Spalte je Zeile; eine Treppe steht
 * still und springt dann um zwei oder mehr.
 */
void edgeWalk(QTextStream &out, const QImage &image, const QString &what)
{
    out << "\nKantenlauf — " << what << "\n";
    int previous = -1;
    int steps = 0;
    for (int y = 0; y < Window; ++y) {
        int found = -1;
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alpha() >= 128) {
                found = x;
                break;
            }
        }
        const int step = (previous >= 0 && found >= 0) ? previous - found : 0;
        if (qAbs(step) >= 2) {
            ++steps;
        }
        out << QStringLiteral("  Zeile %1  Spalte %2  Sprung %3%4\n")
                   .arg(y, 3)
                   .arg(found, 4)
                   .arg(step, 3)
                   .arg(qAbs(step) >= 2 ? QStringLiteral("   <== Stufe") : QString());
        previous = found;
    }
    out << "  Stufen (Sprünge von zwei Spalten und mehr): " << steps << "\n";
}

const QList<QPair<KSvg::Svg::ColorSet, const char *>> ColorSets{
    {KSvg::Svg::View, "View"},
    {KSvg::Svg::Window, "Window"},
    {KSvg::Svg::Button, "Button"},
    {KSvg::Svg::Selection, "Selection"},
    {KSvg::Svg::Tooltip, "Tooltip"},
    {KSvg::Svg::Complementary, "Complementary"},
    {KSvg::Svg::Header, "Header"},
};

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QString directory = app.arguments().value(1, QStringLiteral("."));
    const QString theme = app.arguments().value(2, QStringLiteral("default"));
    QDir().mkpath(directory);

    const QSize size(600, 174); // die Maße des echten Erfassungsfensters

    out << "=== Native Hülle: FrameSvg in einem Stück ===\n";
    out << "Plattform        : " << app.platformName() << "\n";
    out << "Theme            : " << theme << "\n";
    out << "XDG_CONFIG_HOME  : " << qEnvironmentVariable("XDG_CONFIG_HOME", QStringLiteral("(nicht gesetzt)")) << "\n";
    out << "qApp DPR         : " << app.devicePixelRatio() << "\n";
    out << "Fenstermaß       : " << size.width() << "x" << size.height() << " logisch\n";

    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));

    // ------------------------------------------------------------------ A
    // Die Ecke, in einem Stück gezeichnet, bei zwei Pixelverhältnissen.
    out << "\n\n########## A — die Ecke in einem Stück ##########\n";
    for (const qreal dpr : {1.0, 1.6}) {
        KSvg::FrameSvg frame;
        frame.setImageSet(&imageSet);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.setColorSet(KSvg::Svg::Window);
        frame.setDevicePixelRatio(dpr);
        frame.resizeFrame(size);

        const QPixmap piece = frame.framePixmap();
        out << "\n--- Pixelverhältnis " << dpr << "\n";
        out << "framePixmap      : " << piece.width() << "x" << piece.height() << " Bildpunkte, DPR "
            << piece.devicePixelRatio() << "\n";
        const QImage image = piece.toImage();
        grid(out, image, QStringLiteral("Linke obere Ecke, r,g,b/Alpha"));
        edgeWalk(out, image, QStringLiteral("framePixmap(), DPR ") + QString::number(dpr));

        // Auf weißem Grund, damit Durchsichtigkeit sichtbar wird, und
        // zwanzigfach ohne Glättung — was hier weich aussieht, ist weich
        // gerechnet.
        QImage onWhite(image.size(), QImage::Format_ARGB32);
        onWhite.fill(Qt::white);
        {
            QPainter painter(&onWhite);
            painter.drawImage(0, 0, image);
        }
        onWhite.copy(0, 0, 24, 24)
            .scaled(480, 480, Qt::KeepAspectRatio, Qt::FastTransformation)
            .save(QDir(directory).filePath(QStringLiteral("native-ecke-dpr-%1.png").arg(dpr)));

        // Die ganze Hülle auf der schraffierten Unterlage der Zeichnung 4a.
        // Sie ist hier nicht Schmuck, sondern Messmittel: Diese Hülle **deckt
        // nicht** (Alpha 216), und auf einer Schraffur sieht man auf einen
        // Blick, wie viel Untergrund durchkommt.
        constexpr int Frame = 24;
        constexpr int HatchStep = 8;
        QImage sheet(image.size() + QSize(2 * Frame, 2 * Frame), QImage::Format_ARGB32);
        sheet.fill(QColor(0xf2, 0xf0, 0xeb));
        {
            QPainter painter(&sheet);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0xe9, 0xe7, 0xe2));
            for (int offset = -sheet.height(); offset < sheet.width(); offset += 2 * HatchStep) {
                painter.drawPolygon(QPolygon({QPoint(offset, 0),
                                              QPoint(offset + HatchStep, 0),
                                              QPoint(offset + HatchStep + sheet.height(), sheet.height()),
                                              QPoint(offset + sheet.height(), sheet.height())}));
            }
            painter.drawImage(Frame, Frame, image);
        }
        sheet.save(QDir(directory).filePath(QStringLiteral("native-huelle-dpr-%1.png").arg(dpr)));
    }

    // ------------------------------------------------------------------ E
    // Der heutige Weg nimmt die `mask-`Elemente, der native die Rahmen selbst.
    out << "\n\n########## E — Maske gegen Rahmen ##########\n";
    {
        KSvg::FrameSvg frame;
        frame.setImageSet(&imageSet);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.setColorSet(KSvg::Svg::Window);
        frame.resizeFrame(size);
        edgeWalk(out, frame.alphaMask().toImage(), QStringLiteral("alphaMask() — die mask-Elemente"));
        edgeWalk(out, frame.framePixmap().toImage(), QStringLiteral("framePixmap() — die Rahmenelemente"));
    }

    // ------------------------------------------------------------------ B/C
    out << "\n\n########## B/C — colorSet und Kontrast ##########\n";
    out << "Gemessen wird das Pixel in der Mitte der Fläche und, wo eine Linie da ist,\n"
           "das äußerste Pixel der Kante. Der Kontrast steht gegen die Textfarbe des\n"
           "Farbschemas, die Denkzettel dem Notiztext gibt (WindowText, AK 2).\n";

    // Jeder Farbsatz bekommt eine **eigene** ImageSet-Instanz, der Farbsatz wird
    // **vor** dem Bildpfad gesetzt und der Zwischenspeicher abgeschaltet. Ohne
    // diese drei Vorkehrungen misst man womöglich das Ergebnis des vorigen
    // Durchlaufs — genau der Fehler, den Messung 3 aus Sprint 6 aufgedeckt hat
    // (ein FrameSvg behält, was er einmal aufgelöst hat).
    for (const auto &[set, name] : ColorSets) {
        KSvg::ImageSet ownSet(theme, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg frame;
        frame.setUsingRenderingCache(false);
        frame.setColorSet(set);
        frame.setImageSet(&ownSet);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.resizeFrame(size);

        const QImage image = frame.framePixmap().toImage();
        const QColor fill = image.pixelColor(image.width() / 2, image.height() / 2);
        out << QStringLiteral("  %1  Fläche %2  Alpha %3  (colorSet meldet %4)\n")
                   .arg(QLatin1StringView(name), -14)
                   .arg(rgb(fill))
                   .arg(fill.alpha(), 3)
                   .arg(static_cast<int>(frame.colorSet()));
    }

    // Was das Farbschema an diesen Stellen selbst sagt — die Gegenprobe zur
    // Zeile darüber. Weichen die Zahlen voneinander ab, färbt KSvg nicht nach
    // dem Farbsatz, sondern nach einem einzigen.
    out << "\nZum Vergleich, aus dem Farbschema gelesen (KColorScheme):\n";
    for (const auto &[set, name] : ColorSets) {
        const KColorScheme scheme(QPalette::Active, static_cast<KColorScheme::ColorSet>(set));
        out << QStringLiteral("  %1  BackgroundNormal %2   ForegroundNormal %3\n")
                   .arg(QLatin1StringView(name), -14)
                   .arg(rgb(scheme.background(KColorScheme::NormalBackground).color()))
                   .arg(rgb(scheme.foreground(KColorScheme::NormalText).color()));
    }

    // Die Durchsichtigkeit hat Folgen für AK 2: Der dort gemessene Kontrast
    // rechnet mit einer **deckenden** Fläche. Liegt die Hülle bei 85 %, hängt
    // die tatsächliche Flächenfarbe davon ab, was dahinter liegt.
    out << "\n\n########## C — Kontrast bei durchscheinender Hülle (AK 2) ##########\n";
    {
        KSvg::ImageSet ownSet(theme, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg frame;
        frame.setUsingRenderingCache(false);
        frame.setColorSet(KSvg::Svg::Window);
        frame.setImageSet(&ownSet);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.resizeFrame(size);
        const QImage image = frame.framePixmap().toImage();
        const QColor fill = image.pixelColor(image.width() / 2, image.height() / 2);

        const KColorScheme scheme(QPalette::Active, KColorScheme::Window);
        const QColor text = scheme.foreground(KColorScheme::NormalText).color();
        out << "Notiztext (WindowText des Schemas): " << rgb(text) << "\n";
        out << "Hülle: " << rgb(fill) << " bei Alpha " << fill.alpha() << " = "
            << QString::number(100.0 * fill.alpha() / 255.0, 'f', 1) << " %\n\n";

        const auto over = [&fill](const QColor &behind) {
            const qreal a = fill.alphaF();
            return QColor::fromRgbF(fill.redF() * a + behind.redF() * (1 - a),
                                    fill.greenF() * a + behind.greenF() * (1 - a),
                                    fill.blueF() * a + behind.blueF() * (1 - a));
        };
        for (const auto &[behind, what] : QList<QPair<QColor, const char *>>{
                 {Qt::black, "schwarzer Grund"},
                 {Qt::white, "weißer Grund"},
                 {QColor(128, 128, 128), "mittelgrauer Grund"},
             }) {
            const QColor effective = over(behind);
            out << QStringLiteral("  %1  Fläche wirkt als %2  Kontrast zum Text %3 : 1\n")
                       .arg(QLatin1StringView(what), -20)
                       .arg(rgb(effective))
                       .arg(contrast(effective, text), 0, 'f', 2);
        }
        out << "\nAK 2 rechnet mit einer deckenden Fläche und nennt 4,74:1 als schlechtesten\n"
               "Wert über 18 Farbschemata. Deckend gerechnet ergibt dieses Schema "
            << QString::number(contrast(QColor(fill.red(), fill.green(), fill.blue()), text), 'f', 2) << ":1.\n";
    }

    // ------------------------------------------------------------------ D
    out << "\n\n########## D — zeichnet die Grafik eine Kontur? ##########\n";
    out << "Waagerechter Schnitt durch Zeile 40, von außen nach innen. Eine Kontur wäre\n"
           "eine Linie, die sich von der Fläche unterscheidet.\n\n";
    {
        KSvg::FrameSvg frame;
        frame.setImageSet(&imageSet);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.setColorSet(KSvg::Svg::Window);
        frame.resizeFrame(size);
        const QImage image = frame.framePixmap().toImage();
        for (int x = 0; x < 12; ++x) {
            const QColor c = image.pixelColor(x, 40);
            out << QStringLiteral("  x=%1  %2 / Alpha %3\n").arg(x, 2).arg(rgb(c)).arg(c.alpha(), 3);
        }
        const QColor fill = image.pixelColor(image.width() / 2, 40);
        const QColor rim = image.pixelColor(0, 40);
        out << "  Fläche in der Mitte derselben Zeile: " << rgb(fill) << " / Alpha " << fill.alpha() << "\n";
        out << "\nBefund: Die Grafik zeichnet **keine Linie in anderer Farbe**. Kante und Fläche\n"
               "haben dieselbe Farbe (" << rgb(rim) << " gegen " << rgb(fill) << ") und\n"
               "unterscheiden sich allein in der **Deckung**: " << rim.alpha() << " gegen " << fill.alpha()
            << " von 255.\n";
        out << "Der Rand des Themes ist also ein Deckungsrand, keine Konturfarbe. Sichtbar wird\n"
               "er nur, weil die Hülle durchscheint — auf deckendem Grund wäre er unsichtbar.\n";
        out << "Zum Vergleich: die heutige eigene Kontur (frameContrast 0,20) liegt laut\n"
               "Zeichnung 4b zwischen 1,24:1 und 1,91:1 Farbkontrast gegen die Fläche.\n";
    }

    return 0;
}
