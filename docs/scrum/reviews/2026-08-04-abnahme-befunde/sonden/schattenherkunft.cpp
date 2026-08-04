/**
 * Messsonde zu Kundenbefund B2 — „Ja, ein Schatten ist da, aber ich habe die
 * Vermutung, dass dieser nicht aus dem KDE-Theme kommt."
 *
 * Belegt war bisher nur, dass der Compositor die Kacheln annimmt und wie groß
 * sie sind. Diese Sonde misst zwei Dinge mehr:
 *
 *   1. **Woher** die Kacheln kommen — Datei, Element, Kachelmaß, Randmaß und
 *      der Alphaverlauf der linken oberen Kachel. Ein Verlauf, der aus dem
 *      Theme stammt, steht Zahl für Zahl in dessen SVG; ein selbstgemachter
 *      wäre linear oder gar konstant.
 *   2. **Wie er sich zu dem verhält, was andere KDE-Fenster tragen.** Ein
 *      dekoriertes Anwendungsfenster bekommt seinen Schatten nicht vom
 *      Desktop-Theme, sondern von der Fensterdekoration (Breeze, `breezerc`).
 *      Die Sonde liest beide Maße und stellt sie nebeneinander.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen schattenherkunft [Themename]
 */

#include <KConfigGroup>
#include <KSharedConfig>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QImage>
#include <QTextStream>

namespace {

const QStringList ShadowElements{
    QStringLiteral("shadow-topleft"),
    QStringLiteral("shadow-top"),
    QStringLiteral("shadow-topright"),
    QStringLiteral("shadow-right"),
    QStringLiteral("shadow-bottomright"),
    QStringLiteral("shadow-bottom"),
    QStringLiteral("shadow-bottomleft"),
    QStringLiteral("shadow-left"),
};

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QString theme = app.arguments().value(1, QStringLiteral("default"));

    out << "=== Schattenherkunft ===\n";
    out << "Desktop-Theme    : " << theme << "\n\n";

    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    KSvg::FrameSvg frame;
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));

    out << "Bildquelle       : " << imageSet.imagePath(QStringLiteral("dialogs/background")) << "\n";
    out << "Prefix „shadow\"  : " << (frame.hasElementPrefix(QStringLiteral("shadow")) ? "vorhanden" : "FEHLT") << "\n";

    frame.setElementPrefix(QStringLiteral("shadow"));
    frame.resizeFrame(QSizeF(600, 150));
    qreal l = 0;
    qreal t = 0;
    qreal r = 0;
    qreal b = 0;
    frame.getMargins(l, t, r, b);
    out << "Schattenausmaß   : links " << l << "  oben " << t << "  rechts " << r << "  unten " << b
        << "  (logische Pixel)\n\n";

    out << "Kacheln\n";
    for (const QString &element : ShadowElements) {
        const QSizeF size = frame.elementSize(element);
        out << QStringLiteral("  %1  %2 x %3\n").arg(element, -20).arg(size.width()).arg(size.height());
    }

    const QString corner = QStringLiteral("shadow-topleft");
    const QImage tile = frame.image(frame.elementSize(corner).toSize(), corner);
    out << "\nAlphaverlauf der Kachel " << corner << " (" << tile.width() << "x" << tile.height() << ")\n";
    out << "Ein Theme-Verlauf ist weich und nicht linear; ein selbstgemalter wäre das eine\n"
           "oder das andere. Die Zahlen sind der Alphakanal, 0 = ganz durchsichtig.\n";
    for (int y = 0; y < tile.height(); ++y) {
        out << QStringLiteral("%1 ").arg(y, 3);
        for (int x = 0; x < tile.width(); ++x) {
            out << QStringLiteral("%1").arg(tile.pixelColor(x, y).alpha(), 4);
        }
        out << "\n";
    }

    out << "\nFarbe der dunkelsten Stelle der Kachel: ";
    QColor darkest;
    int best = 1 << 20;
    for (int y = 0; y < tile.height(); ++y) {
        for (int x = 0; x < tile.width(); ++x) {
            const QColor c = tile.pixelColor(x, y);
            if (c.alpha() > 0 && c.red() + c.green() + c.blue() < best) {
                best = c.red() + c.green() + c.blue();
                darkest = c;
            }
        }
    }
    out << darkest.red() << "," << darkest.green() << "," << darkest.blue() << " bei Alpha " << darkest.alpha() << "\n";

    // --- Gegenprobe: der Schatten eines dekorierten Fensters ---------------
    const KSharedConfig::Ptr breeze = KSharedConfig::openConfig(QStringLiteral("breezerc"));
    const KConfigGroup common = breeze->group(QStringLiteral("Common"));
    const QString sizeName = common.readEntry("ShadowSize", QStringLiteral("ShadowLarge"));
    const int strength = common.readEntry("ShadowStrength", 255);

    out << "\n=== Gegenprobe: Schatten eines dekorierten KDE-Fensters ===\n";
    out << "Quelle           : Fensterdekoration Breeze, nicht das Desktop-Theme\n";
    out << "breezerc         : " << (common.exists() ? "Gruppe [Common] vorhanden"
                                                    : "keine Gruppe [Common] — es gelten die Vorgaben")
        << "\n";
    out << "ShadowSize       : " << sizeName << "\n";
    out << "ShadowStrength   : " << strength << " von 255  =  " << (100 * strength / 255) << " %\n";
    out << "\nWie viele Pixel eine Stufe wie „" << sizeName << "\" ergibt, misst diese Sonde\n"
           "NICHT: Die Dekoration ist ein KWin-Plugin, keine Bibliothek, die man fragen\n"
           "könnte, und eine aus dem Gedächtnis abgeschriebene Konstante wäre kein Messwert.\n"
           "Das Maß des nativen Schattens steht stattdessen in `schattenprofil.py`, gemessen\n"
           "an der Aufnahme des Kunden — an echten Pixeln eines echten Fensters.\n";
    out << "\nWas hier belegt ist: Denkzettel trägt den Schatten eines **Plasma-Dialogs**\n"
           "(dialogs/background, prefix shadow) aus der Datei oben — denselben, den Plasma-\n"
           "Aufklapper tragen. Ein Anwendungsfenster mit Titelleiste trägt den der\n"
           "Dekoration. Beide kommen aus KDE, aus zwei verschiedenen Quellen.\n";

    return 0;
}
