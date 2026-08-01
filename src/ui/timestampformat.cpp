#include "ui/timestampformat.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QLocale>

namespace
{
/** The day `date`'s calendar week began on, as the locale counts weeks. */
QDate startOfWeek(const QDate &date, const QLocale &locale)
{
    // QDate::dayOfWeek() and Qt::DayOfWeek agree: Monday is 1, Sunday is 7.
    int back = date.dayOfWeek() - static_cast<int>(locale.firstDayOfWeek());
    if (back < 0) {
        back += 7;
    }
    return date.addDays(-back);
}

/** "Di., 28. Juli" — the day and month names come from the locale, the
 * arrangement is the German one, as is the whole application (SPEC 15). */
QString weekdayForm(const QDate &date, const QLocale &locale)
{
    return locale.toString(date, QStringLiteral("ddd, d. MMMM"));
}

QString absoluteForm(const QDate &date, const QLocale &locale)
{
    return locale.toString(date, QStringLiteral("dd.MM.yyyy"));
}
}

library::NoteGroup library::noteGroup(const QDateTime &when, const QDateTime &now, const QLocale &locale)
{
    const QDate day = when.date();
    const QDate today = now.date();

    // A date in the future is a clock jump, not a group of its own: the note is
    // the newest one there is and belongs at the top.
    if (day >= today) {
        return NoteGroup::Today;
    }

    if (day == today.addDays(-1)) {
        return NoteGroup::Yesterday;
    }

    const QDate weekStart = startOfWeek(today, locale);
    if (day >= weekStart) {
        return NoteGroup::ThisWeek;
    }

    if (day >= weekStart.addDays(-7)) {
        return NoteGroup::LastWeek;
    }

    return NoteGroup::Older;
}

QString library::groupTitle(NoteGroup group)
{
    switch (group) {
    case NoteGroup::Today:
        return i18nc("@title:group notes written today", "Heute");
    case NoteGroup::Yesterday:
        return i18nc("@title:group notes written yesterday", "Gestern");
    case NoteGroup::ThisWeek:
        return i18nc("@title:group notes of the current calendar week", "Diese Woche");
    case NoteGroup::LastWeek:
        return i18nc("@title:group notes of the previous calendar week", "Letzte Woche");
    case NoteGroup::Older:
        return i18nc("@title:group notes older than the previous week", "Älter");
    }

    return {};
}

QString library::entryTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale)
{
    if (when.date() > now.date()) {
        return absoluteForm(when.date(), locale);
    }

    switch (noteGroup(when, now, locale)) {
    case NoteGroup::Today:
    case NoteGroup::Yesterday:
        return locale.toString(when.time(), QLocale::ShortFormat);
    case NoteGroup::ThisWeek:
    case NoteGroup::LastWeek:
        return weekdayForm(when.date(), locale);
    case NoteGroup::Older:
        break;
    }

    return absoluteForm(when.date(), locale);
}

QString library::relativeTimestamp(const QDateTime &when, const QDateTime &now, const QLocale &locale)
{
    if (when.date() > now.date()) {
        return absoluteForm(when.date(), locale);
    }

    const QString time = locale.toString(when.time(), QLocale::ShortFormat);

    switch (noteGroup(when, now, locale)) {
    case NoteGroup::Today:
        return i18nc("@item:intext timestamp of a note written today", "Heute %1", time);
    case NoteGroup::Yesterday:
        return i18nc("@item:intext timestamp of a note written yesterday", "Gestern %1", time);
    case NoteGroup::ThisWeek:
    case NoteGroup::LastWeek:
        return weekdayForm(when.date(), locale);
    case NoteGroup::Older:
        break;
    }

    return absoluteForm(when.date(), locale);
}
