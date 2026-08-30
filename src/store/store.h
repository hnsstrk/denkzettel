#pragma once

#include "store/note.h"
#include "store/proposal.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include <optional>

/**
 * One row of `transcribe_jobs` (SPEC 5.1, 12).
 *
 * **`attempts` is what tells a note whose transcription is still outstanding
 * from one that has been given up on**, and `lastError` says why the last
 * attempt came to nothing (the note keeps its audio either way). The reason
 * alone cannot carry that: a daemon that is killed mid-run leaves the attempt
 * counted and no reason behind, and read by the reason that row would pass for
 * one that is still waiting — see Store::noteInterruptedTranscribeJobs(),
 * which is what fills it in.
 */
struct TranscribeJob {
    qint64 noteId = -1;
    QDateTime enqueuedAt;
    int attempts = 0;
    QString lastError;
};

/**
 * How many notes stand behind each entry of the library's category column
 * (SPEC 9, wireframe 1b).
 *
 * `byCategory` is keyed by the stored short form — `todos`, `ideen`, `cli`,
 * `persoenlich`, `software` (SPEC 6, 7.2). A note no analysis run has reached
 * yet carries no category and is counted in none of them; the readable label
 * is a matter of the user interface and is made there (SPEC 7.2).
 */
struct CategoryCounts {
    /** Every note in the library — what the entry "All" counts. */
    int total = 0;

    /** Notes per stored category value; a value no note carries is absent. */
    QHash<QString, int> byCategory;

    /**
     * Notes whose classification attempts are used up (SPEC 7.2).
     *
     * Exactly what an analysis run no longer takes on: `Classifier::start()`
     * skips these and reports them. Without an entry of their own the tray
     * message of issue #118 would name notes the window offers no way to.
     */
    int unclassified = 0;

    /**
     * Notes the analysis has not reached yet (issue #133).
     *
     * Written as the **complement** of the two pots beside it — no category,
     * and not among the given-up notes — so that
     * `total = Σ byCategory + unclassified + waiting` holds by construction
     * and not by a case distinction somebody moves later. Wireframe 1b adds
     * its five counters up to the number beside "All", so the column promises
     * that sum and not merely a list of filters; a note the classifier has
     * neither reached nor given up on fell out of all three counts and left
     * that promise broken by exactly its own number.
     *
     * A voice note whose transcript has not arrived lands here too, for a
     * second reason: `TRIM(content) != ''` keeps it out of `unclassified`, and
     * a category it has none.
     *
     * **The one combination that would break the sum** is a note carrying a
     * category *and* used-up attempts at `state != 'analysed'`: it is counted
     * in `byCategory` and in `unclassified` both. The classifier does not
     * write it — `completeAnalysis()` sets category, `state` and
     * `analysis_attempts = 0` in one UPDATE, so a note that has a category has
     * no attempts left standing — and no migration produces it either.
     * Written down rather than argued
     * away: it is the only input for which the column's promise fails, and
     * `storetest` builds that row by hand so the number is on the record.
     */
    int waiting = 0;
};

/**
 * Access to the SQLite database (SPEC 5.1).
 *
 * One instance owns one database connection; the path is passed in so tests
 * can work on a temporary file instead of the user's database.
 *
 * All methods report failure through their return value; the reason is
 * available from lastError(). **Every call clears it first**, so what stands
 * there belongs to the call that has just returned and never to an older one
 * — measured 2026-08-28: without that, a successful addNote() left the message
 * of a failed call standing, and whoever wrote it into a job line or a tooltip
 * would show a reason that has nothing to do with what went wrong.
 *
 * It announces what it takes in (noteAdded), and that is why it is a QObject:
 * every road a note travels into the database runs through addNote() — the
 * capture window and the D-Bus method AddNote() today, whatever comes with the
 * transcription tomorrow. A window that listens here hears all of them; one
 * that listened to the capture window would hear one of them (issue #105).
 */
class Store : public QObject
{
    Q_OBJECT

public:
    explicit Store(const QString &databasePath);
    ~Store();

    Store(const Store &) = delete;
    Store &operator=(const Store &) = delete;

    /** `~/.local/share/denkzettel/denkzettel.db` for the running application. */
    static QString defaultDatabasePath();

    /** Opens the database, creating directory, file and schema as needed. */
    bool open();

    QString lastError() const;

