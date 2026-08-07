/*
 * Messsonde der Vorprüfung zu Issue #100 (Bearbeiter A, 07.08.2026).
 *
 * Vier Fragen, die vor dem Ziehen der Story am Code zu beantworten sind:
 *
 *   A) Löst `widgets/lineedit` unter den mitgelieferten Prüf-Themes der
 *      Testsuite überhaupt auf? Sie bringen nur `dialogs/background` mit.
 *      Wenn KSvg auf `default` zurückfällt, prüft eine Zusicherung an ihnen
 *      etwas anderes als sie behauptet; wenn nicht, hat die Story kein
 *      Prüfmittel ohne Plasma-Installation.
 *   B) Was gibt `KSvg::Svg::color(ViewText)` unter den Prüf-Themes zurück —
 *      die Ansichtsgruppe der `colors`-Datei (AK 3) oder etwas anderes?
 *   C) Welches Theme-Paar wählt `themes::installedThemePair()`, dem
 *      `captureshots` folgt, und bringt davon eines eine eigene
 *      `lineedit`-Grafik mit und das andere keine (Belegform der Zeichnung)?
 *   D) Am gebauten Fenster: Wo sitzt der Textbereich, welchen Rand nähme die
 *      Feldgrafik, und welche der offenen Wege rückt den Text nach innen (F5)?
 *
 * Schreibt nichts, installiert nichts, ändert keinen Produktivcode.
 *
 * Aufruf: feldprobe <Prüf-Theme-Verzeichnis> [<theme> …]
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLayout>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>

namespace
{
QTextStream out(stdout);

QString show(const QColor &c)
{
    if (!c.isValid()) {
        return QStringLiteral("ungültig");
    }
    return QStringLiteral("%1,%2,%3/a%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

bool hasOwnLineEdit(const QString &theme)
{
    const QStringList roots = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("plasma/desktoptheme"),
                                                        QStandardPaths::LocateDirectory);
    for (const QString &root : roots) {
        for (const QString &suffix : {QStringLiteral("svgz"), QStringLiteral("svg")}) {
            if (QFile::exists(QStringLiteral("%1/%2/widgets/lineedit.%3").arg(root, theme, suffix))) {
                return true;
            }
        }
    }
    return false;
}

QStringList installedThemes(const QStringList &bundled)
{
    QStringList names;
    const QStringList roots = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("plasma/desktoptheme"),
                                                        QStandardPaths::LocateDirectory);
    for (const QString &root : roots) {
        const QStringList entries = QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &name : entries) {
            if (!names.contains(name) && !bundled.contains(name)) {
                names << name;
            }
        }
    }
    return names;
}

qreal borderOf(const QString &theme)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    KSvg::FrameSvg frame;
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(600, 200));
    return frame.isValid() ? frame.marginSize(KSvg::FrameSvg::LeftMargin) : 0;
}

/** A) bis C) je Theme. */
void probeTheme(const QString &theme)
{
    auto set = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::FrameSvg hull;
    hull.setUsingRenderingCache(false);
    hull.setImageSet(set.get());
    hull.setImagePath(QStringLiteral("dialogs/background"));
    hull.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    hull.setColorSet(KSvg::Svg::Window);

    KSvg::FrameSvg field;
    field.setUsingRenderingCache(false);
    field.setImageSet(set.get());
    field.setImagePath(QStringLiteral("widgets/lineedit"));
    field.setElementPrefix(QStringLiteral("base"));
    field.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    field.setColorSet(KSvg::Svg::View);
    field.resizeFrame(QSizeF(560, 90));

    out << "############ " << theme << "\n";
    out << "  dialogs/background gültig : " << (hull.isValid() ? "ja" : "NEIN") << "\n";
    out << "  widgets/lineedit  gültig : " << (field.isValid() ? "ja" : "NEIN") << "\n";
    out << "  eigene lineedit-Grafik    : " << (hasOwnLineEdit(theme) ? "ja" : "nein") << "\n";
    if (field.isValid()) {
        qreal l = 0;
        qreal t = 0;
        qreal r = 0;
        qreal b = 0;
        field.getMargins(l, t, r, b);
        out << QStringLiteral("  Ränder der Feldgrafik     : links %1 oben %2 rechts %3 unten %4\n")
                   .arg(l)
                   .arg(t)
                   .arg(r)
                   .arg(b);
        out << QStringLiteral("  hasElementPrefix(base)    : %1\n")
                   .arg(field.hasElementPrefix(QStringLiteral("base")) ? "ja" : "NEIN");
    }

    // B) — die beiden Textfarben derselben Quelle. `color()` ist von der
    // Farbgruppe unabhängig; die Enumeration nennt Window- und View-Rolle
    // getrennt.
    KSvg::Svg colours;
    colours.setImageSet(set.get());
    colours.setImagePath(QStringLiteral("dialogs/background"));
    out << "  KSvg color(Text)          : " << show(colours.color(KSvg::Svg::Text)) << "\n";
    out << "  KSvg color(ViewText)      : " << show(colours.color(KSvg::Svg::ViewText)) << "\n";
    out << "  KSvg color(ViewBackground): " << show(colours.color(KSvg::Svg::ViewBackground)) << "\n";
    out << "  eigene colors-Datei (KConfig-Weg, [Colors:Window]) : "
        << show(capture::themeTextColoursOf(theme).normal) << "\n";
    out << "\n";
}

