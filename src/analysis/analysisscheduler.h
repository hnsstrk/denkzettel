#pragma once

#include <QObject>
#include <QTimer>

#include <chrono>

class Classifier;
class Embedder;
class Suggester;

/**
 * What `[Analysis] Trigger` may say and how far `IntervalMinutes` may go
 * (SPEC 7.2), and the one place those values stand.
 *
 * Two parties need the same ones: the scheduler below, which reads
 * `denkzettelrc` at runtime, and the settings skeleton, which offers the same
 * choices and the same bounds on the form. **`settings.{h,cpp}` is built from
 * the names and numbers below** — its choice list, its interval default and its
 * two bounds are these — so there is nothing here to keep in step.
 *
 * And nothing could have kept it in step: written down twice, a choice renamed
 * on one side alone leaves the scheduler reading a trigger nobody can set, it
 * falls back to the default without a word, and **all fourteen test sets stay
 * green** — measured on 2026-08-29 by renaming "AfterSaving" to "Immediately"
 * in the skeleton and moving the floor from 5 to 15. Putting that literal back
 * into settings.cpp today leaves all fourteen green again — so what makes one
 * source the answer here is that no check could be the answer: `aitest` cannot
 * link `denkzettelsettings`, which links this library and not the other way
 * round. **The guard is the build, and it was read back**: renaming
 * `TriggerAfterSaving` here alone stops the compiler in `settings.cpp:56` with
 * "is not a member of analysis".
 *
 * What is **not** covered by it, and is a comment on both sides the way the
 * `[AI]` keys are (see settings.h): the group name "Analysis" and the two key
 * names. Whoever changes a key there changes it in applySettings() too.
 *
 * Here and not in the settings, because the dependency only runs one way:
 * `denkzettelsettings` links `denkzettelanalysis`, not the other way round.
 * That is where `ollama::` stands and for the same reason.
 *
 * `const char *` and not QLatin1StringView, because these names are handed to
 * the choice list of KCoreConfigSkeleton, which is built from `const char *`.
 */
namespace analysis
{
inline constexpr const char *TriggerAfterSaving = "AfterSaving";
inline constexpr const char *TriggerPeriodically = "Periodically";
inline constexpr const char *TriggerOnDemand = "OnDemand";

/** SPEC 7.2 names 30 minutes; the reason for the floor stands at the item. */
inline constexpr int DefaultIntervalMinutes = 30;
inline constexpr int MinimumIntervalMinutes = 5;
inline constexpr int MaximumIntervalMinutes = 1440;
}

/**
 * When an analysis run happens, and what one run is (SPEC 7.2).
 *
 * **A run is the three steps of SPEC 7.2, one after the other**: the
 * classification, then the embedding of what came out of it, then the
 * suggestions of SPEC 7.3 and 7.4. Each of them does what its step does and
 * starts nothing by itself; this is the one place that knows the order, and it
 * is the order the data takes — a note is embedded once it has a category, and
 * it is clustered once it has a vector. Chained the other way round every step
 * would work on what the run before it left.
 *
 * **The steps run one after the other and never side by side**: all three go
 * through one AiProvider, and a second call while the first is out would be
 * answered under an id nobody is waiting for.
 *
 * This is also what starts a run, along the three roads SPEC 7.2 names:
 *
 * - **at once** — every note that is written, and every transcript that
 *   arrives for one, is a run. Both come in from outside through noteIsReady():
 *   what a note carries is the store's business and a finished transcript is
 *   the transcription queue's, and this library links neither.
 * - **periodically** — a timer at `[Analysis] IntervalMinutes`.
 * - **on demand** — `AnalyzeNow()` on the bus (SPEC 2.3), the tray entry beside
 *   it and, since issue #132, the library's application menu; all three land on
 *   analyzeNow(). That one runs whatever the setting says: being asked for a run
 *   is what "on demand" means, and the other two modes have no reason to refuse
 *   one.
 *
 * **A note written while a run is going is not lost, and not classified
 * twice.** Every step declines a second start while it is busy, so the request
 * is remembered and the run repeated once the standing one is through — the
 * note then comes out of the database with the others rather than being carried
 * here. Twice cannot happen for the same reason: every run reads its work back
 * out of the database, and what is through is not among it any more.
 *
 * **What is waited for is the end of the last step, not of the first.** A queue
 * is idle between two of its jobs as well (CLAUDE.md, finding 32), so a run
 * that was asked for while one was going has to wait for Suggester::finished()
 * — repeated on Classifier::finished() it would start the classification again
 * while the embedding of the standing run was still out.
 *
 * **Nothing is started when the daemon starts**, unlike the transcription
 * queue, which picks up what a killed run left behind. There is nothing to pick
 * up: an interrupted classification leaves the note exactly as it was, and a
 * run at every login would be one the user asked for under none of the three
 * settings.
 */
