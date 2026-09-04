#include "shell/overflowguard.h"

#include "store/store.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QDateTime>

#include <algorithm>

QString overflowReminder(const Store &store, KConfigGroup &configuration)
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

    // The whole of "exactly one reminder per state": nothing is said while the
    // state is the one already recorded, and the marker is written on both
    // edges — falling back below is what lets the next crossing speak again.
    if (over == configuration.readEntry("OverflowReminded", false)) {
        return {};
    }
    configuration.writeEntry("OverflowReminded", over);
    configuration.sync();

    if (!over) {
        return {};
    }
    // Which of the two criteria crossed decides the sentence, because the two
    // read as different news: at the default thresholds a library of five
    // notes that has lain untouched for a month is nothing the count could
    // explain. Both crossed at once is reported as the count, which is the
    // number the user acts on.
    return tooMany ? i18np("%1 note is waiting for an export.",
                           "%1 notes are waiting for an export.", notes)
                   : i18np("The oldest note has been waiting for an export for %1 day.",
                           "The oldest note has been waiting for an export for %1 days.", days);
}
