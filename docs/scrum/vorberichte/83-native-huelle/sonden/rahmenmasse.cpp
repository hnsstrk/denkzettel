/**
 * Vorprüfung #83, Sonde 1 — was `KSvg::FrameSvg` beim nativen Weg verlangt.
 *
 * Die Story schreibt `framePixmap()` „mit gesetztem Bildpunktverhältnis" vor
 * und will die Maskenregion für `enableBlurBehind`. Beides hat eine
 * Reihenfolge und ein Koordinatensystem, und beides steht in keiner
 * Dokumentation. Gemessen wird:
 *
 *   A  Voreinstellung von devicePixelRatio() und was framePixmap() ohne
 *      Zutun liefert
 *   B  die Reihenfolge: setDevicePixelRatio() vor oder nach resizeFrame()
 *   C  ob ein nachträgliches setDevicePixelRatio() ohne erneutes
 *      resizeFrame() greift
 *   D  mask() — Koordinatensystem, Rechteckzahl, Verhalten bei 1 und 1,6
 *   E  mask() gegen alphaMask(): dieselbe Quelle?
 *   F  marginSize() unter Bildpunktverhältnis 1,6 — logisch oder Gerät?
 *   G  Alphalauf der obersten Zeile: monoton? (AK 3)
 *
 * Aufruf: rahmenmasse [Themename]
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QImage>
#include <QRegion>
#include <QTextStream>

namespace
{
constexpr int WindowWidth = 600;
constexpr int WindowHeight = 174; // Ruhehöhe des Erfassungsfensters

QString describe(const QRegion &region)
{
    const QRect box = region.boundingRect();
    return QStringLiteral("%1 Rechteck(e), Hüllrechteck %2,%3 %4x%5, leer=%6")
        .arg(region.rectCount())
        .arg(box.x())
        .arg(box.y())
        .arg(box.width())
        .arg(box.height())
        .arg(region.isEmpty() ? QStringLiteral("ja") : QStringLiteral("nein"));
}

/** Alpha der obersten Zeile, die ersten 14 Spalten. */
QString topRow(const QImage &image)
{
    QString line;
    for (int x = 0; x < 14 && x < image.width(); ++x) {
        line += QStringLiteral("%1 ").arg(image.pixelColor(x, 0).alpha(), 3);
    }
    return line;
}

/**
 * Der Alphalauf der obersten Zeile, wie AK 3 ihn beschreibt.
 *
 * AK 3 verlangt „monoton, kein Plateau". Beides getrennt gezählt, und die
 * Zählung endet, sobald der Randwert erreicht ist: **die gerade Oberkante
 * selbst ist ein Plateau** — sie hält den Randwert über hunderte Spalten. Wer
 * „kein Plateau" wörtlich über die ganze Zeile prüft, prüft eine Zusicherung,
 * die keine Hülle je erfüllen kann.
 */
QString rampReport(const QImage &image)
{
    int maximum = 0;
    for (int x = 0; x < image.width(); ++x) {
        maximum = qMax(maximum, image.pixelColor(x, 0).alpha());
    }

    int drops = 0;
    int repeats = 0;
    int rampLength = 0;
    int previous = -1;
    for (int x = 0; x < image.width(); ++x) {
        const int alpha = image.pixelColor(x, 0).alpha();
        if (alpha >= maximum) {
            rampLength = x;
            break;
        }
        if (previous >= 0) {
            if (alpha < previous) {
                ++drops;
            }
            if (alpha == previous && alpha > 0) {
                ++repeats;
            }
        }
        previous = alpha;
    }

    return QStringLiteral("Randwert %1, Anstieg über %2 Spalten, Rückschritte %3, "
                          "Wiederholungen im Anstieg %4")
        .arg(maximum)
        .arg(rampLength)
        .arg(drops)
        .arg(repeats);
}

