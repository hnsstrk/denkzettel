/*
 * Fokuszustand des Textfeldes (#100), Messung 2: wo genau die Fokuskante liegt.
 *
 * Die Punktmessung UX1 hat für `default` gemeldet, `focus` zeichne nichts. Sie
 * hat x=1 abgetastet; die Kante liegt dort bei x=0. Die Flächenmessung UX7 hat
 * den Fehler aufgedeckt. Diese Sonde legt die Lage offen, damit die Aussage an
 * einer Geometrie hängt und nicht an einem Abtastpunkt: je Vorsatz das
 * Deckungsprofil der ersten acht Spalten und Zeilen ab dem Rand.
 *
 * Aufruf: kantensonde
 */
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
    const QString themePath = QStringLiteral("plasma/desktoptheme");
    QStringList names;
    for (const QString &root : QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, themePath, QStandardPaths::LocateDirectory)) {
        for (const QString &n : QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            if (!names.contains(n)) names << n;
        }
    }
    const QSize size(544, 90);
    out << "Deckungsprofil vom Rand nach innen, je acht Schritte. links = Spalten x=0..7 in\n";
    out << "der Zeilenmitte, oben = Zeilen y=0..7 in der Spaltenmitte.\n\n";
    for (const QString &theme : names) {
        KSvg::ImageSet set(theme, themePath);
        for (const QString &prefix : {QStringLiteral("base"), QStringLiteral("hover"), QStringLiteral("focus"), QStringLiteral("focusframe")}) {
            KSvg::FrameSvg f;
            f.setImageSet(&set);
            f.setImagePath(QStringLiteral("widgets/lineedit"));
            f.setElementPrefix(prefix);
            f.setEnabledBorders(KSvg::FrameSvg::AllBorders);
            f.resizeFrame(QSizeF(size));
            if (!f.hasElementPrefix(prefix)) {
                out << QStringLiteral("%1 %2 fehlt in diesem Theme\n").arg(theme, -24).arg(prefix, -11);
                continue;
            }
            QImage img(size, QImage::Format_ARGB32);
            img.fill(Qt::transparent);
            QPainter p(&img);
            p.drawPixmap(0, 0, f.framePixmap());
            p.end();
            QStringList links;
            QStringList oben;
            for (int i = 0; i < 8; ++i) {
                links << QString::number(qAlpha(img.pixel(i, size.height() / 2)));
                oben << QString::number(qAlpha(img.pixel(size.width() / 2, i)));
            }
            const QColor stark = img.pixelColor(0, size.height() / 2);
            out << QStringLiteral("%1 %2 links %3 | oben %4 | Farbe x=0 %5\n")
                       .arg(theme, -24)
                       .arg(prefix, -11)
                       .arg(links.join(QLatin1Char(' ')), -32)
                       .arg(oben.join(QLatin1Char(' ')), -32)
                       .arg(stark.name());
        }
    }
    return 0;
}
