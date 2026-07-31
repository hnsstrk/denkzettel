#include "ui/timestampformat.h"

#include <QLocale>
#include <QTest>

/**
 * Unit tests of the library building blocks that work without a visible
 * window (SPEC 16). The window itself — layout, empty states, the look of the
 * message widget — stays on the manual checklist.
 */
class LibraryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void namesTodayAndYesterday();
    void switchesOnTheCalendarDayNotAfterTwentyFourHours();
    void usesTheWeekdayFormWithinTheLastSevenDays();
    void usesTheAbsoluteDateBeyondSevenDays();

private:
    static QDateTime at(const QString &isoDateTime);
    static QLocale german();
};

QDateTime LibraryTest::at(const QString &isoDateTime)
{
    const QDateTime moment = QDateTime::fromString(isoDateTime, Qt::ISODate);
    Q_ASSERT(moment.isValid());
    return moment;
}

QLocale LibraryTest::german()
{
    return QLocale(QLocale::German, QLocale::Germany);
}

void LibraryTest::namesTodayAndYesterday()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-31T14:32:00")), now, german()),
             QStringLiteral("Heute 14:32"));
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-30T21:48:00")), now, german()),
             QStringLiteral("Gestern 21:48"));
}

void LibraryTest::switchesOnTheCalendarDayNotAfterTwentyFourHours()
{
    // Twenty minutes old, but written on the previous calendar day.
    const QDateTime now = at(QStringLiteral("2026-07-31T00:10:00"));

    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-30T23:50:00")), now, german()),
             QStringLiteral("Gestern 23:50"));

    // Almost 24 hours old, but the same calendar day.
    const QDateTime lateEvening = at(QStringLiteral("2026-07-31T23:30:00"));
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-31T00:05:00")), lateEvening, german()),
             QStringLiteral("Heute 00:05"));
}

void LibraryTest::usesTheWeekdayFormWithinTheLastSevenDays()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    // Two days back is the first entry of this form, six days back the last.
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-29T09:00:00")), now, german()),
             QStringLiteral("Mi., 29. Juli"));
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-25T09:00:00")), now, german()),
             QStringLiteral("Sa., 25. Juli"));
}

void LibraryTest::usesTheAbsoluteDateBeyondSevenDays()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    // Seven days back is the switching point, and the year stays four-digit.
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-24T09:00:00")), now, german()),
             QStringLiteral("24.07.2026"));
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2025-07-28T09:00:00")), now, german()),
             QStringLiteral("28.07.2025"));
}

QTEST_MAIN(LibraryTest)

#include "librarytest.moc"
