// Was zeichnet das Fenster in seiner Ecke wirklich? ARGB der linken oberen Ecke.
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    store.open();
    themes::addBundledThemesToDataPath();
    const auto pair = themes::installedThemePair();

    QPalette p;
    p.setColor(QPalette::Window, QColor(0xef, 0xf0, 0xf1));
    p.setColor(QPalette::WindowText, QColor(0x23, 0x26, 0x29));
    app.setPalette(p);

    for (const QString &theme : {pair->first, pair->second}) {
        CaptureWindow w(&store);
        w.reloadDesktopTheme(theme);
        w.show();
        const QImage img = w.grab().toImage().convertToFormat(QImage::Format_ARGB32);
        out << "--- " << theme << "  " << img.width() << "x" << img.height() << "\n";
        out << "    ARGB der Ecke (a,r,g,b):\n";
        for (int y = 0; y < 10; ++y) {
            out << "     ";
            for (int x = 0; x < 10; ++x) {
                const QRgb c = img.pixel(x, y);
                out << QStringLiteral(" %1/%2").arg(qAlpha(c), 3).arg(qRed(c), 3);
            }
            out << "\n";
        }
        const QRgb edge = img.pixel(0, img.height() / 2);
        out << "    linke Kante Mitte: a=" << qAlpha(edge) << " r=" << qRed(edge) << "\n";
    }
    return 0;
}
