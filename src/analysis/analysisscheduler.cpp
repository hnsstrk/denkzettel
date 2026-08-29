#include "analysis/analysisscheduler.h"

#include "analysis/classifier.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <algorithm>

AnalysisScheduler::AnalysisScheduler(Classifier *classifier, QObject *parent)
    : QObject(parent)
    , m_classifier(classifier)
{
    connect(m_classifier, &Classifier::finished, this, [this] {
        if (!m_runWhenIdle) {
            return;
        }
        m_runWhenIdle = false;
        analyzeNow();
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
    if (m_classifier->isBusy()) {
        m_runWhenIdle = true;
        return;
    }
    m_classifier->start();
}

void AnalysisScheduler::noteIsReady()
{
    if (m_afterSaving) {
        analyzeNow();
    }
}
