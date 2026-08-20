#include "shell/shortcutregistration.h"

#include <KDesktopFile>
#include <KLocalizedString>

#include <QFileInfo>

ShortcutRegistration shortcutRegistration(const QList<QKeySequence> &storedByTheDaemon,
                                          bool desktopFileFound,
                                          bool desktopFileDeclaresTheAction)
{
    if (storedByTheDaemon.isEmpty()) {
        return desktopFileFound ? ShortcutRegistration::DaemonKeptNothing
                                : ShortcutRegistration::ApplicationNotInstalled;
    }

    // Which sequence the daemon holds is not our business: the user may have
    // changed it in the Plasma settings, and that is a working registration.
    // Where the key press goes, however, is: with a desktop file the daemon
    // starts the desktop action instead of signalling us.
    if (desktopFileFound && !desktopFileDeclaresTheAction) {
        return ShortcutRegistration::DesktopActionMissing;
    }

    return ShortcutRegistration::Reached;
}

// Healing this means changing the signature or introducing a type of its own,
// which is design rather than tidying up (issue #76). The one case a mix-up
// would be visible in - placeholderPage() in the empty library - gets a test
// assurance instead, as issue #88.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool desktopFileDeclaresAction(const QString &desktopFilePath, const QString &actionId)
{
    if (desktopFilePath.isEmpty() || !QFileInfo::exists(desktopFilePath)) {
        return false;
    }

    // Both halves are needed: kglobalacceld reads the list under `Actions=`, and
    // the group is where the Exec line it starts lives.
    const KDesktopFile file(desktopFilePath);
    return file.readActions().contains(actionId) && file.hasActionGroup(actionId);
}

QString shortcutRegistrationFailure(ShortcutRegistration registration)
{
    switch (registration) {
    case ShortcutRegistration::Reached:
        return {};
    case ShortcutRegistration::ApplicationNotInstalled:
        return i18n("Meta+N never reached the shortcut service: Denkzettel is not installed "
                    "system-wide, and without its desktop file the service creates no entry. "
                    "After the installation the shortcut works; until then the capture window "
                    "stays reachable through the icon in the system tray.");
    case ShortcutRegistration::DaemonKeptNothing:
        // No process name and no question: whoever does not know what
        // kglobalacceld is knows no more after reading it. What is left is a
        // step that can be carried out and checked (KDE HIG, UI review B9).
        return i18n("Meta+N never reached the shortcut service — it did not keep the "
                    "registration. The shortcuts can be checked in the system settings under "
                    "“Shortcuts”; if that does not help, logging in anew brings the service back. "
                    "The capture window stays reachable through the icon in the system tray.");
    case ShortcutRegistration::DesktopActionMissing:
        return i18n("Meta+N is set up but triggers nothing: the desktop file of Denkzettel "
                    "is missing the entry for this shortcut. A complete reinstallation brings it "
                    "back. Until then the capture window stays reachable through the icon in the "
                    "system tray.");
    }

    return {};
}
