#include "settings/aiproviderpage.h"

#include "analysis/ollamaprovider.h"
#include "analysis/openaicompatibleprovider.h"
#include "settings/settings.h"
#include "store/keystore.h"

#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
/**
 * An editable box that offers the default of SPEC 7.1 and takes anything.
 *
 * An empty `suggestion` means the service has no default to offer — which for
 * openrouter is a decision and not a gap (customer, 30.08.2026). The box then
 * carries no entry at all, so nothing can be picked that nobody chose.
 */
QComboBox *modelBox(QWidget *parent, const QString &suggestion)
{
    auto *box = new QComboBox(parent);
    box->setEditable(true);
    // The one entry is the default the SPEC names, so the way back to it is a
    // click rather than a typing exercise. What Ollama actually holds is a
    // question to the server (/api/tags) and belongs to whoever needs the list.
    if (!suggestion.isEmpty()) {
        box->addItem(suggestion);
    }
    box->setInsertPolicy(QComboBox::NoInsert);
    return box;
}

/** A quiet line under a row, built the way the result line has always been. */
QLabel *smallLine(QWidget *parent)
{
    auto *line = new QLabel(parent);
    const QFont body = parent->font();
    if (body.pointSizeF() > 0) {
        QFont small = body;
        small.setPointSizeF(body.pointSizeF() * 0.9);
        line->setFont(small);
    }
    line->setWordWrap(true);
    // **And the layout has to be told that the height depends on the width.**
    // A word-wrapped QLabel answers `minimumSizeHint()` with roughly one line,
    // and QFormLayout believes it: the rows then get less room than the text
    // needs and the page draws them **over each other**. Measured 30.08.2026 on
    // #39, where the four-line note of SPEC 7.5 pushed the page past its budget
    // and the picture showed the provider buttons and the note overlapping —
    // the same page one row shorter came out clean, so it read like a fault of
    // the new row and is a property of every wrapped label on the page.
    // `heightForWidth` is what makes the layout ask the label how tall it has
    // to be at the width it is getting.
    QSizePolicy height = line->sizePolicy();
    height.setHeightForWidth(true);
    line->setSizePolicy(height);
    return line;
}

/**
 * The provider that stands in `denkzettelrc`, as a Settings::Provider value.
 *
 * Read out of the file and not off the buttons, and it is what the constructor
 * sets the rows from: the page is built before KConfigDialogManager fills it,
 * and `providershots` builds it with no manager at all. The names are the ones
 * settings.cpp declares as the enum's choices.
 */
int storedProvider()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    const QString name = group.readEntry("Provider", QStringLiteral("Ollama"));
    if (name == QLatin1String("OpenRouter")) {
        return Settings::OpenRouter;
    }
    if (name == QLatin1String("OpenAI")) {
        return Settings::OpenAi;
    }
    return Settings::Ollama;
}

/**
 * The remote service behind a provider choice, or nullptr for Ollama.
 *
 * The one place this page turns a button index into a service, so the key row,
 * the wallet entry, the model row and the sentence under the result all name
 * the same one. Ollama is the exception and stays nullptr: it needs no key and
 * its address is a setting, which is exactly what the row rule is about.
 */
const AiService *serviceOf(int provider)
{
    if (provider == Settings::OpenRouter) {
        return &openrouter::Service;
    }
    if (provider == Settings::OpenAi) {
        return &openai::Service;
    }
    return nullptr;
}

/** The KeyStore entry name of a provider, empty for Ollama. */
QString keyNameOf(int provider)
{
    const AiService *service = serviceOf(provider);
    return service == nullptr ? QString() : QString(service->keyName);
}
}

