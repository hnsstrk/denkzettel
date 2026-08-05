/**
 * Messsonde 5 zu #83 — teilen sich zwei `KSvg::ImageSet` desselben Themes ihre
 * Auswahlpfade?
 *
 * Gefunden beim Bau des Prüfsatzes zu AK 7, und der Prüfsatz wäre daran
 * gescheitert, ohne rot zu werden: Neben dem Erfassungsfenster, das seine
 * `ImageSet` am Leben hält, sollte eine zweite die **durchscheinende** Fassung
 * derselben Grafik zeichnen — zum Vergleich. Sie zeichnete die deckende, und
 * sie meldete auf Nachfrage `opaque` als ihren Auswahlpfad, obwohl ihr niemand
 * einen gegeben hatte.
 *
 * Diese Sonde zeigt die Eigenschaft in drei Schritten: allein, neben einer
 * lebenden mit `opaque`, und nachdem diese gestorben ist. Sie ist der Beleg für
 * den Satz „zwei lebende ImageSets desselben Themenamens teilen ihre
 * Auswahlpfade" — eine Falle für jeden Vergleich, der zwei Fassungen derselben
 * Grafik nebeneinanderstellt.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen ksvgselektoren [Themename]
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QImage>
#include <QTextStream>

#include <memory>

namespace
{
/** Die Deckung in der Mitte der Fläche — die Zahl, an der sich die Fassung zeigt. */
int coverage(KSvg::ImageSet *imageSet)
{
    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    frame.setColorSet(KSvg::Svg::Window);
    frame.setImageSet(imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(600, 174));

    const QImage image = frame.framePixmap().toImage();
    return qAlpha(image.pixel(image.width() / 2, image.height() / 2));
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QString theme = app.arguments().value(1, QStringLiteral("breeze-dark"));
    const QString path = QStringLiteral("plasma/desktoptheme");

    out << "=== #83, Sonde 5: teilen sich zwei ImageSets ihre Auswahlpfade? ===\n";
    out << "Plattform : " << app.platformName() << "\n";
    out << "Theme     : " << theme << " (es muss beide Fassungen mitbringen)\n\n";

    {
        KSvg::ImageSet alone(theme, path);
        out << "1. eine allein, ohne Auswahlpfad          Deckung " << coverage(&alone)
            << "   meldet [" << alone.selectors().join(QLatin1Char(',')) << "]\n";
    }

    auto living = std::make_unique<KSvg::ImageSet>(theme, path);
    living->setSelectors({QStringLiteral("opaque")});
    out << "2. eine lebende mit `opaque`              Deckung " << coverage(living.get())
        << "   meldet [" << living->selectors().join(QLatin1Char(',')) << "]\n";

    {
        KSvg::ImageSet second(theme, path);
        out << "3. eine zweite daneben, ohne Auswahlpfad  Deckung " << coverage(&second)
            << "   meldet [" << second.selectors().join(QLatin1Char(',')) << "]\n";
    }

    living.reset();
    {
        KSvg::ImageSet after(theme, path);
        out << "4. nach dem Ende der lebenden, ohne       Deckung " << coverage(&after)
            << "   meldet [" << after.selectors().join(QLatin1Char(',')) << "]\n";
    }

    out << "\nLesart: Stimmt Zeile 3 mit Zeile 2 überein statt mit Zeile 1 und 4, dann hat\n"
           "die zweite Instanz die Auswahlpfade der ersten übernommen — ohne dass jemand\n"
           "sie ihr gegeben hätte. Ein Vergleich zweier Fassungen derselben Grafik ist\n"
           "dann nur zu haben, solange keine dritte desselben Themenamens lebt.\n";

    return 0;
}
