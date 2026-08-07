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
#include <QFile>
#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>
#include <QTextStream>

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
    void paintsTheThemesFieldOntoTheHull();
    void fieldColoursComeFromTheThemeBeforeTheScheme();
    void fieldFollowsADesktopThemeChange();
    void textSitsInsideTheFieldBorder();
    void fieldCoverageIsTheThemesOwn();
    void noteTextUsesTheWindowTextRole();
    void readsTheTextColoursOfTheDesktopTheme();
    void noteTextComesFromTheThemesOwnColours();
    void subtleTextsComeFromTheThemesOwnColours();
    void textColoursFollowADesktopThemeChange();
    void themeTextColoursOutlastAColourSchemeChange();
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
    /**
     * The text field the desktop theme itself draws at that size (issue #100).
     *
     * The same helper as themeHull() above, one graphic down: `widgets/lineedit`
     * with the prefix `base`. The selectors are handed in for the same reason —
     * offscreen nothing blurs, so the window draws the opaque variant, and a
     * reference rendered without it would compare a different picture.
     */
    static QPixmap themeField(const QString &theme, const QSize &size, const QStringList &selectors);
    /** The border `widgets/lineedit` claims for itself, or 0 if it does not resolve. */
    static qreal fieldBorderOf(const QString &theme);
    /**
     * Empty where the theme draws a text field, otherwise the sentence a QSKIP
     * gets — the one measured precondition every field assertion rests on.
     *
     * It exists because that precondition is **not** given everywhere, and the
     * public runner is the place it is not: `ksvg` does not depend on
     * `libplasma`, so a build host that installs only this project's KF6 parts
     * has no `widgets/lineedit` anywhere on the data path. The bundled test
     * themes do not bring one either — they ship `dialogs/background` and
     * nothing else — and the fallback KSvg would take needs an installed
     * `default` to fall back **to**.
     *
     * Hung on the measurement and never on the environment: what is asked is
     * whether the graphic resolves and claims a border, which is the same thing
     * `CaptureWindow::paintEvent()` asks before it draws. A condition on
     * `CI=true` or on a variable of ours would silence the assertion wherever
     * somebody set the variable, and a silenced assertion reads like a green
     * one.
     */
    static QString whyNoFieldGraphic(const QString &theme, const QString &criterion);
    /** Hull and field composed the way paintEvent() composes them. */
    static QImage themeHullWithField(const QString &theme, const QSize &size, const QRect &field);
    /** What the window's field draws at its surface, read off a fresh picture. */
    QColor fieldSurfaceColour();
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
 * What the wide theme's own `colors` file names (issue #85).
 *
 * Two colours no colour scheme carries, and that is what makes the assertion
 * one about the **origin**: a fixture holding 35,38,41 could not tell a theme
 * colour from a scheme colour that happens to look the same. The value is
 * written down twice on purpose — here and in the file — so that a change to
 * the fixture turns the test red instead of quietly moving what it proves.
 *
 * The narrow theme deliberately brings no such file; it is the other half.
 */
const QColor WideThemeTextColour(255, 0, 153);
const QColor WideThemeInactiveColour(0, 153, 255);

/**
 * The surface the wide theme's field graphic draws (issue #100).
 *
 * `BackgroundNormal` of that theme's `[Colors:View]` group, and the same kind
 * of value as the two above: one no colour scheme carries, so a comparison
 * against it is one about the **origin**. Since this story the group has two
 * jobs — the mutation probe of #85 and the colour of the field beside it — and
 * the comment head of the file says so.
 */
const QColor WideThemeFieldColour(51, 34, 17);

/**
 * The view background of the colour scheme the child process of
 * fieldColoursComeFromTheThemeBeforeTheScheme() writes for itself.
 *
 * Again a colour no shipped scheme carries: the assertion is that the field of
 * a theme **without** its own `colors` file lands on the scheme's value, and a
 * value that some scheme might hold anyway would not show that.
 */
const QColor SecondSchemeViewColour(13, 29, 47);

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

/** The middle of the text area: the field's own surface, clear of its border. */
QPoint fieldSurface(const QPlainTextEdit *text)
{
    return text->geometry().center();
}

/** The outermost pixel of the field's left border, where the graphic draws its edge. */
QPoint fieldEdge(const QPlainTextEdit *text)
{
    return QPoint(text->x(), text->y() + text->height() / 2);
}

/**
 * A point of the hull the field does not cover: the gap above the text area.
 *
 * The middle of the window used to serve every assertion that wanted the hull,
 * and it cannot any more — since issue #100 it lies inside the text field
 * (measured at both window sizes and under three themes, AK 9). The dangerous
 * half is that it would not have turned red: the field covers 255 where the
 * hull covers 255, so the assertions would have kept passing and measured the
 * field from then on.
 */
QPoint besideTheField(const QPlainTextEdit *text, int windowWidth)
{
    return QPoint(windowWidth / 2, text->y() - 4);
}

