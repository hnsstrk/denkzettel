#include "capture/capturewindow.h"

#include "capture/textareaheight.h"
#include "capture/textcontrast.h"
#include "store/note.h"
#include "store/store.h"

#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowEffects>
#include <KWindowShadow>
#include <KWindowSystem>

#include <QAbstractTextDocumentLayout>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDateTime>
#include <QFileInfo>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>
#include <QtMath>

#include <utility>

namespace
{
constexpr int WindowWidth = 600;

/** Time the compositor gets to notice the surface going away, in ms. */
constexpr int RemapDelayMs = 50;

/** Inner spacing of wireframe 4b — counted on top of the theme's own margin. */
constexpr QMargins ContentMargins = QMargins(12, 10, 12, 8);

/**
 * The whole grouping of the window since the separator line was dropped: the
 * footer gets more air than the application name (wireframe 4b).
 */
constexpr int SpacingBelowAppName = 8;
constexpr int SpacingAboveFooter = 12;

/**
 * Where the hull comes from, and what KSvg falls back to without an answer.
 *
 * Latin-1 views and not `QString`: a `QString` at namespace scope is a non-POD
 * static — it is built in an unspecified order relative to other translation
 * units and torn down again at exit. These four are pure ASCII, so the view
 * costs nothing and carries the same name.
 */
constexpr QLatin1StringView DesktopThemePath("plasma/desktoptheme");
constexpr QLatin1StringView HullImage("dialogs/background");
constexpr QLatin1StringView ShadowPrefix("shadow");
constexpr QLatin1StringView DefaultDesktopTheme("default");

/**
 * Where the text field comes from: the same source as the hull, one graphic
 * down (issue #100, user decision of 06.08.2026).
 *
 * KRunner's entry field is drawn from exactly this image and prefix
 * (`TextField.qml:187–191`), and KRunner is the user's yardstick. `base` is
 * the resting state, `focus` the layer on top of it — Plasma's own layering,
 * named by the element `hint-focus-over-base` in the graphic itself. Under
 * several themes `base` barely covers, and the focus layer is what makes it a
 * field there (issue #102).
 */
constexpr QLatin1StringView FieldImage("widgets/lineedit");
constexpr QLatin1StringView FieldPrefix("base");
constexpr QLatin1StringView FocusPrefix("focus");

/**
 * The variant of the theme's graphic a window without a blurring compositor
 * gets. Plasma picks between `opaque` and `translucent` the same way
 * (`KSvg::ImageSet::setSelectors()`, imageset.h).
 */
constexpr QLatin1StringView OpaqueSelector("opaque");

/**
 * Whether KSvg will keep this name — asked before the name is handed over.
 *
 * `KSvg::ImageSet` keys its shared private by the name it is **given** and
 * removes it again by the name it has **resolved** (ksvg 6.29, `imageset.cpp`
 * and `private/imageset_p.cpp`). A name with no image set behind it resolves to
 * `default`, so the destructor takes the wrong key out of the table and leaves
 * the given one pointing at freed memory. The next set built under that name
 * finds the freed pointer, references it and connects to it — measured with
 * AddressSanitizer as a read through a dangling `d` in the **second**
 * constructor, and in the product as SIGSEGV or as
 * `malloc(): unaligned tcache chunk detected` one allocation later (issue
 * #107). The fault is KSvg's; ours is only not to walk into it.
 *
 * The test is the one `metaDataForImageSet()` makes: a directory of that name
 * on the data path with a metadata file in it.
 */
bool desktopThemeResolves(const QString &name)
{
    const QString directory = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                     QStringLiteral("%1/%2").arg(DesktopThemePath, name),
                                                     QStandardPaths::LocateDirectory);
    return !directory.isEmpty()
        && (QFileInfo::exists(directory + QStringLiteral("/metadata.json"))
            || QFileInfo::exists(directory + QStringLiteral("/metadata.desktop")));
}

/**
 * One shadow tile of the desktop theme, ready for the compositor.
 *
 * Not `Svg::pixmap(element)`, and that is measured: it ignores the element and
 * hands back the **whole** image at the size of the SVG — every tile would then
 * be the entire shadow, corners included, and the compositor would take it
 * without complaint (measurement 6 of this story). `image()` with the element's
 * own size is the call that cuts.
 */
KWindowShadowTile::Ptr shadowTile(KSvg::FrameSvg *tiles, const QString &element)
{
    const QSize size = tiles->elementSize(element).toSize();
    if (size.isEmpty()) {
        return {};
    }

    const QImage image = tiles->image(size, element);
    if (image.isNull()) {
        return {};
    }

    auto tile = KWindowShadowTile::Ptr::create();
    tile->setImage(image);
    return tile;
}

