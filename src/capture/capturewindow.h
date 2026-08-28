#pragma once

#include <KSharedConfig>

#include <QColor>
#include <QList>
#include <QWidget>

#include <memory>

class KWindowShadow;
class Store;
class QLabel;
class QPlainTextEdit;

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

}

/**
 * Frameless window for capturing a text note (SPEC 3).
 *
 * It lives hidden inside the daemon so that it can appear without a process
 * start. Placement is left to KWin (design decision after the T1 spike); a Wayland
 * client cannot position itself anyway.
 *
 * The window wears the hull of the desktop theme — rounding, outline and
 * shadow (issue #55), drawn the way Plasma draws its own overlays: the graphic
 * of `dialogs/background` in one piece, plus the two effect registrations
 * `libPlasmaQuick` makes beside the shadow (issue #83). Nothing of the hull is
 * ours any more; what the theme does not draw, the window does not wear.
 *
 * The text field is drawn the same way, out of `widgets/lineedit` (issue #100)
 * — the graphic KRunner's entry field comes from, on the same image set one
 * layer down. The same sentence holds for it: where the theme draws nothing,
 * nothing is there.
 *
 * The field carries two states of that graphic and not one (issue #102): the
 * resting `base`, and the `focus` layer on top of it while the window is the
 * active one. That is Plasma's own layering — the graphic names it
 * `hint-focus-over-base` — and its other half is the honest one: a window that
 * has not got the keyboard shows no focus edge.
 */
class CaptureWindow : public QWidget
{
    Q_OBJECT

public:
    /** `store` outlives the window and is not owned by it. */
    explicit CaptureWindow(Store *store, QWidget *parent = nullptr);
    ~CaptureWindow() override;

public Q_SLOTS:
    /** Brings the window up with the keyboard focus, empty and ready. */
    void showCapture();

    /**
     * Re-reads the desktop theme and rebuilds the hull on a standing window.
     *
     * An empty `name` means the theme `plasmarc` names — the ordinary case,
     * and what the watch on that file calls. A name is passed by the picture
     * runner, which walks two desktop themes within one run (AK 7).
     */
    void reloadDesktopTheme(const QString &name = {});

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void present();
    void save();
    void discard();
    void adjustHeight();

    void resizeHull();
    void resizeField();
    void applyHullMargins();
    void applyFieldMargin();
    QColor fieldSurfaceOver(const QColor &backdrop) const;
    void applyTextColours();
    void bindShadow();
    void bindWindowEffects();

    Store *m_store;
    QPlainTextEdit *m_text;
    /**
     * The application name, the heading of the window (SPEC 3.1).
     *
     * Its own member and not in the list below, because it carries the note
     * text's colour and not the dimmed one (issue #84).
     */
    QLabel *m_appName = nullptr;
    /**
     * The key hint below the field (SPEC 3.1).
     *
     * Held rather than looked up: since #85 its colour can come from the
     * desktop theme, and a colour has to be written onto the widget where a
     * role resolved itself.
     */
    QList<QLabel *> m_subtleLabels;

    KSharedConfig::Ptr m_plasmaConfig;
    /**
     * Replaced, never renamed: a FrameSvg does not follow its image set when
     * the set is given a new name — only a fresh set re-resolves it (measured,
     * see reloadDesktopTheme()). It has to outlive the frames that point at it.
     */
    std::unique_ptr<KSvg::ImageSet> m_imageSet;
    /** The hull, drawn in one piece out of the theme's own graphic. */
    KSvg::FrameSvg *m_hull;
    /** The `shadow` prefix of the same image: the tiles and their extents. */
    KSvg::FrameSvg *m_shadowTiles;
    /**
     * The text field, out of the theme's `widgets/lineedit` (issue #100).
     *
     * A third frame on the same image set — the user's finding of
     * 05.08.2026 was that the window shows no entry area at all, and the answer
     * is the one KRunner gives: the theme's own graphic, one layer below the
     * hull. It is drawn on the geometry of the text area, so it has to be
     * resized whenever that is.
     */
    KSvg::FrameSvg *m_field;
    /**
     * The focus state of the same graphic, `focus` over `base` (issue #102).
     *
     * A fourth frame rather than a second prefix on `m_field`: both are drawn
     * in the same paintEvent(), one over the other, and a frame carries one
     * prefix at a time. It claims no border of its own — measured 0,1 px under
     * all eight installed themes against 6 px for `base` — so no inner spacing
     * of #100 hangs on it.
     */
    KSvg::FrameSvg *m_focus;
    std::unique_ptr<KWindowShadow> m_shadow;
    /** What the current desktop theme asks the compositor for (AK 6). */
    capture::ContrastEffect m_contrast;
    /**
     * The current desktop theme's own text colours, both invalid when it
     * brings none (issue #85). Kept beside `m_contrast` because it is read
     * from the same file kind at the same moment, and because
     * `applyTextColours()` needs it on every palette change afterwards — the
     * theme does not change when the colour scheme does.
     */
    capture::ThemeTextColours m_themeText;
    /**
     * Which variant of the theme's graphic the window draws — needed before
     * the first frame, and asked again whenever `kwinrc` changes (issue #93):
     * the blur can be switched off while this window already stands, and the
     * translucent variant over a compositor that no longer blurs is what
     * SPEC 3.2 item 4 promises not to happen.
     */
    bool m_blursBehind;
};
