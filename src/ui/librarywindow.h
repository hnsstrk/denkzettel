#pragma once

#include "store/note.h"
#include "ui/timestampformat.h"

#include <QByteArray>
#include <QModelIndex>
#include <QWidget>

#include <cstdint>
#include <optional>

class AudioPlayer;
class NoteChips;
class NoteListDelegate;
class NoteListModel;
class PendingDeletion;
class Store;

class KMessageWidget;
class QAction;
class QKeySequence;
class QLabel;
class QLineEdit;
class QListView;
class QPlainTextEdit;
class QPushButton;
class QSplitter;
class QStackedWidget;
class QTextBrowser;
class QTreeWidget;

/**
 * The library window: header, note list grouped like an inbox, reading pane
 * (SPEC 9, wireframes 2b, 2c, 3a and 3b).
 *
 * The window is created once and shown again on demand, like the capture
 * window — SPEC 2.3 asks for ShowLibrary() to raise the open window rather
 * than open a second one.
 */
class LibraryWindow : public QWidget
{
    Q_OBJECT

public:
    /** `store` outlives the window and is not owned by it. */
    explicit LibraryWindow(Store *store, QWidget *parent = nullptr);

    /**
     * The point in time the grouping is measured against, for tests and for
     * the picture series of the review; an invalid one hands the question back
     * to the clock, which is what the running application uses.
     *
     * Without it, "Diese Woche ist leer" would only be reachable on a Monday
     * and a screenshot would look different on a Sunday than on a Tuesday.
     * Setting it takes effect at the next rebuild of the list or activation of
     * the window, just as a passing midnight does.
     */
    void setReferenceTime(const QDateTime &now);

    /**
     * The sequence the shortcut service really holds for the capture window —
     * the empty library names it instead of spelling out a key (issue #120).
     *
     * **Handed in from outside, and that is the whole of the design here.**
     * `denkzettelui` links neither `KF6::GlobalAccel` nor the shell, so this
     * library cannot ask what is registered; main() knows, the way it knows
     * about store, transcriber and scheduler. A dependency of our own would
     * buy one text the shorter road already carries.
     *
     * An empty sequence is the state issue #74's read-back reports, and it
     * gets the other wording: the hint then names the road that is always
     * there, the icon in the system tray — the same one the three registration
     * failures of `shortcutregistration.cpp` end with, so the user meets one
     * description of one situation and not two. What it does **not** do is
     * explain why no shortcut is there; that explanation exists already and is
     * shown as a message at registration, and an empty library is the place
     * for "this is how it works", not for "this is how it is broken".
     */
    void setCaptureShortcut(const QKeySequence &sequence);

public Q_SLOTS:
    /** Shows the window, or brings the open one to the front. */
    void showLibrary();

    /**
     * Follows the analysis run into the menu entry and the message band
     * (issue #132).
     *
     * Hangs on AnalysisScheduler::busyChanged in main(), because this library
     * links no scheduler — the same arrangement setCaptureShortcut() has and
     * for the same reason.
     */
    void setAnalysisBusy(bool busy);

Q_SIGNALS:
    /**
     * The application menu asks for the settings dialog (the user's decision of
     * 29.08.2026). Not opened here: the dialog needs the running shortcuts and
     * the model download, and both belong to main() — the same signal the tray
     * emits, connected in the same place, so the two routes cannot drift into
     * two dialogs.
     */
    void configureRequested();

    /**
     * The application menu asks for an analysis run — the third road of
     * SPEC 7.2 beside the tray entry and the bus method (issue #132).
     *
     * Not started here: the scheduler lives in `denkzettelanalysis`, which this
     * library does not link, and main() already holds the two other roads to
     * it. One action, one signal, one connect, and no new plumbing.
     */
    void analysisRequested();

    /**
     * How many notes the analysis run has given up on — the counter beside the
     * row "Unclassified", handed on for the tray tooltip of issue #118.
     *
     * **The number travels, not the fact.** The story is that the column and
     * the tooltip say the same thing, and a listener that asked the store
     * again would make them two reads with a window in between. So this is the
     * very value the row is written with, out of `Store::categoryCounts()`.
     *
     * Emitted on every recount, which is every reload of the list — that is
     * how a deletion, an edit and an undo reach the tooltip while the library
     * stands open, none of which the analysis run announces.
     */
    void unclassifiedCountChanged(int count);

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

