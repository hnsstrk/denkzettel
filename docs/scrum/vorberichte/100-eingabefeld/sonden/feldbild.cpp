/*
 * Zweite Messsonde der Vorprüfung zu #100 (Bearbeiter A, 07.08.2026).
 *
 * Die erste hat gezeigt, dass `widgets/lineedit` unter **jedem** Namen
 * auflöst — auch unter den Prüf-Themes der Testsuite, die keine eigene Grafik
 * mitbringen, und unter einem Namen, auf den nichts hört. Daraus folgen zwei
 * Fragen, die eine Zusicherung tragen oder blind machen:
 *
 *   E) Zeichnet die zurückgefallene `default`-Grafik unter zwei Themes
 *      **verschiedene** Bildpunkte, weil das Theme sie über seine eigene
 *      `colors`-Datei umfärbt? Wenn nein, kann F4 (Theme-Wechsel) an den
 *      mitgelieferten Themes nichts messen.
 *   F) Welcher offene Weg rückt den Text im Textfeld nach innen (F5), und was
 *      macht er mit der Höhenrechnung? `setViewportMargins()` ist protected.
 *
 * Aufruf: feldbild <Prüf-Theme-Verzeichnis> [<theme> …]
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>

#include <QApplication>
#include <QImage>
#include <QLayout>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>

namespace
{
QTextStream out(stdout);

QString show(const QColor &c)
{
    return QStringLiteral("%1,%2,%3/a%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

/** E) — die gezeichneten Bildpunkte der Feldgrafik je Theme. */
void fieldPixels(const QString &theme)
{
    auto set = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::FrameSvg field;
    field.setUsingRenderingCache(false);
    field.setImageSet(set.get());
    field.setImagePath(QStringLiteral("widgets/lineedit"));
    field.setElementPrefix(QStringLiteral("base"));
    field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    field.setColorSet(KSvg::Svg::View);
    field.resizeFrame(QSizeF(560, 90));

    QImage picture(560, 90, QImage::Format_ARGB32_Premultiplied);
    picture.fill(Qt::transparent);
    QPainter painter(&picture);
    field.paintFrame(&painter);
    painter.end();

    // Die Mitte ist die Fläche; der kräftigste Punkt der linken Kante ist die
    // Kontur — dieselben zwei Griffe wie in der Sonde vom 06.08.2026.
    const QColor surface = picture.pixelColor(280, 45);
    QColor edge;
    int bestAlpha = -1;
    for (int x = 0; x < 20; ++x) {
        const QColor c = picture.pixelColor(x, 45);
        if (c.alpha() > bestAlpha) {
            bestAlpha = c.alpha();
            edge = c;
        }
    }

    out << QStringLiteral("  %1: Fläche %2  Kante %3\n").arg(theme, show(surface), show(edge));
}

/** I) — Deckung der Hülle in der Fenstermitte, wo capturetest sie abgreift. */
void hullAlphaAtCentre(const QString &theme)
{
    auto set = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));
    set->setSelectors({QStringLiteral("opaque")});

    KSvg::FrameSvg hull;
    hull.setUsingRenderingCache(false);
    hull.setImageSet(set.get());
    hull.setImagePath(QStringLiteral("dialogs/background"));
    hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    hull.setColorSet(KSvg::Svg::Window);
    hull.resizeFrame(QSizeF(600, 174));

    const QImage picture = hull.framePixmap().toImage();
    out << QStringLiteral("  %1: Mitte %2\n").arg(theme, show(picture.pixelColor(300, 87)));
}

/** G) — trägt `setColorSet(View)` etwas, oder ist die Zeile Zierde? */
void colourSetMatters(const QString &theme)
{
    auto set = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));

    out << "  " << theme << ":\n";
    for (const auto &[name, colourSet] :
         {std::pair{QStringLiteral("View"), KSvg::Svg::View},
          std::pair{QStringLiteral("Window"), KSvg::Svg::Window}}) {
        KSvg::FrameSvg field;
        field.setUsingRenderingCache(false);
        field.setImageSet(set.get());
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.setColorSet(colourSet);
        field.resizeFrame(QSizeF(560, 90));

        QImage picture(560, 90, QImage::Format_ARGB32_Premultiplied);
        picture.fill(Qt::transparent);
        QPainter painter(&picture);
        field.paintFrame(&painter);
        painter.end();
        out << QStringLiteral("    colorSet %1: Fläche %2\n")
                   .arg(name, show(picture.pixelColor(280, 45)));
    }
}

/** H) — folgt die Feldgrafik der Skalierung von selbst? (die Falle aus #83) */
void ratioFollows(const QString &theme)
{
    auto set = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::FrameSvg field;
    field.setUsingRenderingCache(false);
    field.setImageSet(set.get());
    field.setImagePath(QStringLiteral("widgets/lineedit"));
    field.setElementPrefix(QStringLiteral("base"));
    field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    field.setColorSet(KSvg::Svg::View);
    field.resizeFrame(QSizeF(560, 90));

    out << QStringLiteral("  %1: Vorgabe-Verhältnis %2, framePixmap %3x%4\n")
               .arg(theme)
               .arg(field.devicePixelRatio())
               .arg(field.framePixmap().width())
               .arg(field.framePixmap().height());

    field.setDevicePixelRatio(1.6);
    field.resizeFrame(QSizeF(560, 90));
    out << QStringLiteral("     nach setDevicePixelRatio(1,6): framePixmap %1x%2, Ränder links %3\n")
               .arg(field.framePixmap().width())
               .arg(field.framePixmap().height())
               .arg(field.marginSize(KSvg::FrameSvg::LeftMargin));
}

