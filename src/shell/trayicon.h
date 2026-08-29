#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class KStatusNotifierItem;
class QDialog;
class QMenu;

/**
 * Permanent tray presence of the daemon (SPEC 10).
 */
class TrayIcon : public QObject
{
    Q_OBJECT

public:
    explicit TrayIcon(QObject *parent = nullptr);

    /**
     * The tray item, for reading only: what the item announces to the host is
     * only knowable by asking the item itself (issue #44). Const because
     * everything the item is told is decided in the constructor.
     */
    const KStatusNotifierItem *item() const;

public Q_SLOTS:
    /**
     * Shows the error state of SPEC 10 with `reason` in the tooltip, or takes
     * it back when `reason` is empty (issue #24).
     *
     * `NeedsAttention` and no badge of our own: that is the state the tray
     * protocol has for exactly this, and Plasma is what decides how loudly the
     * icon is set apart (customer decision of 29.08.2026, against the quieter
     * overlay). It is the only state beside the normal one — a "transcribing
     * right now" does not exist and is not wanted (SPEC 14: the tray is quiet,
     * the log is detailed).
     */
    void setTranscriptionError(const QString &reason);

Q_SIGNALS:
    void captureRequested();
    void libraryRequested();

private:
    QMenu *buildMenu();

    KStatusNotifierItem *m_item;
    /**
     * The open about dialog, or nothing.
     *
     * It deletes itself on close, so the pointer has to notice that by itself;
     * a raw one would be dangling the second time the entry is used.
     */
    QPointer<QDialog> m_about;
};
