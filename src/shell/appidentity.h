#pragma once

class QCoreApplication;

/**
 * Registers name, version and the two identities the session hangs the daemon
 * on: the domain KDBusService builds io.github.hnsstrk.denkzettel from (SPEC
 * 2.3) and the desktop file kglobalacceld names the shortcut component after
 * (SPEC 2.4).
 *
 * Has to run before KDBusService, and nothing beside it may set
 * applicationName, organizationDomain or desktopFileName: KAboutData writes all
 * three, and a second setter would decide which of the two wins by line order.
 */
void registerApplicationIdentity();

/**
 * Answers --version and --help and refuses a switch nobody declared.
 *
 * Ends the process in each of those three cases and returns only for an
 * argument list the daemon is meant to start on. Has to run before
 * KDBusService as well — see the comment at the call site.
 */
void processCommandLineArguments(const QCoreApplication &app);
