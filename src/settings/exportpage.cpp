#include "settings/exportpage.h"

#include "platform/optionaltools.h"
#include "settings/settings.h"

#include <KColorScheme>
#include <KLocalizedString>

#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
/**
 * Empty when the folder is there and can be written to, the sentence to show
 * otherwise — with a `%1` for the folder it is about.
 *
 * **The sentence names the folder because the field cannot.** A refused folder
 * is put back to the one that stands in the configuration, and with nothing
 * stored yet that would leave an empty field beside a complaint about a folder
 * nobody can see any more (review of issue #75).
 *
 * `isWritable()` asks the effective user, so for root it is true whatever the
 * mode bits say — a check of this function run as root measures nothing.
 */
KLocalizedString folderProblem(const QString &path)
{
    const QFileInfo folder(path);
    if (!folder.isDir()) {
        return ki18n("There is no folder %1.");
    }
    if (!folder.isWritable()) {
        return ki18n("%1 cannot be written to.");
    }
    return KLocalizedString();
}
}

ExportPage::ExportPage(QWidget *parent)
    : QWidget(parent)
    , m_vaultPath(new QLineEdit(this))
    , m_pathResult(new QLabel(this))
    // What stands in denkzettelrc. The manager fills the field from the same
    // place a moment later, so this is the folder the page comes up with.
    , m_accepted(Settings::self()->vaultPath())
{
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    layout->addLayout(form);

    // Field plus button and not a KUrlRequester: that one lives in
    // KF6::KIOWidgets, which nothing here links yet, and a whole dependency for
    // one folder chooser is what the packaging check of finding 33 would have
    // to carry from then on. The full export does it the same way
    // (librarywindow.cpp, startFullExport()).
    m_vaultPath->setObjectName(QStringLiteral("kcfg_VaultPath"));
    auto *browse = new QPushButton(i18n("Browse…"), this);
    auto *folder = new QHBoxLayout;
    folder->addWidget(m_vaultPath);
    folder->addWidget(browse);
    form->addRow(i18n("Vault folder:"), folder);

    // The same small coloured line as on the AI provider page, and in the field
    // column so it stands under the path it is about. Not a KMessageWidget:
    // that one would push the form down by its own height on every check.
    const QFont bodyFont = font();
    if (bodyFont.pointSizeF() > 0) {
        QFont small = bodyFont;
        small.setPointSizeF(bodyFont.pointSizeF() * 0.9);
        m_pathResult->setFont(small);
    }
    // No word wrap, unlike the result line of the AI page: that one carries
    // whatever an unreachable server says, these are three short sentences of
    // our own. And a wrapping QLabel in a form row picks its own width out of a
    // heuristic — measured on 29.08.2026 it broke "This folder can be written
    // to." after four words in a row 560 pixels wide.
    m_pathResult->setWordWrap(false);
    form->addRow(QString(), m_pathResult);

    QSpinBox *notes = threshold(form,
                                i18n("Overflow warning at:"),
                                "kcfg_OverflowNotes",
                                Settings::MaximumOverflowNotes,
                                i18n("notes not transferred yet"));
    QSpinBox *days = threshold(form,
                               i18n("or when the oldest is:"),
                               "kcfg_OverflowDays",
                               Settings::MaximumOverflowDays,
                               i18n("days old"));
    QSpinBox *bundle = threshold(form,
                                 i18n("Collective note from:"),
                                 "kcfg_BundleNotes",
                                 Settings::MaximumBundleNotes,
                                 i18n("notes on the same topic"));

    // One width for all three, so the units beside them start on a line. Taken
    // from the widest of the boxes and not written down in pixels: what a spin
    // box needs is its largest value in the user's font, and that is a
    // different number of pixels at every scaling and every font size.
    const int width =
        std::max({notes->sizeHint().width(), days->sizeHint().width(), bundle->sizeHint().width()});
    notes->setFixedWidth(width);
    days->setFixedWidth(width);
    bundle->setFixedWidth(width);

    // Taskwarrior, the second transfer of SPEC 8 (issue #17). Here and not on a
    // page of its own, because SPEC 8 puts Obsidian and Taskwarrior together as
    // the two transfers and one line does not open a sixth page (UX decision of
    // 29.08.2026). Built only when there is something to report: only the lack
    // is reported, the program being there is the ordinary case — and nothing
    // can change it while the dialog stands, so the row needs no member and no
    // later update.
    if (!tools::isRunnable(QString(tools::TaskProgram))) {
        auto *taskState =
            new QLabel(i18n("%1 is not available; nothing can be transferred to Taskwarrior",
                            QString(tools::TaskProgram)),
                       this);
        if (bodyFont.pointSizeF() > 0) {
            QFont small = bodyFont;
            small.setPointSizeF(bodyFont.pointSizeF() * 0.9);
            taskState->setFont(small);
        }
        const KColorScheme scheme(QPalette::Normal, KColorScheme::View);
        QPalette colours = taskState->palette();
        colours.setColor(QPalette::WindowText, scheme.foreground(KColorScheme::NeutralText).color());
        taskState->setPalette(colours);
        // Over both columns, unlike the path line above it: that one belongs to
        // the row it stands under and is indented with it, this one belongs to
        // the page and starts where the labels do.
        form->addRow(taskState);
    }

    // Nothing on these pages wants to grow (UX decision of 29.08.2026), so the
    // room a resized window brings goes underneath the form.
    layout->addStretch();

    connect(browse, &QPushButton::clicked, this, &ExportPage::chooseFolder);
    // editingFinished and not textChanged: the criterion of issue #75 is that a
    // folder is checked when it is SET, and a check on every keystroke would
    // call half a typed path unwritable while it is being typed.
    connect(m_vaultPath, &QLineEdit::editingFinished, this, [this] {
        setFolder(m_vaultPath->text());
    });
    // For the folder a refused one goes back to, see eventFilter().
    m_vaultPath->installEventFilter(this);
}