    /** Schema version recorded in `meta`, 0 before open(). */
    int schemaVersion() const;

    /** Directory holding the audio files referenced by Note::audioPath. */
    QString audioDirectory() const;

    /**
     * Where a recording goes that no note could be made of — beside
     * audioDirectory(), not inside it (SPEC 2.5, addition of 29.08.2026).
     *
     * **A place the sweep does not look, rather than a promise about the
     * sweep.** sweepOrphanedAudio() lists the files of audioDirectory() and no
     * subdirectory, and this is not even that: it is its sibling. A recording
     * whose note never reached the database is indistinguishable in the data
     * from a harmless orphan — a file, no row, nothing beside it — so the only
     * way to keep the sweep off it is to take it out of the directory the
     * sweep reads.
     *
     * **Nothing in Denkzettel ever deletes anything under here.** What happens
     * to the files is the user's decision, and the directory exists only once
     * something has gone wrong.
     */
    QString rescuedDirectory() const;

    /** Inserts a note and returns its new id. */
    std::optional<qint64> addNote(const Note &note);

    /** Writes all fields of an existing note, identified by note.id. */
    bool updateNote(const Note &note);

    std::optional<Note> note(qint64 id) const;

    /** All notes, newest first — the order the library lists them in (SPEC 9). */
    QList<Note> notes() const;

    /**
     * Notes whose text matches a free-text query, newest first (SPEC 6).
     *
     * The terms are ANDed, and each of them matches **anywhere inside a word**:
     * „foto", „grafieren" and „bahn" all find „fotografieren" resp.
     * „Straßenbahnen". Prefix matching is part of that, not a rule of its own.
     *
     * The tokenizer folds diacritics, so „bucher" finds „Bücher"; it does not
     * fold ß, so „strassenbahn" does not find „Straßenbahn" (S30).
     *
     * Terms of **one or two characters** cannot be in a trigram index and are
     * compared as substrings instead, so „KI" finds „KI-Pipeline". That route
     * ignores upper and lower case for ASCII only: it does not fold diacritics
     * („u" does not find „ü") and does not fold case beyond ASCII („ü" does
     * not find „Ü"). Both limits end at three characters.
     *
     * A query without any searchable term — empty, blank, pure punctuation —
     * returns the same list as notes(): clearing the search field restores the
     * full list.
     *
     * The operators of SPEC 6 are taken out by parseSearchQuery() and every
     * one of them narrows the same query further: `tag:` and `kat:` and `typ:`
     * against the columns of that name, `vor:` and `nach:` against the day,
     * and quoted phrases against the text beside the loose words. Two of them
     * inherit the ASCII-only case folding of the short-term route: `tag:` and
     * `kat:` compare COLLATE NOCASE, so `tag:BACKUP` finds `backup` and
     * `tag:BÜCHER` does not find `bücher`.
     */
    QList<Note> search(const QString &text) const;

    /** Deletes note and tags in one transaction, then its audio file. */
    bool removeNote(qint64 id);

    /**
     * Deletes the files in audioDirectory() that no note points at, one log
     * line each (SPEC 2.5).
     *
     * That is the one piece of self-healing the specification allows, and what
     * it heals is the gap removeNote() leaves open by design: the database is
     * the authority, so the file goes after the commit, and an interruption in
     * between leaves a recording nobody can reach any more. An aborted
     * recording is the other road to the same state.
     *
     * **For the service start, and only for it.** A recording writes its file
     * before the note exists (AudioRecorder::startEncoder()), so a sweep while
     * the program runs would take the recording in progress; at the start of
     * the single-instance daemon (SPEC 2.3) nothing of ours has recorded yet.
     * An outstanding transcription is no such case: a queued, a paused and a
     * given-up note all keep their `audio_path`, and this reads that column,
     * not the state.
     *
     * If the referenced names cannot be read, nothing is deleted — without
     * that list every file looks orphaned.
     */
    void sweepOrphanedAudio();

    /** Replaces all tags of a note. */
    bool setTags(qint64 noteId, const QStringList &tags);

    QStringList tags(qint64 noteId) const;

    /**
     * What the category column of the library writes beside its entries
     * (SPEC 9, wireframe 1b).
     *
     * Counted in the database and not over a list read into memory: the
     * counters stand for the whole library while the list beside them shows a
     * search result or one category, so the two must not come out of the same
     * read.
     */
    CategoryCounts categoryCounts() const;

