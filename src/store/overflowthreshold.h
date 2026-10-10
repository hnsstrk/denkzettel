#pragma once

#include <KConfigGroup>

#include <algorithm>

/**
 * The two thresholds of SPEC 11 and the one place their defaults stand.
 *
 * Three parties need the same numbers: the overflow guard, which reads
 * `denkzettelrc` at runtime (shell/overflowguard.h); the settings skeleton,
 * which offers them on the form — `settings.cpp` is built from these, so there
 * is nothing here to keep in step; and since issue #35 the suggestion run,
 * which offers the old notes without a cluster as one bundle at the very
 * threshold the guard reminds about (SPEC 7.3, 11). The reasoning is the one
 * `analysis::DefaultIntervalMinutes` carries: written down twice, a default
 * changed on one side alone would let the dialog show one number while the
 * guard obeys another, and nothing would say which of the two the user is
 * reading (CLAUDE.md, finding 48).
 *
 * **Here and not beside the guard**, which is where it stood until issue #35:
 * `denkzettelanalysis` does not link `denkzettelshell` and must not — the one
 * library both of them link is `denkzettelstore`, and that is the whole reason
 * this file exists. It is the reason `noteFileStem()` stands in `note.h`.
 *
 * `denkzettelstore` itself includes this header nowhere and links no
 * `KF6::ConfigCore` for it. Every library that does include it — analysis,
 * shell, settings — names that component itself, so nothing here hangs on a
 * link that only holds by way of a third library.
 *
 * The **bounds** deliberately do not stand here. They are on the skeleton's
 * items so the form cannot write a number the user did not mean, and their
 * floor exists so a spin box never has to show a singular (settings.h) — both
 * of which are the dialog's business.
 */
namespace overflow
{
inline constexpr int DefaultNotes = 200;
inline constexpr int DefaultDays = 30;
}

/**
 * The age from which on an unexported note counts as having waited too long
 * (SPEC 11): `[Export] OverflowDays` out of the group handed in, default 30.
 *
 * The key is the skeleton's, spelled there and here and in no third place
 * (settings.cpp, group "Export"). A key written under another name would leave
 * the form writing a value nothing ever reads.
 *
 * **The floor of 1 is the only bound this side needs, and it is not the
 * form's**: at 0 or below every note is over the threshold, one written this
 * minute included, and both callers would be **wrong** rather than merely
 * unusual — the guard would remind about an empty library, and the suggestion
 * run would offer today's notes as "the oldest". A hand-written number above
 * the form's ceiling is a threshold the user meant, and it is obeyed.
 *
 * The `KConfigGroup` is the parameter rather than opened here, because
 * `overflowReport()` is handed one for a reason of its own (overflowguard.h):
 * the file name follows the application name, which main() sets.
 */
inline int overflowDayLimit(const KConfigGroup &configuration)
{
    return std::max(1, configuration.readEntry("OverflowDays", overflow::DefaultDays));
}
