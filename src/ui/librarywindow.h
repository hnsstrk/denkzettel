#pragma once

#include "store/note.h"

#include <QModelIndex>
#include <QWidget>

class NoteListModel;
class PendingDeletion;
class Store;

class KMessageWidget;
class QAction;
class QLabel;
class QListView;
class QSplitter;
class QStackedWidget;
class QTextBrowser;

/**
 * The library window: header, chronological note list, reading pane
 * (SPEC 9, wireframes 2b and 2c).
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

public Q_SLOTS:
    /** Shows the window, or brings the open one to the front. */
    void showLibrary();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    /** What a reload does with the note the list has selected. */
    enum class Selection {
        /** A freshly opened library has nothing selected (wireframe 2c). */
        Clear,
        /** The open window keeps the note the user is reading. */
        Keep,
    };

    QWidget *buildHeader();
    QWidget *buildDetail();

    /** Reads the notes from the store into the list. */
    void reload(Selection selection);

    /** Picks list and detail page for the current number of notes. */
    void updatePages();

    void showNote(const QModelIndex &index);
    void deleteCurrentNote();
    void undoDeletion();

    Store *m_store;
    NoteListModel *m_model;
    PendingDeletion *m_deletion;

    QAction *m_deleteAction;
    QAction *m_undoAction;

    QSplitter *m_splitter;
    QListView *m_list;
    KMessageWidget *m_message;

    QStackedWidget *m_listPages;
    QWidget *m_emptyLibraryPage;

    QStackedWidget *m_detailPages;
    QWidget *m_detailPage;
    QWidget *m_noSelectionPage;
    QWidget *m_blankPage;

    QLabel *m_detailTimestamp;
    QTextBrowser *m_detailText;

    /** Where the pending deletion took its note from, for putting it back. */
    int m_deletedRow = -1;
    Note m_deletedNote;
};
