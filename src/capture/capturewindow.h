#pragma once

#include <KSharedConfig>

#include <QWidget>

#include <memory>

class KWindowShadow;
class Store;
class QPlainTextEdit;

namespace KSvg
{
class FrameSvg;
class ImageSet;
}

/**
 * Frameless window for capturing a text note (SPEC 3).
 *
 * It lives hidden inside the daemon so that it can appear without a process
 * start. Placement is left to KWin (PO decision after the T1 spike); a Wayland
 * client cannot position itself anyway.
 *
 * The window wears the hull of the desktop theme — rounding, outline and
 * shadow (issue #55). Form comes from the theme, colour from the palette:
 * of the eight desktop themes installed on the customer's machine only
 * `default` adjusts its fill to the colour scheme, so painting the theme's own
 * pixels would put dark text on a dark surface under seven of them
 * (wireframe 4a, measured 01.08.2026).
 */
class CaptureWindow : public QWidget
{
    Q_OBJECT

public:
    /** `store` outlives the window and is not owned by it. */
    explicit CaptureWindow(Store *store, QWidget *parent = nullptr);
    ~CaptureWindow() override;

    /**
     * The shadow handed to the compositor, or nullptr if the desktop theme
     * brought no shadow tiles.
     *
     * This is the named substitute for a picture (issue #55, AK 7): offscreen
     * `KWindowShadow::create()` always fails — there is no compositor to take
     * the tiles — and `QWidget::grab()` would not show a shadow either, because
     * the shadow lies outside the widget. What can be shown is the object and
     * where its tiles come from.
     */
    const KWindowShadow *shadow() const;

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
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void present();
    void save();
    void discard();
    void adjustHeight();

    void resizeHull();
    void applyHullMargins();
    void applyTextColours();
    void bindShadow();

    Store *m_store;
    QPlainTextEdit *m_text;

    KSharedConfig::Ptr m_plasmaConfig;
    /**
     * Replaced, never renamed: a FrameSvg does not follow its image set when
     * the set is given a new name — only a fresh set re-resolves it (measured,
     * see reloadDesktopTheme()). It has to outlive the frames that point at it.
     */
    std::unique_ptr<KSvg::ImageSet> m_imageSet;
    /** The hull itself, and the same hull one outline width smaller. */
    KSvg::FrameSvg *m_hull;
    KSvg::FrameSvg *m_hullInner;
    /** The `shadow` prefix of the same image: the tiles and their extents. */
    KSvg::FrameSvg *m_shadowTiles;
    std::unique_ptr<KWindowShadow> m_shadow;
};
