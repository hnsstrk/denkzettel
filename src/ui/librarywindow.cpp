#include "ui/librarywindow.h"

#include "store/store.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"
#include "ui/pendingdeletion.h"
#include "ui/timestampformat.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KSharedConfig>
#include <KStandardShortcut>
#include <KWindowConfig>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QLocale>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWindow>

namespace
{
constexpr int WindowWidth = 900;
constexpr int WindowHeight = 600;

/** Width of the note list, as in wireframe 2b. */
constexpr int ListWidth = 300;

/**
 * How far the splitter may squeeze the list. Two lines of preview need room —
 * below this the list stops being a list of readable notes.
 */
constexpr int MinimumListWidth = 220;

/** Edge length of the icon above an empty-state text. */
constexpr int PlaceholderIconSize = 48;

KConfigGroup windowGroup()
{
    return KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Bibliothek"));
}

/** Small, dimmed label — timestamps and hints, as in the capture window. */
QLabel *subtleLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, palette.color(QPalette::PlaceholderText));
    label->setPalette(palette);

    return label;
}

/**
 * A centred empty-state page (wireframe 2c). Without an icon name only the two
 * lines of text are drawn — the empty library says it once, in the list.
 */
QWidget *placeholderPage(const QString &title, const QString &hint, bool withIcon)
{
    auto *page = new QWidget();

    auto *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(6);

    if (withIcon) {
        auto *icon = new QLabel(page);
        icon->setPixmap(qApp->windowIcon().pixmap(PlaceholderIconSize, PlaceholderIconSize));
        icon->setAlignment(Qt::AlignCenter);
        layout->addWidget(icon);
        layout->addSpacing(6);
    }

    auto *titleLabel = new QLabel(title, page);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *hintLabel = subtleLabel(hint, page);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setWordWrap(true);
    layout->addWidget(hintLabel);

    return page;
}
}