ExportPage::~ExportPage()
{
    // ~QWidget closes the window, the line edit loses the focus over it and
    // emits editingFinished — into an object whose ExportPage part is already
    // gone. Through the settings dialog it never gets there, because the page
    // outlives the window; shown on its own it aborts (review of issue #75,
    // gdb: assertObjectType<ExportPage> under ~ExportPage → ~QWidget →
    // QLineEdit::focusOutEvent). Whoever builds the next page of this dialog
    // takes these two lines along.
    //
    // **No check stands behind them.** Under the offscreen platform the suite
    // runs on, the focus-out never arrives: with these two lines taken out,
    // three shapes — plain delete, delete of a host window, close with
    // WA_DeleteOnClose, each with the field holding the focus of an active
    // window — all came through without a murmur (29.08.2026). A case built on
    // any of them would be green with the guard and without it.
    m_vaultPath->disconnect(this);
    m_vaultPath->removeEventFilter(this);
}

bool ExportPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        // What stands in the field the moment the user enters it — and that is
        // what a folder refused afterwards goes back to. Not a value kept from
        // the last check: the field is written from outside too, by the
        // dialog's manager when the page comes up and again when "Defaults"
        // empties it, and a remembered value would send a later refusal back to
        // a folder the user has just thrown away (review of issue #75).
        m_accepted = m_vaultPath->text();
    }
    return QWidget::eventFilter(watched, event);
}

void ExportPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // The check does not only belong on what is typed. A folder that was set
    // months ago can have been moved since, and that is the case the user meets
    // FIRST: they open the settings and a path stands there that no longer
    // exists. The acceptance criterion of issue #75 does not ask for it; the
    // review of 29.08.2026 does. By now the manager has filled the field, so
    // what is checked here is what stands in denkzettelrc.
    setFolder(m_vaultPath->text());
}

QSpinBox *ExportPage::threshold(QFormLayout *form,
                                const QString &label,
                                const char *key,
                                int maximum,
                                const QString &unit)
{
    // The unit stands beside the box and not as a suffix inside it, the way the
    // drawing has it (UX comment on issue #16): the words belong to the
    // sentence the row reads as — "or when the oldest is 30 days old" — and
    // inside the box the cursor would step over them on every change.
    auto *box = new QSpinBox(this);
    box->setObjectName(QLatin1String(key));
    box->setRange(Settings::MinimumThreshold, maximum);

    auto *row = new QHBoxLayout;
    row->addWidget(box);
    row->addWidget(new QLabel(unit, this));
    row->addStretch();
    form->addRow(label, row);
    return box;
}

void ExportPage::chooseFolder()
{
    const QString start = m_vaultPath->text().isEmpty() ? QDir::homePath() : m_vaultPath->text();
    const QString chosen =
        QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Choose the vault folder"), start);
    if (chosen.isEmpty()) {
        return;
    }
    setFolder(chosen);
}

void ExportPage::setFolder(const QString &candidate)
{
    if (candidate.isEmpty()) {
        // The field starts empty and may stay empty until the user sets it, so
        // an empty field is not a fault and gets no red line saying it is.
        m_accepted.clear();
        m_vaultPath->clear();
        m_pathResult->clear();
        return;
    }

    const KLocalizedString problem = folderProblem(candidate);
    if (problem.isEmpty()) {
        m_accepted = candidate;
    }
    // Only an accepted folder ever reaches the field. Otherwise a refused one
    // would stand on the form, and Apply would write it into denkzettelrc —
    // the check would have reported a fault and stored it in the same breath.
    // setText() emits no editingFinished, so this does not come back here.
    if (m_vaultPath->text() != m_accepted) {
        m_vaultPath->setText(m_accepted);
    }

    // KColorScheme and not the palette: PositiveText and NegativeText do not
    // exist in QPalette at all.
    const KColorScheme scheme(QPalette::Normal, KColorScheme::View);
    const KColorScheme::ForegroundRole role =
        problem.isEmpty() ? KColorScheme::PositiveText : KColorScheme::NegativeText;
    QPalette colours = m_pathResult->palette();
    colours.setColor(QPalette::WindowText, scheme.foreground(role).color());
    m_pathResult->setPalette(colours);

    if (problem.isEmpty()) {
        m_pathResult->setText(i18n("This folder can be written to."));
        return;
    }

    // The folder shortened in the middle to what is left of the row beside the
    // rest of the sentence. Without it a long path pushes a scroll bar under
    // the page and runs off its right edge, and the folder the message is about
    // is exactly the part that goes over the edge (measured 29.08.2026 with a
    // path of eleven components). ElideMiddle and not ElideRight: the last
    // components are what tells two neighbouring folders apart.
    // The room is taken from the path FIELD and not from the message label:
    // the label is empty whenever it is about to be filled, so its own width is
    // the width of nothing — measured 29.08.2026, where it cut a 24-character
    // path down to "/t…rt". The field stands in the same column and is a little
    // narrower than it, which leaves the rest of the sentence its place.
    const QFontMetrics metrics(m_pathResult->font());
    const int room = m_vaultPath->width() - metrics.horizontalAdvance(problem.subs(QString()).toString());
    m_pathResult->setText(problem.subs(metrics.elidedText(candidate, Qt::ElideMiddle, std::max(0, room))).toString());
}
