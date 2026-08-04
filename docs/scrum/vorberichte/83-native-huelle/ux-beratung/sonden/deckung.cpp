/**
 * Messsonde zur UX-Beratung #83 — die Deckung der nativen Hülle.
 *
 * Die Vorlage (`native-ak2-kontrast.txt`) hat **20 Farbschemata** unter
 * **einem** Desktop-Theme gefahren (`ImageSet("default", …)`) und Alpha 216
 * gefunden. Die Deckung ist aber eine Eigenschaft des **Desktop-Themes**, nicht
 * des Farbschemas — also hat die Vorlage die Achse festgehalten, auf der sich
 * die Zahl bewegt. Diese Sonde dreht es um: ein Farbschema, alle installierten
 * Desktop-Themes, dazu je Theme der Auswahlpfad `opaque`.
 *
 * `KSvg::ImageSet::setSelectors()` ist der Grund für die zweite Spalte: laut
 * Kopfdatei (`/usr/include/KF6/KSvg/ksvg/imageset.h:98-107`) wählt Plasma
 * darüber `opaque` oder `translucent`, je nachdem, ob Compositing und der
 * KWin-Weichzeichner vorhanden sind.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen deckung
 */

#include <KColorScheme>

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QDir>
#include <QGuiApplication>
#include <QImage>
#include <QTextStream>

namespace {

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

struct Reading {
    int alpha = -1;
    int spread = 0;
    QColor drawn;
    bool valid = false;
};

Reading read(const QString &theme, const QStringList &selectors)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    imageSet.setSelectors(selectors);

    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    frame.setColorSet(KSvg::Svg::Window);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(600, 174));

    Reading r;
    r.valid = frame.isValid();
    if (!r.valid) {
        return r;
    }
    const QImage image = frame.framePixmap().toImage();
    r.drawn = image.pixelColor(image.width() / 2, image.height() / 2);
    r.alpha = r.drawn.alpha();
    // Gegenprobe: fünf Stellen quer durch die Fläche, damit die Mitte nicht
    // als Sonderfall durchgeht.
    for (const QPoint &p : {QPoint(image.width() / 4, image.height() / 4),
                            QPoint(3 * image.width() / 4, image.height() / 4),
                            QPoint(image.width() / 4, 3 * image.height() / 4),
                            QPoint(3 * image.width() / 4, 3 * image.height() / 4)}) {
        r.spread = qMax(r.spread, qAbs(image.pixelColor(p).alpha() - r.alpha));
    }
    return r;
}

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const KColorScheme scheme(QPalette::Active, KColorScheme::Window);
    const QColor text = scheme.foreground(KColorScheme::NormalText).color();
    const QColor window = scheme.background(KColorScheme::NormalBackground).color();

    out << "=== Deckung der nativen Hülle je Desktop-Theme ===\n";
    out << "Plattform       : " << app.platformName() << "\n";
    out << "XDG_CONFIG_HOME : " << qEnvironmentVariable("XDG_CONFIG_HOME", QStringLiteral("(nicht gesetzt)")) << "\n";
    out << "Farbschema      : " << app.arguments().value(1, QStringLiteral("(aus der Umgebung)")) << "\n";
    out << "Window          : " << window.name() << "   WindowText: " << text.name() << "\n\n";

    out << "Desktop-Theme          | Auswahl     | Alpha | Streu | RGB          | = Window?  | Deckung | ungünstigster Kontrast\n";
    out << "-----------------------+-------------+-------+-------+--------------+------------+---------+-----------------------\n";

    QDir dir(QStringLiteral("/usr/share/plasma/desktoptheme"));
    QStringList themes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QString &theme : std::as_const(themes)) {
        for (const auto &sel : {QStringList{}, QStringList{QStringLiteral("opaque")}}) {
            const Reading r = read(theme, sel);
            const QString selName = sel.isEmpty() ? QStringLiteral("(keine)") : sel.first();
            if (!r.valid) {
                out << QStringLiteral("%1| %2| ungültig\n").arg(theme, -23).arg(selName, -12);
                continue;
            }
            const QColor onBlack = over(r.drawn, Qt::black);
            const QColor onWhite = over(r.drawn, Qt::white);
            const qreal worst = qMin(contrast(onBlack, text), contrast(onWhite, text));
            const int dr = qAbs(r.drawn.red() - window.red());
            const int dg = qAbs(r.drawn.green() - window.green());
            const int db = qAbs(r.drawn.blue() - window.blue());
            const int dmax = qMax(dr, qMax(dg, db));
            out << QStringLiteral("%1| %2| %3 | %4 | %5 | %6 | %7 % | %8%9\n")
                       .arg(theme, -23)
                       .arg(selName, -12)
                       .arg(r.alpha, 5)
                       .arg(r.spread, 5)
                       .arg(QStringLiteral("%1,%2,%3").arg(r.drawn.red(), 3).arg(r.drawn.green(), 3).arg(r.drawn.blue(), 3), 12)
                       .arg(dmax == 0 ? QStringLiteral("   ja      ") : QStringLiteral("NEIN (%1)").arg(dmax, 3), -11)
                       .arg(100.0 * r.alpha / 255.0, 6, 'f', 1)
                       .arg(worst, 6, 'f', 2)
                       .arg(worst < 4.5 ? QStringLiteral("  UNTER 4,5:1") : QString());
        }
    }

    return 0;
}
