#include "ui/proposalwindow.h"

#include "analysis/suggester.h"
#include "proposals/bundleexport.h"
#include "store/store.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KSharedConfig>
#include <KStandardShortcut>

#include <QAction>
#include <QFontDatabase>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <utility>

namespace
{
constexpr int WindowWidth = 760;
constexpr int WindowHeight = 600;

/**
 * How many note rows a card shows before its list starts scrolling.
 *
 * SPEC 7.3 bundles from three notes upwards and puts no ceiling on a cluster,
 * so a card can carry more rows than a window has room for. Eight is what
 * wireframe 1c draws with its "+ 3 more" line, and the rest is reached by
 * scrolling inside the card rather than by the card growing without end.
 */
constexpr int VisibleNoteRows = 8;

/** What a note is called in the list: its first line, elided by the view. */
QString noteLine(const Note &note)
{
    return note.content.trimmed().section(QLatin1Char('\n'), 0, 0);
}

/**
 * The vault folder the export writes into.
 *
 * Group and key are `Settings`' own (settings/settings.cpp, group "Export"),
 * read here the way Transcriber and AnalysisScheduler read theirs: this
 * library links no settings library, and a second spelling of the key would
 * send the export somewhere the settings page never pointed.
 *
 * Read at the moment of the export and not once at build time, so a folder the
 * user has just set in the dialog takes effect on the card standing open.
 */
QString vaultPath()
{
    return KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Export"))
        .readEntry("VaultPath", QString());
}

/** The heading and the sentence of the page shown while nothing is waiting. */
QWidget *emptyPage()
{
    auto *page = new QWidget();
    auto *layout = new QVBoxLayout(page);
    layout->addStretch();

    auto *heading = new QLabel(i18n("No suggestions waiting"), page);
    heading->setAlignment(Qt::AlignCenter);
    QFont bold = heading->font();
    bold.setBold(true);
    heading->setFont(bold);
    layout->addWidget(heading);

    auto *hint = new QLabel(i18n("The analysis run offers a bundle once several notes belong together."), page);
    hint->setAlignment(Qt::AlignCenter);
    hint->setWordWrap(true);
    // The role and not a colour: the window lives as long as the daemon, and a
    // colour taken out of the palette once would stay put when the user
    // changes the colour scheme.
    hint->setForegroundRole(QPalette::PlaceholderText);
    layout->addWidget(hint);

    layout->addStretch();
    return page;
}
}

ProposalWindow::ProposalWindow(Store *store, QWidget *parent)
    : QWidget(parent)
    , m_store(store)
    , m_message(new KMessageWidget(this))
    , m_pages(new QStackedWidget(this))
{
    // Only what this window is: the decoration appends the application name by
    // itself, the way the library's title was measured (issue #16).
    setWindowTitle(i18nc("@title:window", "Review suggestions"));

    // The band of wireframe 2b, in the library's make: no close button, hidden
    // it takes no height, and word wrap because a vault path can be long.
    m_message->setCloseButtonVisible(false);
    m_message->setWordWrap(true);
    m_message->hide();

    auto *cards = new QWidget();
    m_cardLayout = new QVBoxLayout(cards);
    m_cardLayout->setContentsMargins(12, 12, 12, 12);
    m_cardLayout->setSpacing(12);
    m_cardLayout->addStretch();

    auto *scroll = new QScrollArea(this);
    scroll->setWidget(cards);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_pages->addWidget(scroll);
    m_pages->addWidget(emptyPage());

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_message);
    layout->addWidget(m_pages, 1);

    auto *closeAction = new QAction(this);
    closeAction->setShortcuts(KStandardShortcut::close());
    connect(closeAction, &QAction::triggered, this, &ProposalWindow::close);
    addAction(closeAction);

    resize(WindowWidth, WindowHeight);
    reload();
}

void ProposalWindow::showProposals()
{
    // The open window reads again as well: a run that finished while it stood
    // there has written new suggestions, and the card of one that was answered
    // elsewhere is gone.
    reload();

    if (isMinimized()) {
        showNormal();
    } else {
        show();
    }
    raise();
    activateWindow();
}

