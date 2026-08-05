/**
 * Messsonde 1 zu #83 — die Grafik der Hülle, ohne Fenster.
 *
 * Sie misst, was `KSvg::FrameSvg::framePixmap()` je Desktop-Theme,
 * Bildpunktverhältnis und Auswahlpfad hergibt. Kein Fenster, keine Palette,
 * keine Schrift: Was hier steht, hängt allein an der Theme-Grafik und an den
 * drei Werten, die das Erfassungsfenster ihr übergibt (Farbsatz `Window`,
 * Verhältnis des Fensters, Auswahlpfad).
 *
 * Belegt damit:
 *   AK 4   der Kantenlauf an der Ecke, bei Verhältnis 1 und 1,6
 *   AK 7   was der Auswahlpfad `opaque` an der Deckung ändert
 *   AK 11  ob die Hülle offscreen und unter Wayland byteweise gleich ist —
 *          dafür steht je Zeile eine Prüfsumme des Bildinhalts
 *
 * Aufruf: huellengrafik [Themename …]
 *         (ohne Argumente: alle unter /usr/share/plasma/desktoptheme sowie die
 *          drei mitgelieferten Prüf-Themes, sofern XDG_DATA_DIRS sie zeigt)
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QCryptographicHash>
#include <QDir>
#include <QGuiApplication>
#include <QImage>
#include <QStandardPaths>
#include <QTextStream>

namespace
{
/** Die Maße des ruhenden Erfassungsfensters. */
const QSize WindowSize(600, 174);

/** So viele Zeilen des Bogens sieht der Kantenlauf an (AK 4). */
constexpr int EdgeWalkRows = 10;

QImage hull(const QString &theme,
            const QStringList &selectors,
            qreal ratio,
            KSvg::Svg::ColorSet set = KSvg::Svg::Window)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    imageSet.setSelectors(selectors);

    KSvg::FrameSvg frame;
    // Ohne diese Zeile misst der zweite Durchlauf womöglich das Bild des
    // ersten: Ein FrameSvg behält, was er einmal aufgelöst hat (Messung 3 aus
    // Sprint 6).
    frame.setUsingRenderingCache(false);
    frame.setColorSet(set);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.setDevicePixelRatio(ratio);
    frame.resizeFrame(WindowSize);

    return frame.isValid() ? frame.framePixmap().toImage() : QImage();
}

struct Walk {
    QStringList columns;
    int stairs = 0;
    bool falling = true;
};

Walk edgeWalk(const QImage &image)
{
    Walk walk;
    int previous = -1;
    for (int y = 0; y < EdgeWalkRows && y < image.height(); ++y) {
        int found = image.width();
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alpha() >= 128) {
                found = x;
                break;
            }
        }
        if (previous >= 0) {
            if (found > previous) {
                walk.falling = false;
            }
            if (previous - found >= 2) {
                ++walk.stairs;
            }
        }
        previous = found;
        walk.columns << QString::number(found);
    }
    return walk;
}

/**
 * Prüfsumme über die Bildpunkte, nicht über die Datei.
 *
 * Sie ist der Kern von AK 11: Zwei Läufe derselben Binärdatei auf zwei
 * Plattformen sind dann und nur dann gleich, wenn diese Zeichenfolge gleich
 * ist. Ein Bildvergleich ganzer Fenster taugt dafür nicht — die
 * Schriftrasterung weicht offscreen ab.
 */
QString fingerprint(const QImage &image)
{
    const QImage normalised = image.convertToFormat(QImage::Format_ARGB32);
    QCryptographicHash hash(QCryptographicHash::Sha256);
    for (int y = 0; y < normalised.height(); ++y) {
        hash.addData(QByteArrayView(reinterpret_cast<const char *>(normalised.constScanLine(y)),
                                    normalised.bytesPerLine()));
    }
    return QString::fromLatin1(hash.result().toHex().left(16));
}

