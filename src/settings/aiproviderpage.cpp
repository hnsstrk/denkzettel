#include "settings/aiproviderpage.h"

#include "analysis/ollamaprovider.h"
#include "settings/settings.h"

#include <KColorScheme>
#include <KLocalizedString>

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
/** An editable box that offers the default of SPEC 7.1 and takes anything. */
QComboBox *modelBox(QWidget *parent, const QString &suggestion)
{
    auto *box = new QComboBox(parent);
    box->setEditable(true);
    // The one entry is the default the SPEC names, so the way back to it is a
    // click rather than a typing exercise. What Ollama actually holds is a
    // question to the server (/api/tags) and belongs to whoever needs the list.
    box->addItem(suggestion);
    box->setInsertPolicy(QComboBox::NoInsert);
    return box;
}
}

AiProviderPage::AiProviderPage(QWidget *parent)
    : QWidget(parent)
    , m_chatModel(modelBox(this, QString(ollama::DefaultChatModel)))
    , m_ollamaUrl(new QLineEdit(this))
    , m_embeddingModel(modelBox(this, QString(ollama::DefaultEmbeddingModel)))
    , m_test(new QPushButton(i18n("Test connection"), this))
    , m_result(new QLabel(this))
    , m_provider(new OllamaProvider(this))
{
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    layout->addLayout(form);

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
    auto *ollama = new QRadioButton(i18n("Ollama"), provider);
    choices->addWidget(ollama);
    choices->addWidget(new QRadioButton(i18n("openrouter.ai"), provider));
    choices->addWidget(new QRadioButton(i18n("OpenAI"), provider));
    form->addRow(i18n("Provider:"), provider);

    // ponytail: the key is read here and stored nowhere — closing the dialog
    // loses it. Ceiling: nothing can authenticate against openrouter or OpenAI
    // yet, which is why no code asks for the value. Upgrade path: `KeyStore`,
    // which issue #37 built for exactly this field — this page wires itself to
    // it in #127. It deliberately has no `kcfg_` name — an API key in
    // denkzettelrc is the one thing SPEC 5.2 forbids outright.
    auto *apiKey = new QLineEdit(this);
    apiKey->setEchoMode(QLineEdit::Password);
    const int apiKeyRow = form->rowCount();
    form->addRow(i18n("API key:"), apiKey);

    m_chatModel->setObjectName(QStringLiteral("kcfg_ChatModel"));
    form->addRow(i18n("Language model:"), m_chatModel);

    // Its own always-usable row rather than an indent under the Ollama button:
    // SPEC 7.1 has every embedding coming from Ollama in v1, so the address is
    // needed under the other two providers as well and must not sit behind an
    // unchosen radio button.
    m_ollamaUrl->setObjectName(QStringLiteral("kcfg_OllamaUrl"));
    form->addRow(i18n("Ollama address:"), m_ollamaUrl);

    m_embeddingModel->setObjectName(QStringLiteral("kcfg_EmbeddingModel"));
    form->addRow(i18n("Embedding model:"), m_embeddingModel);

    // The action at the end of its row, the result as a small coloured line
    // right underneath (wireframe 1d:172–173). Not a KMessageWidget: that one
    // would push the form up and down by its own height on every check.
    auto *actions = new QHBoxLayout;
    actions->addStretch();
    actions->addWidget(m_test);
    form->addRow(actions);

    const QFont bodyFont = font();
    if (bodyFont.pointSizeF() > 0) {
        QFont small = bodyFont;
        small.setPointSizeF(bodyFont.pointSizeF() * 0.9);
        m_result->setFont(small);
    }
    m_result->setWordWrap(true);
    form->addRow(m_result);

    // Nothing on these pages wants to grow (UX decision of 29.08.2026), so the
    // room a resized window brings goes underneath the form.
    layout->addStretch();

    // The key is only asked for where SPEC 7.1 has one. No button is checked
    // before the dialog's manager reads the setting, so the first state always
    // arrives through this signal — the line below only covers the moment
    // between the constructor and that read, in which the page is not shown.
    connect(ollama, &QRadioButton::toggled, this, [form, apiKeyRow](bool chosen) {
        form->setRowVisible(apiKeyRow, !chosen);
    });
    form->setRowVisible(apiKeyRow, false);

    connect(m_test, &QPushButton::clicked, this, &AiProviderPage::startTest);
    connect(m_provider, &AiProvider::connectionTested, this, &AiProviderPage::showResult);
}

void AiProviderPage::startTest()
{
    // What is on the form, not what is in the file: the button is pressed to
    // find out whether the address just typed in works, and Apply has not
    // necessarily been pressed yet.
    //
    // ponytail: always the Ollama backend. Ceiling: with openrouter or OpenAI
    // chosen, what is measured is still the local server — and in v1 that is
    // where every embedding comes from anyway (SPEC 7.1), while neither of the
    // other two has an AiProvider implementation at all yet. Upgrade path: ask
    // the chosen provider once those exist.
    m_provider->setUrl(QUrl(m_ollamaUrl->text()));
    m_provider->setChatModel(m_chatModel->currentText());
    m_provider->setEmbeddingModel(m_embeddingModel->currentText());

    m_test->setEnabled(false);
    m_result->clear();
    m_provider->testConnection();
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

    // **What an unreachable Ollama costs, said where the user finds out that it
    // is unreachable** (SPEC 7.1, issue #28). Every embedding of v1 comes from
    // this one server, so with it away the topic bundles of SPEC 7.3 stop
    // coming — while the classification goes on through whichever provider is
    // chosen above. The test only ever talks to Ollama, so any error here is
    // that case; the second half of the criterion, the same news in the tray
    // tooltip, is issue #118.
    m_result->setText(error.isEmpty()
                          ? i18n("Connection is up · chat %1 ms · embedding %2 ms",
                                 chatMilliseconds,
                                 embedMilliseconds)
                          : i18n("%1\nEvery embedding comes from Ollama: without it there are no topic bundles."
                                 " The classification keeps running through the provider chosen above.",
                                 error));
}
