/*
 * UX-Entscheidung zur Textfarbe (#100), Messung 3: derselbe Vergleich auf dem
 * zweiten Farbweg — der `colors`-Datei des Desktop-Themes.
 *
 * Beide Vorprüfungen rechnen über die 19 Farbschemata. Seit #85 kommt die
 * Schrift aber gar nicht von dort, sobald das Desktop-Theme eine eigene
 * `colors`-Datei mitbringt: dann zieht `KSvg::Svg::color()` die Farbe aus dieser
 * Datei. Vier der acht installierten Themes bringen eine mit. Für sie gilt die
 * Schema-Rechnung nicht, und die Entscheidung hängt an Zahlen, die niemand
 * erhoben hat.
 *
 * Gemessen wird auf demselben Weg wie im Bau: über `KSvg::Svg::color()` mit
 * gesetztem Farbsatz, gegen die Flächen, die `dialogs/background` (Hülle) und
 * `widgets/lineedit`/`base` (Feld) tatsächlich zeichnen.
 *
 * Aufruf: themefarbsonde
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QTextStream>

#include <cmath>

namespace
{
constexpr QLatin1StringView ThemePath("plasma/desktoptheme");

double channel(int c)
{
    const double v = c / 255.0;
    return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
}

double luminance(const QColor &c)
{
    return 0.2126 * channel(c.red()) + 0.7152 * channel(c.green()) + 0.0722 * channel(c.blue());
}

double contrast(const QColor &a, const QColor &b)
{
    const double la = luminance(a);
    const double lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

QStringList allThemes()
{
    QStringList names;
    const QStringList roots = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QString(ThemePath),
                                                        QStandardPaths::LocateDirectory);
    for (const QString &root : roots) {
        for (const QString &name : QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            if (!names.contains(name)) {
                names << name;
            }
        }
    }
    return names;
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    const QSize windowSize(600, 174);
    const QRect fieldRect(28, 40, 544, 90);

    out << "Je Theme: die beiden Textfarben und der Grund, den der Text tatsaechlich traegt.\n";
    out << "Der Grund ist geschichtet gemessen (Huelle, darueber das Feld) ueber mittlerem\n";
    out << "Grau 128,128,128 als Bildschirmhintergrund — teildeckende Grafiken lassen ihn durch.\n\n";

    out << QStringLiteral("%1 %2 %3 %4 %5 %6 %7\n")
               .arg(QStringLiteral("Theme"), -24)
               .arg(QStringLiteral("Deckung"), 8)
               .arg(QStringLiteral("Grund"), 9)
               .arg(QStringLiteral("WindowText"), 11)
               .arg(QStringLiteral("ViewText"), 9)
               .arg(QStringLiteral("K(WT)"), 8)
               .arg(QStringLiteral("K(VT)"), 8);

    for (const QString &theme : allThemes()) {
        KSvg::ImageSet set(theme, QString(ThemePath));

        KSvg::FrameSvg hull;
        hull.setImageSet(&set);
        hull.setImagePath(QStringLiteral("dialogs/background"));
        hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        hull.setColorSet(KSvg::Svg::Window);
        hull.resizeFrame(QSizeF(windowSize));

        KSvg::FrameSvg field;
        field.setImageSet(&set);
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.resizeFrame(QSizeF(fieldRect.size()));

        if (!hull.isValid() || !field.isValid()) {
            out << theme << " — Huelle oder Feld loest nicht auf\n";
            continue;
        }

        // Die Deckung der Feldflaeche allein, ohne Grund darunter.
        QImage bare(fieldRect.size(), QImage::Format_ARGB32);
        bare.fill(Qt::transparent);
        {
            QPainter p(&bare);
            p.drawPixmap(0, 0, field.framePixmap());
        }
        const int alpha = bare.pixelColor(bare.width() / 2, bare.height() / 2).alpha();

        // Der Grund, den der Text traegt: Bildschirm, Huelle, Feld uebereinander.
        QImage picture(windowSize, QImage::Format_ARGB32);
        picture.fill(QColor(128, 128, 128));
        {
            QPainter p(&picture);
            p.drawPixmap(0, 0, hull.framePixmap());
            p.drawPixmap(fieldRect.topLeft(), field.framePixmap());
        }
        const QColor ground = picture.pixelColor(fieldRect.center());

        // Die beiden Textfarben auf dem Weg des Baus: Svg::color() am Rahmen,
        // der die Farbquelle des Themes kennt.
        const QColor windowText = hull.color(KSvg::Svg::Text);
        const QColor viewText = hull.color(KSvg::Svg::ViewText);

        out << QStringLiteral("%1 %2 %3 %4 %5 %6 %7\n")
                   .arg(theme, -24)
                   .arg(alpha, 8)
                   .arg(ground.name(), 9)
                   .arg(windowText.name(), 11)
                   .arg(viewText.name(), 9)
                   .arg(contrast(windowText, ground), 8, 'f', 2)
                   .arg(contrast(viewText, ground), 8, 'f', 2);
    }

    return 0;
}
