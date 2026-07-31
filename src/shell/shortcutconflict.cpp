#include "shell/shortcutconflict.h"

QList<ShortcutOwner> foreignShortcutOwners(const QList<ShortcutOwner> &owners, const QString &ownComponent)
{
    QList<ShortcutOwner> conflicts;
    for (const ShortcutOwner &owner : owners) {
        if (owner.component != ownComponent) {
            conflicts.append(owner);
        }
    }
    return conflicts;
}
