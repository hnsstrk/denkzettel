#pragma once

#include <QString>

class QDateTime;
class QLocale;

namespace library
{
/**
 * The timestamp a note list entry carries (SPEC 9, wireframe 2b): "Heute
 * 14:32", "Gestern 21:48", "Di., 28. Juli" within the last seven days, an
 * absolute date beyond that.
 *
 * The switch happens on the calendar day, not after 24 hours: a note from
 * yesterday at 23:50 reads "Gestern" at 00:10, not "Heute". `now` is a
 * parameter so the rule can be tested against fixed points in time.
 */
QString relativeTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale);
}
