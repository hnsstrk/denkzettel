/**
 * Was `widgets/lineedit` je Desktop-Theme zeichnet — der Beleg zu AK 6b und die
 * Zahlen hinter AK 1.
 *
 * Gemessen wird die **Deckung** und keine Kontrastzahl, und das ist der Kern
 * des Kriteriums: Eine Kontrastzahl gilt für ein Farbschema, einen Auswahlpfad
 * und einen benannten Grund. Der Grund ist hier der Bildschirmhintergrund des
 * Kunden, denn die Hülle über dem Feld scheint durch — dieselbe Grafik misst
 * 1,08 : 1 über schwarzem und 1,88 : 1 über weißem Grund (#100, F11). Die
 * Deckung dagegen ist eine Eigenschaft der Grafik.
 *
 * Der Auswahlpfad `opaque` wird gesetzt, weil der Bau ihn setzt, sobald nichts
 * weichzeichnet — offscreen ist das immer der Fall.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen feldgrafik [Theme …]
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QDir>
#include <QGuiApplication>
#include <QImage>
#include <QStandardPaths>
#include <QTextStream>

namespace {

QString show(const QColor &colour)
{
    return QStringLiteral("%1,%2,%3/a%4")
        .arg(colour.red(), 3)
        .arg(colour.green(), 3)
        .arg(colour.blue(), 3)
        .arg(colour.alpha(), 3);
}

QStringList installedThemes()
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

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    QStringList themes = app.arguments().mid(1);
    if (themes.isEmpty()) {
        themes = installedThemes();
        // Ein Name, auf den nichts hört. Er steht hier, weil er die Falle F1
        // zeigt: KSvg fällt je Bild auf `default` zurück, also löst **jeder**
        // Name auf. „Bringt dieses Theme eine eigene Grafik mit?" ist durch
        // KSvg nicht zu beobachten — die Deckung ist es.
        themes << QStringLiteral("kein-solches-theme");
    }

    out << "=== #100: Deckung, Rand und Farben von widgets/lineedit je Theme (AK 6b) ===\n";
    out << "Auswahlpfad `opaque`, Rahmen 560x90, Verhältnis 1.\n";
    out << "Fläche = Mitte der Grafik, Kante = äußerster Bildpunkt der linken Seite.\n\n";
    out << QStringLiteral("%1  %2  %3  %4  %5\n")
               .arg(QStringLiteral("Theme"), -26)
               .arg(QStringLiteral("gültig"), -7)
               .arg(QStringLiteral("Rand links"), -11)
               .arg(QStringLiteral("Fläche"), -16)
               .arg(QStringLiteral("Kante"));

    for (const QString &theme : std::as_const(themes)) {
        KSvg::ImageSet set(theme, QStringLiteral("plasma/desktoptheme"));
        set.setSelectors({QStringLiteral("opaque")});

        KSvg::FrameSvg frame;
        frame.setUsingRenderingCache(false);
        frame.setImageSet(&set);
        frame.setImagePath(QStringLiteral("widgets/lineedit"));
        frame.setElementPrefix(QStringLiteral("base"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.resizeFrame(QSizeF(560, 90));

        const QImage picture = frame.framePixmap().toImage();
        out << QStringLiteral("%1  %2  %3  %4  %5\n")
                   .arg(theme, -26)
                   .arg(frame.isValid() ? QStringLiteral("ja") : QStringLiteral("nein"), -7)
                   .arg(frame.marginSize(KSvg::FrameSvg::LeftMargin), -11)
                   .arg(show(picture.pixelColor(280, 45)), -16)
                   .arg(show(picture.pixelColor(0, 45)));
    }

    return 0;
}
