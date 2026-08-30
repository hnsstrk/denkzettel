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
 * The timestamp of the detail pane (SPEC 9): the weekday in front of the form
 * the list carries, the same for every note — no "Today", no "Yesterday".
 * Under de_DE "Montag, 24.08.2026 15:42", under en_US
 * "Monday, 8/24/2026 3:42 PM". The comma after the weekday is the one literal
 * in the whole form; everything else again comes out of `locale`.
 *
 * No seconds (issue #124): a note is a thought written down in passing, not a
 * measurement, and the head of the detail pane is the one row in the window
 * where the space is short. They never came out of the locale either — no
 * short format of the 711 locales Qt 6.11 knows carries a seconds field, and
 * the ones written here were appended by a step of this file's own.
 */
QString relativeTimestamp(const QDateTime &when, const QLocale &locale);

/**
 * A running time as the audio player and the list entry of a voice note write
 * it (SPEC 9, wireframe 1b): minutes, a colon, seconds with two digits —
 * "0:41", "12:07".
 *
 * Milliseconds in, because that is what QMediaPlayer counts in and because the
 * step from 0:13 to 0:14 is the one thing a division by a thousand can get
 * wrong. Truncated, not rounded, like every media player: 14.9 seconds are
 * 0:14, and a file of 41.4 seconds is 0:41 both while it is standing and while
 * it is playing.
 *
 * The minutes run on past sixty — an hour reads "60:00", not "1:00:00". SPEC 4
 * ends a recording after fifteen minutes, so no note of this application
 * reaches that; an hour written as "1:00" would be the reading that misleads.
 */
QString clockTime(qint64 milliseconds);
}
