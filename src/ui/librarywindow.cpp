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
    m_listPages->addWidget(m_list);
    m_listPages->addWidget(m_emptyLibraryPage);

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
    m_message->setMessageType(KMessageWidget::Warning);
    m_message->setCloseButtonVisible(false);
    m_message->setWordWrap(true);
    m_message->addAction(m_undoAction);
    m_message->hide();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(buildHeader());
    layout->addWidget(m_message);
    layout->addWidget(m_splitter);

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

    auto *search = new QLineEdit(header);
    search->setPlaceholderText(i18n("Volltextsuche …"));
    search->setClearButtonEnabled(false);
    search->setEnabled(false);

    // A disabled widget gets no mouse events and would swallow its own
    // tooltip, so the tooltip sits on a wrapper of the same size. The field is
    // here for a stable layout; S6/S7 add the search itself.
    auto *wrapper = new QWidget(header);
    wrapper->setToolTip(i18n("Die Volltextsuche steht noch nicht zur Verfügung."));
    auto *wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->addWidget(search);

    auto *layout = new QVBoxLayout(header);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(wrapper);

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
    // Only a closed window reloads: a pending deletion has left its note in
    // the store, and reading it back would put it into a list that is counting
    // the deletion down. Closing the window carries the deletion out, so the
    // reload below never sees a half-deleted note.
    if (!isVisible()) {
        reload();
    }

    if (isMinimized()) {
        showNormal();
    } else {
        show();
    }

    raise();
    activateWindow();
    m_list->setFocus();
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

void LibraryWindow::reload()
{
    m_model->setNotes(m_store->notes());

    // A freshly opened library has nothing selected (wireframe 2c). Saying so
    // explicitly also stops QAbstractItemView from picking the first entry on
    // its own the moment the list takes the focus.
    m_list->setCurrentIndex(QModelIndex());

    updatePages();
}

void LibraryWindow::updatePages()
{
    const bool hasNotes = m_model->rowCount() > 0;
    m_listPages->setCurrentWidget(hasNotes ? static_cast<QWidget *>(m_list) : m_emptyLibraryPage);

    if (!hasNotes) {
        m_detailPages->setCurrentWidget(m_blankPage);
    } else if (m_list->currentIndex().isValid()) {
        m_detailPages->setCurrentWidget(m_detailPage);
    } else {
        m_detailPages->setCurrentWidget(m_noSelectionPage);
    }
}

void LibraryWindow::showNote(const QModelIndex &index)
{
    m_deleteAction->setEnabled(index.isValid());

    if (index.isValid()) {
        const Note note = m_model->noteAt(index.row());
        m_detailTimestamp->setText(library::relativeTimestamp(note.createdAt, QDateTime::currentDateTime(), QLocale()));
        m_detailText->setPlainText(note.content);
    }

    updatePages();
}

void LibraryWindow::deleteCurrentNote()
{
    const QModelIndex current = m_list->currentIndex();
    if (!current.isValid()) {
        return;
    }

    m_deletedRow = current.row();
    m_deletedNote = m_model->noteAt(m_deletedRow);

    m_deletion->request(m_deletedNote.id);
    m_model->takeRow(m_deletedRow);

    // The selection moves on to the following entry, to the preceding one if
    // there is none (wireframe 2c).
    const int remaining = m_model->rowCount();
    if (remaining > 0) {
        m_list->setCurrentIndex(m_model->index(qMin(m_deletedRow, remaining - 1)));
    }

    updatePages();
}

void LibraryWindow::undoDeletion()
{
    m_model->insertNote(m_deletedRow, m_deletedNote);
    m_list->setCurrentIndex(m_model->index(m_deletedRow));

    m_undoAction->setEnabled(false);
    m_message->animatedHide();
    updatePages();
}
