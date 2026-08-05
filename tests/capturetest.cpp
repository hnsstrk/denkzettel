#include "capture/capturewindow.h"
#include "capture/textareaheight.h"
#include "desktopthemes.h"
#include "store/store.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowShadow>

#include <QApplication>
#include <QDir>
#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QPlainTextEdit>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>

#include <memory>
#include <utility>

/**
 * Unit tests of the growth logic and of the saving path of the capture window
 * (SPEC 3). The window's behaviour under the compositor — focus, placement,
 * appearance — belongs to the manual checklist (SPEC 16).
 */
class CaptureTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void restsAtFiveLines();
    void growsWithTheText();
    void stopsAtEightLines();
    void windowFollowsTheTextHeight();
    void heightFollowsAFontChange();

    void savesTextOnControlReturn();
    void keepsBlankTextOutOfTheStore();
    void discardsTextOnEscape();

    void textsFollowAColourSchemeChange();

    void hullFollowsTheDesktopTheme();
    void hullFollowsAnInstalledDesktopTheme();
    void readsTheDesktopThemeFromPlasmarc();
    void paintsTheThemesOwnHullInOnePiece();
    void noteTextUsesTheWindowTextRole();
    void footerHasMoreAirThanTheApplicationName();
    void hullIsCompleteAtFiveAndEightLines();
    void hullFollowsTheWindowPixelRatio();
    void hullHasNoStairAtTheCorner();
    void hullHoldsAtTheCustomersScale();
    void squareThemeKeepsSquareCorners();
    void readsTheContrastEffectOfTheDesktopTheme();
    void takesTheOpaqueVariantWithoutABlurringCompositor();
    void wearsNoDecoration();
    void bindsAShadowFromTheThemeTiles();
    void staysUsableWithoutADesktopTheme();

private:
    QPlainTextEdit *textArea() const;
    static QImage shot(QWidget &window);
    /** How far into the top row of the picture the hull is still transparent. */
    static int cornerRun(const QImage &picture);
    /**
     * The first column of each of the first ten rows in which the hull covers
     * more than half — the edge walk of AK 4. A smoothed arc moves inwards row
     * by row; a stair stands still and then jumps by two columns or more.
     */
    static QList<int> edgeWalk(const QImage &picture);
    /** Empty when the walk is smooth, otherwise what is wrong with it. */
    static QString faultOfEdgeWalk(const QImage &picture, int allowedStairs);
    /**
     * The hull the desktop theme itself draws at that size — the window's only
     * source since issue #83.
     *
     * The `opaque` selector is set here for the same reason the window sets it:
     * offscreen nothing blurs, and the theme's translucent variant would leave
     * a window one cannot read (AK 7). Leaving it out is what
     * takesTheOpaqueVariantWithoutABlurringCompositor() measures.
     */
    static QImage themeHull(const QString &theme, const QSize &size, const QStringList &selectors);
    static void writePlasmarc(const QString &theme);
    void checkHullDiffersBetween(const QString &narrow, const QString &wide);

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    std::unique_ptr<CaptureWindow> m_window;
};

namespace
{
/**
 * No desktop theme is named here, and that is the point.
 *
 * A fixed pair of names ties the assertion to the distribution the test was
 * written on: of the eight themes installed on that machine only CachyOS
 * packages carry a border other than 4 px, so anywhere else the pair collapses
 * and the comparison silently compares 4 against 4. Where the themes come from
 * and why there are two sources stands in `tests/desktopthemes.h`.
 *
 * The assertions below stay **relative** either way — theme against theme,
 * never against a number. `marginSize()` hands out `7,99998` where a drawing
 * would have written 8.
 */
const QString NarrowBorderTheme = themes::bundledNarrow();
const QString WideBorderTheme = themes::bundledWide();

/**
 * The bundled theme whose corner pieces are rectangles (issue #83, AK 9).
 *
 * The customer's question was whether the window rounds everything off even
 * under a theme that does not round. All eight themes installed on his machine
 * round, so no installed theme can answer it; this one was built to, and it
 * moved here out of the acceptance evidence of sprint 6 where it was measured.
 */
const QString SquareCornerTheme = themes::bundledSquare();

/**
 * KSvg's own fallback, and the theme the customer's machine runs on: his
 * `plasmarc` names none, so the window lands here.
 */
QString fallbackTheme()
{
    return QStringLiteral("default");
}

/** The selectors the window uses where nothing blurs — offscreen, that is. */
QStringList opaqueSelectors()
{
    return {QStringLiteral("opaque")};
}

/** How many rows of the corner the edge walk of AK 4 looks at. */
constexpr int EdgeWalkRows = 10;
}

