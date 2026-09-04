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
    const bool over = tooMany || days >= dayLimit;

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

    // **One wording under both criteria, carrying both numbers** (UX decision
    // of 04.09.2026). The two triggers are not two troubles: SPEC 11 calls
    // them one guard and joins them with an `or`, so a sentence naming the
    // count and the age is true under either — "213 notes waiting for export,
    // the oldest for 4 days" is a statement about the count, "6 notes …, the
    // oldest for 41 days" one about the age, and nothing has to decide which.
    // Two wordings would have needed a rule for the case where both criteria
    // give way at once, and that rule is the ranking issue #118 threw out.
    //
    // The count alone would not do it: under the age trigger "6 notes waiting
    // for export" is a riddle rather than a message — a harmless number with
    // no reason beside it.
    report.state = i18np("%1 note waiting for export for %2 days",
                         "%1 notes waiting for export, the oldest for %2 days", notes, days);

    // The whole of "exactly one reminder per state": nothing is **said** while
    // the state is the one already recorded, although the line above stands on
    // every call. The marker is written on both edges — falling back below is
    // what lets the next crossing speak again.
    if (configuration.readEntry("OverflowReminded", false)) {
        return report;
    }
    configuration.writeEntry("OverflowReminded", true);
    configuration.sync();
    // Which of the two criteria crossed decides the sentence, because the two
    // read as different news: at the default thresholds a library of five
    // notes that has lain untouched for a month is nothing the count could
    // explain. Both crossed at once is reported as the count, which is the
    // number the user acts on.
    report.reminder =
        tooMany ? i18np("%1 note is waiting for an export.",
                        "%1 notes are waiting for an export.", notes)
                : i18np("The oldest note has been waiting for an export for %1 day.",
                        "The oldest note has been waiting for an export for %1 days.", days);
    return report;
}
