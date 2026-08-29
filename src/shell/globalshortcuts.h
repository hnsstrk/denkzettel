#pragma once

#include "shell/shortcutconflict.h"

#include <QKeySequence>
#include <QObject>

#include <cstdint>

class QAction;

/**
 * The two global shortcuts of SPEC 2.4 — Meta+N for the capture window and
 * Meta+Shift+N for the recorder.
 *
 * KGlobalAccel identifies an application by its desktop file; the Plasma
 * shortcut settings resolve name and icon through that file, so both have to
 * match.
 *
 * **Two roads lead in here, and they are not the same road.** The start
 * registers Meta+N with autoloading semantics, which is deliberately the road
 * that does *not* overwrite what the user has set. The settings page of SPEC 13
 * takes the other one, changeSequence(), which writes without autoloading and
 * reads back what the service kept (issue #74).
 */
class GlobalShortcuts : public QObject
{
    Q_OBJECT

public:
    /** The two shortcuts, in the order the settings page lists them. */
    enum class Shortcut : std::uint8_t {
        Capture, //< Meta+N, shows the capture window
        Recorder, //< Meta+Shift+N, shows the recorder
    };

    explicit GlobalShortcuts(QObject *parent = nullptr);

    /**
     * Registers the default sequence of `which` and returns the components
     * that already hold it. Registration happens even then: SPEC 2.4 asks for
     * visibility, not for giving up, and the tray entries keep working
     * regardless.
     *
     * Afterwards the registration is read back from the daemon. Did it not
     * arrive, that is reported here — at every start, not only the first one —
     * and the list comes back empty: there is no shortcut left to be in
     * conflict with.
     *
     * **Per shortcut, and SPEC 2.4 says so in as many words:** Meta+Shift+N
     * goes through the same read-back as Meta+N. A pair registered together
     * and reported together would hide which of the two never arrived, and
     * "visible in the system settings" is precisely the state a silently
     * failed shortcut produces.
     */
    QList<ShortcutOwner> registerShortcut(Shortcut which);

    /**
     * The label the same action carries in the tray menu. The settings page
     * writes it in front of the input field, and the message about a failed
     * readback names it — one action, one wording, everywhere (SPEC 13).
     */
    static QString label(Shortcut which);

    /** What SPEC 2.4 lays down, and what "Restore defaults" goes back to. */
    static QKeySequence defaultSequence(Shortcut which);

    /**
     * The component name the shortcut service files us under. The settings page
     * hands it to its input field: without it our own registered sequence comes
     * back out of the conflict check as a conflict with itself.
     */
    static QString shortcutComponent();

    /**
     * What the shortcut service holds for `which` right now, empty if it holds
     * nothing. Asked of the service and not of the QAction: what the process
     * once sent off is not what the service kept (SPEC 2.4, retro B5).
     */
    static QKeySequence assignedSequence(Shortcut which);

    /**
     * Writes `sequence` for `which` and returns **what the service holds
     * afterwards**, which is the only thing that says whether it arrived.
     *
     * The return value of KGlobalAccel is not looked at, and the reason is
     * measured: `setGlobalShortcut()` reports success without reading the
     * answer, and setting a shortcut as a *default* (the IsDefault flag) hands
     * back the keys it was given while the active shortcut stays empty. So the
     * road here is setShortcut() with NoAutoloading — that one carries the
     * SetPresent flag, and SetPresent is what a key press finds — and the
     * answer to "did it work" is read back out of the service.
     *
     * NoAutoloading is what makes it a change at all: with autoloading the
     * service hands back the sequence it already has and the new one is
     * dropped.
     */
    QKeySequence changeSequence(Shortcut which, const QKeySequence &sequence);

Q_SIGNALS:
    void captureRequested();
    void recorderRequested();

private:
    QAction *actionFor(Shortcut which) const;

    QAction *m_captureAction;
    // Beside the capture action, because this is where the identity of a
    // shortcut lives — component, action id and label together.
    QAction *m_recorderAction;
};

/**
 * Tells the user which components hold `sequence` (SPEC 2.4, 14). The sequence
 * is handed in rather than spelled out: since the settings page of SPEC 13 it
 * is whatever the user set, and a message naming Meta+N would name the wrong
 * key.
 */
void notifyShortcutConflict(const QKeySequence &sequence, const QList<ShortcutOwner> &conflicts);
