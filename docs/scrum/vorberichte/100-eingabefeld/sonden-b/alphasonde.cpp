// Die Deckung des Feldes gegen die der Hülle — die Zahl, an der die beiden
// Prüfsätze hängen, die den Fenstermittelpunkt abtasten.
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <QApplication>
#include <QTextStream>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    for (const QString &theme : {QStringLiteral("default"), QStringLiteral("breeze-dark"),
                                 QStringLiteral("CachyOS-Nord-round"), QStringLiteral("cachyos-emerald")}) {
        KSvg::ImageSet set(theme, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg hull;
        hull.setImageSet(&set);
        hull.setImagePath(QStringLiteral("dialogs/background"));
        hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        hull.setColorSet(KSvg::Svg::Window);
        hull.resizeFrame(QSizeF(600, 178));
        KSvg::FrameSvg field;
        field.setImageSet(&set);
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.resizeFrame(QSizeF(564, 98));
        const QImage h = hull.framePixmap().toImage();
        const QImage f = field.framePixmap().toImage();
        out << theme << ": Huelle Deckung Mitte " << qAlpha(h.pixel(300, 89))
            << "  Feld Deckung Mitte " << qAlpha(f.pixel(282, 49)) << "\n";
    }
    return 0;
}