LibraryWindow::LibraryWindow(Store *store, QWidget *parent)
    : QWidget(parent, Qt::Window)
    , m_store(store)
    , m_model(new NoteListModel(this))
    , m_deletion(new PendingDeletion(store, PendingDeletion::DefaultGracePeriodSeconds, this))
    , m_deleteAction(new QAction(i18n("Löschen"), this))
    , m_undoAction(new QAction(i18n("Rückgängig"), this))
    , m_splitter(new QSplitter(Qt::Horizontal, this))
    , m_list(new QListView(this))
    , m_message(new KMessageWidget(this))
{
    setWindowTitle(i18nc("@title:window", "Denkzettel — Bibliothek"));

    m_list->setModel(m_model);
    m_list->setItemDelegate(new NoteListDelegate(m_list));
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setFrameShape(QFrame::NoFrame);

    m_listPages = new QStackedWidget(this);
    m_emptyLibraryPage = placeholderPage(i18n("Noch keine Notizen"),
                                         i18n("Mit Meta+N einen Gedanken festhalten."),
                                         true);
    // A search without a hit is not an empty library: it says something else,
    // and it carries no icon — the icon belongs to the first start, not to a
    // state the user leaves again by typing (wireframe 2c).
    m_noResultsPage = placeholderPage(i18n("Keine Treffer"),
                                      i18n("Den Suchbegriff ändern oder das Feld leeren."),
                                      false);
    m_listPages->addWidget(m_list);
    m_listPages->addWidget(m_emptyLibraryPage);
    m_listPages->addWidget(m_noResultsPage);
    m_listPages->setMinimumWidth(MinimumListWidth);

    m_detailPages = new QStackedWidget(this);
    m_detailPage = buildDetail();
    m_noSelectionPage = placeholderPage(i18n("Keine Notiz ausgewählt"),
                                        i18n("Zum Lesen links eine Notiz auswählen."),
                                        false);
    // The empty library already says everything there is to say; a second
    // message next to it would say it twice (wireframe 2c).
    m_blankPage = new QWidget();
    m_detailPages->addWidget(m_detailPage);
    m_detailPages->addWidget(m_noSelectionPage);
    m_detailPages->addWidget(m_blankPage);

    m_splitter->addWidget(m_listPages);
    m_splitter->addWidget(m_detailPages);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setSizes({ListWidth, WindowWidth - ListWidth});

    // SPEC 9 deletes for good after five seconds, so the message belongs into
    // the window under the header rather than into a screen corner, and it has
    // no close button: it would leave open whether closing carries the
    // deletion out or takes it back (wireframe 2c).
    // Wireframe 2b draws the band as a single row, text and button side by
    // side. Word wrap would put the button underneath and make the band half
    // again as tall; the text is short and fixed in length, so it has nothing
    // to wrap.
    m_message->setMessageType(KMessageWidget::Warning);
    m_message->setCloseButtonVisible(false);
    m_message->setWordWrap(false);
    m_message->addAction(m_undoAction);
    m_message->hide();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(buildHeader());
    layout->addWidget(m_message);
    // The surplus height belongs to list and reading pane. Without the stretch
    // factor no item in this layout has a vertical direction to grow in — a
    // horizontal QSplitter is vertically Preferred, the header as a plain
    // QWidget as well — and Qt hands the surplus to every item alike: header
    // and splitter end up with half the window each (wireframe 2b).
    layout->addWidget(m_splitter, 1);

    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setEnabled(false);
    connect(m_deleteAction, &QAction::triggered, this, &LibraryWindow::deleteCurrentNote);
    addAction(m_deleteAction);

    m_undoAction->setShortcuts(KStandardShortcut::undo());
    m_undoAction->setEnabled(false);
    connect(m_undoAction, &QAction::triggered, m_deletion, &PendingDeletion::undo);
    addAction(m_undoAction);

    auto *closeAction = new QAction(this);
    closeAction->setShortcuts(KStandardShortcut::close());
    connect(closeAction, &QAction::triggered, this, &LibraryWindow::close);
    addAction(closeAction);

    connect(m_list->selectionModel(), &QItemSelectionModel::currentChanged, this, &LibraryWindow::showNote);

    connect(m_deletion, &PendingDeletion::remainingChanged, this, [this](int seconds) {
        m_message->setText(i18n("Notiz gelöscht — noch %1 s", seconds));
        m_undoAction->setEnabled(true);
        if (!m_message->isVisible()) {
            m_message->animatedShow();
        }
    });
    connect(m_deletion, &PendingDeletion::committed, this, [this] {
        if (!m_deletion->isPending()) {
            m_undoAction->setEnabled(false);
            m_message->animatedHide();
        }
    });
    connect(m_deletion, &PendingDeletion::reverted, this, &LibraryWindow::undoDeletion);

    resize(WindowWidth, WindowHeight);
    // windowHandle() exists only once the window has a platform resource, and
    // the stored size has to be in before the first show.
    create();
    KWindowConfig::restoreWindowSize(windowHandle(), windowGroup());
    resize(windowHandle()->size());
    m_splitter->restoreState(windowGroup().readEntry("SplitterState", QByteArray()));

    updatePages();
}

QWidget *LibraryWindow::buildHeader()
{
    auto *header = new QWidget(this);

    m_search = new QLineEdit(header);
    m_search->setPlaceholderText(i18n("Volltextsuche …"));
    // S5 kept the field switched off for a stable layout and said so in a
    // tooltip; this story gives it its function, so both are gone. The clear
    // button is the one-click way back to the full list.
    m_search->setClearButtonEnabled(true);

    connect(m_search, &QLineEdit::textChanged, this, &LibraryWindow::searchChanged);

    auto *layout = new QVBoxLayout(header);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(m_search);

    return header;
}

QWidget *LibraryWindow::buildDetail()
{
    auto *detail = new QWidget();

    m_detailTimestamp = subtleLabel(QString(), detail);

    auto *deleteButton = new QPushButton(i18n("Löschen"), detail);
    connect(deleteButton, &QPushButton::clicked, m_deleteAction, &QAction::trigger);

    auto *head = new QHBoxLayout();
    head->addWidget(m_detailTimestamp);
    head->addStretch();
    head->addWidget(deleteButton);

    m_detailText = new QTextBrowser(detail);
    m_detailText->setFrameShape(QFrame::NoFrame);

    auto *layout = new QVBoxLayout(detail);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(10);
    layout->addLayout(head);
    layout->addWidget(m_detailText);

    return detail;
}