/** D) — am gebauten Fenster. */
void probeWindow(Store &store, const QString &theme)
{
    CaptureWindow window(&store);
    window.reloadDesktopTheme(theme);
    window.show();
    QCoreApplication::processEvents();

    auto *text = window.findChild<QPlainTextEdit *>();
    out << "############ gebautes Fenster unter " << theme << "\n";
    out << QStringLiteral("  Fenster            : %1x%2\n").arg(window.width()).arg(window.height());
    out << QStringLiteral("  Textbereich (Rect) : x=%1 y=%2 %3x%4\n")
               .arg(text->x())
               .arg(text->y())
               .arg(text->width())
               .arg(text->height());
    out << QStringLiteral("  Innenränder Layout : %1\n")
               .arg(QStringLiteral("%1/%2/%3/%4")
                        .arg(window.layout()->contentsMargins().left())
                        .arg(window.layout()->contentsMargins().top())
                        .arg(window.layout()->contentsMargins().right())
                        .arg(window.layout()->contentsMargins().bottom()));
    out << QStringLiteral("  documentMargin     : %1\n").arg(text->document()->documentMargin());
    out << QStringLiteral("  frameWidth         : %1\n").arg(text->frameWidth());
    out << QStringLiteral("  viewport (Rect)    : x=%1 y=%2 %3x%4\n")
               .arg(text->viewport()->x())
               .arg(text->viewport()->y())
               .arg(text->viewport()->width())
               .arg(text->viewport()->height());
    out << QStringLiteral("  Palette Base       : %1\n")
               .arg(show(text->palette().color(QPalette::Base)));
    out << QStringLiteral("  Palette Text       : %1\n")
               .arg(show(text->palette().color(QPalette::Text)));
    out << QStringLiteral("  Fenster WindowText : %1\n")
               .arg(show(window.palette().color(QPalette::WindowText)));
    out << QStringLiteral("  Fenster Text       : %1\n")
               .arg(show(window.palette().color(QPalette::Text)));

    // Der offene Weg, den Text nach innen zu rücken: `setViewportMargins()` ist
    // in QAbstractScrollArea `protected`, von aussen also nicht erreichbar.
    // Gemessen wird deshalb, was `documentMargin` tut — und was das mit der
    // Höhenrechnung macht, die genau diesen Wert schon verrechnet.
    const int heightBefore = window.height();
    const int textHeightBefore = text->height();
    text->document()->setDocumentMargin(text->document()->documentMargin() + 6);
    QCoreApplication::processEvents();
    out << QStringLiteral("  documentMargin +6  -> Fenster %1 (vorher %2), Textbereich %3 (vorher %4)\n")
               .arg(window.height())
               .arg(heightBefore)
               .arg(text->height())
               .arg(textHeightBefore);

    // Und der zweite offene Weg: Innenränder auf dem Textfeld selbst.
    text->document()->setDocumentMargin(text->document()->documentMargin() - 6);
    text->setContentsMargins(6, 6, 6, 6);
    QCoreApplication::processEvents();
    out << QStringLiteral("  contentsMargins 6  -> viewport x=%1 y=%2 %3x%4, Fenster %5\n")
               .arg(text->viewport()->x())
               .arg(text->viewport()->y())
               .arg(text->viewport()->width())
               .arg(text->viewport()->height())
               .arg(window.height());
    out << "\n";
}
}