/** A logical point of the window in the pixels of a grabbed picture. */
QPoint inPicture(const QPoint &logical)
{
    return QPoint(qRound(logical.x() * qApp->devicePixelRatio()),
                  qRound(logical.y() * qApp->devicePixelRatio()));
}
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

    // The run that is about a **second** colour scheme brings its own, and it
    // has to be in place before the first graphic is recoloured: KSvg reads the
    // scheme when it resolves an image set, and it keeps what it read. Only the
    // child process of fieldColoursComeFromTheThemeBeforeTheScheme() sets this,
    // and only that child has a home directory of its own to write it into.
    if (qEnvironmentVariableIsSet("DENKZETTEL_TEST_COLOUR_SCHEME")) {
        const QString target = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
            + QStringLiteral("/kdeglobals");
        QFile::remove(target);
        QVERIFY(QFile::copy(qEnvironmentVariable("DENKZETTEL_TEST_COLOUR_SCHEME"), target));
    }

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
    //
    // Under a theme that brings no `colors` file, which is what the narrow one
    // is here: since #85 a theme with one keeps its own colour through a scheme
    // change, and that half is themeTextColoursOutlastAColourSchemeChange().
    m_window->reloadDesktopTheme(NarrowBorderTheme);

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

    for (const QLabel *label : labels) {
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

QPixmap CaptureTest::themeField(const QString &theme, const QSize &size, const QStringList &selectors)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));
    imageSet.setSelectors(selectors);

    KSvg::FrameSvg frame;
    // No rendering cache, for the same reason themeHull() switches it off: the
    // helper is called several times per run with different themes.
    frame.setUsingRenderingCache(false);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("widgets/lineedit"));
    frame.setElementPrefix(QStringLiteral("base"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.setDevicePixelRatio(qApp->devicePixelRatio());
    frame.resizeFrame(size);

    return frame.framePixmap();
}

qreal CaptureTest::fieldBorderOf(const QString &theme)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("widgets/lineedit"));
    frame.setElementPrefix(QStringLiteral("base"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(560, 90));

    return frame.isValid() ? frame.marginSize(KSvg::FrameSvg::LeftMargin) : 0;
}

QString CaptureTest::whyNoFieldGraphic(const QString &theme, const QString &criterion)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::FrameSvg frame;
    frame.setUsingRenderingCache(false);
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("widgets/lineedit"));
    frame.setElementPrefix(QStringLiteral("base"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(560, 90));

    const qreal border = frame.isValid() ? frame.marginSize(KSvg::FrameSvg::LeftMargin) : 0;
    if (frame.isValid() && frame.hasElementPrefix(QStringLiteral("base")) && border > 0) {
        return {};
    }

    // Named in full, both halves: **which** precondition is missing, and
    // **which** criterion goes unmeasured because of it. A silent skip is the
    // same thing as a silent NOLINT.
    return QStringLiteral(
               "Das Desktop-Theme `%1` löst `widgets/lineedit` nicht auf "
               "(isValid=%2, Vorsatz base=%3, Rand %4) — ohne Plasma-Grafiken auf dem "
               "Datenpfad zeichnet der Bau kein Feld, und %5 ist hier nicht zu messen. "
               "Dass das Fenster ohne Feld brauchbar bleibt, prüft "
               "staysUsableWithoutADesktopTheme(); ungeprüft bleibt allein das Feld selbst.")
        .arg(theme,
             frame.isValid() ? QStringLiteral("ja") : QStringLiteral("nein"),
             frame.hasElementPrefix(QStringLiteral("base")) ? QStringLiteral("ja")
                                                            : QStringLiteral("nein"))
        .arg(border)
        .arg(criterion);
}

QImage CaptureTest::themeHullWithField(const QString &theme, const QSize &size, const QRect &field)
{
    // The two calls of paintEvent(), in its order and with its coordinates: the
    // hull over the whole window, the field over the geometry of the text area.
    // Held against this rather than against a colour, because a colour would be
    // the colour of one theme and this runs under several.
    QImage picture = themeHull(theme, size, opaqueSelectors());

    QPainter painter(&picture);
    painter.drawPixmap(field.topLeft(), themeField(theme, field.size(), opaqueSelectors()));
    painter.end();

    return picture;
}

QColor CaptureTest::fieldSurfaceColour()
{
    const QPlainTextEdit *text = textArea();
    Q_ASSERT(text);
    return shot(*m_window).pixelColor(inPicture(fieldSurface(text)));
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
    // The hull is **one** graphic over the whole window, and it is the theme's,
    // which is what issue #83 turned around. Until then the window filled the
    // theme's alpha mask with a colour of the palette; now it draws the theme's
    // own graphic and adds nothing. The assertion is therefore held against a
    // second rendering of the same graphic and not against a colour — under
    // this bundled theme the two differ visibly, so a window that went back to
    // filling would be caught.
    //
    // What this no longer says is "the ground behind the text is the ground
    // beside it": since issue #100 the text area carries a second graphic of
    // the same theme on top, and that one is
    // paintsTheThemesFieldOntoTheHull(). The two points below therefore lie
    // above and below the text area — where the hull, and only the hull,
    // stands. Above and below, and not one of them: a hull drawn in pieces
    // would pass a single-point check.
    m_window->reloadDesktopTheme(NarrowBorderTheme);

    const QPlainTextEdit *text = textArea();
    QVERIFY(text);

    const QImage picture = shot(*m_window);
    const QImage hull = themeHull(NarrowBorderTheme, m_window->size(), opaqueSelectors());
    QCOMPARE(hull.size(), picture.size());

    // In the gap between application name and text area, and in the wider gap
    // between text area and footer.
    const QPoint above = inPicture(besideTheField(text, m_window->width()));
    const QPoint below = inPicture(QPoint(m_window->width() / 2, text->y() + text->height() + 4));

    QCOMPARE(picture.pixelColor(above), picture.pixelColor(below));
    QCOMPARE(picture.pixelColor(above), hull.pixelColor(above));
    QCOMPARE(picture.pixelColor(below), hull.pixelColor(below));

    // And it is not the palette's colour any more. Without this line the three
    // comparisons above would still hold if the graphic happened to carry the
    // scheme colour — which under `default` it does, and under this theme it
    // does not (measured: the bundled graphic draws black, the scheme does not).
    QVERIFY2(picture.pixelColor(above) != m_window->palette().color(QPalette::Window),
             qPrintable(picture.pixelColor(above).name(QColor::HexArgb)));
}

