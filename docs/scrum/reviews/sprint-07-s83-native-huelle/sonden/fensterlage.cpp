/**
 * Messsonde 2 zu #83 — das **echte** Erfassungsfenster, in der Lage, in der es
 * gestartet wird.
 *
 * Sie baut `CaptureWindow` aus der übersetzten Bibliothek des Projekts, nicht
 * einen Nachbau: Was hier gemessen wird, ist der gelieferte Code. Gedacht ist
 * sie für die **angemeldete Sitzung**, wo das Bildpunktverhältnis vom
 * Compositor kommt und eine Sekunde nach `show()` noch springt; offscreen läuft
 * sie ebenfalls, und der Vergleich der beiden Protokolle ist selbst ein Befund.
 *
 * Belegt damit:
 *   AK 3   Verhältnis des Fensters gegen Verhältnis der Hülle, auch nach dem
 *          späten `DevicePixelRatioChange`
 *   AK 4   der Kantenlauf am gezeichneten Fenster
 *   AK 5   dass die Maskenregion einer Größenänderung folgt (fünf → acht
 *          Zeilen und zurück) — die Region selbst, nicht ihre Wirkung
 *   AK 9   ein Theme mit rechteckigen Eckstücken behält rechteckige Ecken
 *   AK 10  Theme-Wechsel im Betrieb: Rand, Ecke und Schattenkacheln
 *   AK 13  Bild der Ecke für `eckhelligkeit.py`
 *
 * Was sie **nicht** tut: den Bildschirm aufnehmen. `QWidget::grab()` zeichnet
 * das Fenster in eine eigene Fläche; vom Schreibtisch kommt kein Bildpunkt mit.
 *
 * Aufruf: fensterlage <Zielverzeichnis> [ZweitesTheme]
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowShadow>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QLayout>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScreen>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QWindow>

namespace
{
constexpr int EdgeWalkRows = 10;

/** Die Maskenregion, die das Fenster dem Weichzeichner übergibt (AK 5). */
QRegion hullMask(const QString &theme, const QSize &size, bool opaque)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    if (opaque) {
        imageSet.setSelectors({QStringLiteral("opaque")});
    }

    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    frame.setColorSet(KSvg::Svg::Window);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(size);
    return frame.mask();
}

QString edgeWalk(const QImage &image, int *stairs, bool *falling)
{
    QStringList columns;
    int previous = -1;
    *stairs = 0;
    *falling = true;
    for (int y = 0; y < EdgeWalkRows && y < image.height(); ++y) {
        int found = image.width();
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alpha() >= 128) {
                found = x;
                break;
            }
        }
        if (previous >= 0) {
            if (found > previous) {
                *falling = false;
            }
            if (previous - found >= 2) {
                ++*stairs;
            }
        }
        previous = found;
        columns << QString::number(found);
    }
    return columns.join(QLatin1Char('.'));
}

void report(QTextStream &out, CaptureWindow &window, const QString &what)
{
    const QImage picture = window.grab().toImage();
    int stairs = 0;
    bool falling = true;
    const QString walk = edgeWalk(picture, &stairs, &falling);

    int corner = 0;
    while (corner < picture.width() && picture.pixelColor(corner, 0).alpha() == 0) {
        ++corner;
    }

    out << QStringLiteral("  %1\n").arg(what);
    out << QStringLiteral("    Fenster %1x%2 logisch, Bild %3x%4 Bildpunkte\n")
               .arg(window.width())
               .arg(window.height())
               .arg(picture.width())
               .arg(picture.height());
    out << QStringLiteral("    Fenster-DPR %1   Hüllen-DPR %2   %3\n")
               .arg(window.devicePixelRatioF())
               .arg(window.hullDevicePixelRatio())
               .arg(qFuzzyCompare(window.devicePixelRatioF(), window.hullDevicePixelRatio())
                        ? QStringLiteral("gleich")
                        : QStringLiteral("**VERSCHIEDEN — die Hülle hinkt nach**"));
    out << QStringLiteral("    Ecke: cornerRun %1   Kantenlauf %2   Stufen %3   fallend %4\n")
               .arg(corner)
               .arg(walk)
               .arg(stairs)
               .arg(falling ? QStringLiteral("ja") : QStringLiteral("NEIN"));
    out << QStringLiteral("    Alpha: Mitte %1   Randmitte oben %2   Ecke(0,0) %3\n")
               .arg(picture.pixelColor(picture.width() / 2, picture.height() / 2).alpha())
               .arg(picture.pixelColor(picture.width() / 2, 0).alpha())
               .arg(picture.pixelColor(0, 0).alpha());
    out << QStringLiteral("    Schatten: %1\n")
               .arg(window.shadow() ? QStringLiteral("Objekt vorhanden, oberste Kachel %1x%2")
                                          .arg(window.shadow()->topTile()
                                                   ? window.shadow()->topTile()->image().width()
                                                   : 0)
                                          .arg(window.shadow()->topTile()
                                                   ? window.shadow()->topTile()->image().height()
                                                   : 0)
                                    : QStringLiteral("keiner"));
}

