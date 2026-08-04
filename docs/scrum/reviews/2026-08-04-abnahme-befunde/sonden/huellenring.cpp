/**
 * Messsonde zu Kundenbefund B1, zweiter Teil — der Ring aus zwei Rahmen.
 *
 * `CaptureWindow::paintEvent` baut die Kontur als Ring: die Maske des Themes,
 * eingefärbt mit der Konturfarbe, und darüber dieselbe Maske einen Umriss
 * kleiner, eingefärbt mit der Fläche, versetzt um (1,1). Diese Sonde baut
 * genau das nach — Zeile für Zeile derselbe Aufruf — und legt jeden Schritt
 * einzeln offen:
 *
 *   a) die beiden Alphamasken,
 *   b) die beiden eingefärbten Pixmaps (Format, Alphakanal, Werte),
 *   c) das Ergebnis auf weißem Grund.
 *
 * Wo der Bogen seine Kontur verliert und wo ein durchsichtiges Loch zwischen
 * Kontur und Fläche steht, ist damit auf einen der drei Schritte festzulegen.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen huellenring [Themename]
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QTextStream>

namespace {

constexpr int Window = 14;
constexpr int OutlineWidth = 1;
constexpr qreal FrameContrast = 0.20;

/** Wortgleich aus `capturewindow.cpp` übernommen. */
QColor mixed(const QColor &from, const QColor &to, qreal amount)
{
    return QColor::fromRgbF(from.redF() + (to.redF() - from.redF()) * amount,
                            from.greenF() + (to.greenF() - from.greenF()) * amount,
                            from.blueF() + (to.blueF() - from.blueF()) * amount);
}

/** Wortgleich aus `capturewindow.cpp` übernommen. */
QPixmap tinted(const QPixmap &shape, const QColor &colour)
{
    QPixmap result(shape.size());
    result.setDevicePixelRatio(shape.devicePixelRatio());
    result.fill(colour);

    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, shape);

    return result;
}