void CaptureTest::paintsTheThemesFieldOntoTheHull()
{
    // AK 1 and AK 6a: surface and edge of the text field come out of
    // `widgets/lineedit`, prefix `base`, drawn from the same image set as the
    // hull — the graphic KRunner's own entry field is drawn from
    // (`TextField.qml:187–191`), which is the customer's yardstick.
    //
    // Held against a second rendering of both graphics, composed the way
    // paintEvent() composes them. Under two desktop themes, because the graphic
    // comes from the theme and not from the palette: under the wide bundled one
    // the field carries that theme's own colour, under the narrow one the
    // colour scheme's.
    for (const QString &theme : {NarrowBorderTheme, WideBorderTheme}) {
        const QString missing = whyNoFieldGraphic(theme, QStringLiteral("AK 1 (Herkunft)"));
        if (!missing.isEmpty()) {
            QSKIP(qPrintable(missing));
        }

        m_window->reloadDesktopTheme(theme);

        const QPlainTextEdit *text = textArea();
        QVERIFY(text);

        const QImage picture = shot(*m_window);
        const QImage expected = themeHullWithField(theme, m_window->size(), text->geometry());
        // The width has to agree, the height to within one row — and that one
        // row is measured, not tolerated blindly: at the customer's ratio the
        // window's 202 logical pixels are 323,2 device pixels, and `grab()`
        // cuts the fraction where `resizeFrame()` rounds it up. It is the
        // **last** row that differs; the three points below lie far above it,
        // and the field itself is rendered from the same logical rectangle in
        // both pictures, so it lands on the same device pixels.
        QCOMPARE(expected.width(), picture.width());
        QVERIFY2(qAbs(expected.height() - picture.height()) <= 1,
                 qPrintable(QStringLiteral("%1: erwartet %2 hoch, gezeichnet %3")
                                .arg(theme)
                                .arg(expected.height())
                                .arg(picture.height())));

        const QPoint surface = inPicture(fieldSurface(text));
        const QPoint edge = inPicture(fieldEdge(text));
        const QPoint hull = inPicture(besideTheField(text, m_window->width()));

        for (const QPoint &point : {surface, edge, hull}) {
            QVERIFY2(picture.pixelColor(point) == expected.pixelColor(point),
                     qPrintable(QStringLiteral("%1 bei %2,%3: gezeichnet %4, erwartet %5")
                                    .arg(theme)
                                    .arg(point.x())
                                    .arg(point.y())
                                    .arg(picture.pixelColor(point).name(QColor::HexArgb),
                                         expected.pixelColor(point).name(QColor::HexArgb))));
        }

        // The field's whole border ring, pixel for pixel, and not only the
        // three points. It is what catches a field drawn at the wrong pixel
        // ratio: a frame left at 1 in a session that scales to 1,6 covers the
        // same logical area and carries the same colours in its flat middle —
        // three points would all agree, and only the smeared edge of the
        // upscaled graphic gives it away (issue #100, F4; mutation probe 7).
        //
        // The ring and not the whole field, because the window writes into the
        // middle and the reference does not: the placeholder of the empty text
        // area stands there, and measured it is 616 pixels of legitimate
        // difference. It begins a document margin in, which is wider than the
        // ring by the margin the field itself does not claim.
        const QRect field(inPicture(text->geometry().topLeft()),
                          inPicture(text->geometry().bottomRight()));
        const int ring = qRound(fieldBorderOf(theme) * qApp->devicePixelRatio());
        QVERIFY(ring > 0);
        int differing = 0;
        int examined = 0;
        for (int y = field.top(); y <= field.bottom(); ++y) {
            for (int x = field.left(); x <= field.right(); ++x) {
                if (x - field.left() >= ring && field.right() - x >= ring
                    && y - field.top() >= ring && field.bottom() - y >= ring) {
                    continue;
                }
                ++examined;
                if (picture.pixel(x, y) != expected.pixel(x, y)) {
                    ++differing;
                }
            }
        }
        QVERIFY2(differing == 0,
                 qPrintable(QStringLiteral("%1: %2 von %3 Bildpunkten des Feldrandes weichen ab")
                                .arg(theme)
                                .arg(differing)
                                .arg(examined)));

        // Three counter-checks in the same run, because the comparisons above
        // would all hold for a window that drew no field at all — the reference
        // would then be wrong in the same way as the picture.
        //
        // There is a field: its surface is not the hull beside it.
        QVERIFY2(picture.pixelColor(surface) != picture.pixelColor(hull),
                 qPrintable(QStringLiteral("%1: Feld und Hülle bildpunktgleich (%2)")
                                .arg(theme, picture.pixelColor(surface).name(QColor::HexArgb))));
        // It has an edge: the graphic draws its border in another colour than
        // its surface, which is what makes the field readable as a field.
        QVERIFY2(picture.pixelColor(edge) != picture.pixelColor(surface),
                 qPrintable(QStringLiteral("%1: Kante und Fläche bildpunktgleich (%2)")
                                .arg(theme, picture.pixelColor(edge).name(QColor::HexArgb))));
    }

    // And the colour is the **theme's**, not one mixed by us or taken from the
    // palette. The wide bundled theme names a view background no colour scheme
    // carries; a field filled from `QPalette::Base` would land somewhere else.
    // Under the narrow theme this could not be told apart — there the graphic
    // takes the scheme's own view colour, and that is white in this run just
    // like `QPalette::Base` is.
    m_window->reloadDesktopTheme(WideBorderTheme);
    QCOMPARE(fieldSurfaceColour(), WideThemeFieldColour);
}