void CaptureTest::initTestCase()
{
    // Without the domain every i18n() call in the window warns.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // The window reads the desktop theme out of `plasmarc`, and one test
    // writes that file. Test mode points QStandardPaths at a directory of the
    // test's own, so the developer's desktop theme is never touched.
    QStandardPaths::setTestModeEnabled(true);
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));

    // Before the first theme is resolved: the two themes of the test itself go
    // on the data path, next to the installed ones rather than instead of them.
    //
    // Except in the one run that is *about* having no theme at all — there the
    // whole point is an empty data path, and mounting ours would hand it the
    // very thing it must do without.
    if (!qEnvironmentVariableIsSet("DENKZETTEL_TEST_WITHOUT_DESKTOP_THEME")) {
        themes::addBundledThemesToDataPath();
        QVERIFY2(themes::borderOf(NarrowBorderTheme) > 0 && themes::borderOf(WideBorderTheme) > 0,
                 "Die mitgelieferten Prüf-Themes lösen nicht auf — steht tests/themes/ am Platz?");
    }
}

void CaptureTest::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    m_store = std::make_unique<Store>(m_dir->filePath(QStringLiteral("denkzettel.db")));
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    m_window = std::make_unique<CaptureWindow>(m_store.get());
}

void CaptureTest::cleanup()
{
    m_window.reset();
    m_store.reset();
    m_dir.reset();
}

QPlainTextEdit *CaptureTest::textArea() const
{
    return m_window->findChild<QPlainTextEdit *>();
}

void CaptureTest::restsAtFiveLines()
{
    // An empty document still reports one line, so the resting height has to
    // come from the lower bound.
    QCOMPARE(capture::textAreaHeight(0, 20, 4), 104);
    QCOMPARE(capture::textAreaHeight(1, 20, 4), 104);
    QCOMPARE(capture::textAreaHeight(5, 20, 4), 104);
}

void CaptureTest::growsWithTheText()
{
    QCOMPARE(capture::textAreaHeight(6, 20, 4), 124);
    QCOMPARE(capture::textAreaHeight(7, 20, 4), 144);
    QCOMPARE(capture::textAreaHeight(8, 20, 4), 164);

    // The chrome is added on top of the text, whatever the line count.
    QCOMPARE(capture::textAreaHeight(6, 20, 0), 120);
}

void CaptureTest::stopsAtEightLines()
{
    const int eightLines = capture::textAreaHeight(8, 20, 4);
    QCOMPARE(capture::textAreaHeight(9, 20, 4), eightLines);
    QCOMPARE(capture::textAreaHeight(400, 20, 4), eightLines);
}

void CaptureTest::windowFollowsTheTextHeight()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    const int resting = m_window->height();

    text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs"));
    const int sixLines = m_window->height();
    QVERIFY2(sixLines > resting, qPrintable(QStringLiteral("%1 <= %2").arg(sixLines).arg(resting)));

    text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
    const int eightLines = m_window->height();
    QVERIFY(eightLines > sixLines);

    // Beyond the maximum the scrollbar takes over and the window stands still.
    text->setPlainText(text->toPlainText() + QStringLiteral("\nneun\nzehn\nelf\nzwölf"));
    QCOMPARE(m_window->height(), eightLines);

    text->clear();
    QCOMPARE(m_window->height(), resting);
}

void CaptureTest::heightFollowsAFontChange()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    // The font is set on the widget itself, as issue #56 prescribes: Plasma
    // does not hand a font change to a standing Qt Widgets application at all
    // (B6 of the theme report), so waiting for that road would mean waiting
    // for a road that does not exist. The widget sees the change on every
    // road, this one included.
    //
    // Two clearly different sizes, and the assertion is relative — the height
    // is read against the line spacing of the font in force, not against a
    // pixel count.
    for (const int pointSize : {9, 24}) {
        QFont font = text->font();
        font.setPointSize(pointSize);
        text->setFont(font);

        const int lineSpacing = text->fontMetrics().lineSpacing();
        // What the widget needs beyond the text itself; the same two sources
        // adjustHeight() reads.
        const int chrome = 2 * qRound(text->document()->documentMargin()) + 2 * text->frameWidth();

        QCOMPARE(text->height() - chrome, capture::MinTextLines * lineSpacing);

        // And it still grows with the text under the new font, up to eight.
        text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
        QCOMPARE(text->height() - chrome, capture::MaxTextLines * lineSpacing);

        text->clear();
    }
}

void CaptureTest::savesTextOnControlReturn()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    m_window->show();
    QVERIFY(m_window->isVisible());

    text->setPlainText(QStringLiteral("  Bücher über Straßenbahnen ansehen  "));
    QTest::keyClick(text, Qt::Key_Return, Qt::ControlModifier);

    // The first note of a fresh database gets id 1.
    const std::optional<Note> stored = m_store->note(1);
    QVERIFY2(stored.has_value(), qPrintable(m_store->lastError()));
    QCOMPARE(stored->content, QStringLiteral("Bücher über Straßenbahnen ansehen"));
    QCOMPARE(stored->type, Note::Type::Text);
    QCOMPARE(stored->state, Note::State::New);
    QVERIFY(stored->createdAt.isValid());

    QVERIFY(text->toPlainText().isEmpty());
    QVERIFY(!m_window->isVisible());
}

void CaptureTest::keepsBlankTextOutOfTheStore()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    text->setPlainText(QStringLiteral("   \n  "));
    QTest::keyClick(text, Qt::Key_Return, Qt::ControlModifier);

    QVERIFY(!m_store->note(1).has_value());
    QVERIFY(text->toPlainText().isEmpty());
}

