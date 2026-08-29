#pragma once

#include "capture/windowhull.h"

#include <QColor>
#include <QList>
#include <QWidget>

class Store;
class QLabel;
class QPlainTextEdit;

namespace KSvg
{
class FrameSvg;
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

    /** Everything that has to follow a rebuilt hull, in the order it has to. */
    void applyHull();
    void resizeField();
    void applyHullMargins();
    void applyFieldMargin();
    QColor fieldSurfaceOver(const QColor &backdrop) const;
    void applyTextColours();

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

    /**
     * Rounding, outline, shadow, blur and the theme's own colours — shared
     * with the recording window of SPEC 4, which wears the same hull.
     */
    capture::WindowHull *m_hull;
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
};
