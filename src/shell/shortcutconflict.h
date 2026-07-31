#pragma once

#include <QList>
#include <QString>

/** A component holding a global shortcut, as reported by KGlobalAccel. */
struct ShortcutOwner {
    QString component; //< unique component name, e.g. "kwin"
    QString description; //< human readable, for the conflict message
};

/**
 * The entries of `owners` that do not belong to `ownComponent`.
 *
 * A KGlobalAccel registration can fail invisibly: the entry appears and
 * invokeShortcut works, but the real key press keeps going to the existing
 * owner (T1 finding, SPEC 2.4). Only foreign owners are a conflict — from the
 * second start on our own component is part of the list, because kglobalacceld
 * keeps the registration. One sequence can have several owners.
 */
QList<ShortcutOwner> foreignShortcutOwners(const QList<ShortcutOwner> &owners, const QString &ownComponent);