void CaptureTest::discardsTextOnEscape()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    m_window->show();
    QVERIFY(m_window->isVisible());

    text->setPlainText(QStringLiteral("doch nicht"));
    QTest::keyClick(text, Qt::Key_Escape);

    QVERIFY(!m_store->note(1).has_value());
    QVERIFY(text->toPlainText().isEmpty());
    QVERIFY(!m_window->isVisible());
}

void CaptureTest::textsFollowAColourSchemeChange()
{
    // The daemon builds the window once and keeps it (SPEC 2.1), so a colour
    // scheme change reaches a window that is already standing. Every text has
    // to follow it — the application name and the key hint included (issue #54).
    const QPalette startPalette = qApp->palette();

    QPalette switched = startPalette;
    switched.setColor(QPalette::WindowText, QColor(0x23, 0x26, 0x29));
    switched.setColor(QPalette::PlaceholderText, QColor(0x70, 0x7d, 0x8a));
    qApp->setPalette(switched);

    // Qt hands the new palette to the widgets through a posted event; without a
    // running event loop the test has to let it through itself.
    QCoreApplication::processEvents();

    const QList<QLabel *> labels = m_window->findChildren<QLabel *>();
    QCOMPARE(labels.size(), 2);

    for (QLabel *label : labels) {
        // What the label paints with: its own palette, read through its role.
        QCOMPARE(label->palette().color(label->foregroundRole()), QColor(0x70, 0x7d, 0x8a));
    }

    qApp->setPalette(startPalette);
}

QImage CaptureTest::shot(QWidget &window)
{
    // Shown, then let through: a window that was never shown has no laid-out
    // layout at all — every child sits at zero — and one that was only resized
    // gets that resize as a posted event, so the picture would be one step
    // behind the size: the hull of the previous height on the geometry of the
    // current one. Both were measured on this window.
    window.show();
    QCoreApplication::processEvents();

    return window.grab().toImage();
}

int CaptureTest::cornerRun(const QImage &picture)
{
    int x = 0;
    while (x < picture.width() && qAlpha(picture.pixel(x, 0)) == 0) {
        ++x;
    }
    return x;
}

QList<int> CaptureTest::edgeWalk(const QImage &picture)
{
    QList<int> columns;
    for (int y = 0; y < EdgeWalkRows && y < picture.height(); ++y) {
        int found = picture.width();
        for (int x = 0; x < picture.width(); ++x) {
            if (qAlpha(picture.pixel(x, y)) >= 128) {
                found = x;
                break;
            }
        }
        columns << found;
    }
    return columns;
}

QString CaptureTest::faultOfEdgeWalk(const QImage &picture, int allowedStairs)
{
    const QList<int> walk = edgeWalk(picture);
    QStringList parts;
    for (const int column : std::as_const(walk)) {
        parts << QString::number(column);
    }
    const QString trace = parts.join(QStringLiteral("·"));

    int stairs = 0;
    for (int row = 1; row < walk.size(); ++row) {
        // Monotonically **falling**: the corner opens up towards the bottom.
        if (walk.at(row) > walk.at(row - 1)) {
            return QStringLiteral("Rückschritt in Zeile %1: %2").arg(row).arg(trace);
        }
        if (walk.at(row - 1) - walk.at(row) >= 2) {
            ++stairs;
        }
    }

    if (stairs > allowedStairs) {
        return QStringLiteral("%1 Stufen, erlaubt %2: %3").arg(stairs).arg(allowedStairs).arg(trace);
    }
    return {};
}

QImage CaptureTest::themeHull(const QString &theme, const QSize &size, const QStringList &selectors)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    imageSet.setSelectors(selectors);

    KSvg::FrameSvg frame;
    // No rendering cache: a FrameSvg keeps what it once resolved, and this
    // helper is called several times per run with different themes and
    // selectors. Without the line the second call could measure the first
    // one's picture — the fault measurement 3 of sprint 6 uncovered.
    frame.setUsingRenderingCache(false);
    frame.setColorSet(KSvg::Svg::Window);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.setDevicePixelRatio(qApp->devicePixelRatio());
    frame.resizeFrame(size);

    return frame.framePixmap().toImage();
}

void CaptureTest::writePlasmarc(const QString &theme)
{
    auto config = KSharedConfig::openConfig(QStringLiteral("plasmarc"));
    KConfigGroup(config, QStringLiteral("Theme")).writeEntry("name", theme);
    config->sync();
}

void CaptureTest::hullFollowsTheDesktopTheme()
{
    // Against the two themes the tests bring themselves, so that this holds on
    // every machine — including one with no desktop theme installed at all.
    checkHullDiffersBetween(NarrowBorderTheme, WideBorderTheme);
}

void CaptureTest::hullFollowsAnInstalledDesktopTheme()
{
    // The same assertion against **real** Plasma themes, discovered by
    // measurement rather than named. This is the one that shows the code reads
    // a desktop theme and not merely an SVG of ours — and the one that cannot
    // always run: a machine whose themes all carry the same border has no pair
    // to compare (see tests/desktopthemes.h).
    const auto pair = themes::installedThemePair();
    if (!pair) {
        QSKIP("Kein Paar installierter Desktop-Themes mit verschiedenem Rand gefunden — "
              "die mitgelieferten Prüf-Themes tragen die Zusicherung.");
    }

    checkHullDiffersBetween(pair->first, pair->second);
}

