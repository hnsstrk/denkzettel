// UX-Entscheidung zur Textfarbe (#100), Messung 4: die Deckung der Hülle je
// Theme. Sie entscheidet mit, ob unter dem Text überhaupt eine Fensterfläche
// liegt — wo die Hülle durchscheint, trägt den Text der Bildschirminhalt, und
// dort sichert SPEC 3.1 ohnehin keine Lesbarkeit zu.
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QTextStream>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    const QString path = QStringLiteral("plasma/desktoptheme");
    QStringList names;
    for (const QString &root : QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, path, QStandardPaths::LocateDirectory)) {
        for (const QString &n : QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            if (!names.contains(n)) names << n;
        }
    }
    out << "Theme | Deckung Huelle Mitte (von 255)\n";
    for (const QString &theme : names) {
        KSvg::ImageSet set(theme, path);
        KSvg::FrameSvg hull;
        hull.setImageSet(&set);
        hull.setImagePath(QStringLiteral("dialogs/background"));
        hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        hull.setColorSet(KSvg::Svg::Window);
        hull.resizeFrame(QSizeF(600, 174));
        QImage img(600, 174, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter p(&img);
        p.drawPixmap(0, 0, hull.framePixmap());
        p.end();
        out << theme << " | " << img.pixelColor(300, 87).alpha() << "\n";
    }
    return 0;
}