/** F) — die offenen Wege, den Text nach innen zu rücken. */
void insetRoutes(Store &store, const QString &theme)
{
    out << "############ " << theme << "\n";

    // Ausgangslage, jedes Mal an einem frisch gebauten Fenster: ein Fenster,
    // an dem schon geschraubt wurde, misst den Rest der Schraube mit.
    {
        CaptureWindow window(&store);
        window.reloadDesktopTheme(theme);
        window.show();
        QCoreApplication::processEvents();
        auto *text = window.findChild<QPlainTextEdit *>();
        out << QStringLiteral("  Ausgangslage      : Fenster %1, Textbereich %2 hoch, viewport %3x%4 bei (%5,%6)\n")
                   .arg(window.height())
                   .arg(text->height())
                   .arg(text->viewport()->width())
                   .arg(text->viewport()->height())
                   .arg(text->viewport()->x())
                   .arg(text->viewport()->y());
    }

    {
        CaptureWindow window(&store);
        window.reloadDesktopTheme(theme);
        window.show();
        QCoreApplication::processEvents();
        auto *text = window.findChild<QPlainTextEdit *>();
        text->document()->setDocumentMargin(text->document()->documentMargin() + 6);
        QCoreApplication::processEvents();
        out << QStringLiteral("  documentMargin +6 : Fenster %1, Textbereich %2 hoch, viewport %3x%4 bei (%5,%6)\n")
                   .arg(window.height())
                   .arg(text->height())
                   .arg(text->viewport()->width())
                   .arg(text->viewport()->height())
                   .arg(text->viewport()->x())
                   .arg(text->viewport()->y());
    }

    {
        CaptureWindow window(&store);
        window.reloadDesktopTheme(theme);
        window.show();
        QCoreApplication::processEvents();
        auto *text = window.findChild<QPlainTextEdit *>();
        text->setContentsMargins(6, 6, 6, 6);
        text->updateGeometry();
        QCoreApplication::processEvents();
        out << QStringLiteral("  contentsMargins 6 : Fenster %1, Textbereich %2 hoch, viewport %3x%4 bei (%5,%6)\n")
                   .arg(window.height())
                   .arg(text->height())
                   .arg(text->viewport()->width())
                   .arg(text->viewport()->height())
                   .arg(text->viewport()->x())
                   .arg(text->viewport()->y());
    }

    // Der dritte Weg: das Textfeld in einen Halter mit Innenrand setzen. Nicht
    // gebaut, nur die Rechnung — der Halter läge zwischen Layout und Textfeld
    // und ist ein Eingriff in den Aufbau, den die beiden anderen nicht
    // brauchen.
    out << "\n";
}
}

int main(int argc, char **argv)
{
    // NOLINTNEXTLINE(misc-const-correctness)
    QApplication app(argc, argv);

    if (argc < 2) {
        out << "Aufruf: feldbild <Prüf-Theme-Verzeichnis> [<theme> …]\n";
        return 2;
    }

    const QByteArray bundled = QByteArray(argv[1]);
    const QByteArray existing = qgetenv("XDG_DATA_DIRS");
    qputenv("XDG_DATA_DIRS", existing.isEmpty() ? bundled : bundled + ':' + existing);

    out << "== E: Zeichnet die zurückgefallene default-Grafik je Theme verschieden? ==\n\n";
    out << "  (Rohe Bildpunkte der Feldgrafik, ohne Grund darunter.)\n\n";
    for (int i = 2; i < argc; ++i) {
        fieldPixels(QString::fromLocal8Bit(argv[i]));
    }
    out << "\n";

    out << "== I: Deckung der Hülle in der Fenstermitte (dort greift capturetest ab) ==\n\n";
    for (int i = 2; i < argc; ++i) {
        hullAlphaAtCentre(QString::fromLocal8Bit(argv[i]));
    }
    out << "\n";

    out << "== G: Trägt setColorSet(View) etwas? ==\n\n";
    for (int i = 2; i < argc; ++i) {
        colourSetMatters(QString::fromLocal8Bit(argv[i]));
    }
    out << "\n";

    out << "== H: Folgt die Feldgrafik der Skalierung von selbst? ==\n\n";
    for (int i = 2; i < argc; ++i) {
        ratioFollows(QString::fromLocal8Bit(argv[i]));
    }
    out << "\n";

    out << "== F: Die offenen Wege, den Text nach innen zu rücken ==\n\n";
    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store liess sich nicht oeffnen\n";
        return 1;
    }
    insetRoutes(store, QStringLiteral("denkzettel-test-schmal"));
    insetRoutes(store, QStringLiteral("denkzettel-test-breit"));

    return 0;
}
