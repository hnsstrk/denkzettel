#include "capture/windowhull.h"

#include <KConfigGroup>
#include <KDirWatch>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowEffects>
#include <KWindowShadow>
#include <KWindowSystem>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QFileInfo>
#include <QPainter>
#include <QStandardPaths>
#include <QWidget>
#include <QWindow>

#include <utility>

namespace
{
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
 * without complaint (measurement 6 of issue #55). `image()` with the element's
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

WindowHull::WindowHull(QWidget *window)
    : QObject(window)
    , m_window(window)
    , m_plasmaConfig(KSharedConfig::openConfig(QStringLiteral("plasmarc")))
    , m_hull(new KSvg::FrameSvg(this))
    , m_shadowTiles(new KSvg::FrameSvg(this))
    , m_blursBehind(sessionBlursBehindWindows())
{
    for (KSvg::FrameSvg *frame : {m_hull, m_shadowTiles}) {
        frame->setEnabledBorders(KSvg::FrameSvg::AllBorders);
        connect(frame, &KSvg::Svg::repaintNeeded, m_window, qOverload<>(&QWidget::update));
    }
    m_hull->setImagePath(HullImage);
    m_shadowTiles->setImagePath(HullImage);
    m_shadowTiles->setElementPrefix(ShadowPrefix);
    // The colour set of a dialog background, the one Plasma draws this graphic
    // with. Under `default` all seven sets render the same pixels (measured,
    // `native-huelle-breeze.txt`, section B); under a theme that ships more
    // than one they would not, and this is the set the image is meant for.
    m_hull->setColorSet(KSvg::Svg::Window);

    // A desktop theme change has to reach a window that is already standing —
    // the daemon builds its windows once and keeps them (SPEC 2.1). The watch
    // sits on the file and not on KConfigWatcher, and that is measured: a
    // writer that omits `KConfig::Notify` reaches KConfigWatcher not at all,
    // while KDirWatch sees both kinds (measurement 2 of issue #55). KConfig
    // replaces the file rather than rewriting it, which is why `created`
    // counts too.
    const QString configDirectory =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString plasmarc = configDirectory + QStringLiteral("/plasmarc");
    KDirWatch::self()->addFile(plasmarc);
    // The blur being switched off takes the same road (issue #93). The switch
    // in the system settings writes `[Plugins] blurEnabled` into `kwinrc` and
    // tells KWin over D-Bus; `org.kde.kwin.Effects` carries no signal to listen
    // to instead — neither one of its own (finding 15) nor the
    // `PropertiesChanged` its `emits-change` annotation promises (finding 18).
    // So the file announces the change, and it does not answer it. Its value is
    // deliberately not read: it says what the user *wants*, not whether
    // anything blurs — outside a Plasma session the same `blurEnabled=true`
    // stands in it and nothing is blurred, which is the case SPEC 3.2 point 4
    // is written for. And it is not even a reliable account of KWin's own
    // state: measured 2026-08-28 in a nested session, `reconfigure()` after
    // `blurEnabled=false` leaves the effect loaded for good. KWin itself is
    // asked again below, and its answer is the new one by then: KDirWatch
    // delivers the change 500 ms after the write, KWin has switched over after
    // 20 ms (same measurement).
    const QString kwinrc = configDirectory + QStringLiteral("/kwinrc");
    KDirWatch::self()->addFile(kwinrc);
    // Both connections spelled out rather than looped over the two signals:
    // clazy cannot see through a loop variable that a pointer-to-member is a
    // signal and reports the pair as a non-signal connect. Two lines are
    // shorter than the loop plus a suppression, and the next reader gets them
    // for free.
    auto onConfigChanged = [this, plasmarc, kwinrc](const QString &path) {
        if (path == plasmarc) {
            reload();
        } else if (path == kwinrc) {
            // Only when the answer has really changed: KWin writes into
            // `kwinrc` for its own reasons — a tiling layout, a virtual desktop
            // — and rebuilding the hull on each of them would throw the image
            // set away for nothing.
            //
            // ponytail: the question is a blocking D-Bus call, so a KWin that
            // hangs holds this event loop for as long as the call waits, up to
            // one second. Against a KWin that answers, twenty of these calls
            // take 1 to 2 ms (measured 2026-08-28), and `kwinrc` is written
            // seldom. If that ever stops holding, the way out is
            // QDBusConnection::asyncCall() with the comparison in its watcher.
            const bool blursBehind = sessionBlursBehindWindows();
            if (blursBehind != m_blursBehind) {
                m_blursBehind = blursBehind;
                reload();
            }
        }
    };
    connect(KDirWatch::self(), &KDirWatch::dirty, this, onConfigChanged);
    connect(KDirWatch::self(), &KDirWatch::created, this, onConfigChanged);
}

WindowHull::~WindowHull() = default;

bool WindowHull::isValid() const
{
    return m_hull->isValid();
}

KSvg::ImageSet *WindowHull::imageSet() const
{
    return m_imageSet.get();
}

QMargins WindowHull::margins() const
{
    qreal left = 0;
    qreal top = 0;
    qreal right = 0;
    qreal bottom = 0;
    if (m_hull->isValid()) {
        m_hull->getMargins(left, top, right, bottom);
    }

    return {qRound(left), qRound(top), qRound(right), qRound(bottom)};
}

QColor WindowHull::textColour() const
{
    return m_hull->color(KSvg::Svg::Text);
}

ThemeTextColours WindowHull::themeTextColours() const
{
    return m_themeText;
}

void WindowHull::paint(QPainter &painter) const
{
    // One graphic, one call, no colour of ours. What the theme draws is what
    // the window wears — rounding, edge and translucency included. The edge is
    // not a line in another colour: theme graphics draw it as a step in
    // coverage (235 against 216 under `default`, measured), and it becomes
    // visible only because the hull lets the ground through.
    painter.drawPixmap(0, 0, m_hull->framePixmap());
}

void WindowHull::reload(const QString &name)
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
    // this line — the window's first call and the watch on the file.
    if (!desktopThemeResolves(theme)) {
        qWarning("Desktop theme \"%s\" is not on the data path; falling back to \"default\".",
                 qPrintable(theme));
        theme = DefaultDesktopTheme;
    }

    // Two measured properties of KSvg in one line (measurements 1 and 3 of
    // issue #55). First: KSvg does not read `plasmarc` itself — pointed at the
    // desktop theme path alone it stays on `default`, whatever the file says,
    // so the name has to be handed over. Second: a FrameSvg keeps the image it
    // once resolved. Renaming its image set does not move it, re-setting the
    // path does not, re-assigning the same set does not — only a **fresh** set
    // does. Hence a new one here rather than a rename, and the old one only
    // goes once every frame points at the new one.
    auto imageSet = std::make_unique<KSvg::ImageSet>(theme, DesktopThemePath);
    // Outside a session that blurs, the translucent variant of the graphic
    // would leave a window one can see through and hardly read — SPEC 3.2
    // point 4 promises the opposite. `opaque` is the theme's own answer to
    // that, and picking it is what Plasma does, not an adjustment of ours. A
    // theme that ships no such variant simply keeps the one it has.
    //
    // Set on both branches, and that is measured rather than tidiness: KSvg
    // keys its private by the theme name and every live set of that name shares
    // it — the selectors with it (finding 4). A set built later inherits what an
    // earlier one selected, so leaving the empty case out kept the window opaque
    // when the blur came back (measured 2026-08-28, `blur on again`: selectors
    // still `[opaque]`, hull pixel alpha 255 instead of 216).
    imageSet->setSelectors(m_blursBehind ? QStringList()
                                         : QStringList{QString(OpaqueSelector)});
    for (KSvg::FrameSvg *frame : {m_hull, m_shadowTiles}) {
        frame->setImageSet(imageSet.get());
    }
    m_contrast = contrastEffectOf(theme);
    // Read here and kept: the writing comes from the same hand as the surface
    // (user decision 04.08.2026, issue #85), so it is read where the
    // surface is. Kept rather than asked for again, because the next palette
    // change has to find it — a colour scheme change does not change the
    // theme, and the windows re-apply their colours on both occasions.
    m_themeText = themeTextColoursOf(theme);

    // The previous set is held on to across the signal and dies at the end of
    // this function. A window that hangs frames of its own on imageSet() —
    // the capture window's text field does — moves them over in changed(), and
    // dropping the old set before that would leave them pointing at freed
    // memory. The same trap issue #107 is about, from the other side.
    const std::unique_ptr<KSvg::ImageSet> previous = std::move(m_imageSet);
    m_imageSet = std::move(imageSet);

    Q_EMIT changed();
}

void WindowHull::resizeToWindow()
{
    // The ratio before the size: a FrameSvg does not follow the screen by
    // itself — it stands at 1 whatever the session scales to, and the
    // application has to hand the number over (measured,
    // `sonde1-rahmenmasse-offscreen.txt`). Setting it after resizeFrame() works
    // just as well; there is no order to find here.
    m_hull->setDevicePixelRatio(m_window->devicePixelRatioF());
    m_hull->resizeFrame(m_window->size());
    m_shadowTiles->resizeFrame(m_window->size());

    // The blur region **is** the hull's mask, and the hull just changed shape.
    bindWindowEffects();
}

void WindowHull::bindToWindow()
{
    bindShadow();
    bindWindowEffects();
}

void WindowHull::bindShadow()
{
    m_shadow.reset();

    // The tiles come from the same image as the hull, under its `shadow`
    // prefix. All eight desktop themes on the user's machine bring them
    // (measured 01.08.2026); a theme without them simply gets no shadow.
    // Before the first show() there is no native window to hang it on;
    // bindToWindow() comes back here once there is.
    if (!m_window->windowHandle() || !m_shadowTiles->hasElementPrefix(ShadowPrefix)) {
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

    shadow->setWindow(m_window->windowHandle());
    // Fails without a compositor, and that is not an error of ours: offscreen
    // there is nobody to hand the tiles to (measured, sprint 6 planning 3.3).
    // The tiles above are the part that can be shown either way (issue #83).
    shadow->create();

    m_shadow = std::move(shadow);
}

void WindowHull::bindWindowEffects()
{
    // Passing a window that has no native handle yet is not a failure but a
    // crash: `enableBlurBehind(nullptr, …)` ends in SIGSEGV under Wayland, and
    // offscreen the same call returns quietly — so no test of this project
    // would find the guard missing (measured, `sonde2-fensterlauf-*`, part E).
    if (!m_window->windowHandle() || !m_hull->isValid()) {
        return;
    }

    // Both calls are void, and the one value that could be read back lies
    // before the first registration (F9/F10 of the pre-check). Nothing here
    // reports a failure; what this does is shown by a picture out of a session
    // and by nothing else.
    //
    // The region is the theme's mask and is given in logical pixels, which is
    // what the header asks for — measured 600x186 under `default` at ratio 1
    // and at 1,6 alike.
    const QRegion region = m_hull->mask();
    KWindowEffects::enableBlurBehind(m_window->windowHandle(), true, region);

    // The second registration Plasma makes, and the themes that draw almost
    // nothing ask for it in so many words: it is what puts a readable ground
    // under text in a see-through window. A theme without the group gets none.
    if (m_contrast.enabled) {
        KWindowEffects::enableBackgroundContrast(m_window->windowHandle(),
                                                 true,
                                                 m_contrast.contrast,
                                                 m_contrast.intensity,
                                                 m_contrast.saturation,
                                                 region);
    }
}

}
