#include "analysis/analysisscheduler.h"

#include "analysis/classifier.h"
#include "analysis/embedder.h"
#include "analysis/suggester.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <algorithm>

AnalysisScheduler::AnalysisScheduler(Classifier *classifier, Embedder *embedder, Suggester *suggester,
                                     QObject *parent)
    : QObject(parent)
    , m_classifier(classifier)
    , m_embedder(embedder)
    , m_suggester(suggester)
{
    // The three steps of SPEC 7.2 in the order the data takes them, and the
    // chain lives here rather than in the steps: each of them would otherwise
    // have to know the one after it, and the classification would link the
    // clustering to start it.
    connect(m_classifier, &Classifier::finished, m_embedder, &Embedder::start);
    connect(m_embedder, &Embedder::finished, m_suggester, &Suggester::start);

    connect(m_suggester, &Suggester::finished, this, [this] {
        if (m_runWhenIdle) {
            m_runWhenIdle = false;
            analyzeNow();
        }
        // Behind the owed run and not instead of it: a run taken up here leaves
        // isBusy() true, so updateBusy() reports nothing and the state of the
        // window outside stays "busy" across the seam between the two runs
        // (issue #132).
        updateBusy();
    });

    // Half an hour by default, and nothing here needs it to the second: a
    // coarse timer lets the kernel group the wakeup with others and keeps the
    // daemon from waking a sleeping machine on its own account.
    m_interval.setTimerType(Qt::VeryCoarseTimer);
    connect(&m_interval, &QTimer::timeout, this, &AnalysisScheduler::analyzeNow);

    applySettings();
}

std::chrono::milliseconds AnalysisScheduler::interval() const
{
    return m_interval.isActive() ? m_interval.intervalAsDuration() : std::chrono::milliseconds(0);
}

void AnalysisScheduler::applySettings()
{
    // The same denkzettelrc the settings dialog writes (SPEC 5.2), read through
    // the group rather than through the skeleton: the skeleton lives in
    // denkzettelsettings, which links this library and not the other way round.
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Analysis"));
    const QString trigger =
        group.readEntry("Trigger", QString::fromLatin1(analysis::TriggerPeriodically));

    m_afterSaving = trigger == QLatin1StringView(analysis::TriggerAfterSaving);

    if (trigger != QLatin1StringView(analysis::TriggerPeriodically)) {
        m_interval.stop();
        return;
    }

    // Clamped here as well as at the item in the settings: a hand-written
    // denkzettelrc reaches this line without ever passing the spin box, and a
    // zero there would be a timer firing as fast as the event loop turns — one
    // LLM call per note per turn.
    const int minutes = std::clamp(group.readEntry("IntervalMinutes", analysis::DefaultIntervalMinutes),
                                   analysis::MinimumIntervalMinutes,
                                   analysis::MaximumIntervalMinutes);
    m_interval.start(std::chrono::minutes(minutes));
}

void AnalysisScheduler::analyzeNow()
{
    if (isBusy()) {
        m_runWhenIdle = true;
        return;
    }
    m_classifier->start();
    // After the start and not before it, because start() may be the whole run:
    // with nothing to analyse all three steps walk through inside this call and
    // isBusy() is false again by the time it returns. Nothing is emitted then,
    // which is the honest answer — the state never moved.
    updateBusy();
}

void AnalysisScheduler::updateBusy()
{
    const bool busy = isBusy();
    if (busy == m_busy) {
        return;
    }
    m_busy = busy;
    Q_EMIT busyChanged(busy);
}

bool AnalysisScheduler::isBusy() const
{
    // All three, and not the classification alone: between two steps of one run
    // the first one is through and the run is not (CLAUDE.md, finding 32).
    return m_classifier->isBusy() || m_embedder->isBusy() || m_suggester->isBusy();
}

void AnalysisScheduler::noteIsReady()
{
    if (m_afterSaving) {
        analyzeNow();
    }
}
