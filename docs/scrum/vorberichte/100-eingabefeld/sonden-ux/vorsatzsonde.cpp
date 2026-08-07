/*
 * UX-Entscheidung zur Textfarbe (#100), Messung 1: Was zeichnet
 * `widgets/lineedit` unter den drei Vorsätzen, die Plasma dafür vorsieht?
 *
 * Anlass: Die Vorprüfung hat gemessen, dass fünf der acht installierten Themes
 * den Vorsatz `base` nur als Hauch zeichnen (Deckung 15 von 255), und daraus
 * die Grenze in AK 6 abgeleitet. Gemessen wurde dabei allein `base`. Die
 * Grafik trägt in allen fünf Themes zusätzlich `hover` und `focus` — und das
 * Erfassungsfenster von Denkzettel zeigt seinen Textbereich ausschließlich im
 * Fokus, weil es nur zum Tippen aufgeht.
 *
 * Gemessen wird deshalb je Theme und je Vorsatz die Deckung und die Farbe in
 * der Mitte und an der linken Kante — über schwarzem Grund, damit die Deckung
 * ablesbar bleibt.
 *
 * Aufruf: vorsatzsonde
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

namespace
{
constexpr QLatin1StringView ThemePath("plasma/desktoptheme");

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

    const QSize fieldSize(544, 90);

    out << "Theme | Vorsatz | gueltig | Deckung Mitte | Farbe Mitte | Deckung Kante | Farbe Kante\n";

    for (const QString &theme : allThemes()) {
        KSvg::ImageSet set(theme, QString(ThemePath));

        for (const QString &prefix : {QStringLiteral("base"), QStringLiteral("hover"), QStringLiteral("focus"), QStringLiteral("focusframe")}) {
            KSvg::FrameSvg field;
            field.setImageSet(&set);
            field.setImagePath(QStringLiteral("widgets/lineedit"));
            field.setElementPrefix(prefix);
            field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
            field.resizeFrame(QSizeF(fieldSize));

            const bool valid = field.isValid() && field.hasElementPrefix(prefix);

            QImage picture(fieldSize, QImage::Format_ARGB32);
            picture.fill(Qt::transparent);
            QPainter painter(&picture);
            painter.drawPixmap(0, 0, field.framePixmap());
            painter.end();

            const QColor middle = picture.pixelColor(fieldSize.width() / 2, fieldSize.height() / 2);
            const QColor edge = picture.pixelColor(1, fieldSize.height() / 2);

            out << theme << " | " << prefix << " | " << (valid ? "ja" : "nein") << " | " << middle.alpha()
                << " | " << middle.name() << " | " << edge.alpha() << " | " << edge.name() << "\n";
        }
    }

    return 0;
}
