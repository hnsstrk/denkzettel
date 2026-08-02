#pragma once

#include "store/note.h"
#include "ui/timestampformat.h"

#include <QModelIndex>
#include <QWidget>

#include <optional>

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
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    /** What a reload does with the note the list has selected. */
    enum class Selection {
        /** A freshly opened library has nothing selected (wireframe 2c). */
        Clear,
        /** The open window keeps the note the user is reading. */
        Keep,
    };

    /** The answers of the guard dialog over unsaved changes (wireframe 2a). */
    enum class UnsavedAnswer {
        /** Write, then carry the triggering act out. */
        Save,
        /** Carry the triggering act out without writing. */
        Discard,
        /** Take the act back and stay in the edit state. */
        Cancel,
    };

    QWidget *buildHeader();
    QWidget *buildDetail();

    /** Reads the notes matching the search field from the store into the list. */
    void reload(Selection selection);

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

    /** „Abbrechen“ and Esc: leaves the editor, asking first if need be. */
    void cancelEdit();

    /** Leaves the edit state without writing and without asking. */
    void stopEditing();

    /** Shows the dialog of wireframe 2a, state C, and reports the answer. */
    UnsavedAnswer askAboutUnsavedChanges();

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

    QSplitter *m_splitter;
    QLineEdit *m_search;
    QListView *m_list;
    KMessageWidget *m_message;

    QStackedWidget *m_listPages;
    QWidget *m_emptyLibraryPage;
    QWidget *m_noResultsPage;

    QStackedWidget *m_detailPages;
    QWidget *m_detailPage;
    QWidget *m_noSelectionPage;
    QWidget *m_blankPage;

    QLabel *m_detailTimestamp;
    QTextBrowser *m_detailText;

    /** Reader and editor share one place in the pane (wireframe 2a). */
    QStackedWidget *m_textPages;
    QPlainTextEdit *m_editor;

    /**
     * Buttons and badge share one place in the head row: page 0 carries
     * „Bearbeiten“ and „Löschen“, page 1 the badge.
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

    /** Fixed reference time of the grouping; invalid means "ask the clock". */
    QDateTime m_referenceTime;
};
