#include "ui/pendingdeletion.h"

#include "store/store.h"

#include <utility>

PendingDeletion::PendingDeletion(Store *store, int gracePeriodSeconds, QObject *parent)
    : QObject(parent)
    , m_store(store)
    , m_gracePeriodSeconds(gracePeriodSeconds)
{
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &PendingDeletion::tick);
}

void PendingDeletion::request(qint64 noteId)
{
    // The new request takes over before the earlier one is carried out, so
    // isPending() is true throughout: the window keeps one standing message
    // instead of hiding and re-showing it (wireframe 2c, undo edges).
    const qint64 earlier = std::exchange(m_noteId, noteId);
    m_remainingSeconds = m_gracePeriodSeconds;
    m_timer.start();

    if (earlier >= 0) {
        carryOut(earlier);
    }

    Q_EMIT remainingChanged(m_remainingSeconds);
}

void PendingDeletion::flush()
{
    if (isPending()) {
        commit();
    }
}

void PendingDeletion::undo()
{
    if (!isPending()) {
        return;
    }

    m_timer.stop();
    const qint64 noteId = std::exchange(m_noteId, qint64(-1));

    Q_EMIT reverted(noteId);
}

bool PendingDeletion::isPending() const
{
    return m_noteId >= 0;
}

void PendingDeletion::tick()
{
    --m_remainingSeconds;
    if (m_remainingSeconds <= 0) {
        commit();
        return;
    }

    Q_EMIT remainingChanged(m_remainingSeconds);
}

void PendingDeletion::commit()
{
    m_timer.stop();
    carryOut(std::exchange(m_noteId, qint64(-1)));
}

void PendingDeletion::carryOut(qint64 noteId)
{
    if (!m_store->removeNote(noteId)) {
        // The note stays; the list no longer shows it. Saying so is all this
        // layer can do — the next time the library opens, it reads the store
        // again and the note is back.
        qWarning("Deleting the note %lld failed: %s", noteId, qPrintable(m_store->lastError()));
    }

    Q_EMIT committed(noteId);
}