void CaptureTest::fieldColoursComeFromTheThemeBeforeTheScheme()
{
    // The second colour scheme of the belegform, and it earns its process: the
    // field graphic is recoloured out of `kdeglobals` and not out of
    // `qApp->palette()` (measured — a palette set on the application moves the
    // texts and leaves the graphic where it is). A scheme change therefore
    // cannot be staged inside a running test the way
    // textsFollowAColourSchemeChange() stages one.
    //
    // What the child measures is the order of precedence, and it is the same
    // one as for the writing since #85: a theme that brings its own `colors`
    // file decides, and a theme that brings none leaves it to the scheme.
    // Asked in **both** halves, and in the parent before the child is started
    // at all: an exit code says that the child failed and never why. A parent
    // that let the child run into the missing graphic would report a number and
    // hide the reason behind it.
    for (const QString &theme : {NarrowBorderTheme, WideBorderTheme}) {
        const QString missing =
            whyNoFieldGraphic(theme, QStringLiteral("die Farbschema-Hälfte von AK 1"));
        if (!missing.isEmpty()) {
            QSKIP(qPrintable(missing));
        }
    }

    if (qEnvironmentVariableIsSet("DENKZETTEL_TEST_COLOUR_SCHEME")) {
        // No own `colors` file: the scheme the child wrote for itself shows.
        m_window->reloadDesktopTheme(NarrowBorderTheme);
        QCOMPARE(fieldSurfaceColour(), SecondSchemeViewColour);

        // Own `colors` file: the theme keeps its say, whatever the scheme says.
        m_window->reloadDesktopTheme(WideBorderTheme);
        QCOMPARE(fieldSurfaceColour(), WideThemeFieldColour);
        return;
    }

    const QTemporaryDir home;
    QVERIFY(home.isValid());

    const QString scheme = home.filePath(QStringLiteral("zweites-schema"));
    {
        QFile file(scheme);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream(&file) << "[Colors:View]\nBackgroundNormal="
                           << SecondSchemeViewColour.red() << ','
                           << SecondSchemeViewColour.green() << ','
                           << SecondSchemeViewColour.blue() << "\n";
    }

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("DENKZETTEL_TEST_COLOUR_SCHEME"), scheme);
    // A home of the child's own, and that is not tidiness: test mode puts the
    // config directory under `$HOME/.qttest`, so a scheme written there would
    // outlive the run and colour every later one — a leftover file nobody wrote
    // on purpose. With its own home it goes when the directory goes.
    environment.insert(QStringLiteral("HOME"), home.path());

    QProcess child;
    child.setProcessEnvironment(environment);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(QCoreApplication::applicationFilePath(),
                {QStringLiteral("fieldColoursComeFromTheThemeBeforeTheScheme")});

    QVERIFY(child.waitForFinished(60000));
    QVERIFY2(child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0,
             child.readAll().constData());
}

void CaptureTest::fieldFollowsADesktopThemeChange()
{
    // AK 4, and it hangs on the measured trap of #83: a FrameSvg follows only a
    // **fresh** image set. Left out of the loop in reloadDesktopTheme() the
    // field would keep drawing the old theme's graphic on a standing window,
    // and no return value would say so — the daemon builds this window once and
    // keeps it (SPEC 2.1), so a theme change has to reach a standing one.
    //
    // There and back, because a colour that moves once would also be explained
    // by one that was set once and never cleared.
    for (const QString &theme : {NarrowBorderTheme, WideBorderTheme}) {
        const QString missing = whyNoFieldGraphic(theme, QStringLiteral("AK 4 (Theme-Wechsel)"));
        if (!missing.isEmpty()) {
            QSKIP(qPrintable(missing));
        }
    }

    m_window->reloadDesktopTheme(NarrowBorderTheme);
    const QColor narrow = fieldSurfaceColour();

    m_window->reloadDesktopTheme(WideBorderTheme);
    const QColor wide = fieldSurfaceColour();

    m_window->reloadDesktopTheme(NarrowBorderTheme);
    const QColor back = fieldSurfaceColour();

    QVERIFY2(narrow != wide,
             qPrintable(QStringLiteral("beide %1").arg(narrow.name(QColor::HexArgb))));
    QCOMPARE(back, narrow);
    // And the colour that arrived is the one the new theme names, not merely
    // another one: without this line a field that fell back to the palette
    // halfway would pass the two comparisons above.
    QCOMPARE(wide, WideThemeFieldColour);
}

