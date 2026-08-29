#pragma once

#include "store/note.h"

#include <QList>
#include <QObject>
#include <QString>

#include <cstdint>

class AiProvider;
class Store;

/** What the model answered about one cluster, or why it answered nothing. */
struct BundleNaming {
    /** The topic name, as the model wrote it. */
    QString title;
    /** The notes it kept, in the order they were laid before it. */
    QList<qint64> noteIds;
    /** Empty exactly when the two above carry an answer. */
    QString error;
};

/**
 * What one cluster is asked with (SPEC 7.3): name the topic, and say which of
 * the notes really belong to it.
 *
 * **The notes are numbered, and the answer names numbers.** An answer naming
 * note ids could name one that is not in this cluster at all — a bundle would
 * then carry a note the clustering never put there, and nothing in the answer
 * would say so. The numbers are bounded by the list that was handed over.
 *
 * Not through i18n(), like classificationPrompt(): the text is addressed to a
 * model, not to the user, and a German catalogue would change what the model is
 * bound to.
 */
QString bundlePrompt(const QList<Note> &notes);

/**
 * Turns the answer into a title and the notes it kept, or into a reason there
 * is none.
 *
 * `notes` is the cluster as it was laid before the model, in the same order —
 * that is what the numbers of the answer are resolved against. A number outside
 * the list is dropped rather than refusing the answer: the model miscounting
 * costs one note, and it costs the bundle nothing.
 *
 * **An answer without the list of notes is refused**, and it is not read as
 * "all of them". The sanity check of SPEC 7.3 is half of what the model is
 * asked for, and an answer that skips it is not one that says every note fits.
 */
BundleNaming readBundleNaming(const QString &answer, const QList<Note> &notes);

/**
 * The collective note of SPEC 8.1: `# <topic>`, then a `## <YYYY-MM-DD>`
 * section per day with the notes of that day as paragraphs, chronological.
 *
 * **We write this, not the model**, and SPEC 7.3 says so since the decision of
 * 29.08.2026 (issue #29). An earlier wording had the model generate it. What
 * settles it is the note text: it is the one thing in the bundle the user
 * really typed, and a model that shortens, rewrites or summarises it hands
 * them something else back on the export to Obsidian (#32) — in the one place
 * where the original is then deleted (SPEC 8.1).
 *
 * **The frontmatter of SPEC 8.1 is not part of it.** `type`, `tags` and
 * `created` are written against the conventions of the vault this is exported
 * into, and SPEC 8.1 has them verified against that vault when the export is
 * built (#32). What stands here is the body — which is also what the card of
 * the review shows as its preview (SPEC 9).
 */
QString bundleMarkdown(const QString &title, const QList<Note> &notes);

/**
 * Step 3 of the analysis run of SPEC 7.2: the suggestions of SPEC 7.3 and 7.4,
 * written into `proposals` (issue #29).
 *
 * Two kinds, and only one of them costs a call:
 *
 * - **Task suggestions** (SPEC 7.4) are made out of what the classification
 *   already extracted. `notes.task` is the JSON object of the fields, and it is
 *   handed on unchanged — assembled a second time it would be a second place
 *   for the same fact. Nothing is carried out: SPEC 7.4 rules out an automatic
 *   `task add`, so what comes into being is a row and nothing else.
 * - **Bundle suggestions** (SPEC 7.3) come out of the clusters of the stored
 *   vectors. One `chat` call per cluster: the model names the topic and may
 *   drop an outlier; the Markdown is written here (see bundleMarkdown()).
 *
 * **What a standing suggestion does to the next run** is the difference between
 * the two statuses of SPEC 5.1:
 *
 * - A note in an **open** bundle is held out of the corpus. The question has
 *   been put and is waiting for an answer; clustering it again would put the
 *   same question a second time, every run.
 * - A note in a **deferred** bundle goes back into the corpus — "later"
 *   postpones, it hides nothing (SPEC 7.3). If the cluster forms again, the new
 *   suggestion **replaces** the deferred one rather than standing beside it
 *   (UX decision of 29.08.2026): two cards over the same notes are the same
 *   decision twice, and the user would have to answer both.
 * - A note that already carries a task suggestion gets no second one, whatever
 *   its status. A task is not recomputed from a corpus the way a cluster is —
 *   it is the note's own fields, and they have not changed.
 *
 * **Nothing here is counted against a note.** `analysis_attempts` belongs to
 * the classification and the embedding of that one note (SPEC 7.2); a bundle is
 * not a note, and a cluster whose call failed would burn the counters of every
 * note in it. A failed cluster is reported and passed over, and the next run
 * takes it up again.
 *
 * ponytail: a cluster that fails for ever is therefore asked for ever — once
 * per run, at the interval of SPEC 7.2. The upgrade path is a counter beside
 * the suggestion, and SPEC 5.1 names no column for one.
 *
 * **It starts nothing by itself**, like the two steps before it: when a run
 * happens is AnalysisScheduler's business (SPEC 7.2).
 */
class Suggester : public QObject
{
    Q_OBJECT

public:
    /**
     * Neither `store` nor `provider` is owned; both outlive the suggester.
     *
     * `embeddingModel` is the model the vectors were made with —
     * Embedder::model(), and not the setting read a second time: what is
     * clustered has to be what that run wrote, or the corpus comes out empty
     * and nothing says why.
     */
    Suggester(Store *store, AiProvider *provider, const QString &embeddingModel, QObject *parent = nullptr);

    /**
     * Takes up the notes and the clusters and returns at once — the work runs
     * in the event loop. Calling it while a run is going does nothing.
     */
    void start();

    /** Whether a cluster is being named right now. */
    bool isBusy() const;

public Q_SLOTS:
    /**
     * Re-reads `[AI] EmbeddingModel` out of `denkzettelrc`, the third of the
     * three that have to agree on it (issue #119).
     *
     * Out of the same ollama::configuredEmbeddingModel() the embedder reads,
     * and hung on the same signal: read a moment apart from the same file, the
     * two hold the same name — read out of two places they would be two names,
     * and this class would ask the store for a model nothing had written.
     */
    void reloadSettings();

Q_SIGNALS:
    /** A suggestion has been written and carries the id `proposalId`. */
    void suggested(qint64 proposalId);

    /** One cluster or one task suggestion came to nothing; the run goes on. */
    void failed(const QString &reason);

    /** The run is through and nothing is outstanding. */
    void finished();

private:
    void takeNextCluster();

    Store *m_store;
    AiProvider *m_provider;
    QString m_embeddingModel;
    /** The clusters of this run that are still outstanding. */
    QList<QList<Note>> m_clusters;
    /** The cluster laid before the model, in the order it was numbered in. */
    QList<Note> m_cluster;
    /** The id the answer being waited for arrives under (AiProvider). */
    int m_requestId = -1;
    bool m_busy = false;
};
