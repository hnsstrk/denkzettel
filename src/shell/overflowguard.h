#pragma once

#include <QString>

class KConfigGroup;
class Store;

/**
 * The two thresholds of SPEC 11 and the one place their defaults stand.
 *
 * Two parties need the same numbers: the reminder below, which reads
 * `denkzettelrc` at runtime, and the settings skeleton, which offers them on
 * the form — `settings.cpp` is built from these, so there is nothing here to
 * keep in step. The reasoning is the one `analysis::DefaultIntervalMinutes`
 * carries: written down twice, a default changed on one side alone would let
 * the dialog show one number while the guard obeys another, and nothing would
 * say which of the two the user is reading (CLAUDE.md, finding 48).
 *
 * Here and not in the settings, because the dependency only runs one way:
 * `denkzettelsettings` links `denkzettelshell`, not the other way round.
 *
 * The **bounds** deliberately do not stand here. They are on the skeleton's
 * items so the form cannot write a number the user did not mean, and their
 * floor exists so a spin box never has to show a singular (settings.h) — both
 * of which are the dialog's business. What the reminder guards itself against
 * is only the one value that would make it answer wrongly rather than
 * unusually; see the floor in overflowguard.cpp.
 */
namespace overflow
{
inline constexpr int DefaultNotes = 200;
inline constexpr int DefaultDays = 30;
}

/**
 * The overflow guard of SPEC 11: says once that the library is due an export,
 * and says nothing at all the rest of the time.
 *
 * Returns the sentence to remind with at the moment the library **crosses**
 * one of the two thresholds — count of unexported notes, or age of the oldest
 * of them — and an empty string on every other call. That is the whole of the
 * story's first acceptance criterion: crossing gives exactly one reminder, a
 * library that stays over gives none, and a library that falls back below
 * (an export happened) may remind again the next time it fills up.
 *
 * **The reminded state is written into `configuration` and not held in
 * memory**, because it has to survive a restart: the daemon runs for a session
 * and the library fills up over weeks, so a marker that died with the process
 * would greet the user with the same reminder at every login. `KConfigGroup`
 * is the parameter and not opened here for the reason runFirstStart() gives —
 * the file name follows the application name, which main() sets.
 *
 * **It exports nothing and asks nobody to** (SPEC 11, acceptance criterion 2).
 * It reads two counts and writes one marker; `denkzettelshell` does not even
 * link `denkzettelproposals`, where every export of SPEC 8 lives.
 */
QString overflowReminder(const Store &store, KConfigGroup &configuration);