void CaptureTest::textSitsInsideTheFieldBorder()
{
    // AK 5: the inner spacings of 4b count on top of the strip
    // `widgets/lineedit` claims for itself, exactly as they already count on
    // top of the hull's. The text moves inwards — that is wanted — while the
    // application name and the footer stay where they are.
    //
    // Read off the graphic and never against 6: `marginSize()` hands out
    // 5,99999 under four of the eight installed themes, and a comparison
    // against a whole number falls over that (issue #100, F9; the same
    // observation stands for the hull's margin at the head of this file).
    for (const QString &theme : {NarrowBorderTheme, WideBorderTheme}) {
        const QString missing = whyNoFieldGraphic(theme, QStringLiteral("AK 5 (Innenabstände)"));
        if (!missing.isEmpty()) {
            QSKIP(qPrintable(missing));
        }

        const qreal border = fieldBorderOf(theme);
        QVERIFY2(border > 0, qPrintable(theme));

        m_window->reloadDesktopTheme(theme);
        m_window->show();
        QCoreApplication::processEvents();

        QPlainTextEdit *text = textArea();
        QVERIFY(text);

        // The text keeps at least the field's own border away from the graphic
        // that draws the field. Without applyFieldMargin() this is 4 against 6.
        QVERIFY2(text->document()->documentMargin() >= border,
                 qPrintable(QStringLiteral("%1: documentMargin %2 < Feldrand %3")
                                .arg(theme)
                                .arg(text->document()->documentMargin())
                                .arg(border)));
        QVERIFY2(text->cursorRect().left() >= qRound(border),
                 qPrintable(QStringLiteral("%1: Cursor bei %2, Feldrand %3")
                                .arg(theme)
                                .arg(text->cursorRect().left())
                                .arg(border)));

        // And the five lines survived the inset: the widget grew by the two
        // borders instead of losing room for text. At both sizes SPEC 3 knows,
        // as DoD 1 asks.
        const int chrome = 2 * qRound(text->document()->documentMargin()) + 2 * text->frameWidth();
        QCOMPARE(text->height() - chrome, capture::MinTextLines * text->fontMetrics().lineSpacing());

        text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
        QCoreApplication::processEvents();
        QCOMPARE(text->height() - chrome, capture::MaxTextLines * text->fontMetrics().lineSpacing());
        text->clear();
        QCoreApplication::processEvents();

        // The other half of the criterion, and it is the half that would go
        // unnoticed: the field's border reaches the text and nothing else. The
        // labels stand where the layout puts them, which is the hull's border
        // plus the inner spacing of 4b and not a pixel more.
        const QList<QLabel *> labels = m_window->findChildren<QLabel *>();
        QCOMPARE(labels.size(), 2);
        for (const QLabel *label : labels) {
            QCOMPARE(label->x(), m_window->layout()->contentsMargins().left());
        }
    }
}

void CaptureTest::fieldCoverageIsTheThemesOwn()
{
    // AK 6b — the limit of this story, and it is measured at the **coverage**
    // of the graphic rather than at a contrast number. A contrast number holds
    // for one colour scheme, one selector and one named ground, and the ground
    // here is the customer's wallpaper: the same graphic measures 1,08:1 over
    // black and 1,88:1 over white, because the hull above it lets the ground
    // through (F11).
    //
    // The themes are found by measurement and never named, and here that is not
    // only the usual reason: whether a theme brings a `lineedit` graphic of its
    // own is **not observable at all**. KSvg falls back to `default` per image,
    // so every name resolves — an invented one included (F1). What is
    // observable is what the graphic covers.
    const QSize size(560, 90);
    const QPoint centre(size.width() / 2, size.height() / 2);

    QString covering;
    QString faint;
    int faintAlpha = 0;
    const QStringList installed = themes::installedThemes();
    for (const QString &theme : installed) {
        const int alpha = qAlpha(themeField(theme, size, opaqueSelectors()).toImage().pixel(centre));
        if (alpha == 255 && covering.isEmpty()) {
            covering = theme;
        }
        if (alpha <= 15 && faint.isEmpty()) {
            faint = theme;
            faintAlpha = alpha;
        }
    }

    // The order of these two matters, and it was wrong once: the assertion used
    // to stand **before** the skip, so on a host without Plasma graphics this
    // run stated the missing precondition in its own failure text and failed
    // anyway (öffentlicher Lauf 31216657864). A precondition is asked first or
    // it is not a precondition.
    if (covering.isEmpty() && faint.isEmpty()) {
        QSKIP(qPrintable(
            QStringLiteral("Keines der %1 gefundenen installierten Desktop-Themes zeichnet "
                           "`widgets/lineedit` — ohne Plasma-Grafiken auf dem Datenpfad sind "
                           "die beiden Deckungsklassen aus AK 6b nicht zu messen, weder die "
                           "deckende noch die schwache.")
                .arg(installed.size())));
    }

    // Beyond that a machine that has Plasma themes at all has a covering one:
    // `default` and the two Breeze themes cover 255, and one of the three is on
    // every such machine. Left as an assertion on purpose — where the graphics
    // are there, this is a statement about them and not about the host.
    QVERIFY2(!covering.isEmpty(),
             qPrintable(QStringLiteral("Themes gefunden (%1), aber keines zeichnet die "
                                       "Feldgrafik deckend — dann ist die Gegenklasse aus "
                                       "AK 6b nicht zu messen.")
                            .arg(installed.join(QStringLiteral(", ")))));

    if (faint.isEmpty()) {
        // Spoken out rather than left to a green run: the five themes that draw
        // a hint only are CachyOS packages. On the public runner there are
        // `default` and `breeze-*` and nothing else, and every graphic there
        // covers 255 — the limit is real, it is just not measurable here.
        QSKIP("Kein installiertes Theme zeichnet die Feldgrafik als Hauch — die Grenze aus "
              "AK 6b (Deckung 15 gegen 255) ist auf diesem Läufer nicht zu messen.");
    }

    QVERIFY2(faintAlpha <= 15,
             qPrintable(QStringLiteral("%1 deckt %2").arg(faint).arg(faintAlpha)));

    // And the window draws what the graphic gives — in **both** classes. That
    // is the whole of the assurance: where the theme draws a hint, the window
    // shows a hint, and there is nothing here to repair without giving up the
    // decision "form and colour come from the theme" (#83).
    for (const QString &theme : {covering, faint}) {
        m_window->reloadDesktopTheme(theme);

        const QPlainTextEdit *text = textArea();
        QVERIFY(text);

        const QImage picture = shot(*m_window);
        const QImage expected = themeHullWithField(theme, m_window->size(), text->geometry());
        QCOMPARE(expected.size(), picture.size());

        const QPoint surface = inPicture(fieldSurface(text));
        QVERIFY2(picture.pixelColor(surface) == expected.pixelColor(surface),
                 qPrintable(QStringLiteral("%1: gezeichnet %2, erwartet %3")
                                .arg(theme,
                                     picture.pixelColor(surface).name(QColor::HexArgb),
                                     expected.pixelColor(surface).name(QColor::HexArgb))));
    }
}

