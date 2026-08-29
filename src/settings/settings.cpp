#include "settings/settings.h"

#include "analysis/analysisscheduler.h"
#include "analysis/clustering.h"
#include "analysis/ollamaprovider.h"
#include "transcribe/transcriber.h"

namespace
{
/** One enum choice; the name is what stands in `denkzettelrc`. */
KCoreConfigSkeleton::ItemEnum::Choice choice(const char *name)
{
    KCoreConfigSkeleton::ItemEnum::Choice entry;
    entry.name = QString::fromLatin1(name);
    return entry;
}
}

Settings *Settings::self()
{
    // Function-local and not a file-static: this is built the first time the
    // dialog is opened, and a file-static would open the configuration before
    // main() has set the application name — which is what decides the file is
    // called denkzettelrc at all.
    static Settings instance;
    return &instance;
}

Settings::Settings()
{
    // No file name: KCoreConfigSkeleton then takes KSharedConfig::openConfig(),
    // the same denkzettelrc that OllamaProvider and Transcriber read from
    // (SPEC 5.2).

    // The group and the key name are OriginWatcher's, which reads them out of
    // the same denkzettelrc at every reloadSettings() — a key written under
    // another name would leave the script loaded with the switch off (issue
    // #47). Off by default, and SPEC 13 says so: the origin is a window title
    // and therefore personal data, so nothing is determined until the user
    // says so.
    setCurrentGroup(QStringLiteral("Capture"));
    addItemBool(QStringLiteral("StoreOrigin"), m_storeOrigin, false);

    setCurrentGroup(QStringLiteral("AI"));
    addItem(new ItemEnum(currentGroup(),
                         QStringLiteral("Provider"),
                         m_provider,
                         {choice("Ollama"), choice("OpenRouter"), choice("OpenAI")},
                         Ollama),
            QStringLiteral("Provider"));
    addItemString(QStringLiteral("OllamaUrl"), m_ollamaUrl, QString(ollama::DefaultUrl));
    addItemString(QStringLiteral("ChatModel"), m_chatModel, QString(ollama::DefaultChatModel));
    addItemString(QStringLiteral("EmbeddingModel"), m_embeddingModel, QString(ollama::DefaultEmbeddingModel));

    setCurrentGroup(QStringLiteral("Analysis"));
    // Periodic and not "at once", although SPEC 7.2 lists that one first: the
    // only default the SPEC does name is the interval of 30 minutes, and an
    // interval carries a default because it is what runs by default. "At once"
    // would also hang one LLM call behind every single capture, so a session
    // without a reachable Ollama would meet the error state of SPEC 12 on the
    // very first note. The SPEC does not decide this and this comment is where
    // the decision is written down (issue #16, 29.08.2026).
    addItem(new ItemEnum(currentGroup(),
                         QStringLiteral("Trigger"),
                         m_analysisTrigger,
                         {choice(analysis::TriggerAfterSaving),
                          choice(analysis::TriggerPeriodically),
                          choice(analysis::TriggerOnDemand)},
                         Periodically),
            QStringLiteral("Trigger"));
    // The bounds are on the item and not only on the spin box: the skeleton is
    // what the dialog reads a stored value back through, and one outside the
    // range would come up on the form as a number the run does not obey — the
    // scheduler clamps a hand-written denkzettelrc to the same two values, out
    // of the same constants. The floor of 5 minutes is not taste — at 1 the field
    // would have to read "1 Minute" in German, and a spin box that declines its
    // own suffix is KPluralHandlingSpinBox out of KTextWidgets, a dependency
    // for one word.
    ItemInt *interval =
        addItemInt(QStringLiteral("IntervalMinutes"), m_analysisInterval, analysis::DefaultIntervalMinutes);
    interval->setMinValue(MinimumAnalysisInterval);
    interval->setMaxValue(MaximumAnalysisInterval);

    // SPEC 11 names the two defaults, 200 notes and 30 days, and no key for
    // either of them; SPEC 7.3 names the third, 3 notes, the same way. So the
    // names below follow the form of the two groups above — one group per page
    // of SPEC 13, the key written out — and whoever builds the overflow guard
    // reads them from here rather than inventing a second spelling.
    setCurrentGroup(QStringLiteral("Export"));
    // No default for the path on purpose (issue #75): it is a folder outside
    // this project that only the user knows, and a made-up one would send the
    // export of SPEC 8.1 somewhere nobody asked for. Empty means "not set".
    addItemString(QStringLiteral("VaultPath"), m_vaultPath, QString());
    // The bounds are on the items and not only on the spin boxes, for the
    // reason the analysis interval carries them: the guard of SPEC 11 reads a
    // hand-written denkzettelrc that never passed through the dialog.
    ItemInt *notes = addItemInt(QStringLiteral("OverflowNotes"), m_overflowNotes, 200);
    notes->setMinValue(MinimumThreshold);
    notes->setMaxValue(MaximumOverflowNotes);
    ItemInt *days = addItemInt(QStringLiteral("OverflowDays"), m_overflowDays, 30);
    days->setMinValue(MinimumThreshold);
    days->setMaxValue(MaximumOverflowDays);
    ItemInt *bundleNotes = addItemInt(QStringLiteral("BundleNotes"), m_bundleNotes, bundle::DefaultNotes);
    bundleNotes->setMinValue(MinimumThreshold);
    bundleNotes->setMaxValue(MaximumBundleNotes);

    // The group and both key names are Transcriber's, which reads them at
    // every reloadSettings() — a key written under another name would never
    // reach the queue.
    setCurrentGroup(QStringLiteral("Transcription"));
    // The choices are `whisper::Sizes` and not a list of our own: what a size
    // means is the file name Transcriber::modelPath() builds from it, so the
    // spelling and the order have exactly one place. The order is what a
    // written denkzettelrc depends on — see the comment at that array.
    QList<ItemEnum::Choice> sizes;
    sizes.reserve(whisper::Sizes.size());
    for (const QLatin1StringView size : whisper::Sizes) {
        ItemEnum::Choice entry;
        entry.name = QString(size);
        sizes.append(entry);
    }
    addItem(new ItemEnum(currentGroup(),
                         QStringLiteral("ModelSize"),
                         m_modelSize,
                         sizes,
                         whisper::DefaultSize),
            QStringLiteral("ModelSize"));
    addItemString(QStringLiteral("WhisperProgram"),
                  m_whisperProgram,
                  QString(whisper::DefaultProgram));
}

QString Settings::vaultPath() const
{
    return m_vaultPath;
}
