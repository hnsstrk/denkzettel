#include "shell/overflowguard.h"

#include "store/store.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QDateTime>

#include <algorithm>

OverflowReport overflowReport(const Store &store, KConfigGroup &configuration)
{
    // The two keys are the skeleton's, spelled there and here and in no third
    // place (settings.cpp, group "Export"). A key written under another name
    // would leave the form writing a value nothing ever reads.
    //
    // The floor of 1 is the only bound this side needs, and it is not the
    // form's: at 0 or below every library is over the threshold, an empty one
    // included, and the reminder would be **wrong** rather than merely
    // unusual. A hand-written number above the form's ceiling is a threshold
    // the user meant, and it is obeyed.
    const int noteLimit = std::max(1, configuration.readEntry("OverflowNotes", overflow::DefaultNotes));
    const int dayLimit = std::max(1, configuration.readEntry("OverflowDays", overflow::DefaultDays));

    // `categoryCounts().total` and no count of its own: that is the number the
    // library writes beside "All", and two readings of one condition agree
    // until somebody edits one of them (CLAUDE.md, finding 48).
    const int notes = store.categoryCounts().total;
    const QDateTime oldest = store.oldestNoteTimestamp();
    const int days =
        oldest.isValid() ? static_cast<int>(oldest.daysTo(QDateTime::currentDateTime())) : 0;

    const bool tooMany = notes >= noteLimit;
    const bool tooOld = days >= dayLimit;
    const bool over = tooMany || tooOld;

    OverflowReport report;
    if (!over) {
        // Nothing to show and, below, nothing to say. The empty `state` is what
        // takes the tray part back — the tray's other parts are taken back the
        // same way, and a part that stayed after its cause was gone would be
        // the permanent finding nobody reads any more (issue #118).
        if (configuration.readEntry("OverflowReminded", false)) {
            configuration.writeEntry("OverflowReminded", false);
            configuration.sync();
        }
        return report;
    }

    // **The criterion that gave way writes the line, and each of the two
    // carries exactly one number** (UX decision of 04.09.2026, which replaced
    // a single wording carrying both).
    //
    // One sentence with two numbers had to be given up for a reason a check
    // could not have found, only a readback off the running item: at the count
    // trigger it read "2 notes waiting for export, the oldest for **0 days**"
    // — not a grammatical slip but a claim that is untrue, and it stands on
    // every day somebody captures a lot. Beside it, `i18np` picks its form by
    // the count and never by the second number, so "the oldest for 1 days" was
    // unavoidable in the same sentence.
    //
    // **This is no ranking in the sense of issue #118.** There a second
    // *trouble* was being hidden. Here there is one trouble — SPEC 11 calls it
    // one guard and joins its two criteria with an `or` — and the line says
    // which of the two is the reason. Both at once takes the count, which is
    // the number the user acts on.
    report.state = tooMany
                       ? i18np("%1 note waiting for export", "%1 notes waiting for export", notes)
                       : i18np("The oldest note has been waiting %1 day for export",
                               "The oldest note has been waiting %1 days for export", days);

    // The whole of "exactly one reminder per state": nothing is **said** while
    // the state is the one already recorded, although the line above stands on
    // every call. The marker is written on both edges — falling back below is
    // what lets the next crossing speak again.
    if (configuration.readEntry("OverflowReminded", false)) {
        return report;
    }
    configuration.writeEntry("OverflowReminded", true);
    configuration.sync();
    // The same branch as the line above, and deliberately **not** the same
    // words (UX decision of 04.09.2026): the tray part is one item of a list
    // that gets skimmed and carries no full stop, like the four parts beside
    // it; a notification is a sentence that stands alone and carries one, like
    // every other notification here. Two msgids saved would have cost each
    // channel what it needs.
    report.reminder = tooMany
                          ? i18np("%1 note has not been exported yet.",
                                  "%1 notes have not been exported yet.", notes)
                          : i18np("The oldest unexported note is %1 day old.",
                                  "The oldest unexported note is %1 days old.", days);
    return report;
}
