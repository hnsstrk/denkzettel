#pragma once

#include "store/note.h"
#include "store/proposal.h"

#include <QList>
#include <QWidget>

class Store;

class KMessageWidget;
class QCheckBox;
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
 * **Bundle cards only, so far.** The task cards of SPEC 7.4 belong to issue
 * #31 together with the Taskwarrior road of SPEC 8.2; a task suggestion is
 * therefore passed over here rather than shown as a card nobody can answer.
 * The list this window is built around is `Store::proposals()`, so #31 adds a
 * branch and no second reader.
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
        /** One row per note, checkable; same order as `notes`. */
        QListWidget *noteList = nullptr;
        QTextBrowser *preview = nullptr;
        QWidget *frame = nullptr;
    };

    /** Reads `Store::proposals()` and builds the cards again from nothing. */
    void reload();

    /** Builds the widgets of one card and writes them into it. */
    void buildCard(Card &card);

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
    void defer(qint64 id);
    void discard(qint64 id);

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
