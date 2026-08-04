/**
 * Messsonde zur UX-Beratung #83 — die native Hülle als Bild, über zwei Gründen.
 *
 * Zweck: Die Zahlen aus `deckung` sichtbar machen. Je Desktop-Theme wird die
 * Hülle in einem Stück gezeichnet (`framePixmap()`, der native Weg aus #83) und
 * über einen **hellen** und einen **dunklen** Grund gelegt — das sind die beiden
 * Enden dessen, was hinter einer Überlagerung liegen kann. Dazu der Notiztext in
 * `WindowText` des eingestellten Farbschemas, damit der Kontrast nicht nur als
 * Zahl, sondern als Bild vorliegt.
 *
 * Was diese Bilder **nicht** belegen (B21): Hülle, Rundung, Kontur, Schatten und
 * Dekoration in der angemeldeten Sitzung. Sie belegen Deckung, Flächenfarbe und
 * Textkontrast — das, was rechnet, nicht das, was Theme und Compositor zeichnen.
 *
 * Aufruf: huellenbild <Zielordner> <Theme> [<Theme> …]
 */

#include <KColorScheme>

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QDir>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QTextStream>

namespace {

QImage backdrop(const QSize &size, const QColor &a, const QColor &b)
{
    // Schraffur wie in Zeichnung 4a — ein gleichförmiger Grund verbirgt gerade
    // das, worum es geht.
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(a);
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(b);
    for (int x = -size.height(); x < size.width(); x += 16) {
        painter.drawPolygon(QPolygon({QPoint(x, 0), QPoint(x + 8, 0), QPoint(x + 8 + size.height(), size.height()), QPoint(x + size.height(), size.height())}));
    }
    return image;
}

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    const QStringList args = app.arguments();
    if (args.size() < 3) {
        out << "Aufruf: huellenbild <Zielordner> <Theme> [<Theme> …]\n";
        return 2;
    }
    const QString target = args.at(1);
    QDir().mkpath(target);

    const KColorScheme scheme(QPalette::Active, KColorScheme::Window);
    const QColor text = scheme.foreground(KColorScheme::NormalText).color();
    const QColor placeholder = scheme.foreground(KColorScheme::InactiveText).color();

    out << "Plattform : " << app.platformName() << "\n";
    out << "WindowText: " << text.name() << "\n";

    const QSize hull(600, 174);
    const QSize canvas(660, 234);

    for (int i = 2; i < args.size(); ++i) {
        const QString theme = args.at(i);

        KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg frame;
        frame.setUsingRenderingCache(false);
        frame.setColorSet(KSvg::Svg::Window);
        frame.setImageSet(&imageSet);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.resizeFrame(hull);
        const QImage hullImage = frame.framePixmap().toImage();
        // Die Hülle allein, ohne einen einzigen Buchstaben — damit sich
        // Plattformunterschiede der Grafik von denen des Textsatzes trennen
        // lassen (AK 4 von #83 spricht von „dasselbe Bild").
        hullImage.save(QStringLiteral("%1/%2-nurhuelle.png").arg(target, theme));

        for (const auto &ground : {std::pair{QStringLiteral("hell"), std::pair{QColor(0xf2, 0xf0, 0xeb), QColor(0xe9, 0xe7, 0xe2)}},
                                   std::pair{QStringLiteral("dunkel"), std::pair{QColor(0x22, 0x22, 0x26), QColor(0x2c, 0x2c, 0x32)}}}) {
            QImage sheet = backdrop(canvas, ground.second.first, ground.second.second);
            QPainter painter(&sheet);
            painter.drawImage(30, 30, hullImage);

            qreal left = 0;
            qreal top = 0;
            qreal right = 0;
            qreal bottom = 0;
            frame.getMargins(left, top, right, bottom);

            QFont small = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
            painter.setFont(small);
            painter.setPen(placeholder);
            painter.drawText(QRect(30 + qRound(left) + 12, 30 + qRound(top) + 10, 400, 20), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("Denkzettel"));

            painter.setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
            painter.setPen(text);
            painter.drawText(QRect(30 + qRound(left) + 12, 30 + qRound(top) + 36, 500, 60), Qt::AlignLeft | Qt::AlignTop, QStringLiteral("Notiztext in WindowText — liest sich das?"));

            painter.setFont(small);
            painter.setPen(placeholder);
            painter.drawText(QRect(30, 30 + hull.height() - qRound(bottom) - 26, hull.width(), 20), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral("Esc verwirft · Strg+Enter speichert"));
            painter.end();

            const QString file = QStringLiteral("%1/%2-%3.png").arg(target, theme, ground.first);
            sheet.save(file);
            out << "geschrieben: " << file << "\n";
        }
    }

    return 0;
}