KSvg::FrameSvg *build(KSvg::ImageSet *set)
{
    auto *frame = new KSvg::FrameSvg;
    frame->setImageSet(set);
    frame->setImagePath(QStringLiteral("dialogs/background"));
    frame->setEnabledBorders(KSvg::FrameSvg::AllBorders);
    return frame;
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QString theme = app.arguments().value(1, QStringLiteral("default"));
    const QSize size(WindowWidth, WindowHeight);

    out << "=== Vorprüfung #83, Sonde 1: FrameSvg-Maße und Maskenregion ===\n";
    out << "Plattform   : " << app.platformName() << "\n";
    out << "Theme       : " << theme << "\n";
    out << "qApp DPR    : " << app.devicePixelRatio() << "\n";
    out << "Fenstermaß  : " << size.width() << "x" << size.height() << " logisch\n";

    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));

    // ---------------------------------------------------------------- A
    out << "\n########## A — Voreinstellung ##########\n";
    {
        KSvg::FrameSvg *frame = build(&imageSet);
        frame->resizeFrame(size);
        out << "devicePixelRatio() ohne Zutun : " << frame->devicePixelRatio() << "\n";
        const QPixmap piece = frame->framePixmap();
        out << "framePixmap()                 : " << piece.width() << "x" << piece.height()
            << " Bildpunkte, DPR " << piece.devicePixelRatio() << "\n";
        out << "  -> Folgt die Grafik dem Bildschirm von selbst? "
            << (qFuzzyCompare(frame->devicePixelRatio(), app.devicePixelRatio())
                    && !qFuzzyCompare(app.devicePixelRatio(), qreal(1))
                        ? "ja"
                        : "NEIN — der Wert muss gesetzt werden")
            << "\n";
        delete frame;
    }

    // ---------------------------------------------------------------- B
    out << "\n########## B — Reihenfolge: DPR vor oder nach resizeFrame ##########\n";
    for (const bool ratioFirst : {true, false}) {
        KSvg::FrameSvg *frame = build(&imageSet);
        if (ratioFirst) {
            frame->setDevicePixelRatio(1.6);
            frame->resizeFrame(size);
        } else {
            frame->resizeFrame(size);
            frame->setDevicePixelRatio(1.6);
        }
        const QPixmap piece = frame->framePixmap();
        out << (ratioFirst ? "  DPR vor  resizeFrame : " : "  DPR nach resizeFrame : ")
            << piece.width() << "x" << piece.height() << " Bildpunkte, DPR "
            << piece.devicePixelRatio() << ", frameSize " << frame->frameSize().width() << "x"
            << frame->frameSize().height() << "\n";
        delete frame;
    }

    // ---------------------------------------------------------------- C
    out << "\n########## C — DPR-Wechsel am stehenden Rahmen ##########\n";
    {
        KSvg::FrameSvg *frame = build(&imageSet);
        frame->setDevicePixelRatio(1.0);
        frame->resizeFrame(size);
        const QPixmap before = frame->framePixmap();
        frame->setDevicePixelRatio(1.6);
        const QPixmap afterRatio = frame->framePixmap();
        frame->resizeFrame(size);
        const QPixmap afterResize = frame->framePixmap();
        out << "  vorher (DPR 1)                : " << before.width() << "x" << before.height()
            << ", DPR " << before.devicePixelRatio() << "\n";
        out << "  nur setDevicePixelRatio(1,6)  : " << afterRatio.width() << "x"
            << afterRatio.height() << ", DPR " << afterRatio.devicePixelRatio() << "\n";
        out << "  danach resizeFrame(gleich)    : " << afterResize.width() << "x"
            << afterResize.height() << ", DPR " << afterResize.devicePixelRatio() << "\n";
        delete frame;
    }

    // ---------------------------------------------------------------- D/E/F/G
    out << "\n########## D/E/F/G — Maske, Ränder, Alphalauf ##########\n";
    for (const qreal dpr : {1.0, 1.6}) {
        KSvg::FrameSvg *frame = build(&imageSet);
        frame->setDevicePixelRatio(dpr);
        frame->resizeFrame(size);

        const QPixmap piece = frame->framePixmap();
        const QImage image = piece.toImage();
        const QPixmap alphaMask = frame->alphaMask();
        const QRegion mask = frame->mask();

        out << "\n--- Bildpunktverhältnis " << dpr << "\n";
        out << "  framePixmap : " << piece.width() << "x" << piece.height() << " Bildpunkte, DPR "
            << piece.devicePixelRatio() << ", Format " << static_cast<int>(image.format())
            << ", Alphakanal " << (image.hasAlphaChannel() ? "ja" : "nein") << "\n";
        out << "  alphaMask   : " << alphaMask.width() << "x" << alphaMask.height()
            << " Bildpunkte, DPR " << alphaMask.devicePixelRatio() << "\n";
        out << "  mask()      : " << describe(mask) << "\n";
        out << "     -> Hüllrechteck gegen logisches Fenstermaß (" << size.width() << "x"
            << size.height() << "): "
            << (mask.boundingRect().size() == size ? "gleich — logische Bildpunkte"
                                                   : "ABWEICHEND")
            << "\n";
        out << "     -> Hüllrechteck gegen Gerätemaß (" << piece.width() << "x" << piece.height()
            << "): "
            << (mask.boundingRect().size() == piece.size() ? "gleich — Gerätebildpunkte"
                                                           : "abweichend")
            << "\n";

        qreal left = 0;
        qreal top = 0;
        qreal right = 0;
        qreal bottom = 0;
        frame->getMargins(left, top, right, bottom);
        out << "  getMargins  : " << left << " " << top << " " << right << " " << bottom << "\n";
        out << "  Mitte       : " << image.pixelColor(image.width() / 2, image.height() / 2).red()
            << "," << image.pixelColor(image.width() / 2, image.height() / 2).green() << ","
            << image.pixelColor(image.width() / 2, image.height() / 2).blue() << " / Alpha "
            << image.pixelColor(image.width() / 2, image.height() / 2).alpha() << "\n";
        out << "  Kantenmitte oben, Alpha : "
            << image.pixelColor(image.width() / 2, 0).alpha() << "\n";
        out << "  oberste Zeile, Alpha    : " << topRow(image) << "\n";
        out << "  Alphalauf (AK 3)        : " << rampReport(image) << "\n";

        delete frame;
    }

    // ---------------------------------------------------------------- E2
    out << "\n########## E2 — mask() bei einer Hülle, die nicht deckt ##########\n";
    out << "mask() ist dokumentiert als „tightly contains the fully opaque areas\"."
        << " Die native\nHülle deckt zu 84,7 %. Deshalb die ausdrückliche Gegenprobe, ob die"
        << " Region\nüberhaupt etwas enthält — sie ist die Region für enableBlurBehind.\n";
    {
        KSvg::FrameSvg *frame = build(&imageSet);
        frame->resizeFrame(size);
        const QRegion mask = frame->mask();
        const QImage maskImage = frame->alphaMask().toImage();
        out << "  mask()        : " << describe(mask) << "\n";
        out << "  Fläche der Region : " << [&mask] {
            int area = 0;
            for (const QRect &r : mask) {
                area += r.width() * r.height();
            }
            return area;
        }() << " von " << size.width() * size.height() << " logischen Bildpunkten\n";
        out << "  alphaMask() Mitte, Alpha : "
            << maskImage.pixelColor(maskImage.width() / 2, maskImage.height() / 2).alpha() << "\n";
        out << "  alphaMask() Ecke (0,0), Alpha : " << maskImage.pixelColor(0, 0).alpha() << "\n";
        delete frame;
    }

    return 0;
}
