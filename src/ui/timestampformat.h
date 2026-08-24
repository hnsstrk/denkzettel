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
 * The timestamp a list entry carries (SPEC 9, wireframe 3a): date and time,
 * the same form whichever of the five groups the note falls into — the group
 * head names none of it any more (issue #108).
 *
 * Arrangement, separators and the AM/PM marker all come out of `locale`, none
 * of them out of a pattern written here: under de_DE this reads
 * "24.08.2026 15:42", under en_US "8/24/2026 3:42 PM". The year is always
 * four digits; the list carries no seconds, so it stays narrow.
 */
QString entryTimestamp(const QDateTime &when, const QLocale &locale);

/**
 * The timestamp of the detail pane (SPEC 9): weekday, date and time with
 * seconds, the same form for every note — no "Today", no "Yesterday". Under
 * de_DE "Montag, 24.08.2026 15:42:07", under en_US
 * "Monday, 8/24/2026 3:42:07 PM". The comma after the weekday is the one
 * literal in the whole form; everything else again comes out of `locale`.
 */
QString relativeTimestamp(const QDateTime &when, const QLocale &locale);
}
