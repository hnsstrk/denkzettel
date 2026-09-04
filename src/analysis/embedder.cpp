#include "analysis/embedder.h"

#include "analysis/aiprovider.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QtLogging>

#include <optional>

Embedder::Embedder(Store *store, AiProvider *provider, QObject *parent)
    : QObject(parent)
    , m_store(store)
    , m_provider(provider)
{
    reloadSettings();

    connect(m_provider,
            &AiProvider::embedFinished,
            this,
            [this](int id, const QList<double> &vector, const QString &error, AiFailure failure) {
        // Every call answers under an id of its own (AiProvider), and the
        // provider is shared: the connection test of SPEC 7.1 embeds over the
        // same object, and its answer is none of this run's business.
        if (!m_busy || id != m_requestId) {
            return;
        }
        m_requestId = -1;

        if (!error.isEmpty()) {
            // The one decision this class exists for, see the class comment: a
            // backend that never answered stops the run and costs nobody an
            // attempt, one that answered and refused costs this note one.
            if (failure == AiFailure::Unreachable) {
                stop(error);
            } else {
                fail(error);
            }
            return;
        }

        // A provider that answers without an error and without a vector: stored
        // as it comes, that would be a BLOB of nothing with the `needs_reembed`
        // flag cleared — a note that never clusters and is never asked about
        // again. OllamaProvider says so itself; this catches whoever does not.
        // It answered, so it is the note's attempt that is spent.
        if (vector.isEmpty()) {
            fail(i18n("The answer carried no embedding vector."));
            return;
        }

        // float32, the way SPEC 5.1 stores it. The provider hands doubles over
        // because that is what JSON carries.
        QList<float> components;
        components.reserve(vector.size());
        for (const double component : vector) {
            components.append(float(component));
        }

        // **Stored under the model it was sent with, not under the one standing
        // now.** reloadSettings() can land between the request and its answer,
        // and m_model read here would write a vector of the OLD model under the
        // NEW name — the very mixing this class re-reads the setting to avoid
        // (issue #119), and permanent, because notesToEmbed() sees a note that
        // already has a vector for the new name and never asks again. Measured:
        // with the setting changed 50 ms into a 300 ms call, embeddings(old)
        // came out 0 against 1.
        //
        // A database that will not take the vector is no fault of the note's
        // and none of the backend's, and the next note would meet it too.
        if (!m_store->setEmbedding(m_noteId, m_sentModel, m_sentService, components)) {
            stop(m_store->lastError());
            return;
        }

        Q_EMIT embedded(m_noteId);
        takeNextNote();
    });
}

void Embedder::start()
{
    if (m_busy) {
        return;
    }

    m_queue.clear();

    // **Asked before a note is taken out**, because taking one out is what
    // spends an attempt of SPEC 7.2 (aiprovider.h). A remote provider whose
    // embedding model nobody has filled in yet is a precondition not yet met,
    // and the run says so and takes nothing — the shape Classifier::start()
    // has for the chat model, down to the queue that is simply left empty so
    // that a run always ends the one way.
    const QString missing = m_provider->unmetEmbeddingPrecondition();
    Q_EMIT notReady(missing);

    const QList<Note> notes = missing.isEmpty() ? m_store->notesToEmbed(m_model, m_service) : QList<Note>();
    for (const Note &note : notes) {
        // What the counter of SPEC 7.2 skips is reported rather than passed
        // over — including after a restart, which is the only place the tray
        // could learn of a note that was given up on in an earlier run.
        if (note.analysisAttempts >= Store::analysisAttemptLimit) {
            Q_EMIT paused(note.id, note.analysisLastError);
            continue;
        }
        m_queue.append(note);
    }

    m_busy = true;
    takeNextNote();
}

bool Embedder::isBusy() const
{
    return m_busy;
}

QString Embedder::model() const
{
    return m_model;
}

QString Embedder::service() const
{
    return m_service;
}

void Embedder::reloadSettings()
{
    const QString model = m_provider->embeddingModel();
    const QString service = m_provider->serviceId();
    // Only a real change, and only away from a pair that was already set — the
    // constructor comes through here with both empty, and marking there would
    // re-embed a sound corpus at every start of the daemon (see the header).
    const bool changed = !m_model.isEmpty() && (model != m_model || service != m_service);
    m_model = model;
    m_service = service;
    if (changed && !m_store->markAllForReembedding()) {
        // Nothing else to do about it: the next run picks the notes up anyway,
        // because notesToEmbed() compares the pair as well. The flag is what
        // makes the switch readable in the database (SPEC 7.1).
        qWarning("The corpus could not be marked for re-embedding: %s", qUtf8Printable(m_store->lastError()));
    }
}

void Embedder::takeNextNote()
{
    if (m_queue.isEmpty()) {
        m_noteId = -1;
        m_busy = false;
        Q_EMIT finished();
        return;
    }

    const Note note = m_queue.takeFirst();
    m_noteId = note.id;
    m_sentModel = m_model;
    m_sentService = m_service;
    m_requestId = m_provider->embed(note.content);
}

void Embedder::fail(const QString &reason)
{
    const qint64 noteId = m_noteId;
    // The count comes out of the database and not out of the note this run
    // read at its start, for the reason Classifier::fail() reads it back: it is
    // what has to survive a restart (SPEC 7.2). The columns are the
    // classification's, and so is the reset — a note that is classified again
    // starts over on both steps (Store::completeAnalysis).
    const std::optional<int> attempts = m_store->failAnalysis(noteId, reason);

    if (attempts.has_value() && *attempts >= Store::analysisAttemptLimit) {
        Q_EMIT paused(noteId, reason);
    } else {
        Q_EMIT failed(noteId, reason);
    }

    takeNextNote();
}

void Embedder::stop(const QString &reason)
{
    const qint64 noteId = m_noteId;
    m_queue.clear();
    m_noteId = -1;
    m_busy = false;
    Q_EMIT failed(noteId, reason);
    Q_EMIT finished();
}