QStringList allThemes()
{
    QStringList names;
    const QStringList roots = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("plasma/desktoptheme"),
                                                        QStandardPaths::LocateDirectory);
    for (const QString &root : roots) {
        const QStringList entries = QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &name : entries) {
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
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    QStringList themes = app.arguments().mid(1);
    if (themes.isEmpty()) {
        themes = allThemes();
    }

    out << "=== #83, Sonde 1: die Grafik der Hülle ===\n";
    out << "Plattform     : " << app.platformName() << "\n";
    out << "QT_SCALE_FACTOR: "
        << qEnvironmentVariable("QT_SCALE_FACTOR", QStringLiteral("(nicht gesetzt)")) << "\n";
    out << "Fenstermaß    : " << WindowSize.width() << "x" << WindowSize.height() << " logisch\n";
    out << "Farbsatz      : Window (der des Erfassungsfensters)\n\n";

    out << "Theme                      Ausw.  DPR   Maß        Alpha  Deckung  Stufen  fallend  "
           "Kantenlauf                    Prüfsumme\n";
    out << "-------------------------- ------ ----- ---------- ------ -------- ------- -------- "
           "----------------------------- ----------------\n";

    for (const QString &theme : std::as_const(themes)) {
        for (const auto &selectors : {QStringList{}, QStringList{QStringLiteral("opaque")}}) {
            for (const qreal ratio : {1.0, 1.6}) {
                const QImage image = hull(theme, selectors, ratio);
                if (image.isNull()) {
                    out << QStringLiteral("%1 %2  ungültig\n")
                               .arg(theme, -26)
                               .arg(selectors.isEmpty() ? QStringLiteral("(kein)") : selectors.first(), -6);
                    continue;
                }
                const QColor centre = image.pixelColor(image.width() / 2, image.height() / 2);
                const Walk walk = edgeWalk(image);
                out << QStringLiteral("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10\n")
                           .arg(theme, -26)
                           .arg(selectors.isEmpty() ? QStringLiteral("(kein)") : selectors.first(), -6)
                           .arg(ratio, 5, 'f', 1)
                           .arg(QStringLiteral("%1x%2").arg(image.width()).arg(image.height()), 10)
                           .arg(centre.alpha(), 6)
                           .arg(QStringLiteral("%1 %").arg(100.0 * centre.alpha() / 255.0, 0, 'f', 1), 8)
                           .arg(walk.stairs, 7)
                           .arg(walk.falling ? QStringLiteral("ja") : QStringLiteral("NEIN"), 8)
                           .arg(walk.columns.join(QLatin1Char('.')), -29)
                           .arg(fingerprint(image));
            }
        }
    }

    // Der Anstieg der obersten Zeile des Bogens — die erste Hälfte von AK 4.
    // Streng steigend bis zum **Randwert**, nicht bis zur Flächendeckung: Der
    // Rand des Themes deckt dichter als seine Fläche (235 gegen 216 unter
    // `default`), die Flächendeckung wird auf diesem Weg also nie erreicht.
    out << "\n\n=== Anstieg der obersten Zeile des Bogens (AK 4, erste Hälfte) ===\n";
    for (const QString &theme : std::as_const(themes)) {
        for (const qreal ratio : {1.0, 1.6}) {
            const QImage image = hull(theme, {}, ratio);
            if (image.isNull()) {
                continue;
            }
            // Der Randwert ist der höchste Deckungswert der Zeile; geprüft wird
            // der Lauf vom ersten Bildpunkt mit Alpha > 0 bis zu seinem ersten
            // Auftreten. Die Grenze wird **gesucht**, nicht beim ersten
            // Rückschritt angenommen — sonst wäre „streng steigend" per
            // Konstruktion wahr und die Zeile ein Prüfsatz, der nicht fallen
            // kann.
            int rim = 0;
            for (int x = 0; x < image.width(); ++x) {
                rim = qMax(rim, image.pixelColor(x, 0).alpha());
            }
            int start = 0;
            while (start < image.width() && image.pixelColor(start, 0).alpha() == 0) {
                ++start;
            }
            int end = start;
            while (end < image.width() && image.pixelColor(end, 0).alpha() != rim) {
                ++end;
            }

            QStringList values;
            bool strictlyRising = true;
            int last = -1;
            for (int x = start; x <= end && x < image.width(); ++x) {
                const int alpha = image.pixelColor(x, 0).alpha();
                if (last >= 0 && alpha <= last) {
                    strictlyRising = false;
                }
                values << QString::number(alpha);
                last = alpha;
            }
            out << QStringLiteral("  %1 DPR %2  Randwert %3  streng steigend bis dahin: %4  %5\n")
                       .arg(theme, -26)
                       .arg(ratio, 3, 'f', 1)
                       .arg(rim, 4)
                       .arg(strictlyRising ? QStringLiteral("ja") : QStringLiteral("NEIN"), -4)
                       .arg(values.join(QLatin1Char('.')));
        }
    }

    // Der Farbsatz, den das Erfassungsfenster setzt (`Window` — der eines
    // Dialoggrundes, so wie Plasma diese Grafik zeichnet). Ob die Wahl an
    // diesen Themes überhaupt etwas ändert, ist eine Messfrage und keine
    // Meinung: Die Mutationsprobe zu dieser Zeile bleibt grün, und dieser
    // Abschnitt sagt, warum.
    out << "\n\n=== Ändert der Farbsatz die Zeichnung? ===\n";
    out << "Je Theme die sieben Farbsätze von KSvg, gemessen an der Mitte der Fläche.\n\n";
    const QList<QPair<KSvg::Svg::ColorSet, const char *>> sets{
        {KSvg::Svg::View, "View"},
        {KSvg::Svg::Window, "Window"},
        {KSvg::Svg::Button, "Button"},
        {KSvg::Svg::Selection, "Selection"},
        {KSvg::Svg::Tooltip, "Tooltip"},
        {KSvg::Svg::Complementary, "Complementary"},
        {KSvg::Svg::Header, "Header"},
    };
    for (const QString &theme : std::as_const(themes)) {
        QStringList seen;
        for (const auto &[set, name] : sets) {
            const QImage image = hull(theme, {}, 1.0, set);
            if (image.isNull()) {
                continue;
            }
            const QColor fill = image.pixelColor(image.width() / 2, image.height() / 2);
            const QString value = QStringLiteral("%1,%2,%3/%4")
                                      .arg(fill.red())
                                      .arg(fill.green())
                                      .arg(fill.blue())
                                      .arg(fill.alpha());
            if (!seen.contains(value)) {
                seen << value;
            }
        }
        out << QStringLiteral("  %1 %2 verschiedene Ergebnisse: %3\n")
                   .arg(theme, -26)
                   .arg(seen.size())
                   .arg(seen.join(QStringLiteral("  ")));
    }
    out << "\nLesart: Steht überall eine 1, unterscheidet auf dieser Maschine **kein**\n"
           "Theme die Farbsätze. Die Wahl ist dann richtig und zugleich nicht messbar —\n"
           "eine benannte Grenze, keine ungeprüfte Behauptung.\n";

    return 0;
}
