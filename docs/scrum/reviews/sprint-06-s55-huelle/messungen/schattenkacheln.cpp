// Messung 6 — Welcher Aufruf schneidet eine Schattenkachel heraus?
//
// Diese Sonde ist aus einem Fehler in diesem Strang entstanden, nicht aus einer
// Vorsichtsmaßnahme. Der erste Bau übergab dem Compositor die acht Kacheln über
// `KSvg::Svg::pixmap(elementID)`. Der Aufruf sieht richtig aus, nimmt die
// Element-Kennung entgegen — und **ignoriert sie**: Zurück kommt jedes Mal das
// gesamte Bild in der Größe des SVG.
//
// Warum das nicht auffiel, ist der eigentliche Punkt:
//
//   * `KWindowShadow::create()` nimmt die falschen Kacheln **an** und meldet
//     wahr. Der Rückgabewert — die Zusicherung aus AK 7 — war grün.
//   * Der Schatten liegt außerhalb des Fensters, also zeigt ihn kein Bild
//     dieses Projekts, weder offscreen noch am Compositor.
//   * Die Zusicherung im Test verglich die gebundene Kachel gegen **denselben**
//     Aufruf. Sie war grün, weil beide Seiten denselben Fehler machten.
//
// Das ist die Bauart, gegen die CLAUDE.md seine Prüfhaltung fasst. Die Heilung
// ist `image(elementSize(id), id)`, und die Zusicherung im Test hält seither
// zusätzlich fest, dass die Eckkachel **nicht** so groß ist wie die obere —
// die Aussage, an der ein zurückfallender Bau scheitert.

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QTextStream>

namespace
{
QTextStream out(stdout);
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    const QStringList elements{QStringLiteral("shadow-topleft"),
                               QStringLiteral("shadow-top"),
                               QStringLiteral("shadow-left"),
                               QStringLiteral("shadow-bottomright")};

    out << "Messung 6 — Welcher Aufruf schneidet eine Schattenkachel? (#55, AK 7)\n";
    out << "=====================================================================\n\n";

    for (const QString &theme : {QStringLiteral("default"), QStringLiteral("CachyOS-Nord-round")}) {
        KSvg::ImageSet set(theme, QStringLiteral("plasma/desktoptheme"));

        KSvg::FrameSvg tiles;
        tiles.setImageSet(&set);
        tiles.setImagePath(QStringLiteral("dialogs/background"));
        tiles.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        tiles.setElementPrefix(QStringLiteral("shadow"));
        tiles.resizeFrame(QSizeF(600, 174));

        out << theme << "  —  Svg::size() = " << tiles.size().width() << "x" << tiles.size().height() << "\n";
        out << "   Element              elementSize   pixmap(id)   image(elementSize, id)\n";
        out << "   ---------------------------------------------------------------------\n";

        for (const QString &element : elements) {
            const QSize natural = tiles.elementSize(element).toSize();
            const QSize wrong = tiles.pixmap(element).size();
            const QSize right = tiles.image(natural, element).size();

            out << "   " << element.leftJustified(21)
                << QStringLiteral("%1x%2").arg(natural.width()).arg(natural.height()).leftJustified(14)
                << QStringLiteral("%1x%2").arg(wrong.width()).arg(wrong.height()).leftJustified(13)
                << QStringLiteral("%1x%2").arg(right.width()).arg(right.height()) << "\n";
        }
        out << "\n";
        out.flush();
    }

    out << "Befund: `pixmap(id)` liefert für **jedes** Element dieselbe Größe — das\n"
           "ganze Bild. `image(elementSize(id), id)` liefert das Element. Ein Bau auf\n"
           "dem ersten Weg übergibt dem Compositor achtmal den kompletten Schatten;\n"
           "`KWindowShadow::create()` nimmt das an und meldet wahr.\n";
    out.flush();

    return 0;
}
