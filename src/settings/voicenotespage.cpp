#include "settings/voicenotespage.h"

#include "transcribe/transcriber.h"

#include <KColorScheme>
#include <KLocalizedString>

#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace
{
/**
 * The small coloured line of the wireframe (1d:172–173), directly under the
 * row it belongs to.
 *
 * KColorScheme and not the palette: NeutralText and NegativeText are two roles
 * QPalette does not have. Written out a second time beside AiProviderPage
 * rather than lifted into something shared — three further stories are
 * building into this dialog at the same time, and every page is a file of its
 * own so that they do not meet as merge conflicts.
 */
void showLine(QLabel *line, const QString &text, KColorScheme::ForegroundRole role)
{
    const KColorScheme scheme(QPalette::Normal, KColorScheme::View);
    QPalette colours = line->palette();
    colours.setColor(QPalette::WindowText, scheme.foreground(role).color());
    line->setPalette(colours);
    line->setText(text);
}

/** Both message lines carry the smaller type of the wireframe. */
void makeSmall(QWidget *page, QLabel *line)
{
    const QFont &bodyFont = page->font();
    if (bodyFont.pointSizeF() > 0) {
        QFont small = bodyFont;
        small.setPointSizeF(bodyFont.pointSizeF() * 0.9);
        line->setFont(small);
    }
    line->setWordWrap(true);
}
}

VoiceNotesPage::VoiceNotesPage(QWidget *parent)
    : QWidget(parent)
    , m_size(new QComboBox(this))
    , m_program(new QLineEdit(this))
    , m_stored(new QLineEdit(this))
    , m_modelState(new QLabel(this))
    , m_programState(new QLabel(this))
{
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    layout->addLayout(form);

    // All five sizes, and what is not on disk is greyed rather than left out
    // (UX, 29.08.2026): leaving it out would keep the range of the program a
    // secret, offering it would send the queue after a file that is not there.
    //
    // ponytail: the state is one exists() look per size, taken while the page
    // is built. Ceiling — nothing fetches a model yet, so nothing can change
    // under an open dialog; upgrade path is issue #23, which brings the
    // download and refreshes the entry it has just filled.
    m_size->setObjectName(QStringLiteral("kcfg_ModelSize"));
    // What KConfigDialogManager stores from a combo box is its `currentIndex`
    // — its own table says so and no line here has to. What that table also
    // says is that a QComboBox has changed when it emits `activated(int)`, and
    // that one only a hand on the popup emits: measured 29.08.2026, a size set
    // from anywhere else moves the box and leaves Apply grey, with nothing
    // saying so. The property below is the documented way past it
    // (kconfigdialogmanager.h:101–118).
    m_size->setProperty("kcfg_propertyNotify", QByteArray(SIGNAL(currentIndexChanged(int))));
    auto *entries = qobject_cast<QStandardItemModel *>(m_size->model());
    for (int index = 0; index < static_cast<int>(whisper::Sizes.size()); ++index) {
        const QString size(whisper::Sizes.at(index));
        const bool downloaded = QFileInfo::exists(Transcriber::modelPath(size));
        m_size->addItem(downloaded ? size
                                   : i18nc("@item:inlistbox a whisper.cpp model that is not on disk",
                                           "%1 — not downloaded",
                                           size));
        if (!downloaded && entries != nullptr) {
            entries->item(index)->setEnabled(false);
        }
    }
    form->addRow(i18n("Model size:"), m_size);
    makeSmall(this, m_modelState);
    form->addRow(m_modelState);

    // The hidden field is what KConfigDialogManager loads, stores and greys
    // the Apply button from; the visible one beside it is only the editor.
    // See the class comment for why the two are not one.
    m_stored->setObjectName(QStringLiteral("kcfg_WhisperProgram"));
    m_stored->setVisible(false);

    m_program->setObjectName(QStringLiteral("whisperProgram"));
    auto *chooser = new QHBoxLayout;
    chooser->addWidget(m_program);
    auto *browse = new QPushButton(i18n("Browse…"), this);
    chooser->addWidget(browse);
    // No KUrlRequester: it lies in KF6::KIOWidgets, which would be a new
    // dependency and with it a new `depends` line in the PKGBUILD — for one
    // file chooser (project lead, 29.08.2026).
    form->addRow(i18n("whisper-cli program:"), chooser);
    m_programState->setObjectName(QStringLiteral("whisperProgramState"));
    makeSmall(this, m_programState);
    form->addRow(m_programState);

    // Nothing on these pages wants to grow (UX, 29.08.2026), so the room a
    // resized window brings goes underneath the form.
    layout->addStretch();

    connect(m_size, &QComboBox::currentIndexChanged, this, &VoiceNotesPage::showModelState);
    connect(m_program, &QLineEdit::textChanged, this, &VoiceNotesPage::takeProgram);
    // The other direction, and it is what fills the editor at all: the manager
    // writes the setting into the hidden field when the dialog opens, on
    // Cancel and on "Restore defaults". setText() is quiet when the text is
    // already what it is being set to, so the two do not chase each other.
    connect(m_stored, &QLineEdit::textChanged, m_program, &QLineEdit::setText);
    connect(browse, &QPushButton::clicked, this, &VoiceNotesPage::browseForProgram);

    showModelState();
}

void VoiceNotesPage::browseForProgram()
{
    const QString chosen =
        QFileDialog::getOpenFileName(this,
                                     i18nc("@title:window", "Choose the whisper-cli program"),
                                     QFileInfo(m_program->text()).absolutePath());
    if (!chosen.isEmpty()) {
        // Through the editor and not past it: a program picked out of the
        // chooser is checked like a typed one — the dialog hands out
        // directories and unreadable files just the same.
        m_program->setText(chosen);
    }
}

void VoiceNotesPage::takeProgram(const QString &path)
{
    // isFile() beside isExecutable(): a directory is executable too, and a
    // directory in this field is exactly the value that would go through
    // unremarked and only be noticed at the next recording.
    const QFileInfo program(path);
    if (program.isFile() && program.isExecutable()) {
        m_stored->setText(path);
        showLine(m_programState, QString(), KColorScheme::NormalText);
        return;
    }
    showLine(m_programState,
             path.isEmpty() ? i18n("No program is set for the transcription")
                            : i18n("%1 is not an executable program",
                                   QDir::toNativeSeparators(path)),
             KColorScheme::NegativeText);
}

void VoiceNotesPage::showModelState()
{
    const int index = m_size->currentIndex();
    if (index < 0) {
        return;
    }
    // Only the lack is reported; a model that lies there is the ordinary case
    // and says nothing (UX, 29.08.2026). What the line carries is the place
    // the file is expected in, because that is the one thing the user can act
    // on until the download of issue #23 exists.
    const QString path = Transcriber::modelPath(QString(whisper::Sizes.at(index)));
    showLine(m_modelState,
             QFileInfo::exists(path) ? QString() : i18n("Expected at %1", QDir::toNativeSeparators(path)),
             KColorScheme::NeutralText);
}