void grid(QTextStream &out, const QImage &image, const QString &what, bool alphaOnly)
{
    out << "\n" << what << "\n     ";
    for (int x = 0; x < Window; ++x) {
        out << QStringLiteral("%1").arg(x, alphaOnly ? 5 : 18);
    }
    out << "\n";
    for (int y = 0; y < Window; ++y) {
        out << QStringLiteral("%1  ").arg(y, 3);
        for (int x = 0; x < Window; ++x) {
            const QColor c = image.pixelColor(x, y);
            if (alphaOnly) {
                out << QStringLiteral("%1").arg(c.alpha(), 5);
            } else {
                out << QStringLiteral("%1").arg(QStringLiteral("%1,%2,%3/%4")
                                                    .arg(c.red())
                                                    .arg(c.green())
                                                    .arg(c.blue())
                                                    .arg(c.alpha()),
                                                18);
            }
        }
        out << "\n";
    }
}

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QString theme = app.arguments().value(1, QStringLiteral("default"));
    const QSize size(600, 174); // die Maße des echten Fensters unter Breeze

    // Die Farben von Breeze Light, wie sie in der Kundenaufnahme stehen.
    const QColor surface(0xef, 0xf0, 0xf1);
    const QColor textColour(0x23, 0x26, 0x29);
    const QColor outline = mixed(surface, textColour, FrameContrast);

    out << "=== Hüllenring ===\n";
    out << "Plattform        : " << app.platformName() << "\n";
    out << "Theme            : " << theme << "\n";
    out << "Fläche           : " << surface.red() << "," << surface.green() << "," << surface.blue() << "\n";
    out << "Konturfarbe      : " << outline.red() << "," << outline.green() << "," << outline.blue()
        << "   (Fläche 20 % Richtung Textfarbe)\n";

    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    KSvg::FrameSvg hull;
    KSvg::FrameSvg hullInner;
    for (KSvg::FrameSvg *frame : {&hull, &hullInner}) {
        frame->setImageSet(&imageSet);
        frame->setImagePath(QStringLiteral("dialogs/background"));
        frame->setEnabledBorders(KSvg::FrameSvg::AllBorders);
    }
    hull.resizeFrame(size);
    hullInner.resizeFrame(size - QSize(2 * OutlineWidth, 2 * OutlineWidth));

    const QPixmap outerMask = hull.alphaMask();
    const QPixmap innerMask = hullInner.alphaMask();

    out << "\na) Die beiden Masken\n";
    out << "   außen : " << outerMask.width() << "x" << outerMask.height()
        << "  Alphakanal " << (outerMask.hasAlphaChannel() ? "ja" : "NEIN") << "\n";
    out << "   innen : " << innerMask.width() << "x" << innerMask.height()
        << "  Alphakanal " << (innerMask.hasAlphaChannel() ? "ja" : "NEIN") << "\n";
    grid(out, outerMask.toImage(), QStringLiteral("Alpha der äußeren Maske"), true);
    grid(out, innerMask.toImage(), QStringLiteral("Alpha der inneren Maske"), true);

    const QPixmap outerTinted = tinted(outerMask, outline);
    const QPixmap innerTinted = tinted(innerMask, surface);

    out << "\nb) Die beiden eingefärbten Pixmaps\n";
    out << "   außen : Alphakanal " << (outerTinted.hasAlphaChannel() ? "ja" : "NEIN")
        << "  Format " << static_cast<int>(outerTinted.toImage().format()) << "\n";
    out << "   innen : Alphakanal " << (innerTinted.hasAlphaChannel() ? "ja" : "NEIN")
        << "  Format " << static_cast<int>(innerTinted.toImage().format()) << "\n";
    grid(out, outerTinted.toImage(), QStringLiteral("Äußere Pixmap, r,g,b/Alpha"), false);
    grid(out, innerTinted.toImage(), QStringLiteral("Innere Pixmap, r,g,b/Alpha"), false);

    // c) Der Ring, so wie paintEvent ihn malt — und darunter Weiß, damit jede
    //    Durchsichtigkeit sichtbar wird.
    QImage composed(size, QImage::Format_ARGB32_Premultiplied);
    composed.fill(Qt::transparent);
    {
        QPainter painter(&composed);
        painter.drawPixmap(0, 0, outerTinted);
        painter.drawPixmap(OutlineWidth, OutlineWidth, innerTinted);
    }
    grid(out, composed, QStringLiteral("c) Der Ring, r,g,b/Alpha (Alpha < 255 = durchsichtig)"), false);

    QImage onWhite(size, QImage::Format_ARGB32);
    onWhite.fill(Qt::white);
    {
        QPainter painter(&onWhite);
        painter.drawImage(0, 0, composed);
    }
    grid(out, onWhite, QStringLiteral("Der Ring auf weißem Grund"), false);

    out << "\nZum Vergleich: Fläche " << surface.red() << ", Kontur " << outline.red()
        << ", Grund 255. Ein Pixel mit " << surface.red()
        << " ist Fläche, eines mit " << outline.red() << " Kontur, eines mit 255 ein Loch.\n";

    // --- d) Gegenprobe -----------------------------------------------------
    // Dieselben zwei Masken, dieselben zwei Farben, derselbe Versatz — nur
    // führt die Einfärbung diesmal einen **echten** Alphakanal. Bleibt der
    // Bogen dann vollständig, liegt es an nichts anderem als daran.
    const auto tintedWithAlpha = [](const QPixmap &shape, const QColor &colour) {
        QImage result(shape.size(), QImage::Format_ARGB32_Premultiplied);
        result.setDevicePixelRatio(shape.devicePixelRatio());
        result.fill(colour);

        QPainter painter(&result);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawPixmap(0, 0, shape);
        painter.end();
        return result;
    };

    const QImage outerAlpha = tintedWithAlpha(outerMask, outline);
    const QImage innerAlpha = tintedWithAlpha(innerMask, surface);

    out << "\nd) Gegenprobe mit echtem Alphakanal\n";
    out << "   außen : Alphakanal " << (outerAlpha.hasAlphaChannel() ? "ja" : "NEIN")
        << "  Format " << static_cast<int>(outerAlpha.format()) << "\n";
    out << "   innen : Alphakanal " << (innerAlpha.hasAlphaChannel() ? "ja" : "NEIN")
        << "  Format " << static_cast<int>(innerAlpha.format()) << "\n";

    QImage composedAlpha(size, QImage::Format_ARGB32_Premultiplied);
    composedAlpha.fill(Qt::transparent);
    {
        QPainter painter(&composedAlpha);
        painter.drawImage(0, 0, outerAlpha);
        painter.drawImage(OutlineWidth, OutlineWidth, innerAlpha);
    }
    grid(out, composedAlpha, QStringLiteral("d) Derselbe Ring mit Alphakanal, r,g,b/Alpha"), false);

    QImage onWhiteAlpha(size, QImage::Format_ARGB32);
    onWhiteAlpha.fill(Qt::white);
    {
        QPainter painter(&onWhiteAlpha);
        painter.drawImage(0, 0, composedAlpha);
    }
    grid(out, onWhiteAlpha, QStringLiteral("d) Derselbe Ring mit Alphakanal, auf weißem Grund"), false);

    // --- e) Der zweite Hebel: die Auflösung der Maske -----------------------
    // Der Kunde fährt Skalierung 1,6. Liefert KSvg die Maske in Gerätepixeln,
    // wenn man es darum bittet? Wenn ja, ist die Treppe zweiter Ordnung
    // (Hochskalieren einer 1x-Maske) vermeidbar; wenn nein, nicht.
    out << "\ne) Auflösung der Maske bei setDevicePixelRatio(1.6)\n";
    for (KSvg::FrameSvg *frame : {&hull, &hullInner}) {
        frame->setDevicePixelRatio(1.6);
    }
    hull.resizeFrame(size);
    hullInner.resizeFrame(size - QSize(2 * OutlineWidth, 2 * OutlineWidth));

    const QPixmap scaledMask = hull.alphaMask();
    out << "   verlangt        : " << size.width() << "x" << size.height() << " logisch\n";
    out << "   alphaMask Größe : " << scaledMask.width() << "x" << scaledMask.height() << " Bildpunkte\n";
    out << "   alphaMask DPR   : " << scaledMask.devicePixelRatio() << "\n";
    out << "   erwartet bei 1,6: " << qRound(size.width() * 1.6) << "x" << qRound(size.height() * 1.6) << "\n";
    grid(out, scaledMask.toImage(), QStringLiteral("Alpha der Maske bei DPR 1,6"), true);

    // --- Bilder ------------------------------------------------------------
    // Beide Ecken auf dem hellen Grund, den auch die Aufnahme des Kunden hat,
    // zwanzigfach vergrößert und ohne Glättung: Was hier weich aussieht, ist
    // weich gerechnet und nicht weich skaliert.
    const QString directory = app.arguments().value(2, QStringLiteral("."));
    const auto writeCorner = [&directory](const QImage &image, const QString &name) {
        image.copy(0, 0, 24, 24)
            .scaled(480, 480, Qt::KeepAspectRatio, Qt::FastTransformation)
            .save(directory + QLatin1Char('/') + name);
    };
    writeCorner(onWhite, QStringLiteral("ring-ist.png"));
    writeCorner(onWhiteAlpha, QStringLiteral("ring-gegenprobe-alphakanal.png"));

    return 0;
}
