#include "settings/settingsdialog.h"

#include "settings/aiproviderpage.h"
#include "settings/analysispage.h"
#include "settings/exportpage.h"
#include "settings/settings.h"
#include "settings/voicenotespage.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

namespace
{
/** The name the one dialog per session is found again under. */
QString dialogName()
{
    return QStringLiteral("settings");
}

KConfigGroup windowGroup()
{
    return KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Settings"));
}

/** Wide enough for the page list beside a form, high enough for five pages. */
constexpr int WindowWidth = 640;
constexpr int WindowHeight = 480;
}

void SettingsDialog::showSettings()
{
    if (KConfigDialog::showDialog(dialogName())) {
        return;
    }
    // It deletes itself when it is closed (Qt::WA_DeleteOnClose below), and
    // KConfigDialog takes its name out of the list of open dialogs in its
    // destructor — so the next call builds a fresh one.
    (new SettingsDialog)->show();
}

SettingsDialog::SettingsDialog()
    : KConfigDialog(nullptr, dialogName(), Settings::self())
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFaceType(KPageDialog::List);

    // One line per page — see the class comment for why that is a rule here.
    addPage(new AiProviderPage(this),
            i18n("AI provider"),
            QStringLiteral("preferences-system-network-server"));
    addPage(new AnalysisPage(this), i18n("Analysis"), QStringLiteral("preferences-system-time"));
    addPage(new ExportPage(this), i18n("Export"), QStringLiteral("document-export"));
    addPage(new VoiceNotesPage(this),
            i18n("Voice notes"),
            QStringLiteral("audio-input-microphone"));

    // Hidden, not removed: the button leads to KHelpCenter on help:/denkzettel,
    // and there is no handbook — the user would read "The requested help file
    // could not be found". Replacing the button set instead would silently
    // unhook Apply, see the class comment.
    button(QDialogButtonBox::Help)->hide();

    resize(WindowWidth, WindowHeight);
    // windowHandle() only exists once the window has a platform resource, and
    // the stored size has to be in before the first show (librarywindow.cpp).
    create();
    KWindowConfig::restoreWindowSize(windowHandle(), windowGroup());
    resize(windowHandle()->size());
}

void SettingsDialog::done(int result)
{
    KConfigGroup group = windowGroup();
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.sync();

    KConfigDialog::done(result);
}
