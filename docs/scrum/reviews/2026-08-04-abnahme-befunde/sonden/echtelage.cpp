/**
 * Messsonde zu Kundenbefund B1, dritter Teil — welches Pixelverhältnis gilt
 * am laufenden Bildschirm wirklich?
 *
 * Offscreen lässt sich das Verhältnis nur **vorgeben** (`QT_SCALE_FACTOR`), und
 * die Aufnahme des Kunden zeigt eine Kontur von zwei Bildpunkten, wo die
 * offscreen erzeugte eine von einem zeigt. Diese Sonde beendet das Raten: Sie
 * baut das echte Fenster in der angemeldeten Sitzung, lässt den Compositor ihm
 * seine Fläche geben und liest ab, was dabei herauskommt.
 *
 * Was sie **nicht** tut: den Bildschirm aufnehmen. `QWidget::grab()` zeichnet
 * das Fenster erneut in eine eigene Fläche — vom Schreibtisch des Kunden kommt
 * dabei kein einziger Bildpunkt mit. Sichtbar ist das Fenster für gut eine
 * Sekunde.
 *
 * Aufruf: echtelage <Zielverzeichnis>   (in der angemeldeten Sitzung)
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QScreen>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QWindow>

namespace {

constexpr int Window = 30;

void dumpRed(QTextStream &out, const QImage &image, const QString &what)
{
    out << "\n" << what << "\n     ";
    for (int x = 0; x < Window; ++x) {
        out << QStringLiteral("%1").arg(x, 4);
    }
    out << "\n";
    for (int y = 0; y < Window; ++y) {
        out << QStringLiteral("%1  ").arg(y, 3);
        for (int x = 0; x < Window; ++x) {
            out << QStringLiteral("%1").arg(image.pixelColor(x, y).red(), 4);
        }
        out << "\n";
    }
}

} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    const QString directory = app.arguments().value(1, QStringLiteral("."));
    QDir().mkpath(directory);

    const QTemporaryDir tmp;
    Store store(tmp.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen\n";
        return 1;
    }

    CaptureWindow window(&store);
    const QString theme = app.arguments().value(2);
    if (!theme.isEmpty()) {
        window.reloadDesktopTheme(theme);
    }
    // showCapture() statt show(): erst present() hängt den Schatten an das
    // native Fenster (bindShadow braucht ein windowHandle). Mit show() allein
    // bliebe der Schatten aus — das wäre ein Fehler der Sonde, kein Befund.
    window.showCapture();

    // Dem Compositor Zeit geben, die Fläche zu vergeben — vorher steht das
    // Pixelverhältnis des Fensters nicht fest.
    QTimer::singleShot(1200, &app, [&] {
        out << "=== Echte Lage in der angemeldeten Sitzung ===\n";
        if (QScreen *screen = app.primaryScreen()) {
            out << "Bildschirm       : " << screen->geometry().width() << "x" << screen->geometry().height()
                << " logisch, DPR " << screen->devicePixelRatio() << "\n";
        }
        if (QWindow *handle = window.windowHandle()) {
            out << "Fenster DPR      : " << handle->devicePixelRatio() << "\n";
        }
        out << "Desktop-Theme    : " << (theme.isEmpty() ? QStringLiteral("(aus plasmarc)") : theme) << "\n";
        out << "Fenstergröße     : " << window.width() << "x" << window.height() << " logisch\n";
        out << "Schatten         : " << (window.shadow() ? "vom Compositor angenommen" : "keiner") << "\n";

        const QPixmap grabbed = window.grab();
        out << "grab()           : " << grabbed.width() << "x" << grabbed.height() << " Bildpunkte, DPR "
            << grabbed.devicePixelRatio() << "\n";

        QImage laid(grabbed.size(), QImage::Format_ARGB32);
        laid.fill(Qt::white);
        {
            QPainter painter(&laid);
            painter.drawImage(0, 0, grabbed.toImage());
        }
        dumpRed(out, laid, QStringLiteral("Rotkanal der linken oberen Ecke auf weißem Grund"));
        laid.copy(0, 0, 24, 24)
            .scaled(480, 480, Qt::KeepAspectRatio, Qt::FastTransformation)
            .save(QDir(directory).filePath(QStringLiteral("echte-ecke.png")));
        grabbed.save(QDir(directory).filePath(QStringLiteral("echtes-fenster.png")));
        out.flush();
        app.quit();
    });

    return app.exec();
}