    /**
     * Tells a selection made with the mouse from one made with a key (#57).
     *
     * QListView::pressed would come too late — it is emitted after
     * currentChanged, and by then the list has already been moved.
     */
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    /** What a reload does with the note the list has selected. */
    enum class Selection : std::uint8_t {
        /** A freshly opened library has nothing selected (wireframe 2c). */
        Clear,
        /** The open window keeps the note the user is reading. */
        Keep,
    };

    /** The answers of the guard dialog over unsaved changes (wireframe 2a). */
    enum class UnsavedAnswer : std::uint8_t {
        /** Write, then carry the triggering act out. */
        Save,
        /** Carry the triggering act out without writing. */
        Discard,
        /** Take the act back and stay in the edit state. */
        Cancel,
    };

    QWidget *buildHeader();

    /**
     * The category column left of the list: "All", the five categories of
     * SPEC 6 with their counters, and the entry for the notes the analysis run
     * has given up on (SPEC 9, wireframe 1b).
     */
    QWidget *buildSidebar();

    QWidget *buildDetail();

    /** Reads the notes matching the search field from the store into the list. */
    void reload(Selection selection);

    /**
     * Writes the counters of the category column, asking the store for them.
     *
     * Counted in the database and not over the list beside it: that one shows
     * a search result or one category, and a counter taken from it would count
     * what it is standing next to (issue #18).
     */
    void updateCategoryCounts();

    /**
     * Writes the chosen entry into the search field (SPEC 9, issue #18).
     *
     * The column is not a second way of filtering beside the search — it is
     * the search, written out, so that whoever uses it reads the language the
     * field speaks. Everything else in the field stays; only the `kat:` is
     * replaced.
     */
    void categoryChosen();

    /**
     * Moves the mark of the column onto what the search field says.
     *
     * The field is the one truth about the category, in both directions:
     * deleting `kat:software` by hand takes the mark off "Software ideas",
     * typing it puts it there. Without that the two would stand beside each
     * other and disagree.
     */
    void followTheSearchField();

    /**
     * Takes out what the two machine-state entries exclude — "Waiting for
     * analysis" and "Unclassified", the two no `kat:` can express.
     */
    void applyCategoryFilter(QList<Note> &notes) const;

    /**
     * Takes a note that has just been written into the open list (issue #105).
     *
     * Hangs on Store::noteAdded, so every road into the store leads here: the
     * capture window, the D-Bus method AddNote(), and whatever comes after
     * them. Called a second time from the end of a grace period, for the note
     * that had to wait for it.
     */
    void takeUpNewNotes();

    /**
     * reload(Selection::Keep) that leaves the page the user is reading where
     * it is (issue #105).
     *
     * Only for the reload nobody asked for. The one in showLibrary() answers
     * an act of the user's and may move the list; this one happens under his
     * hands while he reads.
     */
    void reloadKeepingThePlace();

    /** Follows a change of the search text into the list. */
    void searchChanged();

    /** Works the groups out again without reading the store. */
    void regroupList();

    /** The reference time of the grouping: the set one, otherwise the clock. */
    QDateTime referenceTime() const;

    /** Picks list and detail page for the current number of notes. */
    void updatePages();

    /** The head row of the group `note` belongs to; invalid if there is none. */
    QModelIndex groupHeadOf(const QModelIndex &note) const;

    /**
     * The group the note in `row` belongs to; nothing for a head row, an
     * invalid row or one that holds no note any more.
     *
     * Told from the note rather than from the row number of its head: deleting
     * and undoing rebuild the rows, and numbers shift as they do.
     */
    std::optional<library::NoteGroup> groupOf(const QModelIndex &row) const;

    /**
     * Shows the selected note and, when the selection has crossed into
     * another group, brings that group's head into view (AK 7).
     *
     * `previous` comes from QItemSelectionModel::currentChanged; an invalid
     * one means the list has just been opened or rebuilt.
     */
    void showNote(const QModelIndex &index, const QModelIndex &previous = QModelIndex());

