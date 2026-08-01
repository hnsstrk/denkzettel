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

bool desktopFileDeclaresAction(const QString &desktopFilePath, const QString &actionId)
{
    if (desktopFilePath.isEmpty() || !QFileInfo::exists(desktopFilePath)) {
        return false;
    }

    // Both halves are needed: kglobalacceld reads the list under `Actions=`, and
    // the group is where the Exec line it starts lives.
    KDesktopFile file(desktopFilePath);
    return file.readActions().contains(actionId) && file.hasActionGroup(actionId);
}

QString shortcutRegistrationFailure(ShortcutRegistration registration)
{
    switch (registration) {
    case ShortcutRegistration::Reached:
        return {};
    case ShortcutRegistration::ApplicationNotInstalled:
        return i18n("Meta+N ist beim Kurzbefehl-Dienst nicht angekommen: Denkzettel ist nicht "
                    "systemweit installiert, und ohne seine Desktop-Datei legt der Dienst keinen "
                    "Eintrag an. Nach der Installation steht das Kürzel zur Verfügung; bis dahin "
                    "bleibt das Capture-Fenster über das Symbol im Systemabschnitt erreichbar.");
    case ShortcutRegistration::DaemonKeptNothing:
        // No process name and no question: whoever does not know what
        // kglobalacceld is knows no more after reading it. What is left is a
        // step that can be carried out and checked (KDE HIG, UI review B9).
        return i18n("Meta+N ist beim Kurzbefehl-Dienst nicht angekommen — er hat die Registrierung "
                    "nicht behalten. Die Kurzbefehle lassen sich in den Systemeinstellungen unter "
                    "„Kurzbefehle“ prüfen; hilft das nicht, bringt eine neue Anmeldung den Dienst "
                    "zurück. Das Capture-Fenster bleibt über das Symbol im Systemabschnitt "
                    "erreichbar.");
    case ShortcutRegistration::DesktopActionMissing:
        return i18n("Meta+N ist eingerichtet, löst aber nichts aus: In der Desktop-Datei von "
                    "Denkzettel fehlt der Eintrag zu diesem Kürzel. Eine vollständige "
                    "Neuinstallation bringt ihn zurück. Bis dahin bleibt das Capture-Fenster "
                    "über das Symbol im Systemabschnitt erreichbar.");
    }

    return {};
}