AiProviderPage::AiProviderPage(QWidget *parent)
    : QWidget(parent)
    , m_form(new QFormLayout)
    , m_ollamaButton(nullptr)
    , m_openRouterButton(nullptr)
    , m_openAiButton(nullptr)
    , m_apiKey(new QLineEdit(this))
    , m_keyState(smallLine(this))
    , m_chatModel(modelBox(this, QString(ollama::DefaultChatModel)))
    // Empty, and it carries no suggestion to fall back to: SPEC 7.1 names no
    // model for this service on purpose (customer decision 30.08.2026), so
    // there is nothing to offer and the placeholder says what to do instead.
    , m_openRouterModel(modelBox(this, QString()))
    // The same for OpenAI, and for the same customer decision: SPEC 7.1 names
    // no model for either remote service.
    , m_openAiModel(modelBox(this, QString()))
    , m_ollamaUrl(new QLineEdit(this))
    , m_embeddingModel(modelBox(this, QString(ollama::DefaultEmbeddingModel)))
    // Empty for the reason the two chat model boxes above are empty, and one
    // degree stronger: an embedding run touches every note, repeatedly (SPEC
    // 7.1, issue #130).
    , m_openRouterEmbeddingModel(modelBox(this, QString()))
    , m_openAiEmbeddingModel(modelBox(this, QString()))
    , m_thresholdNote(smallLine(this))
    , m_test(new QPushButton(i18n("Test connection"), this))
    , m_result(smallLine(this))
    , m_openAiNote(smallLine(this))
    , m_remoteTextNote(smallLine(this))
    , m_ollama(new OllamaProvider(this))
    , m_openRouter(new OpenAiCompatibleProvider(openrouter::Service, this))
    , m_openAi(new OpenAiCompatibleProvider(openai::Service, this))
{
    // **The two remote backends are one class**, so a check looking for one of
    // them by type would take whichever was built first and never know it was
    // measuring the wrong service (issue #39). These names are what
    // `settingstest` points at its stand-in.
    m_openRouter->setObjectName(QStringLiteral("openRouterProvider"));
    m_openAi->setObjectName(QStringLiteral("openAiProvider"));

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(m_form);

    // A group box named `kcfg_<key>` whose direct children are auto-exclusive
    // buttons is what KConfigDialogManager stores an int setting from: it keeps
    // the INDEX of the checked button (kconfigdialogmanager.cpp:151–170, 465,
    // 503). **The index is the order the buttons are created in**, so the three
    // below have to stand in the order of Settings::Provider — a button
    // inserted between them moves every value after it and breaks the stored
    // setting without a sound.
    //
    // Flat and without a title: the box is a container for the setting, not a
    // heading. "Provider:" is the ordinary form label of the row, and a title
    // on the box would be the second label of the same row (UX, 29.08.2026).
    auto *provider = new QGroupBox(this);
    provider->setObjectName(QStringLiteral("kcfg_Provider"));
    provider->setFlat(true);
    auto *choices = new QVBoxLayout(provider);
    choices->setContentsMargins(0, 0, 0, 0);
    // Both, and the second one is measured: a flat titleless group box draws no
    // frame under Breeze, but it still keeps the style's margin free. Without
    // this line the three buttons sit 3 device pixels lower and everything
    // below the row 6 (2 and 4 logical pixels at the user's scaling of 1.5) —
    // with it, the page comes out bit for bit as it did before the group box
    // took the place of a plain column (29.08.2026, finding 39's method).
    provider->setContentsMargins(0, 0, 0, 0);
    m_ollamaButton = new QRadioButton(i18n("Ollama"), provider);
    choices->addWidget(m_ollamaButton);
    m_openRouterButton = new QRadioButton(i18n("openrouter.ai"), provider);
    choices->addWidget(m_openRouterButton);
    // The access route in the label, because that is what tells the two OpenAI
    // routes apart: SPEC 7.5 has a platform API key and expressly not "Sign in
    // with ChatGPT", which hands out name, e-mail address and profile picture
    // and no model access at all (customer's request, issue #127).
    m_openAiButton = new QRadioButton(i18n("OpenAI (API key)"), provider);
    choices->addWidget(m_openAiButton);
    m_form->addRow(i18n("Provider:"), provider);

    // **All three buttons take clicks now.** #127 locked the two remote ones
    // and put a sentence under them saying so, because main.cpp built the same
    // Ollama provider behind all three; #38 unlocked openrouter and this story
    // unlocks OpenAI. Both the lock and the sentence go with it — a page that
    // says a provider is not connected while it is, is the same untruth the
    // other way round (wireframe supplement 1d, UX decision 30.08.2026).

    // **What the choice above costs, said where the choice is made** (issue
    // #144). SPEC 7.1 requires exactly this, and requires it here: "What the
    // choice costs is said where it is made, on the settings page and not in a
    // manual: every note leaves the machine". Until this row the page said
    // where the **key** is kept and never where the **text** goes — the one
    // claim about Denkzettel a reader is most likely to have taken from the
    // README, where the same overpromise stood until 03.09.2026.
    //
    // Directly under the provider row and above everything else, because that
    // is what the criterion "visible without scrolling" means on a page whose
    // rows come and go: a consequence read after the fields it applies to is
    // read too late. Measured in the built 640x480 dialog rather than on the
    // page, which is where the question lives — KConfigDialog wraps every page
    // in a QScrollArea, and under OpenAI the page is taller than its viewport.
    // The sentence stands at viewport y 90..124 against a fold at 399.
    //
    // The text is **set in showRowsOfTheChosenProvider() and cleared there**,
    // not written once here like the two notes below. A hidden label that keeps
    // its last sentence answers the same thing for "nothing to say" as for
    // "said it and wrongly hidden" (CLAUDE.md, finding 79), and this is the one
    // line on the page whose absence is itself a statement.
    //
    // The name says when it shows rather than what it is about (UX, 04.09.2026):
    // it is the note of the **remote** providers, and the picture runner asks
    // the label by this name so a run whose catalogue was not found says so
    // instead of printing nothing (findings 31 and 59).
    m_remoteTextNote->setObjectName(QStringLiteral("remoteTextNote"));
    m_remoteTextNoteRow = m_form->rowCount();
    m_form->addRow(m_remoteTextNote);

    // **Why there is no "Sign in with ChatGPT"**, and the wording is not
    // invented here: SPEC 7.5 settled it on the research of 2026-07-31, and
    // #127 wrote the sentence down in the wireframe supplement so it would not
    // be written twice. It is shown only under OpenAI — a note about a provider
    // nobody has chosen is the functionless place on the page that #127 removed.
    //
    // The object name is what the picture runner asks the sentence by, so that
    // a run whose catalogue was not found says so instead of printing nothing
    // (CLAUDE.md, findings 31 and 59).
    m_openAiNote->setObjectName(QStringLiteral("openAiNote"));
    m_openAiNote->setText(
        i18n("A route over \"Sign in with ChatGPT\" is not possible: the procedure hands out name,"
             " e-mail address and profile picture, no model access. Denkzettel therefore needs an"
             " API key from the OpenAI platform account."));
    m_openAiNoteRow = m_form->rowCount();
    m_form->addRow(m_openAiNote);

    // **The key goes into KWallet and never into denkzettelrc** (SPEC 5.2), so
    // this field has no `kcfg_` name — the dialog's manager would take one for
    // a stored setting. What carries it is save() below, over the routes
    // KConfigDialog offers for exactly this case. **Write-only** — nothing here
    // ever reads a key back, see the class comment; the placeholder is what
    // says so, because an empty password field otherwise reads as "no key".
    //
    // The object name is what the picture runner and settingstest ask the row
    // by; it must not begin with `kcfg_`, for the reason above.
    m_apiKey->setObjectName(QStringLiteral("apiKey"));
    m_apiKey->setEchoMode(QLineEdit::Password);
    m_apiKey->setPlaceholderText(i18n("Kept in KWallet · empty keeps the stored key"));
    m_apiKeyRow = m_form->rowCount();
    m_form->addRow(i18n("API key:"), m_apiKey);

    // The third state of the password store, and the note SPEC 5.2's behaviour
    // owes the user: the wallet may be standing in front of its own password
    // dialog, and without this line that whole time looks like a field that
    // stayed empty (keystore.h). It carries the wallet's error just as well.
    m_keyState->setObjectName(QStringLiteral("keyState"));
    m_keyStateRow = m_form->rowCount();
    m_form->addRow(m_keyState);

    m_chatModel->setObjectName(QStringLiteral("kcfg_ChatModel"));
    m_chatModelRow = m_form->rowCount();
    m_form->addRow(i18n("Language model:"), m_chatModel);

    // The same row for the other service, and a widget of its own because the
    // two names are two settings: `ChatModel` is what OllamaProvider asks its
    // server for (settings.cpp says why). One of the two is shown at a time.
    m_openRouterModel->setObjectName(QStringLiteral("kcfg_OpenRouterModel"));
    // The field is empty on the first switch, so it has to say what belongs in
    // it — the models the service offers are #128/#129 and this does not wait
    // for them.
    m_openRouterModel->lineEdit()->setPlaceholderText(
        i18n("For example openai/gpt-4o-mini · openrouter.ai/models"));
    // **And it has to have room for that.** A combo box sizes itself to its
    // content, and this one has none — measured 30.08.2026 in the picture,
    // where the field came out 122 device pixels wide and the placeholder read
    // "Zum B…". The Ollama box beside it goes on sizing to the entry it has;
    // this one is a free-text field for a name like
    // `anthropic/claude-3.5-sonnet` and takes the width the line edits above it
    // take.
    m_openRouterModel->setSizePolicy(QSizePolicy::Expanding,
                                     m_openRouterModel->sizePolicy().verticalPolicy());
    m_openRouterModelRow = m_form->rowCount();
    m_form->addRow(i18n("Language model:"), m_openRouterModel);

    // The third of the same row, and a widget of its own for the reason the
    // second one is: `OpenAiModel` and `OpenRouterModel` are two settings, and
    // one shared field would carry whichever name was typed last to the other
    // service (settings.cpp says it at the two items). One of the three is
    // shown at a time.
    m_openAiModel->setObjectName(QStringLiteral("kcfg_OpenAiModel"));
    m_openAiModel->lineEdit()->setPlaceholderText(i18n("For example gpt-4o-mini · platform.openai.com/docs/models"));
    // The width, for the reason the row above carries it: an empty editable
    // combo box sizes itself to nothing and elides its own placeholder.
    m_openAiModel->setSizePolicy(QSizePolicy::Expanding, m_openAiModel->sizePolicy().verticalPolicy());
    m_openAiModelRow = m_form->rowCount();
    m_form->addRow(i18n("Language model:"), m_openAiModel);

    // **The address stands under Ollama and nowhere else**, which is the row
    // rule of 30.08.2026 as it was written. #38 kept it under all three on
    // purpose and said so: until this story the embedding run asked this
    // address whatever was chosen above, so hiding it would have hidden the one
    // field the topic bundles depend on. That premise is gone — the chosen
    // service answers the embedding call — and the sentence that used to stand
    // under the two rows goes with it, at the same moment and on the same
    // condition (issue #130, the criterion inherited from #38).
    m_ollamaUrl->setObjectName(QStringLiteral("kcfg_OllamaUrl"));
    m_ollamaUrlRow = m_form->rowCount();
    m_form->addRow(i18n("Ollama address:"), m_ollamaUrl);

    // Three boxes for one row, the way the language model above has three: the
    // three keys are three settings, and one shared field would carry an Ollama
    // model name to openrouter — where `baai/bge-m3` is what the list holds and
    // `bge-m3` is a name their server does not know. One of the three is shown
    // at a time.
    m_embeddingModel->setObjectName(QStringLiteral("kcfg_EmbeddingModel"));
    m_embeddingModelRow = m_form->rowCount();
    m_form->addRow(i18n("Embedding model:"), m_embeddingModel);

    m_openRouterEmbeddingModel->setObjectName(QStringLiteral("kcfg_OpenRouterEmbeddingModel"));
    m_openRouterEmbeddingModel->lineEdit()->setPlaceholderText(
        i18n("For example baai/bge-m3 · openrouter.ai/models"));
    // The width, for the reason the chat model rows carry it: an empty editable
    // combo box sizes itself to nothing and elides its own placeholder.
    m_openRouterEmbeddingModel->setSizePolicy(QSizePolicy::Expanding,
                                              m_openRouterEmbeddingModel->sizePolicy().verticalPolicy());
    m_openRouterEmbeddingModelRow = m_form->rowCount();
    m_form->addRow(i18n("Embedding model:"), m_openRouterEmbeddingModel);

    m_openAiEmbeddingModel->setObjectName(QStringLiteral("kcfg_OpenAiEmbeddingModel"));
    m_openAiEmbeddingModel->lineEdit()->setPlaceholderText(
        i18n("For example text-embedding-3-small · platform.openai.com/docs/models"));
    m_openAiEmbeddingModel->setSizePolicy(QSizePolicy::Expanding,
                                          m_openAiEmbeddingModel->sizePolicy().verticalPolicy());
    m_openAiEmbeddingModelRow = m_form->rowCount();
    m_form->addRow(i18n("Embedding model:"), m_openAiEmbeddingModel);

    // **What the threshold of SPEC 7.3 was measured against, said where the
    // model is chosen** (PO decision 04.09.2026, issue #130). One value serves
    // every provider because the program can calibrate nothing about a model
    // the user names freely — there is no reference corpus in it — so what is
    // owed the user is the sentence, not a slider.
    //
    // Under all three and not only under the remote two: the field is a free
    // one under Ollama as well, and a condition here would be a second thing to
    // forget beside the address row.
    m_thresholdNote->setObjectName(QStringLiteral("thresholdNote"));
    m_thresholdNote->setText(
        i18n("The topic bundles are tuned to bge-m3. Another model may group more"
             " freely or hardly at all. Changing the model marks every note to be"
             " embedded again, at most 50 per run."));
    m_form->addRow(m_thresholdNote);

    // The action at the end of its row, the result as a small coloured line
    // right underneath (wireframe 1d:172–173). Not a KMessageWidget: that one
    // would push the form up and down by its own height on every check.
    auto *actions = new QHBoxLayout;
    actions->addStretch();
    // Both carry an object name so a check can press the one and read the
    // other: what the button measures and what this line then claims have to
    // agree, and that is an acceptance criterion of its own (issue #127, moved
    // to #38). A search over every QPushButton of the dialog would find the
    // button box's as well.
    m_test->setObjectName(QStringLiteral("testConnection"));
    actions->addWidget(m_test);
    m_form->addRow(actions);

    m_result->setObjectName(QStringLiteral("testResult"));
    m_form->addRow(m_result);

    // Nothing on these pages wants to grow (UX decision of 29.08.2026), so the
    // room a resized window brings goes underneath the form.
    layout->addStretch();

    // **All three buttons, and the state is read off the buttons.** Which of
    // them the manager checks when it reads the setting is not knowable here,
    // and the one thing that is certain is that it does not toggle the others
    // — that was the whole defect (see the class comment).
    for (const QRadioButton *button : {m_ollamaButton, m_openRouterButton, m_openAiButton}) {
        connect(button, &QRadioButton::toggled, this, [this](bool) {
            showRowsOfTheChosenProvider();
        });
    }
    // And the stored value for the state before any of that happens: the
    // manager has not run yet, and on a page built without one it never will.
    // Where the manager does run it reads the same file and checks the same
    // button, so nothing moves and Apply stays grey.
    const int stored = storedProvider();
    m_ollamaButton->setChecked(stored == Settings::Ollama);
    m_openRouterButton->setChecked(stored == Settings::OpenRouter);
    m_openAiButton->setChecked(stored == Settings::OpenAi);
    showRowsOfTheChosenProvider();

    // Only what the user typed counts as a change. `textEdited` and not
    // `textChanged`: the second one fires for a programmatic fill as well, and
    // the Apply button would light up on an opening nobody touched.
    connect(m_apiKey, &QLineEdit::textEdited, this, [this] {
        m_keyEdited = true;
        m_keyEditedFor = keyNameOf(chosenProvider());
        Q_EMIT changed();
    });

    connect(KeyStore::self(), &KeyStore::waitingForWallet, this, [this](const QString &provider) {
        if (provider == keyNameOf(chosenProvider())) {
            sayAboutTheKey(i18n("Waiting for the password store…"));
        }
    });
    // The one answer this page listens for. Nothing is ever read back out of
    // the wallet here (see the class comment), so a failed **write** is the
    // only outcome the user has to be told about — and told they must be, or a
    // key that never arrived looks exactly like one that did.
    connect(KeyStore::self(), &KeyStore::keyStored, this, [this](const QString &provider, const QString &error) {
        if (provider != keyNameOf(chosenProvider())) {
            return;
        }
        sayAboutTheKey(error.isEmpty() ? i18n("The key is in the password store.") : error);
    });

    connect(m_test, &QPushButton::clicked, this, &AiProviderPage::startTest);
    connect(m_ollama, &AiProvider::connectionTested, this, &AiProviderPage::showResult);
    connect(m_openRouter, &AiProvider::connectionTested, this, &AiProviderPage::showResult);
    connect(m_openAi, &AiProvider::connectionTested, this, &AiProviderPage::showResult);
}

