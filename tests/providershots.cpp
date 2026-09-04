#include "settings/aiproviderpage.h"
#include "settings/settings.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QRadioButton>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>

/**
 * The pictures of issues #38, #39 and #144: the page "AI provider" once **all
 * three**
 * providers take clicks, with the row rule the Product Owner settled on
 * 30.08.2026 — the key row under a provider that needs one, the language model
 * row of the chosen service, and the sentence naming Ollama as what answers the
 * embedding call whatever is chosen. Under OpenAI the note of SPEC 7.5 stands
 * with it, saying why the field wants an API key and not a "Sign in with
 * ChatGPT".
 *
 * Since #144 they carry one row more: the sentence that the text of every
 * classified note leaves the machine and goes to the chosen remote service. It is the picture that
 * has to show it — the criterion is "visible without scrolling and without
 * opening anything", and that is a statement about the drawn page and not about
 * a value in a widget (CLAUDE.md, finding 51). What the readback adds is the
 * half a picture cannot carry: that under Ollama the label is **empty** and not
 * merely undrawn.
 *
 * They replace nothing: the `127-`, `38-` and `39-anbieter-*.png` beside them
 * are the record of the states before, and each of those stories changed that
 * state. #144 is the newest, so this runner writes the `144-` set — **and its
 * Ollama picture is byte for byte the one #39 wrote**. That is not a duplicate
 * by accident, it is the evidence: under the default provider this story
 * changes nothing at all, and `cmp` on the two files is what says so
 * (CLAUDE.md, finding 39).
 *
 * Not a test and out of `add_test()`, for the reason `readmeshots` is out of
 * it: a broken picture writer must not turn the suite red. It is built with
 * the suite all the same, because a runner nobody rebuilds ages unnoticed and
 * then writes plausible pictures of an **old** state with a fresh timestamp
 * (CLAUDE.md, rule 4).
 *
 * **One page per stored value, and the value is written into `denkzettelrc`
 * before the page is built** — not checked on the button afterwards. That is
 * the whole point of the run since #38: the defect #127 measured was a key row
 * that followed the Ollama button's `toggled` signal, so a page coming up with
 * another provider already stored never grew the row, and only a switch **away
 * from Ollama** ever brought it up. A runner that checks the button by hand
 * walks the road that always worked and would have been green over the fault.
 * Writing the setting first is what makes the picture the dialog's own state.
 *
 * The control after the three is the other road: a switch by hand, which has
 * to keep working as well.
 *
 * Beside every picture the run prints what it drew with and what it measured:
 * the style it resolved, the two colours, the resolved wording of the OpenAI
 * note (which says whether the catalogue was found), and for all three buttons
 * `isEnabled()` and `isChecked()` next to the API key row's state. The checked
 * state is read **after** `QTest::qWait`, because Breeze animates the dot and
 * a `grab()` in the same turn draws the state the animation starts from
 * (CLAUDE.md, finding 43).
 *
 * The page stands on its own here, without the dialog that otherwise holds it.
 * Since #38 it reads `[AI] Provider` itself while it is built, so the stored
 * value reaches the buttons and the rows without a KConfigDialogManager — the
 * `kcfg_` fields stay empty all the same, because filling those is the
 * manager's work and there is none. The rows that carry a value of their own in
 * the built settings are named in the readback below.
 *
 * That the stored value goes back into the file unchanged, and that the key row
 * follows it, is the business of `settingstest` —
 * `aStoredProviderReachesTheButtonsAndTheRows()`; that the button asks the chosen
 * provider is `theConnectionTestAsksTheChosenProvider()`.
 *
 * **The committed pictures under `docs/images/reviews/` are the German ones**,
 * and the call below is the one that reproduces them byte for byte. The run
 * writes one language set per call under **the same three file names**
 * (`144-anbieter-*.png`), so an English run pointed at that directory
 * overwrites them with English pictures
 * of the same state — it belongs in a throwaway directory. Read it back: run
 * the line, then `git status`, and nothing may have changed.
 *
 * Usage — the environment is not optional, see rule 2 and finding 28. The
 * catalogue has to be findable at runtime, the way the README section
 * "Screenshots" describes it; nothing is written outside the staging root:
 *
 *   cmake --build build --target providershots
 *   conf=$(mktemp -d); printf '[Theme]\nname=breeze-dark\n' > "$conf/plasmarc"
 *   dest=$(mktemp -d); DESTDIR="$dest" cmake --install build
 *   env LANGUAGE=de LANG=de_DE.UTF-8 LC_ALL=de_DE.UTF-8 \
 *       XDG_DATA_DIRS="$dest/usr/share:/usr/share" XDG_CONFIG_DIRS="$conf:/etc/xdg" \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/providershots docs/images/reviews
 *
 * The same call without `XDG_DATA_DIRS` and with `-u LANGUAGE
 * LANG=en_US.UTF-8` writes the English set — **into a directory of its own**,
 * `build/providershots-en` or a `mktemp -d`, never into `docs/images/reviews`.
 * It is the control that says the catalogue is what makes the difference: the
 * sentence and the OpenAI label have to come out English then.
 *
 * The run writes into an `XDG_CONFIG_HOME` of its own (below), so the
 * `denkzettelrc` it sets the provider in is its own and never the reader's.
 *
 * Without `QT_FORCE_STDERR_LOGGING=1` every line this run reports goes into
 * the journal instead of the terminal (CLAUDE.md, finding 25).
 */