class AnalysisScheduler : public QObject
{
    Q_OBJECT

public:
    /** None of the three steps is owned; all of them outlive the scheduler. */
    AnalysisScheduler(Classifier *classifier, Embedder *embedder, Suggester *suggester,
                      QObject *parent = nullptr);

    /**
     * How long the periodic timer stands at, or zero when none is armed.
     *
     * For reading back what applySettings() made of the setting. A timer that
     * was never started looks exactly like one running at the wrong interval,
     * and neither is visible from outside (CLAUDE.md, finding 27).
     */
    std::chrono::milliseconds interval() const;

    /**
     * Whether any of the three steps of SPEC 7.2 is working.
     *
     * All three and not the classification alone: between two steps of one run
     * the first one is through and the run is not (CLAUDE.md, finding 32).
     *
     * Public since issue #132, where the library has to say which of the two
     * states it is in. Read on the scheduler and never off the button that
     * asked for the run — a run that was never started looks exactly like one
     * that finished (finding 27).
     */
    bool isBusy() const;

public Q_SLOTS:
    /**
     * Reads `[Analysis] Trigger` and `IntervalMinutes` and arms what they name.
     *
     * Called at construction and whenever the settings dialog has written
     * (KCoreConfigSkeleton::configChanged) — without that a switched mode would
     * only take effect at the next start of the daemon.
     */
    void applySettings();

    /** The on-demand road of SPEC 7.2, and where the other two end up as well. */
    void analyzeNow();

    /** A note has been written or has got its transcript (SPEC 7.2, "at once"). */
    void noteIsReady();

Q_SIGNALS:
    /**
     * A run has begun or the last of its three steps has ended (issue #132).
     *
     * Emitted only when the answer of isBusy() really changes, so that a caller
     * may hang a state on it: a request that finds the queue busy already
     * changes nothing and says nothing.
     *
     * **The end is the end of the last step, not of the first.** Hung on
     * Classifier::finished this would report the run over while the embedding
     * of the same run was still out — a queue is idle between two of its jobs
     * as well (CLAUDE.md, finding 32).
     *
     * **A run with nothing to analyse emits nothing at all**, and that is not
     * an oversight: all three steps then walk through inside analyzeNow(), the
     * answer of isBusy() never leaves false, and a signal saying otherwise
     * would be a state nobody ever takes back. Whoever wants to know how such
     * a press ended reads isBusy() after it.
     */
    void busyChanged(bool busy);

private:
    /** Emits busyChanged() if, and only if, the answer of isBusy() has moved. */
    void updateBusy();

    Classifier *m_classifier;
    Embedder *m_embedder;
    Suggester *m_suggester;
    QTimer m_interval;
    /** Whether noteIsReady() is a run, i.e. whether the trigger is "at once". */
    bool m_afterSaving = false;
    /** A run was asked for while one was going, and is owed once it ends. */
    bool m_runWhenIdle = false;
    /** What busyChanged() last reported, so that it reports only changes. */
    bool m_busy = false;
};