void CaptureTest::checkHullDiffersBetween(const QString &narrow, const QString &wide)
{
    // AK 1 and AK 5 in one run, and deliberately on **one** window: the daemon
    // builds it at start and keeps it (SPEC 2.1), so a theme change has to
    // reach a window that is already standing.
    //
    // Nothing here compares against a number. Two themes with different borders
    // are held against each other — that is the assertion wireframe 4b writes
    // down, and the reason it writes it that way is that a drawing which fixes
    // a radius would be wrong.
    m_window->reloadDesktopTheme(narrow);
    const QMargins narrowMargins = m_window->layout()->contentsMargins();
    const int narrowCorner = cornerRun(shot(*m_window));

    m_window->reloadDesktopTheme(wide);
    const QMargins wideMargins = m_window->layout()->contentsMargins();
    const int wideCorner = cornerRun(shot(*m_window));

    // The border of the theme is claimed on top of the inner spacing of 4b, so
    // the wider theme pushes the content further in on every side.
    QVERIFY2(wideMargins.left() > narrowMargins.left(),
             qPrintable(QStringLiteral("%1 <= %2").arg(wideMargins.left()).arg(narrowMargins.left())));
    QVERIFY(wideMargins.top() > narrowMargins.top());
    QVERIFY(wideMargins.right() > narrowMargins.right());
    QVERIFY(wideMargins.bottom() > narrowMargins.bottom());

    // And the corner is a different corner. Measured on the picture, and the
    // assertion is that the two **differ** — not that the wider border carries
    // the rounder corner. Deriving the one from the other is what wireframe 4b
    // forbids in so many words, and the installed themes show why: here `default`
    // pairs a 4 px border with a corner run of 6, `CachyOS-Nord-round` an 8 px
    // border with 7. Both must be corners at all — a square window runs to zero.
    QVERIFY(narrowCorner > 0);
    QVERIFY(wideCorner > 0);
    QVERIFY2(wideCorner != narrowCorner,
             qPrintable(QStringLiteral("beide %1").arg(wideCorner)));
}

void CaptureTest::readsTheDesktopThemeFromPlasmarc()
{
    // KSvg does not find the desktop theme by itself — it stands in `plasmarc`,
    // and without being handed over KSvg stays on `default` whatever that file
    // says. This is the road the watch on the file takes; that the watch fires
    // at all is measured beside the test (measurement 2 of this story).
    writePlasmarc(NarrowBorderTheme);
    m_window->reloadDesktopTheme();
    const QMargins narrowMargins = m_window->layout()->contentsMargins();

    writePlasmarc(WideBorderTheme);
    m_window->reloadDesktopTheme();

    QVERIFY(m_window->layout()->contentsMargins().left() > narrowMargins.left());
}

void CaptureTest::paintsTheThemesOwnHullInOnePiece()
{
    // One continuous surface, not a box inside a box (wireframe 4b): the ground
    // behind the text area is the same ground as beside it.
    //
    // And it is the **theme's** ground, which is what issue #83 turned around.
    // Until then the window filled the theme's alpha mask with a colour of the
    // palette; now it draws the theme's own graphic and adds nothing. The
    // assertion is therefore held against a second rendering of the same
    // graphic and not against a colour — under this bundled theme the two
    // differ visibly, so a window that went back to filling would be caught.
    m_window->reloadDesktopTheme(NarrowBorderTheme);

    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    const QImage picture = shot(*m_window);
    const QImage hull = themeHull(NarrowBorderTheme, m_window->size(), opaqueSelectors());
    QCOMPARE(hull.size(), picture.size());

    // Inside the text area, in its lower right corner where no text stands.
    const QPoint behindTheText(text->x() + text->width() - 20, text->y() + text->height() - 6);
    // Beside it, in the gap between application name and text area.
    const QPoint besideTheText(m_window->width() / 2, text->y() - 4);

    QCOMPARE(picture.pixelColor(behindTheText), picture.pixelColor(besideTheText));
    QCOMPARE(picture.pixelColor(behindTheText), hull.pixelColor(behindTheText));
    QCOMPARE(picture.pixelColor(besideTheText), hull.pixelColor(besideTheText));

    // And it is not the palette's colour any more. Without this line the three
    // comparisons above would still hold if the graphic happened to carry the
    // scheme colour — which under `default` it does, and under this theme it
    // does not (measured: the bundled graphic draws black, the scheme does not).
    QVERIFY2(picture.pixelColor(besideTheText) != m_window->palette().color(QPalette::Window),
             qPrintable(picture.pixelColor(besideTheText).name(QColor::HexArgb)));
}

