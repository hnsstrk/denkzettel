/*
 * Fokuszustand des Textfeldes (#100, Fund vom 07.08.2026), Messung 1:
 * was die vier Vorsätze von `widgets/lineedit` über die **ganze** Fläche
 * zeichnen.
 *
 * Die erste Messung (UX1) hat zwei Punkte abgetastet — Mitte und linke Kante.
 * Das genügt für eine Meldung und nicht für eine Zusicherung: Eine dünne Linie
 * an anderer Stelle bliebe unsichtbar, und die Aussage „`focus` zeichnet unter
 * `default` gar nichts" hinge an zwei Bildpunkten. Hier wird deshalb je Vorsatz
 * die **größte** Deckung über die gesamte Fläche gesucht, dazu die Zahl der
 * Bildpunkte mit Deckung über 128 und die Dicke des gezeichneten Randes.
 *
 * Zweite Frage derselben Messung: ob `hasElementPrefix()` einen fehlenden
 * Vorsatz **im vorhandenen Theme** verlässlich meldet. Für einen fehlenden
 * Theme-Namen tut es das nicht (Vorprüfung A, F1: KSvg fällt auf `default`
 * zurück und meldet trotzdem `true`). Ob dieselbe Falle für den Vorsatz gilt,
 * entscheidet, ob der Bau eine Fallunterscheidung braucht.
 *
 * Aufruf: fokussonde
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

    out << "Je Theme und Vorsatz: was ueber die ganze Flaeche gezeichnet wird.\n";
    out << "hasPrefix = was hasElementPrefix() meldet. maxDeckung/deckende Punkte ueber\n";
    out << "die gesamte Flaeche. Randdicke = Zeilen von oben, bis die Deckung unter 128\n";
    out << "faellt (0 heisst: schon die erste Zeile traegt nichts).\n\n";

    out << QStringLiteral("%1 %2 %3 %4 %5 %6 %7\n")
               .arg(QStringLiteral("Theme"), -24)
               .arg(QStringLiteral("Vorsatz"), -11)
               .arg(QStringLiteral("hasPrefix"), 10)
               .arg(QStringLiteral("maxDeck"), 8)
               .arg(QStringLiteral("Pkt>128"), 9)
               .arg(QStringLiteral("Randdicke"), 10)
               .arg(QStringLiteral("Randfarbe"), 10);

    for (const QString &theme : allThemes()) {
        KSvg::ImageSet set(theme, QString(ThemePath));

        for (const QString &prefix : {QStringLiteral("base"),
                                      QStringLiteral("hover"),
                                      QStringLiteral("focus"),
                                      QStringLiteral("focusframe")}) {
            KSvg::FrameSvg field;
            field.setImageSet(&set);
            field.setImagePath(QStringLiteral("widgets/lineedit"));
            field.setElementPrefix(prefix);
            field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
            field.resizeFrame(QSizeF(fieldSize));

            const bool hasPrefix = field.hasElementPrefix(prefix);

            QImage picture(fieldSize, QImage::Format_ARGB32);
            picture.fill(Qt::transparent);
            QPainter painter(&picture);
            painter.drawPixmap(0, 0, field.framePixmap());
            painter.end();

            int maxAlpha = 0;
            int opaque = 0;
            for (int y = 0; y < picture.height(); ++y) {
                for (int x = 0; x < picture.width(); ++x) {
                    const int a = qAlpha(picture.pixel(x, y));
                    maxAlpha = std::max(maxAlpha, a);
                    if (a > 128) {
                        ++opaque;
                    }
                }
            }

            // Dicke des oberen Randes an der Mittelspalte.
            const int column = picture.width() / 2;
            int thickness = 0;
            while (thickness < picture.height() && qAlpha(picture.pixel(column, thickness)) > 128) {
                ++thickness;
            }
            const QColor edge = thickness > 0 ? picture.pixelColor(column, 0) : QColor(Qt::transparent);

            out << QStringLiteral("%1 %2 %3 %4 %5 %6 %7\n")
                       .arg(theme, -24)
                       .arg(prefix, -11)
                       .arg(hasPrefix ? QStringLiteral("ja") : QStringLiteral("nein"), 10)
                       .arg(maxAlpha, 8)
                       .arg(opaque, 9)
                       .arg(thickness, 10)
                       .arg(thickness > 0 ? edge.name() : QStringLiteral("—"), 10);
        }
    }

    return 0;
}
