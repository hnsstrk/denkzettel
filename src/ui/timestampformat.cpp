#include "ui/timestampformat.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QLocale>

namespace
{
/** Days after which the weekday form stops being useful (wireframe 2b). */
constexpr int WeekdayFormLimit = 7;
}

QString library::relativeTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale)
{
    const qint64 days = when.date().daysTo(now.date());
    const QString time = locale.toString(when.time(), QLocale::ShortFormat);

    if (days == 0) {
        return i18nc("@item:intext timestamp of a note written today", "Heute %1", time);
    }

    if (days == 1) {
        return i18nc("@item:intext timestamp of a note written yesterday", "Gestern %1", time);
    }

    // The day and month names come from the locale; the arrangement is the
    // German one, as is the whole application (SPEC 15).
    if (days > 1 && days < WeekdayFormLimit) {
        return locale.toString(when.date(), QStringLiteral("ddd, d. MMMM"));
    }

    // A timestamp in the future is a clock jump, not a case of its own — the
    // absolute date is the honest answer for it as well.
    return locale.toString(when.date(), QStringLiteral("dd.MM.yyyy"));
}
