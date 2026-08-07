// Messung B, zweiter Teil: das Feld auf der Hülle, nicht auf Schwarz.
//
// Die erste Sonde hat Hülle und Feld je einzeln über schwarzem Grund gemessen.
// Der Kunde sieht sie übereinander, und beide sind teildeckend — die Zahl aus
// AK 6 („1,03–1,10 : 1") kann deshalb nur aus der Schichtung stammen.
//
// Gemessen wird je Theme: Grund → Hülle → Feld, und zwar über **zwei** Gründen
// (schwarz und weiß). Zwei, weil eine teildeckende Hülle den Grund
// durchscheinen lässt: Was das Feld abhebt, hängt dann am Bildschirmhintergrund
// des Kunden — und genau das entscheidet, ob AK 6 eine prüfbare Zusicherung
// ist.

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QTextStream>

namespace
{
constexpr QLatin1StringView ThemePath("plasma/desktoptheme");

double luminance(const QColor &c)
{
    auto channel = [](double v) {
        v /= 255.0;
        return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
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

    if (argc > 1) {
        qputenv("XDG_DATA_DIRS", QByteArray(argv[1]) + ':' + qgetenv("XDG_DATA_DIRS"));
    }

    // Fenstermaß und Feldmaß wie im Bau: 600 breit, das Feld innerhalb der
    // Innenabstände 12/10/8 zuzüglich Theme-Rand.
    const QSize windowSize(600, 174);
    const QRect fieldRect(28, 40, 544, 90);

    out << "Theme | Grund | Huelle | Feldflaeche | Feldkante | K(Fl:Hu) | K(Ka:Hu)\n";

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
            out << theme << " | — | Huelle oder Feld loest nicht auf\n";
            continue;
        }

        for (const QColor ground : {QColor(0, 0, 0), QColor(255, 255, 255)}) {
            QImage picture(windowSize, QImage::Format_ARGB32);
            picture.fill(ground);
            QPainter painter(&picture);
            painter.drawPixmap(0, 0, hull.framePixmap());
            // Neben dem Feld, auf der Hülle: die Lücke über dem Feld.
            const QColor hullColour = picture.pixelColor(windowSize.width() / 2, fieldRect.top() - 6);
            painter.drawPixmap(fieldRect.topLeft(), field.framePixmap());
            painter.end();

            const QColor fieldColour = picture.pixelColor(fieldRect.center());
            const QColor edgeColour = picture.pixelColor(fieldRect.left(), fieldRect.center().y());

            out << theme << " | " << (ground == QColor(0, 0, 0) ? "schwarz" : "weiss") << " | "
                << hullColour.name() << " | " << fieldColour.name() << " | " << edgeColour.name()
                << " | " << QString::number(contrast(fieldColour, hullColour), 'f', 3) << " | "
                << QString::number(contrast(edgeColour, hullColour), 'f', 3) << "\n";
        }
    }

    return 0;
}
