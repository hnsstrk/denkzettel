// Nachprüfung dreier Messungen aus Bericht A, die in Messung B nicht vorkamen
// oder ihr widersprechen. Nicht übernommen — nachgemessen.
//
//  V1  Deckung der Hülle **mit** dem Auswahlpfad `opaque` gegen die ohne ihn.
//      Messung B hat ohne ihn gemessen und 216 unter `default` gefunden;
//      Bericht A nennt 255. Der Bau setzt den Pfad, sobald nichts weichzeichnet
//      (`capturewindow.cpp:315`), und `themeHull()` im Test tut dasselbe — die
//      Zahl ohne den Pfad ist damit die falsche für beide Prüfsätze.
//      Daran hängt, ob sie rot werden oder grün bleiben und das Falsche messen.
//
//  V2  Löst `widgets/lineedit` auch unter einem Namen auf, auf den nichts hört?
//      Bericht A sagt ja, weil KSvg je Bild auf `default` zurückfällt. Davon
//      hängt ab, wo AK 8 überhaupt auslösen kann.
//
//  V3  Fläche und Kante gegen die Hülle über **mittlerem Grau** — der Grund,
//      den Bericht A für die Zahlen 1,39 : 1 und 1,33 : 1 nennt.

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QTextStream>

namespace
{
constexpr QLatin1StringView ThemePath("plasma/desktoptheme");

double luminance(const QColor &c)
{
    auto channel = [](double v) {
        v /= 255.0;
        return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(c.red()) + 0.7152 * channel(c.green()) + 0.0722 * channel(c.blue());
}

double contrast(const QColor &a, const QColor &b)
{
    const double la = luminance(a);
    const double lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

QStringList allThemes()
{
    QStringList names;
    const QStringList roots = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QString(ThemePath),
                                                        QStandardPaths::LocateDirectory);
    for (const QString &root : roots) {
        for (const QString &name : QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
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
    QApplication app(argc, argv);
    QTextStream out(stdout);

    if (argc > 1) {
        qputenv("XDG_DATA_DIRS", QByteArray(argv[1]) + ':' + qgetenv("XDG_DATA_DIRS"));
    }

    const QSize windowSize(600, 174);
    const QRect fieldRect(28, 40, 544, 90);

    out << "== V1: Deckung in der Fenstermitte, mit und ohne Auswahlpfad `opaque` ==\n";
    out << "Theme | Huelle ohne | Huelle opaque | Feld opaque | Prüfsatz nach dem Feld\n";
    for (const QString &theme : allThemes()) {
        auto alphaOf = [&](const QString &image, const QString &prefix, const QStringList &selectors,
                           const QSize &size, const QPoint &at) {
            KSvg::ImageSet set(theme, QString(ThemePath));
            set.setSelectors(selectors);
            KSvg::FrameSvg frame;
            frame.setUsingRenderingCache(false);
            frame.setImageSet(&set);
            frame.setImagePath(image);
            if (!prefix.isEmpty()) {
                frame.setElementPrefix(prefix);
            } else {
                frame.setColorSet(KSvg::Svg::Window);
            }
            frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
            frame.resizeFrame(QSizeF(size));
            if (!frame.isValid()) {
                return -1;
            }
            return qAlpha(frame.framePixmap().toImage().pixel(at));
        };

        const QPoint centre(windowSize.width() / 2, windowSize.height() / 2);
        const int loose = alphaOf(QStringLiteral("dialogs/background"), {}, {}, windowSize, centre);
        const int tight = alphaOf(QStringLiteral("dialogs/background"), {},
                                  {QStringLiteral("opaque")}, windowSize, centre);
        const int field = alphaOf(QStringLiteral("widgets/lineedit"), QStringLiteral("base"),
                                  {QStringLiteral("opaque")}, fieldRect.size(),
                                  QPoint(fieldRect.width() / 2, fieldRect.height() / 2));

        // Der Prüfsatz vergleicht die Deckung des gezeichneten Fensters mit der
        // der Hülle allein. Über der Hülle liegt jetzt das Feld: die Deckung
        // danach ist die Alphakomposition der beiden.
        const int composed = field < 0 || tight < 0
            ? -1
            : qRound(field + tight * (1.0 - field / 255.0));
        out << theme << " | " << loose << " | " << tight << " | " << field << " | "
            << (composed < 0 ? QStringLiteral("—")
                             : QStringLiteral("%1 gegen %2 → %3")
                                   .arg(composed)
                                   .arg(tight)
                                   .arg(composed == tight ? QStringLiteral("GRUEN") : QStringLiteral("rot")))
            << "\n";
    }

    out << "\n== V2: Auflösung unter einem Namen, auf den nichts hört ==\n";
    for (const QString &theme : {QStringLiteral("kein-solches-theme"), QStringLiteral("default")}) {
        KSvg::ImageSet set(theme, QString(ThemePath));
        KSvg::FrameSvg field;
        field.setImageSet(&set);
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.resizeFrame(QSizeF(fieldRect.size()));
        qreal l = 0;
        qreal t = 0;
        qreal r = 0;
        qreal b = 0;
        if (field.isValid()) {
            field.getMargins(l, t, r, b);
        }
        out << theme << ": isValid=" << (field.isValid() ? "ja" : "nein")
            << "  hasElementPrefix(base)=" << (field.hasElementPrefix(QStringLiteral("base")) ? "ja" : "nein")
            << "  Rand " << l << "/" << t << "/" << r << "/" << b << "\n";
    }

    out << "\n== V3: Fläche und Kante gegen die Hülle über mittlerem Grau (128,128,128) ==\n";
    out << "Theme | Huelle | Flaeche | Kante | K(Fl:Hu) | K(Ka:Hu)\n";
    for (const QString &theme : allThemes()) {
        KSvg::ImageSet set(theme, QString(ThemePath));
        KSvg::FrameSvg hull;
        hull.setImageSet(&set);
        hull.setImagePath(QStringLiteral("dialogs/background"));
        hull.setColorSet(KSvg::Svg::Window);
        hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        hull.resizeFrame(QSizeF(windowSize));
        KSvg::FrameSvg field;
        field.setImageSet(&set);
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.resizeFrame(QSizeF(fieldRect.size()));
        if (!hull.isValid() || !field.isValid()) {
            continue;
        }

        QImage picture(windowSize, QImage::Format_ARGB32);
        picture.fill(QColor(128, 128, 128));
        QPainter painter(&picture);
        painter.drawPixmap(0, 0, hull.framePixmap());
        const QColor hullColour = picture.pixelColor(windowSize.width() / 2, fieldRect.top() - 6);
        painter.drawPixmap(fieldRect.topLeft(), field.framePixmap());
        painter.end();

        const QColor fieldColour = picture.pixelColor(fieldRect.center());
        const QColor edgeColour = picture.pixelColor(fieldRect.left(), fieldRect.center().y());
        out << theme << " | " << hullColour.name() << " | " << fieldColour.name() << " | "
            << edgeColour.name() << " | " << QString::number(contrast(fieldColour, hullColour), 'f', 3)
            << " | " << QString::number(contrast(edgeColour, hullColour), 'f', 3) << "\n";
    }

    return 0;
}
