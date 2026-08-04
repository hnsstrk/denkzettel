#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

/**
 * Writes the picture series of the capture window's hull for the handover
 * (DoD 2 and DoD 3, wireframes 4a and 4b, issues #55 and #56).
 *
 * Not a test — a picture maker. It is run by hand, so it stays out of
 * `add_test()`: a failing screenshot writer must not colour the suite red.
 * It is built by an ordinary build all the same, because a bench nobody
 * rebuilds goes stale silently (sprint 5, `libraryshots`).
 *
 * Run it with QT_QPA_PLATFORMTHEME=kde. Without it Qt falls back to a
 * substitute font whose sizes are not the ones the running application uses,
 * and a picture of a window whose height is a line count would then misstate
 * the very thing it is taken for.
 *
 * What these pictures can and cannot show: rounding, outline, surface, colour
 * roles and measurements are all inside the window and come out. The **shadow**
 * does not — it lies outside the widget, so `QWidget::grab()` never sees it,
 * and offscreen there is no compositor that would draw one (measured, #55
 * AK 7). A picture here without a shadow is not a finding.
 *
 * Usage: QT_QPA_PLATFORMTHEME=kde captureshots <target directory>
 */
namespace
{
/**
 * The two desktop themes of wireframe 4a: a narrow border against a wide one.
 *
 * Not named but found, and preferably **real**: the pictures are what the
 * customer and the UI review look at, so they should show Plasma's own hulls
 * and not an SVG of the test suite. Where no such pair is installed — every
 * theme of the official KDE stack carries 4 px, so anywhere without CachyOS
 * packages there is none — the bundled pair steps in, and the runner says so
 * rather than producing two identical series in silence.
 */
std::pair<QString, QString> pickThemes(QTextStream &out)
{
    if (const auto installed = themes::installedThemePair()) {
        out << "Desktop-Themes dieser Bildreihe (installiert): schmal=" << installed->first
            << "  breit=" << installed->second << "\n";
        return *installed;
    }

    out << "Desktop-Themes dieser Bildreihe (MITGELIEFERT — kein installiertes Paar mit\n"
           "verschiedenem Rand gefunden; die Bilder zeigen die Prüf-Themes der Testsuite,\n"
           "nicht Plasmas eigene Hüllen): schmal="
        << themes::bundledNarrow() << "  breit=" << themes::bundledWide() << "\n";
    return {themes::bundledNarrow(), themes::bundledWide()};
}

/**
 * The two colour schemes, with the measured colours of Breeze Light and Breeze
 * Dark (UX investigation of 01.08.2026, `2026-08-01-capture-theme/palette.txt`).
 */
QPalette breezePalette(bool dark)
{
    const QColor window = dark ? QColor(0x20, 0x23, 0x26) : QColor(0xef, 0xf0, 0xf1);
    const QColor text = dark ? QColor(0xfc, 0xfc, 0xfc) : QColor(0x23, 0x26, 0x29);
    const QColor placeholder = dark ? QColor(0xa1, 0xa9, 0xb1) : QColor(0x70, 0x7d, 0x8a);

    QPalette palette;
    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, dark ? QColor(0x14, 0x16, 0x18) : QColor(0xff, 0xff, 0xff));
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::PlaceholderText, placeholder);
    palette.setColor(QPalette::Highlight, QColor(0x3d, 0xae, 0xe9));
    palette.setColor(QPalette::HighlightedText, dark ? QColor(0xfc, 0xfc, 0xfc) : QColor(0xff, 0xff, 0xff));

    return palette;
}

/**
 * The window on a backdrop, the way wireframe 4a draws it.
 *
 * A bare `grab()` of this window has transparent corners, and transparency in
 * a PNG looks like whatever the viewer puts behind it — usually white, on
 * which a rounded white corner is invisible. The rounding is the whole point
 * of AK 1, so the picture brings its own ground: a hatch, in the same two
 * greys the drawing uses, so that every pixel the hull does not claim is
 * visibly not claimed.
 */
