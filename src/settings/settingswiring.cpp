#include "settings/settingswiring.h"

#include "analysis/analysisscheduler.h"
#include "analysis/embedder.h"
#include "analysis/ollamaprovider.h"
#include "analysis/suggester.h"
#include "settings/settings.h"
#include "shell/originwatcher.h"
#include "transcribe/transcriber.h"

#include <QObject>

void connectSettingsToRunningObjects(Transcriber *transcriber,
                                     OriginWatcher *origins,
                                     OllamaProvider *provider,
                                     Embedder *embedder,
                                     Suggester *suggester,
                                     AnalysisScheduler *analysis)
{
    const Settings *settings = Settings::self();

    // What the settings page "Voice notes" writes reaches the running queue at
    // once — model size and program path take hold without a restart (SPEC 13,
    // issue #27). The connection hangs on the skeleton and not on the dialog:
    // the dialog is built and destroyed per opening, these objects live as long
    // as the process. KCoreConfigSkeleton::save() emits configChanged() once
    // per save that changed something and not at all for one that did not
    // (measured 29.08.2026), so this runs when there is something new to read
    // and never on its own.
    QObject::connect(settings, &Settings::configChanged, transcriber, &Transcriber::reloadSettings);
    // And the queue is taken up again after the road that can put the missing
    // model of SPEC 12 in place from here: a size that is already on disk,
    // chosen in the settings. Without it the job the queue stopped for would lie
    // there until the next start of the daemon — it is still in the queue with
    // its attempts untouched, and nothing else ever asks again. start() does
    // nothing when there is no job and nothing when the model is still missing.
    //
    // The second road to the same thing, a download that has just finished,
    // stays in main.cpp: it is not a setting.
    QObject::connect(settings, &Settings::configChanged, transcriber, &Transcriber::start);

    // The context stamp of SPEC 5.1 and 13 (issue #47) takes hold on a running
    // daemon the same way: with „[Capture] StoreOrigin" switched the watcher
    // loads or unloads the KWin script at once, and without this the switch
    // would look like a setting that does nothing until the next login.
    QObject::connect(settings, &Settings::configChanged, origins, &OriginWatcher::reloadSettings);

    // What the settings dialog wrote reaches the running trigger here, and only
    // here: without it a switched mode would take effect at the next start of
    // the daemon and look like a setting that does nothing (SPEC 13, issue #16).
    QObject::connect(settings, &Settings::configChanged, analysis, &AnalysisScheduler::applySettings);

    // And the same for the backend itself (issue #119): the address and the two
    // models of SPEC 7.1 were read once at construction, so a server or a model
    // chosen in the dialog reached the running run at the next start of the
    // daemon and not before — while "Test connection" on that very page, which
    // works with the value out of the form, reported the new address as
    // reachable. The check said yes and the analysis talked to the old server.
    //
    // Three connections and not one, because three objects hold the embedding
    // model: the provider asks for the vector, the embedder writes the name
    // beside it and the suggester looks the vectors up by it (SPEC 7.3). Left
    // out, either of the two latter would go on with the old name while the
    // provider asked the new model — vectors of two models under one name, or a
    // corpus that comes out empty. They read it out of one function for that
    // reason (ollama::configuredEmbeddingModel()).
    QObject::connect(settings, &Settings::configChanged, provider, &OllamaProvider::reloadSettings);
    QObject::connect(settings, &Settings::configChanged, embedder, &Embedder::reloadSettings);
    QObject::connect(settings, &Settings::configChanged, suggester, &Suggester::reloadSettings);
}