namespace
{
/** What the page carries, read back beside every picture. */
void report(const QString &what, QWidget &page)
{
    const auto *box = page.findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
    const auto *key = page.findChild<QLineEdit *>(QStringLiteral("apiKey"));
    const QList<QRadioButton *> buttons = box->findChildren<QRadioButton *>();
    for (const QRadioButton *button : buttons) {
        qWarning("%s  button \"%s\" enabled=%d checked=%d",
                 qUtf8Printable(what),
                 qUtf8Printable(button->text()),
                 int(button->isEnabled()),
                 int(button->isChecked()));
    }
    // `isHidden()` and not `isVisible()`: a row hidden by the page and a row on
    // a widget that has not been shown look the same to `isVisible()`.
    qWarning("%s  API key row shown=%d", qUtf8Printable(what), int(!key->isHidden()));

    // The row rule of 30.08.2026, read back beside the picture: one language
    // model row per provider, and the sentence that says which service the
    // address and the embedding model belong to. Three model rows since #39,
    // and the note of SPEC 7.5 beside them — a row shown under the wrong
    // provider is what the picture alone cannot tell from a row shown under all
    // of them (CLAUDE.md, finding 79).
    const auto *ollamaModel = page.findChild<QComboBox *>(QStringLiteral("kcfg_ChatModel"));
    const auto *remoteModel = page.findChild<QComboBox *>(QStringLiteral("kcfg_OpenRouterModel"));
    const auto *openAiModel = page.findChild<QComboBox *>(QStringLiteral("kcfg_OpenAiModel"));
    const auto *fromOllama = page.findChild<QLabel *>(QStringLiteral("embeddingsFromOllama"));
    const auto *openAiNote = page.findChild<QLabel *>(QStringLiteral("openAiNote"));
    if (ollamaModel == nullptr || remoteModel == nullptr || openAiModel == nullptr
        || fromOllama == nullptr || openAiNote == nullptr) {
        qFatal("the page carries no model rows to report on");
    }
    qWarning("%s  model rows shown: Ollama=%d openrouter=%d OpenAI=%d"
             "  embedding note shown=%d  OpenAI note shown=%d",
             qUtf8Printable(what),
             int(!ollamaModel->isHidden()),
             int(!remoteModel->isHidden()),
             int(!openAiModel->isHidden()),
             int(!fromOllama->isHidden()),
             int(!openAiNote->isHidden()));

    // The sentence of issue #144, and **both** halves of it: whether the row is
    // shown, and what the label holds. The second one is what the criterion
    // asks for — under Ollama the text has to be empty and not merely hidden,
    // or the readback answers the same for "nothing to say" as for "said it and
    // wrongly hidden" (CLAUDE.md, finding 79). Printed as a quoted string, so
    // an empty text is visible as `""` rather than as a missing line, and
    // printed for **all three** providers rather than only the two that have
    // something to say (UX, 04.09.2026) — with the service named in the
    // sentence the three come out three different ways, which is the readback
    // that can be wrong (finding 10).
    const auto *remoteText = page.findChild<QLabel *>(QStringLiteral("remoteTextNote"));
    if (remoteText == nullptr) {
        qFatal("the page carries no note named remoteTextNote");
    }
    qWarning("%s  remote-text note shown=%d text=\"%s\"",
             qUtf8Printable(what),
             int(!remoteText->isHidden()),
             qUtf8Printable(remoteText->text()));
}

void shoot(QWidget &page, const QString &directory, const QString &name)
{
    const QPixmap picture = page.grab();
    if (!picture.save(directory + QLatin1Char('/') + name)) {
        qFatal("Picture %s could not be written", qUtf8Printable(name));
    }
    qWarning("written: %s (%dx%d px)", qUtf8Printable(name), picture.width(), picture.height());
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        qFatal("usage: providershots <target directory>");
    }

