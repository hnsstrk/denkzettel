/**
 * Das Erfassungsfenster in der **angemeldeten Sitzung** — der Beleg zu AK 2 und
 * AK 4 von #100 (B21).
 *
 * Warum es diese Sonde gibt und ein offscreen erzeugtes Bild nicht genügt: Ein
 * offscreen gezeichnetes Bild belegt Geometrie, Textsatz und Farbrollen. Hülle,
 * Rundung, Kontur, Schatten und Durchsichtigkeit belegt es **nicht** — die
 * zeichnen Theme und Compositor, und offscreen fehlt beiden die Grundlage
 * (B21, gemessen am 04.08.2026). AK 2 behauptet über genau diese Größen etwas,
 * und AK 4 über Farben, die aus der Theme-Grafik kommen.
 *
 * Was die Sonde **nicht** tut: den Bildschirm aufnehmen. `QWidget::grab()`
 * zeichnet das Fenster in eine eigene Fläche; vom Schreibtisch des Kunden kommt
 * dabei kein Bildpunkt mit. Sichtbar ist das Fenster für wenige Sekunden.
 *
 * `QT_SCALE_FACTOR` wird hier **nicht** gesetzt, und das ist gemessen: Unter
 * Wayland multipliziert die Variable mit der Sitzungsskalierung (1 → 1,6;
 * 1,6 → 2,56). Die Skalierung des Kunden ist vorzufinden, nicht einzustellen
 * (#100, F10).
 *
 * Aufruf: sitzungsbild <Zielverzeichnis> [Theme A] [Theme B]
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowShadow>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScreen>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QWindow>

namespace {

/** Eine Ausfertigung der Theme-Grafik daneben — dieselbe Bauart wie themeHull(). */
QPixmap themeGraphic(const QString &theme,
                     const QString &image,
                     const QString &prefix,
                     const QSize &size,
                     const QStringList &selectors,
                     qreal ratio)
{
    KSvg::ImageSet set(theme, QStringLiteral("plasma/desktoptheme"));
    set.setSelectors(selectors);

    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    if (image == QLatin1String("dialogs/background")) {
        frame.setColorSet(KSvg::Svg::Window);
    }
    frame.setImageSet(&set);
    frame.setImagePath(image);
    if (!prefix.isEmpty()) {
        frame.setElementPrefix(prefix);
    }
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.setDevicePixelRatio(ratio);
    frame.resizeFrame(size);
    return frame.framePixmap();
}

QString show(const QColor &colour)
{
    return QStringLiteral("%1,%2,%3/a%4")
        .arg(colour.red())
        .arg(colour.green())
        .arg(colour.blue())
        .arg(colour.alpha());
}

/** Wie weit die Hülle in der obersten Zeile noch durchsichtig ist. */
int cornerRun(const QImage &picture)
{
    int x = 0;
    while (x < picture.width() && qAlpha(picture.pixel(x, 0)) == 0) {
        ++x;
    }
    return x;
}

/** Die erste Spalte je Zeile, in der die Hülle mehr als halb deckt. */
QList<int> edgeWalk(const QImage &picture)
{
    QList<int> columns;
    for (int y = 0; y < 10 && y < picture.height(); ++y) {
        int found = picture.width();
        for (int x = 0; x < picture.width(); ++x) {
            if (qAlpha(picture.pixel(x, y)) >= 128) {
                found = x;
                break;
            }
        }
        columns << found;
    }
    return columns;
}

QPoint inPicture(const QPoint &logical, qreal ratio)
{
    return QPoint(qRound(logical.x() * ratio), qRound(logical.y() * ratio));
}

