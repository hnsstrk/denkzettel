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

    /** Works the groups out again without reading the store. */
    void regroupList();

    /** The reference time of the grouping: the set one, otherwise the clock. */
    QDateTime referenceTime() const;

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