OpenAiCompatibleProvider *AiProviderPage::chosenRemote() const
{
    if (m_openRouterButton->isChecked()) {
        return m_openRouter;
    }
    if (m_openAiButton->isChecked()) {
        return m_openAi;
    }
    return nullptr;
}

int AiProviderPage::chosenProvider() const
{
    if (m_openRouterButton->isChecked()) {
        return Settings::OpenRouter;
    }
    if (m_openAiButton->isChecked()) {
        return Settings::OpenAi;
    }
    return Settings::Ollama;
}

void AiProviderPage::showRowsOfTheChosenProvider()
{
    const int chosen = chosenProvider();
    const bool needsKey = !keyNameOf(chosen).isEmpty();
    // Both remote services now, and that is the whole of the row rule: what
    // used to read "openrouter" reads "not Ollama" (issue #39).
    const AiService *service = serviceOf(chosen);
    const bool remoteChat = service != nullptr;

    // **The sentence and its row go together** (issue #144, CLAUDE.md finding
    // 79): under Ollama there is nothing to say, and the label is emptied
    // rather than merely hidden — otherwise a readback cannot tell "nothing to
    // report" from "reported and wrongly hidden", and for a statement about
    // where the note text goes those are the two states that matter most.
    //
    // **The wording is the UX expert's, not the implementer's** (04.09.2026,
    // an acceptance criterion of #144), and "leaves the machine" is not a
    // fresh phrase: SPEC 7.1 and SPEC 14 both use it, both times as the price
    // of a choice rather than as a warning. A third formulation for one fact
    // is one too many.
    //
    // **What it deliberately does not say** is that nothing leaves the machine
    // under Ollama. `kcfg_OllamaUrl` is free text, so a user who points it at
    // another host sends the text off just the same — an assurance a field on
    // this very page can break is worse than none.
    //
    // The service is named rather than spelled into the sentence, for the
    // reason the result line below names it: with two remote backends a fixed
    // wording would tell the user about the service they did not choose. It
    // also makes the readback come out **three** ways instead of two, so a row
    // shown under the wrong provider cannot pass for the right one (finding 10).
    //
    // **Every note, and not only the ones being classified** — the two words
    // #144 left in on purpose came out with #130, which makes the chosen
    // service answer the embedding as well: an embedding run touches every
    // note, after every edit again. That is SPEC 7.1's "every note leaves the
    // machine" word for word.
    m_remoteTextNote->setText(
        remoteChat ? i18n("The text of every note leaves the machine and goes to %1.", QString(service->name))
                   : QString());
    m_form->setRowVisible(m_remoteTextNoteRow, remoteChat);

    m_form->setRowVisible(m_apiKeyRow, needsKey);
    // The note about "Sign in with ChatGPT" belongs to the one provider it is
    // about (SPEC 7.5), and it is shown with the key row rather than instead of
    // it: it says why the field beside it wants a key and not a login.
    m_form->setRowVisible(m_openAiNoteRow, chosen == Settings::OpenAi);
    // Only once it has something to say. An empty label still takes its row's
    // height, which stood in the picture as a hand's width of nothing between
    // the key field and the model row (measured 30.08.2026) — and a line that
    // keeps its last sentence while hidden answers the same for "nothing to
    // report" as for "reported and wrongly hidden" (CLAUDE.md, finding 79).
    m_keyState->clear();
    m_form->setRowVisible(m_keyStateRow, false);
    // The model row of the chosen service, and only that one.
    m_form->setRowVisible(m_chatModelRow, !remoteChat);
    m_form->setRowVisible(m_openRouterModelRow, chosen == Settings::OpenRouter);
    m_form->setRowVisible(m_openAiModelRow, chosen == Settings::OpenAi);
    // The address of the local service, under the local service and nowhere
    // else (issue #130).
    m_form->setRowVisible(m_ollamaUrlRow, !remoteChat);
    // And the embedding model row of the chosen service, the way the language
    // model row above works.
    m_form->setRowVisible(m_embeddingModelRow, !remoteChat);
    m_form->setRowVisible(m_openRouterEmbeddingModelRow, chosen == Settings::OpenRouter);
    m_form->setRowVisible(m_openAiEmbeddingModelRow, chosen == Settings::OpenAi);

    // Emptied on a switch, because what stands in it belongs to the provider
    // that was chosen when it was typed: applied under the other one it would
    // put one service's key into the other's wallet entry.
    if (m_keyEdited && keyNameOf(chosen) != m_keyEditedFor) {
        m_apiKey->clear();
        m_keyEdited = false;
    }
}

