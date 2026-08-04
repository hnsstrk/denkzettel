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
    void paintsOneSurfaceInThePaletteColours();
    void noteTextUsesTheWindowTextRole();
    void footerHasMoreAirThanTheApplicationName();
    void hullIsCompleteAtFiveAndEightLines();
    void wearsNoDecoration();
    void bindsAShadowFromTheThemeTiles();
    void staysUsableWithoutADesktopTheme();

private:
    QPlainTextEdit *textArea() const;
    static QImage shot(QWidget &window);
    /** How far into the top row of the picture the hull is still transparent. */
    static int cornerRun(const QImage &picture);
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

void CaptureTest::paintsOneSurfaceInThePaletteColours()
{
    // One continuous surface, not a box inside a box (wireframe 4b): the ground
    // behind the text area is the same ground as beside it. And it is the
    // palette's, not the theme's — of the eight installed desktop themes only
    // `default` adjusts its fill to the colour scheme.
    m_window->reloadDesktopTheme(NarrowBorderTheme);

    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    const QImage picture = shot(*m_window);
    const QColor surface = m_window->palette().color(QPalette::Window);

    // Inside the text area, in its lower right corner where no text stands.
    const QColor behindTheText =
        picture.pixelColor(text->x() + text->width() - 20, text->y() + text->height() - 6);
    // Beside it, in the gap between application name and text area.
    const QColor besideTheText = picture.pixelColor(m_window->width() / 2, text->y() - 4);

    QCOMPARE(behindTheText, surface);
    QCOMPARE(besideTheText, surface);
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

    for (const QString &theme : checked) {
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
            // Closed: opaque along the middle of every edge, and in the middle.
            QVERIFY(qAlpha(picture.pixel(picture.width() / 2, 0)) == 255);
            QVERIFY(qAlpha(picture.pixel(picture.width() / 2, picture.height() - 1)) == 255);
            QVERIFY(qAlpha(picture.pixel(0, picture.height() / 2)) == 255);
            QVERIFY(qAlpha(picture.pixel(picture.width() - 1, picture.height() / 2)) == 255);
            QVERIFY(qAlpha(picture.pixel(picture.width() / 2, picture.height() / 2)) == 255);
        }
    }
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

    for (const QString &theme : checked) {
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
