#pragma once

#include <KSharedConfig>

#include <QColor>
#include <QMargins>
#include <QObject>

#include <memory>

class KWindowShadow;
class QPainter;
class QWidget;

namespace KSvg
{
class FrameSvg;
class ImageSet;
}

namespace capture
{

/**
 * What a desktop theme asks the compositor for in its `[ContrastEffect]` group.
 *
 * Plasma reads these four keys out of the theme's metadata file and hands them
 * to `KWindowEffects::enableBackgroundContrast()` (`Plasma/plasma/theme.h`).
 * A theme that names no group gets no contrast effect — `enabled` stays false.
 */
struct ContrastEffect {
    bool enabled = false;
    qreal contrast = 1;
    qreal intensity = 1;
    qreal saturation = 1;
};

/**
 * The `[ContrastEffect]` group of a desktop theme, or a disabled one.
 *
 * Read out of `metadata.desktop` ourselves rather than through `Plasma::Theme`:
 * that class lives in libPlasma, which the project does not depend on and which
 * would pull QtQuick in for four numbers (issue #83, build decision of the
 * strand). The file is an ordinary KConfig file, and the group is the one
 * `Plasma::Theme` documents.
 */
ContrastEffect contrastEffectOf(const QString &desktopTheme);

/**
 * The two text colours a desktop theme brings in its own `colors` file.
 *
 * Both invalid when the theme ships no such file — the measured majority: of
 * the eight themes on the user's machine four bring one and four do not
 * (measured, pre-check for #85). That is the fork of the user decision
 * of 04.08.2026: the writing comes from the same hand as the surface, and
 * where the theme keeps no hand of its own, the colour scheme keeps it.
 */
struct ThemeTextColours {
    QColor normal;
    QColor inactive;
};

/**
 * The `[Colors:Window]` group of a desktop theme's own `colors` file.
 *
 * Read ourselves and not through `KSvg::Svg::color()`, for one measured
 * reason: `KSvg::Svg::StyleSheetColor` has **no counterpart to
 * `ForegroundInactive`** (`KSvg/ksvg/svg.h`). The dimmed class has no KSvg
 * road, so it takes the theme's own KConfig file beside the graphic.
 *
 * `normal` is read all the same, and it is not the value the note text is
 * painted with — `KSvg::Svg::color(Text)` is. It says whether the theme brings
 * a hand of its own at all.
 */
ThemeTextColours themeTextColoursOf(const QString &desktopTheme);

/**
 * Whether this session can blur behind a window at all (issue #83, AK 7).
 *
 * Asked **before** the first registration, and therefore not through
 * `KWindowEffects::isEffectAvailable(BlurBehind)`: that one answers `false` in
 * the user's own session until we have registered once (measured,
 * pre-check for #83). A window built on that value would start out opaque in
 * exactly the session this story is for.
 *
 * Asked of KWin itself over D-Bus instead. A platform without a compositing
 * window server answers false without asking anybody.
 */
bool sessionBlursBehindWindows();

/**
 * The hull a frameless window of this application wears: rounding, outline,
 * shadow, blur and contrast, all of them out of the desktop theme.
 *
 * It exists because there are two such windows — the capture window of SPEC 3
 * and the recording window of SPEC 4, which SPEC 4 asks to be of the "same
 * make". Everything in here was measured once, in the capture window, and the
 * findings behind it fill four entries of the list in CLAUDE.md; a second
 * window with a hull of its own would have to earn every one of them again.
 *
 * **It is not a base class**, and that is deliberate: the two windows share
 * their outside and nothing of their inside. One grows with the text typed
 * into it and draws a second graphic for its entry field, the other counts
 * seconds. A common ancestor would have had to carry both.
 *
 * The window it is built on owns it and outlives it. Nothing here shows,
 * hides or resizes that window; the hull follows what the window does and is
 * told when to.
 */
class WindowHull : public QObject
{
    Q_OBJECT

public:
    /**
     * Builds the hull for `window`, which becomes its parent, and asks the
     * compositor once whether it blurs at all.
     *
     * It draws nothing yet: reload() is what reads the theme, and the window
     * calls it once everything that listens for changed() is connected.
     */
    explicit WindowHull(QWidget *window);
    ~WindowHull() override;