void CaptureTest::noteTextUsesTheWindowTextRole()
{
    // On the continuous surface the note text carries `WindowText` and not the
    // role for entry fields — measured over 18 colour schemes at 4,74:1 against
    // 4,22:1, above and below the minimum of 4,5:1 (wireframe 4b). Copied on
    // every palette change, so it follows the scheme instead of freezing (#54).
    QPlainTextEdit *text = textArea();
    QVERIFY(text);
    QCOMPARE(text->palette().color(QPalette::Text),
             m_window->palette().color(QPalette::WindowText));

    const QPalette startPalette = qApp->palette();
    QPalette switched = startPalette;
    switched.setColor(QPalette::WindowText, QColor(0xfc, 0xfc, 0xfc));
    switched.setColor(QPalette::Text, QColor(0x11, 0x22, 0x33));
    qApp->setPalette(switched);
    QCoreApplication::processEvents();

    QCOMPARE(text->palette().color(QPalette::Text), QColor(0xfc, 0xfc, 0xfc));

    qApp->setPalette(startPalette);
    QCoreApplication::processEvents();
}

void CaptureTest::footerHasMoreAirThanTheApplicationName()
{
    // Since the separator line was dropped this difference is the entire
    // grouping of the window (wireframe 4b) — measured on the laid-out widgets,
    // not on the spacing values that produced them.
    //
    // At both window sizes SPEC 3 knows, as DoD 1 asks: the window grows in the
    // middle, and a spacing that only held at the resting height would pass a
    // single-size check.
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    // Which label is which is read off the laid-out window, not off the order
    // they were created in — before the window has been shown they all sit at
    // zero, and the sorting below would then pick either one.
    m_window->show();
    QCoreApplication::processEvents();

    const QList<QLabel *> labels = m_window->findChildren<QLabel *>();
    QCOMPARE(labels.size(), 2);
    QLabel *appName = labels.at(0)->y() < labels.at(1)->y() ? labels.at(0) : labels.at(1);
    QLabel *footer = appName == labels.at(0) ? labels.at(1) : labels.at(0);

    for (const QString &content :
         {QString(), QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht")}) {
        text->setPlainText(content);
        QCoreApplication::processEvents();

        const int belowTheName = text->y() - (appName->y() + appName->height());
        const int aboveTheFooter = footer->y() - (text->y() + text->height());

        QVERIFY2(belowTheName > 0 && aboveTheFooter > 0,
                 qPrintable(QStringLiteral("%1 / %2").arg(belowTheName).arg(aboveTheFooter)));
        QVERIFY2(aboveTheFooter > belowTheName,
                 qPrintable(QStringLiteral("%1 <= %2").arg(aboveTheFooter).arg(belowTheName)));
    }

    text->clear();
}

void CaptureTest::hullIsCompleteAtFiveAndEightLines()
{
    // The window grows with every keystroke (AK 4), and the hull has to grow
    // with it: same corner, closed everywhere, on both of the sizes SPEC 3
    // names. Checked under both desktop themes — a hull that only survives the
    // narrow border would pass a one-theme check.
    // The bundled themes always, an installed one on top wherever there is one:
    // the shape has to have held against a real Plasma theme, not merely
    // against an SVG of ours.
    QStringList checked{NarrowBorderTheme, WideBorderTheme};
    if (const auto installed = themes::anyInstalledTheme()) {
        checked << *installed;
    }

    for (const QString &theme : std::as_const(checked)) {
        // A window of its own per theme, and that is not tidiness. A window
        // that has been shown does not shrink back below the minimum its layout
        // took from the taller state — measured, and measured on the state
        // before this story as well, so it is not this story's doing. Reusing
        // one window here would carry the eight-line height into the next
        // theme's resting picture.
        CaptureWindow window(m_store.get());
        window.reloadDesktopTheme(theme);

        auto *text = window.findChild<QPlainTextEdit *>();
        QVERIFY(text);

        const QImage atFive = shot(window);
        const int restingHeight = atFive.height();
        // There has to be a hull at all before it is worth asking whether it is
        // the same one at both sizes: without one every corner reads zero and
        // the comparison below would hold on a square window.
        QVERIFY(cornerRun(atFive) > 0);

        text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
        const QImage atEight = shot(window);
        QVERIFY(atEight.height() > restingHeight);

        for (const QImage &picture : {atFive, atEight}) {
            // The corner is the theme's, and it is the same one at either size:
            // the corner pieces do not stretch, only the middle does.
            QCOMPARE(cornerRun(picture), cornerRun(atFive));

            // Closed — and since issue #83 that no longer means opaque. Under
            // a Plasma overlay the surface lets the ground through on purpose
            // (`default` covers 84,7 %), so the demand is that every edge and
            // the middle carry **the theme's own** coverage and no hole of
            // ours. Held against a second rendering of the same graphic: a
            // number here would have to be the number of one theme, and this
            // run walks three.
            // The size out of the picture, not out of the window: the window
            // has grown to eight lines by now, and `atFive` is the shorter of
            // the two. Taken from the window both rounds would be measured
            // against the taller hull.
            const QImage hull = themeHull(theme,
                                          picture.size() / qApp->devicePixelRatio(),
                                          opaqueSelectors());
            QCOMPARE(hull.size(), picture.size());

            for (const QPoint &point : {QPoint(picture.width() / 2, 0),
                                        QPoint(picture.width() / 2, picture.height() - 1),
                                        QPoint(0, picture.height() / 2),
                                        QPoint(picture.width() - 1, picture.height() / 2),
                                        QPoint(picture.width() / 2, picture.height() / 2)}) {
                QVERIFY2(qAlpha(picture.pixel(point)) > 0,
                         qPrintable(QStringLiteral("%1 bei %2,%3")
                                        .arg(theme)
                                        .arg(point.x())
                                        .arg(point.y())));
                QCOMPARE(qAlpha(picture.pixel(point)), qAlpha(hull.pixel(point)));
            }
        }
    }
}

void CaptureTest::hullFollowsTheWindowPixelRatio()
{
    // A FrameSvg stands at ratio 1 whatever the session scales to — it does not
    // follow the screen by itself, and the application has to hand the number
    // over (measured, `sonde1-rahmenmasse-offscreen.txt`). This run holds the
    // hull's ratio against the window's; hullHoldsAtTheCustomersScale() below
    // runs the same assertion again at the customer's 1,6, where the two can
    // actually differ.
    m_window->reloadDesktopTheme(NarrowBorderTheme);
    const QImage picture = shot(*m_window);

    QCOMPARE(m_window->hullDevicePixelRatio(), m_window->devicePixelRatioF());

    // And the picture is that many pixels wide, not merely labelled so.
    QCOMPARE(picture.width(), qRound(m_window->width() * m_window->devicePixelRatioF()));

    // A resize has to carry the ratio with it: resizeFrame() alone would leave
    // the hull at whatever it was set to last.
    textArea()->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
    QCoreApplication::processEvents();
    QCOMPARE(m_window->hullDevicePixelRatio(), m_window->devicePixelRatioF());
}

void CaptureTest::hullHasNoStairAtTheCorner()
{
    // The customer's finding B1: the arc of the corner ran in steps. The cause
    // was the road, not the theme — the old hull came from the `mask-` elements
    // of the graphic, which are coarser than its frame elements and were being
    // scaled up from ratio 1. Drawn in one piece from the frame elements the
    // walk is smooth (measured, `native-huelle-breeze.txt`).
    //
    // What is asserted is the **number of stairs**, not a step of at most one:
    // an arc of radius r moves √(2r−1) pixels in its topmost row, so near the
    // apex it always moves by more than one. That is geometry, not a fault.
    //
    // Against `default`, and that name is not a shortcut: the numbers of AK 4
    // were measured on it, it is KSvg's own fallback, and it is the theme the
    // customer's machine runs on because `plasmarc` names none. The counts do
    // not carry over to an arbitrary theme — a wider border makes a longer arc
    // and the topmost row then moves by more (measured over eight themes,
    // `messungen/kantenlauf-je-theme.txt`).
    if (themes::borderOf(fallbackTheme()) <= 0) {
        QSKIP("Das Theme `default` löst hier nicht auf — ohne installierte Plasma-Themes "
              "ist der Kantenlauf von AK 4 nicht zu messen.");
    }

    const int allowed = qFuzzyCompare(m_window->devicePixelRatioF(), 1.0) ? 0 : 1;

    m_window->reloadDesktopTheme(fallbackTheme());
    QVERIFY2(faultOfEdgeWalk(shot(*m_window), allowed).isEmpty(),
             qPrintable(faultOfEdgeWalk(shot(*m_window), allowed)));

    // And once more on the variant the customer actually sees. Offscreen the
    // window draws the opaque one — nothing blurs here, so AK 7 picks it — and
    // that variant hardly rounds at all (walk 1·0·0…). The translucent one
    // carries the arc the finding was about, and no picture taken offscreen can
    // show it on the window itself.
    const QString fault =
        faultOfEdgeWalk(themeHull(fallbackTheme(), m_window->size(), {}), allowed);
    QVERIFY2(fault.isEmpty(), qPrintable(QStringLiteral("durchscheinende Fassung: %1").arg(fault)));
}

void CaptureTest::hullHoldsAtTheCustomersScale()
{
    // The two assertions above once more, in a process that scales the way the
    // customer's session does. It has to be a process of its own: the pixel
    // ratio is fixed when the platform comes up, and the geometry assertions of
    // the other tests measure spacings at ratio 1.
    //
    // Offscreen `QT_SCALE_FACTOR=1,6` yields exactly 1,6. Under Wayland it
    // would **multiply** with the session's own scaling and yield 2,56, which
    // is why the session evidence of AK 3 sets nothing at all (measured, F4).
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("QT_SCALE_FACTOR"), QStringLiteral("1.6"));

    QProcess child;
    child.setProcessEnvironment(environment);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(QCoreApplication::applicationFilePath(),
                {QStringLiteral("hullFollowsTheWindowPixelRatio"),
                 QStringLiteral("hullHasNoStairAtTheCorner")});

    QVERIFY(child.waitForFinished(60000));
    QVERIFY2(child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0,
             child.readAll().constData());
}