/** Small label in the smallest readable font — the two texts around the field. */
QLabel *smallLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

    return label;
}

/** Small and dimmed — the key hint below the field. */
QLabel *subtleLabel(const QString &text, QWidget *parent)
{
    QLabel *label = smallLabel(text, parent);

    // The role, not the colour: the daemon keeps the window for its whole life
    // (SPEC 2.1), and a colour taken from the palette once would stay put when
    // the user changes the colour scheme (issue #54). A role is resolved anew
    // on every palette change.
    label->setForegroundRole(QPalette::PlaceholderText);

    return label;
}
}

namespace capture
{

ContrastEffect contrastEffectOf(const QString &desktopTheme)
{
    // Themes carry their KPlugin data in `metadata.json` these days, but the
    // effect groups stayed where Plasma::Theme reads them: in the KConfig file
    // beside it. A theme may have either file, both, or only the JSON one —
    // `default` has no `metadata.desktop` at all, and that is the measured case
    // of "no group, no contrast effect".
    const QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QStringLiteral("%1/%2/metadata.desktop")
                                                    .arg(DesktopThemePath, desktopTheme));
    if (file.isEmpty()) {
        return {};
    }

    const KConfigGroup group(KSharedConfig::openConfig(file, KConfig::SimpleConfig),
                             QStringLiteral("ContrastEffect"));
    if (!group.exists()) {
        return {};
    }

    ContrastEffect effect;
    // The defaults are the ones Plasma::Theme documents: a group that names
    // only `enabled` leaves the picture as it is.
    effect.enabled = group.readEntry("enabled", false);
    effect.contrast = group.readEntry("contrast", 1.0);
    effect.intensity = group.readEntry("intensity", 1.0);
    effect.saturation = group.readEntry("saturation", 1.0);
    return effect;
}

ThemeTextColours themeTextColoursOf(const QString &desktopTheme)
{
    // Beside the graphic and beside `metadata.desktop`, in the same file kind
    // and read the same way. A theme that ships no `colors` file — four of the
    // eight on the user's machine — leaves both colours invalid, and the
    // window then keeps to the colour scheme, which is the other half of the
    // user decision of 04.08.2026.
    const QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QStringLiteral("%1/%2/colors")
                                                    .arg(DesktopThemePath, desktopTheme));
    if (file.isEmpty()) {
        return {};
    }

    const KConfigGroup group(KSharedConfig::openConfig(file, KConfig::SimpleConfig),
                             QStringLiteral("Colors:Window"));
    if (!group.exists()) {
        return {};
    }

    ThemeTextColours colours;
    colours.normal = group.readEntry("ForegroundNormal", QColor());
    colours.inactive = group.readEntry("ForegroundInactive", QColor());
    return colours;
}

bool sessionBlursBehindWindows()
{
    // Neither offscreen nor any other platform without a compositing window
    // server can blur anything, and asking a session bus about it would make
    // the answer depend on whether a Plasma session happens to stand beside
    // the test run — the very unsteadiness that makes a theme-dependent test
    // trustworthy in one place and worthless in another (issue #83, F16).
    if (!KWindowSystem::isPlatformWayland() && !KWindowSystem::isPlatformX11()) {
        return false;
    }

    QDBusMessage question = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                           QStringLiteral("/Effects"),
                                                           QStringLiteral("org.kde.kwin.Effects"),
                                                           QStringLiteral("isEffectLoaded"));
    question << QStringLiteral("blur");

    // A short timeout, because this stands in the way of the first frame: a
    // window server that does not answer is one that does not blur either.
    const QDBusReply<bool> answer =
        QDBusConnection::sessionBus().call(question, QDBus::Block, 1000);
    return answer.isValid() && answer.value();
}

}

