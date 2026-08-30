#include "ui/librarywindow.h"

#include "platform/systemfonts.h"
#include "proposals/fullexport.h"
#include "store/searchquery.h"
#include "store/store.h"
#include "ui/audioplayer.h"
#include "ui/notechips.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"
#include "ui/pendingdeletion.h"
#include "ui/timestampformat.h"

#include <KConfigGroup>
#include <KDesktopFile>
#include <KGuiItem>
#include <KHamburgerMenu>
#include <KLocalizedString>
#include <KMessageDialog>
#include <KMessageWidget>
#include <KSharedConfig>
#include <KStandardAction>
#include <KStandardGuiItem>
#include <KStandardShortcut>
#include <KWindowConfig>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QLocale>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWindow>

namespace
{
constexpr int WindowWidth = 900;
constexpr int WindowHeight = 600;

/** Width of the note list, as in wireframe 2b. */
constexpr int ListWidth = 300;

/** Width of the category column, as in wireframe 1b. */
constexpr int SidebarWidth = 158;

/**
 * How far the splitter may squeeze the list. Two lines of preview need room —
 * below this the list stops being a list of readable notes.
 */
constexpr int MinimumListWidth = 220;

/**
 * How far the splitter may squeeze the category column.
 *
 * Not "the longest label plus its counter" any more (that reading was 6 px
 * short of its own promise from the day it was written, and a four-digit
 * counter breaks it at any width): the **counter** is what this column
 * asserts, and it is held by the header configuration in buildSidebar(), not
 * by this number — section 1 keeps its content width while the stretched
 * label section shrinks. This floor keeps the labels far enough apart to be
 * told from one another; below it they elide into each other.
 *
 * Measured at this floor on 2026-08-30 rather than read out of the header
 * configuration, because a configuration is a statement of intent until
 * somebody measures it (finding 50), and the row that made this a question is
 * the one whose counter once stood outside the viewport (issue #18,
 * finding 51). With the sidebar squeezed to 120 px the counter section keeps
 * its content width — **37 px at three digits, 45 px at four** — the stretched
 * label section takes what is left (83 and 75), and both counters stand
 * complete inside the viewport (`categoryshots`, pictures
 * `133-schmal-123.png` and `133-schmal-1234.png`).
 *
 * **The 22 px this was first argued with are the width of a *one*-digit
 * counter**, measured on the small populations of those same pictures and then
 * carried into the reasoning as the three-digit figure. Three digits cost
 * 37 px, so the label keeps 83 px and not the 98 the argument assumed. The
 * conclusion is unaffected and was checked by looking: at 83 px the three long
 * labels read "Software-…", "Wartet au…" and "Nicht ein…", and at 75 px
 * "Softwar…", "Wartet a…" and "Nicht ei…" — still three different lines. What
 * elides is the label, never the counter, which is the whole point.
 */
constexpr int MinimumSidebarWidth = 120;

/** Which notes the selected entry of the category column lets through. */
enum class CategoryFilter : std::uint8_t {
    /** Everything the search field left over. */
    All,
    /** The notes carrying the short form in ValueRole. */
    Category,
    /** The notes the analysis has not reached yet (issue #133). */
    Waiting,
    /** The notes whose classification attempts are used up (SPEC 7.2). */
    Unclassified,
};

/**
 * The two entries the search language of SPEC 6 has no word for.
 *
 * Both are states of the machine rather than categories, so both take the
 * `kat:` out of the field instead of writing one in, and both narrow the list
 * here rather than in the store.
 */
constexpr bool isMachineState(CategoryFilter kind)
{
    return kind == CategoryFilter::Waiting || kind == CategoryFilter::Unclassified;
}

/** Roles the entries of the category column carry their filter in. */
constexpr int FilterRole = Qt::UserRole;
constexpr int ValueRole = Qt::UserRole + 1;

/** What the marked entry of the column filters by, "All" while nothing is marked. */
CategoryFilter chosenFilter(const QTreeWidget *categories)
{
    const QTreeWidgetItem *current = categories->currentItem();
    return current ? static_cast<CategoryFilter>(current->data(0, FilterRole).toInt())
                   : CategoryFilter::All;
}

/** One entry of the category column: the stored short form and its label. */
struct CategoryEntry {
    QLatin1StringView value;
    QString label;
};

/**
 * The five categories of SPEC 6 with the labels wireframe 1b writes beside
 * their counters.
 *
 * The short form is what stands in `notes.category` and what `kat:` compares
 * against; the readable label is a matter of the user interface and is made
 * **here and nowhere else** (SPEC 7.2). Fixed rather than read out of the
 * database: the classifier accepts no other value, and an entry generated from
 * the contents would give the column a shape that changes with them.
 */
QList<CategoryEntry> categoryEntries()
{
    return {
        {QLatin1StringView("todos"), i18nc("@item:inlistbox note category", "TODOs")},
        {QLatin1StringView("ideen"), i18nc("@item:inlistbox note category", "Ideas")},
        {QLatin1StringView("cli"), i18nc("@item:inlistbox note category", "CLI commands")},
        {QLatin1StringView("persoenlich"), i18nc("@item:inlistbox note category", "Personal")},
        {QLatin1StringView("software"), i18nc("@item:inlistbox note category", "Software ideas")},
    };
}

/**
 * The readable label of a stored category value.
 *
 * A value the list of SPEC 6 does not know is handed back as it stands: an
 * older database can carry one, the classifier of issue #14 rejects it, and
 * such a note is reachable through "All" and through the search — it gets no
 * entry of its own, or a value nothing writes any more would become a heading
 * for good.
 */
QString categoryLabel(const QString &value)
{
    const QList<CategoryEntry> entries = categoryEntries();
    for (const CategoryEntry &entry : entries) {
        if (entry.value == value) {
            return entry.label;
        }
    }
    return value;
}

/**
 * The origin of a note as one readable line: the application in front of the
 * window title (SPEC 5.1, 13; issue #47).
 *
 * `notes.origin_app` was stored from the first version of this story and shown
 * nowhere — the id was held to be there for the classification of SPEC 7 and
 * not for reading. The customer's report of 29.08.2026 overturned that: a
 * terminal writes the work into its title and its own name into nothing, so
 * „Sprint triage: roadmap Q3-Q4" says what was being done and not that it was
 * being done in a terminal. The application therefore stands in front, where
 * an elision from the right cannot reach it.
 *
 * **The id becomes a name here and not at capture time.** The desktop file is
 * read again on every showing, so a renamed or freshly installed application
 * reads correctly instead of a name frozen months ago, and nothing is stored
 * that a third column would have to keep in step — SPEC 5.1 names two columns
 * and says why. The price is one file lookup per note *shown in the pane*, one
 * at a time on selection; the list carries no origin at all (customer decision
 * 29.08.2026), so this is not a lookup per row.
 *
 * **Where the lookup fails the raw id stands.** Not every `app_id` has a
 * desktop file and not every desktop file is named after it, and
 * „com.mitchellh.ghostty" still says which program it was — dropping it would
 * put the line back where the report found it. `QStandardPaths` plus
 * `KDesktopFile` rather than `KService::serviceByDesktopName()`: the same
 * answer for the same names, out of KF6::ConfigCore, which is linked here —
 * KService would be a framework more in `find_package` and in the package's
 * `depends` for four lines.
 *
 * With no id the title stands alone and with no title the application does: a
 * separator with nothing behind it reads as a half sentence.
 */
QString originLine(const QString &title, const QString &appId)
{
    QString application = appId;
    if (!appId.isEmpty()) {
        const QString file = QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                                    appId + QStringLiteral(".desktop"));
        if (!file.isEmpty()) {
            const QString name = KDesktopFile(file).readName();
            if (!name.isEmpty()) {
                application = name;
            }
        }
    }

    if (application.isEmpty()) {
        return title;
    }
    if (title.isEmpty()) {
        return application;
    }
    return i18nc("@info the origin of a note: application, then the window title",
                 "%1 · %2", application, title);
}

/** Edge length of the icon above an empty-state text. */
constexpr int PlaceholderIconSize = 48;

/** Stands in for a category or a tag list no analysis run has filled in yet. */
QString missingValue()
{
    return QStringLiteral("—");
}

KConfigGroup windowGroup()
{
    return KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Bibliothek"));
}

/**
 * "Delete note (Del)" — the wording of a tooltip that names its key.
 *
 * The key is read off the action every time rather than written down: „Entf"
 * is what a German session calls it and „Del" what a run without a locale
 * does, and the undo keys are not even fixed on one machine — they come out of
 * kdeglobals and whoever changed them there would read a wrong tooltip
 * (issue #72).
 */
QString tooltipNaming(const QString &activity, const QAction *action)
{
    return i18nc("@info:tooltip", "%1 (%2)", activity, action->shortcut().toString(QKeySequence::NativeText));
}

/**
 * Puts Return on the primary action of a message dialog.
 *
 * Two calls, because one of them alone does not hold (measured 02.08.2026,
 * issue #66): among buttons that are auto-default — and dialog buttons are —
 * **the default follows the focus**, and the KDE build hands the focus to the
 * cancel button the moment the dialog reaches the screen.
 *
 * Setting the focus is also the honest half of the pair: the button Return
 * triggers is the button that wears the focus ring.
 */
void putReturnOnThePrimaryAction(KMessageDialog *dialog)
{
    auto *buttons = dialog->findChild<QDialogButtonBox *>();
    if (!buttons) {
        return;
    }

    const QList<QAbstractButton *> answers = buttons->buttons();
    for (QAbstractButton *answer : answers) {
        if (buttons->buttonRole(answer) != QDialogButtonBox::YesRole) {
            continue;
        }
        if (auto *push = qobject_cast<QPushButton *>(answer)) {
            push->setDefault(true);
            push->setFocus();
        }
        return;
    }
}

/** Small label in the regular text colour — the values of the meta row. */
QLabel *smallLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setFont(platform::smallestReadableFont());
    label->setProperty(platform::FontSetByHand.data(), true);

    return label;
}

/** Small, dimmed label — timestamps and hints, as in the capture window. */
QLabel *subtleLabel(const QString &text, QWidget *parent)
{
    QLabel *label = smallLabel(text, parent);

    // The role, not the colour: this window is built at daemon start and kept
    // (SPEC 2.1, main.cpp), so a colour taken from the palette once would stay
    // put when the user changes the colour scheme — and stay put until the
    // daemon is restarted. A role is resolved anew on every palette change
    // (issue #58, the second site of issue #54).
    label->setForegroundRole(QPalette::PlaceholderText);

    return label;
}

/**
 * The second line of the empty library: how to capture a thought (wireframe
 * 2c, issue #120).
 *
 * The sequence is named and not spelled out. Since the settings page of
 * SPEC 13 the user chooses it, and „Press Meta+N" then names a key that does
 * nothing — the same fault issue #74 took out of the four messages in the
 * shell, left standing here because this library is another one.
 *
 * **Without a sequence the hint changes its subject, not its tone.** That is
 * the state the read-back of SPEC 2.4 reports, and telling the user to press
 * nothing would be worse than saying nothing. It names the road that is there
 * whatever the shortcut service does — and in the wording the three failure
 * messages of `shortcutregistration.cpp` already end with, so one situation
 * gets one description. The reason no shortcut is there is deliberately not
 * given: it is shown at registration, and an empty library is where „this is
 * how it works" belongs, not „this is how it is broken" — a state one leaves
 * with the first note is no place for a fault report.
 */
