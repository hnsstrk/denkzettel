#pragma once

#include <QKeySequence>
#include <QList>
#include <QString>

/** What kglobalacceld answers when asked what it holds for our action. */
enum class ShortcutRegistration {
    Reached, //< the daemon holds a sequence for us
    ApplicationNotInstalled, //< no desktop file, so the daemon drops the component
    DaemonKeptNothing, //< desktop file in place, daemon holds nothing anyway
};

/**
 * Reads the answer of the daemon to `globalShortcut(component, action)`.
 *
 * `KGlobalAccel::setGlobalShortcut()` cannot report a backend failure: it sends
 * the D-Bus call and never looks at the answer, so `false` only ever means a
 * garbage key code or a nameless action. The registration has to be read back
 * from the daemon instead — an empty answer means it never arrived (SPEC 2.4,
 * retro Sprint 2, B5).
 *
 * `desktopFileFound` separates the two ways of failing. kglobalacceld resolves
 * a component whose name ends in `.desktop` through the desktop file and drops
 * it when there is none — that is what happened on 01.08.2026, and a message
 * that blamed the daemon instead would have sent the user down the wrong path.
 */
ShortcutRegistration shortcutRegistration(const QList<QKeySequence> &storedByTheDaemon, bool desktopFileFound);

/**
 * What to tell the user about a registration that did not arrive — empty for
 * ShortcutRegistration::Reached, because then there is nothing to report.
 */
QString shortcutRegistrationFailure(ShortcutRegistration registration);
