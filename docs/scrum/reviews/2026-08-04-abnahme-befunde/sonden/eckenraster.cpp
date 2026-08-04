/**
 * Messsonde zu Kundenbefund B1 — „Die Ecke ist eine Treppe, nicht ein Bogen."
 *
 * Der Kunde fährt seinen Bildschirm mit **Skalierung 1,6** (kscreen-doctor:
 * 3840x2160 physisch, 2400x1350 logisch). Diese Sonde misst deshalb nicht eine
 * Ecke, sondern dieselbe Ecke unter mehreren Geräte-Pixelverhältnissen, und sie
 * misst getrennt:
 *
 *   1. was KSvg als Maske liefert (Größe, devicePixelRatio, Alphaverlauf am
 *      Bogen) — die Form, bevor Denkzettel sie anfasst,
 *   2. was am gemalten Fenster ankommt (Geräte-Pixelraster der linken oberen
 *      Ecke) — die Form, die der Kunde sieht.
 *
 * Weichen 1 und 2 auseinander, liegt die Ursache zwischen beiden, also in
 * `CaptureWindow::paintEvent`. Stimmen sie überein, liegt sie in KSvg oder im
 * Theme.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
 *         QT_SCALE_FACTOR=<f> eckenraster <Zielverzeichnis> [Themename]
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>
#include <QTextStream>

namespace {

/** Wie viele Geräte-Pixel der Ecke ins Protokoll wandern. */
constexpr int Window = 34;

void dumpChannel(QTextStream &out, const QImage &image, const char *what, bool alpha)
{
    out << "\n" << what << "  (" << image.width() << "x" << image.height() << " Gerätepixel)\n    ";
    for (int x = 0; x < qMin(Window, image.width()); ++x) {
        out << QStringLiteral("%1").arg(x, 4);
    }
    out << "\n";
    for (int y = 0; y < qMin(Window, image.height()); ++y) {
        out << QStringLiteral("%1 ").arg(y, 3);
        for (int x = 0; x < qMin(Window, image.width()); ++x) {
            const QColor c = image.pixelColor(x, y);
            out << QStringLiteral("%1").arg(alpha ? c.alpha() : c.red(), 4);
        }
        out << "\n";
    }
}

/**
 * Zählt, wie oft der Rand des Bogens die Spalte wechselt und wie weit.
 *
 * Ein kantengeglätteter Bogen wandert je Zeile um höchstens ein Pixel; eine
 * Treppe springt um zwei oder mehr und steht dazwischen still. Gemessen wird
 * die erste Spalte je Zeile, deren Deckung über der Hälfte liegt.
 */
void dumpEdgeWalk(QTextStream &out, const QImage &image, bool alpha, int threshold)
{
    out << "\nKantenlauf (erste Spalte je Zeile mit Wert " << (alpha ? ">= " : "<= ") << threshold << ")\n";
    int previous = -1;
    for (int y = 0; y < qMin(Window, image.height()); ++y) {
        int found = -1;
        for (int x = 0; x < image.width(); ++x) {
            const QColor c = image.pixelColor(x, y);
            const int v = alpha ? c.alpha() : c.red();
            if (alpha ? (v >= threshold) : (v <= threshold)) {
                found = x;
                break;
            }
        }
        const int step = (previous >= 0 && found >= 0) ? previous - found : 0;
        out << QStringLiteral("  Zeile %1  Spalte %2  Sprung %3%4\n")
                   .arg(y, 3)
                   .arg(found, 4)
                   .arg(step, 3)
                   .arg(qAbs(step) >= 2 ? QStringLiteral("   <== Stufe") : QString());
        previous = found;
    }
}

} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QTextStream out(stdout);
    const QString directory = app.arguments().value(1, QStringLiteral("."));
    const QString theme = app.arguments().value(2, QStringLiteral("default"));
    QDir().mkpath(directory);

    out << "=== Eckenraster ===\n";
    out << "Theme            : " << theme << "\n";
    out << "QT_SCALE_FACTOR  : " << qEnvironmentVariable("QT_SCALE_FACTOR", QStringLiteral("(nicht gesetzt)")) << "\n";
    out << "qApp DPR         : " << app.devicePixelRatio() << "\n";

    // --- 1. Was KSvg liefert, bevor Denkzettel es anfasst -------------------
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    KSvg::FrameSvg frame;
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.resizeFrame(QSizeF(600, 150));

    out << "FrameSvg gültig  : " << (frame.isValid() ? "ja" : "nein") << "\n";
    out << "FrameSvg DPR     : " << frame.devicePixelRatio() << "\n";

    const QPixmap mask = frame.alphaMask();
    out << "alphaMask Größe  : " << mask.width() << "x" << mask.height() << "\n";
    out << "alphaMask DPR    : " << mask.devicePixelRatio() << "\n";
    out << "  (verlangt wurde 600x150 logisch — bei DPR " << app.devicePixelRatio()
        << " wären das " << qRound(600 * app.devicePixelRatio()) << "x" << qRound(150 * app.devicePixelRatio())
        << " Gerätepixel)\n";

    const QImage maskImage = mask.toImage();
    dumpChannel(out, maskImage, "Alphakanal der Maske, linke obere Ecke", true);
    dumpEdgeWalk(out, maskImage, true, 128);
    maskImage.copy(0, 0, 64, 64).scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation)
        .save(QDir(directory).filePath(QStringLiteral("maske-ecke.png")));

    // --- 2. Was am gemalten Fenster ankommt --------------------------------
    const QTemporaryDir tmp;
    Store store(tmp.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen\n";
        return 1;
    }

    CaptureWindow window(&store);
    window.reloadDesktopTheme(theme);
    window.show();
    QCoreApplication::processEvents();

    const QPixmap grabbed = window.grab();
    out << "\ngrab() Größe     : " << grabbed.width() << "x" << grabbed.height()
        << " Gerätepixel, DPR " << grabbed.devicePixelRatio() << "\n";
    out << "Fenstergröße     : " << window.width() << "x" << window.height() << " logisch\n";

    // Auf einen bekannten Grund legen, damit Durchsichtigkeit messbar wird:
    // reines Weiß außen (255), damit jede Abweichung nach unten von der Hülle
    // stammt und jede nach oben unmöglich ist.
    QImage laid(grabbed.size(), QImage::Format_ARGB32);
    laid.fill(Qt::white);
    {
        QPainter painter(&laid);
        painter.drawImage(0, 0, grabbed.toImage());
    }
    dumpChannel(out, laid, "Rotkanal des Fensters auf weißem Grund, linke obere Ecke", false);
    dumpEdgeWalk(out, laid, false, 250);

    laid.copy(0, 0, 64, 64).scaled(512, 512, Qt::KeepAspectRatio, Qt::FastTransformation)
        .save(QDir(directory).filePath(QStringLiteral("fenster-ecke.png")));
    grabbed.save(QDir(directory).filePath(QStringLiteral("fenster-ganz.png")));

    return 0;
}
