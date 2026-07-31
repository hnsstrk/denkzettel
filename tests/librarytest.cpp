#include "ui/elidedlines.h"
#include "ui/notelistmodel.h"
#include "ui/timestampformat.h"

#include <QFontMetrics>
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

    void keepsShortTextOnOneLine();
    void elidesTextBeyondTwoLines();
    void foldsLineBreaksIntoThePreview();
    void hasNoLinesForEmptyText();

    void listsNotesWithTheirTimestamp();
    void takesAndReinsertsARow();

private:
    static QDateTime at(const QString &isoDateTime);
    static QLocale german();
    static Note noteWith(const QString &content);

    /** Width of ten wide characters — enough for a few words, not for many. */
    static int narrowWidth();
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

Note LibraryTest::noteWith(const QString &content)
{
    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.content = content;
    return note;
}

int LibraryTest::narrowWidth()
{
    return QFontMetrics(QFont()).horizontalAdvance(QStringLiteral("MMMMMMMMMM"));
}

void LibraryTest::keepsShortTextOnOneLine()
{
    const QStringList lines = library::elidedLines(QStringLiteral("kurz"), QFont(), narrowWidth(), 2);

    QCOMPARE(lines, QStringList{QStringLiteral("kurz")});
}

void LibraryTest::elidesTextBeyondTwoLines()
{
    const QString long_ = QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten, "
                                         "als Cronjob auf dem NAS einrichten und danach einmal wiederherstellen");

    const QStringList lines = library::elidedLines(long_, QFont(), narrowWidth(), 2);

    QCOMPARE(lines.size(), 2);
    QVERIFY2(lines.last().endsWith(QChar(0x2026)), qPrintable(lines.last()));
    QVERIFY(long_.startsWith(lines.first()));
}

void LibraryTest::foldsLineBreaksIntoThePreview()
{
    const QStringList lines =
        library::elidedLines(QStringLiteral("erste\n\nzweite"), QFont(), 10 * narrowWidth(), 2);

    QCOMPARE(lines, QStringList{QStringLiteral("erste zweite")});
}

void LibraryTest::hasNoLinesForEmptyText()
{
    QVERIFY(library::elidedLines(QString(), QFont(), narrowWidth(), 2).isEmpty());
    QVERIFY(library::elidedLines(QStringLiteral("   \n "), QFont(), narrowWidth(), 2).isEmpty());
}

void LibraryTest::listsNotesWithTheirTimestamp()
{
    NoteListModel model;
    QCOMPARE(model.rowCount(), 0);

    Note yesterday = noteWith(QStringLiteral("gestern gedacht"));
    yesterday.createdAt = QDateTime::currentDateTime().addDays(-1);
    model.setNotes({noteWith(QStringLiteral("heute gedacht")), yesterday});

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.index(0).data(Qt::DisplayRole).toString(), QStringLiteral("heute gedacht"));
    QVERIFY(model.index(0).data(NoteListModel::TimestampRole).toString().startsWith(QStringLiteral("Heute ")));
    QVERIFY(model.index(1).data(NoteListModel::TimestampRole).toString().startsWith(QStringLiteral("Gestern ")));
    QCOMPARE(model.noteAt(1).content, yesterday.content);

    // A row outside the list is a question, not a crash.
    QVERIFY(model.noteAt(2).content.isEmpty());
    QVERIFY(!model.index(2).data(Qt::DisplayRole).isValid());
}

void LibraryTest::takesAndReinsertsARow()
{
    NoteListModel model;
    model.setNotes({noteWith(QStringLiteral("eins")), noteWith(QStringLiteral("zwei")), noteWith(QStringLiteral("drei"))});

    const Note removed = model.noteAt(1);
    model.takeRow(1);

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.noteAt(1).content, QStringLiteral("drei"));

    // Undo puts the note back where it was, not at the end.
    model.insertNote(1, removed);

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.noteAt(1).content, QStringLiteral("zwei"));
    QCOMPARE(model.noteAt(2).content, QStringLiteral("drei"));
}

QTEST_MAIN(LibraryTest)

#include "librarytest.moc"