    /**
     * Asks the view to paint the row above `row` again — nothing if there is
     * none or if `row` is invalid.
     *
     * The separator line under a note depends on whether the row below it is
     * the selected one, so a selection that moves changes the picture of the
     * row above it as well. The view repaints only the stretch between the old
     * and the new selection, and the row above either end lies outside it:
     * measured over six switches, a line stayed where it had to go or was
     * missing where it had to come back (issue #101).
     */
    void repaintTheRowAbove(const QModelIndex &row);

    /** Puts the note of `index` into the reading pane, leaving the list alone. */
    void showNoteText(const QModelIndex &index);

    void deleteCurrentNote();
    void undoDeletion();

    /**
     * Takes the origin off the note in the pane, without touching its text
     * (SPEC 5.1, 13; issue #47).
     *
     * Straight away and without a confirmation dialog — a window title is not
     * worth one, and the way back is the "Undo" in the band under the header,
     * the same one a deletion is taken back with (customer decision
     * 29.08.2026). Not while the editor is open: saveEdit() writes the note it
     * was opened with, and that copy still carries the origin.
     */
    void removeOrigin();

    /** Writes the removed origin back onto the note (issue #47). */
    void restoreOrigin();

    /**
     * Writes the origin onto its line under the head row — the application in
     * front of the window title — elided to the width of the reading pane.
     *
     * A QLabel does not elide by itself, and left to it the pane would claim
     * the width of the longest title ever shown as its minimum. So the label
     * is told to ignore its own width wish and the text is cut to the width it
     * really gets, which happens on every resize (eventFilter).
     *
     * Without an origin the line is **hidden**: no placeholder, no empty row,
     * no height (acceptance criterion 5).
     */
    void showOrigin();

    /** True while the pane holds a note in the editor rather than in the reader. */
    bool isEditing() const;

    /** True while the editor holds something else than what is stored. */
    bool hasUnsavedChanges() const;

    /** Opens the editor on the selected note (SPEC 9, wireframe 2a). */
    void startEditing();

    /** Writes the edited text; keeps category, tags and state (SPEC 9). */
    void saveEdit();

    /** "Cancel" and Esc: leaves the editor, asking first if need be. */
    void cancelEdit();

    /** Leaves the edit state without writing and without asking. */
    void stopEditing();

    /** Shows the dialog of wireframe 2a, state C, and reports the answer. */
    UnsavedAnswer askAboutUnsavedChanges();

    /**
     * Asks for a folder and writes the whole corpus into it (SPEC 8.3,
     * issue #36).
     *
     * Purely reading, so it runs whatever the window is doing — an edit in
     * progress writes nothing until the user saves it, and the export takes
     * what stands in the store.
     */
    void startFullExport();

    /**
     * Puts one line into the band under the header (wireframe 2b).
     *
     * The band is shared with the pending deletion, which brings its own
     * colour, its own layout and the "Undo" button with it — each of them sets
     * what it needs, or the export would report itself beside a greyed out
     * button in the colour of a warning.
     *
     * Two writers use it in this shape: the full export of SPEC 8.3 and, since
     * issue #132, the analysis run started from this window.
     */
    void showBandMessage(const QString &text, bool isError);

    /**
     * Asks for an analysis run and says in the band which state that left
     * (issue #132, wireframe 2b).
     */
    void startAnalysis();

    /**
     * The band under the header with the "Undo" of the origin removal
     * (issue #47).
     *
     * The third writer of that band beside the pending deletion and the export,
     * and each of the three has to take the other's button out — left standing,
     * an "Undo" of a deletion would sit beside this line and take back
     * something else.
     */
    void showOriginMessage();

    /** Follows the edit state into buttons, rows, actions and search field. */
    void updateEditState();

    /**
     * Shows or hides the category column (issue #134).
     *
     * Hiding is `setVisible(false)` and not a width of 0: a column of width 0
     * that is formally still visible is the state issue #18 measured and
     * excluded, and it is what a collapsible splitter would leave behind. The
     * splitter gives the freed room to the note list and the reading pane on
     * its own.
     *
     * A filter the column applied does not survive its column invisibly: a
     * category stands written out in the search field anyway, so it stays; the
     * two entries the search language of SPEC 6 cannot express — the machine
     * states of issue #133 — fall back to "All".
     */
    void showSidebar(bool visible);