void LibraryWindow::showLibrary()
{
    const bool wasVisible = isVisible();

    // The open window reads the store again as well — a note captured while the
    // library stood open belongs into the list. Only a pending deletion stops
    // that: its note is still in the store, and reading it back would put it
    // into a list that is counting the deletion down.
    if (!m_deletion->isPending()) {
        reload(wasVisible ? Selection::Keep : Selection::Clear);
    }

    if (isMinimized()) {
        showNormal();
    } else {
        show();
    }

    raise();
    activateWindow();

    // Only a window that was off screen sends the focus into the list; on an
    // open window that would pull it out of the reading pane. A minimized
    // window counts as visible — the window manager restores the focus it had.
    if (!wasVisible) {
        m_list->setFocus();
    }
}

void LibraryWindow::setReferenceTime(const QDateTime &now)
{
    // Setting it alone changes nothing on screen: the list takes it up the
    // next time it is rebuilt or the window is activated, exactly like the
    // clock the running application reads (wireframe 3b).
    m_referenceTime = now;
}

QDateTime LibraryWindow::referenceTime() const
{
    return m_referenceTime.isValid() ? m_referenceTime : QDateTime::currentDateTime();
}

void LibraryWindow::changeEvent(QEvent *event)
{
    // The grouping is worked out whenever the list is rebuilt and whenever the
    // window is activated — there is no midnight timer (wireframe 3b). A
    // window left standing over night sorts itself anew at the next look, not
    // on its own.
    if (event->type() == QEvent::ActivationChange && isActiveWindow()) {
        regroupList();
    }

    QWidget::changeEvent(event);
}

void LibraryWindow::closeEvent(QCloseEvent *event)
{
    // SPEC 9: the grace period ends with the window — a deletion the user
    // walked away from is a deletion.
    m_deletion->flush();

    KConfigGroup group = windowGroup();
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.writeEntry("SplitterState", m_splitter->saveState());
    group.sync();

    QWidget::closeEvent(event);
}

void LibraryWindow::reload(Selection selection)
{
    // The selection follows the note by its id, not by its row: notes written
    // while the library stood open take the rows above it, and a new group
    // brings a head row along with them.
    const qint64 selected =
        selection == Selection::Keep ? m_model->noteAt(m_list->currentIndex().row()).id : -1;

    // An empty search field returns the whole library from the store, so the
    // full list and a result list are the same code path — and clearing the
    // field needs no case of its own (SPEC 6).
    m_model->setNotes(m_store->search(m_search->text()), referenceTime());

    // Saying the empty selection explicitly also stops QAbstractItemView from
    // picking the first entry on its own the moment the list takes the focus.
    const int row = m_model->rowOf(selected);
    m_list->setCurrentIndex(row >= 0 ? m_model->index(row) : QModelIndex());

    updatePages();
}

void LibraryWindow::regroupList()
{
    const qint64 selected = m_model->noteAt(m_list->currentIndex().row()).id;

    m_model->regroup(referenceTime());

    const int row = m_model->rowOf(selected);
    m_list->setCurrentIndex(row >= 0 ? m_model->index(row) : QModelIndex());

    updatePages();
}

void LibraryWindow::searchChanged()
{
    // The list has to agree with the store before it is read again: a note in
    // its grace period has left the list but is still stored, and a search
    // would fetch it back. Carrying the deletion out first is the rule a
    // second deletion and the closing window already follow (SPEC 9).
    m_deletion->flush();

    // The note the user was reading stays selected if it is among the hits.
    reload(Selection::Keep);
}

void LibraryWindow::updatePages()
{
    const bool hasNotes = m_model->noteCount() > 0;
    if (hasNotes) {
        m_listPages->setCurrentWidget(m_list);
    } else if (m_search->text().isEmpty()) {
        m_listPages->setCurrentWidget(m_emptyLibraryPage);
    } else {
        // A search over an empty library lands here too and offers to change
        // the term. Clearing the field then says "Noch keine Notizen", so the
        // window corrects itself with the next keystroke.
        m_listPages->setCurrentWidget(m_noResultsPage);
    }

    if (!hasNotes) {
        m_detailPages->setCurrentWidget(m_blankPage);
    } else if (m_list->currentIndex().isValid()) {
        m_detailPages->setCurrentWidget(m_detailPage);
    } else {
        m_detailPages->setCurrentWidget(m_noSelectionPage);
    }
}

