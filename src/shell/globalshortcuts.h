#pragma once

#include "shell/shortcutconflict.h"

#include <QObject>

class QAction;

/**
 * The global shortcut Meta+N for the capture window (SPEC 2.4).
 *
 * KGlobalAccel identifies an application by its desktop file; the Plasma
 * shortcut settings resolve name and icon through that file, so both have to
 * match. Meta+Umschalt+N for the recorder follows with M4, which is when
 * ShowRecorder() exists.
 */
class GlobalShortcuts : public QObject
{
    Q_OBJECT

public:
    explicit GlobalShortcuts(QObject *parent = nullptr);

    /**
     * Registers Meta+N and returns the components that already hold the
     * sequence. Registration happens even then: SPEC 2.4 asks for visibility,
     * not for giving up, and the tray entry keeps working regardless.
     *
     * Afterwards the registration is read back from the daemon. Did it not
     * arrive, that is reported here — at every start, not only the first one —
     * and the list comes back empty: there is no shortcut left to be in
     * conflict with.
     */
    QList<ShortcutOwner> registerCaptureShortcut();

Q_SIGNALS:
    void captureRequested();

private:
    QAction *m_captureAction;
};

/** Tells the user which components hold the shortcut (SPEC 2.4, 14). */
void notifyShortcutConflict(const QList<ShortcutOwner> &conflicts);