    Store *m_store;
    NoteListModel *m_model;
    PendingDeletion *m_deletion;

    QAction *m_deleteAction;
    QAction *m_undoAction;
    /**
     * "Undo" for the origin removal — a second action and not the deletion's.
     *
     * It carries no shortcut: two actions on Ctrl+Z in one window make the key
     * ambiguous, and Qt then delivers it to neither.
     */
    QAction *m_undoOriginAction;
    QAction *m_editAction;
    QAction *m_saveAction;
    QAction *m_cancelEditAction;

    /**
     * The one entry of the hamburger menu in the header (SPEC 8.3).
     *
     * Switched off while a run is going, and that is the whole guard against
     * two exports at once: the run is short, purely reading, and the only door
     * into it is this action.
     */
    QAction *m_exportAction;

    /**
     * The checkable entry of the hamburger menu that hides the category column
     * (issue #134, UX decision 30.08.2026).
     *
     * Without a symbol, because a check mark and a symbol in one menu row
     * compete under Breeze — and without a shortcut: the KDE HIG bar a bare
     * function key ("never just a function key, as these can be hard to access
     * on laptops"), which rules out the obvious F9 of Dolphin's places panel,
     * and an invented modifier combination is one nobody remembers. An
     * accelerator is fitted when the customer asks for one; it is not invented
     * in advance.
     */
    QAction *m_sidebarAction;

    /**
     * The first entry of the hamburger menu: the on-demand road of SPEC 7.2
     * (issue #132).
     *
     * Wording and symbol are the tray entry's, verbatim, because it is the same
     * act seen a third time — the settings entry below is the precedent. The
     * text carries the state rather than a tooltip: a deactivated row without an
     * explanation is unfriendly, and a tooltip is invisible until somebody
     * points at it — and would need setToolTipsVisible() on the menu before Qt
     * showed it at all.
     */
    QAction *m_analysisAction;

    /**
     * Whether the run now going was asked for in this window (issue #132).
     *
     * The band belongs to the act in the window (wireframe 2b), so a periodic
     * run must not write into it — every half hour a line would appear where
     * nobody did anything.
     */
    bool m_analysisRequestedHere = false;

    QSplitter *m_splitter;

    /**
     * The category column as the splitter's first child (issue #134).
     *
     * Held so that it can be hidden outright rather than squeezed to width 0:
     * setChildrenCollapsible(false) stays, and the splitter hands the room of a
     * hidden child to the other two by itself.
     */
    QWidget *m_sidebar;

    /**
     * The splitter's division as it stood while the category column was last
     * visible (issue #134).
     *
     * `ColumnSizes` is written from here instead of from saveState() whenever
     * the column is hidden. A state saved over a hidden child carries a 0 for
     * that child, and a window restored from it is exactly the one the comment
     * at the restore in the constructor was written against — formally
     * successful, unusable to look at.
     */
    QByteArray m_columnSizes;
    QLineEdit *m_search;

    /**
     * The category column of wireframe 1b: entry and counter in two columns,
     * so a QTreeWidget rather than a list — the counter stands right-aligned
     * in a column of its own, which no list item does without a delegate.
     */
    QTreeWidget *m_categories;

    QListView *m_list;
    NoteListDelegate *m_delegate;
    KMessageWidget *m_message;

    QStackedWidget *m_listPages;
    QWidget *m_emptyLibraryPage;

    /**
     * The second line of the empty-library page — it names the capture
     * shortcut, so it is rewritten whenever that changes (issue #120).
     */
    QLabel *m_emptyLibraryHint;

    QWidget *m_noResultsPage;

    /**
     * The second line of the "No matches" page.
     *
     * It says something else for a category with nothing in it than for a
     * search without a hit — and until the analysis run of M3 has been through
     * the library, an empty category is every category (issue #18).
     */
    QLabel *m_noResultsHint;

    QStackedWidget *m_detailPages;
    QWidget *m_detailPage;
    QWidget *m_noSelectionPage;
    QWidget *m_blankPage;

    QLabel *m_detailTimestamp;