void ProposalWindow::reload()
{
    for (const Card &card : std::as_const(m_cards)) {
        // Out of the layout at once and freed by the event loop: reload() runs
        // out of a button's own click handler, and deleting that button under
        // its signal would pull the ground from under the emission.
        card.frame->setParent(nullptr);
        card.frame->deleteLater();
    }
    m_cards.clear();

    const QList<Proposal> proposals = m_store->proposals();
    for (const Proposal &proposal : proposals) {
        // Task suggestions belong to issue #31; until then they are passed
        // over rather than shown as a card with no answer behind it.
        if (proposal.kind != Proposal::Kind::Bundle || proposal.status != Proposal::Status::Open) {
            continue;
        }

        Card card;
        card.proposal = proposal;
        for (const qint64 noteId : proposal.noteIds) {
            const std::optional<Note> note = m_store->note(noteId);
            if (note.has_value()) {
                card.notes.append(*note);
            }
        }

        // **A bundle whose notes are all gone does not stand here, and it does
        // not stay in the database either** (issue #30, the finding out of the
        // review of #29). Deleting a note takes its `proposal_notes` row with
        // it but not the suggestion, so a proposal without notes is a state
        // the store can reach — and SPEC 9 already says what such a suggestion
        // is worth: "If the note is part of an open suggestion, editing or
        // deleting discards that suggestion (its preview would be out of
        // date)". With every note gone the preview is not out of date but
        // empty, and there is no question left to put: nothing to export,
        // nothing to keep, nothing the three buttons could mean. A card
        // explaining its own emptiness would ask the user to answer for a row
        // in a table. So the review carries the deletion out that the note's
        // own deletion could not reach, with the same call "Discard" uses —
        // the notes are untouched by it, because there are none.
        if (card.notes.isEmpty()) {
            m_store->removeProposal(proposal.id);
            continue;
        }

        buildCard(card);
        m_cards.append(card);
        // Before the stretch, which keeps the cards at the top of a window
        // with room to spare.
        m_cardLayout->insertWidget(m_cardLayout->count() - 1, card.frame);
    }

    m_pages->setCurrentIndex(m_cards.isEmpty() ? 1 : 0);
}