void AiProviderPage::sayAboutTheKey(const QString &sentence)
{
    m_keyState->setText(sentence);
    m_form->setRowVisible(m_keyStateRow, !sentence.isEmpty());
}

void AiProviderPage::save()
{
    // Only what the user typed. Without this a dialog opened while the wallet
    // was locked — the field empty, the key never read — would write that empty
    // field back over a key that is perfectly good.
    if (!m_keyEdited) {
        return;
    }
    const QString name = keyNameOf(chosenProvider());
    if (name.isEmpty()) {
        return;
    }

    if (m_apiKey->text().isEmpty()) {
        KeyStore::self()->removeKey(name);
    } else {
        KeyStore::self()->storeKey(name, m_apiKey->text());
    }
    m_keyEdited = false;
}

bool AiProviderPage::hasChanged() const
{
    return m_keyEdited;
}

void AiProviderPage::startTest()
{
    // What is on the form, not what is in the file: the button is pressed to
    // find out whether what was just typed in works, and Apply has not
    // necessarily been pressed yet. That holds for the key as well — otherwise
    // the check would measure a key out of the wallet while the user is looking
    // at another one.
    m_test->setEnabled(false);
    m_result->clear();

    if (OpenAiCompatibleProvider *remote = chosenRemote(); remote != nullptr) {
        const bool openRouter = chosenProvider() == Settings::OpenRouter;
        remote->setChatModel(openRouter ? m_openRouterModel->currentText() : m_openAiModel->currentText());
        // **Both models, because the test makes both calls under every provider
        // now** (issue #130): the embedding half is what the topic bundles hang
        // on, and a test that skipped it would leave the user's one way of
        // checking it unmeasured.
        remote->setEmbeddingModel(openRouter ? m_openRouterEmbeddingModel->currentText()
                                             : m_openAiEmbeddingModel->currentText());
        // **Only a key that was typed**, and this line is the one the review of
        // 30.08.2026 found: the field is write-only and therefore empty on
        // every opening, so handing its text over unconditionally wiped the
        // stored key and the user read "openrouter.ai refused the request" over
        // an installation that was perfectly fine. Measured, and it comes out
        // different: 0 bytes of key on the wire with the field untouched, 32
        // with one typed.
        //
        // Untouched, nothing is set and the provider fetches the stored key
        // from KeyStore itself — which is what its own placeholder promises and
        // what KeyStore's rule allows: **a key press may open the wallet, the
        // opening of a dialog may not.**
        if (m_keyEdited) {
            remote->setKey(m_apiKey->text());
        }
        remote->testConnection();
        return;
    }

    m_ollama->setUrl(QUrl(m_ollamaUrl->text()));
    m_ollama->setChatModel(m_chatModel->currentText());
    m_ollama->setEmbeddingModel(m_embeddingModel->currentText());
    m_ollama->testConnection();
}