QString captureHint(const QKeySequence &sequence)
{
    if (sequence.isEmpty()) {
        return i18n("Capture a thought through the icon in the system tray.");
    }

    return i18n("Press %1 to capture a thought.", sequence.toString(QKeySequence::NativeText));
}

/**
 * A centred empty-state page (wireframe 2c). Without an icon name only the two
 * lines of text are drawn — the empty library says it once, in the list.
 */
// Healing this means changing the signature or introducing a type of its own,
// which is design rather than tidying up (issue #76). The one case a mix-up
// would be visible in - placeholderPage() in the empty library - gets a test
// assurance instead, as issue #88.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
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
    // Named so that the caller can reach it again: the "No matches" page says
    // something else for an empty category than for a search without a hit
    // (issue #18).
    hintLabel->setObjectName(QStringLiteral("hint"));
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
    , m_deleteAction(new QAction(i18n("Delete"), this))
    // The fourth labelled control of the library and the only one outside the
    // detail pane: KMessageWidget draws its actions as buttons with the symbol
    // beside the text, so the symbol belongs on the action (wireframe 2b,
    // issue #67).
    , m_undoAction(new QAction(QIcon::fromTheme(QStringLiteral("edit-undo")), i18n("Undo"), this))
    // The same wording and the same symbol as the deletion's: the band under
    // the header offers one button, and which of the two acts it takes back is
    // said by the line beside it (issue #47).
    , m_undoOriginAction(new QAction(QIcon::fromTheme(QStringLiteral("edit-undo")), i18n("Undo"), this))
    , m_editAction(new QAction(i18n("Edit"), this))
    , m_saveAction(new QAction(i18n("Save"), this))
    , m_cancelEditAction(new QAction(i18n("Cancel"), this))
    // The ellipsis says an input follows — the folder dialog. The symbol is
    // the one the settings page "Export" carries, because it is the same act
    // (SPEC 8.3, UX decision 2026-08-29).
    , m_exportAction(new QAction(QIcon::fromTheme(QStringLiteral("document-export")),
                                 i18nc("@action", "Export all notes…"),
                                 this))
    // No symbol, for the reason at the member: the check mark is what this row
    // says, and a second graphic beside it competes with it under Breeze.
    , m_sidebarAction(new QAction(i18nc("@action", "Show category column"), this))
    // Wording and symbol taken from the tray entry, verbatim (SPEC 7.2,
    // src/shell/trayicon.cpp:163) — the same act seen a third time. Plain
    // i18n() and not i18nc(): the settings entry below shares its msgid with
    // the tray's in exactly this way, and a context of its own would make a
    // **second** catalogue entry for the same sentence — which is how two
    // wordings that must not differ start differing.
    , m_analysisAction(new QAction(QIcon::fromTheme(QStringLiteral("system-run")),
                                   i18n("Analyze now"),
                                   this))
    , m_splitter(new QSplitter(Qt::Horizontal, this))
    , m_list(new QListView(this))
    , m_delegate(new NoteListDelegate(m_list))
    , m_message(new KMessageWidget(this))
{
    // Only what this window is, not the application name: the window decoration
    // appends the display name out of KAboutData by itself. Measured 2026-08-24
    // in a nested kwin_wayland — with "Denkzettel — Library" the title bar reads
    // "Denkzettel — Library — Denkzettel", with this one "Library — Denkzettel".
    setWindowTitle(i18nc("@title:window", "Library"));

    m_list->setModel(m_model);
    m_list->setItemDelegate(m_delegate);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setFrameShape(QFrame::NoFrame);

    m_listPages = new QStackedWidget(this);
    // Built with the wording for "no sequence", because at this point there is
    // none: the window is created at daemon start and the shortcuts are
    // registered further down in main(), which then hands the sequence in
    // (issue #120). Starting from the other wording would put a key on the
    // screen that nothing has confirmed yet.
    m_emptyLibraryPage = placeholderPage(i18n("No notes yet"), captureHint({}), true);
    // Named so that the check of issue #88 can look at this page alone: it
    // tells heading from hint by their text role, and the window carries other
    // labels in those roles — the foot of the category column among them.
    m_emptyLibraryPage->setObjectName(QStringLiteral("emptyLibraryPage"));
    m_emptyLibraryHint = m_emptyLibraryPage->findChild<QLabel *>(QStringLiteral("hint"));
    Q_ASSERT(m_emptyLibraryHint);
    // A search without a hit is not an empty library: it says something else,
    // and it carries no icon — the icon belongs to the first start, not to a
    // state the user leaves again by typing (wireframe 2c).
    m_noResultsPage = placeholderPage(i18n("No matches"),
                                      i18n("Change the search term or clear the field."),
                                      false);
    m_noResultsHint = m_noResultsPage->findChild<QLabel *>(QStringLiteral("hint"));
    Q_ASSERT(m_noResultsHint);
    m_listPages->addWidget(m_list);
    m_listPages->addWidget(m_emptyLibraryPage);
    m_listPages->addWidget(m_noResultsPage);
    m_listPages->setMinimumWidth(MinimumListWidth);

    m_detailPages = new QStackedWidget(this);
    m_detailPage = buildDetail();
    m_noSelectionPage = placeholderPage(i18n("No note selected"),
                                        i18n("Select a note on the left to read it."),
                                        false);
    // The empty library already says everything there is to say; a second
    // message next to it would say it twice (wireframe 2c).
    m_blankPage = new QWidget();
    m_detailPages->addWidget(m_detailPage);
    m_detailPages->addWidget(m_noSelectionPage);
    m_detailPages->addWidget(m_blankPage);

    m_splitter->addWidget(buildSidebar());
    m_splitter->addWidget(m_listPages);
    m_splitter->addWidget(m_detailPages);
    // The surplus width belongs to the reading pane: the category column and
    // the list are both as wide as their contents need (wireframe 1b).
    m_splitter->setStretchFactor(2, 1);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setSizes({SidebarWidth, ListWidth, WindowWidth - ListWidth - SidebarWidth});

    // SPEC 9 deletes for good after five seconds, so the message belongs into
    // the window under the header rather than into a screen corner, and it has
    // no close button: it would leave open whether closing carries the
    // deletion out or takes it back (wireframe 2c).
    // Wireframe 2b draws the band as a single row, text and button side by
    // side. Word wrap would put the button underneath and make the band half
    // again as tall; the text is short and fixed in length, so it has nothing
    // to wrap.
    // Type, word wrap and the "Undo" button are set by whoever fills the band
    // — the deletion below and the export both use it, and they need different
    // ones (issue #36).
    m_message->setCloseButtonVisible(false);
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

    // No shortcut, and **not added to the window**: it is reached through the
    // button KMessageWidget makes out of it and nowhere else. Two actions on
    // Ctrl+Z would make the key ambiguous and Qt would hand it to neither, and
    // a second window action reading "Undo" is one that whoever looks an action
    // up by its wording picks by accident — which is what it did (librarytest,
    // takesUpTheWaitingNoteWhenTheDeletionIsUndone).
    m_undoOriginAction->setEnabled(false);
    connect(m_undoOriginAction, &QAction::triggered, this, &LibraryWindow::restoreOrigin);

    m_undoAction->setShortcuts(KStandardShortcut::undo());
    m_undoAction->setEnabled(false);
    connect(m_undoAction, &QAction::triggered, m_deletion, &PendingDeletion::undo);
    addAction(m_undoAction);

    // The keys of the edit state, each of them the accelerator of a button
    // that is visible beside it (wireframe 2a): the library is a regular
    // window, not the keyboard-only capture.
    m_editAction->setShortcut(Qt::Key_F2);
    m_editAction->setEnabled(false);
    connect(m_editAction, &QAction::triggered, this, &LibraryWindow::startEditing);
    addAction(m_editAction);

    // The three action surfaces name their key (UI review S5, H1). Set here
    // rather than where the buttons are built: the keys are only fixed by the
    // lines above, and a tooltip written before them would name none.
    //
    // „Undo" is no button of this code — KMessageWidget makes one out of
    // the action and passes text, symbol and tooltip on, so the tooltip goes to
    // the action.
    m_editButton->setToolTip(tooltipNaming(i18nc("@action", "Edit note"), m_editAction));
    m_deleteButton->setToolTip(tooltipNaming(i18nc("@action", "Delete note"), m_deleteAction));
    m_undoAction->setToolTip(tooltipNaming(i18nc("@action", "Undo deletion"), m_undoAction));

    // Both Return keys, as in the capture window (SPEC 3).
    m_saveAction->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_Return), QKeySequence(Qt::CTRL | Qt::Key_Enter)});
    m_saveAction->setEnabled(false);
    connect(m_saveAction, &QAction::triggered, this, [this] {
        saveEdit();
        showNoteText(m_list->currentIndex());
    });
    addAction(m_saveAction);

    m_cancelEditAction->setShortcut(Qt::Key_Escape);
    m_cancelEditAction->setEnabled(false);
    connect(m_cancelEditAction, &QAction::triggered, this, &LibraryWindow::cancelEdit);
    addAction(m_cancelEditAction);

    // No key of its own and not added to the window: the export is reached
    // through the hamburger menu, and SPEC 2.4 keeps the shortcuts of this
    // program to the two global ones.
    connect(m_exportAction, &QAction::triggered, this, &LibraryWindow::startFullExport);

    // The same for the category column, and the reason for the missing key is
    // written out at the member.
    m_sidebarAction->setCheckable(true);
    m_sidebarAction->setChecked(true);
    connect(m_sidebarAction, &QAction::toggled, this, &LibraryWindow::showSidebar);

    // And the same for the analysis run: reached through the menu, so no key
    // and not added to the window.
    connect(m_analysisAction, &QAction::triggered, this, &LibraryWindow::startAnalysis);

    auto *closeAction = new QAction(this);
    closeAction->setShortcuts(KStandardShortcut::close());
    connect(closeAction, &QAction::triggered, this, &LibraryWindow::close);
    addAction(closeAction);

    connect(m_list->selectionModel(), &QItemSelectionModel::currentChanged, this, &LibraryWindow::showNote);

    // Pointing is not typing (issue #57), and the difference has to be known
    // before the view moves: the press goes to the viewport, the key to the
    // list itself.
    m_list->viewport()->installEventFilter(this);
    m_list->installEventFilter(this);

    connect(m_deletion, &PendingDeletion::remainingChanged, this, [this](int seconds) {
        m_message->setMessageType(KMessageWidget::Warning);
        m_message->setWordWrap(false);
        // The origin's "Undo" would take back the wrong act beside this line.
        m_message->removeAction(m_undoOriginAction);
        if (!m_message->actions().contains(m_undoAction)) {
            m_message->addAction(m_undoAction);
        }
        m_message->setText(i18n("Note deleted — %1 s left", seconds));
        m_undoAction->setEnabled(true);
        if (!m_message->isVisible()) {
            m_message->animatedShow();
        }
    });
    connect(m_deletion, &PendingDeletion::committed, this, [this] {
        if (!m_deletion->isPending()) {
            m_undoAction->setEnabled(false);
            m_message->animatedHide();
            // The note is out of the store now, and until this line the column
            // went on counting it: deleteCurrentNote() takes the row out of the
            // list and leaves the counters standing, which is right while the
            // grace period runs — the note is still there to be undone — and
            // wrong from the moment it is over. "All" then read one more than
            // the list showed. The tray tooltip is what made it visible: the
            // last note out of the given-up set has to take the error state
            // with it (issue #118), and the recount is what carries it there.
            updateCategoryCounts();
            // The period is over, so a note that arrived during it can come in
            // now. A second deletion that has taken over leaves it waiting.
            if (m_newNoteWaits) {
                takeUpNewNotes();
            }
        }
    });
    connect(m_deletion, &PendingDeletion::reverted, this, &LibraryWindow::undoDeletion);

    // The road of issue #105: the store says what it has taken in, and the open
    // window follows. It hangs on the store rather than on the capture window
    // because the capture window is one of the doors and not the only one — the
    // D-Bus method AddNote() writes without any window at all.
    connect(m_store, &Store::noteAdded, this, &LibraryWindow::takeUpNewNotes);

    resize(WindowWidth, WindowHeight);
    // windowHandle() exists only once the window has a platform resource, and
    // the stored size has to be in before the first show.
    create();
    KWindowConfig::restoreWindowSize(windowHandle(), windowGroup());
    resize(windowHandle()->size());
    // A key of its own since the category column came (issue #18), and the
    // reason is measured, not cosmetic: `restoreState()` fed the stored state
    // of **two** panes into these three returns **true** and leaves them at
    // (297, 595, 0) — the reading pane collapsed to nothing, reported as a
    // success. With an empty QByteArray it returns false and the sizes set
    // above stand. So the old key stays unread on purpose; whoever renames it
    // back hands every user who has opened the library once a window with no
    // reading pane. (That is finding 15's family: a return value that says
    // "done" over an unusable result.)
    //
    // The price is named: the old entry stays behind as a dead line in
    // `denkzettelrc`, and the division falls back to the default once.
    m_columnSizes = windowGroup().readEntry("ColumnSizes", QByteArray());
    m_splitter->restoreState(m_columnSizes);

    // A second key beside the division rather than a 0 inside it (issue #134).
    // `ColumnSizes` keeps its form and its meaning, so a denkzettelrc written
    // before this story reads back as it always did: the key below is missing,
    // its default is false, and the column stands. Written into the division
    // instead, "hidden" would be a 0 for the first pane — and that is the state
    // the comment above was written against, only for another column.
    //
    // setChecked() rather than showSidebar() straight away: the entry in the
    // menu carries the state, so it is the one place it is set from, and the
    // check mark cannot fall out of step with the column.
    if (windowGroup().readEntry("SidebarHidden", false)) {
        m_sidebarAction->setChecked(false);
    }

    updatePages();
    updateEditState();
}

