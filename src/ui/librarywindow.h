#pragma once

#include "store/note.h"
#include "ui/timestampformat.h"

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

public Q_SLOTS:
    /** Shows the window, or brings the open one to the front. */
    void showLibrary();

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
     * Takes out what the entry "Unclassified" excludes — the one entry that no
     * `kat:` can express.
     */
    void applyCategoryFilter(QList<Note> &notes) const;

    /** True while the column stands on the entry for the given-up notes. */
    bool isUnclassifiedChosen() const;

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
     * Puts one line of the export into the band under the header
     * (wireframe 2b).
     *
     * The band is shared with the pending deletion, which brings its own
     * colour, its own layout and the "Undo" button with it — each of the two
     * sets what it needs, or the export would report itself beside a greyed
     * out button in the colour of a warning.
     */
    void showExportMessage(const QString &text, bool isError);

    /** Follows the edit state into buttons, rows, actions and search field. */
    void updateEditState();

    Store *m_store;
    NoteListModel *m_model;
    PendingDeletion *m_deletion;

    QAction *m_deleteAction;
    QAction *m_undoAction;
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

    QSplitter *m_splitter;
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
