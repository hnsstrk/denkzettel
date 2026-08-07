// Messung B zur Vorprüfung von Issue #100 — das Eingabefeld aus der Theme-Grafik.
//
// Vier Fragen, unabhängig von Bearbeiter A gestellt:
//
//  1. Löst `widgets/lineedit`, Vorsatz `base`, unter jedem installierten Theme
//     und unter den drei mitgelieferten Prüf-Themes auf?
//  2. Welchen Rand beansprucht der Vorsatz `base` (AK 5 behauptet 6 px ringsum
//     unter allen acht Themes)?
//  3. Wie hebt sich die Feldfläche und die Feldkante gegen die Hülle ab
//     (AK 6 behauptet 1,03–1,10 : 1 unter fünf Themes)?
//  4. Liefert `KSvg::Svg::color(ViewText)` bei `colorSet(View)` überhaupt
//     etwas — und etwas anderes als die Fensterrolle (AK 3)?
//
// Läuft offscreen; die Zahlen betreffen Grafik und Palette, nicht die Sitzung.

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>

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

/** Farbe an einem Punkt eines FrameSvg, auf schwarzem Grund gezeichnet. */
QColor renderedAt(KSvg::FrameSvg *frame, const QSize &size, const QPoint &point, const QColor &ground)
{
    QImage picture(size, QImage::Format_ARGB32);
    picture.fill(ground);
    QPainter painter(&picture);
    painter.drawPixmap(0, 0, frame->framePixmap());
    painter.end();
    return picture.pixelColor(point);
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

    // Die mitgelieferten Prüf-Themes mit auf den Pfad — die Frage, ob sie eine
    // lineedit-Grafik haben, gehört zur Messung.
    if (argc > 1) {
        const QByteArray bundled = QByteArray(argv[1]);
        qputenv("XDG_DATA_DIRS", bundled + ':' + qgetenv("XDG_DATA_DIRS"));
    }

    const QSize size(560, 90);

    out << "== M1/M2/M3: lineedit-base je Theme ==\n";
    out << "Theme | eigene Grafik | base gueltig | Rand L/T/R/B | Flaeche | Kante | Huelle | K:Fl | K:Ka\n";

    for (const QString &theme : allThemes()) {
        KSvg::ImageSet set(theme, QString(ThemePath));

        KSvg::FrameSvg hull;
        hull.setImageSet(&set);
        hull.setImagePath(QStringLiteral("dialogs/background"));
        hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        hull.setColorSet(KSvg::Svg::Window);
        hull.resizeFrame(QSizeF(size));

        KSvg::FrameSvg field;
        field.setImageSet(&set);
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.resizeFrame(QSizeF(size));

        // Bringt das Theme die Grafik selbst mit, oder kommt sie per Rueckfall?
        const QString own = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                   QStringLiteral("%1/%2/widgets/lineedit.svg").arg(QString(ThemePath), theme));
        const QString ownZ = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                    QStringLiteral("%1/%2/widgets/lineedit.svgz").arg(QString(ThemePath), theme));

        qreal l = 0;
        qreal t = 0;
        qreal r = 0;
        qreal b = 0;
        if (field.isValid()) {
            field.getMargins(l, t, r, b);
        }

        const QColor ground(0, 0, 0);
        const QColor hullColour = hull.isValid() ? renderedAt(&hull, size, QPoint(size.width() / 2, size.height() / 2), ground) : QColor();
        const QColor fieldColour = field.isValid() ? renderedAt(&field, size, QPoint(size.width() / 2, size.height() / 2), ground) : QColor();
        // Die Kante: ein Bildpunkt am linken Rand auf halber Hoehe.
        const QColor edgeColour = field.isValid() ? renderedAt(&field, size, QPoint(0, size.height() / 2), ground) : QColor();

        out << theme << " | " << (own.isEmpty() && ownZ.isEmpty() ? "nein" : "ja") << " | "
            << (field.isValid() ? "ja" : "NEIN") << " | ";
        if (field.isValid()) {
            out << l << "/" << t << "/" << r << "/" << b;
        } else {
            out << "—";
        }
        out << " | " << (fieldColour.isValid() ? fieldColour.name() : QStringLiteral("—")) << " | "
            << (edgeColour.isValid() ? edgeColour.name() : QStringLiteral("—")) << " | "
            << (hullColour.isValid() ? hullColour.name() : QStringLiteral("—")) << " | ";
        if (field.isValid() && hull.isValid()) {
            out << QString::number(contrast(fieldColour, hullColour), 'f', 3) << " | "
                << QString::number(contrast(edgeColour, hullColour), 'f', 3);
        } else {
            out << "— | —";
        }
        out << "\n";
    }

    out << "\n== M4: Svg::color() je Farbsatz und Theme ==\n";
    out << "Theme | colors-Datei | Window/Text | View/Text | View/ViewText | Window/ViewText | Window/Frame\n";
    for (const QString &theme : allThemes()) {
        KSvg::ImageSet set(theme, QString(ThemePath));

        const QString colours = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                       QStringLiteral("%1/%2/colors").arg(QString(ThemePath), theme));

        auto ask = [&](KSvg::Svg::ColorSet cs, KSvg::Svg::StyleSheetColor c) {
            KSvg::Svg svg;
            svg.setImageSet(&set);
            svg.setImagePath(QStringLiteral("dialogs/background"));
            svg.setColorSet(cs);
            const QColor value = svg.color(c);
            return value.isValid() ? value.name() : QStringLiteral("ungueltig");
        };

        out << theme << " | " << (colours.isEmpty() ? "nein" : "ja") << " | "
            << ask(KSvg::Svg::Window, KSvg::Svg::Text) << " | "
            << ask(KSvg::Svg::View, KSvg::Svg::Text) << " | "
            << ask(KSvg::Svg::View, KSvg::Svg::ViewText) << " | "
            << ask(KSvg::Svg::Window, KSvg::Svg::ViewText) << " | "
            << ask(KSvg::Svg::Window, KSvg::Svg::Frame) << "\n";
    }

    out << "\n== M5: dritter FrameSvg auf demselben ImageSet, Theme-Wechsel ==\n";
    {
        auto first = std::make_unique<KSvg::ImageSet>(QStringLiteral("default"), QString(ThemePath));
        KSvg::FrameSvg field;
        field.setImageSet(first.get());
        field.setImagePath(QStringLiteral("widgets/lineedit"));
        field.setElementPrefix(QStringLiteral("base"));
        field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        field.resizeFrame(QSizeF(size));
        const QColor before = renderedAt(&field, size, QPoint(size.width() / 2, size.height() / 2), QColor(0, 0, 0));

        // (a) dasselbe Set umbenennen
        first->setImageSetName(QStringLiteral("cachyos-emerald"));
        field.resizeFrame(QSizeF(size));
        const QColor afterRename = renderedAt(&field, size, QPoint(size.width() / 2, size.height() / 2), QColor(0, 0, 0));

        // (b) frisches Set
        auto fresh = std::make_unique<KSvg::ImageSet>(QStringLiteral("cachyos-emerald"), QString(ThemePath));
        field.setImageSet(fresh.get());
        field.resizeFrame(QSizeF(size));
        const QColor afterFresh = renderedAt(&field, size, QPoint(size.width() / 2, size.height() / 2), QColor(0, 0, 0));

        out << "default: " << before.name() << "  nach Umbenennen: " << afterRename.name()
            << "  nach frischem Set: " << afterFresh.name() << "\n";
        out << "Umbenennen wirkt: " << (afterRename != before ? "ja" : "NEIN")
            << "  frisches Set wirkt: " << (afterFresh != before ? "ja" : "NEIN") << "\n";
    }

    return 0;
}