void CaptureTest::squareThemeKeepsSquareCorners()
{
    // The customer's second question to finding B1: "does it bluntly round
    // everything off, even under a theme with no rounded corners?" All eight
    // themes on his machine round, so this is the theme that answers it — the
    // shape comes from the theme's corner pieces and from nowhere else.
    QVERIFY2(themes::borderOf(SquareCornerTheme) > 0,
             "Das eckige Prüf-Theme löst nicht auf — steht tests/themes/ am Platz?");

    m_window->reloadDesktopTheme(SquareCornerTheme);
    const QImage picture = shot(*m_window);

    // No corner is cut away at all, and the very first pixel of the top row
    // already carries the theme's full coverage.
    QCOMPARE(cornerRun(picture), 0);
    QCOMPARE(qAlpha(picture.pixel(0, 0)), 255);

    // Not one column of movement over the whole walk: a rectangle has no arc.
    const QList<int> walk = edgeWalk(picture);
    for (const int column : walk) {
        QCOMPARE(column, 0);
    }
}

void CaptureTest::readsTheContrastEffectOfTheDesktopTheme()
{
    // The second registration Plasma makes (AK 6). The four numbers come out of
    // the theme's own metadata file, read with KConfig rather than through
    // `Plasma::Theme` — that class would pull libPlasma and QtQuick in for four
    // numbers (build decision of the strand, handover report).
    const capture::ContrastEffect wide = capture::contrastEffectOf(WideBorderTheme);
    QVERIFY(wide.enabled);
    QCOMPARE(wide.contrast, 1.0);
    QCOMPARE(wide.intensity, 0.4);
    QCOMPARE(wide.saturation, 1.4);

    // A theme without the group asks for nothing, and neither does a name
    // nothing answers to. Both matter: the second is the road a machine with no
    // desktop themes at all takes.
    QVERIFY(!capture::contrastEffectOf(NarrowBorderTheme).enabled);
    QVERIFY(!capture::contrastEffectOf(QStringLiteral("kein-solches-theme")).enabled);
}

