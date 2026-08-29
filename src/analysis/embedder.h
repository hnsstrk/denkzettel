#pragma once

#include "store/note.h"

#include <QList>
#include <QObject>
#include <QString>

class AiProvider;
class Store;

/**
 * Step 2 of the analysis run of SPEC 7.2: one `embed` call per note, the vector
 * into the `embeddings` table, the `needs_reembed` flag cleared with it.
 *
 * Built like the classification run beside it — one note at a time, the queue
 * read once at the start, the failures counted persistently, nothing reported
 * to the user directly. What differs is which failures are counted, and the
 * line runs between the two kinds AiFailure names:
 *
 * - **The backend never answered** — a timeout, a refused connection, a
 *   transport error. That says nothing about the note that was first in the
 *   queue: every embedding of v1 comes from the one Ollama (SPEC 7.1), so the
 *   next note fares the same. The run ends and **no counter moves**. Counted,
 *   two outages would leave the whole corpus given up on for good; carried on
 *   with, the run spends a timeout per note to be told the same thing fifty
 *   times. This is what SPEC 7.1 means by the bundles falling away while
 *   Ollama is unreachable: the classification keeps working, the vectors wait.
 * - **The backend answered and refused this text** — an HTTP error, an
 *   unreadable body, an answer without a vector in it. It answered, so it is
 *   there; what it choked on is this note. That one is counted the way
 *   Classifier::fail() counts, and after the second attempt the note is skipped
 *   and reported (SPEC 7.2, 14).
 *
 * **The distinction is the difference between a pause and a lock-up**, and it
 * was measured: with the run ended on every failure, one note that no backend
 * will ever embed stopped the whole corpus for good — three runs, three
 * attempts on the same note, the two healthy ones never asked once, no vector
 * written. That is the endless retrying SPEC 7.2 rules out, and nobody would
 * ever have seen it.
 *
 * **It starts nothing by itself**, like the classification: when a run happens
 * and how many notes it may take is the trigger and the budget of SPEC 7.2/14
 * (#15).
 */
class Embedder : public QObject
{
    Q_OBJECT

public:
    /** Neither `store` nor `provider` is owned; both outlive the embedder. */
    Embedder(Store *store, AiProvider *provider, QObject *parent = nullptr);

    /**
     * Takes up the notes that need a vector and returns at once — the work runs
     * in the event loop. Calling it while a run is going does nothing.
     */
    void start();

    /** Whether a note is being embedded right now. */
    bool isBusy() const;

    /**
     * The embedding model of SPEC 7.1 out of `denkzettelrc`, as reloadSettings()
     * last read it.
     *
     * It is written beside every vector and it is what the clustering asks the
     * store for (Store::embeddings()) — so whoever clusters what this run
     * wrote takes the name from here rather than reading the setting a second
     * time. The same key OllamaProvider reads, for the same reason: two
     * spellings would be two models.
     */
    QString model() const;

public Q_SLOTS:
    /**
     * Re-reads `[AI] EmbeddingModel` out of `denkzettelrc`.
     *
     * It hangs on the same `Settings::configChanged` as
     * OllamaProvider::reloadSettings() and for the same reason (issue #119):
     * the provider would otherwise ask a **new** model for the vector while
     * this class went on writing the **old** name beside it, and the
     * clustering, which looks the vectors up by that name, would compare two
     * models' vectors as if they were one.
     *
     * **It takes hold from the next request on, and a call already on its way
     * keeps the name it was sent with** — see m_sentModel. Not "a run that is
     * already going keeps what it started with": the next note of the same run
     * is asked with the new model and stored under it, which is what
     * notesToEmbed() then goes by.
     */
    void reloadSettings();

Q_SIGNALS:
    /** The note carries its vector, and its `needs_reembed` is cleared. */
    void embedded(qint64 noteId);

    /**
     * The attempt for this note failed.
     *
     * One per attempt, whether it was counted against the note or not — what
     * ended the run is told apart by paused() below, not by this.
     */
    void failed(qint64 noteId, const QString &reason);

    /**
     * The note is skipped: the two attempts of SPEC 7.2 are used up on
     * refusals, and it is not handed to the backend again until a
     * classification resets its counter. That is what SPEC 14 asks to be
     * reported.
     */
    void paused(qint64 noteId, const QString &reason);

    /** The run is through and nothing is outstanding. */
    void finished();

private:
    void takeNextNote();
    /** Counts the refusal against the note, reports it, and goes on. */
    void fail(const QString &reason);
    /** Reports the failure and ends the run, counting nothing. */
    void stop(const QString &reason);

    Store *m_store;
    AiProvider *m_provider;
    QString m_model;
    /**
     * The model the outstanding request was sent with, and what its vector is
     * stored under.
     *
     * m_model can change between the request and the answer (reloadSettings()),
     * and a vector belongs to the model that made it. Written under the new
     * name it would be a vector of two models under one, and notesToEmbed()
     * would never ask for that row again.
     */
    QString m_sentModel;
    /** The notes of this run that are still outstanding, oldest first. */
    QList<Note> m_queue;
    /** The note being embedded, and -1 between two of them. */
    qint64 m_noteId = -1;
    /** The id the answer being waited for arrives under (AiProvider). */
    int m_requestId = -1;
    bool m_busy = false;
};