std::optional<library::NoteGroup> LibraryWindow::groupOf(const QModelIndex &row) const
{
    // Asked of the note, not of the row its head sits in: deleting and undoing
    // rebuild the rows, and a group told apart by row number would compare
    // right or wrong by accident.
    if (!row.isValid()) {
        return std::nullopt;
    }

    const Note note = m_model->noteAt(row.row());
    if (note.id < 0) {
        return std::nullopt;
    }

    return library::noteGroup(note.createdAt, referenceTime(), QLocale());
}

QModelIndex LibraryWindow::groupHeadOf(const QModelIndex &note) const
{
    // The head of a group is the next head row above the note — not
    // necessarily the row right above it, which is what a note in the middle
    // of its group has (UI review of 01.08.2026).
    for (int row = note.row() - 1; row >= 0; --row) {
        const QModelIndex candidate = m_model->index(row);
        if (candidate.data(NoteListModel::GroupHeaderRole).toBool()) {
            return candidate;
        }
    }

    return {};
}

void LibraryWindow::showNote(const QModelIndex &index, const QModelIndex &previous)
{
    m_deleteAction->setEnabled(index.isValid());

    if (index.isValid()) {
        // Three conditions have to hold before the list is moved for a head,
        // and each of them keeps out a way of moving it against the user.
        //
        // It crosses a group boundary — that is what AK 7 and wireframe 3b,
        // case 4 ask for, and what the first selection after opening or
        // rebuilding counts as, its predecessor being none. Moving within a
        // group fetches nothing: the user has rolled the list to where he
        // wants it, and one arrow key must not throw that away, least of all
        // against the direction he presses in.
        //
        // Whether the entry is in the picture already does not enter into it.
        // A note can stand in full view while its head sits just above the
        // upper edge — that is the very case this fetches the head for (PO
        // decision of 01.08.2026, after the case was measured).
        //
        // And both fit into the list at once. In a group taller than the
        // window the head cannot be shown without pushing the selection out.
        const QModelIndex head = groupHeadOf(index);
        const std::optional<library::NoteGroup> group = groupOf(index);
        const std::optional<library::NoteGroup> previousGroup = groupOf(previous);
        const bool crossesAGroupBoundary = !previousGroup.has_value() || previousGroup != group;

        if (head.isValid() && crossesAGroupBoundary) {
            const QRect heading = m_list->visualRect(head);
            const QRect selected = m_list->visualRect(index);
            if (selected.bottom() - heading.top() <= m_list->viewport()->height()) {
                m_list->scrollTo(head, QAbstractItemView::EnsureVisible);
            }
        }
        m_list->scrollTo(index, QAbstractItemView::EnsureVisible);

        const Note note = m_model->noteAt(index.row());
        // The detail pane stands under no head and keeps the full timestamp.
        m_detailTimestamp->setText(library::relativeTimestamp(note.createdAt, referenceTime(), QLocale()));
        // Setting the same text again would send the reader back to its first
        // line; a reload of the open window leaves the reader where it was.
        if (m_detailText->toPlainText() != note.content) {
            m_detailText->setPlainText(note.content);
        }
    }

    updatePages();
}

void LibraryWindow::deleteCurrentNote()
{
    const QModelIndex current = m_list->currentIndex();
    // A group head carries no note; the selection never lands on one, and Entf
    // finds nothing to delete there (wireframe 3b).
    const int index = current.isValid() ? m_model->noteIndexAt(current.row()) : -1;
    if (index < 0) {
        return;
    }

    m_deletedIndex = index;
    m_deletedNote = m_model->noteAt(current.row());

    m_deletion->request(m_deletedNote.id);
    m_model->takeNote(m_deletedIndex);

    // The selection moves on to the following note, to the preceding one if
    // there is none (wireframe 2c) — counted in notes, so it never lands on a
    // head, whether or not the deletion took one with it.
    const int remaining = m_model->noteCount();
    if (remaining > 0) {
        m_list->setCurrentIndex(m_model->index(m_model->rowOfNote(qMin(m_deletedIndex, remaining - 1))));
    }

    updatePages();
}

void LibraryWindow::undoDeletion()
{
    // The note comes back where it was — and its group head with it, if the
    // deletion had emptied the group.
    m_model->insertNote(m_deletedIndex, m_deletedNote);
    m_list->setCurrentIndex(m_model->index(m_model->rowOfNote(m_deletedIndex)));

    m_undoAction->setEnabled(false);
    m_message->animatedHide();
    updatePages();
}
