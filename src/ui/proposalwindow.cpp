#include "ui/proposalwindow.h"

#include "analysis/suggester.h"
#include "proposals/bundleexport.h"
#include "proposals/taskexport.h"
#include "store/store.h"
#include "ui/timestampformat.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KSharedConfig>
#include <KStandardShortcut>

#include <QAction>
#include <QFontDatabase>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocale>
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

/**
 * How many lines of the note the annotation preview shows before it scrolls.
 *
 * Three, because a note is a thought written down in passing and three lines
 * are what one of them usually is; a transcript can be many more, and the card
 * must not grow to the length of the longest note the library happens to hold.
 */
constexpr int AnnotationRows = 3;

/**
 * The five fields of SPEC 7.2, in the order wireframe 1c puts them.
 *
 * One word does two jobs on purpose: it is the key of the payload object and
 * the role in the object name of the field widget (`description-<id>`), so a
 * field the card shows and a field the export reads cannot be two different
 * things.
 */
constexpr QLatin1StringView DescriptionRole("description");
constexpr QLatin1StringView ProjectRole("project");
constexpr QLatin1StringView TagsRole("tags");
constexpr QLatin1StringView DueRole("due");
constexpr QLatin1StringView PriorityRole("priority");

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
        Card card;
        card.proposal = proposal;
        for (const qint64 noteId : proposal.noteIds) {
            const std::optional<Note> note = m_store->note(noteId);
            if (note.has_value()) {
                card.notes.append(*note);
            }
        }

        // **A suggestion whose notes are all gone does not stand here, and it
        // does not stay in the database either** (SPEC 9, PO decision
        // 04.09.2026). Deleting a note takes its `proposal_notes` row with it
        // but not the suggestion, so a proposal without notes is a state the
        // store can reach, and there is no question left to put: nothing to
        // export, nothing to keep, nothing the three buttons could mean. A card
        // explaining its own emptiness would ask the user to answer for a row
        // in a table. So the review carries the deletion out that the note's
        // own deletion could not reach, with the same call "Discard" uses —
        // the notes are untouched by it, because there are none.
        // This stands **before** the status is looked at, and that is the whole
        // of issue #147: a deferred bundle whose notes are gone is exactly as
        // unanswerable as an open one — "Later" would put it aside a second
        // time, "Accept" would export nothing — and nothing else can reach it
        // afterwards. The analysis run clears a deferred suggestion only where
        // a new bundle shares a note with it (Suggester::run()), which a
        // bundle without notes can never do again. Skipped by status first, it
        // would stay in the database for good, against the sentence SPEC 9
        // makes without a word about status.
        //
        // **And it holds for a task suggestion too, which issue #31 made
        // askable.** SPEC 9 writes the sentence for a bundle because a task
        // card did not exist when it was written; nothing in the mechanism is a
        // bundle's. Neither `proposal_notes` nor `Store::proposals()` knows the
        // kind — the table carries `ON DELETE CASCADE` on the note and the
        // query filters on nothing — so a task suggestion reaches the identical
        // state, and for it "the last note" is the only note it ever had.
        if (card.notes.isEmpty()) {
            m_store->removeProposal(proposal.id);
            continue;
        }

        // Put aside is answered: it carries no card until the next analysis run
        // takes its notes back into the corpus (SPEC 7.3, SPEC 9).
        if (proposal.status != Proposal::Status::Open) {
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
    const bool isTask = card.proposal.kind == Proposal::Kind::Task;

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
    card.frame = frame;

    auto *layout = new QVBoxLayout(frame);

    // The head row of wireframe 1c: what the suggestion is called on the left,
    // and where accepting sends it on the right.
    auto *head = new QHBoxLayout();
    auto *title = new QLabel(frame);
    title->setObjectName(QStringLiteral("title-%1").arg(id));
    QFont bold = title->font();
    bold.setBold(true);
    title->setFont(bold);
    title->setWordWrap(true);
    if (!isTask) {
        title->setText(i18nc("@title, the name of a bundle suggestion",
                             "Bundle: %1",
                             payload.value(QLatin1String("title")).toString()));
    }
    head->addWidget(title, 1);
    auto *target = new QLabel(isTask
                                  ? i18nc("@info, where an accepted task is written", "→ Taskwarrior")
                                  : i18nc("@info, where an accepted bundle is written", "→ Obsidian _INBOX"),
                              frame);
    target->setForegroundRole(QPalette::PlaceholderText);
    head->addWidget(target);
    layout->addLayout(head);

    if (isTask) {
        buildTaskBody(card, layout);
    } else {
        buildBundleBody(card, layout);
    }

    // The card's own error row (SPEC 8.2): hidden it takes no height, the make
    // the band under the window title uses for itself.
    card.error = new KMessageWidget(frame);
    card.error->setObjectName(QStringLiteral("error-%1").arg(id));
    card.error->setMessageType(KMessageWidget::Error);
    card.error->setCloseButtonVisible(false);
    card.error->setWordWrap(true);
    card.error->hide();
    layout->addWidget(card.error);

    auto *line = new QFrame(frame);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    auto *actions = new QHBoxLayout();
    auto *acceptButton = new QPushButton(QIcon::fromTheme(QStringLiteral("document-export")),
                                         i18nc("@action:button", "Accept"), frame);
    acceptButton->setObjectName(QStringLiteral("accept-%1").arg(id));
    acceptButton->setToolTip(
        isTask ? i18nc("@info:tooltip", "Create the task and take the note out of the library")
               : i18nc("@info:tooltip", "Write the collective note and take the notes out of the library"));
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

    if (isTask) {
        // The description is three things at once and they have to stay one:
        // the name of the card, the answer to whether the annotation would say
        // anything the task does not already say, and — empty — the reason
        // Taskwarrior would refuse the add. So all three follow the field.
        //
        // The note text is read once here rather than at every keystroke: a
        // task suggestion stands for exactly one note, and that note is not
        // going to change while the card is open.
        const QLineEdit *description = taskField(card, DescriptionRole);
        const QString noteText = card.notes.constFirst().content.trimmed();
        QTextBrowser *preview = card.preview;
        auto follow = [title, description, preview, acceptButton, noteText] {
            const QString text = description->text().trimmed();
            title->setText(i18nc("@title, the name of a task suggestion", "Task: %1", text));
            // What exportTaskProposal() decides, said out loud: a note that
            // says no more than the description is not attached a second time.
            // Written from the same comparison, so the card cannot promise an
            // annotation the export then leaves out.
            preview->setPlainText(noteText.isEmpty() || noteText == text
                                      ? i18n("The note says no more than the description; nothing is annotated.")
                                      : noteText);
            // Taskwarrior refuses an add without a description, and
            // taskAddArguments() does not even build one (SPEC 5.1).
            acceptButton->setEnabled(!text.isEmpty());
        };
        follow();
        connect(description, &QLineEdit::textChanged, this, follow);
        return;
    }

    // Connected after the rows are in: setCheckState() while filling would fire
    // this for every one of them, against a card the window does not hold yet.
    connect(card.noteList, &QListWidget::itemChanged, this, [this, id] {
        selectionChanged(id);
    });

    // The preview of the full bundle, written by the same bundleMarkdown() a
    // deselection writes it with — so the text on the card comes from one place
    // whether anything was taken out or not, and the stored payload is not a
    // second reading of the same bundle.
    card.preview->setPlainText(
        bundleMarkdown(payload.value(QLatin1String("title")).toString(), card.notes));
}

