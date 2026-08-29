#pragma once

#include <KConfigDialog>

/**
 * The settings of SPEC 13 — a page list, one page per subject, and the button
 * row KConfigDialog brings with it.
 *
 * **Every page is a class of its own in a file of its own, and this dialog
 * adds it in exactly one line.** That is not a matter of taste and not an
 * abstraction laid in for later: four further stories build into this same
 * dialog — the pages "Export" (#75), "Voice notes" (#27) and "Shortcuts"
 * (#74), and the API key out of KWallet (#37) — and they are built in separate
 * working trees at the same time. Written into one file they would meet as
 * four merge conflicts instead of four independent stories.
 *
 * Three things about it were measured on 29.08.2026 and are not to be
 * rediscovered (issue #16):
 *
 * - **The Help button is hidden, never replaced.** `setStandardButtons()`
 *   after the constructor leaves an Apply button that looks right and writes
 *   nothing — KConfigDialog wires its buttons up in its own constructor, and
 *   exchanging the set cuts that wiring with nothing reporting it.
 * - **No window title of its own.** KConfigDialog sets "Configure" ("Einrichten"
 *   in German) by itself, and the decoration appends the application name — a
 *   title here would read "Denkzettel — Settings — Denkzettel"
 *   (librarywindow.cpp:198–202 holds the measurement).
 * - **Icons in the page list are load-bearing.** KPageDialog::List keeps the
 *   height of an icon free whether one is there or not, so entries without one
 *   stand with holes between them.
 */
class SettingsDialog : public KConfigDialog
{
    Q_OBJECT

public:
    /**
     * Brings the dialog up — the standing one if there is one, a new one
     * otherwise (one per session).
     *
     * Free-standing and without a parent window on purpose: what opens it is
     * the tray menu, and that has no window; hung on the library it would hang
     * on one of several equal-ranking roads (SPEC 2.1).
     */
    static void showSettings();

public Q_SLOTS:
    /**
     * Where the window size is written down, and it has to be here rather than
     * in closeEvent(): `QDialog::done()` sends no QCloseEvent, so OK, Cancel
     * and Esc would all lose the size. Measured 29.08.2026 — over close() the
     * group held 900 × 700 and the next opening came up at 900 × 700, over OK
     * the group stayed empty and the next opening fell back to 640 × 480.
     * close() runs through QDialog::closeEvent → reject() → done(), so this
     * one place covers every way out.
     */
    void done(int result) override;

private:
    SettingsDialog();
};