CaptureWindow::CaptureWindow(Store *store, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , m_store(store)
    , m_text(new QPlainTextEdit(this))
    , m_plasmaConfig(KSharedConfig::openConfig(QStringLiteral("plasmarc")))
    , m_hull(new KSvg::FrameSvg(this))
    , m_shadowTiles(new KSvg::FrameSvg(this))
    , m_field(new KSvg::FrameSvg(this))
    , m_focus(new KSvg::FrameSvg(this))
    , m_blursBehind(capture::sessionBlursBehindWindows())
{
    setWindowTitle(i18n("Denkzettel"));

    // The hull has rounded corners, so the corners of the window have to be
    // able to disappear. The theme's own graphic decides how much of the rest
    // stays see-through; without a theme paintEvent() fills every pixel.
    setAttribute(Qt::WA_TranslucentBackground);

    for (KSvg::FrameSvg *frame : {m_hull, m_shadowTiles, m_field, m_focus}) {
        frame->setEnabledBorders(KSvg::FrameSvg::AllBorders);
        connect(frame, &KSvg::Svg::repaintNeeded, this, qOverload<>(&QWidget::update));
    }
    m_hull->setImagePath(HullImage);
    m_shadowTiles->setImagePath(HullImage);
    m_shadowTiles->setElementPrefix(ShadowPrefix);
    // The colour set of a dialog background, the one Plasma draws this graphic
    // with. Under `default` all seven sets render the same pixels (measured,
    // `native-huelle-breeze.txt`, section B); under a theme that ships more
    // than one they would not, and this is the set the image is meant for.
    m_hull->setColorSet(KSvg::Svg::Window);

    // The field carries no colour set of its own, and that is measured rather
    // than an omission: `setColorSet(View)` renders pixel for pixel what
    // `Window` renders, under all eleven themes measured (issue #100, F3). The
    // colour of this graphic comes out of the class names in the SVG
    // (`ColorScheme-ViewBackground`, `ColorScheme-Frame`), not out of the set.
    m_field->setImagePath(FieldImage);
    m_field->setElementPrefix(FieldPrefix);
    // The same image and the same set, one prefix further: what the theme draws
    // while the field has the keyboard. No colour set here either, for the
    // reason above — the colour comes out of the class names in the SVG, here
    // `ColorScheme-ViewFocus` (measured: under the user's scheme that is their
    // accent, 61,212,37).
    m_focus->setImagePath(FieldImage);
    m_focus->setElementPrefix(FocusPrefix);

    m_text->setFrameShape(QFrame::NoFrame);
    m_text->setPlaceholderText(i18n("Capture a thought…"));
    m_text->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_text->installEventFilter(this);
    // The text area draws no ground of **its own**, and since issue #100 that
    // is no longer the same sentence as "one continuous surface". There is a
    // second surface again — it comes out of the theme's own `widgets/lineedit`
    // and is painted below in paintEvent(). What this line rules out is the
    // ground Qt would fill in: a rectangle, which would square off the rounded
    // corners the field graphic draws for itself.
    m_text->viewport()->setAutoFillBackground(false);
    // No applyTextColours() here: since #85 the colours depend on the desktop
    // theme, so they are set where the theme is read. reloadDesktopTheme()
    // below calls it, and it is the only place that has both halves.

    // Activating the window puts the keyboard focus straight into the text.
    setFocusProxy(m_text);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    // Not dimmed, unlike the hint below: the name is the heading of this
    // window, and at rest it is the only text in it that is not a placeholder.
    // A window that shows nothing but dimmed text looks foreign to the scheme
    // under it even where every single colour is right — the customer's finding
    // of 04.08.2026 on two screenshots of theirs, measured as F6/F7 (issue
    // #84).
    QLabel *appName = smallLabel(i18n("Denkzettel"), this);
    appName->setForegroundRole(QPalette::WindowText);
    layout->addWidget(appName);
    layout->addSpacing(SpacingBelowAppName);
    layout->addWidget(m_text);
    layout->addSpacing(SpacingAboveFooter);

    QLabel *hint = subtleLabel(i18n("Esc discards · Ctrl+Enter saves"), this);
    hint->setAlignment(Qt::AlignCenter);
    layout->addWidget(hint);
    m_appName = appName;
    m_subtleLabels = {hint};

    connect(m_text->document()->documentLayout(),
            &QAbstractTextDocumentLayout::documentSizeChanged,
            this,
            &CaptureWindow::adjustHeight);

    // A desktop theme change has to reach a window that is already standing —
    // the daemon builds this one once and keeps it (SPEC 2.1). The watch sits
    // on the file and not on KConfigWatcher, and that is measured: a writer
    // that omits `KConfig::Notify` reaches KConfigWatcher not at all, while
    // KDirWatch sees both kinds (measurement 2 of this story). KConfig replaces
    // the file rather than rewriting it, which is why `created` counts too.
    const QString plasmarc = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QStringLiteral("/plasmarc");
    KDirWatch::self()->addFile(plasmarc);
    // Both connections spelled out rather than looped over the two signals:
    // clazy cannot see through a loop variable that a pointer-to-member is a
    // signal and reports the pair as a non-signal connect. Two lines are
    // shorter than the loop plus a suppression, and the next reader gets them
    // for free.
    auto onPlasmarcChanged = [this, plasmarc](const QString &path) {
        if (path == plasmarc) {
            reloadDesktopTheme();
        }
    };
    connect(KDirWatch::self(), &KDirWatch::dirty, this, onPlasmarcChanged);
    connect(KDirWatch::self(), &KDirWatch::created, this, onPlasmarcChanged);

    reloadDesktopTheme();
    adjustHeight();
    resize(WindowWidth, sizeHint().height());
}

CaptureWindow::~CaptureWindow() = default;

void CaptureWindow::reloadDesktopTheme(const QString &name)
{
    QString theme = name;
    if (theme.isEmpty()) {
        m_plasmaConfig->reparseConfiguration();
        theme = KConfigGroup(m_plasmaConfig, QStringLiteral("Theme"))
                    .readEntry("name", QString(DefaultDesktopTheme));
    }

    // KSvg's own fallback, taken here rather than there: handing it a name it
    // cannot resolve is what corrupts the heap (desktopThemeResolves() above,
    // issue #107). Whoever sets a desktop theme and later removes its package
    // has exactly that `plasmarc`, and both roads into this function come past
    // this line — the constructor and the watch on the file.
    if (!desktopThemeResolves(theme)) {
        qWarning("Desktop theme \"%s\" is not on the data path; falling back to \"default\".",
                 qPrintable(theme));
        theme = DefaultDesktopTheme;
    }

    // Two measured properties of KSvg in one line (measurements 1 and 3 of this
    // story). First: KSvg does not read `plasmarc` itself — pointed at the
    // desktop theme path alone it stays on `default`, whatever the file says,
    // so the name has to be handed over. Second: a FrameSvg keeps the image it
    // once resolved. Renaming its image set does not move it, re-setting the
    // path does not, re-assigning the same set does not — only a **fresh** set
    // does. Hence a new one here rather than a rename, and the old one only
    // goes once all three frames point at the new one.
    auto imageSet = std::make_unique<KSvg::ImageSet>(theme, DesktopThemePath);
    // Outside a session that blurs, the translucent variant of the graphic
    // would leave a window one can see through and hardly read — SPEC 3.2
    // point 4 promises the opposite. `opaque` is the theme's own answer to
    // that, and picking it is what Plasma does, not an adjustment of ours. A
    // theme that ships no such variant simply keeps the one it has.
    if (!m_blursBehind) {
        imageSet->setSelectors({QString(OpaqueSelector)});
    }
    // The field frame belongs in this loop, and leaving it out would fail
    // silently: it would keep drawing the old theme's graphic on a window that
    // has changed theme, and no return value would say so (issue #100, F5). The
    // focus layer beside it is the same trap a fourth time (issue #102, AK 4).
    for (KSvg::FrameSvg *frame : {m_hull, m_shadowTiles, m_field, m_focus}) {
        frame->setImageSet(imageSet.get());
    }
    m_imageSet = std::move(imageSet);
    m_contrast = capture::contrastEffectOf(theme);
    // Read here and kept: the writing comes from the same hand as the surface
    // (user decision 04.08.2026, issue #85), so it is read where the
    // surface is. Kept rather than asked for again, because the next palette
    // change has to find it — a colour scheme change does not change the
    // theme, and applyTextColours() runs on both occasions.
    m_themeText = capture::themeTextColoursOf(theme);
    applyTextColours();

    applyHullMargins();
    applyFieldMargin();
    // The margins are part of the height: a wider theme border makes a taller
    // window at the same five lines, and so does the field's own border.
    adjustHeight();
    resizeHull();
    resizeField();
    bindShadow();
    update();
}

void CaptureWindow::showCapture()
{
    // A mapped window cannot take the keyboard focus back on Wayland (T1,
    // issue #1): hide() destroys the surface, so the following show() is a
    // fresh mapping, and a fresh toplevel is focused by the compositor on its
    // own. The delay gives the compositor time to see the surface go away.
    if (isVisible()) {
        hide();
        QTimer::singleShot(RemapDelayMs, this, &CaptureWindow::present);
        return;
    }

    present();
}

bool CaptureWindow::eventFilter(QObject *watched, QEvent *event)
{
    // The height rests on the line spacing, and that changes with the font —
    // but the document does not, so documentSizeChanged stays silent and the
    // field keeps the height it was built with (issue #56). The filter is the
    // place for this and an overridden changeEvent() of the window is not: a
    // font set on the text area alone never reaches the window, and that is
    // the road the test takes (measurement 3 of the sprint 6 estimate).
    if (watched == m_text && event->type() == QEvent::FontChange) {
        adjustHeight();
        return QWidget::eventFilter(watched, event);
    }

    // The field is drawn on the geometry of the text area, so it has to hear of
    // every change to it — and the text area changes on its own account, when
    // the layout hands it a new size after a keystroke. The window's own
    // resizeEvent() is not that moment: it fires before the layout has run.
    if (watched == m_text && event->type() == QEvent::Resize) {
        resizeField();
        return QWidget::eventFilter(watched, event);
    }

    // Both text classes may be drawn in a colour of the scheme, so they have
    // to be written over again whenever the scheme moves (issue #54: a colour
    // taken once and kept stays put). Under a theme that brings its own
    // `colors` file applyTextColours() keeps the theme's colour here — that is
    // the whole point of the precedence living in one place (issue #85, AK 7).
    if (watched == m_text && event->type() == QEvent::PaletteChange) {
        applyTextColours();
        return QWidget::eventFilter(watched, event);
    }

    if (watched == m_text && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        const bool isReturn = keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter;

        if (isReturn && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            save();
            return true;
        }

        if (keyEvent->key() == Qt::Key_Escape) {
            discard();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CaptureWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (!m_hull->isValid()) {
        // Outside a Plasma session `dialogs/background` is simply not there
        // (SPEC 3.2 point 4). The window then wears no hull — and that is the
        // whole difference: it stays opaque, and it stays usable.
        painter.fillRect(rect(), palette().color(QPalette::Window));
        QWidget::paintEvent(event);
        return;
    }

    // One graphic, one call, no colour of ours. What the theme draws is what
    // the window wears — rounding, edge and translucency included. The edge is
    // not a line in another colour: theme graphics draw it as a step in
    // coverage (235 against 216 under `default`, measured), and it becomes
    // visible only because the hull lets the ground through.
    painter.drawPixmap(0, 0, m_hull->framePixmap());

    // The field on top of the hull and underneath the text (issue #100): the
    // same image set, one graphic down, drawn the way KRunner draws its own
    // entry field. Nothing of it is ours either — where the theme's graphic
    // covers by 15 of 255, the field stays a hint, and that is the limit
    // SPEC 3.1 names rather than one to repair.
    //
    // Guarded, because a theme may resolve the hull and not this graphic: then
    // no field is drawn, and the window is the one SPEC 3.2 point 4 promises.
    if (m_field->isValid()) {
        painter.drawPixmap(m_text->pos(), m_field->framePixmap());
    }

    // And the focus layer over the resting one, on the same rectangle: Plasma's
    // own order, written into the graphic as `hint-focus-over-base` (issue
    // #102, AK 2).
    //
    // Drawn while the window is the **active** one, which is Plasma's
    // `activeFocus` and not Qt's `hasFocus()` alone. A field that kept its edge
    // in a window the keyboard has left would say something untrue about where
    // typing goes — and the window does stay standing when the focus leaves it
    // (SPEC 3). `hasFocus()` resolves through the focus proxy set above, so it
    // asks about the text area and not about the window's own frame.
    if (m_focus->isValid() && hasFocus() && isActiveWindow()) {
        painter.drawPixmap(m_text->pos(), m_focus->framePixmap());
    }

    QWidget::paintEvent(event);
}

void CaptureWindow::resizeEvent(QResizeEvent *event)
{
    resizeHull();
    QWidget::resizeEvent(event);
}

bool CaptureWindow::event(QEvent *event)
{
    // The pixel ratio of the window is not settled when show() returns: under
    // Wayland Qt reports 2 first and 1,6 about a second later, and it delivers
    // that as a DevicePixelRatioChange **without** a Resize beside it
    // (measured, `sonde2-fensterlauf-wayland-skala-1.txt`). A hull that is only
    // redrawn out of resizeEvent() would keep drawing at 2 on a window that is
    // 1,6, for good. Offscreen this event never arrives — no test of this
    // project would notice the line missing.
    if (event->type() == QEvent::DevicePixelRatioChange) {
        resizeHull();
        resizeField();
        update();
    }

    // No branch of ours for the activation, and that is measured rather than
    // forgotten (issue #102). Qt already repaints a toplevel when it gains or
    // loses the activation, and the paint arrives before anything can be read
    // back — measured on both platforms, with the branch built in and built
    // out. What holds the assurance instead is the picture out of the session.

    return QWidget::event(event);
}

void CaptureWindow::resizeHull()
{
    // The ratio before the size: a FrameSvg does not follow the screen by
    // itself — it stands at 1 whatever the session scales to, and the
    // application has to hand the number over (measured,
    // `sonde1-rahmenmasse-offscreen.txt`). Setting it after resizeFrame() works
    // just as well; there is no order to find here.
    m_hull->setDevicePixelRatio(devicePixelRatioF());
    m_hull->resizeFrame(size());
    m_shadowTiles->resizeFrame(size());

    // The blur region **is** the hull's mask, and the hull just changed shape.
    bindWindowEffects();
}

void CaptureWindow::resizeField()
{
    // The field wears the geometry of the text area, so it moves and grows with
    // it — the window grows with every keystroke (SPEC 3).
    //
    // The ratio first, for the same measured reason as the hull: a FrameSvg
    // stands at 1 whatever the session scales to, and offscreen the event that
    // would betray a missing line never arrives (issue #100, F4).
    m_field->setDevicePixelRatio(devicePixelRatioF());
    m_field->resizeFrame(m_text->size());
    // The focus layer lies on the same rectangle, so it takes the same two
    // lines. Left out, it would be drawn at ratio 1 over a field drawn at 1,6 —
    // and offscreen that never shows, because the event that hands the second
    // ratio over does not arrive there (issue #100, F4; issue #83).
    m_focus->setDevicePixelRatio(devicePixelRatioF());
    m_focus->resizeFrame(m_text->size());
}

void CaptureWindow::applyHullMargins()
{
    qreal left = 0;
    qreal top = 0;
    qreal right = 0;
    qreal bottom = 0;
    if (m_hull->isValid()) {
        m_hull->getMargins(left, top, right, bottom);
    }

    // The inner spacing of 4b counts on top of the strip the theme claims for
    // itself: the text starts 16 px from the window edge under Breeze and
    // 20 px under an 8 px theme.
    layout()->setContentsMargins(ContentMargins.left() + qRound(left),
                                 ContentMargins.top() + qRound(top),
                                 ContentMargins.right() + qRound(right),
                                 ContentMargins.bottom() + qRound(bottom));
}

void CaptureWindow::applyFieldMargin()
{
    qreal left = 0;
    qreal top = 0;
    qreal right = 0;
    qreal bottom = 0;
    if (m_field->isValid()) {
        m_field->getMargins(left, top, right, bottom);
    }

    // The strip the field graphic claims for itself, and nothing on top of it:
    // the note text begins on the inner edge of the drawn border and no
    // further in. The application name and the footer stay where they are,
    // because the layout does not hear of this at all.
    //
    // Qt's own document margin of four used to be added here. That put the
    // note text nine to ten points right of the application name while only
    // six of them were drawn by anything — measured 24.08.2026 across all
    // eight installed themes, whose border is 6.00 on every one of them, and
    // in the picture at 9.3 points. Four of them belonged to nobody. Where a
    // theme brings no field graphic the margin is now zero and both edges fall
    // together, which is the single surface of wireframe 4b (issue #81; the
    // issue predates the field graphic of #100 and asked for the two edges to
    // coincide, which with a drawn frame between them is not what a text field
    // does).
    //
    // One number for four sides, because a document has one margin. The widest
    // of the four, so no side of the text can end up underneath the border
    // (issue #100, AK 5 — met exactly now, with no slack left over).
    //
    // `documentMargin` and not `setViewportMargins()`, which is protected, nor
    // `setContentsMargins()` on the text area, which measurably does nothing
    // (F7). And adjustHeight() already counts the document margin into the
    // chrome, so the window grows by the two borders on its own.
    const qreal border = qMax(qMax(left, right), qMax(top, bottom));
    m_text->document()->setDocumentMargin(border);
}

/**
 * The colour the note text and the placeholder really stand on, over a backdrop
 * of the given colour.
 *
 * Neither a colour of the scheme nor one of the theme: with a valid hull the
 * window fills nothing of its own (see paintEvent). It draws the hull, the
 * field on top of it, and lets whatever the compositor holds behind it show
 * through both. What lies under the writing is therefore a mixture of backdrop,
 * hull coverage and field coverage, and under themes whose graphic covers by
 * 2.7 percent the backdrop is nearly all of it (issue #93).
 *
 * So the colour is sampled and not derived: the same two draws paintEvent makes,
 * into a picture the size of the text area, then the pixel in its middle. The
 * hull is drawn at that size as well — its middle carries one colour, which is
 * the one asked for here, and drawing the whole window for one pixel would buy
 * nothing.
 */
QColor CaptureWindow::fieldSurfaceOver(const QColor &backdrop) const
{
    const QSize size = m_text->size();
    if (!m_hull->isValid() || size.isEmpty()) {
        return backdrop;
    }

    QImage probe(size, QImage::Format_ARGB32_Premultiplied);
    probe.fill(backdrop);
    QPainter painter(&probe);
    painter.drawPixmap(0, 0, m_hull->framePixmap());
    if (m_field->isValid()) {
        painter.drawPixmap(0, 0, m_field->framePixmap());
    }
    painter.end();

    return probe.pixelColor(size.width() / 2, size.height() / 2);
}

void CaptureWindow::applyTextColours()
{
    // The one place both text classes get their colour, and the only place the
    // order of precedence is written down (issue #85). It has to be one place:
    // the two sources move on different occasions — the theme on a theme
    // change, the scheme on a palette change — and a rule spread over two
    // places would be wrong after the next change of the other kind.
    //
    // The note text through KSvg and the dimmed class through the file we read
    // ourselves, and that is not a taste: `KSvg::Svg::StyleSheetColor` has no
    // counterpart to `ForegroundInactive` (see themeTextColoursOf()).
    //
    // The scheme's half of the note text is `WindowText` and not the role for
    // entry fields: since issue #100 the text stands on two grounds depending
    // on the theme, and `WindowText` stays above 4,5:1 in both while the view
    // role falls to 4,22:1 on the hull. Customer's instruction, 07.08.2026.
    QColor noteColour = m_themeText.normal.isValid()
        ? m_hull->color(KSvg::Svg::Text)
        : this->palette().color(QPalette::WindowText);
    const QColor subtleColour = m_themeText.inactive.isValid()
        ? m_themeText.inactive
        : this->palette().color(QPalette::PlaceholderText);

    // The ranking of the two writings that share a ground. The note text and
    // the placeholder it replaces both stand on the field, and under two themes
    // the placeholder read better than the note it makes way for — measured
    // 05.08.2026, over a light ground 2.06:1 against 4.64:1 and over a dark one
    // 1.36:1 against 4.62:1 (issue #97). Whoever begins to type then sees their
    // note worse than the prompt to write it.
    //
    // **Judged on the worse of the two cases, because the window lets the
    // screen through.** What lies under the writing depends on what the user
    // has behind the window, so each colour is asked over a white and over a
    // black backdrop and kept at its **poorer** of the two. The note is lifted
    // where its poorer case is worse than the placeholder's — that is what
    // "never the quieter of the two" means for a window one cannot see behind.
    //
    // Measured 24.08.2026 with a real colour scheme in an isolated
    // configuration, over the six themes the finding touches: it lifts under
    // `cachyos-emerald-color` (note 1.91:1 over a light ground against the
    // placeholder's 3.69:1 poorest) and under `cachyos-emerald-light` (1.32:1
    // over a dark one against 3.66:1) — the two the issue names, reproduced
    // almost to the digit. Under `default`, `breeze-dark` and `breeze-light`
    // the field graphic is opaque, both backdrops give the same ground, and
    // nothing changes.
    //
    // Where it does lift, the note takes the dimmed colour: the more legible of
    // the two the theme itself holds, so nothing is invented that the theme does
    // not offer. Where a theme ranks its two writings the right way round this
    // changes nothing — a window that always lifted would break the ordinary
    // case to heal the exception (customer's decision of 24.08.2026).
    const capture::WritingChoice choice{.note = noteColour,
                                        .placeholder = subtleColour,
                                        .groundOverWhite = fieldSurfaceOver(QColor(Qt::white)),
                                        .groundOverBlack = fieldSurfaceOver(QColor(Qt::black))};
    if (capture::noteIsTheQuieterWriting(choice)) {
        noteColour = subtleColour;
    }

    QPalette palette = m_text->palette();
    // Written onto the widget on every occasion rather than set once — a
    // colour taken from the palette and kept would freeze (issue #54).
    palette.setColor(QPalette::Text, noteColour);
    // The third place the dimmed class shows: the placeholder of the empty
    // text area draws out of this role of the text area's own palette.
    palette.setColor(QPalette::PlaceholderText, subtleColour);
    palette.setColor(QPalette::Base, Qt::transparent);

    if (palette != m_text->palette()) {
        m_text->setPalette(palette);
    }

    // The heading carries the note text's colour, the hint below stays dimmed
    // (issue #84).
    if (m_appName) {
        QPalette namePalette = m_appName->palette();
        namePalette.setColor(m_appName->foregroundRole(), noteColour);
        if (namePalette != m_appName->palette()) {
            m_appName->setPalette(namePalette);
        }
    }

    for (QLabel *label : std::as_const(m_subtleLabels)) {
        QPalette labelPalette = label->palette();
        labelPalette.setColor(label->foregroundRole(), subtleColour);
        if (labelPalette != label->palette()) {
            label->setPalette(labelPalette);
        }
    }
}

void CaptureWindow::bindShadow()
{
    m_shadow.reset();

    // The tiles come from the same image as the hull, under its `shadow`
    // prefix. All eight desktop themes on the user's machine bring them
    // (measured 01.08.2026); a theme without them simply gets no shadow.
    // Before the first show() there is no native window to hang it on; present()
    // comes back here once there is.
    if (!windowHandle() || !m_shadowTiles->hasElementPrefix(ShadowPrefix)) {
        return;
    }

    auto shadow = std::make_unique<KWindowShadow>();
    shadow->setTopLeftTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-topleft")));
    shadow->setTopTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-top")));
    shadow->setTopRightTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-topright")));
    shadow->setRightTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-right")));
    shadow->setBottomRightTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-bottomright")));
    shadow->setBottomTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-bottom")));
    shadow->setBottomLeftTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-bottomleft")));
    shadow->setLeftTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-left")));

    qreal left = 0;
    qreal top = 0;
    qreal right = 0;
    qreal bottom = 0;
    m_shadowTiles->getMargins(left, top, right, bottom);
    shadow->setPadding(QMargins(qRound(left), qRound(top), qRound(right), qRound(bottom)));

    shadow->setWindow(windowHandle());
    // Fails without a compositor, and that is not an error of ours: offscreen
    // there is nobody to hand the tiles to (measured, sprint 6 planning 3.3).
    // The tiles above are the part that can be shown either way (AK 7).
    shadow->create();

    m_shadow = std::move(shadow);
}

void CaptureWindow::bindWindowEffects()
{
    // Passing a window that has no native handle yet is not a failure but a
    // crash: `enableBlurBehind(nullptr, …)` ends in SIGSEGV under Wayland, and
    // offscreen the same call returns quietly — so no test of this project
    // would find the guard missing (measured, `sonde2-fensterlauf-*`, part E).
    if (!windowHandle() || !m_hull->isValid()) {
        return;
    }

    // Both calls are void, and the one value that could be read back lies
    // before the first registration (F9/F10 of the pre-check). Nothing here
    // reports a failure; what this does is shown by a picture out of a session
    // and by nothing else.
    //
    // The region is the theme's mask and is given in logical pixels, which is
    // what the header asks for — measured 600x186 under `default` at ratio 1
    // and at 1,6 alike. (174 until issue #100; the field's own border made the
    // window twelve pixels taller. What the measurement is about is the word
    // "logical", and that is unchanged.)
    const QRegion region = m_hull->mask();
    KWindowEffects::enableBlurBehind(windowHandle(), true, region);

    // The second registration Plasma makes, and the themes that draw almost
    // nothing ask for it in so many words: it is what puts a readable ground
    // under text in a see-through window. A theme without the group gets none.
    if (m_contrast.enabled) {
        KWindowEffects::enableBackgroundContrast(windowHandle(),
                                                 true,
                                                 m_contrast.contrast,
                                                 m_contrast.intensity,
                                                 m_contrast.saturation,
                                                 region);
    }
}

void CaptureWindow::present()
{
    show();
    raise();
    activateWindow();

    // After show(), and after every show(): each appearance destroys the
    // Wayland surface and maps a fresh one (SPEC 3), and a shadow bound to the
    // old one is gone with it. This is the line no test of this project would
    // notice missing — it shows only on the second opening.
    bindShadow();

    // And the effects immediately behind it, for a harder reason than the
    // shadow: measured over seven runs the blur takes hold **only** when it is
    // registered right after show(). Registered a second later it does nothing
    // at all — not with our mask, and not with the empty region that means the
    // whole window (`sonde4-weichzeichner-*.txt`). No return value says so.
    bindWindowEffects();
}

void CaptureWindow::save()
{
    const QString content = m_text->toPlainText().trimmed();
    if (content.isEmpty()) {
        discard();
        return;
    }

    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.type = Note::Type::Text;
    note.content = content;

    if (!m_store->addNote(note)) {
        // Keep window and text: a lost thought is worse than a window that
        // stays open.
        qWarning("Storing the note failed: %s", qPrintable(m_store->lastError()));
        return;
    }

    m_text->clear();
    hide();
}

void CaptureWindow::discard()
{
    m_text->clear();
    hide();
}

void CaptureWindow::adjustHeight()
{
    // QPlainTextDocumentLayout reports its height as a line count, not in
    // pixels — wrapped lines included.
    const int documentLines = qCeil(m_text->document()->size().height());
    const int chrome = 2 * qRound(m_text->document()->documentMargin()) + 2 * m_text->frameWidth();

    m_text->setFixedHeight(capture::textAreaHeight(documentLines, m_text->fontMetrics().lineSpacing(), chrome));
    // Both lines before the resize, and only the second one is obvious. The
    // setFixedHeight() above does not reach the layout at once; it posts a
    // layout request that is delivered later. Until then the layout hands out
    // the hint it cached and the window keeps the minimum the layout applied
    // when it was last activated — and `resize()` is clamped by that minimum.
    // On a **shown** window the resize therefore takes no effect **in either
    // direction**. Measured 24.08.2026 on the unfixed state: a window at rest
    // stays at rest even after eight lines are typed into it, 186 against 186.
    // Issue #79 describes only the way back — its own measurement of 04.08.2026
    // shows the window still growing then (174 → 228, and staying at 228 after
    // clearing). Since then it does not even grow while shown; what changed in
    // between is not established here.
    //
    // `invalidate()` drops the cached hint, `activate()` recomputes it and
    // applies the minimum at once — and does nothing at all while the layout
    // still counts as activated, which is why both lines are needed and why
    // the order is this one.
    //
    // On a hidden window no minimum has been applied yet, so the fault cannot
    // occur there. That is why the check for it has to show the window.
    layout()->invalidate();
    layout()->activate();
    resize(width(), sizeHint().height());
}