void AiProviderPage::showResult(qint64 chatMilliseconds, qint64 embedMilliseconds, const QString &error)
{
    m_test->setEnabled(true);

    // KColorScheme and not the palette: the two roles this line needs —
    // PositiveText and NegativeText — do not exist in QPalette at all.
    const KColorScheme scheme(QPalette::Normal, KColorScheme::View);
    const KColorScheme::ForegroundRole role =
        error.isEmpty() ? KColorScheme::PositiveText : KColorScheme::NegativeText;
    QPalette colours = m_result->palette();
    colours.setColor(QPalette::WindowText, scheme.foreground(role).color());
    m_result->setPalette(colours);

    if (!error.isEmpty()) {
        // **The sentence names the service that answered** and is left as the
        // backend wrote it, because the backend's own error sentences do that —
        // "openrouter.ai refused the request" and "Ollama could not be reached"
        // are not the same news, and a missing local service must not read like
        // a remote API failure (issue #38). The line that used to be appended
        // under Ollama went with issue #130: one service answers both calls
        // now, so "the classification keeps running" is no longer true of the
        // case this branch is in.
        m_result->setText(error);
        return;
    }

    m_result->setText(
        i18n("Connection is up · chat %1 ms · embedding %2 ms", chatMilliseconds, embedMilliseconds));
}