QWidget *LibraryWindow::buildHeader()
{
    auto *header = new QWidget(this);

    m_search = new QLineEdit(header);
    m_search->setPlaceholderText(i18n("Full-text search…"));
    // S5 kept the field switched off for a stable layout and said so in a
    // tooltip; this story gives it its function, so both are gone. The clear
    // button is the one-click way back to the full list.
    m_search->setClearButtonEnabled(true);

    connect(m_search, &QLineEdit::textChanged, this, &LibraryWindow::searchChanged);

    // The library has no menu bar, and KHamburgerMenu is what KDE puts in that
    // place (UX decision 2026-08-29). It stands to the right of the search
    // field, so the header keeps the one row of wireframe 2b.
    //
    // setMenu() rather than the aboutToShowMenu() signal the class documents:
    // that signal is for a menu expensive to build, and these two actions are
    // built once. Without a menu bar there is nothing to advertise either — the
    // sub-menu that advertises one is switched off, or the menu would carry an
    // empty section beside its entries.
    //
    // Through KStandardAction rather than the constructor: the icon and the
    // name are the standard action's doing, not the class's. Built by hand the
    // button comes up with `text=` empty and `icon().isNull()`, and a name set
    // here by hand would need a catalogue line of its own — this way both come
    // out of KF6's own translation (`&Menü öffnen`, `application-menu`), which
    // is what tooltip and accessible name read.
    KHamburgerMenu *hamburger = KStandardAction::hamburgerMenu(nullptr, nullptr, this);
    hamburger->setMenuBarAdvertised(false);
    auto *menu = new QMenu(header);
    // The on-demand road of SPEC 7.2, above the export: both come out of the
    // "File" heading a menu bar would have, and this one is the act, the export
    // the rescue path (issue #132).
    menu->addAction(m_analysisAction);
    menu->addAction(m_exportAction);

    // The category column of wireframe 1b, in a group of its own (issue #134,
    // UX decision 30.08.2026). Three groups because the entries come out of
    // three menu-bar headings — File, View, Settings — which a hamburger menu
    // pulls flat; with the analysis entry above it that is four entries, well
    // under the "about 15 items or fewer" the KDE HIG name.
    menu->addSeparator();
    menu->addAction(m_sidebarAction);

    // The settings, the second route to them beside the tray entry (the user's
    // decision of 29.08.2026, over the argument at TrayIcon that a statement
    // about the whole application must not hang off one window). The wording is
    // the tray's, verbatim, because it is the same action seen twice — and it
    // sits below the separator, in the administrative group a KDE application
    // menu ends with.
    menu->addSeparator();
    const QAction *configureAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure Denkzettel…"));
    connect(configureAction, &QAction::triggered, this, &LibraryWindow::configureRequested);

    hamburger->setMenu(menu);

    auto *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(m_search);
    // requestWidget() is how a QWidgetAction hands out its button outside a
    // QToolBar; the header is a plain QWidget, so nobody asks for it otherwise.
    row->addWidget(hamburger->requestWidget(header));

    auto *layout = new QVBoxLayout(header);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addLayout(row);

    return header;
}

QWidget *LibraryWindow::buildSidebar()
{
    // Held as a member, the way m_categories below is: hiding the column is
    // done on this widget and not on the tree inside it, or the splitter would
    // keep a child that has nothing left to draw (issue #134).
    m_sidebar = new QWidget(this);
    QWidget *sidebar = m_sidebar;
    // Below this the longest label ("Software ideas") and its counter no longer
    // stand side by side, and the column stops saying what it counts.
    sidebar->setMinimumWidth(MinimumSidebarWidth);

    m_categories = new QTreeWidget(sidebar);
    m_categories->setColumnCount(2);
    m_categories->setHeaderHidden(true);
    m_categories->setRootIsDecorated(false);
    // A flat list in a tree: without this every entry would keep the room a
    // branch marker needs, and none of them has children.
    m_categories->setIndentation(0);
    m_categories->setUniformRowHeights(true);
    m_categories->setSelectionMode(QAbstractItemView::SingleSelection);
    m_categories->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_categories->setFrameShape(QFrame::NoFrame);
    m_categories->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // The label takes what is left, the counter what it needs — the two-column
    // division of wireframe 1b.
    //
    // The smallest section has to be said, and that is not a nicety: measured
    // 29.08.2026, the style's own minimum is 100 px, so the counter column took
    // 100 of the 158 the sidebar is wide, the labels elided to "CLI com…" and
    // the numbers stood outside the viewport — a column of headings with no
    // counters at all. A counter is at most four digits wide.
    //
    // And the stretch of the last section has to go, or the counter column
    // keeps that 100 px whatever its resize mode says — a QTreeView switches
    // it on by itself, and it beats the mode.
    m_categories->header()->setMinimumSectionSize(20);
    m_categories->header()->setStretchLastSection(false);
    m_categories->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_categories->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    // One ground for the whole column, and a different one from the note list
    // beside it (wireframe 1b): an item view draws on `Base`, which is the
    // ground the note list already stands on.
    m_categories->viewport()->setBackgroundRole(QPalette::Window);

    const auto entry = [this](const QString &label, CategoryFilter kind, QLatin1StringView value) {
        auto *item = new QTreeWidgetItem(m_categories, {label, QString()});
        item->setData(0, FilterRole, static_cast<int>(kind));
        item->setData(0, ValueRole, QString(value));
        item->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        return item;
    };

    QTreeWidgetItem *all = entry(i18nc("@item:inlistbox all note categories", "All"),
                                 CategoryFilter::All,
                                 QLatin1StringView());
    const QList<CategoryEntry> categories = categoryEntries();
    for (const CategoryEntry &category : categories) {
        entry(category.label, CategoryFilter::Category, category.value);
    }
    // The two machine states go under the five categories, in the order a note
    // travels through them: waiting first, given up on second (issue #133).
    //
    // The first of them is what makes the column add up. Wireframe 1b writes
    // "Alle 128" beside five counters that come to 128, so the column promises
    // a sum; a note the analysis has not reached yet carried no category, was
    // not given up on either, and so stood in "All" and in no row below it —
    // the customer counted 23 against 20 and nothing on the page said where
    // the three were. Not folded into "Unclassified": that row means "given up
    // on" and the tray message of issue #118 counts exactly its condition.
    //
    // The wording deliberately avoids the word "classified": two rows telling
    // each other apart by one word are two rows nobody reads apart.
    entry(i18nc("@item:inlistbox notes the analysis has not reached yet", "Waiting for analysis"),
          CategoryFilter::Waiting,
          QLatin1StringView());
    // The way to the notes the analysis run has given up on (SPEC 7.2). It is
    // where such a note can be dealt with, and without it the tray message of
    // issue #118 would report a state the window offers no way into.
    // updateCategoryCounts() hides it while it counts nothing.
    entry(i18nc("@item:inlistbox notes the analysis gave up on", "Unclassified"),
          CategoryFilter::Unclassified,
          QLatin1StringView());

    m_categories->setCurrentItem(all);
    // Connected after the entries are in: building them moves the current item,
    // and a click written into the search field before the window is finished
    // would search for something nobody asked for.
    connect(m_categories, &QTreeWidget::currentItemChanged, this, &LibraryWindow::categoryChosen);

    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_categories, 1);

    return sidebar;
}

