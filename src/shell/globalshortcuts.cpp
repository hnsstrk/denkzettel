#include "shell/globalshortcuts.h"

#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#include <KLocalizedString>
#include <KNotification>

#include <QAction>
#include <QGuiApplication>
#include <QKeySequence>
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
    const QString component =
        info.componentFriendlyName().isEmpty() ? info.componentUniqueName() : info.componentFriendlyName();
    if (info.friendlyName().isEmpty()) {
        return component;
    }
    return i18nc("Kürzel-Besitzer: Anwendung und ihre Aktion", "%1 — %2", component, info.friendlyName());
}
}

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent)
    , m_captureAction(new QAction(i18n("Capture öffnen"), this))
{
    // The object name identifies the action across restarts and must not change
    // once it is registered; the component decides where the shortcut shows up
    // in the Plasma settings.
    m_captureAction->setObjectName(QStringLiteral("show_capture"));
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
    // and only stores ours on the very first registration (SPEC 2.4).
    KGlobalAccel::setGlobalShortcut(m_captureAction, sequence);

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
                              "womöglich nicht. Das Kürzel lässt sich in den Systemeinstellungen "
                              "unter „Kurzbefehle“ ändern.",
                              descriptions.join(QStringLiteral(", "))));
}
