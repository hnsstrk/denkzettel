#include "settings/settingsdialog.h"

#include "settings/aiproviderpage.h"
#include "settings/analysispage.h"
#include "settings/capturepage.h"
#include "settings/exportpage.h"
#include "settings/settings.h"
#include "settings/shortcutspage.h"
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

void SettingsDialog::showSettings(GlobalShortcuts *shortcuts, ModelDownload *download)
{
    if (KConfigDialog::showDialog(dialogName())) {
        return;
    }
    // It deletes itself when it is closed (Qt::WA_DeleteOnClose below), and
    // KConfigDialog takes its name out of the list of open dialogs in its
    // destructor — so the next call builds a fresh one.
    (new SettingsDialog(shortcuts, download))->show();
}

SettingsDialog::SettingsDialog(GlobalShortcuts *shortcuts, ModelDownload *download)
    : KConfigDialog(nullptr, dialogName(), Settings::self())
    , m_shortcutsPage(new ShortcutsPage(shortcuts, this))
    , m_aiPage(new AiProviderPage(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFaceType(KPageDialog::List);

    // One line per page — see the class comment for why that is a rule here.
    // "Capture" stands first, and SPEC 13 says why: it carries the privacy
    // switch of issue #47, and findability is that switch's purpose. The icon
    // is `document-edit`, the same one the tray gives „Notiz erfassen", because
    // it is the same action.
    addPage(new CapturePage(this), i18n("Capture"), QStringLiteral("document-edit"));
    addPage(m_aiPage,
            i18n("AI provider"),
            QStringLiteral("preferences-system-network-server"));
    addPage(new AnalysisPage(this), i18n("Analysis"), QStringLiteral("preferences-system-time"));
    addPage(new ExportPage(this), i18n("Export"), QStringLiteral("document-export"));
    addPage(new VoiceNotesPage(download, this),
            i18n("Voice notes"),
            QStringLiteral("audio-input-microphone"));
    addPage(m_shortcutsPage, i18n("Shortcuts"), QStringLiteral("preferences-desktop-keyboard-shortcut"));

    // The page carries no `kcfg_` widget, so nothing tells the dialog that
    // something changed — this does, and the five overrides below do the rest.
    connect(m_shortcutsPage, &ShortcutsPage::changed, this, &SettingsDialog::updateButtons);
    // And the API key field on the AI page, for the same reason: it has no
    // `kcfg_` name, so nothing else would ever light the Apply button for it.
    connect(m_aiPage, &AiProviderPage::changed, this, &SettingsDialog::updateButtons);

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

void SettingsDialog::updateSettings()
{
    m_shortcutsPage->save();
    m_aiPage->save();
}

void SettingsDialog::updateWidgets()
{
    m_shortcutsPage->load();
}

void SettingsDialog::updateWidgetsDefault()
{
    m_shortcutsPage->loadDefaults();
}

bool SettingsDialog::hasChanged()
{
    return m_shortcutsPage->hasChanged() || m_aiPage->hasChanged();
}

bool SettingsDialog::isDefault()
{
    return m_shortcutsPage->isDefault();
}

void SettingsDialog::done(int result)
{
    if (result == QDialog::Accepted) {
        // **The save has to happen here, not in updateSettings().** OK reaches
        // accept() through the button box, and the box handles the button
        // before the connections KConfigDialog makes to the button's own
        // clicked() — so updateSettings() runs *after* this. Asked here for a
        // result of its own, the failure flag would answer with the state of
        // the *previous* save: measured 29.08.2026, finished(1) fired while the
        // service still held the sequence of the run before, and a failed
        // readback closed the window with its message on it.
        //
        // The updateSettings() behind this one then finds every field on the
        // value the service holds and writes nothing, which is why save() has
        // to leave the report standing when it wrote nothing (shortcutspage.cpp).
        m_shortcutsPage->save();
        // The key goes with it on the OK road, which never reaches
        // updateSettings() when the shortcut page holds the window back below.
        m_aiPage->save();
        if (m_shortcutsPage->takeReadbackFailure()) {
            // The window stays where it is — otherwise the message SPEC 2.4
            // asks for would flash past with the closing dialog. Once, see
            // takeReadbackFailure().
            return;
        }
    }

    KConfigGroup group = windowGroup();
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.sync();

    KConfigDialog::done(result);
}