    /**
     * How often one note is classified before it is left alone (SPEC 7.2:
     * "from the second failure on, the note is skipped").
     */
    static constexpr int analysisAttemptLimit = 2;

    /**
     * The notes an analysis run has work on, oldest first (SPEC 7.2).
     *
     * Everything that is not `analysiert` yet — the counter is **not**
     * filtered here, although a note that has used up its attempts is never
     * classified again: the caller has to see it to report it (SPEC 14 asks
     * for the tray tooltip and the log), and a query that hides it would make
     * the skip the silent kind. Classifier::start() is what does both.
     *
     * Notes without text are left out, and that is the untranscribed voice
     * note: it stands at `neu` until its transcript arrives (SPEC 12) and has
     * nothing to classify until then. The next run takes it, `transkribiert`
     * and with a text.
     */
    QList<Note> unanalysedNotes() const;

    /**
     * Writes what the classification of SPEC 7.2 found and resets the error
     * counting, in one transaction.
     *
     * Category, tags, the task fields and the state `analysiert` belong
     * together — a note whose category is written and whose tags are not would
     * count as analysed and be findable under none of them. `task` empty means
     * the note is no todo (see Note::task).
     */
    bool completeAnalysis(qint64 noteId, const QString &category, const QStringList &tags, const QString &task);

    /**
     * Counts a failed classification and records its reason, and answers with
     * the new count.
     *
     * The count is what survives a restart (SPEC 7.2), so it is read back out
     * of the database rather than added up in the caller: at
     * `analysisAttemptLimit` the note is done with, and that decision must not
     * hang on a number a run carried in memory.
     *
     * Counted **on failure** and not on the way out, which is where
     * takeTranscribeJob() counts and for a reason that does not hold here: a
     * whisper.cpp run occupies the graphics card and can take the daemon with
     * it, and an uncounted attempt would then be repeated for ever. A
     * classification is one HTTP request; a daemon that dies during it leaves
     * the note unanalysed, which is the state it was in before.
     */
    std::optional<int> failAnalysis(qint64 noteId, const QString &error);

    /**
     * The notes step 2 of an analysis run has to embed (SPEC 7.2), oldest
     * first.
     *
     * Three cases, and the query is the only place they are written down:
     * a note that carries no vector yet, one whose vector was made by a
     * **different** model than the one asked for, and one the user has edited
     * since — that is `needs_reembed`, which SPEC 9 sets on saving and
     * setEmbedding() clears.
     *
     * Only analysed notes, because only those are clustered (SPEC 7.3): a note
     * whose classification failed is not in the corpus, and an `embed` call for
     * it would be paid for and never asked after. Notes without text are left
     * out for the reason unanalysedNotes() leaves them out — a voice note
     * waiting for its transcript has nothing to embed yet.
     */
    QList<Note> notesToEmbed(const QString &model) const;

    /**
     * Writes the vector of one note and clears its `needs_reembed`, in one
     * transaction (SPEC 7.2 step 2).
     *
     * An existing vector of the same note is replaced: there is one current
     * text per note and therefore one current vector.
     */
    bool setEmbedding(qint64 noteId, const QString &model, const QList<float> &vector);

    /**
     * What the topic clustering of SPEC 7.3 compares: the embeddings of all
     * unexported, analysed notes made by `model`, oldest first.
     *
     * The model is a parameter and not a setting read here, because it is the
     * embedding run that knows which one wrote the vectors — the store keeps
     * the name, the analysis owns it (SPEC 7.1).
     */
    QList<NoteEmbedding> embeddings(const QString &model) const;

    /**
     * Writes one suggestion together with its note references, in one
     * transaction, and returns its new id (SPEC 7.3, 7.4).
     *
     * The two belong together: a suggestion without its references stands for
     * nothing, and the review would offer a bundle of no notes. `proposal.id`
     * is ignored — the database gives it one.
     *
     * A note id that no note carries makes the whole call fail rather than
     * writing a suggestion with a gap in it: `PRAGMA foreign_keys` is on and
     * the reference is what points at the note.
     */
    std::optional<qint64> addProposal(const Proposal &proposal);