int main(int argc, char **argv)
{
    // NOLINTNEXTLINE(misc-const-correctness)
    QApplication app(argc, argv);

    if (argc < 2) {
        out << "Aufruf: feldprobe <Prüf-Theme-Verzeichnis> [<theme> …]\n";
        return 2;
    }

    const QByteArray bundled = QByteArray(argv[1]);
    const QByteArray existing = qgetenv("XDG_DATA_DIRS");
    qputenv("XDG_DATA_DIRS", existing.isEmpty() ? bundled : bundled + ':' + existing);

    const QStringList bundledNames{QStringLiteral("denkzettel-test-schmal"),
                                   QStringLiteral("denkzettel-test-breit"),
                                   QStringLiteral("denkzettel-pruef-eckig")};

    out << "== A/B: Die mitgelieferten Prüf-Themes der Testsuite ==\n\n";
    for (const QString &theme : bundledNames) {
        probeTheme(theme);
    }

    out << "== A/B: Ein Name, auf den nichts hört (der Bauwirt ohne Plasma-Themes) ==\n\n";
    probeTheme(QStringLiteral("kein-solches-theme"));

    out << "== A/B: Die auf dieser Maschine installierten Themes ==\n\n";
    for (int i = 2; i < argc; ++i) {
        probeTheme(QString::fromLocal8Bit(argv[i]));
    }

    out << "== C: Welches Paar wählt installedThemePair(), dem captureshots folgt? ==\n\n";
    {
        QString narrow;
        qreal narrowBorder = 0;
        bool found = false;
        for (const QString &theme : installedThemes(bundledNames)) {
            const qreal border = borderOf(theme);
            out << QStringLiteral("  %1: Rand %2, eigene lineedit-Grafik %3\n")
                       .arg(theme)
                       .arg(border)
                       .arg(hasOwnLineEdit(theme) ? "ja" : "nein");
            if (border <= 0 || found) {
                continue;
            }
            if (narrow.isEmpty()) {
                narrow = theme;
                narrowBorder = border;
                continue;
            }
            if (!qFuzzyCompare(border, narrowBorder)) {
                const auto pair = border > narrowBorder ? std::make_pair(narrow, theme)
                                                        : std::make_pair(theme, narrow);
                out << QStringLiteral("  -> Paar: schmal=%1 (eigene Grafik %2)  breit=%3 (eigene Grafik %4)\n")
                           .arg(pair.first,
                                hasOwnLineEdit(pair.first) ? QStringLiteral("ja") : QStringLiteral("nein"),
                                pair.second,
                                hasOwnLineEdit(pair.second) ? QStringLiteral("ja") : QStringLiteral("nein"));
                found = true;
            }
        }
        if (!found) {
            out << "  -> kein Paar gefunden\n";
        }
    }
    out << "\n";

    out << "== D: Das gebaute Fenster ==\n\n";
    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store liess sich nicht oeffnen\n";
        return 1;
    }
    probeWindow(store, QStringLiteral("denkzettel-test-schmal"));
    probeWindow(store, QStringLiteral("denkzettel-test-breit"));

    return 0;
}