void CaptureTest::noteTextUsesTheWindowTextRole()
{
    // The note text carries `WindowText` and not the role for entry fields.
    // Copied on every palette change, so it follows the scheme instead of
    // freezing (#54).
    //
    // Since issue #100 the reason is another one: the text no longer stands on
    // one ground but on two, depending on the theme — on the field's surface
    // where `widgets/lineedit` covers, on the hull where it draws a hint only.
    // `WindowText` stays above 4,5:1 in both, worst case 5,70:1 and 4,74:1 over
    // 19 colour schemes, while the view role falls to 4,22:1 in the second. The
    // pair 4,74:1 / 4,22:1 that used to stand here was the number of a ground
    // that is now one of two (UX decision of 07.08.2026,
    // `docs/scrum/reviews/2026-08-07-textfarbe/entscheidung.md`).
    const QPlainTextEdit *text = textArea();
    QVERIFY(text);

    // Since #85 the scheme is only half the rule, so the theme is named here
    // rather than inherited: the narrow one brings no `colors` file, which is
    // the case this assertion is about. Without the line the test would run
    // under whatever `plasmarc` the run before it left behind — and the run
    // before it writes the **wide** theme, which since #85 has a file.
    m_window->reloadDesktopTheme(NarrowBorderTheme);

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

void CaptureTest::readsTheTextColoursOfTheDesktopTheme()
{
    // The gate of the whole story (issue #85, AK 1): does this theme bring a
    // hand of its own? Read out of the theme's `colors` file with KConfig, the
    // same road contrastEffectOf() takes to the file beside it.
    const capture::ThemeTextColours wide = capture::themeTextColoursOf(WideBorderTheme);
    QCOMPARE(wide.normal, WideThemeTextColour);
    QCOMPARE(wide.inactive, WideThemeInactiveColour);

    // A theme without the file hands out nothing, and neither does a name
    // nothing answers to. Both matter: the second is the road a machine with no
    // desktop themes at all takes, and it must not end in a colour.
    QVERIFY(!capture::themeTextColoursOf(NarrowBorderTheme).normal.isValid());
    QVERIFY(!capture::themeTextColoursOf(NarrowBorderTheme).inactive.isValid());
    QVERIFY(!capture::themeTextColoursOf(QStringLiteral("kein-solches-theme")).normal.isValid());
}

void CaptureTest::noteTextComesFromTheThemesOwnColours()
{
    const QPlainTextEdit *text = textArea();
    QVERIFY(text);

    // AK 1 for the note text: the writing comes from the same hand as the
    // surface. Asserted on the **origin** and not on a contrast number, and
    // that is measured rather than modest: in test mode QStandardPaths no
    // longer finds the customer's `kdeglobals`, so KSvg reckons with Qt's
    // fallback palette while the application palette still carries the KDE
    // platform theme's colours. A contrast asserted here would be a number for
    // a state no machine is ever in (1,11:1, measured — pre-check F3).
    m_window->reloadDesktopTheme(WideBorderTheme);
    QCOMPARE(text->palette().color(QPalette::Text), WideThemeTextColour);

    // And it is not the scheme's colour. Without this line the comparison above
    // would still hold if the fixture happened to name the scheme's own value —
    // the very confusion the fixture's unusual colours are chosen against.
    QVERIFY2(text->palette().color(QPalette::Text)
                 != m_window->palette().color(QPalette::WindowText),
             qPrintable(text->palette().color(QPalette::Text).name()));

    // The two roads have to agree, or they drift apart unnoticed: the note text
    // is painted with `KSvg::Svg::color(Text)`, the gate is read out of the file
    // with KConfig. Where a theme brings the file, both must name the same
    // colour — measured over eight themes and three colour schemes, and held
    // here so that a KSvg that changed its mind would turn something red.
    QCOMPARE(text->palette().color(QPalette::Text),
             capture::themeTextColoursOf(WideBorderTheme).normal);

    // The other half of the customer decision: a theme without a `colors` file
    // leaves the note text to the colour scheme, exactly as before #85.
    m_window->reloadDesktopTheme(NarrowBorderTheme);
    QCOMPARE(text->palette().color(QPalette::Text),
             m_window->palette().color(QPalette::WindowText));
}

void CaptureTest::subtleTextsComeFromTheThemesOwnColours()
{
    // The same rule for the dimmed class — application name, key hint and the
    // placeholder of the empty text area (SPEC 3.1). It gets there by another
    // road, and not by choice: `KSvg::Svg::StyleSheetColor` has no counterpart
    // to `ForegroundInactive`, so this class is read out of the theme's file.
    //
    // Expressly **not** asserted: that this makes the class readable. Under
    // `breeze-light` neither source reaches 4,5:1 — 3,70:1 against 2,09:1,
    // measured. That is issue #84, and AK 4 asks for the limit to be named.
    const QPlainTextEdit *text = textArea();
    QVERIFY(text);

    m_window->reloadDesktopTheme(WideBorderTheme);

    const QList<QLabel *> labels = m_window->findChildren<QLabel *>();
    QCOMPARE(labels.size(), 2);
    for (const QLabel *label : labels) {
        // What the label paints with: its own palette, read through its role.
        QCOMPARE(label->palette().color(label->foregroundRole()), WideThemeInactiveColour);
    }
    QCOMPARE(text->palette().color(QPalette::PlaceholderText), WideThemeInactiveColour);

    QVERIFY2(WideThemeInactiveColour != m_window->palette().color(QPalette::PlaceholderText),
             qPrintable(WideThemeInactiveColour.name()));

    // And back to the scheme under a theme that brings no file of its own.
    m_window->reloadDesktopTheme(NarrowBorderTheme);
    for (const QLabel *label : labels) {
        QCOMPARE(label->palette().color(label->foregroundRole()),
                 m_window->palette().color(QPalette::PlaceholderText));
    }
    QCOMPARE(text->palette().color(QPalette::PlaceholderText),
             m_window->palette().color(QPalette::PlaceholderText));
}

void CaptureTest::textColoursFollowADesktopThemeChange()
{
    // AK 5, on **one** window: the daemon builds it at start and keeps it
    // (SPEC 2.1), so a theme change reaches a window that is already standing.
    // There and back again, because a colour that only ever moved one way
    // would also be explained by a colour that was set once and never cleared.
    const QPlainTextEdit *text = textArea();
    QVERIFY(text);
    const QList<QLabel *> labels = m_window->findChildren<QLabel *>();
    QCOMPARE(labels.size(), 2);

    m_window->reloadDesktopTheme(NarrowBorderTheme);
    const QColor schemeNote = text->palette().color(QPalette::Text);
    const QColor schemeSubtle = labels.first()->palette().color(labels.first()->foregroundRole());

    m_window->reloadDesktopTheme(WideBorderTheme);
    QCOMPARE(text->palette().color(QPalette::Text), WideThemeTextColour);
    QCOMPARE(labels.first()->palette().color(labels.first()->foregroundRole()),
             WideThemeInactiveColour);

    m_window->reloadDesktopTheme(NarrowBorderTheme);
    QCOMPARE(text->palette().color(QPalette::Text), schemeNote);
    QCOMPARE(labels.first()->palette().color(labels.first()->foregroundRole()), schemeSubtle);
}

void CaptureTest::themeTextColoursOutlastAColourSchemeChange()
{
    // AK 7, and it is the criterion the story was not ready without: the whole
    // mechanism hangs on the palette change, so an implementation that only
    // met AK 1 in reloadDesktopTheme() would be right at the acceptance and
    // wrong after the first scheme change — without a word from anybody.
    const QPlainTextEdit *text = textArea();
    QVERIFY(text);
    const QList<QLabel *> labels = m_window->findChildren<QLabel *>();
    QCOMPARE(labels.size(), 2);

    m_window->reloadDesktopTheme(WideBorderTheme);

    const QPalette startPalette = qApp->palette();
    QPalette switched = startPalette;
    switched.setColor(QPalette::WindowText, QColor(0x23, 0x26, 0x29));
    switched.setColor(QPalette::PlaceholderText, QColor(0x70, 0x7d, 0x8a));
    qApp->setPalette(switched);
    // Qt hands the new palette to the widgets through a posted event; without a
    // running event loop the test has to let it through itself.
    QCoreApplication::processEvents();

    QCOMPARE(text->palette().color(QPalette::Text), WideThemeTextColour);
    QCOMPARE(labels.first()->palette().color(labels.first()->foregroundRole()),
             WideThemeInactiveColour);
    QCOMPARE(text->palette().color(QPalette::PlaceholderText), WideThemeInactiveColour);

    // The counter-case in the same run, or the assertion above would also hold
    // for a window that ignores palette changes altogether: without a `colors`
    // file the texts follow the scheme, and they follow it to the values this
    // test has just set.
    m_window->reloadDesktopTheme(NarrowBorderTheme);
    QCOMPARE(text->palette().color(QPalette::Text), QColor(0x23, 0x26, 0x29));
    QCOMPARE(labels.first()->palette().color(labels.first()->foregroundRole()),
             QColor(0x70, 0x7d, 0x8a));

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
    const QLabel *appName = labels.at(0)->y() < labels.at(1)->y() ? labels.at(0) : labels.at(1);
    const QLabel *footer = appName == labels.at(0) ? labels.at(1) : labels.at(0);

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

            // The fifth point used to be the middle of the window and had to
            // move (issue #100, AK 9): since the text field draws a graphic of
            // its own the middle lies **inside the field**, and this run would
            // have gone on measuring the field while reading like a statement
            // about the hull. It would not have turned red about it — under
            // every theme any run of this project reaches, field and hull both
            // cover 255. The point now sits in the gap above the text area,
            // which is hull and nothing else.
            for (const QPoint &point : {QPoint(picture.width() / 2, 0),
                                        QPoint(picture.width() / 2, picture.height() - 1),
                                        QPoint(0, picture.height() / 2),
                                        QPoint(picture.width() - 1, picture.height() / 2),
                                        inPicture(besideTheField(text, window.width()))}) {
                QVERIFY2(qAlpha(picture.pixel(point)) > 0,
                         qPrintable(QStringLiteral("%1 bei %2,%3")
                                        .arg(theme)
                                        .arg(point.x())
                                        .arg(point.y())));
                QCOMPARE(qAlpha(picture.pixel(point)), qAlpha(hull.pixel(point)));
            }

            // The fifth point once more, and by **colour** this time. Coverage
            // alone cannot tell the hull from the field: measured, both cover
            // 255 under every theme a run of this project reaches, so a field
            // spread over the whole window would pass the loop above without a
            // murmur (mutation probe 6). The colour tells them apart, and this
            // is the line that makes "measures the hull" a measurable claim
            // rather than a claim about where a point sits.
            const QPoint clearOfTheField = inPicture(besideTheField(text, window.width()));
            QVERIFY2(picture.pixelColor(clearOfTheField) == hull.pixelColor(clearOfTheField),
                     qPrintable(QStringLiteral("%1: gezeichnet %2, Hülle %3")
                                    .arg(theme,
                                         picture.pixelColor(clearOfTheField).name(QColor::HexArgb),
                                         hull.pixelColor(clearOfTheField).name(QColor::HexArgb))));
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
                 QStringLiteral("hullHasNoStairAtTheCorner"),
                 // The field goes along, and it is the only place a missing
                 // ratio on it can show: a FrameSvg stands at 1 whatever the
                 // session scales to, and offscreen the event that would betray
                 // the omission never arrives (issue #100, F4). At ratio 1 the
                 // picture and the reference are wrong in the same way.
                 //
                 // Where the theme draws no field it carries its own skip, and
                 // the child then exits 0 with one test fewer. That leaves no
                 // hole to overlook: the same assertion runs in **this**
                 // process too, so the missing precondition is announced at the
                 // top level of the same run and not only inside a child whose
                 // output nobody reads when it succeeds.
                 QStringLiteral("paintsTheThemesFieldOntoTheHull")});

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
    // Shown before the geometry is read, and that is not a formality: a window
    // that was never shown has no laid-out layout at all — every child sits at
    // zero, and the sample point would land four pixels **above** the picture.
    // Measured on this very run: the search then found no theme at all and the
    // assertion skipped itself with a plausible reason.
    m_window->show();
    QCoreApplication::processEvents();

    const QSize size = m_window->size();
    // Out of the field and into the gap above it (issue #100, AK 9). The middle
    // of the window stood here until then, and it stopped measuring the hull
    // the moment the field arrived — **without turning red**: measured, the
    // reading there is 255 with the selector and 255 without it, so the very
    // mutation this assertion exists for would have walked through it. The
    // reading below is taken at the same point on both sides.
    const QPlainTextEdit *narrowText = textArea();
    QVERIFY(narrowText);
    const QPoint sample = inPicture(besideTheField(narrowText, m_window->width()));

    QString candidate;
    int translucentAlpha = 0;
    int opaqueAlpha = 0;
    const QStringList installed = themes::installedThemes();
    for (const QString &theme : installed) {
        const int loose = qAlpha(themeHull(theme, size, {}).pixel(sample));
        const int tight = qAlpha(themeHull(theme, size, opaqueSelectors()).pixel(sample));
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

    // The reading is taken where the two variants were held against each other,
    // and that point has to have stayed out of the field: a wider theme border
    // pushes the text area down, never up, so it did — and this says so rather
    // than trusting it. Every installed theme carries at least the 4 px of the
    // bundled narrow one.
    const QPlainTextEdit *text = textArea();
    QVERIFY(text);
    QVERIFY2(sample.y() < inPicture(QPoint(0, text->y())).y(),
             qPrintable(QStringLiteral("%1: Abgriff %2 läge im Feld ab %3")
                            .arg(candidate)
                            .arg(sample.y())
                            .arg(inPicture(QPoint(0, text->y())).y())));

    // The coverage in the stretched middle of the graphic is the same number at
    // every window size — that piece is stretched, not redrawn — so the two
    // readings above stay comparable even though a wider theme border makes a
    // taller window.
    const int drawn = qAlpha(picture.pixel(sample));
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

        // And no field either (issue #100, AK 8). `widgets/lineedit` is as
        // absent here as `dialogs/background` is, and a build that drew it all
        // the same would put a graphic on a window that has no theme — the
        // middle of the text area is where it would stand.
        QCOMPARE(picture.pixelColor(inPicture(fieldSurface(text))),
                 m_window->palette().color(QPalette::Window));
        QCOMPARE(picture.pixelColor(inPicture(fieldEdge(text))),
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
