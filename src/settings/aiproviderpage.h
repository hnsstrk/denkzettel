#pragma once

#include <QWidget>

class OllamaProvider;
class OpenAiCompatibleProvider;
class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

/**
 * The page "AI provider" of SPEC 13: which backend answers, which models it
 * uses, and whether it can be reached at all (SPEC 7.1).
 *
 * Everything that is persisted in `denkzettelrc` is a `kcfg_` widget, so the
 * dialog's manager loads, saves and greys the Apply button for those by itself.
 * **The API key is the one exception and it has to be**: SPEC 5.2 forbids a key
 * in a configuration file, so it goes through `KeyStore` into KWallet, and the
 * routes KConfigDialog offers for widgets its manager does not know about are
 * what carry it — the same road the page "Shortcuts" takes for a sequence that
 * lives in the shortcut service (issue #74). save() and hasChanged() below are
 * called by SettingsDialog and by nothing else.
 *
 * **The field is write-only, and that is KeyStore's own rule kept** (issue
 * #37): "the wallet is opened on the first request and not before, so a user
 * who never enters a key is never asked for a wallet password". Reading the key
 * back into the field would open the wallet on every opening of this dialog —
 * a password prompt for looking at a page — and it would put the secret on
 * screen and into every check that builds this page. An untouched field
 * therefore writes nothing and keeps what is stored; a field the user empties
 * removes the key.
 *
 * **Which rows are shown follows the chosen provider** (row rule, Product Owner
 * 30.08.2026, issues #38 and #39): the Ollama address only under Ollama, the
 * key row only under a provider that needs one, and the language model row of
 * the service that is going to answer. Since #39 all three buttons take clicks,
 * so the lock and the "not connected yet" sentence #127 put here are gone.
 *
 * **One of those rows says where the note text goes** (issue #144). SPEC 7.1
 * demands it and demands it here — "What the choice costs is said where it is
 * made, on the settings page and not in a manual" — and until #144 the page
 * said where the *key* is kept and never where the *text* goes. The sentence
 * therefore sits directly under the provider row, where the choice is made,
 * and its text is set and cleared with its row rather than written once: the
 * absence of this line is itself a statement, and a hidden label that keeps its
 * last sentence cannot say which of the two it is (CLAUDE.md, finding 79).
 *
 * **And it follows the stored value, not a toggle.**
 * Before the dialog's manager reads the setting no button is checked, so a
 * stored `Provider=OpenRouter` checks button 1 and toggles the Ollama button
 * not at all: a visibility hung on that one signal left the key field away for
 * exactly the users who need it (measured 30.08.2026 on #127). Every button's
 * toggled() is therefore listened to, and the state is read off the buttons
 * rather than off the signal's argument.
 */
class AiProviderPage : public QWidget
{
    Q_OBJECT

public:
    explicit AiProviderPage(QWidget *parent = nullptr);

    /** Writes the field into KWallet — only when the user typed in it. */
    void save();
    /** Whether the key field was edited, which is what Apply has to know. */
    bool hasChanged() const;

Q_SIGNALS:
    /** The dialog greys its Apply button by this, see hasChanged(). */
    void changed();

private:
    /**
     * The backend behind the checked button, or nullptr under Ollama.
     *
     * Both remote backends are built and only the chosen one is asked: they are
     * two accounts at two companies, and a connection test that reached the
     * wrong one would bill the wrong key (issue #39).
     */
    OpenAiCompatibleProvider *chosenRemote() const;
    /** Which of the three buttons is checked, as a Settings::Provider value. */
    int chosenProvider() const;
    /** Shows exactly the rows that belong to the chosen provider. */
    void showRowsOfTheChosenProvider();
    /** Puts a sentence under the key field, and takes its row away when empty. */
    void sayAboutTheKey(const QString &sentence);
    void startTest();
    void showResult(qint64 chatMilliseconds, qint64 embedMilliseconds, const QString &error);

    QFormLayout *m_form;
    QRadioButton *m_ollamaButton;
    QRadioButton *m_openRouterButton;
    QRadioButton *m_openAiButton;
    QLineEdit *m_apiKey;
    QLabel *m_keyState;
    QComboBox *m_chatModel;
    QComboBox *m_openRouterModel;
    QComboBox *m_openAiModel;
    QLineEdit *m_ollamaUrl;
    QComboBox *m_embeddingModel;
    QComboBox *m_openRouterEmbeddingModel;
    QComboBox *m_openAiEmbeddingModel;
    /** What the 0.60 of SPEC 7.3 was calibrated against; shown under all three. */
    QLabel *m_thresholdNote;
    QPushButton *m_test;
    QLabel *m_result;
    /** Why there is no "Sign in with ChatGPT" (SPEC 7.5); shown under OpenAI. */
    QLabel *m_openAiNote;
    /** That the note text leaves the machine; shown under a remote provider. */
    QLabel *m_remoteTextNote;
    OllamaProvider *m_ollama;
    OpenAiCompatibleProvider *m_openRouter;
    OpenAiCompatibleProvider *m_openAi;

    /** The row numbers the form knows each row by, filled while building. */
    int m_apiKeyRow = -1;
    int m_keyStateRow = -1;
    int m_chatModelRow = -1;
    int m_openRouterModelRow = -1;
    int m_openAiModelRow = -1;
    int m_openAiNoteRow = -1;
    int m_remoteTextNoteRow = -1;
    int m_ollamaUrlRow = -1;
    int m_embeddingModelRow = -1;
    int m_openRouterEmbeddingModelRow = -1;
    int m_openAiEmbeddingModelRow = -1;

    /** True once the user has typed in the key field, see save(). */
    bool m_keyEdited = false;
    /** Which provider the typed key was typed for, see the row switch. */
    QString m_keyEditedFor;
};