void LibraryWindow::updateCategoryCounts()
{
    const CategoryCounts counts = m_store->categoryCounts();
    // The brush is taken from the palette at every write rather than once: the
    // window outlives a change of the colour scheme (issue #54), and the counts
    // are written again on every reload and on every palette change.
    const QBrush dimmed = palette().brush(QPalette::PlaceholderText);

    for (int row = 0; row < m_categories->topLevelItemCount(); ++row) {
        QTreeWidgetItem *item = m_categories->topLevelItem(row);
        const auto kind = static_cast<CategoryFilter>(item->data(0, FilterRole).toInt());

        int count = 0;
        switch (kind) {
        case CategoryFilter::All:
            count = counts.total;
            break;
        case CategoryFilter::Category:
            count = counts.byCategory.value(item->data(0, ValueRole).toString());
            break;
        case CategoryFilter::Waiting:
            count = counts.waiting;
            // A queue that is empty gets no permanent line either — with the
            // analysis caught up the column is the five categories again, which
            // is the state wireframe 1b draws. Same exception as below: never
            // while it is the entry the list is standing on.
            item->setHidden(count == 0 && item != m_categories->currentItem());
            break;
        case CategoryFilter::Unclassified:
            count = counts.unclassified;
            // A fault that is not there gets no permanent line — the same rule
            // the reading pane follows for a note without category and tags.
            // Never while it is the selected entry, though: it would take its
            // own filter away with it, and the empty list beside it would then
            // have nothing left explaining it.
            item->setHidden(count == 0 && item != m_categories->currentItem());
            break;
        }

        item->setText(1, QLocale().toString(count));
        item->setForeground(1, dimmed);
    }

    // The one number this window has that somebody outside it needs: the tray
    // tooltip names the notes the analysis gave up on, and it has to name the
    // same ones this column offers a way to (issue #118). Sent from here and
    // not read a second time on the other side — that is what makes the two
    // agree rather than usually agree.
    Q_EMIT unclassifiedCountChanged(counts.unclassified);
}

void LibraryWindow::categoryChosen()
{
    // The mark is being moved onto what the field already says; writing the
    // field again here would be the same text and an endless pair.
    if (m_followingTheSearchField) {
        return;
    }

    const QTreeWidgetItem *current = m_categories->currentItem();
    if (!current) {
        return;
    }

    // "All" and the two machine states are no categories: the first is the
    // **absence** of a `kat:`, the other two are conditions the search language
    // has no word for (see applyCategoryFilter). All three therefore take the
    // `kat:` out and leave everything else in the field standing.
    const auto kind = static_cast<CategoryFilter>(current->data(0, FilterRole).toInt());
    const QString category =
        kind == CategoryFilter::Category ? current->data(0, ValueRole).toString() : QString();

    const QString typed = m_search->text();
    const QString written = withSearchCategory(typed, category);
    if (written != typed) {
        // Setting the text is the whole act: textChanged reaches searchChanged()
        // and that reads the store. A click that only wrote a word into the
        // field and left the list standing would look like a fault.
        m_search->setText(written);
        return;
    }

    // The text says the same as before and the entry still changed — "All" over
    // a field that carried no `kat:`, or the entry for the given-up notes.
    reload(Selection::Keep);
}

void LibraryWindow::followTheSearchField()
{
    const QStringList categories = parseSearchQuery(m_search->text()).categories;
    // One `kat:` marks its entry. Several of them ask for a note with two
    // categories and no note has one, and a value the fixed list does not know
    // belongs to no entry either — in both cases the column marks **nothing**,
    // which is the honest answer to "none of these".
    const QString category = categories.size() == 1 ? categories.constFirst() : QString();

    // The two entries the field cannot contradict, because it cannot express
    // them: a word typed while a machine state is chosen narrows **within**
    // that state and does not leave it. Only a `kat:` moves the mark away, and
    // the loop below is what does that.
    //
    // Measured 2026-08-29: without this the mark fell back to "All" at the
    // first keystroke and the bucket was gone without a word — the list then
    // answered out of the whole library.
    if (categories.isEmpty() && isMachineState(chosenFilter(m_categories))) {
        return;
    }

    QTreeWidgetItem *mark = nullptr;
    for (int row = 0; row < m_categories->topLevelItemCount(); ++row) {
        QTreeWidgetItem *item = m_categories->topLevelItem(row);
        const auto kind = static_cast<CategoryFilter>(item->data(0, FilterRole).toInt());

        if (kind == CategoryFilter::All && categories.isEmpty()) {
            mark = item;
        }
        // ASCII case folded, the way `kat:` compares in the store (SPEC 6):
        // whoever types `kat:TODOs` gets the entry the search finds under.
        if (kind == CategoryFilter::Category && !category.isEmpty()
            && item->data(0, ValueRole).toString().compare(category, Qt::CaseInsensitive) == 0) {
            mark = item;
        }
    }

    const QScopeGuard following = qScopeGuard([this] {
        m_followingTheSearchField = false;
    });
    m_followingTheSearchField = true;
    m_categories->setCurrentItem(mark);
}

void LibraryWindow::applyCategoryFilter(QList<Note> &notes) const
{
    // The five categories are gone from here: a click writes `kat:` into the
    // search field, so the store answers them and there is exactly one
    // comparison in the program (SPEC 6, UX decision 2026-08-29).
    //
    // The two machine states are what stays behind, because the search
    // language of SPEC 6 has no operator for "the classification was given up
    // on" or "the analysis has not been here yet". Inventing one is a change to
    // that language and belongs to the customer, not here.

    // The condition Classifier::start() skips by, and the one
    // Store::categoryCounts() counts as `unclassified` — written here over
    // notes already read rather than as a query of its own. All three parts of
    // it, the empty text included: a note without text never reaches the queue
    // (Store::unanalysedNotes()), so it is not one the run gave up on, and the
    // entry has to show what its counter counted.
    const auto givenUp = [](const Note &note) {
        return note.state != Note::State::Analysed && !note.content.trimmed().isEmpty()
            && note.analysisAttempts >= Store::analysisAttemptLimit;
    };

    // Both rows are "no category" first and differ only in the second half —
    // the same shape the two SQL conditions have, and the reason the counters
    // add up. A note with a category belongs to neither, whatever its attempt
    // counter says (issue #133, UX decision 30.08.2026).
    //
    // **These are the second copy of a rule the store writes in SQL**, and two
    // copies drift. What holds them together is not this comment but
    // `librarytest::eachMachineRowHoldsTheNotesItsCounterCounted()`, which
    // chooses each machine row and compares the counter beside it with the
    // notes the list then holds. Measured in review on 29.08.2026: with the
    // empty-text clause taken out of this predicate alone, all fourteen test
    // sets stayed green while the counter of "Waiting for analysis" said 2 and
    // its filter showed 1 — a comment claiming a coupling nothing checked
    // (finding 48). That case is the check; do not describe the coupling here
    // without it.
    switch (chosenFilter(m_categories)) {
    case CategoryFilter::Unclassified:
        notes.removeIf([&givenUp](const Note &note) {
            return !note.category.isEmpty() || !givenUp(note);
        });
        break;
    case CategoryFilter::Waiting:
        notes.removeIf([&givenUp](const Note &note) {
            return !note.category.isEmpty() || givenUp(note);
        });
        break;
    case CategoryFilter::All:
    case CategoryFilter::Category:
        break;
    }
}