    // A configuration directory of its own, and a real colour scheme in it
    // **before** QApplication: without a kdeglobals the platform theme and
    // KColorScheme read two different sources, and the picture then shows a
    // fault of the runner (CLAUDE.md, finding 38). The desktop theme goes
    // beside it for finding 8.
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());
    QFile::copy(QStringLiteral("/usr/share/color-schemes/BreezeDark.colors"),
                configuration.path() + QStringLiteral("/kdeglobals"));
    QFile scheme(configuration.path() + QStringLiteral("/kdeglobals"));
    if (scheme.open(QIODevice::Append)) {
        scheme.write("\n[General]\nColorScheme=BreezeDark\n");
        scheme.close();
    }
    QFile plasma(configuration.path() + QStringLiteral("/plasmarc"));
    if (plasma.open(QIODevice::WriteOnly)) {
        plasma.write("[Theme]\nname=breeze-dark\n");
        plasma.close();
    }

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("denkzettel"));
    // Without the domain every i18n() falls back to English and the run writes
    // an English picture under a German name, whatever LANGUAGE says (#138).
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Read back what the run really drew with, rather than trusting that the
    // variables were set (findings 28 and 38).
    qWarning("style: %s", qUtf8Printable(app.style()->objectName()));
    qWarning("palette Window %s WindowText %s",
             qUtf8Printable(app.palette().color(QPalette::Window).name()),
             qUtf8Printable(app.palette().color(QPalette::WindowText).name()));

    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    struct State {
        int provider;
        const char *stored;
        QString file;
    };
    const QList<State> states{{Settings::Ollama, "Ollama", QStringLiteral("144-anbieter-ollama.png")},
                              {Settings::OpenRouter, "OpenRouter", QStringLiteral("144-anbieter-openrouter.png")},
                              {Settings::OpenAi, "OpenAI", QStringLiteral("144-anbieter-openai.png")}};

    for (const State &state : states) {
        // **The setting first, then the page**: the page reads `[AI] Provider`
        // while it is built, which is what a freshly opened dialog does — and
        // it is the road the defect of #127 lived on. Written into the run's
        // own XDG_CONFIG_HOME above, never into the reader's.
        // **Through KSharedConfig::openConfig() and not through a KConfig of its
        // own.** The page reads the shared one, which keeps the file it parsed
        // at the first access — measured on 30.08.2026, where a separate
        // KConfig plus sync() left the second page reading `Ollama` off a copy
        // that was already in memory. The qFatal below is what caught it
        // instead of a plausible wrong picture.
        {
            KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
            group.writeEntry("Provider", QString::fromLatin1(state.stored));
            group.sync();
        }

        // A page of its own per state, so that every one of them comes up the
        // way a freshly opened dialog comes up.
        AiProviderPage page;
        // The width the page has in the dialog at its built-in size: 640 less
        // the page list. Printed with every picture, so the numbers below can
        // be held against a rectangle set from outside (finding 64).
        //
        // **The height is asked of the page and no longer a number of this
        // runner's own** (issue #39). A fixed 260 was enough while the page
        // carried one word-wrapped line; with the note of SPEC 7.5 beside it,
        // `resize()` was clamped to the layout's minimum — 359 — while the page
        // needs 410 at this width, and the picture came out with the type
        // clipped along every wrapped row. That reads as a fault of the product
        // and is one of the runner: `KConfigDialog` gives the page its
        // sizeHint, which is taller still. Read the numbers beside the picture.
        page.resize(470, page.heightForWidth(470));
        page.show();
        if (!QTest::qWaitForWindowExposed(&page)) {
            qFatal("the page never reached the screen");
        }

        // Breeze animates the dot, so both the readback and the grab wait for
        // the animation to finish (finding 43). Nothing is checked by hand here
        // any more — the readback below is what says the stored value arrived.
        QTest::qWait(400);

        const auto *box = page.findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
        if (box->findChildren<QRadioButton *>().at(state.provider)->isChecked() != true) {
            qFatal("the stored provider did not reach the buttons");
        }

        // The note of SPEC 7.5 as it really resolved. That is the one line which
        // says whether the message catalogue was found — an English wording
        // under a German LANGUAGE is a runner without its domain or without its
        // catalogue, not a page without its note. It took the place of the
        // "not connected yet" sentence #127 put here, which #38 and #39 removed
        // with the lock it explained.
        //
        // Asked by object name and fatal when it is missing: a search over
        // every QLabel for a word out of the text prints nothing when it finds
        // nothing, and the run then ends with 0 and without the one line it is
        // here for (CLAUDE.md, findings 31 and 59). **The text is printed
        // whether the row is shown or not**, and the readback of `shown` beside
        // it is what tells "nothing to say" from "said and wrongly hidden"
        // (finding 79).
        const auto *sentence = page.findChild<QLabel *>(QStringLiteral("openAiNote"));
        if (sentence == nullptr) {
            qFatal("the page carries no note named openAiNote");
        }
        qWarning("%s  OpenAI note: %s", qUtf8Printable(state.file), qUtf8Printable(sentence->text()));
        qWarning("%s  page %dx%d logical, ratio %.1f, sizeHint %dx%d, minimum %dx%d,"
                 " heightForWidth(470) %d",
                 qUtf8Printable(state.file),
                 page.width(),
                 page.height(),
                 page.devicePixelRatioF(),
                 page.sizeHint().width(),
                 page.sizeHint().height(),
                 page.minimumSizeHint().width(),
                 page.minimumSizeHint().height(),
                 page.heightForWidth(470));
        report(state.file, page);
        // The rows "Ollama address", "Language model" and "Embedding model" are
        // **empty in every picture**, and that is this runner and not the
        // product: they are `kcfg_` widgets that KConfigDialogManager fills,
        // and the page stands here without the dialog that owns one. In the
        // built settings they carry the stored address and the two models.
        // What this run is about is which of them is **shown**.
        shoot(page, directory, state.file);
    }

    // The control that has to come out different: the three pictures above show
    // the rows set from the **stored** value, this one shows them still
    // following a live switch. Both roads matter — the defect of #127 was that
    // only this second one worked (CLAUDE.md, verification stance).
    {
        KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
        group.writeEntry("Provider", QStringLiteral("Ollama"));
        group.sync();
    }
    AiProviderPage control;
    control.resize(470, control.heightForWidth(470));
    control.show();
    if (!QTest::qWaitForWindowExposed(&control)) {
        qFatal("the control page never reached the screen");
    }
    auto *controlBox = control.findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
    const QList<QRadioButton *> controlButtons = controlBox->findChildren<QRadioButton *>();
    controlButtons.at(Settings::Ollama)->setChecked(true);
    QTest::qWait(50);
    report(QStringLiteral("control, Ollama checked"), control);
    controlButtons.at(Settings::OpenRouter)->setChecked(true);
    QTest::qWait(50);
    report(QStringLiteral("control, switched to openrouter.ai by hand"), control);
    // And on to the button #39 unlocked, so the live switch is walked for the
    // provider whose rows are new.
    controlButtons.at(Settings::OpenAi)->setChecked(true);
    QTest::qWait(50);
    report(QStringLiteral("control, switched to OpenAI by hand"), control);

    return 0;
}