void measure(QTextStream &out,
             CaptureWindow &window,
             const QString &theme,
             const QString &directory,
             const QString &name)
{
    const auto *text = window.findChild<QPlainTextEdit *>();
    const QPixmap grabbed = window.grab();
    const QImage picture = grabbed.toImage();
    const qreal ratio = grabbed.devicePixelRatio();

    out << "\n############ " << name << " (Theme " << theme << ")\n";
    out << "  Fenster            : " << window.width() << "x" << window.height()
        << " logisch, Bild " << picture.width() << "x" << picture.height()
        << ", Verhältnis " << ratio << "\n";
    out << "  Textbereich        : x=" << text->x() << " y=" << text->y() << " "
        << text->width() << "x" << text->height() << "\n";
    out << "  Schatten           : "
        << (window.shadow() ? "vom Compositor angenommen" : "keiner") << "\n";

    // AK 2 — die Hülle, an denselben Größen gemessen, die die Prüfsätze von #83
    // zusichern: die Ecke ist eine Ecke, und der Bogen läuft ohne Rückschritt.
    out << "  Eckenlauf          : " << cornerRun(picture) << "\n";
    QStringList walk;
    for (const int column : edgeWalk(picture)) {
        walk << QString::number(column);
    }
    out << "  Kantenlauf         : " << walk.join(QLatin1Char('.')) << "\n";

    // Welche Fassung der Grafik zeichnet das Fenster hier? In der Sitzung
    // weichzeichnet der Compositor, also die durchscheinende — offscreen die
    // deckende. Beide werden gerechnet, damit die Antwort im Protokoll steht
    // und nicht in einer Annahme.
    const QPoint besideTheField = inPicture(QPoint(window.width() / 2, text->y() - 4), ratio);
    const QPoint fieldSurface = inPicture(text->geometry().center(), ratio);
    const QPoint fieldEdge = inPicture(QPoint(text->x(), text->y() + text->height() / 2), ratio);

    for (const QStringList &selectors : {QStringList{}, QStringList{QStringLiteral("opaque")}}) {
        const QImage hull = themeGraphic(theme,
                                         QStringLiteral("dialogs/background"),
                                         {},
                                         window.size(),
                                         selectors,
                                         ratio)
                                .toImage();
        QImage both = hull;
        {
            QPainter painter(&both);
            painter.drawPixmap(text->geometry().topLeft(),
                               themeGraphic(theme,
                                            QStringLiteral("widgets/lineedit"),
                                            QStringLiteral("base"),
                                            text->size(),
                                            selectors,
                                            ratio));
        }

        const QString label = selectors.isEmpty() ? QStringLiteral("durchscheinend")
                                                  : QStringLiteral("deckend      ");
        out << "  Fassung " << label << ": Hülle daneben "
            << (hull.rect().contains(besideTheField)
                    ? show(hull.pixelColor(besideTheField))
                    : QStringLiteral("außerhalb"))
            << "  Feldfläche "
            << (both.rect().contains(fieldSurface) ? show(both.pixelColor(fieldSurface))
                                                   : QStringLiteral("außerhalb"))
            << "  Feldkante "
            << (both.rect().contains(fieldEdge) ? show(both.pixelColor(fieldEdge))
                                                : QStringLiteral("außerhalb"))
            << "\n";
    }

    out << "  Gezeichnet         : Hülle daneben " << show(picture.pixelColor(besideTheField))
        << "  Feldfläche " << show(picture.pixelColor(fieldSurface)) << "  Feldkante "
        << show(picture.pixelColor(fieldEdge)) << "\n";
    out << "  Feld gegen Hülle   : "
        << (picture.pixelColor(fieldSurface) == picture.pixelColor(besideTheField)
                ? "bildpunktgleich — das Feld hebt sich hier NICHT ab"
                : "verschieden — das Feld hebt sich ab")
        << "\n";

    // Das Bild auf hellem Grund, damit die durchsichtigen Ecken sichtbar sind,
    // und die linke obere Ecke vergrößert daneben.
    QImage laid(picture.size(), QImage::Format_ARGB32);
    laid.setDevicePixelRatio(ratio);
    laid.fill(QColor(0xf2, 0xf0, 0xeb));
    {
        QPainter painter(&laid);
        painter.drawImage(0, 0, picture);
    }
    laid.save(QDir(directory).filePath(name + QStringLiteral(".png")));
    laid.copy(0, 0, qRound(40 * ratio), qRound(40 * ratio))
        .scaled(480, 480, Qt::KeepAspectRatio, Qt::FastTransformation)
        .save(QDir(directory).filePath(name + QStringLiteral("-ecke.png")));
}

} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    const QString directory = app.arguments().value(1, QStringLiteral("."));
    QDir().mkpath(directory);
    const QString themeA = app.arguments().value(2, QStringLiteral("default"));
    const QString themeB = app.arguments().value(3, QStringLiteral("breeze-light"));

    const QTemporaryDir tmp;
    Store store(tmp.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen\n";
        return 1;
    }

    CaptureWindow window(&store);
    window.reloadDesktopTheme(themeA);
    // showCapture() und nicht show(): erst present() hängt den Schatten an das
    // native Fenster, und AK 2 spricht über den Schatten.
    window.showCapture();

    out << "=== #100, Sitzungsbild: das Feld am laufenden Compositor (AK 2, AK 4) ===\n";
    if (QScreen *screen = app.primaryScreen()) {
        out << "Bildschirm         : " << screen->geometry().width() << "x"
            << screen->geometry().height() << " logisch, Verhältnis "
            << screen->devicePixelRatio() << "\n";
    }

    // Dem Compositor Zeit geben, die Fläche zu vergeben — vorher steht das
    // Bildpunktverhältnis des Fensters nicht fest (gemessen, #83).
    QTimer::singleShot(1500, &app, [&] {
        measure(out, window, themeA, directory, QStringLiteral("sitzung-1-theme-a"));

        // AK 4 — hin und zurück am **stehenden** Fenster. Zurück, weil eine
        // Farbe, die sich nur einmal bewegt, auch von einer einmal gesetzten
        // und nie geräumten erklärt würde.
        window.reloadDesktopTheme(themeB);
        QTimer::singleShot(600, &app, [&] {
            measure(out, window, themeB, directory, QStringLiteral("sitzung-2-theme-b"));

            window.reloadDesktopTheme(themeA);
            QTimer::singleShot(600, &app, [&] {
                measure(out, window, themeA, directory, QStringLiteral("sitzung-3-zurueck"));
                out.flush();
                app.quit();
            });
        });
    });

    return app.exec();
}