    /**
     * Whether the theme brings a hull graphic at all.
     *
     * Outside a Plasma session `dialogs/background` is simply not there
     * (SPEC 3.2 point 4). The window then wears no hull — and that is the
     * whole difference: it stays opaque, and it stays usable.
     */
    bool isValid() const;

    /**
     * The image set the hull is drawn from — for a window that hangs a graphic
     * of its own on the same theme. It is replaced on every reload(), so
     * whoever holds a frame on it re-points that frame in changed().
     */
    KSvg::ImageSet *imageSet() const;

    /** The strip the theme's graphic claims for itself, rounded to pixels. */
    QMargins margins() const;

    /** The colour the theme's own graphic writes normal text in. */
    QColor textColour() const;

    /** The theme's own two text colours, both invalid when it brings none. */
    ThemeTextColours themeTextColours() const;

    /**
     * Draws the hull at its current size, at the origin of `painter`.
     *
     * Also the road to the ground a text stands on: drawn into a picture of
     * the window's own colour, its pixels are what lies under the writing.
     */
    void paint(QPainter &painter) const;

    /**
     * Takes the window's current size and pixel ratio, and registers the blur
     * region anew — the region **is** the hull's mask, and the hull just
     * changed shape.
     */
    void resizeToWindow();

    /**
     * Hangs shadow and window effects on the native window.
     *
     * After show(), and after **every** show(): each appearance destroys the
     * Wayland surface and maps a fresh one (SPEC 3), and a shadow bound to the
     * old one is gone with it. The effects come immediately behind it for a
     * harder reason — measured over seven runs the blur takes hold only when
     * it is registered right after show(); registered a second later it does
     * nothing at all, and no return value says so.
     */
    void bindToWindow();

public Q_SLOTS:
    /**
     * Re-reads the desktop theme and rebuilds, on a standing window too.
     *
     * An empty `name` means the theme `plasmarc` names — the ordinary case,
     * and what the watch on that file calls. A name is passed by the picture
     * runner, which walks two desktop themes within one run (issue #83, AK 7).
     */
    void reload(const QString &name = {});

Q_SIGNALS:
    /**
     * The theme or the compositor's blur has changed and the hull is rebuilt.
     *
     * Emitted while the **previous** image set is still alive, so a window
     * that hangs frames of its own on imageSet() can move them over before the
     * old one goes — the other order leaves those frames pointing at freed
     * memory.
     */
    void changed();

private:
    void bindShadow();
    void bindWindowEffects();

    QWidget *m_window;
    KSharedConfig::Ptr m_plasmaConfig;
    /**
     * Replaced, never renamed: a FrameSvg does not follow its image set when
     * the set is given a new name — only a fresh set re-resolves it (measured,
     * see reload()). It has to outlive the frames that point at it.
     */
    std::unique_ptr<KSvg::ImageSet> m_imageSet;
    /** The hull, drawn in one piece out of the theme's own graphic. */
    KSvg::FrameSvg *m_hull;
    /** The `shadow` prefix of the same image: the tiles and their extents. */
    KSvg::FrameSvg *m_shadowTiles;
    std::unique_ptr<KWindowShadow> m_shadow;
    /** What the current desktop theme asks the compositor for. */
    ContrastEffect m_contrast;
    /**
     * The current desktop theme's own text colours, both invalid when it
     * brings none (issue #85). Kept beside `m_contrast` because it is read
     * from the same file kind at the same moment, and because the windows need
     * it on every palette change afterwards — the theme does not change when
     * the colour scheme does.
     */
    ThemeTextColours m_themeText;
    /**
     * Which variant of the theme's graphic the window draws — needed before
     * the first frame, and asked again whenever `kwinrc` changes (issue #93):
     * the blur can be switched off while the window already stands, and the
     * translucent variant over a compositor that no longer blurs is what
     * SPEC 3.2 item 4 promises not to happen.
     */
    bool m_blursBehind;
};

}