void CaptureTest::takesTheOpaqueVariantWithoutABlurringCompositor()
{
    // SPEC 3.2 point 4 promises a window that stays opaque and usable outside a
    // Plasma session. The theme's own answer to that is its `opaque` variant,
    // and picking it is what Plasma does — not an adjustment of ours (AK 7).
    //
    // Offscreen nothing blurs, so this is the state under test here. The
    // condition is deliberately not hung on
    // `KWindowEffects::isEffectAvailable(BlurBehind)`: that one answers false in
    // the customer's own session until we have registered once, and a window
    // built on it would start out opaque in exactly the session where it must
    // not (measured, F9 of the pre-check).
    QVERIFY(!capture::sessionBlursBehindWindows());

    // The theme has to be one that **ships both variants**, and it is searched
    // for rather than named. Taking whatever theme came first turned this into
    // an assertion that could not fail: `CachyOS-Nord-round` ships neither
    // variant, so the two renderings were identical and removing the selector
    // from the window changed nothing. Measured — the mutation probe stayed
    // green and said so (`messungen/m12-mutationsproben.txt`, erster Lauf).
    //
    // The search runs while the window wears a **bundled** theme, and that is
    // not tidiness either: two live `KSvg::ImageSet` instances of the same theme
    // name share their selectors. A second one built beside the window's own
    // reports `opaque` although nobody gave it any, and renders accordingly
    // (measured, `messungen/m13-ksvg-selektoren.txt`). Held against the theme
    // the window is wearing, the counter-check would compare opaque with opaque.
    m_window->reloadDesktopTheme(NarrowBorderTheme);

    const QSize size = m_window->size();
    const QPoint centre(size.width() / 2, size.height() / 2);
    QString candidate;
    int translucentAlpha = 0;
    int opaqueAlpha = 0;
    const QStringList installed = themes::installedThemes();
    for (const QString &theme : installed) {
        const int loose = qAlpha(themeHull(theme, size, {}).pixel(centre));
        const int tight = qAlpha(themeHull(theme, size, opaqueSelectors()).pixel(centre));
        if (loose != tight) {
            candidate = theme;
            translucentAlpha = loose;
            opaqueAlpha = tight;
            break;
        }
    }
    if (candidate.isEmpty()) {
        QSKIP("Kein installiertes Theme unterscheidet opaque von translucent — die Wahl wäre "
              "hier nicht messbar. Die mitgelieferten Prüf-Themes bringen keine "
              "Auswahlpfade mit.");
    }

    m_window->reloadDesktopTheme(candidate);
    const QImage picture = shot(*m_window);

    // The coverage in the middle of the surface is the same number at every
    // window size — the centre piece of the graphic is stretched, not redrawn —
    // so the two readings above stay comparable even though a wider theme
    // border makes a taller window.
    const int drawn = qAlpha(picture.pixel(picture.width() / 2, picture.height() / 2));
    const QString trace = QStringLiteral("%1: gezeichnet %2, opaque %3, durchscheinend %4")
                              .arg(candidate)
                              .arg(drawn)
                              .arg(opaqueAlpha)
                              .arg(translucentAlpha);
    QVERIFY2(drawn == opaqueAlpha, qPrintable(trace));
    QVERIFY2(drawn != translucentAlpha, qPrintable(trace));
}

