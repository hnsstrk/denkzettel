/*
 * Fokuszustand des Textfeldes (#100), Messung 4: was die Schicht dem Kunden
 * bringt — die Abhebung des Feldes gegen die Hülle, mit und ohne sie.
 *
 * Gerechnet wie in der Entscheidung vom 07.08.2026: Hülle und Feld werden über
 * einem benannten Grund geschichtet, weil beide Grafiken durchscheinen können
 * und die Zahl sonst nicht reproduzierbar ist. Zwei Gründe (schwarz und weiss),
 * damit sichtbar bleibt, wie stark die Zahl am Bildschirminhalt hängt.
 *
 * Gemessen wird die stärkste Kante des Feldes gegen die Hülle daneben — das ist
 * die Grösse, die der Kundenbefund aus #100 meint („der Eingabebereich ist nicht
 * klar erkennbar"). Der Maßstab steht in SPEC 3.1: KRunners Feld hebt sich im
 * Sitzungsbild des Sprint-7-UI-Reviews mit 1,41 : 1 ab.
 *
 * Aufruf: gewinnsonde
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
}

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
    const QSize windowSize(600, 174);
    const QRect fieldRect(28, 40, 544, 90);

    out << "Abhebung des Feldbereichs gegen die Huelle daneben.\n";
    out << "Flaeche = die Feldmitte. beste ohne/mit = der groesste Wert, den der\n";
    out << "Feldrand in seinen ersten acht Spalten erreicht — das ist die Groesse, die\n";
    out << "der Kundenbefund meint. Fokuskante = allein die Kante der focus-Schicht.\n";
    out << "ohne = nur Vorsatz base (der Stand nach #100). mit = base, darueber focus.\n";
    out << "Maßstab: KRunners Feld hebt sich im Sitzungsbild mit 1,41 : 1 ab (SPEC 3.1).\n\n";
    out << QStringLiteral("%1 %2 %3 %4 %5 %6 %7\n")
               .arg(QStringLiteral("Theme"), -24)
               .arg(QStringLiteral("Grund"), -8)
               .arg(QStringLiteral("Flaeche"), 9)
               .arg(QStringLiteral("beste ohne"), 11)
               .arg(QStringLiteral("beste mit"), 11)
               .arg(QStringLiteral("Fokuskante"), 11)
               .arg(QStringLiteral("Gewinn"), 9);

    for (const QString &theme : names) {
        KSvg::ImageSet set(theme, themePath);

        KSvg::FrameSvg hull;
        hull.setImageSet(&set);
        hull.setImagePath(QStringLiteral("dialogs/background"));
        hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        hull.setColorSet(KSvg::Svg::Window);
        hull.resizeFrame(QSizeF(windowSize));

        auto feld = [&set, &fieldRect](const QString &prefix) {
            auto *f = new KSvg::FrameSvg;
            f->setImageSet(&set);
            f->setImagePath(QStringLiteral("widgets/lineedit"));
            f->setElementPrefix(prefix);
            f->setEnabledBorders(KSvg::FrameSvg::AllBorders);
            f->resizeFrame(QSizeF(fieldRect.size()));
            return f;
        };
        KSvg::FrameSvg *base = feld(QStringLiteral("base"));
        KSvg::FrameSvg *focus = feld(QStringLiteral("focus"));

        for (const QColor ground : {QColor(0, 0, 0), QColor(255, 255, 255)}) {
            // Ohne die Schicht.
            QImage a(windowSize, QImage::Format_ARGB32);
            a.fill(ground);
            {
                QPainter p(&a);
                p.drawPixmap(0, 0, hull.framePixmap());
                p.drawPixmap(fieldRect.topLeft(), base->framePixmap());
            }
            // Mit der Schicht: base, darueber focus (hint-focus-over-base ist
            // unter allen acht Themes vorhanden, UX9).
            QImage b(windowSize, QImage::Format_ARGB32);
            b.fill(ground);
            {
                QPainter p(&b);
                p.drawPixmap(0, 0, hull.framePixmap());
                p.drawPixmap(fieldRect.topLeft(), base->framePixmap());
                p.drawPixmap(fieldRect.topLeft(), focus->framePixmap());
            }

            const QColor hullColour = a.pixelColor(windowSize.width() / 2, fieldRect.top() - 6);
            const QColor flaeche = a.pixelColor(fieldRect.center());

            // Die staerkste Kante: ueber die ersten acht Spalten am linken Rand
            // suchen, in beiden Bildern an derselben Stelle.
            auto staerkste = [&](const QImage &img) {
                double best = 1.0;
                for (int dx = 0; dx < 8; ++dx) {
                    const QColor c = img.pixelColor(fieldRect.left() + dx, fieldRect.center().y());
                    best = std::max(best, contrast(c, hullColour));
                }
                return best;
            };
            const double ohne = staerkste(a);
            const double mit = staerkste(b);
            // Die Fokuskante allein: die Spalte, in der sich a und b am
            // staerksten unterscheiden.
            double kante = 1.0;
            for (int dx = 0; dx < 8; ++dx) {
                const QColor c = b.pixelColor(fieldRect.left() + dx, fieldRect.center().y());
                const QColor d = a.pixelColor(fieldRect.left() + dx, fieldRect.center().y());
                if (c != d) {
                    kante = std::max(kante, contrast(c, hullColour));
                }
            }

            out << QStringLiteral("%1 %2 %3 %4 %5 %6 %7\n")
                       .arg(theme, -24)
                       .arg(ground == QColor(0, 0, 0) ? QStringLiteral("schwarz") : QStringLiteral("weiss"), -8)
                       .arg(contrast(flaeche, hullColour), 9, 'f', 2)
                       .arg(ohne, 11, 'f', 2)
                       .arg(mit, 11, 'f', 2)
                       .arg(kante, 11, 'f', 2)
                       .arg(mit / ohne, 9, 'f', 2);
        }
        delete base;
        delete focus;
    }
    return 0;
}
