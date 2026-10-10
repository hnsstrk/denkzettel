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
 *   queue: one provider answers every embedding of a run (SPEC 7.1), so the
 *   next note fares the same. The run ends and **no counter moves**. Counted,
 *   two outages would leave the whole corpus given up on for good; carried on
 *   with, the run spends a timeout per note to be told the same thing fifty
 *   times. This is what SPEC 7.1 means by the bundles falling away while the
 *   provider is unreachable: the classification keeps working, the vectors
 *   wait.
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
     * Whether the last run broke off because the embedding backend did not
     * answer — the state unreachable() raises and embedded() takes back.
     *
     * **Kept here because two displays read it and neither of them is
     * running when it comes about** (issue #153). The tray is told by the
     * signal, which is enough for a window that stands all session; the
     * settings page of SPEC 7.1 is built when the user opens the dialog, and
     * by then the signal is long gone. Asked here it answers for a page that
     * was not there yet, and SPEC 7.1 owes the hint to "the settings **and**
     * the tray tooltip".
     *
     * **No reason with it.** What the user is owed on either channel is what
     * they are not getting: this state comes up for a service that went
     * silent, for a password store that would not open and for a key that was
     * never entered, so a sentence built out of a cause would be wrong for
     * two of the three (unreachable()).
     *
     * It lives in this object and not in the store, so a restart clears it and
     * the next run that breaks off brings it back — which is right, because
     * what it reports is how the machine stands at this moment and not
     * something that happened to a note.
     */
    bool isUnreachable() const;

    /**
     * The embedding model of SPEC 7.1, as reloadSettings() last read it off the
     * provider.
     *
     * It is written beside every vector and it is what the clustering asks the
     * store for (Store::embeddings()) — so whoever clusters what this run
     * wrote takes the name from here rather than reading a setting a second
     * time. Which key holds it depends on which backend is chosen, and the
     * backend is the one that knows: two readers would be two chances to open
     * the wrong key.
     */
    QString model() const;

    /**
     * The service the vectors of this run come from, kept beside the model name
     * (Store::setEmbedding(), issue #130).
     *
     * The same name exists on two services — `bge-m3` is an Ollama model and
     * `baai/bge-m3` stands in openrouter's list, and the model field is a free
     * one — so since the provider choice opened, the name alone stopped being
     * an identifier.
     */
    QString service() const;

public Q_SLOTS:
    /**
     * Takes the model and the service off the provider again.
     *
     * It hangs on the same `Settings::configChanged` as the provider's own
     * reloadSettings() and for the same reason (issue #119): the provider would
     * otherwise ask a **new** model for the vector while this class went on
     * writing the **old** name beside it, and the clustering, which looks the
     * vectors up by that name, would compare two models' vectors as if they
     * were one. **It has to run after the provider's**, which is what the order
     * of the connections in settingswiring.cpp gives it.
     *
     * **A change marks the whole corpus for re-embedding** (SPEC 7.1, PO
     * decision 04.09.2026): vectors of two models are not comparable, so the
     * flag of SPEC 9 goes on every note and the next runs work it off within
     * the budget of §14 — at most 50 a run, nothing in bulk and nothing
     * refused. Only a **change**, and only from a pair that was already set:
     * marked at construction too, every start of the daemon would embed a
     * corpus again that has perfectly good vectors, unattended and on a billed
     * service.
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

    /**
     * The run took no note at all, because the backend cannot embed yet — the
     * sentence says what is missing
     * (AiProvider::unmetEmbeddingPrecondition()).
     *
     * Classifier::notReady()'s counterpart, and for the same rule of SPEC 12: a
     * precondition not yet met is not a failed attempt. Without it a remote
     * provider whose embedding model the user has not filled in yet would spend
     * both attempts of every note on that sentence, every 30 minutes. Empty
     * takes the report back.
     */
    void notReady(const QString &reason);

    /**
     * The run stopped because the embedding backend did not answer, so no new
     * vector came into being and the topic bundles of SPEC 7.3 stand still.
     *
     * **Why this is a channel of its own and not `failed()`.** SPEC 7.1 owes
     * the user a visible degradation for exactly this case — "classification
     * through the chosen provider keeps working, topic bundles are dropped
     * (hint in the settings and the tray tooltip)" — and `failed()` cannot
     * carry it: it is emitted for a refusal as well, which costs the note an
     * attempt and says nothing about the service being gone.
     *
     * **Emitted on the `AiFailure::Unreachable` branch and not in stop()**,
     * although stop() is what both of them call: a database that refuses the
     * vector ends the run through stop() too, and a sentence about the
     * embedding would then stand over an SQLite write error.
     *
     * It is **not** the unmet precondition either: an embedding model the user
     * has not filled in yet is notReady()'s business and is no outage
     * (SPEC 12's rule). What does come through here beside a silent service is
     * a password store that would not open and a key that was never entered
     * (OpenAiCompatibleProvider::releaseWaiting()) — all of them mean the same
     * thing to the user, that no bundle is coming, which is why the sentence
     * the tray shows names **that** and not a cause.
     *
     * Emitted once per stopped run. The state it raises is kept in
     * isUnreachable() above, because the settings page of SPEC 7.1 is built
     * after the signal has gone (issue #153); a restart clears it and the next
     * run that breaks off brings it back — unlike the two counts of issue #118
     * beside it, which are read out of the store at start.
     */
    void unreachable(const QString &reason);

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
    /** The service the model above belongs to (issue #130). */
    QString m_service;
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
    /** The service the outstanding request went to, for the reason above. */
    QString m_sentService;
    /** The notes of this run that are still outstanding, oldest first. */
    QList<Note> m_queue;
    /** The note being embedded, and -1 between two of them. */
    qint64 m_noteId = -1;
    /** The id the answer being waited for arrives under (AiProvider). */
    int m_requestId = -1;
    bool m_busy = false;
    /** The state isUnreachable() answers with. */
    bool m_unreachable = false;
};
