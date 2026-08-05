/*
 * Sonde der Vorprüfung zu #85, Bearbeiter A (UX).
 *
 * Frage: Woher käme die Schriftfarbe, wenn sie „aus derselben Quelle wie die
 * Fläche" kommen soll — und was ergibt das je Desktop-Theme auf dem Stand
 * **nach** #83?
 *
 * Gemessen wird je Theme:
 *   1. die eigene `colors`-Datei des Themes (KConfig, [Colors:Window]),
 *   2. was `KSvg::Svg::color()` zu demselben Theme sagt — der Weg, der schon
 *      im Projekt verlinkt ist,
 *   3. die Fläche, die der Code von heute zeichnet, auf **beiden**
 *      Auswahlpfaden (durchscheinend und `opaque`),
 *   4. die Kontraste beider Textklassen gegen diese Fläche, deckend gerechnet
 *      und zusätzlich über einem benannten Grund zusammengesetzt.
 *
 * Schreibt nichts und liest keine Einstellung des Kunden ausser dem
 * Farbschema, das die Anwendungspalette ohnehin trägt.
 *
 * Aufruf: themetext <theme> [<theme> …]
 */

#include <KConfigGroup>
#include <KSharedConfig>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QCoreApplication>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QStandardPaths>
#include <QTextStream>

#include <cmath>
#include <memory>

