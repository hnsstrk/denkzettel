/**
 * Messsonde zu AK 2 unter dem nativen Weg.
 *
 * AK 2 verlangt die Fläche in `Window` und den Notiztext in `WindowText` und
 * beruft sich auf eine Messung über alle installierten Farbschemata: 4,74:1 im
 * schlechtesten Fall, deckend gerechnet. Unter dem nativen Weg kommt die Fläche
 * aus der eingefärbten Theme-Grafik — und die ist **durchscheinend**. Damit
 * hängt der tatsächliche Kontrast davon ab, was hinter dem Fenster liegt.
 *
 * Diese Sonde fährt jedes installierte Farbschema durch und misst drei Zahlen
 * je Schema:
 *
 *   1. die Farbe, die die Grafik annimmt, gegen `Window` des Schemas
 *      (stimmen sie überein, hält AK 2 der Sache nach),
 *   2. den Kontrast deckend gerechnet — die Zahl, die AK 2 heute nennt,
 *   3. den Kontrast auf dem **ungünstigsten** Grund, den der Schreibtisch
 *      bieten kann (schwarz oder weiß, je nachdem, welcher näher an die
 *      Textfarbe heranrückt).
 *
 * Jedes Schema läuft in einem **eigenen** XDG_CONFIG_HOME unter /tmp; das
 * Schema des Kunden wird gelesen, nie geschrieben.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen nativekontrast <Schemadatei ...>
 */

#include <KColorScheme>

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QTextStream>

namespace {

QString rgb(const QColor &c)
{
    return QStringLiteral("%1,%2,%3").arg(c.red(), 3).arg(c.green(), 3).arg(c.blue(), 3);
}

qreal luminance(const QColor &c)
{
    const auto channel = [](qreal v) {
        return v <= 0.03928 ? v / 12.92 : qPow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) + 0.0722 * channel(c.blueF());
}

qreal contrast(const QColor &a, const QColor &b)
{
    const qreal la = luminance(a);
    const qreal lb = luminance(b);
    return (qMax(la, lb) + 0.05) / (qMin(la, lb) + 0.05);
}

QColor over(const QColor &front, const QColor &behind)
{
    const qreal a = front.alphaF();
    return QColor::fromRgbF(front.redF() * a + behind.redF() * (1 - a),
                            front.greenF() * a + behind.greenF() * (1 - a),
                            front.blueF() * a + behind.blueF() * (1 - a));
}

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    out << "=== AK 2 unter dem nativen Weg ===\n";
    out << "Plattform        : " << app.platformName() << "\n";
    out << "XDG_CONFIG_HOME  : " << qEnvironmentVariable("XDG_CONFIG_HOME", QStringLiteral("(nicht gesetzt)")) << "\n";
    out << "Schema           : " << app.arguments().value(1, QStringLiteral("(aus der Umgebung)")) << "\n\n";

    KSvg::ImageSet imageSet(QStringLiteral("default"), QStringLiteral("plasma/desktoptheme"));
    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    frame.setColorSet(KSvg::Svg::Window);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(600, 174));

    const QImage image = frame.framePixmap().toImage();
    const QColor drawn = image.pixelColor(image.width() / 2, image.height() / 2);

    const KColorScheme scheme(QPalette::Active, KColorScheme::Window);
    const QColor window = scheme.background(KColorScheme::NormalBackground).color();
    const QColor text = scheme.foreground(KColorScheme::NormalText).color();

    const QColor opaque(drawn.red(), drawn.green(), drawn.blue());
    const QColor onBlack = over(drawn, Qt::black);
    const QColor onWhite = over(drawn, Qt::white);
    const qreal worst = qMin(contrast(onBlack, text), contrast(onWhite, text));

    // Eine Zeile je Schema, damit sich die Läufe zu einer Tabelle stapeln lassen.
    out << QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8\n")
               .arg(app.arguments().value(1), -26)
               .arg(rgb(window))
               .arg(rgb(opaque))
               .arg(window.rgb() == opaque.rgb() ? QStringLiteral("gleich   ") : QStringLiteral("ABWEICHUNG"))
               .arg(drawn.alpha(), 4)
               .arg(contrast(opaque, text), 6, 'f', 2)
               .arg(worst, 6, 'f', 2)
               .arg(worst < 4.5 ? QStringLiteral(" UNTER 4,5:1") : QString());

    return 0;
}