void ProposalWindow::buildBundleBody(Card &card, QVBoxLayout *layout)
{
    const qint64 id = card.proposal.id;

    // The note list and the preview side by side. An item view rather than a
    // column of QCheckBox: the view elides a long first line by itself and
    // scrolls once the bundle is longer than the card, both of which a row of
    // check boxes would need code for.
    auto *body = new QHBoxLayout();

    auto *noteList = new QListWidget(card.frame);
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

    auto *preview = new QTextBrowser(card.frame);
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

    // The height of both halves, in rows: a card of three notes is three rows
    // tall, a longer bundle scrolls inside the card.
    if (noteList->count() > 0) {
        const int rows = qMin(noteList->count(), VisibleNoteRows);
        const int height = rows * noteList->sizeHintForRow(0) + 2 * noteList->frameWidth();
        noteList->setFixedHeight(height);
        preview->setFixedHeight(height);
    }

    card.noteList = noteList;
    card.preview = preview;
}

void ProposalWindow::buildTaskBody(Card &card, QVBoxLayout *layout)
{
    const qint64 id = card.proposal.id;
    const QJsonObject payload = QJsonDocument::fromJson(card.proposal.payload.toUtf8()).object();

    const auto addField = [&card, id](QFormLayout *form, QLatin1StringView role, const QString &label) {
        auto *field = new QLineEdit(card.frame);
        field->setObjectName(QStringLiteral("%1-%2").arg(QString(role)).arg(id));
        form->addRow(label, field);
        return field;
    };

    auto *form = new QFormLayout();
    addField(form, DescriptionRole, i18nc("@label:textbox", "Description:"))
        ->setText(payload.value(DescriptionRole).toString());
    addField(form, ProjectRole, i18nc("@label:textbox", "Project:"))
        ->setText(payload.value(ProjectRole).toString());

    // The tags with the `+` Taskwarrior writes them with (wireframe 1c) — the
    // card is where its syntax is met, the way the category column of SPEC 9 is
    // where the search language is met. taskPayload() takes the sign off again.
    QStringList tags;
    const QJsonArray stored = payload.value(TagsRole).toArray();
    for (const auto &value : stored) {
        const QString tag = value.toString().trimmed();
        if (!tag.isEmpty()) {
            tags.append(QLatin1Char('+') + tag);
        }
    }
    QLineEdit *tagField = addField(form, TagsRole, i18nc("@label:textbox", "Tags:"));
    tagField->setText(tags.join(QLatin1Char(' ')));
    // A blank separates two tags and cannot stand inside one — a tag with a
    // blank is none for Taskwarrior, and it would fall into the description
    // without a word (see taskAddArguments()).
    tagField->setPlaceholderText(i18nc("@info:placeholder", "+tag1 +tag2"));

    addField(form, DueRole, i18nc("@label:textbox, the due date of a task", "Due:"))
        ->setText(payload.value(DueRole).toString());
    QLineEdit *priorityField = addField(form, PriorityRole, i18nc("@label:textbox", "Priority:"));
    priorityField->setText(payload.value(PriorityRole).toString());
    priorityField->setPlaceholderText(i18nc("@info:placeholder, the three priorities Taskwarrior knows", "H, M or L"));

    layout->addLayout(form);

    // The annotation preview of wireframe 1c, with the note's own timestamp
    // beside the heading: what would be attached is the note, and which note
    // that is stands nowhere else on the card.
    const Note &note = card.notes.constFirst();
    auto *heading = new QLabel(i18nc("@label, the note text a task carries as an annotation",
                                     "Annotation (%1):",
                                     library::entryTimestamp(note.createdAt, QLocale())),
                               card.frame);
    heading->setForegroundRole(QPalette::PlaceholderText);
    layout->addWidget(heading);

    // A text browser and not a wrapping label, for the reason the bundle card
    // has one: a transcript can be long, and a label would grow the card to
    // whatever the longest note happens to be. Bounded here and scrolled
    // inside, the card keeps its height whatever it holds. The text is written
    // by the description field's follow-up in buildCard(), which is the one
    // place that knows whether an annotation happens at all.
    auto *preview = new QTextBrowser(card.frame);
    preview->setObjectName(QStringLiteral("preview-%1").arg(id));
    preview->setFixedHeight(AnnotationRows * preview->fontMetrics().lineSpacing()
                            + 2 * preview->frameWidth());
    layout->addWidget(preview);

    card.preview = preview;
}

