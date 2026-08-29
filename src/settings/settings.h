#pragma once

#include <KCoreConfigSkeleton>

#include <QString>

#include <cstdint>

/**
 * Every setting the dialog of SPEC 13 writes, in one skeleton (SPEC 5.2).
 *
 * It is one object and not one per page because KConfigDialog takes exactly
 * one, and because the group and key names have to agree with the code that
 * *reads* them at runtime: OllamaProvider opens the group "AI" itself and
 * would not see a key written under another name. Whoever changes a key here
 * changes it there too.
 *
 * **What a page contributes stands in a block of its own below**, and the
 * pages still to come (#74, #75, #27, #37) append theirs. The storage cannot
 * move into the page classes: a KConfigSkeleton item holds a reference to the
 * variable it fills, and a page is destroyed with the dialog while this object
 * outlives every one of them.
 *
 * No API key. It belongs in KWallet and never in a configuration file
 * (SPEC 5.2), so the field on the provider page has no item here.
 */
class Settings : public KCoreConfigSkeleton
{
    Q_OBJECT

public:
    /** The one instance, built on first use and living as long as the process. */
    static Settings *self();

    /** SPEC 7.1. What lands in `denkzettelrc` is the name, not the number. */
    enum Provider : std::uint8_t {
        Ollama,
        OpenRouter,
        OpenAi,
    };
    Q_ENUM(Provider)

    /** SPEC 7.2: when an analysis run starts. */
    enum AnalysisTrigger : std::uint8_t {
        AfterSaving,
        Periodically,
        OnDemand,
    };
    Q_ENUM(AnalysisTrigger)

    /** SPEC 7.2 names 30 minutes; 5 is the floor, see the comment at the item. */
    static constexpr int MinimumAnalysisInterval = 5;
    static constexpr int MaximumAnalysisInterval = 1440;

    /**
     * The floor of the three thresholds of the export page. Two and not one,
     * for the reason the analysis interval has a floor of five: a spin box
     * whose suffix is a fixed word can never be allowed to show a singular,
     * and one note or one day is no threshold anybody would set.
     */
    static constexpr int MinimumThreshold = 2;
    static constexpr int MaximumOverflowNotes = 100000;
    static constexpr int MaximumOverflowDays = 3650;
    static constexpr int MaximumBundleNotes = 100;

    /**
     * Where the export of SPEC 8.1 writes to — empty until the user sets it.
     *
     * The one getter on this object, and it is here because the export page
     * puts a refused folder back to the value that is stored (issue #75); the
     * overflow guard of SPEC 11 will read its two thresholds the same way.
     */
    QString vaultPath() const;

private:
    Settings();

    // Page "AI provider" (SPEC 7.1). The three defaults of these keys are not
    // written down here: they stand in `ollama::` in analysis/ollamaprovider.h,
    // where OllamaProvider reads them from as well (see the comment there).
    qint32 m_provider = Ollama;
    QString m_ollamaUrl;
    QString m_chatModel;
    QString m_embeddingModel;

    // Page "Analysis" (SPEC 7.2).
    qint32 m_analysisTrigger = Periodically;
    qint32 m_analysisInterval = 30;

    // Page "Export" (SPEC 13). The values belong to three different sections —
    // the path to SPEC 8.1, the two overflow thresholds to SPEC 11, the bundle
    // threshold to SPEC 7.3 — and the page is what holds them together.
    QString m_vaultPath;
    qint32 m_overflowNotes = 200;
    qint32 m_overflowDays = 30;
    qint32 m_bundleNotes = 3;
};