namespace
{
QTextStream out(stdout);

double kanal(int c)
{
    const double v = c / 255.0;
    return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
}

double leuchtdichte(const QColor &c)
{
    return 0.2126 * kanal(c.red()) + 0.7152 * kanal(c.green()) + 0.0722 * kanal(c.blue());
}

double kontrast(const QColor &a, const QColor &b)
{
    const double la = leuchtdichte(a);
    const double lb = leuchtdichte(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

/** Die Fläche über einem Grund zusammengesetzt — das, was das Auge sieht. */
QColor ueber(const QColor &oben, const QColor &grund)
{
    const double a = oben.alpha() / 255.0;
    return QColor(qRound(a * oben.red() + (1 - a) * grund.red()),
                  qRound(a * oben.green() + (1 - a) * grund.green()),
                  qRound(a * oben.blue() + (1 - a) * grund.blue()));
}

QString farbe(const QColor &c)
{
    return QStringLiteral("%1,%2,%3/a%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

/** Die `colors`-Datei des Themes, unmittelbar gelesen. */
struct ThemeFarben {
    bool vorhanden = false;
    QColor vordergrund;
    QColor gedaempft;
    QColor grund;
};

ThemeFarben themeFarben(const QString &theme)
{
    ThemeFarben f;
    const QString datei = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                 QStringLiteral("plasma/desktoptheme/%1/colors").arg(theme));
    if (datei.isEmpty()) {
        return f;
    }

    const KConfigGroup gruppe(KSharedConfig::openConfig(datei, KConfig::SimpleConfig),
                              QStringLiteral("Colors:Window"));
    if (!gruppe.exists()) {
        return f;
    }

    f.vorhanden = true;
    f.vordergrund = gruppe.readEntry("ForegroundNormal", QColor());
    f.gedaempft = gruppe.readEntry("ForegroundInactive", QColor());
    f.grund = gruppe.readEntry("BackgroundNormal", QColor());
    return f;
}

/** Die Fläche, die der Zeichenweg von heute in der Fenstermitte hinterlässt. */
QColor gezeichnet(const QString &theme, const QStringList &auswahl)
{
    auto satz = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));
    if (!auswahl.isEmpty()) {
        satz->setSelectors(auswahl);
    }

    KSvg::FrameSvg rahmen;
    rahmen.setImageSet(satz.get());
    rahmen.setImagePath(QStringLiteral("dialogs/background"));
    rahmen.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    rahmen.setColorSet(KSvg::Svg::Window);
    rahmen.resizeFrame(QSizeF(600, 174));
    if (!rahmen.isValid()) {
        return {};
    }

    QImage bild(600, 174, QImage::Format_ARGB32);
    bild.fill(Qt::transparent);
    {
        QPainter maler(&bild);
        maler.drawPixmap(0, 0, rahmen.framePixmap());
    }
    return bild.pixelColor(300, 87);
}

/** Was KSvg selbst zu den Farben dieses Themes sagt. */
QColor ksvgTextfarbe(const QString &theme, KSvg::Svg::StyleSheetColor rolle)
{
    auto satz = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::Svg svg;
    svg.setImageSet(satz.get());
    svg.setImagePath(QStringLiteral("dialogs/background"));
    svg.setColorSet(KSvg::Svg::Window);
    return svg.color(rolle);
}
}

int main(int argc, char **argv)
{
    // Der Testmodus ist hier zuschaltbar, weil er die Messung verschiebt und
    // nicht nur den Ablageort: `QStandardPaths::setTestModeEnabled(true)` biegt
    // GenericConfigLocation um, damit findet Qt die `kdeglobals` des Kunden
    // nicht mehr, und die Anwendung läuft auf der Ersatzpalette. Ein Theme, das
    // dem Farbschema folgt, folgt dann einem anderen Schema als dem, über das
    // die Story spricht. Genau in diesem Modus laufen die Tests des Projekts.
    const bool testmodus = qEnvironmentVariableIntValue("DENKZETTEL_SONDE_TESTMODUS") == 1;
    if (testmodus) {
        QStandardPaths::setTestModeEnabled(true);
    }
    QGuiApplication app(argc, argv);
    const QStringList themes = app.arguments().mid(1);

    const QPalette schema = QGuiApplication::palette();
    const QColor schemaText = schema.color(QPalette::WindowText);
    const QColor schemaGedaempft = schema.color(QPalette::PlaceholderText);
    const QColor schemaGrund = schema.color(QPalette::Window);

    out << "Farbschema der Anwendung: Window " << farbe(schemaGrund) << "  WindowText "
        << farbe(schemaText) << "  PlaceholderText " << farbe(schemaGedaempft) << "\n";
    out << "Gruende fuer die Zusammensetzung: hell 255,255,255 und dunkel 0,0,0\n\n";

    for (const QString &theme : themes) {
        const ThemeFarben tf = themeFarben(theme);
        const QColor lose = gezeichnet(theme, {});
        const QColor fest = gezeichnet(theme, {QStringLiteral("opaque")});
        const QColor ksvgText = ksvgTextfarbe(theme, KSvg::Svg::Text);
        const QColor ksvgGrund = ksvgTextfarbe(theme, KSvg::Svg::Background);

        out << "==== " << theme << "\n";
        out << "  colors-Datei          : " << (tf.vorhanden ? QStringLiteral("ja") : QStringLiteral("nein"))
            << "\n";
        if (tf.vorhanden) {
            out << "    ForegroundNormal    : " << farbe(tf.vordergrund) << "\n";
            out << "    ForegroundInactive  : " << farbe(tf.gedaempft) << "\n";
            out << "    BackgroundNormal    : " << farbe(tf.grund) << "\n";
        }
        out << "  KSvg::Svg::color(Text): " << farbe(ksvgText)
            << (ksvgText == schemaText ? QStringLiteral("   == Schemaschrift") : QString())
            << (tf.vorhanden && ksvgText == tf.vordergrund ? QStringLiteral("   == Themeschrift") : QString())
            << "\n";
        out << "  KSvg::Svg::color(Bkgd): " << farbe(ksvgGrund) << "\n";
        out << "  gezeichnet lose       : " << farbe(lose) << "\n";
        out << "  gezeichnet opaque     : " << farbe(fest) << "\n";

        struct Fall {
            QString name;
            QColor flaeche;
        };
        const QList<Fall> faelle{{QStringLiteral("lose"), lose}, {QStringLiteral("opaque"), fest}};

        for (const Fall &fall : faelle) {
            if (!fall.flaeche.isValid()) {
                continue;
            }
            const QColor deckend(fall.flaeche.red(), fall.flaeche.green(), fall.flaeche.blue());
            const QColor ueberHell = ueber(fall.flaeche, QColor(255, 255, 255));
            const QColor ueberDunkel = ueber(fall.flaeche, QColor(0, 0, 0));

            out << "  --- Fläche " << fall.name << ", Deckung "
                << QString::number(100.0 * fall.flaeche.alpha() / 255.0, 'f', 1) << " %\n";
            out << QStringLiteral("      Schemaschrift  deckend %1  ueber hell %2  ueber dunkel %3\n")
                       .arg(kontrast(schemaText, deckend), 6, 'f', 2)
                       .arg(kontrast(schemaText, ueberHell), 6, 'f', 2)
                       .arg(kontrast(schemaText, ueberDunkel), 6, 'f', 2);
            out << QStringLiteral("      Schema gedämpft deckend %1  ueber hell %2  ueber dunkel %3\n")
                       .arg(kontrast(schemaGedaempft, deckend), 6, 'f', 2)
                       .arg(kontrast(schemaGedaempft, ueberHell), 6, 'f', 2)
                       .arg(kontrast(schemaGedaempft, ueberDunkel), 6, 'f', 2);
            if (tf.vorhanden) {
                out << QStringLiteral("      Themeschrift   deckend %1  ueber hell %2  ueber dunkel %3\n")
                           .arg(kontrast(tf.vordergrund, deckend), 6, 'f', 2)
                           .arg(kontrast(tf.vordergrund, ueberHell), 6, 'f', 2)
                           .arg(kontrast(tf.vordergrund, ueberDunkel), 6, 'f', 2);
                out << QStringLiteral("      Theme gedämpft deckend %1  ueber hell %2  ueber dunkel %3\n")
                           .arg(kontrast(tf.gedaempft, deckend), 6, 'f', 2)
                           .arg(kontrast(tf.gedaempft, ueberHell), 6, 'f', 2)
                           .arg(kontrast(tf.gedaempft, ueberDunkel), 6, 'f', 2);
            }
        }
        out << "\n";
    }

    out.flush();
    return 0;
}
