#pragma once

class KConfigGroup;

/**
 * Carries out the first start of the daemon (SPEC 2.5): writes the default
 * configuration and records that the first start has happened. Returns true on
 * the first start only; later starts find the marker and leave the
 * configuration alone.
 *
 * Data directory and database at the current schema version are created by
 * Store::open(), which runs before this — the marker guards the steps that must
 * not repeat, such as the shortcut conflict warning of SPEC 2.4.
 *
 * The configuration group is a parameter rather than opened here: KSharedConfig
 * derives the file name from the application name, and the daemon changes that
 * name while registering on the bus (see main.cpp).
 */
bool runFirstStart(KConfigGroup &configuration);
