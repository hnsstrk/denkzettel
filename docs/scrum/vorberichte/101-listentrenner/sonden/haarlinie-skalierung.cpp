/*
 * Sonde zur Vorprüfung von #101, Messung B (07.08.2026).
 *
 * AK 7 verlangt ein Bild unter der Skalierung des Kunden (gemessen 04.08.2026:
 * 1,6). AK 1 bis AK 3 sprechen dagegen von „der letzten Bildpunktzeile" und von
 * „12 px vom Rand" — Angaben in logischen Bildpunkten.
 *
 * Prüffrage 1: Wie breit wird eine 1 px hohe Linie bei 1,6, und trägt sie dort
 *   die volle Farbe oder eine gemischte?
 * Prüffrage 2: Wo landen die logischen 12 px in Gerätebildpunkten?
 *
 * Gemessen wird an einer QPixmap mit gesetztem devicePixelRatio — derselbe Weg,
 * den `grab()` in den Bildläufern nimmt.
 */

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QTextStream>

namespace
{
/** Die Zeilen des Bildes, in denen überhaupt Linienfarbe steht. */
QString paintedRows(const QImage &picture, const QColor &background, int column)
{
    QString text;
    for (int y = 0; y < picture.height(); ++y) {
        const QColor pixel = picture.pixelColor(column, y);
        if (pixel != background) {
            text += QStringLiteral("y=%1 %2  ").arg(y).arg(pixel.name());
        }
    }
    return text.isEmpty() ? QStringLiteral("(keine)") : text;
}

/** Die Spalten, in denen in Zeile `row` Linienfarbe steht — erste und letzte. */
QString paintedColumns(const QImage &picture, const QColor &background, int row)
{
    int first = -1;
    int last = -1;
    for (int x = 0; x < picture.width(); ++x) {
        if (picture.pixelColor(x, row) != background) {
            if (first < 0) {
                first = x;
            }
            last = x;
        }
    }
    return first < 0 ? QStringLiteral("(keine)")
                     : QStringLiteral("x=%1 bis x=%2").arg(first).arg(last);
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QColor background(0x14, 0x16, 0x18); // Listengrund Breeze Dark
    const QColor line(0x42, 0x44, 0x46); // Grund und Text im Verhältnis 0,20

    // Eine Notizzeile, wie der Delegate sie bekommt: 300 × 60 logische Bildpunkte.
    const QSize logical(300, 60);

    for (const double ratio : {1.0, 1.6, 2.0}) {
        QPixmap canvas(QSize(qRound(logical.width() * ratio), qRound(logical.height() * ratio)));
        canvas.setDevicePixelRatio(ratio);
        canvas.fill(background);

        {
            QPainter painter(&canvas);
            // Die Haarlinie an der Unterkante, eingerückt auf die Textkante:
            // 12 px links und rechts, 1 px hoch, in logischen Koordinaten.
            painter.fillRect(QRect(12, logical.height() - 1, logical.width() - 24, 1), line);
        }

        const QImage picture = canvas.toImage();
        out << QStringLiteral("### Skalierung %1 — Bild %2×%3 Gerätebildpunkte\n")
                   .arg(ratio)
                   .arg(picture.width())
                   .arg(picture.height());
        out << "  Zeilen mit Linienfarbe (Spalte 150): " << paintedRows(picture, background, 150) << "\n";
        out << "  Spalten in der untersten bemalten Zeile: "
            << paintedColumns(picture, background, picture.height() - 1) << "\n";
        out << QStringLiteral("  logische 12 px entsprechen %1 Gerätebildpunkten\n\n").arg(12 * ratio);
    }

    out.flush();
    return 0;
}
