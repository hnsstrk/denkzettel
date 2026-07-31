#pragma once

#include <QObject>
#include <QTimer>

class Store;

/**
 * The five-second grace period of the library's delete action (SPEC 9).
 *
 * Deleting is delayed on the client side only — the note stays in the store
 * until the period runs out, so taking the deletion back means forgetting the
 * request rather than restoring anything. There is never more than one pending
 * deletion: a second request carries out the first one at once.
 */
class PendingDeletion : public QObject
{
    Q_OBJECT

public:
    /** SPEC 9 fixes the period; the tests are what shortens it. */
    static constexpr int DefaultGracePeriodSeconds = 5;

    explicit PendingDeletion(Store *store,
                             int gracePeriodSeconds = DefaultGracePeriodSeconds,
                             QObject *parent = nullptr);

    /** Asks for `noteId` to be deleted once the grace period has passed. */
    void request(qint64 noteId);

    /** Carries out a pending deletion now — the window is closing. */
    void flush();

    /** Drops a pending deletion; the note was never touched. */
    void undo();

    bool isPending() const;

Q_SIGNALS:
    /** Seconds left of the period, from the full period down to one. */
    void remainingChanged(int seconds);

    /** The note has left the store. */
    void committed(qint64 noteId);

    /** The deletion was taken back in time. */
    void reverted(qint64 noteId);

private:
    void tick();
    void commit();

    Store *m_store;
    int m_gracePeriodSeconds;
    qint64 m_noteId = -1;
    int m_remainingSeconds = 0;
    QTimer m_timer;
};