void CaptureTest::wearsNoDecoration()
{
    // SPEC 3 unchanged: the hull replaces nothing, it is what a window without
    // a title bar wears instead of nothing (AK 6).
    QVERIFY(m_window->windowFlags().testFlag(Qt::FramelessWindowHint));
    QVERIFY(m_window->windowFlags().testFlag(Qt::Window));
}

void CaptureTest::bindsAShadowFromTheThemeTiles()
{
    // The named substitute for a picture (AK 7). `KWindowShadow::create()` is
    // false offscreen — there is no compositor to hand the tiles to — and
    // `grab()` would not show a shadow either, because it lies outside the
    // widget. What can be shown is that a shadow object exists and that its
    // tiles are the ones of the desktop theme, pixel for pixel.
    // The bundled theme always, an installed one on top wherever there is one:
    // the tiles have to have come from a real Plasma theme, not merely from an
    // SVG of ours.
    QStringList checked{NarrowBorderTheme};
    if (const auto installed = themes::anyInstalledTheme()) {
        checked << *installed;
    }

    for (const QString &theme : std::as_const(checked)) {
        m_window->reloadDesktopTheme(theme);
        m_window->showCapture();

        const KWindowShadow *shadow = m_window->shadow();
        QVERIFY2(shadow, qPrintable(theme));
        QVERIFY(shadow->topTile());
        QVERIFY(!shadow->topTile()->image().isNull());

        KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
        KSvg::FrameSvg tiles;
        tiles.setImageSet(&imageSet);
        tiles.setImagePath(QStringLiteral("dialogs/background"));
        tiles.setElementPrefix(QStringLiteral("shadow"));
        QVERIFY2(tiles.hasElementPrefix(QStringLiteral("shadow")), qPrintable(theme));

        // Each tile carries its **own** element, at its own size. The guard is
        // not decoration: `Svg::pixmap(element)` ignores the element and returns
        // the whole image, so a wrong implementation hands the compositor eight
        // copies of the entire shadow — which it accepts without complaint, and
        // which no picture of this project would show. That was the first
        // implementation here, and the assertion it passed compared against the
        // same wrong call.
        QVERIFY2(shadow->topTile()->image().size() != shadow->topLeftTile()->image().size(),
                 qPrintable(theme));

        for (const QString &element : {QStringLiteral("shadow-top"), QStringLiteral("shadow-topleft")}) {
            const QSize size = tiles.elementSize(element).toSize();
            QVERIFY(!size.isEmpty());

            const KWindowShadowTile::Ptr tile =
                element.endsWith(QStringLiteral("topleft")) ? shadow->topLeftTile() : shadow->topTile();
            QCOMPARE(tile->image().size(), size);
            QCOMPARE(tile->image(), tiles.image(size, element));
        }
    }
}

void CaptureTest::staysUsableWithoutADesktopTheme()
{
    // Outside a Plasma session `dialogs/background` is not there (AK 8), and
    // the demand is the modest one the criterion makes: no crash, and a window
    // one can still see and type into. Not transparent — which is what a
    // hull-less window with a translucent background would otherwise be.
    //
    // The state cannot be produced inside this process, and that is measured:
    // an unknown theme name does **not** leave KSvg empty-handed, it falls back
    // to `default` and the hull renders as usual. A test that set one would be
    // a test in which the fault cannot occur. What produces it is an
    // environment with no theme files on the data path — hence a process of its
    // own, which the branch below runs.
    if (qEnvironmentVariableIsSet("DENKZETTEL_TEST_WITHOUT_DESKTOP_THEME")) {
        QPlainTextEdit *text = textArea();
        QVERIFY(text);

        // Every road the standing window takes, not only the one it was built
        // on: reading plasmarc, and being handed a name that nothing answers to.
        m_window->reloadDesktopTheme();
        m_window->reloadDesktopTheme(NarrowBorderTheme);
        m_window->showCapture();

        const QImage picture = shot(*m_window);
        QVERIFY(picture.width() > 0);
        // No hull, so no corner is cut away — and no pixel is left transparent.
        QCOMPARE(cornerRun(picture), 0);
        QCOMPARE(picture.pixelColor(0, 0), m_window->palette().color(QPalette::Window));
        QCOMPARE(picture.pixelColor(picture.width() / 2, picture.height() / 2),
                 m_window->palette().color(QPalette::Window));

        text->setPlainText(QStringLiteral("geht trotzdem"));
        QCOMPARE(text->toPlainText(), QStringLiteral("geht trotzdem"));
        return;
    }

    const QTemporaryDir empty;
    QVERIFY(empty.isValid());

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("DENKZETTEL_TEST_WITHOUT_DESKTOP_THEME"), QStringLiteral("1"));
    environment.insert(QStringLiteral("XDG_DATA_DIRS"), empty.path());
    environment.insert(QStringLiteral("XDG_DATA_HOME"), empty.path());

    QProcess child;
    child.setProcessEnvironment(environment);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(QCoreApplication::applicationFilePath(),
                {QStringLiteral("staysUsableWithoutADesktopTheme")});

    QVERIFY(child.waitForFinished(60000));
    QVERIFY2(child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0,
             child.readAll().constData());
}

QTEST_MAIN(CaptureTest)

#include "capturetest.moc"