QLineEdit *ProposalWindow::taskField(const Card &card, QLatin1StringView role) const
{
    return card.frame->findChild<QLineEdit *>(QStringLiteral("%1-%2").arg(QString(role)).arg(card.proposal.id));
}

QString ProposalWindow::taskPayload(const Card &card) const
{
    const auto text = [this, &card](QLatin1StringView role) {
        const QLineEdit *field = taskField(card, role);
        return field ? field->text().trimmed() : QString();
    };

    QJsonArray tags;
    // simplified() first, so a tab or a run of blanks separates as one blank
    // does; taskAddArguments() drops a tag with whitespace in it, and this is
    // what keeps the field from producing one at all.
    const QStringList words = text(TagsRole).simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString &word : words) {
        // The `+` the field shows is Taskwarrior's own mark for a tag, not part
        // of it; taskAddArguments() puts it back on. Left standing it would
        // arrive as `++anruf`.
        const QString tag = word.startsWith(QLatin1Char('+')) ? word.mid(1) : word;
        if (!tag.isEmpty()) {
            tags.append(tag);
        }
    }

    const QJsonObject fields{{DescriptionRole, text(DescriptionRole)},
                             {ProjectRole, text(ProjectRole)},
                             {TagsRole, tags},
                             {DueRole, text(DueRole)},
                             {PriorityRole, text(PriorityRole)}};
    return QString::fromUtf8(QJsonDocument(fields).toJson(QJsonDocument::Compact));
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

    if (card->proposal.kind == Proposal::Kind::Task) {
        acceptTask(*card);
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

void ProposalWindow::acceptTask(const Card &card)
{
    // The note the card stands for, not the id list of the suggestion: a note
    // that has been deleted since the run leaves `proposal.noteIds` holding an
    // id nothing answers, and removeExportedBundle() would then refuse the
    // whole call. reload() has already dropped a card with no notes left, so
    // what stands here is what is really there.
    const Note &note = card.notes.constFirst();

    const TaskExportResult result = exportTaskProposal(taskPayload(card), note.content);
    if (!result.ok()) {
        // Nothing is cleared up (SPEC 8.2): the note stays, the suggestion
        // stays open, and the user decides what to do with the card.
        //
        // The two ends are told apart, and the second one is why `uuid` exists
        // beside `error` at all: with a UUID in hand the task **is** in
        // Taskwarrior and only the note text is missing, so a second press
        // would create it twice. The card has to say so, or the user reads the
        // red line as "nothing happened".
        showCardError(card,
                      result.uuid.isEmpty()
                          ? result.error
                          : i18n("The task was created, but the note text was not attached to it: %1",
                                 result.error));
        return;
    }

    // The note and the suggestion in one transaction, through the same call an
    // exported bundle takes (SPEC 8.2, "as in 8.1") — one road out of the
    // corpus and not two.
    if (!m_store->removeExportedBundle({note.id}, card.proposal.id)) {
        showCardError(card,
                      i18n("The task was created, but the note could not be deleted: %1", m_store->lastError()));
        return;
    }

    showBandMessage(i18n("The task was created in Taskwarrior."), false);
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

void ProposalWindow::showCardError(const Card &card, const QString &text)
{
    // The text goes even when the row stays hidden: a label that keeps its
    // last sentence answers the same thing for "nothing to report" and for
    // "reported and wrongly hidden" (CLAUDE.md, finding 79).
    card.error->setText(text);
    if (text.isEmpty()) {
        card.error->hide();
        return;
    }
    if (!card.error->isVisible()) {
        card.error->animatedShow();
    }
}