void ProposalWindow::buildCard(Card &card)
{
    const qint64 id = card.proposal.id;
    const QJsonObject payload = QJsonDocument::fromJson(card.proposal.payload.toUtf8()).object();

    auto *frame = new QFrame();
    frame->setObjectName(QStringLiteral("card-%1").arg(id));
    frame->setFrameShape(QFrame::StyledPanel);
    // A ground of its own, and it is what makes a card a card: Breeze draws no
    // line for `StyledPanel` on a plain QFrame — measured on the first picture
    // of this story, where the card had no visible boundary at all and three
    // of them would have run into one another (wireframe 1c draws a border).
    // The role and not a colour, so a changed colour scheme reaches it
    // (issue #58); `Base` is the content ground the note list and the preview
    // inside already stand on, and it steps away from `Window` in every scheme.
    frame->setAutoFillBackground(true);
    frame->setBackgroundRole(QPalette::Base);

    auto *layout = new QVBoxLayout(frame);

    // The head row of wireframe 1c: what the bundle is called on the left, and
    // where accepting sends it on the right.
    auto *head = new QHBoxLayout();
    auto *title = new QLabel(i18nc("@title, the name of a bundle suggestion",
                                   "Bundle: %1",
                                   payload.value(QLatin1String("title")).toString()),
                             frame);
    QFont bold = title->font();
    bold.setBold(true);
    title->setFont(bold);
    title->setWordWrap(true);
    head->addWidget(title, 1);
    auto *target = new QLabel(i18nc("@info, where an accepted bundle is written", "→ Obsidian _INBOX"), frame);
    target->setForegroundRole(QPalette::PlaceholderText);
    head->addWidget(target);
    layout->addLayout(head);

    // The note list and the preview side by side. An item view rather than a
    // column of QCheckBox: the view elides a long first line by itself and
    // scrolls once the bundle is longer than the card, both of which a row of
    // check boxes would need code for.
    auto *body = new QHBoxLayout();

    auto *noteList = new QListWidget(frame);
    noteList->setObjectName(QStringLiteral("notes-%1").arg(id));
    noteList->setSelectionMode(QAbstractItemView::NoSelection);
    noteList->setUniformItemSizes(true);
    for (const Note &note : std::as_const(card.notes)) {
        auto *item = new QListWidgetItem(noteLine(note), noteList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        // Everything is in the bundle until the user takes it out (wireframe
        // 1c): the suggestion is the analysis run's proposal, and the review
        // narrows it.
        item->setCheckState(Qt::Checked);
        // The whole line as a tooltip, because the row elides it.
        item->setToolTip(note.content.trimmed());
    }

    auto *preview = new QTextBrowser(frame);
    preview->setObjectName(QStringLiteral("preview-%1").arg(id));
    preview->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    // Plain text and not Markdown rendered: what the preview shows is the file
    // that will be written, and a rendered heading would hide the very
    // structure SPEC 8.1 fixes. Wrapped at the widget width, which is the
    // default — measured on the first picture of this story, where `NoWrap`
    // cut every note off at the right edge and put a horizontal scrollbar
    // under a preview four lines high.

    body->addWidget(noteList, 1);
    body->addWidget(preview, 1);
    layout->addLayout(body);

    auto *line = new QFrame(frame);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    auto *actions = new QHBoxLayout();
    auto *acceptButton = new QPushButton(QIcon::fromTheme(QStringLiteral("document-export")),
                                         i18nc("@action:button", "Accept"), frame);
    acceptButton->setObjectName(QStringLiteral("accept-%1").arg(id));
    acceptButton->setToolTip(
        i18nc("@info:tooltip", "Write the collective note and take the notes out of the library"));
    auto *laterButton = new QPushButton(QIcon::fromTheme(QStringLiteral("media-playback-pause")),
                                        i18nc("@action:button", "Later"), frame);
    laterButton->setObjectName(QStringLiteral("later-%1").arg(id));
    laterButton->setToolTip(i18nc("@info:tooltip", "Put the suggestion aside; the notes stay in the library"));
    auto *discardButton = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-delete")),
                                          i18nc("@action:button", "Discard"), frame);
    discardButton->setObjectName(QStringLiteral("discard-%1").arg(id));
    discardButton->setToolTip(i18nc("@info:tooltip", "Drop the suggestion; no note is deleted"));
    actions->addWidget(acceptButton);
    actions->addWidget(laterButton);
    actions->addWidget(discardButton);
    actions->addStretch();
    layout->addLayout(actions);

    // The three buttons are reached through this card alone, so none of them
    // becomes an action of the window: two cards would then carry two actions
    // reading "Accept", and whoever looks one up by its wording takes the
    // first (finding 61). The object names above are what a check addresses
    // one card by.
    connect(acceptButton, &QPushButton::clicked, this, [this, id] {
        accept(id);
    });
    connect(laterButton, &QPushButton::clicked, this, [this, id] {
        defer(id);
    });
    connect(discardButton, &QPushButton::clicked, this, [this, id] {
        discard(id);
    });
    // Connected after the rows are in: setCheckState() while filling would fire
    // this for every one of them, against a card the window does not hold yet.
    connect(noteList, &QListWidget::itemChanged, this, [this, id] {
        selectionChanged(id);
    });

    // The preview of the full bundle, written by the same bundleMarkdown() a
    // deselection writes it with — so the text on the card comes from one place
    // whether anything was taken out or not, and the stored payload is not a
    // second reading of the same bundle.
    preview->setPlainText(
        bundleMarkdown(payload.value(QLatin1String("title")).toString(), card.notes));

    // The height of both halves, in rows: a card of three notes is three rows
    // tall, a longer bundle scrolls inside the card.
    if (noteList->count() > 0) {
        const int rows = qMin(noteList->count(), VisibleNoteRows);
        const int height = rows * noteList->sizeHintForRow(0) + 2 * noteList->frameWidth();
        noteList->setFixedHeight(height);
        preview->setFixedHeight(height);
    }

    card.frame = frame;
    card.noteList = noteList;
    card.preview = preview;
}

