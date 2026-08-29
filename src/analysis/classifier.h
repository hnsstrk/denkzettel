#pragma once

#include "store/note.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class AiProvider;
class Store;

/**
 * What one classification call yielded, or why it yielded nothing (SPEC 7.2).
 *
 * `is_todo` of the JSON schema is no field here for the same reason it is no
 * column: it is exactly `task` being non-empty (see Note::task).
 */
struct Classification {
    /** One of the five short forms of SPEC 6, lower case. */
    QString category;
    /** One to four tags, lower case, without repetitions. */
    QStringList tags;
    /** The task fields as a compact JSON object; empty for a note that is none. */
    QString task;
    /** Empty exactly when the three above carry an answer. */
    QString error;
};

/**
 * The five categories SPEC 6 offers and SPEC 7.2 binds the classifier to.
 *
 * Without umlaut and without hyphen, so that `kat:` stays a literal comparison
 * (SPEC 6, user decision 2026-08-29): what stands in `notes.category` is this
 * short form, the readable label is made in the user interface.
 */
QStringList analysisCategories();

/**
 * What one note is asked with (SPEC 7.2).
 *
 * Not through i18n(): the text is addressed to a model, not to the user, and a
 * German catalogue would change what the model is bound to. It names the five
 * categories in English prose and the notes are mostly German — measured
 * against qwen3:8b on 2026-08-29, that combination answers with the short forms
 * unchanged and with German tags.
 */
QString classificationPrompt(const QString &noteText);

/**
 * Turns what the model wrote into a Classification, or into a reason there is
 * none.
 *
 * The JSON object is taken out of the answer by modelAnswerObject(), which
 * carries the reasoning block and the prose a model writes around its object;
 * `category` is the key that tells the answer from anything else in braces.
 *
 * **What the model may write is decided here, not by the model.** The category
 * has to be one of analysisCategories() after trimming and ASCII case folding;
 * anything else is refused with the value named, because a sixth category would
 * be a note that no `kat:` search and no sidebar entry ever reaches again. Tags
 * are lower-cased, emptied of repetitions and cut to the four SPEC 7.2 allows;
 * an answer without a single one is refused.
 *
 * **What decides whether there is a task is the description, not `is_todo`.**
 * SPEC 7.4 makes a suggestion out of the extracted fields, so a description is
 * what a task can be had from at all, and SPEC 5.1 puts the statement "this
 * note is a task" in `task IS NOT NULL`. The flag beside it decides nothing —
 * and every refusal above costs a whole classification and an attempt, which
 * after two of them leaves the note without a category for good. That price is
 * paid for a category that would be unreachable and for tags that are not
 * there; it is not paid for missing task fields, which SPEC 7.2 lets be null.
 */
Classification readClassification(const QString &answer);

/**
 * The classification run of SPEC 7.2, step 1: one LLM call per note, the
 * answer onto the note, the failures counted.
 *
 * One note at a time. Both providers SPEC 7.1 names beside Ollama are paid by
 * the call, and the local one has one graphics card — a run that asked for
 * fifty answers at once would queue them there anyway.
 *
 * **It starts nothing by itself.** When a run happens — at once, periodically
 * or on demand — is the trigger of SPEC 7.2, and that is AnalysisScheduler;
 * this class does what one run does and is called by it. How much one run may
 * take is notesPerRun below.
 *
 * **Nothing here reports to the user directly**, as with the transcription
 * queue: a routine run is silent (SPEC 14), and what SPEC 7.2 asks to be
 * reported — the note that is skipped after two failures — leaves as paused().
 * start() emits it again for a note that was already given up on before this
 * process began, so that a restart does not swallow the report.
 */
class Classifier : public QObject
{
    Q_OBJECT

public:
    /** Neither `store` nor `provider` is owned; both outlive the classifier. */
    Classifier(Store *store, AiProvider *provider, QObject *parent = nullptr);

    /**
     * The budget of SPEC 14: one run hands at most this many notes to a model,
     * the rest follow in the next run.
     *
     * It bounds what is **classified**, not what is looked at: a note that was
     * given up on is reported whether it stands before or behind the fiftieth,
     * or a library of a thousand unanalysed notes would keep the report of the
     * skipped ones to itself for days.
     */
    static constexpr int notesPerRun = 50;

    /**
     * Takes up the unanalysed notes and returns at once — the work runs in the
     * event loop. Calling it while a run is going does nothing.
     */
    void start();

    /** Whether a note is being classified right now. */
    bool isBusy() const;

Q_SIGNALS:
    /** The note carries its category, its tags and its state `analysiert`. */
    void classified(qint64 noteId);

    /**
     * The attempt failed and is counted; the note stays unanalysed.
     *
     * One per attempt. Whoever wants the end of the road wants paused() below.
     */
    void failed(qint64 noteId, const QString &reason);

    /**
     * The note is skipped: the two attempts of SPEC 7.2 are used up and it will
     * not be handed to a model again. That is what SPEC 14 asks to be reported.
     */
    void paused(qint64 noteId, const QString &reason);

    /**
     * The run is through and nothing is outstanding.
     *
     * A signal and not a look at isBusy(): between two notes a run is not busy
     * either, and whoever waited for that would count a half-done run as a
     * finished one.
     */
    void finished();

private:
    void takeNextNote();
    /** Counts the failure, reports it, and goes on to the next note. */
    void fail(const QString &reason);

    Store *m_store;
    AiProvider *m_provider;
    /** The notes of this run that are still outstanding, oldest first. */
    QList<Note> m_queue;
    /** The note being classified, and -1 between two of them. */
    qint64 m_noteId = -1;
    /** The id the answer being waited for arrives under (AiProvider). */
    int m_requestId = -1;
    bool m_busy = false;
};
