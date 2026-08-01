#pragma once

#include <QString>

class QDateTime;
class QLocale;

namespace library
{
/**
 * The group a note falls into in the library list (SPEC 9, wireframe 3a):
 * five groups in a fixed order.
 *
 * The groups overlap — on a Monday the previous day lies both in "Gestern" and
 * in the previous calendar week — so the first matching one wins, in the order
 * they are written here.
 */
enum class NoteGroup {
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
 * future is a clock jump and sorts into "Heute", the group at the top.
 */
NoteGroup noteGroup(const QDateTime &when, const QDateTime &now, const QLocale &locale);

/** The heading the list writes above `group`. */
QString groupTitle(NoteGroup group);

/**
 * The timestamp a list entry carries (SPEC 9, wireframe 3a). It says what its
 * group leaves open and nothing twice: under "Heute" and "Gestern" the head
 * carries the day, so the entry shows the time alone ("14:32"); the week
 * groups name no day, so the entry does ("Di., 28. Juli"); under "Älter" only
 * the date says anything ("28.07.2026").
 *
 * A timestamp from the future keeps its absolute date although it sorts into
 * "Heute" — "09:00" under today's head would be a lie about a clock jump.
 */
QString entryTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale);

/**
 * The timestamp of the detail pane, which stands under no group head and
 * therefore keeps the day: "Heute 14:32", "Gestern 21:48", "Di., 28. Juli"
 * within this and the last calendar week, an absolute date beyond that.
 *
 * The switch happens on the calendar day, not after 24 hours: a note from
 * yesterday at 23:50 reads "Gestern" at 00:10, not "Heute". `now` is a
 * parameter so the rule can be tested against fixed points in time.
 */
QString relativeTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale);
}
