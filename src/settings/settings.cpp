#include "settings/settings.h"

#include "analysis/ollamaprovider.h"

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
                         {choice("AfterSaving"), choice("Periodically"), choice("OnDemand")},
                         Periodically),
            QStringLiteral("Trigger"));
    // The bounds are on the item and not only on the spin box: a hand-written
    // denkzettelrc reaches the analysis run of SPEC 7.2 without ever passing
    // through the dialog. The floor of 5 minutes is not taste — at 1 the field
    // would have to read "1 Minute" in German, and a spin box that declines its
    // own suffix is KPluralHandlingSpinBox out of KTextWidgets, a dependency
    // for one word.
    ItemInt *interval = addItemInt(QStringLiteral("IntervalMinutes"), m_analysisInterval, 30);
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
    ItemInt *bundle = addItemInt(QStringLiteral("BundleNotes"), m_bundleNotes, 3);
    bundle->setMinValue(MinimumThreshold);
    bundle->setMaxValue(MaximumBundleNotes);
}

QString Settings::vaultPath() const
{
    return m_vaultPath;
}
