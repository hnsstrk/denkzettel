#include "shell/globalshortcuts.h"

#include "shell/shortcutregistration.h"

#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#include <KLocalizedString>
#include <KNotification>

#include <QAction>
#include <QGuiApplication>
#include <QKeySequence>
#include <QStandardPaths>
#include <QStringList>

namespace
{
/**
 * kglobalacceld names an application component after its desktop file,
 * suffix included — the components registered on a running Plasma read
 * `org.kde.spectacle.desktop`. Deriving it from the application keeps the
 * name that main() sets from being spelled out twice.
 */
QString componentName()
{
    return QGuiApplication::desktopFileName() + QStringLiteral(".desktop");
}

QString ownerDescription(const KGlobalShortcutInfo &info)
{
    QString component =
        info.componentFriendlyName().isEmpty() ? info.componentUniqueName() : info.componentFriendlyName();
    if (info.friendlyName().isEmpty()) {
        return component;
    }
    return i18nc("Kürzel-Besitzer: Anwendung und ihre Aktion", "%1 — %2", component, info.friendlyName());
}

/**
 * The desktop file that names our component, or an empty string. kglobalacceld
 * resolves a component name ending in `.desktop` through KService and falls back
 * to `<data>/kglobalaccel/<name>`; finding neither, it creates no component at
 * all. Both places are looked at here, so the message can name the real cause
 * (retro Sprint 2, 9.1) — and so the file can be read for its actions.
 */
QString desktopFilePath()
{
    // No const on a local that is returned: it would stop the return from
    // moving and cost a copy on every call, and it buys nothing, because the
    // variable dies with the return anyway. The rule for the whole project is
    // written down in .clang-tidy, next to the check list (issue #76).
    // NOLINTNEXTLINE(misc-const-correctness)
    QString application = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, componentName());
    if (!application.isEmpty()) {
        return application;
    }

    return QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                  QStringLiteral("kglobalaccel/") + componentName());
}
}

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent)
    , m_captureAction(new QAction(i18n("Notiz erfassen"), this))
{
    // The object name identifies the action across restarts and must not change
    // once it is registered; the component decides where the shortcut shows up
    // in the Plasma settings. It doubles as the name of the desktop action that
    // kglobalacceld starts on the key press (SPEC 2.4), so it has to be a valid
    // XDG action identifier — letters, digits and the hyphen, no underscore.
    // Being an identifier and not a label, it keeps its name while the labels
    // around it turn German (issue #60): renaming it would drop the shortcut
    // the user has set.
    m_captureAction->setObjectName(QStringLiteral("show-capture"));
    m_captureAction->setProperty("componentName", componentName());
    m_captureAction->setProperty("componentDisplayName", i18n("Denkzettel"));

    connect(m_captureAction, &QAction::triggered, this, &GlobalShortcuts::captureRequested);
}

QList<ShortcutOwner> GlobalShortcuts::registerCaptureShortcut()
{
    const QKeySequence sequence(Qt::META | Qt::Key_N);

    QList<ShortcutOwner> owners;
    const QList<KGlobalShortcutInfo> registered =
        KGlobalAccel::globalShortcutsByKey(sequence, KGlobalAccel::Equal);
    owners.reserve(registered.size());
    for (const KGlobalShortcutInfo &info : registered) {
        owners.append({info.componentUniqueName(), ownerDescription(info)});
    }

    // Autoloading restores a sequence the user changed in the Plasma settings
    // and only stores ours on the very first registration (SPEC 2.4). The
    // return value is not looked at: it cannot show a backend failure, because
    // doRegister() sends its D-Bus call and drops the answer.
    KGlobalAccel::setGlobalShortcut(m_captureAction, sequence);

    // So the daemon is asked what it actually holds, and the desktop file it
    // resolves us through is read for the action it starts on the key press.
    // Both failures are silent otherwise — the customer met each of them once,
    // on 01.08.2026 (retro B5).
    const QString desktopFile = desktopFilePath();
    const ShortcutRegistration registration =
        shortcutRegistration(KGlobalAccel::self()->globalShortcut(componentName(), m_captureAction->objectName()),
                             !desktopFile.isEmpty(),
                             desktopFileDeclaresAction(desktopFile, m_captureAction->objectName()));
    if (registration != ShortcutRegistration::Reached) {
        // Unlike a conflict this leaves no working shortcut at all, so it is
        // reported at every start rather than on the first one only.
        const QString failure = shortcutRegistrationFailure(registration);
        qWarning("%s", qPrintable(failure));
        KNotification::event(KNotification::Error, i18n("Kürzel nicht einsatzbereit"), failure);
        return {};
    }

    return foreignShortcutOwners(owners, componentName());
}

void notifyShortcutConflict(const QList<ShortcutOwner> &conflicts)
{
    QStringList descriptions;
    descriptions.reserve(conflicts.size());
    for (const ShortcutOwner &conflict : conflicts) {
        descriptions.append(conflict.description);
    }

    KNotification::event(KNotification::Warning,
                         i18n("Kürzel bereits belegt"),
                         i18n("Meta+N gehört bereits zu: %1. Der Tastendruck erreicht Denkzettel "
                              "womöglich nicht. Zum Ändern die Systemeinstellungen unter "
                              "„Kurzbefehle“ öffnen.",
                              descriptions.join(QStringLiteral(", "))));
}