QWidget *LibraryWindow::buildDetail()
{
    auto *detail = new QWidget();

    m_detailTimestamp = subtleLabel(QString(), detail);

    // A line of its own under the head row and above the note text, reading as
    // the head's second line (customer decision 29.08.2026, second pass). It
    // stood behind the timestamp first, and the measurement of that version is
    // why it does not any more: in the built window the head row left it 57 to
    // 74 logical pixels, so „Konsole — ssh sample" came out as „· Konsole …" —
    // eight characters that promise an answer and give none
    // (docs/images/reviews/47-herkunft-lang.png).
    //
    // `Ignored` is what keeps it from claiming the width of its text as the
    // pane's minimum: it says "give me what is left", and showOrigin() cuts
    // the text to that. Without an origin the line is hidden, so it takes no
    // height at all — no placeholder, no empty row.
    m_detailOrigin = subtleLabel(QString(), detail);
    m_detailOrigin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_detailOrigin->installEventFilter(this);

    // The one entry of the context menu on the origin, and Qt builds the menu
    // out of it (`ActionsContextMenu`) rather than this window building one per
    // click. Whether there is a menu at all is decided in showOrigin(): a note
    // without an origin offers none, and neither does one whose editor is open
    // — while the head row says „Editing" and its two buttons are gone, the
    // note is not up for anything else either, exactly as m_deleteAction is
    // switched off there. What once made a *wrong save* out of that is fixed
    // where it belonged, in saveEdit().
    auto *removeOriginAction = new QAction(i18nc("@action:inmenu", "Remove origin"), m_detailOrigin);
    connect(removeOriginAction, &QAction::triggered, this, &LibraryWindow::removeOrigin);
    m_detailOrigin->addAction(removeOriginAction);

    // Wireframe 2a: while editing, the head says so where the two buttons
    // stand while reading.
    m_editingBadge = smallLabel(i18n("Editing"), detail);
    // The role rather than a colour taken from the palette once: the window
    // lives as long as the daemon and has to follow a colour scheme changed
    // underneath it (issue #54).
    m_editingBadge->setForegroundRole(QPalette::Link);

    // Symbols from the icon theme (wireframe 2a, table „Symbole an den
    // Schaltflächen“; issue #67). What that table lays down is the *name* —
    // the graphic comes from the theme and changes with it, as in the tray
    // menu (#60). The symbol steps beside the label and never in its place:
    // the KDE HIG have symbols explain a label rather than replace it, and a
    // symbol-only button appears nowhere in this window.
    m_editButton = new QPushButton(QIcon::fromTheme(QStringLiteral("document-edit")), i18n("Edit"), detail);
    connect(m_editButton, &QPushButton::clicked, m_editAction, &QAction::trigger);

    // „Delete“ takes its symbol from KStandardGuiItem::del(), so it is the one
    // every other KDE window deletes with — the icon only, the wording stays
    // under this application's own i18n() and must not depend on which KF6
    // catalogue happens to be installed.
    m_deleteButton = new QPushButton(KStandardGuiItem::del().icon(), i18n("Delete"), detail);
    connect(m_deleteButton, &QPushButton::clicked, m_deleteAction, &QAction::trigger);

    auto *reading = new QWidget(detail);
    auto *readingRow = new QHBoxLayout(reading);
    readingRow->setContentsMargins(0, 0, 0, 0);
    readingRow->addWidget(m_editButton);
    readingRow->addWidget(m_deleteButton);

    auto *editing = new QWidget(detail);
    auto *editingRow = new QHBoxLayout(editing);
    editingRow->setContentsMargins(0, 0, 0, 0);
    editingRow->addStretch();
    editingRow->addWidget(m_editingBadge);

    // Buttons and badge share one stack, and a QStackedLayout takes its size
    // hint from the largest of its pages — so the head row keeps the height of
    // the buttons in both states and the note text below it does not move when
    // the state changes (UI review of 02.08.2026, finding 1).
    //
    // Simply hiding the buttons let the row shrink from button height to label
    // height, and the text jumped by that difference. A minimum height copied
    // off the button once would work today and go stale at the next font
    // change — the mistake issue #54 taught for colours. The stack asks anew
    // every time it lays out.
    m_headPages = new QStackedWidget(detail);
    // …and it never takes more width than its widest page needs, or the two
    // buttons inside it grow with the window — 80 px of label in a 194 px
    // button at 1200 px window width (UI review, second pass, finding 7).
    // `Maximum` is the half that was measured to matter; `Fixed` guards the
    // other direction, where a head row has nothing to gain either.
    //
    // The stretch on the editing page is not the cause: taking it out changes
    // none of these numbers (measured 02.08.2026). It is the stack itself that
    // asks for the surplus, and the stretch in the head layout cannot hold it
    // back — so the stack has to be told not to want it.
    m_headPages->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_headPages->addWidget(reading);
    m_headPages->addWidget(editing);

    auto *head = new QHBoxLayout();
    head->addWidget(m_detailTimestamp);
    head->addStretch();
    head->addWidget(m_headPages);

    // Head row and origin as one block with a spacing of its own: the ten
    // pixels the pane's layout puts between its items would read as a line
    // standing on its own rather than as the second line of the head.
    auto *headBlock = new QVBoxLayout();
    headBlock->setSpacing(2);
    headBlock->addLayout(head);
    headBlock->addWidget(m_detailOrigin);

    // Above the transcript and below the head row, and only for a voice note
    // (SPEC 9, wireframe 1b). showNoteText() shows and fills it.
    m_audioPlayer = new AudioPlayer(detail);
    m_audioPlayer->hide();

    m_detailText = new QTextBrowser(detail);
    m_detailText->setFrameShape(QFrame::NoFrame);

    m_editor = new QPlainTextEdit(detail);
    connect(m_editor, &QPlainTextEdit::textChanged, this, &LibraryWindow::updateEditState);

    m_textPages = new QStackedWidget(detail);
    m_textPages->addWidget(m_detailText);
    m_textPages->addWidget(m_editor);

    // The read state carries the same two as pills — the category filled, the
    // tags outlined (UX decision 2026-08-29 on issue #18, wireframe 2b). It
    // hides itself while it has nothing, so nothing below it moves.
    m_chips = new NoteChips(detail);

    // Category and tags as plain display, deliberately not as greyed-out input
    // fields: those would promise an editing that SPEC 9 does not grant — the
    // analysis run keeps these two, not the editor.
    m_category = smallLabel(QString(), detail);
    m_tags = smallLabel(QString(), detail);

    m_metaRow = new QWidget(detail);
    auto *meta = new QHBoxLayout(m_metaRow);
    meta->setContentsMargins(0, 0, 0, 0);
    meta->addWidget(subtleLabel(i18n("Category"), m_metaRow));
    meta->addWidget(m_category);
    meta->addSpacing(12);
    meta->addWidget(subtleLabel(i18n("Tags"), m_metaRow));
    meta->addWidget(m_tags);
    meta->addStretch();

    // The same two symbols the guard dialog gives the same two answers — the
    // window says the same thing in both places (issue #67).
    m_saveButton = new QPushButton(KStandardGuiItem::save().icon(), i18n("Save"), detail);
    connect(m_saveButton, &QPushButton::clicked, m_saveAction, &QAction::trigger);

    auto *cancelButton = new QPushButton(KStandardGuiItem::cancel().icon(), i18n("Cancel"), detail);
    connect(cancelButton, &QPushButton::clicked, m_cancelEditAction, &QAction::trigger);

    // The order of the two is the platform's, not this window's.
    auto *buttons = new QDialogButtonBox(detail);
    buttons->addButton(m_saveButton, QDialogButtonBox::AcceptRole);
    buttons->addButton(cancelButton, QDialogButtonBox::RejectRole);

    m_editFooter = new QWidget(detail);
    auto *footer = new QHBoxLayout(m_editFooter);
    footer->setContentsMargins(0, 0, 0, 0);
    footer->addWidget(subtleLabel(i18n("Esc cancels · Ctrl+Enter saves"), m_editFooter));
    footer->addStretch();
    footer->addWidget(buttons);

    auto *layout = new QVBoxLayout(detail);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(10);
    layout->addLayout(headBlock);
    layout->addWidget(m_audioPlayer);
    // The surplus height belongs to the note text, said out loud rather than
    // left to Qt's distribution rules. Measured on 02.08.2026: at both tested
    // window sizes the layout holds without the factor as well, so this is a
    // guard and not a repair — it becomes load-bearing as soon as one of the
    // two rows below can grow, which the tag row does once M3 fills it.
    layout->addWidget(m_textPages, 1);
    layout->addWidget(m_chips);
    layout->addWidget(m_metaRow);
    layout->addWidget(m_editFooter);

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

void LibraryWindow::setCaptureShortcut(const QKeySequence &sequence)
{
    // Written straight into the label and not kept: the page stands for the
    // life of the window, so there is no later moment at which the value would
    // be read again.
    m_emptyLibraryHint->setText(captureHint(sequence));
}

QDateTime LibraryWindow::referenceTime() const
{
    return m_referenceTime.isValid() ? m_referenceTime : QDateTime::currentDateTime();
}

bool LibraryWindow::event(QEvent *event)
{
    // The small labels carry a font of their own and do not follow the
    // application font; the list's rows are measured from the same fonts by the
    // delegate, which asks anew on every paint but has to be told to measure
    // again (issue #68).
    if (event->type() == QEvent::ApplicationFontChange) {
        const QFont small = platform::smallestReadableFont();
        const QList<QLabel *> labels = findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->property(platform::FontSetByHand.data()).toBool()) {
                label->setFont(small);
            }
        }
        m_list->doItemsLayout();
    }

    return QWidget::event(event);
}

bool LibraryWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Only the mark is set here; the press itself takes its ordinary road. The
    // filter runs before the view handles it, which is what makes the mark
    // available to the currentChanged that the press is about to cause —
    // QListView::pressed arrives after it and would be of no use (issue #57).
    if (watched == m_list->viewport() && event->type() == QEvent::MouseButtonPress) {
        m_selectionFollowsAPress = true;
    }

    // A press that selected nothing leaves its mark lying around: group heads
    // cannot be picked (wireframe 3b), and neither can the empty space below
    // the list. The release ends it, and so does the next key, or it would make
    // the keyboard behave like a mouse.
    //
    // The release was added for issue #71, and it is what keeps the mark from
    // sticking. Everything the press causes runs inside the handling of the
    // press itself, so nothing is taken away too early — but from the release
    // on the mark is spent, and a selection changed from the program afterwards
    // (a deletion by button, an undo, a reload) is no longer taken for a mouse.
    if (watched == m_list->viewport() && event->type() == QEvent::MouseButtonRelease) {
        m_selectionFollowsAPress = false;
    }

    if (watched == m_list && event->type() == QEvent::KeyPress) {
        m_selectionFollowsAPress = false;
    }

    // The label is told to ignore its own width wish, so how much it really
    // gets is only known once the layout has run — and it changes with every
    // window and splitter movement (issue #47).
    if (watched == m_detailOrigin && event->type() == QEvent::Resize) {
        showOrigin();
    }

    return QWidget::eventFilter(watched, event);
}

void LibraryWindow::changeEvent(QEvent *event)
{
    // The grouping is worked out whenever the list is rebuilt and whenever the
    // window is activated — there is no midnight timer (wireframe 3b). A
    // window left standing over night sorts itself anew at the next look, not
    // on its own.
    //
    // But only then. Regrouping resets the model, and putting the selection
    // back afterwards scrolls the list to it: an Alt-Tab within the same day
    // threw the reader 459 px — 7 rows of a 552 px picture — back onto his
    // selection, although nothing about the grouping had changed (issue #59,
    // measured on 04.08.2026).
    if (event->type() == QEvent::ActivationChange && isActiveWindow()
        && referenceTime().date() != m_groupedOn) {
        regroupList();
    }

    // The counters of the category column carry a brush of their own, and a
    // brush is a colour rather than a role — so it has to be written again when
    // the scheme changes underneath the standing window (issue #54).
    if (event->type() == QEvent::PaletteChange) {
        updateCategoryCounts();
    }

    QWidget::changeEvent(event);
}

void LibraryWindow::closeEvent(QCloseEvent *event)
{
    // The third way out of the edit state (wireframe 2a, state C). Unlike the
    // deletion below, a change is never carried out by walking away: the note
    // is already stored, and losing a correction to a stray click is what the
    // dialog is there to stop.
    if (isEditing()) {
        switch (hasUnsavedChanges() ? askAboutUnsavedChanges() : UnsavedAnswer::Discard) {
        case UnsavedAnswer::Save:
            saveEdit();
            break;
        case UnsavedAnswer::Discard:
            stopEditing();
            break;
        case UnsavedAnswer::Cancel:
            event->ignore();
            return;
        }
    }

    // SPEC 9: the grace period ends with the window — a deletion the user
    // walked away from is a deletion.
    m_deletion->flush();

    // The fourth way out of the playback, and the only one that leaves nothing
    // to see: this window is an object on the daemon's stack (main.cpp), so
    // closing it hides it and destroys nothing. Without this line a voice note
    // plays on behind a window that is gone — up to the fifteen minutes of
    // SPEC 4 — and the only way to stop it is to open the library again.
    m_audioPlayer->stop();

    KConfigGroup group = windowGroup();
    KWindowConfig::saveWindowSize(windowHandle(), group);
    // The division is written only while the category column stands. Hidden, its
    // pane measures 0 and saveState() would carry that 0 into the file — the
    // next start would restore a column that is formally there and shows
    // nothing, which is the window issue #18's comment at the restore describes
    // (issue #134). What is written then is the division of the moment it was
    // hidden, so bringing the column back brings back its old width and not the
    // default.
    group.writeEntry("ColumnSizes", m_sidebar->isVisible() ? m_splitter->saveState() : m_columnSizes);
    group.writeEntry("SidebarHidden", !m_sidebar->isVisible());
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

    // The same text that picks the notes says what is marked in them, and it
    // is the parsed terms rather than the field: `tag:` and its four siblings
    // pick a note and stand in none of them (issue #77).
    m_delegate->setSearchTerms(parseSearchQuery(m_search->text()).terms);

    // The counters stand for the whole library and not for what the list beside
    // them is showing, so they are asked of the store rather than counted off
    // the result below (issue #18).
    updateCategoryCounts();

    // An empty search field returns the whole library from the store, so the
    // full list and a result list are the same code path — and clearing the
    // field needs no case of its own (SPEC 6). The category column narrows what
    // comes back; both narrow the same list and neither replaces the other.
    QList<Note> notes = m_store->search(m_search->text());
    applyCategoryFilter(notes);
    m_model->setNotes(notes, referenceTime());
    m_groupedOn = referenceTime().date();

    // Saying the empty selection explicitly also stops QAbstractItemView from
    // picking the first entry on its own the moment the list takes the focus.
    const int row = m_model->rowOf(selected);
    m_list->setCurrentIndex(row >= 0 ? m_model->index(row) : QModelIndex());

    updatePages();
}

