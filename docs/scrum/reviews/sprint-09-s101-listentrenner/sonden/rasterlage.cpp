// Messsonde zu Befund N1 des karpathy-Nachlaufs (Issue #101).
//
// Frage: Trägt der Term `std::round(top * ratio) / ratio` in `hairline()`
// etwas? Er soll die Oberkante der Haarlinie auf eine Gerätebildpunktgrenze
// legen. Gemessen wird, ob das gerasterte Ergebnis MIT und OHNE ihn irgendwo
// auseinandergeht.
//
// Gemessen wird über drei Achsen, weil jede einzeln eine Falle hat:
//
//   1. Das Verhältnis — bei ganzen Verhältnissen kann nichts schiefgehen, der
//      Fall liegt zwischen ihnen.
//   2. Die Zeilenlage `top` — der Fehler von L9 hing genau daran.
//   3. Der MALERURSPRUNG. Das Sichtfeld der Liste beginnt bei logisch 48, also
//      bei 76,8 Gerätebildpunkten unter 1,6. Ein Term, der gegen den Ursprung
//      des Malers rundet, rundet dann gegen etwas, das selbst zwischen zwei
//      Bildpunkten liegt. Eine Sonde, die nur bei Ursprung 0 misst, könnte den
//      Term für wirksam halten, obwohl er es im Fenster nicht ist.
//
// Ausgegeben werden je Fall die belegten Gerätebildpunktzeilen als [Anfang,
// Höhe] — einmal mit Term, einmal ohne.
//
// Aufruf: rasterlage

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
constexpr int Breite = 40;
// Hoch genug, dass auch der verschobene Malerursprung samt aller Zeilenlagen
// hineinpasst. Beim ersten Lauf war es 40, und die Hälfte der Lagen fiel aus
// dem Bild — die Sonde meldete „kein Unterschied" über lauter leere Fälle.
constexpr int Hoehe = 100;

/** Die gebaute Fassung: ganzzahlige Höhe UND ausgerichtete Oberkante. */
QRectF mitTerm(qreal ratio, int top)
{
    const qreal rows = std::max(1.0, std::round(ratio));
    return QRectF(0, std::round(top * ratio) / ratio, Breite, rows / ratio);
}

/** Dieselbe Fassung ohne die Ausrichtung der Oberkante. */
QRectF ohneTerm(qreal ratio, int top)
{
    const qreal rows = std::max(1.0, std::round(ratio));
    return QRectF(0, top, Breite, rows / ratio);
}

/**
 * Rastert `rect` so, wie der Delegate es tut: aliased fillRect auf ein Bild
 * mit `ratio`, wobei der Maler um `ursprung` logische Punkte verschoben ist.
 *
 * Zurück kommt der belegte Bereich als „Anfang+Höhe" in GERÄTEbildpunkten.
 */
QString raster(qreal ratio, const QRectF &rect, qreal ursprung)
{
    QImage bild(QSize(qRound(Breite * ratio), qRound(Hoehe * ratio)), QImage::Format_ARGB32_Premultiplied);
    bild.setDevicePixelRatio(ratio);
    bild.fill(Qt::white);

    {
        QPainter maler(&bild);
        maler.translate(0, ursprung);
        maler.fillRect(rect, Qt::black);
    }

    int anfang = -1;
    int hoehe = 0;
    for (int y = 0; y < bild.height(); ++y) {
        if (qGray(bild.pixel(1, y)) < 128) {
            if (anfang < 0) {
                anfang = y;
            }
            ++hoehe;
        }
    }
    return anfang < 0 ? QStringLiteral("     —") : QStringLiteral("%1+%2").arg(anfang, 4).arg(hoehe);
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    const QList<qreal> verhaeltnisse = {1.0, 1.25, 1.4, 1.5, 1.6, 2.0, 2.5};
    // 0 als Laborfall, 48 als der echte Anfang des Sichtfelds der Liste.
    const QList<qreal> urspruenge = {0.0, 48.0};

    int unterschiede = 0;
    int faelle = 0;
    int leere = 0;

    for (const qreal ursprung : urspruenge) {
        printf("\n=== Malerursprung %.0f logische Punkte (= %.1f Gerätebildpunkte bei 1,6) ===\n",
               ursprung, ursprung * 1.6);
        for (const qreal ratio : verhaeltnisse) {
            printf("\nVerhältnis %.2f — Höhe %g Gerätebildpunktzeilen, %g logische Punkte\n", ratio,
                   std::max(1.0, std::round(ratio)), std::max(1.0, std::round(ratio)) / ratio);
            printf("  %-6s %-12s %-12s %s\n", "top", "mit Term", "ohne Term", "");
            for (int top = 0; top < 20; ++top) {
                const QString a = raster(ratio, mitTerm(ratio, top), ursprung);
                const QString b = raster(ratio, ohneTerm(ratio, top), ursprung);
                ++faelle;
                if (a.trimmed() == QLatin1String("—")) {
                    ++leere;
                }
                const bool anders = a != b;
                if (anders) {
                    ++unterschiede;
                }
                printf("  %-6d %-12s %-12s %s\n", top, qPrintable(a), qPrintable(b),
                       anders ? "<-- UNTERSCHIED" : "");
            }
        }
    }

    printf("\n=== Bilanz ===\n");
    printf("Gemessene Lagen          : %d\n", faelle);
    printf("davon LEER (keine Linie) : %d\n", leere);
    printf("Lagen mit Unterschied    : %d\n", unterschiede);
    if (leere > 0) {
        printf("\nABBRUCH: %d Lagen haben gar keine Linie getroffen. Eine leere Lage meldet\n"
               "„kein Unterschied\", ohne etwas gemessen zu haben — das Urteil unten trüge nicht.\n",
               leere);
        return 1;
    }
    printf("→ Der Term %s.\n",
           unterschiede == 0 ? "ändert in KEINER der gemessenen Lagen etwas"
                             : "ändert in mindestens einer Lage etwas");
    return 0;
}
