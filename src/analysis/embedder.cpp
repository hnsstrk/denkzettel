#include "analysis/embedder.h"

#include "analysis/aiprovider.h"
#include "analysis/ollamaprovider.h"
#include "store/store.h"

#include <KLocalizedString>

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
        if (!m_store->setEmbedding(m_noteId, m_sentModel, components)) {
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
    const QList<Note> notes = m_store->notesToEmbed(m_model);
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

void Embedder::reloadSettings()
{
    m_model = ollama::configuredEmbeddingModel();
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