void saveCorner(const QImage &picture, const QString &directory, const QString &name)
{
    // Auf weißem Grund, damit die Durchsichtigkeit sichtbar wird, und
    // zwanzigfach ohne Glättung: Was hier weich aussieht, ist weich gerechnet.
    QImage laid(picture.size(), QImage::Format_ARGB32);
    laid.fill(Qt::white);
    {
        QPainter painter(&laid);
        painter.drawImage(0, 0, picture);
    }
    laid.copy(0, 0, 24, 24)
        .scaled(480, 480, Qt::KeepAspectRatio, Qt::FastTransformation)
        .save(QDir(directory).filePath(name));
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    const QString directory = app.arguments().value(1, QStringLiteral("."));
    const QString secondTheme = app.arguments().value(2);
    QDir().mkpath(directory);

    const QTemporaryDir tmp;
    Store store(tmp.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen\n";
        return 1;
    }

    auto *window = new CaptureWindow(&store);
    window->showCapture();

    // Dem Compositor Zeit geben, dem Fenster seine Fläche zu vergeben: **vorher
    // steht das Bildpunktverhältnis nicht fest**, und der Sprung kommt ohne
    // Größenänderung daher (SPEC 3.2, Punkt 7).
    QTimer::singleShot(1500, &app, [&] {
        out << "=== #83, Sonde 2: das echte Erfassungsfenster ===\n";
        out << "Plattform       : " << app.platformName() << "\n";
        out << "QT_SCALE_FACTOR : "
            << qEnvironmentVariable("QT_SCALE_FACTOR", QStringLiteral("(nicht gesetzt)")) << "\n";
        if (QScreen *screen = app.primaryScreen()) {
            out << "Bildschirm      : " << screen->geometry().width() << "x"
                << screen->geometry().height() << " logisch, DPR " << screen->devicePixelRatio()
                << "\n";
        }
        out << "Weichzeichnende Sitzung: "
            << (capture::sessionBlursBehindWindows() ? "ja — durchscheinende Fassung"
                                                     : "nein — Auswahlpfad opaque")
            << "\n";
        out << "\n########## A — nach dem ersten Zeigen (AK 3, AK 4) ##########\n";
        report(out, *window, QStringLiteral("Ruhezustand, fünf Zeilen"));
        window->grab().toImage().save(QDir(directory).filePath(QStringLiteral("fenster-ruhe.png")));
        saveCorner(window->grab().toImage(), directory, QStringLiteral("ecke-ruhe.png"));

        // ------------------------------------------------------------ AK 5
        // Die Region ist die Maske, und die hängt an der Fenstergröße. Gemessen
        // wird die Folge fünf → acht → fünf: Was das Fenster beim Tippen tut.
        out << "\n########## B — folgt die Maskenregion der Größe? (AK 5) ##########\n";
        auto *text = window->findChild<QPlainTextEdit *>();
        const bool opaque = !capture::sessionBlursBehindWindows();
        const QString theme = QStringLiteral("default");
        for (const QString &content :
             {QString(),
              QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"),
              QString()}) {
            text->setPlainText(content);
            QCoreApplication::processEvents();
            const QRegion region = hullMask(theme, window->size(), opaque);
            out << QStringLiteral("    Fensterhöhe %1  →  Region %2 Rechteck(e), Hüllrechteck "
                                  "%3x%4, %5 Bildpunkte\n")
                       .arg(window->height(), 4)
                       .arg(region.rectCount())
                       .arg(region.boundingRect().width())
                       .arg(region.boundingRect().height())
                       .arg(region.boundingRect().width() * region.boundingRect().height());
        }
        out << "    Lesart: Die Höhe des Hüllrechtecks folgt der Fensterhöhe. Ob eine späte\n"
               "    Anmeldung die Region eines **laufenden** Weichzeichners noch ändert, sagt\n"
               "    diese Messung nicht — das misst Sonde 3.\n";

        // ------------------------------------------------------------ AK 9
        out << "\n########## C — eckiges Theme, eckige Ecken (AK 9) ##########\n";
        window->reloadDesktopTheme(QStringLiteral("denkzettel-pruef-eckig"));
        QCoreApplication::processEvents();
        report(out, *window, QStringLiteral("denkzettel-pruef-eckig"));
        window->grab().toImage().save(
            QDir(directory).filePath(QStringLiteral("fenster-eckig.png")));
        saveCorner(window->grab().toImage(), directory, QStringLiteral("ecke-eckig.png"));

        // ------------------------------------------------------------ AK 10
        out << "\n########## D — Theme-Wechsel im Betrieb (AK 10) ##########\n";
        for (const QString &name : {QStringLiteral("default"),
                                    secondTheme.isEmpty() ? QStringLiteral("CachyOS-Nord-round")
                                                          : secondTheme}) {
            window->reloadDesktopTheme(name);
            QCoreApplication::processEvents();
            const QMargins margins = window->layout()->contentsMargins();
            const QRegion region = hullMask(name, window->size(), opaque);
            out << QStringLiteral("  Theme %1\n").arg(name);
            out << QStringLiteral("    Innenrand links %1  Region %2x%3\n")
                       .arg(margins.left())
                       .arg(region.boundingRect().width())
                       .arg(region.boundingRect().height());
            report(out, *window, QStringLiteral("Fensterbild"));
            window->grab().toImage().save(
                QDir(directory).filePath(QStringLiteral("fenster-theme-%1.png").arg(name)));
            saveCorner(window->grab().toImage(),
                       directory,
                       QStringLiteral("ecke-theme-%1.png").arg(name));
        }

        out.flush();
        delete window;
        app.quit();
    });

    return app.exec();
}
