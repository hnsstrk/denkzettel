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
 * What the overflow guard of SPEC 11 has to say — the two channels SPEC 11
 * names, and they say different things.
 */
struct OverflowReport {
    /**
     * The tray part: the quiet channel of SPEC 14, standing for **as long as
     * its cause does** like the four parts beside it, and empty while the
     * library is under both thresholds.
     */
    QString state;
    /**
     * The loud channel: the sentence to notify with, filled **only** at the
     * moment the library crosses, and empty on every other call.
     */
    QString reminder;
};

/**
 * The overflow guard of SPEC 11: says once that the library is due an export,
 * and goes on showing where it stands for as long as it stands there.
 *
 * `reminder` is filled at the moment the library **crosses** one of the two
 * thresholds — count of unexported notes, or age of the oldest of them — and
 * empty on every other call. That is the whole of the story's first acceptance
 * criterion: crossing gives exactly one reminder, a library that stays over
 * gives none, and a library that falls back below (an export happened) may
 * remind again the next time it fills up. `state` follows the other rule, the
 * one the tray's four existing parts follow: it stands while the library is
 * over and is empty while it is not, so a caller that hands it to the tray on
 * every call is right at every moment.
 *
 * **One function and not two**, although the two channels behave differently:
 * both rest on the same two counts and the same two thresholds, and two
 * readings of one condition agree until somebody edits one of them
 * (CLAUDE.md, finding 48). It also keeps the marker to one writer.
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
OverflowReport overflowReport(const Store &store, KConfigGroup &configuration);