    /**
     * The analysed notes that carry task fields and no task suggestion yet,
     * oldest first (SPEC 7.4).
     *
     * `task IS NOT NULL` **is** the statement that the note is a task; there is
     * no `is_todo` column and SPEC 5.1 says why. The second half of the
     * condition is what keeps a run from offering the same task again every
     * half hour, and it counts a **deferred** suggestion too: a task is the
     * note's own fields, not something recomputed out of a corpus, so "later"
     * has nothing to come back to.
     */
    QList<Note> notesForTaskProposals() const;

    /**
     * Every suggestion with its notes, oldest first (SPEC 9).
     *
     * One read for everything the suggestion run needs to know about what
     * already stands: which notes carry an open bundle, which ones already have
     * a task suggestion, and which deferred bundle a fresh one supersedes. At
     * the ~200 notes of the overflow guard that is a handful of rows, and three
     * queries of their own would be three places for the same list.
     */
    QList<Proposal> proposals() const;

    /**
     * Deletes one suggestion; its note references go with it (ON DELETE
     * CASCADE, migration 6). The notes themselves stay.
     *
     * That is the end of every road SPEC 8.1 names — accepting, discarding, and
     * a deferred bundle that a later run has replaced.
     */
    bool removeProposal(qint64 id);

    /**
     * How often one note is handed out for transcription before the job pauses
     * (SPEC 12). Counted on the way out, see takeTranscribeJob().
     */
    static constexpr int transcribeAttemptLimit = 2;

    /** Puts a note into the transcription queue (SPEC 12). */
    bool enqueueTranscription(qint64 noteId);

    /**
     * The oldest job still worth an attempt, **counting that attempt**, or
     * nothing if the queue holds none.
     *
     * The count happens here and not on failure, and that is what makes the
     * queue survive a crash: a daemon killed while whisper.cpp was running
     * leaves the row behind with the attempt already counted. Counted on
     * failure, the same row would come back untouched after every restart and
     * be retried for ever — the "stays 'running' for ever" of the other kind.
     */
    std::optional<TranscribeJob> takeTranscribeJob();

    /**
     * Writes the transcript onto the note and takes the job out of the queue,
     * in one transaction: an interruption in between would otherwise leave a
     * transcribed note in the queue and transcribe it a second time.
     */
    bool completeTranscription(qint64 noteId, const QString &transcript);

    /**
     * Notes why the attempt failed. **The row stays**, including after the
     * last attempt: it is the only place that tells a note whose transcription
     * is still outstanding from one that has been given up on — `notes.state`
     * reads 'neu' in both cases (SPEC 5.1).
     */
    bool failTranscribeJob(qint64 noteId, const QString &error);

    std::optional<TranscribeJob> transcribeJob(qint64 noteId) const;

    /**
     * The newest job that has used up its attempts, or nothing.
     *
     * The condition is the exact complement of the one takeTranscribeJob()
     * selects by, so the two cannot drift apart: what that one no longer hands
     * out is what this one reports. The newest and not the oldest — of several
     * given-up notes the one the user recorded last is the one they are
     * looking for.
     */
    std::optional<TranscribeJob> pausedTranscribeJob() const;

    /**
     * Writes `reason` on every job whose attempt was counted and never
     * answered for, and reports whether the write went through.
     *
     * That is the state a daemon that was killed leaves behind: the attempt is
     * counted in takeTranscribeJob(), and the answer never comes — neither
     * failTranscribeJob() nor completeTranscription() runs. The row then reads
     * as one whose last attempt is still outstanding, although with the
     * attempts used up it is never handed out again.
     *
     * **The caller has to be one that knows no job is running**, or it
     * overwrites the running attempt's empty reason. In the daemon that is
     * Transcriber::start(): the service is single-instance (SPEC 2.3) and
     * start() runs before a job of its own is taken.
     */
    bool noteInterruptedTranscribeJobs(const QString &reason);

Q_SIGNALS:
    /**
     * A note has been written and carries the id `id` (issue #105).
     *
     * Emitted after the insert has succeeded, so a listener that reads the
     * store again finds the note. It says that something was added, not what:
     * the library reads its own list back and applies its own search term to
     * it, and a signal carrying the note would tempt it to skip that.
     */
    void noteAdded(qint64 id);

private:
    bool migrate();

    /** Replaces the tags of one note. The caller owns the transaction. */
    bool replaceTags(qint64 noteId, const QStringList &tags);

    QString m_databasePath;
    QString m_connectionName;
    QSqlDatabase m_db;
    int m_schemaVersion = 0;
    mutable QString m_lastError;
};
