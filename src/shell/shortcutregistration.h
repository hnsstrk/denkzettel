#pragma once

#include <QKeySequence>
#include <QList>
#include <QString>

#include <cstdint>

/** What kglobalacceld answers when asked what it holds for our action. */
enum class ShortcutRegistration : std::uint8_t {
    Reached, //< the daemon holds a sequence for us
    ApplicationNotInstalled, //< no desktop file, so the daemon drops the component
    DaemonKeptNothing, //< desktop file in place, daemon holds nothing anyway
    DesktopActionMissing, //< registered, but the desktop file declares no such action
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
 *
 * `desktopFileDeclaresTheAction` catches the failure of the second attempt:
 * with a desktop file in place the daemon starts the desktop action of the same
 * name on the key press instead of telling us over D-Bus, and without that
 * group it logs an error and stops. The registration then reads back fine while
 * the key does nothing. Only asked when the file was found — an installation we
 * cannot see is none we should judge.
 */
ShortcutRegistration shortcutRegistration(const QList<QKeySequence> &storedByTheDaemon,
                                          bool desktopFileFound,
                                          bool desktopFileDeclaresTheAction);

/**
 * Whether the desktop file at `desktopFilePath` declares `actionId` — listed
 * under `Actions=` and present as a `[Desktop Action <actionId>]` group, which
 * is what kglobalacceld looks for. False for an empty path or an unreadable
 * file: what cannot be read cannot be vouched for.
 */
bool desktopFileDeclaresAction(const QString &desktopFilePath, const QString &actionId);

/**
 * What to tell the user about a registration that did not arrive — empty for
 * ShortcutRegistration::Reached, because then there is nothing to report.
 */
QString shortcutRegistrationFailure(ShortcutRegistration registration);
