// Prüft die Ursache des Konturbefunds: deckt sich die Alphamaske des inneren
// Rahmens (um 2 px kleiner, um 1,1 versetzt) an der Ecke mit der äußeren?
#include "desktopthemes.h"
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <QGuiApplication>
#include <QImage>
#include <QTextStream>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);
    themes::addBundledThemesToDataPath();
    const auto pair = themes::installedThemePair();

    for (const QString &theme : {pair->first, pair->second}) {
        KSvg::ImageSet set(theme, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg outer, inner;
        for (KSvg::FrameSvg *f : {&outer, &inner}) {
            f->setImageSet(&set);
            f->setImagePath(QStringLiteral("dialogs/background"));
            f->setEnabledBorders(KSvg::FrameSvg::AllBorders);
        }
        outer.resizeFrame(QSizeF(600, 174));
        inner.resizeFrame(QSizeF(598, 172));

        const QImage a = outer.alphaMask().toImage().convertToFormat(QImage::Format_ARGB32);
        const QImage b = inner.alphaMask().toImage().convertToFormat(QImage::Format_ARGB32);
        out << "--- " << theme << "  aussen " << a.width() << "x" << a.height()
            << "  innen " << b.width() << "x" << b.height() << "\n";
        out << "    Alpha der Ecke oben links, Diagonale (aussen | innen um 1,1 versetzt | Ringanteil):\n";
        for (int k = 0; k < 12; ++k) {
            const int ao = qAlpha(a.pixel(k, k));
            const int ai = (k >= 1) ? qAlpha(b.pixel(k - 1, k - 1)) : 0;
            out << "      k=" << k << "  aussen " << ao << "  innen " << ai
                << "  Ring " << qMax(0, ao - ai) << "\n";
        }
        const int my = 87;
        out << "    linke Kante (y=" << my << "): aussen "
            << qAlpha(a.pixel(0, my)) << "/" << qAlpha(a.pixel(1, my))
            << "  innen(versetzt) " << 0 << "/" << qAlpha(b.pixel(0, my - 1))
            << "  -> Ring in Spalte 0 = " << qAlpha(a.pixel(0, my)) << "\n";
    }
    return 0;
}
