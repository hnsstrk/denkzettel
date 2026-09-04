#pragma once

#include "store/note.h"
#include "store/proposal.h"

#include <QList>
#include <QWidget>

class Store;

class KMessageWidget;
class QCheckBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QTextBrowser;
class QVBoxLayout;

/**
 * The suggestion review of SPEC 9, wireframe 1c: one card per open bundle
 * suggestion, with the notes it stands for, a live Markdown preview and the
 * three answers Accept · Later · Discard (issue #30).
 *
 * **A window of its own and not a page of the library** (wireframe 1c draws it
 * with a title bar of its own): the library is a place to read in, this is a
 * place to answer questions in, and the two are reached separately — the tray
 * entry "Suggestions" opens this one.
 *
 * **Two kinds of card, one frame.** A task suggestion (SPEC 7.4) is answered
 * here as well since issue #31: the same head row, the same separator and the
 * same three buttons, and only the body between them differs — a bundle shows
 * its notes and the Markdown, a task shows its five fields and what would be
 * annotated. The list both are built from is `Store::proposals()`, so the
 * second kind added a branch and no second reader.
 */
class ProposalWindow : public QWidget
{
    Q_OBJECT

public:
    /** `store` outlives the window and is not owned by it. */
    explicit ProposalWindow(Store *store, QWidget *parent = nullptr);

public Q_SLOTS:
    /** Reads the suggestions again and shows the window, or raises the open one. */
    void showProposals();

private:
    /**
     * One card and everything an answer to it needs.
     *
     * The notes are kept beside the widgets because the Markdown is written
     * from them at every change of the selection — the same
     * `bundleMarkdown()` the analysis run wrote the stored payload with, so
     * what the preview shows and what the export writes cannot drift apart
     * (SPEC 7.3, 8.1).
     */
    struct Card {
        Proposal proposal;
        /** The notes of the suggestion, in the order `Store::proposals()` hands them over. */
        QList<Note> notes;
        /** One row per note, checkable; same order as `notes`. Bundle cards only. */
        QListWidget *noteList = nullptr;
        /** The Markdown of a bundle, or what a task would annotate. */
        QTextBrowser *preview = nullptr;
        /**
         * Where an error of this card's own is reported (SPEC 8.2, issue #33).
         *
         * On the card and not in the band under the window title: the band
         * says one thing for a window that can hold several cards, and a
         * failed `task add` has to name the suggestion it belongs to. Hidden
         * it takes no height.
         */
        KMessageWidget *error = nullptr;
        QWidget *frame = nullptr;
    };

    /** Reads `Store::proposals()` and builds the cards again from nothing. */
    void reload();

    /** Builds the widgets of one card and writes them into it. */
    void buildCard(Card &card);

    /** The body of a bundle card: the notes on the left, the Markdown on the right. */
    void buildBundleBody(Card &card, QVBoxLayout *layout);

    /**
     * The body of a task card: the five fields of SPEC 7.2 and the annotation
     * preview under them (wireframe 1c, issue #31).
     *
     * The fields are editable, and what they say at the moment "Accept" is
     * pressed is what reaches Taskwarrior — that is the whole of acceptance
     * criterion 1. They are addressed by object name (`description-<id>` and
     * so on), the way the buttons of this window are, because two cards carry
     * two fields of every name.
     */
    void buildTaskBody(Card &card, QVBoxLayout *layout);

    /** One field of a task card, or nullptr for a bundle card. */
    QLineEdit *taskField(const Card &card, QLatin1StringView role) const;

    /**
     * The payload of SPEC 7.2 as the five fields of `card` now read.
     *
     * Built from the widgets and not from `card.proposal.payload`: the stored
     * text is what the analysis run proposed, and the review is where the user
     * corrects it. `tags` is split on whitespace — a tag with a blank in it is
     * none for Taskwarrior (see taskAddArguments()), so the field cannot
     * produce one.
     */
    QString taskPayload(const Card &card) const;

    /** The card of `id`; null once its suggestion is gone. */
    Card *cardFor(qint64 id);

    /** The notes of `card` whose row is ticked, in the order of the bundle. */
    QList<Note> selectedNotes(const Card &card) const;

    /**
     * What an accepted card hands to `exportBundle()`: the ticked notes and a
     * Markdown written from exactly those.
     *
     * A deselected note is in neither, which is what keeps it in the corpus —
     * `Store::removeExportedBundle()` deletes the ids it is given, and the
     * collective note holds the text of the same ones.
     */
    Proposal selection(const Card &card) const;

    /** Follows a tick into the preview and into the "Accept" button. */
    void selectionChanged(qint64 id);

    void accept(qint64 id);

    /**
     * Carries an accepted task card into Taskwarrior and clears up after it
     * (SPEC 8.2, issue #33).
     *
     * On an error nothing is cleared up: the note stays, the suggestion stays
     * open, and the card says what happened. On success the note and the
     * suggestion go the way an exported bundle goes — one transaction, through
     * `Store::removeExportedBundle()`, so the two roads out of the corpus
     * cannot drift apart.
     */
    void acceptTask(const Card &card);

    void defer(qint64 id);
    void discard(qint64 id);

    /** Puts one line into the card's own error row, or hides it again. */
    void showCardError(const Card &card, const QString &text);

    /**
     * One line in the band under the top of the window, in the make the
     * library uses for its own (wireframe 2b): hidden it takes no height.
     */
    void showBandMessage(const QString &text, bool isError);

    Store *m_store;

    KMessageWidget *m_message;

    /** Page 0 carries the cards, page 1 the sentence for "nothing is waiting". */
    QStackedWidget *m_pages;

    /** Where buildCard() puts its frames; the stretch stays at the bottom. */
    QVBoxLayout *m_cardLayout;

    QList<Card> m_cards;
};