void LibraryWindow::takeUpNewNotes()
{
    // A window nobody has on screen has nothing to keep up to date: showLibrary()
    // reads the store on its way up. A minimized one counts as visible here for
    // the same reason it does there — the user gets it back as he left it.
    if (!isVisible()) {
        m_newNoteWaits = false;
        return;
    }

    // The condition showLibrary() names, and it holds for this road just as
    // much: a note in its grace period is still in the store, and reading the
    // store now would fetch it back into a list that is counting it down.
    //
    // The note waits instead of the deletion being carried out — which is what
    // the search field does (searchChanged()). Who acted is the difference:
    // typing a search is an act in this window, in sight of the message and of
    // the seconds it is counting. Writing a note in the capture window is not,
    // and it must not spend an undo the user is still being offered here.
    if (m_deletion->isPending()) {
        m_newNoteWaits = true;
        return;
    }

    m_newNoteWaits = false;
    reloadKeepingThePlace();
}

void LibraryWindow::reloadKeepingThePlace()
{
    // Where the list stands, held by the note it stands on rather than by the
    // row number: the new note takes the top row and pushes everything below it
    // down, so the number would name the neighbour afterwards.
    const QScrollBar *bar = m_list->verticalScrollBar();
    const bool standsAtItsBeginning = bar->value() == bar->minimum();

    const QModelIndex top = m_list->indexAt(QPoint(0, 0));
    // A head carries no note of its own, so the row is held by the note under
    // it — and put back one row higher, where its head stands again.
    const bool topIsAHead = top.data(NoteListModel::GroupHeaderRole).toBool();
    const qint64 anchor = m_model->noteAt(topIsAHead ? top.row() + 1 : top.row()).id;

    reload(Selection::Keep);

    // A list standing at its beginning keeps no place: its beginning is where
    // the new note goes, and that is where the user has to see it (issue #105).
    if (standsAtItsBeginning || anchor < 0) {
        return;
    }

    int row = m_model->rowOf(anchor);
    if (topIsAHead && row > 0) {
        --row;
    }
    if (row >= 0) {
        // The last word on where the list stands, and it has to be: putting the
        // selection back can move the list by itself — showNote() fetches the
        // head of the group it finds the selection in, and after a rebuilt list
        // that counts as a crossed boundary (issue #70).
        m_list->scrollTo(m_model->index(row), QAbstractItemView::PositionAtTop);
    }
}

void LibraryWindow::regroupList()
{
    const qint64 selected = m_model->noteAt(m_list->currentIndex().row()).id;

    m_model->regroup(referenceTime());
    m_groupedOn = referenceTime().date();

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

    // ponytail: one keystroke, one whole query — nothing waits between the key
    // and the store, and Store::search() runs on the thread that draws.
    // Ceiling: at the 20,000 notes SPEC 6 sizes the index for, a term that
    // matches every note costs 120 ms in the query and 26 MiB for the list, and
    // the window stands still for that long under every further key; the same
    // corpus with 50 hits costs 0.4 ms (measured in issue #78, the bench is
    // `tests/searchbench.cpp`). The way up is a restartable QTimer of some
    // 150 ms in front of reload(), so that only the pause in the typing reaches
    // the store. Not a LIMIT on the result list: that was measured as well
    // (26 ms instead of 120) and the customer turned it down on 2026-08-28 — it
    // costs a number, a hint line and a rule the user has to learn (SPEC 6).

    // Before the list, because the mark of the column decides what
    // applyCategoryFilter() takes out (issue #18).
    followTheSearchField();

    // The note the user was reading stays selected if it is among the hits.
    reload(Selection::Keep);
}

