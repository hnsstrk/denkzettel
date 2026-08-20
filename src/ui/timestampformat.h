#pragma once

#include <QString>

#include <cstdint>

class QDateTime;
class QLocale;

namespace library
{
/**
 * The group a note falls into in the library list (SPEC 9, wireframe 3a):
 * five groups in a fixed order.
 *
 * The groups overlap — on a Monday the previous day lies both in "Yesterday" and
 * in the previous calendar week — so the first matching one wins, in the order
 * they are written here.
 */
enum class NoteGroup : std::uint8_t {
    Today,
    Yesterday,
    ThisWeek,
    LastWeek,
    Older,
};

/**
 * The group `when` belongs to, seen from `now`.
 *
 * "Woche" is the calendar week; its first day comes from `locale`
 * (`QLocale::firstDayOfWeek`, Monday in Germany — SPEC 9). A timestamp in the
 * future is a clock jump and sorts into "Today", the group at the top.
 */
NoteGroup noteGroup(const QDateTime &when, const QDateTime &now, const QLocale &locale);

/** The heading the list writes above `group`. */
QString groupTitle(NoteGroup group);

/**
 * The timestamp a list entry carries (SPEC 9, wireframe 3a). It says what its
 * group leaves open and nothing twice: under "Today" and "Yesterday" the head
 * carries the day, so the entry shows the time alone ("2:32 PM"); the week
 * groups name no day, so the entry does ("Tue, July 28"); under "Older" only
 * the date says anything ("7/28/2026").
 *
 * Names, arrangement and separators all come out of `locale`, none of them out
 * of a pattern written here: under de_DE the same three forms read "14:32",
 * "Di., 28. Juli" and "28.07.2026".
 *
 * A timestamp from the future keeps its absolute date although it sorts into
 * "Today" — "9:00 AM" under today's head would be a lie about a clock jump.
 */
QString entryTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale);

/**
 * The timestamp of the detail pane, which stands under no group head and
 * therefore keeps the day: "Today 2:32 PM", "Yesterday 9:48 PM",
 * "Tue, July 28" within this and the last calendar week, an absolute date
 * beyond that.
 *
 * The switch happens on the calendar day, not after 24 hours: a note from
 * yesterday at 23:50 reads "Yesterday" at 00:10, not "Today". `now` is a
 * parameter so the rule can be tested against fixed points in time.
 */
QString relativeTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale);
}
