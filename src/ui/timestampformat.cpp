#include "ui/timestampformat.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QList>
#include <QLocale>

#include <algorithm>

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

/** Whether `c` stands for a field of a date or time format rather than for text. */
bool isFieldLetter(QChar c)
{
    // Only these four carry meaning to the functions below — the day, month
    // and year fields `withFourDigitYear()` widens, and the minute field
    // `withSeconds()` extends. Every other letter is text that Qt copies out
    // unchanged — the "de" of "d 'de' MMMM 'de' yyyy", the 年 of
    // "yyyy年M月d日" and the hour, am/pm and time-zone letters this file
    // never touches among them.
    return c == u'd' || c == u'M' || c == u'y' || c == u'm';
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
 * `format` with its minute field widened by seconds — built the way
 * `withFourDigitYear()` widens the year. `QLocale::LongFormat` times carry a
 * time zone name that has no place in this UI (measured with Qt 6.11), so the
 * short pattern is the one seconds are added to.
 */
QString withSeconds(const QString &format)
{
    const QList<FormatPart> parts = splitFormat(format);

    QString pattern;
    for (const FormatPart &part : parts) {
        pattern += part.isField && part.text.startsWith(u'm') ? part.text + QStringLiteral(":ss") : part.text;
    }
    return pattern;
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

QString library::entryTimestamp(const QDateTime &when, const QLocale &locale)
{
    return locale.toString(when, withFourDigitYear(locale.dateTimeFormat(QLocale::ShortFormat)));
}

QString library::relativeTimestamp(const QDateTime &when, const QLocale &locale)
{
    const QString pattern = withSeconds(withFourDigitYear(locale.dateTimeFormat(QLocale::ShortFormat)));
    const QString weekday = locale.toString(when.date(), QStringLiteral("dddd"));
    return weekday + QStringLiteral(", ") + locale.toString(when, pattern);
}

QString library::clockTime(qint64 milliseconds)
{
    const qint64 seconds = std::max(qint64(0), milliseconds) / 1000;

    return QStringLiteral("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QLatin1Char('0'));
}
