/*
 * Vierte Messsonde vom 06.08.2026: die Zahlen für das Mockup.
 *
 * Ein Mockup mit geschätzten Farben und geschätzten Schriftgrößen beantwortet
 * die Kontrastfrage nicht — es zeigt dann die Vermutung seines Zeichners. Also
 * werden beide hier abgefragt, an derselben Stelle und auf demselben Weg wie
 * im Bau: die Farben über `KColorScheme::createApplicationPalette` aus der
 * `.colors`-Datei, die Schriften über `QFontDatabase::systemFont` unter
 * `QT_QPA_PLATFORMTHEME=kde`.
 *
 * Dazu die Farbe der Trennlinie im Kirigami-Verfahren: Grund und Textfarbe
 * gemischt im Verhältnis `frameContrast` des Schemas.
 *
 * Aufruf: farbtafel <Pfad zur .colors> …
 */

#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QApplication>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QStyleFactory>
#include <QTextStream>

#include <cmath>

namespace
{
QTextStream out(stdout);

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

QColor mixed(const QColor &ground, const QColor &text, double share)
{
    return QColor::fromRgbF(ground.redF() * (1 - share) + text.redF() * share,
                            ground.greenF() * (1 - share) + text.greenF() * share,
                            ground.blueF() * (1 - share) + text.blueF() * share);
}

void line(const QString &name, const QColor &c)
{
    out << QStringLiteral("  %1 %2  (%3,%4,%5)\n")
               .arg(name, -18)
               .arg(c.name())
               .arg(c.red())
               .arg(c.green())
               .arg(c.blue());
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create(QStringLiteral("breeze")));

    const QFont body = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    const QFont small = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
    out << "Schriften (QT_QPA_PLATFORMTHEME=kde):\n";
    out << QStringLiteral("  Notiztext   %1  %2 pt / %3 px\n")
               .arg(body.family())
               .arg(body.pointSizeF())
               .arg(QFontMetrics(body).height());
    out << QStringLiteral("  Zeitstempel %1  %2 pt / %3 px\n")
               .arg(small.family())
               .arg(small.pointSizeF())
               .arg(QFontMetrics(small).height());
    QFont head = small;
    head.setWeight(QFont::DemiBold);
    out << QStringLiteral("  Gruppenkopf %1  %2 pt / %3 px, DemiBold\n\n")
               .arg(head.family())
               .arg(head.pointSizeF())
               .arg(QFontMetrics(head).height());

    for (int i = 1; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
        const QPalette p = KColorScheme::createApplicationPalette(scheme);
        const double frameContrast =
            KConfigGroup(scheme, QStringLiteral("General")).readEntry("frameContrast", 0.20);

        out << "############ " << QFileInfo(path).baseName() << "\n";
        line(QStringLiteral("Window"), p.color(QPalette::Active, QPalette::Window));
        line(QStringLiteral("WindowText"), p.color(QPalette::Active, QPalette::WindowText));
        line(QStringLiteral("Base"), p.color(QPalette::Active, QPalette::Base));
        line(QStringLiteral("AlternateBase"), p.color(QPalette::Active, QPalette::AlternateBase));
        line(QStringLiteral("Text"), p.color(QPalette::Active, QPalette::Text));
        line(QStringLiteral("PlaceholderText"), p.color(QPalette::Active, QPalette::PlaceholderText));
        line(QStringLiteral("Highlight"), p.color(QPalette::Active, QPalette::Highlight));
        line(QStringLiteral("HighlightedText"), p.color(QPalette::Active, QPalette::HighlightedText));

        const QColor base = p.color(QPalette::Active, QPalette::Base);
        const QColor text = p.color(QPalette::Active, QPalette::Text);
        const QColor separator = mixed(base, text, frameContrast);
        line(QStringLiteral("Trennlinie"), separator);
        out << QStringLiteral("  frameContrast %1 — Trennlinie gegen Base %2:1\n")
                   .arg(frameContrast)
                   .arg(contrast(separator, base), 0, 'f', 2);
        out << QStringLiteral("  AlternateBase gegen Base %1:1\n")
                   .arg(contrast(p.color(QPalette::Active, QPalette::AlternateBase), base), 0, 'f', 2);
        out << QStringLiteral("  Text gegen Base %1:1, PlaceholderText gegen Base %2:1\n")
                   .arg(contrast(text, base), 0, 'f', 2)
                   .arg(contrast(p.color(QPalette::Active, QPalette::PlaceholderText), base), 0, 'f', 2);

        // Der vierte Weg, den niemand vorgeschlagen hat: eine Fläche hinter
        // dem Gruppenkopf aus dem Kopf-Farbsatz des Schemas (KColorScheme
        // kennt ihn seit KF5.19; Kirigami färbt damit Listenüberschriften).
        const KColorScheme header(QPalette::Active, KColorScheme::Header, scheme);
        const QColor headerBackground = header.background().color();
        line(QStringLiteral("Header-Fläche"), headerBackground);
        out << QStringLiteral("  Header-Fläche gegen Base %1:1\n\n")
                   .arg(contrast(headerBackground, base), 0, 'f', 2);
    }

    return 0;
}
