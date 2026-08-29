#pragma once

#include "shell/globalshortcuts.h"

#include <QWidget>

#include <array>

class GlobalShortcuts;
class KKeySequenceWidget;
class QLabel;

/**
 * The page "Shortcuts" of SPEC 13: both global shortcuts of SPEC 2.4, each in
 * a KKeySequenceWidget, with one small coloured line underneath.
 *
 * **Nothing here is a `kcfg_` widget**, and that is not an oversight: a global
 * shortcut lives in the shortcut service, not in `denkzettelrc`, so the
 * dialog's manager has nothing to load, save or grey out. The dialog therefore
 * routes its own hasChanged(), isDefault(), updateSettings(), updateWidgets()
 * and updateWidgetsDefault() into this page — that is the road KConfigDialog
 * documents for widgets it does not manage.
 *
 * **The conflict of SPEC 2.4 is not reported here.** KKeySequenceWidget brings
 * that check with it — `setCheckForConflictsAgainst(StandardShortcuts |
 * GlobalShortcuts)`, which its own header names as the combination for a global
 * shortcut — and asks, translated and modally, over
 * KGlobalAccel::promptStealShortcutSystemwide. A message of ours beside it
 * would be a second truth. What the coloured line carries is the other half of
 * SPEC 2.4: the **readback**, which falls due on Apply/OK and not while typing.
 *
 * ponytail: the second row writes a shortcut that nothing reacts to yet.
 * Ceiling: ShowRecorder() and the desktop action a key press starts arrive with
 * issue #21, and until then Meta+Shift+N is set and does nothing. Upgrade path:
 * #21 — no line here changes with it.
 */
class ShortcutsPage : public QWidget
{
    Q_OBJECT

public:
    /** `shortcuts` outlives the dialog and is not owned by it. */
    explicit ShortcutsPage(GlobalShortcuts *shortcuts, QWidget *parent = nullptr);

    /** Whether a field shows something else than the service holds. */
    bool hasChanged() const;

    /** Whether both fields show what SPEC 2.4 lays down. */
    bool isDefault() const;

    /**
     * Whether the last save() had something to report — and it answers `true`
     * only **once**. The dialog asks before it closes on OK: a message the user
     * never gets to read is none, so the first OK is refused and leaves the red
     * line standing. The second one closes, because refusing for good would
     * make a dialog nobody can leave; what the fields then show is what the
     * service really holds, so nothing is lost by going.
     */
    bool takeReadbackFailure();

public Q_SLOTS:
    /** Fills both fields with what the shortcut service holds. */
    void load();

    /** Fills both fields with the defaults of SPEC 2.4. */
    void loadDefaults();

    /**
     * Writes every field that differs from what the service holds, reads the
     * result back, and on a difference puts the field back onto the value the
     * service really holds instead of leaving the wish standing.
     */
    void save();

Q_SIGNALS:
    /** A field was changed — the dialog re-reads its buttons on it. */
    void changed();

private:
    struct Row {
        GlobalShortcuts::Shortcut which;
        KKeySequenceWidget *editor;
    };

    void report(const QStringList &failures);

    GlobalShortcuts *m_shortcuts;
    std::array<Row, 2> m_rows;
    QLabel *m_readback;
    bool m_readbackFailed = false;
};