void LibraryWindow::updatePages()
{
    const bool hasNotes = m_model->noteCount() > 0;

    // Asked of the parsed query and no longer of the field being empty: a field
    // holding nothing but `kat:cli` is the category column speaking, and it has
    // its own sentence below (issue #18).
    const SearchQuery query = parseSearchQuery(m_search->text());
    const bool searching = !query.tags.isEmpty() || !query.types.isEmpty() || query.before.isValid()
        || query.after.isValid() || !query.terms.isEmpty();
    const CategoryFilter chosen = chosenFilter(m_categories);
    const bool byCategory = !query.categories.isEmpty() || isMachineState(chosen);

    if (hasNotes) {
        m_listPages->setCurrentWidget(m_list);
    } else if (!searching && !byCategory) {
        m_listPages->setCurrentWidget(m_emptyLibraryPage);
    } else {
        // A search over an empty library lands here too and offers to change
        // the term. Clearing the field then says "Noch keine Notizen", so the
        // window corrects itself with the next keystroke.
        //
        // A category with nothing in it says something else, and it says it
        // often: until the analysis run of M3 has been through the library
        // every category is empty, and "change the search term" would point at
        // a field the user never touched (issue #18).
        // And the two machine-state entries say a third and a fourth thing:
        // each stays standing while it is chosen even after its last note has
        // left it, and that is the moment these sentences are for.
        if (searching) {
            m_noResultsHint->setText(i18n("Change the search term or clear the field."));
        } else if (chosen == CategoryFilter::Unclassified) {
            m_noResultsHint->setText(i18n("Every note has been classified."));
        } else if (chosen == CategoryFilter::Waiting) {
            m_noResultsHint->setText(i18n("The analysis has caught up."));
        } else {
            m_noResultsHint->setText(i18n("No note carries this category yet."));
        }
        m_listPages->setCurrentWidget(m_noResultsPage);
    }

    if (!hasNotes) {
        m_detailPages->setCurrentWidget(m_blankPage);
    } else if (m_list->currentIndex().isValid()) {
        m_detailPages->setCurrentWidget(m_detailPage);
    } else {
        m_detailPages->setCurrentWidget(m_noSelectionPage);
    }

    // A note that leaves the pane takes its sound with it. Deleting one selects
    // nothing, and a hidden player would otherwise play the deleted note to the
    // end.
    if (m_detailPages->currentWidget() != m_detailPage) {
        m_audioPlayer->stop();
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

void LibraryWindow::repaintTheRowAbove(const QModelIndex &row)
{
    if (!row.isValid() || row.row() == 0) {
        return;
    }

    m_list->update(m_model->index(row.row() - 1));
}

void LibraryWindow::showNote(const QModelIndex &index, const QModelIndex &previous)
{
    // Both ends of the move first, and before every guard below: the separator
    // lines have to follow the selection wherever it goes, and one of the ways
    // out of here is the one that puts it back. Repainting a row asks no
    // question and costs nothing when there is nothing to change, so it needs
    // none of the conditions the rest of this function turns on (issue #101).
    repaintTheRowAbove(index);
    repaintTheRowAbove(previous);

    // The way back out of a cancelled switch runs through here as well, and it
    // must not raise the same question a second time. The mark of the press
    // stays where it is: it belongs to the switch this one is putting back, and
    // that switch is still being handled.
    if (m_restoringSelection) {
        return;
    }

    // The mark is consumed by the call it belongs to, whichever way that call
    // leaves — and there are four ways out of this one. A reset posted to the
    // event loop instead would be carried out inside the guard dialog, which
    // runs a loop of its own: the mark would be gone before the answer came
    // back (issue #57).
    const QScopeGuard pressConsumed = qScopeGuard([this] { m_selectionFollowsAPress = false; });

    // A rebuilt list can move the note under the editor into another row —
    // that is no change of note and no reason to ask anything (SPEC 9).
    if (isEditing() && m_model->noteAt(index.row()).id != m_editedNote.id) {
        // An untouched editor is left behind without a word; only a change
        // that would be lost is worth a dialog (wireframe 2a, state C).
        if (!hasUnsavedChanges()) {
            stopEditing();
        } else {
            // The selection goes back onto the edited note before the question
            // is asked. currentChanged runs after the selection has already
            // jumped, so without this the list highlights one note while the
            // dialog asks about another — and that is the moment the user has
            // to decide in (UI review of 02.08.2026, finding 2).
            //
            // Told by the note's id, not by `previous`: the list may have been
            // rebuilt since, and a row number would then point somewhere else.
            const int editedRow = m_model->rowOf(m_editedNote.id);
            if (editedRow >= 0) {
                m_restoringSelection = true;
                m_list->setCurrentIndex(m_model->index(editedRow));
                m_restoringSelection = false;
            }

            switch (askAboutUnsavedChanges()) {
            case UnsavedAnswer::Save:
                saveEdit();
                break;
            case UnsavedAnswer::Discard:
                stopEditing();
                break;
            case UnsavedAnswer::Cancel:
                // The selection already stands where it belongs.
                return;
            }

            // The edit state has ended, so carrying the switch out now takes
            // the ordinary path — the group logic then even sees the note that
            // was really left behind as the predecessor.
            if (editedRow >= 0) {
                m_list->setCurrentIndex(index);
                return;
            }
        }
    }

    if (index.isValid()) {
        // Four conditions have to hold before the list is moved for a head,
        // and each of them keeps out a way of moving it against the user.
        //
        // The selection was made with a key, not with the mouse. Pressing an
        // arrow key, the user moves through a list and expects it to move with
        // him; clicking, he points at a place and expects that place to stay
        // (issue #57, UI review of 01.08.2026). What the head would have told
        // him stands in the reading pane anyway.
        //
        // It crosses a group boundary — what AK 7 and wireframe 3b, case 4 ask
        // for, and what the first selection after opening counts as — or it
        // reaches the first note of its group, which needs no boundary to be
        // crossed (issue #70). Without the head in view, nothing says which of
        // the five groups the entry stands in — the entry names its own date
        // and time (issue #108), never the group.
        //
        // Otherwise moving within a group fetches nothing: the user has rolled
        // the list to where he wants it, and one arrow key must not throw that
        // away, least of all against the direction he presses in.
        //
        // Whether the entry is in the picture already does not enter into it.
        // A note can stand in full view while its head sits just above the
        // upper edge — that is the very case this fetches the head for.
        //
        // And both fit into the list at once. In a group taller than the
        // window the head cannot be shown without pushing the selection out.
        const QModelIndex head = groupHeadOf(index);
        const std::optional<library::NoteGroup> group = groupOf(index);
        const std::optional<library::NoteGroup> previousGroup = groupOf(previous);
        const bool crossesAGroupBoundary = !previousGroup.has_value() || previousGroup != group;

        // Told by the structure, not by pixels: a head always stands
        // immediately above the first note of its group, so one row up is the
        // whole test. No measure of how far the head misses the picture enters
        // into it — that number belongs to a screen, and every screen has its
        // own (issue #70).
        const bool isFirstOfItsGroup = head.isValid() && head.row() == index.row() - 1;

        if (head.isValid() && (crossesAGroupBoundary || isFirstOfItsGroup) && !m_selectionFollowsAPress) {
            const QRect heading = m_list->visualRect(head);
            const QRect selected = m_list->visualRect(index);
            if (selected.bottom() - heading.top() <= m_list->viewport()->height()) {
                m_list->scrollTo(head, QAbstractItemView::EnsureVisible);
            }
        }

        // The same mark holds the list still for the selection itself. A press
        // sets the current row first — which brings us here — and only
        // afterwards picks its selection from the rectangle it remembered at
        // the press. Moving the list in between hands that rectangle another
        // row, and the click ends up on the neighbour of what was pointed at
        // (issue #71). A row the lower edge cuts through therefore stays cut
        // through: it would only become fully visible by moving out from under
        // the cursor, which is the fault itself (design decision of 05.08.2026).
        if (!m_selectionFollowsAPress) {
            m_list->scrollTo(index, QAbstractItemView::EnsureVisible);
        }

        showNoteText(index);
    }

    updatePages();
    updateEditState();
}

void LibraryWindow::showNoteText(const QModelIndex &index)
{
    const Note note = m_model->noteAt(index.row());

    // The detail pane stands under no head and carries the weekday as well.
    m_detailTimestamp->setText(library::relativeTimestamp(note.createdAt, QLocale()));

    // And the origin under it, or nothing at all (SPEC 5.1, 13; issue #47).
    m_originText = originLine(note.origin, note.originApp);
    showOrigin();

    // The player belongs to a voice note and to no other, and it is set even
    // when it stays hidden — an empty source is what stops the file of the note
    // just left from playing on.
    const bool spoken = note.type == Note::Type::Audio && !note.audioPath.isEmpty();
    m_audioPlayer->setSource(spoken ? m_store->audioDirectory() + QLatin1Char('/') + note.audioPath
                                    : QString(),
                             note.audioDurationS.value_or(0));
    m_audioPlayer->setVisible(spoken);

    // Setting the same text again would send the reader back to its first
    // line; a reload of the open window leaves the reader where it was.
    if (m_detailText->toPlainText() != note.content) {
        m_detailText->setPlainText(note.content);
    }

    // The readable label is made here (SPEC 7.2); the database keeps the short
    // form. Without a category and without tags the row hides itself.
    m_chips->setChips(categoryLabel(note.category), m_store->tags(note.id));
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

    // Nothing is counting down any more, so a note that arrived meanwhile can
    // come in — and it finds the undone note already back in the list.
    if (m_newNoteWaits) {
        takeUpNewNotes();
    }
}

void LibraryWindow::showOrigin()
{
    // No menu where there is nothing to take off, and none over an open editor.
    m_detailOrigin->setContextMenuPolicy(m_originText.isEmpty() || isEditing() ? Qt::NoContextMenu
                                                                              : Qt::ActionsContextMenu);

    // Hidden and not merely empty: a hidden widget is out of the layout, so a
    // note without an origin keeps the note text where it always stood
    // (acceptance criterion 5).
    m_detailOrigin->setVisible(!m_originText.isEmpty());
    if (m_originText.isEmpty()) {
        m_detailOrigin->clear();
        return;
    }

    // The „ · " inside the text joins the application to the window title and
    // no longer this line to the timestamp — originLine() writes it, and only
    // where there is something on both sides of it.
    m_detailOrigin->setText(m_detailOrigin->fontMetrics().elidedText(m_originText, Qt::ElideRight,
                                                                     m_detailOrigin->width()));
}

void LibraryWindow::removeOrigin()
{
    const QModelIndex current = m_list->currentIndex();
    const int index = current.isValid() ? m_model->noteIndexAt(current.row()) : -1;
    if (index < 0) {
        return;
    }

    Note note = m_model->noteAt(current.row());
    if (note.origin.isEmpty() && note.originApp.isEmpty()) {
        return;
    }

    // Kept before the write, not read back after it: what the "Undo" needs is
    // the pair that stood there, and after the update nothing holds it any more.
    m_removedOriginId = note.id;
    m_removedOrigin = note.origin;
    m_removedOriginApp = note.originApp;

    note.origin.clear();
    note.originApp.clear();
    if (!m_store->updateNote(note)) {
        qWarning("Removing the origin failed: %s", qPrintable(m_store->lastError()));
        m_removedOriginId = -1;
        return;
    }

    // As saveEdit() does it: the list is not read from the store again, because
    // nothing about which notes belong in it has changed.
    m_model->replaceNote(index, note);
    m_originText.clear();
    showOrigin();
    showOriginMessage();
}

void LibraryWindow::restoreOrigin()
{
    if (m_removedOriginId < 0) {
        return;
    }

    // Out of the store and not out of the model: the text may have been edited
    // in between, and writing the model's copy back would take that edit with
    // it. A note that is gone meanwhile has nothing to write onto.
    const std::optional<Note> stored = m_store->note(m_removedOriginId);
    if (stored.has_value()) {
        Note note = *stored;
        note.origin = m_removedOrigin;
        note.originApp = m_removedOriginApp;
        if (m_store->updateNote(note)) {
            const int index = m_model->noteIndexAt(m_model->rowOf(note.id));
            if (index >= 0) {
                m_model->replaceNote(index, note);
            }
            if (m_model->rowOf(note.id) == m_list->currentIndex().row()) {
                m_originText = originLine(note.origin, note.originApp);
                showOrigin();
            }
        } else {
            qWarning("Putting the origin back failed: %s", qPrintable(m_store->lastError()));
        }
    }

    m_removedOriginId = -1;
    m_undoOriginAction->setEnabled(false);
    m_message->animatedHide();
}

void LibraryWindow::showOriginMessage()
{
    // The band carries one button, and it has to be this one: the deletion's
    // would take back a deletion.
    m_message->removeAction(m_undoAction);
    if (!m_message->actions().contains(m_undoOriginAction)) {
        m_message->addAction(m_undoOriginAction);
    }
    m_undoOriginAction->setEnabled(true);
    // One short line beside a button, as the deletion has it (wireframe 2b).
    m_message->setWordWrap(false);
    m_message->setMessageType(KMessageWidget::Information);
    m_message->setText(i18n("Origin removed"));
    if (!m_message->isVisible()) {
        m_message->animatedShow();
    }
}

bool LibraryWindow::isEditing() const
{
    return m_editedNote.id >= 0;
}

bool LibraryWindow::hasUnsavedChanges() const
{
    return isEditing() && m_editor->toPlainText() != m_textBeforeEditing;
}

void LibraryWindow::startEditing()
{
    const Note note = m_model->noteAt(m_list->currentIndex().row());
    // A group head carries no note, and the selection never lands on one
    // (wireframe 3b) — F2 finds nothing to edit there.
    if (isEditing() || note.id < 0) {
        return;
    }

    m_editedNote = note;
    m_textBeforeEditing = note.content;
    m_editor->setPlainText(note.content);

    // Wireframe 2a: the cursor stands at the end of the text and nothing is
    // selected — a first keystroke must not be able to overwrite the note.
    m_editor->moveCursor(QTextCursor::End);

    // The readable label here as well as on the pill of the read state: the
    // short form is what the database keeps, and one pane must not spell the
    // same value two ways (SPEC 7.2). The row itself stays what wireframe 2a
    // draws — labels and not marks (UX decision 2026-08-29).
    m_category->setText(note.category.isEmpty() ? missingValue() : categoryLabel(note.category));
    const QStringList tags = m_store->tags(note.id);
    m_tags->setText(tags.isEmpty() ? missingValue() : tags.join(QStringLiteral(" · ")));

    updateEditState();
    m_editor->setFocus();
}

void LibraryWindow::saveEdit()
{
    const QString content = m_editor->toPlainText().trimmed();
    // An empty field is no state to save: deleting runs over the delete
    // action, not over emptying the field (wireframe 2a).
    if (!isEditing() || content.isEmpty()) {
        return;
    }

    // **The row as it stands now, not the copy the editor was opened with.**
    // updateNote() writes every column, and m_editedNote is a snapshot: what
    // changed on the note while the editor stood is written back the way it
    // was, silently. The origin is the case that made it visible — removed,
    // then put back through the band's "Undo" while the editor was open, the
    // save wrote the snapshot and took the restored value away again without a
    // word (finding of the review, 29.08.2026). Every column a later story adds
    // beside the text falls into the same hole; reading the row back is the one
    // place that closes it for all of them.
    const std::optional<Note> stored = m_store->note(m_editedNote.id);
    if (!stored.has_value()) {
        qWarning("The note being edited is gone: %s", qPrintable(m_store->lastError()));
        return;
    }

    Note note = *stored;
    note.content = content;
    // SPEC 9: editing keeps category, tags and state and sets needs_reembed —
    // only the embedding ages with the text (7.2). The full-text index follows
    // from the update trigger on `notes` (SPEC 5.1).
    note.needsReembed = true;

    if (!m_store->updateNote(note)) {
        // Keep the editor and its text: a lost correction is worse than an
        // editor that stays open, as in the capture window (SPEC 3).
        qWarning("Saving the note failed: %s", qPrintable(m_store->lastError()));
        return;
    }

    // The list is not read from the store again. Nothing but this one text has
    // changed, and a note whose new text drops it out of the running result
    // list has to stay in sight until the search term changes (issue #11, K2).
    m_model->replaceNote(m_model->noteIndexAt(m_model->rowOf(note.id)), note);

    stopEditing();
}

void LibraryWindow::cancelEdit()
{
    if (!isEditing()) {
        return;
    }

    if (hasUnsavedChanges()) {
        switch (askAboutUnsavedChanges()) {
        case UnsavedAnswer::Save:
            saveEdit();
            showNoteText(m_list->currentIndex());
            return;
        case UnsavedAnswer::Cancel:
            return;
        case UnsavedAnswer::Discard:
            break;
        }
    }

    // The reader still holds the stored text — it was never written over.
    stopEditing();
}

void LibraryWindow::stopEditing()
{
    m_editedNote = Note();
    m_textBeforeEditing.clear();
    m_editor->clear();

    updateEditState();
    m_list->setFocus();
}

LibraryWindow::UnsavedAnswer LibraryWindow::askAboutUnsavedChanges()
{
    // KMessageDialog, not QMessageBox: under the KDE platform integration a
    // built QMessageBox is not the dialog the user gets — the integration
    // answers with a message box of its own and takes over labels, roles and
    // order but nothing set on our buttons afterwards (issue #66, measured
    // 02.08.2026). A KMessageDialog is a plain QDialog and stays ours.
    //
    // The timestamp stands in brackets, not in the middle of the sentence: it
    // comes in the full detail-pane form — “Monday, 8/24/2026 3:42 PM”
    // under en_US — and no single sentence carries weekday, date and time as
    // one object.
    // The brackets take the grammar out of the format's hands.
    //
    // Both sentences stand in one text because KMessageDialog has no
    // informative text beside the main one (wireframe 2a, state C).
    KMessageDialog dialog(KMessageDialog::WarningTwoActionsCancel,
                          i18n("Save changes?\n\nThe edited note (%1) has unsaved changes. "
                               "Without saving they are lost.",
                               library::relativeTimestamp(m_editedNote.createdAt, QLocale())),
                          this);
    dialog.setCaption(i18nc("@title:window", "Unsaved changes"));

    // The warning symbol, set out loud although the dialog type is a warning
    // one. KMessageDialog::setIcon() promises a generic symbol by type for a
    // null QIcon; measured on 02.08.2026 it hands out none, and the dialog
    // then carries no picture label at all. A dialog about losing work is the
    // very case the symbol exists for — and it is the one symbol the
    // platform's substitute dialog did have (wireframe 2a, state C).
    dialog.setIcon(QIcon::fromTheme(QStringLiteral("dialog-warning")));

    // Symbols from KStandardGuiItem, so they are the ones every other KDE
    // dialog uses — the icons only: the texts stay under this application's own
    // i18n() and must not depend on which KF6 catalogue happens to be installed.
    //
    // The order the three appear in is the platform's. What this window fixes
    // is which answer means what: primary saves, secondary discards, cancel
    // stays (wireframe 2a, state C).
    dialog.setButtons(KGuiItem(i18n("Save"), KStandardGuiItem::save().icon()),
                      KGuiItem(i18n("Discard"), KStandardGuiItem::discard().icon()),
                      KGuiItem(i18n("Cancel"), KStandardGuiItem::cancel().icon()));

    // „Save“ is the default answer, because Return then does what someone
    // who has just been typing most likely means, and it is the one answer
    // that loses nothing (design decision F3 of 02.08.2026). It has to be said out
    // loud: the KDE build puts the default on the cancel button.
    //
    // Hence the three lines instead of a plain exec(), each of them measured
    // on 02.08.2026:
    //  * a default set before show() is overwritten while showing, so it has
    //    to be set afterwards;
    //  * showing by hand costs the modality exec() would have set, because
    //    exec() no longer makes a dialog modal once it is visible — the guard
    //    then stands open with no way for anyone to answer it;
    //  * so the modality is set by hand as well.
    dialog.setWindowModality(Qt::ApplicationModal);
    dialog.show();
    putReturnOnThePrimaryAction(&dialog);

    // Esc is answered by the dialog itself and comes back as Cancel — the
    // harmless answer, back into the edit state (measured 02.08.2026).
    switch (dialog.exec()) {
    case KMessageDialog::PrimaryAction:
        return UnsavedAnswer::Save;
    case KMessageDialog::SecondaryAction:
        return UnsavedAnswer::Discard;
    default:
        // Cancel, Esc and a dialog closed from outside all end in the state
        // that loses nothing.
        return UnsavedAnswer::Cancel;
    }
}

void LibraryWindow::updateEditState()
{
    const bool editing = isEditing();
    const bool hasNote = m_list->currentIndex().isValid();

    m_textPages->setCurrentWidget(editing ? static_cast<QWidget *>(m_editor) : m_detailText);
    // Page 0 carries the two buttons, page 1 the badge.
    m_headPages->setCurrentIndex(editing ? 1 : 0);
    m_metaRow->setVisible(editing);
    // The other half of the same swap: pills while reading, fields while
    // editing (UX decision 2026-08-29 on issue #18). A note that carries
    // neither shows nothing in either state.
    m_chips->setVisible(!editing && !m_chips->isEmpty());
    m_editFooter->setVisible(editing);

    // Dimmed while the transcript is edited (wireframe 2a, state B): the audio
    // file is never changed, only its transcript. And it falls silent as it is
    // dimmed — a player that keeps playing under a button the same act has just
    // switched off is a recording nobody can stop.
    m_audioPlayer->setEnabled(!editing);
    if (editing) {
        m_audioPlayer->stop();
    }

    // And neither is its origin: showOrigin() takes the context menu off the
    // line while the editor stands (issue #47). The band's "Undo" stays
    // reachable — since saveEdit() reads the row back, a restored origin
    // survives the save.
    showOrigin();

    // The note under the editor is not up for deletion — the button is gone,
    // and so is the key behind it.
    m_deleteAction->setEnabled(!editing && hasNote);
    m_editAction->setEnabled(!editing && hasNote);
    m_cancelEditAction->setEnabled(editing);

    const bool savable = editing && !m_editor->toPlainText().trimmed().isEmpty();
    m_saveAction->setEnabled(savable);
    m_saveButton->setEnabled(savable);

    // A search while the editor is open would rebuild the list under it, and a
    // cancelled switch would then have no row left to return to. The field
    // comes back the moment the edit state ends (discovered condition, SPEC 9).
    //
    // And it says why while it rests: the KDE HIG argue against controls that
    // are switched off without a visible reason, and that the field is off is
    // all one could see (UI review of 02.08.2026, finding 4). The hint goes
    // with the state that explains it.
    m_search->setEnabled(!editing);
    m_search->setToolTip(editing ? i18n("Switched off while editing — a search would rebuild the "
                                        "list underneath the editor.")
                                 : QString());

    // The category column rebuilds the same list and is switched off for the
    // same reason, and says so in the same way.
    m_categories->setEnabled(!editing);
    m_categories->setToolTip(m_search->toolTip());
    // And the entry that hides the column with it: hiding it takes "Unclassified"
    // back to "All", and that rebuilds the list — the very thing the two lines
    // above are switched off for (issue #134).
    m_sidebarAction->setEnabled(!editing);
}

void LibraryWindow::startAnalysis()
{
    m_analysisRequestedHere = true;
    // Synchronous as far as the scheduler is concerned: main() connects this
    // signal straight to AnalysisScheduler::analyzeNow(), so by the time the
    // emit returns the run is either out at the provider or already through.
    Q_EMIT analysisRequested();

    // The band reports the state the scheduler is in **after** the press, never
    // the one that was expected of it (CLAUDE.md, finding 27). A run with
    // nothing to analyse walks all three steps of SPEC 7.2 inside that call and
    // never reports itself busy, so "Analysis running…" would stand there for
    // good with nothing left to take it back. The action carries that state,
    // because setAnalysisBusy() is the one place it is set from.
    //
    // Nothing to do when the flag is already down: setAnalysisBusy() has then
    // reported the end itself, inside the emit above.
    if (m_analysisRequestedHere) {
        const bool running = !m_analysisAction->isEnabled();
        m_analysisRequestedHere = running;
        showBandMessage(running ? i18n("Analysis running…") : i18n("Analysis finished."), false);
    }
}

void LibraryWindow::setAnalysisBusy(bool busy)
{
    // The whole guard against a second run from this window, the way the export
    // action is the whole guard against a second export.
    m_analysisAction->setEnabled(!busy);
    m_analysisAction->setText(busy ? i18n("Analysis running…") : i18n("Analyze now"));

    // Only for the run this window asked for: the band belongs to the act in
    // the window (wireframe 2b), and a periodic run would otherwise write a
    // line into it every half hour with nobody having done anything.
    if (!busy && m_analysisRequestedHere) {
        m_analysisRequestedHere = false;
        showBandMessage(i18n("Analysis finished."), false);
    }
}

void LibraryWindow::showSidebar(bool visible)
{
    if (!visible) {
        // Remembered before the column goes, because from here on saveState()
        // writes a 0 for its pane (issue #134, and the reason at closeEvent()).
        m_columnSizes = m_splitter->saveState();

        // A filter the window no longer shows must not go on filtering — the
        // acceptance criterion of issue #134 and the plain reading of it. A
        // **category** needs nothing done: categoryChosen() has written its
        // `kat:` into the search field, where it stays readable with the column
        // gone. The two **machine states** of issue #133 are the ones the
        // search language of SPEC 6 has no word for, so hiding the column while
        // one of them is marked would leave a filter with nothing on screen
        // naming it; they are taken back to "All" instead.
        //
        // Through isMachineState() and not through a condition written out
        // again here: the same rule copied to a second place is what the review
        // of issue #133 found and CLAUDE.md's finding 48 describes — two
        // transcriptions drift, and nothing in the build says they have.
        if (isMachineState(chosenFilter(m_categories))) {
            // The "All" entry, which buildSidebar() adds before every other one.
            m_categories->setCurrentItem(m_categories->topLevelItem(0));
        }
    }

    // Not a width of 0: setChildrenCollapsible(false) stays, and the splitter
    // hands the freed room to the note list and the reading pane by itself.
    m_sidebar->setVisible(visible);
}

void LibraryWindow::startFullExport()
{
    // The folder is asked for at every run rather than fixed: a rescue path
    // that writes into the home directory without a word is the case in which
    // the user hunts for the result afterwards, and whoever exports mostly
    // wants a stick or a vault (UX decision 2026-08-29).
    const QString parent =
        QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Export all notes"), QDir::homePath());
    if (parent.isEmpty()) {
        return;
    }

    // The whole guard against two runs at once. The act has one door, and it
    // is this action.
    m_exportAction->setEnabled(false);
    showBandMessage(i18n("Export running…"), false);

    // One turn of the event loop, so the line above stands on screen before
    // the writing starts — the export holds this thread, and a text set and
    // replaced inside one turn is never painted. A thread would buy nothing
    // here: the run copies a few hundred small files off the local disk.
    QTimer::singleShot(0, this, [this, parent] {
        const FullExportResult result = ::exportAllNotes(*m_store, parent);
        m_exportAction->setEnabled(true);

        if (!result.ok()) {
            showBandMessage(result.error, true);
            return;
        }

        // The count is the one the export read back off the folder, not the
        // one its loop kept — the number the user reads has to come from the
        // folder it names.
        QString text =
            i18np("%1 note exported to %2.", "%1 notes exported to %2.", result.noteCount, result.directory);
        // Named, not passed over in silence. The reasons go into the log, where
        // a silent fault is looked for anyway.
        const QStringList reasons = result.missing + result.incomplete;
        for (const QString &line : reasons) {
            qWarning("Full export: %s", qUtf8Printable(line));
        }
        // Two sentences and not one: a note that never reached the folder and a
        // note that reached it without its recording call for different next
        // steps, and calling both of them incomplete would name the milder of
        // the two for the worse case.
        if (!result.missing.isEmpty()) {
            text += QLatin1Char(' ')
                + i18np("%1 note is missing, see the log.",
                        "%1 notes are missing, see the log.",
                        static_cast<int>(result.missing.size()));
        }
        if (!result.incomplete.isEmpty()) {
            text += QLatin1Char(' ')
                + i18np("%1 note is without its recording, see the log.",
                        "%1 notes are without their recording, see the log.",
                        static_cast<int>(result.incomplete.size()));
        }
        showBandMessage(text, !result.missing.isEmpty() || !result.incomplete.isEmpty());
    });
}

void LibraryWindow::showBandMessage(const QString &text, bool isError)
{
    // Neither "Undo" belongs to this line; left standing in the band one of
    // them would sit beside it as a greyed out button.
    m_message->removeAction(m_undoAction);
    m_message->removeAction(m_undoOriginAction);
    // The folder the user picked can be a long path, and without word wrap the
    // band widens the window until it fits. It costs the second row the
    // deletion saves, and this message has no button to put into it.
    m_message->setWordWrap(true);
    m_message->setMessageType(isError ? KMessageWidget::Error : KMessageWidget::Information);
    m_message->setText(text);
    if (!m_message->isVisible()) {
        m_message->animatedShow();
    }
    // The line stays until something else claims the band — the path is what
    // the user came for, and a line that fades takes it away again.
    //
    // A deletion running at the same moment does take the band away, on three
    // roads and not one: its next countdown tick overwrites the text, and both
    // its ends — PendingDeletion::committed and undoDeletion() — hide the band
    // outright. All three lie inside the same grace period of five seconds, so
    // that is the whole window in which an export message can be lost, and the
    // folder stands on disk whatever happens to the line. Naming only the tick
    // would read more complete than it is.
}