void shoot(QWidget &window, const QString &directory, const QString &name)
{
    constexpr int Frame = 24;
    constexpr int HatchStep = 8;

    const QPixmap grabbed = window.grab();

    QImage picture(grabbed.size() + QSize(2 * Frame, 2 * Frame), QImage::Format_ARGB32);
    QPainter painter(&picture);
    picture.fill(QColor(0xf2, 0xf0, 0xeb));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0xe9, 0xe7, 0xe2));
    for (int offset = -picture.height(); offset < picture.width(); offset += 2 * HatchStep) {
        painter.drawPolygon(QPolygon({QPoint(offset, 0),
                                      QPoint(offset + HatchStep, 0),
                                      QPoint(offset + HatchStep + picture.height(), picture.height()),
                                      QPoint(offset + picture.height(), picture.height())}));
    }
    painter.drawPixmap(Frame, Frame, grabbed);

    if (!picture.save(QDir(directory).filePath(name))) {
        qFatal("Bild ließ sich nicht schreiben: %s", qPrintable(name));
    }
}

/** The three states of AK 7: empty, typed, and eight lines with a scrollbar. */
void typeState(QPlainTextEdit *text, int state)
{
    switch (state) {
    case 0:
        text->clear();
        return;
    case 1:
        text->setPlainText(QStringLiteral("Denkzettel soll die Hülle des Desktop-Themes tragen"));
        return;
    default:
        text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht\nneun\nzehn\nelf\nzwölf"));
        return;
    }
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    if (app.arguments().size() < 2) {
        qFatal("Aufruf: captureshots <Zielverzeichnis>");
    }
    const QString directory = app.arguments().at(1);
    if (!QDir().mkpath(directory)) {
        qFatal("Zielverzeichnis ließ sich nicht anlegen: %s", qPrintable(directory));
    }

    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        qFatal("Store ließ sich nicht öffnen: %s", qPrintable(store.lastError()));
    }

    QTextStream out(stdout);
    // The bundled test themes go on the data path before the first theme is
    // resolved: they are the fallback for a machine that has no two installed
    // themes with different borders.
    themes::addBundledThemesToDataPath();

    const auto [narrowTheme, wideTheme] = pickThemes(out);
    const QStringList themeList{narrowTheme, wideTheme};
    const QStringList themeNames{QStringLiteral("schmal"), QStringLiteral("breit")};
    const QStringList stateNames{QStringLiteral("leer"),
                                 QStringLiteral("getippt"),
                                 QStringLiteral("acht-zeilen")};

    // 1 to 12 — the hull under two desktop themes and two colour schemes, in
    // each of the three states (#55, AK 7). One window per picture: the hull
    // has to be right when the window is built, not only after a change.
    int number = 0;
    for (int theme = 0; theme < themeList.size(); ++theme) {
        for (const bool dark : {false, true}) {
            for (int state = 0; state < stateNames.size(); ++state) {
                app.setPalette(breezePalette(dark));

                CaptureWindow window(&store);
                window.reloadDesktopTheme(themeList.at(theme));
                typeState(window.findChild<QPlainTextEdit *>(), state);
                window.show();

                shoot(window,
                      directory,
                      QStringLiteral("%1-rand-%2-%3-%4.png")
                          .arg(++number, 2, 10, QLatin1Char('0'))
                          .arg(themeNames.at(theme),
                               dark ? QStringLiteral("dunkel") : QStringLiteral("hell"),
                               stateNames.at(state)));
            }
        }
    }

    // 13 and 14 — the field height after a font change (#56). The font is set
    // on the text area itself, as AK 2 of that issue prescribes; both pictures
    // show the same standing window, so what differs between them is the font
    // and nothing else.
    app.setPalette(breezePalette(false));
    for (const int pointSize : {9, 24}) {
        CaptureWindow window(&store);
        window.reloadDesktopTheme(narrowTheme);

        auto *text = window.findChild<QPlainTextEdit *>();
        QFont font = text->font();
        font.setPointSize(pointSize);
        text->setFont(font);
        window.show();

        shoot(window,
              directory,
              QStringLiteral("%1-schrift-%2pt-fuenf-zeilen.png").arg(++number).arg(pointSize));
    }

    return 0;
}
