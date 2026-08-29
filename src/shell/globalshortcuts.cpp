#include "shell/globalshortcuts.h"

#include "shell/shortcutregistration.h"

#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#include <KLocalizedString>
#include <KNotification>

#include <QAction>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QStringList>

namespace
{
QString ownerDescription(const KGlobalShortcutInfo &info)
{
    QString component =
        info.componentFriendlyName().isEmpty() ? info.componentUniqueName() : info.componentFriendlyName();
    if (info.friendlyName().isEmpty()) {
        return component;
    }
    return i18nc("shortcut owner: application and its action", "%1 — %2", component, info.friendlyName());
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
    QString application =
        QStandardPaths::locate(QStandardPaths::ApplicationsLocation, GlobalShortcuts::shortcutComponent());
    if (!application.isEmpty()) {
        return application;
    }

    return QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                  QStringLiteral("kglobalaccel/") + GlobalShortcuts::shortcutComponent());
}
}

QString GlobalShortcuts::actionId(Shortcut which)
{
    switch (which) {
    case Shortcut::Capture:
        return QStringLiteral("show-capture");
    case Shortcut::Recorder:
        return QStringLiteral("show-recorder");
    }

    return {};
}

QString GlobalShortcuts::shortcutComponent()
{
    // kglobalacceld names an application component after its desktop file,
    // suffix included — the components registered on a running Plasma read
    // `org.kde.spectacle.desktop`. Deriving it from the application keeps the
    // name that main() sets from being spelled out twice.
    return QGuiApplication::desktopFileName() + QStringLiteral(".desktop");
}

QString GlobalShortcuts::label(Shortcut which)
{
    switch (which) {
    case Shortcut::Capture:
        return i18n("Capture note");
    case Shortcut::Recorder:
        return i18n("Record voice note");
    }

    return {};
}

QKeySequence GlobalShortcuts::defaultSequence(Shortcut which)
{
    switch (which) {
    case Shortcut::Capture:
        return {Qt::META | Qt::Key_N};
    case Shortcut::Recorder:
        return {Qt::META | Qt::SHIFT | Qt::Key_N};
    }

    return {};
}

QKeySequence GlobalShortcuts::assignedSequence(Shortcut which)
{
    const QList<QKeySequence> held = KGlobalAccel::self()->globalShortcut(shortcutComponent(), actionId(which));
    return held.isEmpty() ? QKeySequence() : held.constFirst();
}

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent)
    , m_captureAction(new QAction(label(Shortcut::Capture), this))
    , m_recorderAction(new QAction(label(Shortcut::Recorder), this))
{
    // The object name identifies the action across restarts, the component
    // decides where the shortcut shows up in the Plasma settings, and the
    // display name is what stands beside it there.
    for (const Shortcut which : {Shortcut::Capture, Shortcut::Recorder}) {
        QAction *action = actionFor(which);
        action->setObjectName(actionId(which));
        action->setProperty("componentName", shortcutComponent());
        action->setProperty("componentDisplayName", i18n("Denkzettel"));
    }

    connect(m_captureAction, &QAction::triggered, this, &GlobalShortcuts::captureRequested);
    connect(m_recorderAction, &QAction::triggered, this, &GlobalShortcuts::recorderRequested);
}

QAction *GlobalShortcuts::actionFor(Shortcut which) const
{
    return which == Shortcut::Capture ? m_captureAction : m_recorderAction;
}

QList<ShortcutOwner> GlobalShortcuts::registerShortcut(Shortcut which)
{
    QAction *action = actionFor(which);
    const QKeySequence sequence = defaultSequence(which);

    QList<ShortcutOwner> owners;
    const QList<KGlobalShortcutInfo> registered = KGlobalAccel::globalShortcutsByKey(sequence, KGlobalAccel::Equal);
    owners.reserve(registered.size());
    for (const KGlobalShortcutInfo &info : registered) {
        owners.append({info.componentUniqueName(), ownerDescription(info)});
    }

    // Autoloading restores a sequence the user changed in the Plasma settings
    // or on the settings page, and only stores ours on the very first
    // registration (SPEC 2.4). The return value is not looked at: it cannot
    // show a backend failure, because doRegister() sends its D-Bus call and
    // drops the answer.
    KGlobalAccel::setGlobalShortcut(action, sequence);

    // So the daemon is asked what it actually holds, and the desktop file it
    // resolves us through is read for the action it starts on the key press.
    // Both failures are silent otherwise — the user met each of them once,
    // on 01.08.2026 (retro B5).
    const QString desktopFile = desktopFilePath();
    const QKeySequence held = assignedSequence(which);
    const ShortcutRegistration registration =
        shortcutRegistration(held.isEmpty() ? QList<QKeySequence>() : QList<QKeySequence>{held},
                             !desktopFile.isEmpty(),
                             desktopFileDeclaresAction(desktopFile, action->objectName()));
    if (registration != ShortcutRegistration::Reached) {
        // Unlike a conflict this leaves no working shortcut at all, so it is
        // reported at every start rather than on the first one only. The
        // message names what the service holds, and falls back to what was
        // asked for only where it holds nothing: DesktopActionMissing is the
        // one branch reached **with** a stored sequence, and that is precisely
        // the case where the user has chosen one of their own.
        const QString failure = shortcutRegistrationFailure(registration, held.isEmpty() ? sequence : held);
        qWarning("%s", qPrintable(failure));
        KNotification::event(KNotification::Error, i18n("Shortcut not ready"), failure);
        return {};
    }

    return foreignShortcutOwners(owners, shortcutComponent());
}

QKeySequence GlobalShortcuts::changeSequence(Shortcut which, const QKeySequence &sequence)
{
    const QList<QKeySequence> wanted = sequence.isEmpty() ? QList<QKeySequence>() : QList<QKeySequence>{sequence};
    KGlobalAccel::self()->setShortcut(actionFor(which), wanted, KGlobalAccel::NoAutoloading);

    return assignedSequence(which);
}

void notifyShortcutConflict(const QKeySequence &sequence, const QList<ShortcutOwner> &conflicts)
{
    QStringList descriptions;
    descriptions.reserve(conflicts.size());
    for (const ShortcutOwner &conflict : conflicts) {
        descriptions.append(conflict.description);
    }

    KNotification::event(KNotification::Warning,
                         i18n("Shortcut already taken"),
                         i18n("%1 already belongs to: %2. The key press may not reach Denkzettel. "
                              "To change that, open the system settings under “Shortcuts”.",
                              sequence.toString(QKeySequence::NativeText),
                              descriptions.join(QStringLiteral(", "))));
}