ProposalWindow::Card *ProposalWindow::cardFor(qint64 id)
{
    for (Card &card : m_cards) {
        if (card.proposal.id == id) {
            return &card;
        }
    }
    return nullptr;
}

QList<Note> ProposalWindow::selectedNotes(const Card &card) const
{
    QList<Note> kept;
    for (int row = 0; row < card.notes.size(); ++row) {
        if (card.noteList->item(row)->checkState() == Qt::Checked) {
            kept.append(card.notes.at(row));
        }
    }
    return kept;
}

Proposal ProposalWindow::selection(const Card &card) const
{
    const QList<Note> kept = selectedNotes(card);

    Proposal chosen = card.proposal;
    chosen.noteIds.clear();
    for (const Note &note : kept) {
        chosen.noteIds.append(note.id);
    }

    // The Markdown is written again from the notes that are left, with the
    // function the analysis run wrote the stored payload with (SPEC 7.3): the
    // stored text still holds the deselected note, and exportBundle() writes
    // what the payload carries. Rebuilt here, the file in the vault and the
    // notes that are deleted for it are the same set.
    QJsonObject payload = QJsonDocument::fromJson(card.proposal.payload.toUtf8()).object();
    const QString title = payload.value(QLatin1String("title")).toString();
    payload[QLatin1String("markdown")] = bundleMarkdown(title, kept);
    chosen.payload = QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact));

    return chosen;
}

void ProposalWindow::selectionChanged(qint64 id)
{
    Card *card = cardFor(id);
    if (!card) {
        return;
    }

    const QList<Note> kept = selectedNotes(*card);
    const QJsonObject payload = QJsonDocument::fromJson(card->proposal.payload.toUtf8()).object();
    card->preview->setPlainText(bundleMarkdown(payload.value(QLatin1String("title")).toString(), kept));

    // With nothing ticked there is nothing to write: the collective note would
    // be a heading over an empty page, and the export would delete no note for
    // it. "Later" and "Discard" stay reachable — an empty selection is still a
    // card the user may want to put aside.
    auto *accept = card->frame->findChild<QPushButton *>(QStringLiteral("accept-%1").arg(id));
    if (accept) {
        accept->setEnabled(!kept.isEmpty());
    }
}

void ProposalWindow::accept(qint64 id)
{
    const Card *card = cardFor(id);
    if (!card) {
        return;
    }

    const BundleExportResult result = exportBundle(*m_store, selection(*card), vaultPath());
    if (!result.ok()) {
        // The card stays: SPEC 8.2 says so for the task road, and SPEC 8.1
        // builds the bundle road the same way — the file was not written, so
        // nothing was decided and the suggestion is still open.
        showBandMessage(result.error, true);
        return;
    }

    showBandMessage(i18n("The collective note was written to %1.", result.file), false);
    reload();
}

void ProposalWindow::defer(qint64 id)
{
    if (!m_store->setProposalStatus(id, Proposal::Status::Deferred)) {
        showBandMessage(i18n("The suggestion could not be put aside: %1", m_store->lastError()), true);
        return;
    }
    reload();
}

void ProposalWindow::discard(qint64 id)
{
    // Only the suggestion (SPEC 9): removeProposal() deletes the row in
    // `proposals` and the references in `proposal_notes` that hang off it by
    // ON DELETE CASCADE. No note is named in it and none is touched.
    if (!m_store->removeProposal(id)) {
        showBandMessage(i18n("The suggestion could not be discarded: %1", m_store->lastError()), true);
        return;
    }
    reload();
}

void ProposalWindow::showBandMessage(const QString &text, bool isError)
{
    m_message->setMessageType(isError ? KMessageWidget::Error : KMessageWidget::Information);
    m_message->setText(text);
    if (!m_message->isVisible()) {
        m_message->animatedShow();
    }
}
