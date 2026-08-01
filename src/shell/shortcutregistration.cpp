#include "shell/shortcutregistration.h"

#include <KLocalizedString>

ShortcutRegistration shortcutRegistration(const QList<QKeySequence> &storedByTheDaemon, bool desktopFileFound)
{
    // Which sequence the daemon holds is not our business: the user may have
    // changed it in the Plasma settings, and that is a working registration.
    if (!storedByTheDaemon.isEmpty()) {
        return ShortcutRegistration::Reached;
    }

    return desktopFileFound ? ShortcutRegistration::DaemonKeptNothing
                            : ShortcutRegistration::ApplicationNotInstalled;
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
        return i18n("Meta+N ist beim Kurzbefehl-Dienst nicht angekommen — er hat die Registrierung "
                    "nicht behalten. Läuft kglobalacceld? Das Capture-Fenster bleibt über das "
                    "Symbol im Systemabschnitt erreichbar.");
    }

    return {};
}
