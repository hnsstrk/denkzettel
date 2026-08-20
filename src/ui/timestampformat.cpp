#include "ui/timestampformat.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QList>
#include <QLocale>

#include <algorithm>
#include <utility>

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

/**
 * One piece of a date format: either a run of one and the same field letter
 * ("dddd", "MM", "yyyy") or the text between two such runs.
 */
struct FormatPart {
    QString text;
    bool isField = false;
};

/** Whether `c` stands for a field of a date format rather than for text. */
bool isFieldLetter(QChar c)
{
    // Only these three carry meaning in a date format. Every other letter is
    // text that Qt copies out unchanged — the "de" of "d 'de' MMMM 'de' yyyy"
    // and the 年 of "yyyy年M月d日" among them.
    return c == u'd' || c == u'M' || c == u'y';
}

/** `format` taken apart into its fields and the text between them. */
QList<FormatPart> splitFormat(const QString &format)
{
    QList<FormatPart> parts;
    for (qsizetype i = 0; i < format.size();) {
        const QChar c = format.at(i);
        if (isFieldLetter(c)) {
            qsizetype end = i;
            while (end < format.size() && format.at(end) == c) {
                ++end;
            }
            parts.append({format.sliced(i, end - i), true});
            i = end;
            continue;
        }

        qsizetype end = i;
        while (end < format.size()) {
            if (format.at(end) == u'\'') {
                // A quoted run counts as text as a whole, so the "d" in "'de'"
                // does not read as a day.
                const qsizetype close = format.indexOf(u'\'', end + 1);
                end = close < 0 ? format.size() : close + 1;
                continue;
            }
            if (isFieldLetter(format.at(end))) {
                break;
            }
            ++end;
        }
        parts.append({format.sliced(i, end - i), false});
        i = end;
    }
    return parts;
}

/**
 * The pattern of the week form: `longFormat` without its year and with the
 * abbreviated weekday name in place of the written-out one.
 *
 * `QLocale::FormatType` knows three date formats and all three carry a year,
 * so the week form has to be cut out of the long one. What goes with the year
 * is the text next to it, and which side that stands on is a property of the
 * pattern, not of a list of separators: "dddd, MMMM d, yyyy" would leave a
 * hanging comma, "dddd, d 'de' MMMM 'de' yyyy" a hanging "de",
 * "yyyy年M月d日dddd" a hanging 年.
 */
QString weekPattern(const QString &longFormat)
{
    QList<FormatPart> parts = splitFormat(longFormat);
    const auto year = std::find_if(parts.begin(), parts.end(), [](const FormatPart &part) {
        return part.isField && part.text.startsWith(u'y');
    });
    if (year != parts.end()) {
        const auto afterYear = year + 1;
        if (afterYear != parts.end() && !afterYear->isField) {
            parts.erase(year, afterYear + 1);
        } else if (year != parts.begin() && !(year - 1)->isField) {
            parts.erase(year - 1, afterYear);
        } else {
            parts.erase(year);
        }
    }

    QString pattern;
    for (const FormatPart &part : std::as_const(parts)) {
        pattern += part.isField && part.text == QStringLiteral("dddd") ? QStringLiteral("ddd") : part.text;
    }

    // What the cut leaves standing at an edge is separator, never a name: the
    // space of "dddd, d. MMMM yyyy" ahead of the year, the period of the
    // Croatian "dddd, d. MMMM yyyy." behind it.
    while (!pattern.isEmpty() && !pattern.back().isLetterOrNumber() && pattern.back() != u'\'') {
        pattern.chop(1);
    }
    while (!pattern.isEmpty() && !pattern.front().isLetterOrNumber() && pattern.front() != u'\'') {
        pattern.remove(0, 1);
    }
    return pattern;
}

/** `format` with its year widened to four digits. */
QString withFourDigitYear(const QString &format)
{
    const QList<FormatPart> parts = splitFormat(format);

    QString pattern;
    for (const FormatPart &part : parts) {
        pattern += part.isField && part.text.startsWith(u'y') ? QStringLiteral("yyyy") : part.text;
    }
    return pattern;
}

/**
 * "Di., 28. Juli", "Tue, July 28" — day and month in the arrangement of the
 * locale, without the year, which the group already carries.
 */
QString weekdayForm(const QDate &date, const QLocale &locale)
{
    return locale.toString(date, weekPattern(locale.dateFormat(QLocale::LongFormat)));
}

/**
 * "10.07.2026", "7/10/2026" — the short date of the locale, but with a
 * four-digit year: under de_DE the short form writes "10.07.26", and a note
 * from last July must not read like one from this July.
 */
QString absoluteForm(const QDate &date, const QLocale &locale)
{
    return locale.toString(date, withFourDigitYear(locale.dateFormat(QLocale::ShortFormat)));
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
        return i18nc("@title:group notes written today", "Today");
    case NoteGroup::Yesterday:
        return i18nc("@title:group notes written yesterday", "Yesterday");
    case NoteGroup::ThisWeek:
        return i18nc("@title:group notes of the current calendar week", "This week");
    case NoteGroup::LastWeek:
        return i18nc("@title:group notes of the previous calendar week", "Last week");
    case NoteGroup::Older:
        return i18nc("@title:group notes older than the previous week", "Older");
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
        return i18nc("@item:intext timestamp of a note written today", "Today %1", time);
    case NoteGroup::Yesterday:
        return i18nc("@item:intext timestamp of a note written yesterday", "Yesterday %1", time);
    case NoteGroup::ThisWeek:
    case NoteGroup::LastWeek:
        return weekdayForm(when.date(), locale);
    case NoteGroup::Older:
        break;
    }

    return absoluteForm(when.date(), locale);
}