    /**
     * The origin on a line of its own under the head row, in the same type and
     * the same colour role (customer decision 29.08.2026, second pass;
     * issue #47): the application it was written in and the title of its
     * window, in that order.
     *
     * Hidden while the note carries none, so it then takes no height at all.
     */
    QLabel *m_detailOrigin;

    /**
     * The player of a voice note, between the head row and the transcript
     * (SPEC 9, wireframe 1b); hidden for a text note.
     *
     * It does not depend on what stands below it: a voice note whose
     * transcription is still running or has failed is a regular state, and the
     * player then stands alone under the head row (SPEC 12, error path).
     */
    AudioPlayer *m_audioPlayer;

    QTextBrowser *m_detailText;

    /** Reader and editor share one place in the pane (wireframe 2a). */
    QStackedWidget *m_textPages;
    QPlainTextEdit *m_editor;

    /**
     * Buttons and badge share one place in the head row: page 0 carries
     * "Edit" and "Delete", page 1 the badge.
     *
     * A stack rather than showing and hiding, because its size hint is the
     * largest of its pages — that is what keeps the row, and with it the note
     * text below, from moving when the state changes.
     */
    QStackedWidget *m_headPages;

    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_saveButton;
    QLabel *m_editingBadge;

    /** Category and tags as plain display, only while editing. */
    QWidget *m_metaRow;
    QLabel *m_category;
    QLabel *m_tags;

    /** The same two as pills, only while reading (wireframe 2b, issue #18). */
    NoteChips *m_chips;

    /** Key hint and the two buttons of the edit state. */
    QWidget *m_editFooter;

    /**
     * The note under the editor; an id below zero means the pane is reading.
     *
     * Kept whole rather than as an id: the guard dialog runs after the
     * selection has already moved on, so the list no longer knows which note
     * is being edited.
     */
    Note m_editedNote;

    /** What the editor was opened with — the measure of an unsaved change. */
    QString m_textBeforeEditing;

    /** True while a cancelled switch puts the selection back; stops the loop. */
    bool m_restoringSelection = false;

    /**
     * True while the mark of the category column is being moved onto what the
     * search field says — the field writes the column here, so the column must
     * not write the field back.
     */
    bool m_followingTheSearchField = false;

    /**
     * True while the selection change being handled goes back to a mouse press
     * in the list — the mark showNote() reads to leave the picture alone (#57).
     *
     * Set before the view sees the press, consumed by the showNote() call the
     * press causes, and dropped again at the next key: a press that selected
     * nothing (a group head, the empty space below the list) must not be read
     * into the keystroke after it.
     */
    bool m_selectionFollowsAPress = false;

    /**
     * Where the pending deletion took its note from, counted in notes rather
     * than in rows: the row it sat in disappears with its group head when it
     * was the last note of its group (wireframe 3b).
     */
    int m_deletedIndex = -1;
    Note m_deletedNote;

    /**
     * The note the pane is reading, as the origin label needs it: the full
     * line to elide, and the id and the two values to write back.
     *
     * The line is the application and the window title together, made by
     * `originLine()` in the .cpp — not the stored title, which is only one of
     * the two things the note carries (issue #47, customer report 29.08.2026).
     * Empty means the note has no origin at all, and the label is then hidden.
     *
     * The id is below zero while nothing has been removed — that is what the
     * band's "Undo" asks.
     */
    QString m_originText;
    qint64 m_removedOriginId = -1;
    QString m_removedOrigin;
    QString m_removedOriginApp;

    /**
     * True while a note written meanwhile waits for a running deletion to end
     * (issue #105).
     *
     * How many arrived does not matter: the list is read from the store as a
     * whole, and one read brings all of them.
     */
    bool m_newNoteWaits = false;

    /** Fixed reference time of the grouping; invalid means "ask the clock". */
    QDateTime m_referenceTime;

    /**
     * The calendar day the list was last grouped for; invalid before the first
     * build.
     *
     * All four group boundaries are day boundaries (SPEC 9), so this one date
     * tells whether the grouping on screen is still the right one — and an
     * activation that finds it unchanged leaves the list alone (issue #59).
     */
    QDate m_groupedOn;
};
