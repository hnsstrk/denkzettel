#include "store/store.h"
#include "ui/elidedlines.h"
#include "ui/librarywindow.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"
#include "ui/pendingdeletion.h"
#include "ui/timestampformat.h"

#include <KMessageWidget>
#include <KStandardShortcut>

#include <QAction>
#include <QFontMetrics>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QApplication>
#include <QLocale>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSplitter>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QTimer>
#include <QToolButton>

#include <memory>

/**
 * Unit tests of the library building blocks and of the window itself (SPEC 16).
 * The layout is measured here instead of standing on a manual checklist: the
 * offscreen platform shows a real window, so position and height are as
 * testable as any other state. Manual stays what only a compositor produces —
 * decoration, colour scheme, HiDPI.
 */
class LibraryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void namesTodayAndYesterday();
    void switchesOnTheCalendarDayNotAfterTwentyFourHours();
    void usesTheWeekdayFormWithinTheTwoCalendarWeeks();
    void usesTheAbsoluteDateBeyondTheLastWeek();

    void sortsIntoTheFiveGroupsWithTheFirstMatchWinning();
    void countsTheWeekAsACalendarWeek();
    void leavesTheWeekToTheLocale();
    void shortensTheEntryTimestampToWhatItsGroupLeavesOpen();
    void sortsATimestampFromTheFutureIntoToday();
    void namesTheGroups();

    void leavesThePreviewEmptyForASingleLine();
    void wrapsALongFirstLineIntoThePreview();
    void splitsSubjectAndPreviewAtTheLineBreak();
    void readsFurtherLineBreaksAsSeparators();
    void hasNoTextForAnEmptyNote();

    void listsNotesWithTheirTimestamp();
    void writesAHeadOverEachGroupAndNoneOverAnEmptyOne();
    void keepsTheHeadsOutOfReachOfTheSelection();
    void takesAndReinsertsANote();
    void dropsTheHeadWithTheLastNoteOfItsGroup();
    void groupsAgainOnTheNewReferenceTime();

    void keepsTheGracePeriodOfFiveSeconds();
    void deletesTheNoteWhenTheGracePeriodRunsOut();
    void countsTheRemainingSecondsDown();
    void keepsTheNoteWhenTheDeletionIsUndone();
    void carriesOutTheFirstDeletionWhenASecondArrives();
    void carriesOutThePendingDeletionOnFlush();

    void showsOnlyTheListPlaceholderWhenNothingIsStored();
    void asksForASelectionWhileNothingIsSelected();
    void readsTheSelectedNote();
    void movesTheSelectionToTheFollowingNote();
    void movesTheSelectionBackwardsAfterTheLastNote();
    void fallsBackToTheEmptyStateAfterTheLastNoteIsDeleted();
    void carriesOutTheDeletionWhenTheWindowCloses();
    void walksTheListWithTheArrowKeys();
    void walksPastTheGroupHeadsWithTheArrowKeys();
    void bringsTheHeadOfTheNewGroupIntoView_data();
    void bringsTheHeadOfTheNewGroupIntoView();
    void bringsBackTheHeadWhenTheDeletionIsUndone();
    void regroupsWhenTheWindowIsActivated();
    void undoesTheDeletionByKeyboard();
    void deletesWithTheDeleteKey();
    void showsTheRemainingTimeInTheMessage();
    void keepsOneMessageWhenASecondNoteIsDeleted();
    void closesWithTheStandardShortcut();
    void showsTheSearchFieldWithoutItsFunction();
    void doesNotReadTheStoreAgainWhileADeletionIsCountingDown();
    void readsTheStoreAgainWhenTheOpenWindowIsShownAgain();
    void leavesTheFocusAloneWhenTheOpenWindowIsShownAgain();
    void keepsTheListWideEnoughForThePreview();
    void keepsTheHeaderAtTheTopAndTheRestForTheNotes_data();
    void keepsTheHeaderAtTheTopAndTheRestForTheNotes();
    void alignsTheGroupHeadsWithTheEntryTimestamps_data();
    void alignsTheGroupHeadsWithTheEntryTimestamps();
    void putsTheMessageBetweenTheHeaderAndTheNotes();
    void keepsTheWindowSizeForTheNextSession();

    // Qt emits aboutToQuit once per process, so the test of the quit path has
    // to be the last one of this class.
    void carriesOutTheDeletionWhenTheApplicationQuits();

private:
    static QDateTime at(const QString &isoDateTime);
    static QLocale german();
    static Note noteWith(const QString &content);
    static Note noteWith(const QString &content, const QString &isoDateTime);

    /** The rows of `model` as "Kopf: …" and "Notiz: …", for whole-list checks. */
    static QStringList rowsOf(const NoteListModel &model);

    /** The row the note `noteIndex` sits in, as an index of `list`. */
    static QModelIndex noteRow(const QListView *list, int noteIndex);

    /** Width of ten wide characters — enough for a few words, not for many. */
    static int narrowWidth();

    /** Adds a note to the store; the first one added is the newest. */
    qint64 storedNote(const QString &content);

    /** Adds a note of a fixed age, for the tests that look at the groups. */
    qint64 storedNote(const QString &content, const QString &isoDateTime);

    /** Texts of the labels the window shows right now. */
    static QStringList visibleLabels(const QWidget &window);

    static QListView *listOf(const QWidget &window);
    static NoteListModel *modelOf(const QListView *list);
    static QAction *actionNamed(const QWidget &window, const QString &text);

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    int m_storedNotes = 0;
};

void LibraryTest::initTestCase()
{
    // The window stores its size through KSharedConfig. Without the test mode
    // that write would land in the user's denkzettelrc.
    QStandardPaths::setTestModeEnabled(true);
}

void LibraryTest::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    m_store = std::make_unique<Store>(m_dir->filePath(QStringLiteral("denkzettel.db")));
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    m_storedNotes = 0;
}

void LibraryTest::cleanup()
{
    m_store.reset();
    m_dir.reset();
}

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

void LibraryTest::usesTheWeekdayFormWithinTheTwoCalendarWeeks()
{
    // A Friday: its calendar week began on Monday, 27 July.
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    // Two days back, still this week …
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-29T09:00:00")), now, german()),
             QStringLiteral("Mi., 29. Juli"));
    // … the Monday it began on …
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-27T09:00:00")), now, german()),
             QStringLiteral("Mo., 27. Juli"));
    // … and the whole week before it, down to its Monday.
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-26T09:00:00")), now, german()),
             QStringLiteral("So., 26. Juli"));
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-20T09:00:00")), now, german()),
             QStringLiteral("Mo., 20. Juli"));
}

void LibraryTest::usesTheAbsoluteDateBeyondTheLastWeek()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    // The Sunday before last week is the switching point — under the rolling
    // seven days it had been twelve days back, now it is one day past the
    // week boundary. The year stays four-digit.
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-19T09:00:00")), now, german()),
             QStringLiteral("19.07.2026"));
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2025-07-28T09:00:00")), now, german()),
             QStringLiteral("28.07.2025"));
}

void LibraryTest::sortsIntoTheFiveGroupsWithTheFirstMatchWinning()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-31T00:01:00")), now, german()),
             library::NoteGroup::Today);
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-30T23:59:00")), now, german()),
             library::NoteGroup::Yesterday);
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-29T09:00:00")), now, german()),
             library::NoteGroup::ThisWeek);
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-27T09:00:00")), now, german()),
             library::NoteGroup::ThisWeek);
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-26T09:00:00")), now, german()),
             library::NoteGroup::LastWeek);
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-20T09:00:00")), now, german()),
             library::NoteGroup::LastWeek);
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-19T23:59:00")), now, german()),
             library::NoteGroup::Older);
}

void LibraryTest::countsTheWeekAsACalendarWeek()
{
    // The Monday probe of wireframe 3b: the day before belongs to "Gestern"
    // although it lies in the previous calendar week — the first matching
    // group wins …
    const QDateTime monday = at(QStringLiteral("2026-08-03T10:00:00"));

    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-08-02T18:00:00")), monday, german()),
             library::NoteGroup::Yesterday);

    // … and the Thursday before is three days old and still "Letzte Woche".
    // Under the rolling seven days it had read "Do., 30. Juli".
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-30T09:00:00")), monday, german()),
             library::NoteGroup::LastWeek);

    // Nothing can fall into "Diese Woche" on a Monday; the list draws no head
    // over an empty group.
    for (int hoursBack = 1; hoursBack < 24 * 14; ++hoursBack) {
        const library::NoteGroup group =
            library::noteGroup(monday.addSecs(-3600 * hoursBack), monday, german());
        QVERIFY2(group != library::NoteGroup::ThisWeek,
                 qPrintable(QStringLiteral("%1 Stunden zurück fällt in „Diese Woche").arg(hoursBack)));
    }
}

void LibraryTest::leavesTheWeekToTheLocale()
{
    // SPEC 9 names Monday because that is what the German locale says; the
    // rule itself is QLocale::firstDayOfWeek. In a locale whose week starts on
    // Sunday, that Sunday is the first day of "Diese Woche" — otherwise the
    // group would contradict the calendar the rest of the system draws.
    const QLocale american(QLocale::English, QLocale::UnitedStates);
    QCOMPARE(german().firstDayOfWeek(), Qt::Monday);
    QCOMPARE(american.firstDayOfWeek(), Qt::Sunday);

    const QDateTime wednesday = at(QStringLiteral("2026-07-29T16:00:00"));
    const QDateTime sunday = at(QStringLiteral("2026-07-26T09:00:00"));

    QCOMPARE(library::noteGroup(sunday, wednesday, german()), library::NoteGroup::LastWeek);
    QCOMPARE(library::noteGroup(sunday, wednesday, american), library::NoteGroup::ThisWeek);
}

void LibraryTest::shortensTheEntryTimestampToWhatItsGroupLeavesOpen()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    // The head carries the day, so the entry only carries the time …
    QCOMPARE(library::entryTimestamp(at(QStringLiteral("2026-07-31T14:32:00")), now, german()),
             QStringLiteral("14:32"));
    QCOMPARE(library::entryTimestamp(at(QStringLiteral("2026-07-30T21:48:00")), now, german()),
             QStringLiteral("21:48"));

    // … in the week groups the head names no day, so the entry does …
    QCOMPARE(library::entryTimestamp(at(QStringLiteral("2026-07-28T09:00:00")), now, german()),
             QStringLiteral("Di., 28. Juli"));
    QCOMPARE(library::entryTimestamp(at(QStringLiteral("2026-07-23T09:00:00")), now, german()),
             QStringLiteral("Do., 23. Juli"));

    // … and under "Älter" only the date says anything at all.
    QCOMPARE(library::entryTimestamp(at(QStringLiteral("2026-07-19T09:00:00")), now, german()),
             QStringLiteral("19.07.2026"));

    // The detail pane stands under no head and keeps the full form.
    QCOMPARE(library::relativeTimestamp(at(QStringLiteral("2026-07-31T11:05:00")), now, german()),
             QStringLiteral("Heute 11:05"));
}

void LibraryTest::sortsATimestampFromTheFutureIntoToday()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));
    const QDateTime ahead = at(QStringLiteral("2026-08-04T09:00:00"));

    // A clock jump is no group of its own — the note goes to the top …
    QCOMPARE(library::noteGroup(ahead, now, german()), library::NoteGroup::Today);

    // … but "Heute 09:00" would be a lie, so the date stays absolute in both
    // the list and the detail pane (wireframe 3b).
    QCOMPARE(library::entryTimestamp(ahead, now, german()), QStringLiteral("04.08.2026"));
    QCOMPARE(library::relativeTimestamp(ahead, now, german()), QStringLiteral("04.08.2026"));
}

void LibraryTest::namesTheGroups()
{
    QCOMPARE(library::groupTitle(library::NoteGroup::Today), QStringLiteral("Heute"));
    QCOMPARE(library::groupTitle(library::NoteGroup::Yesterday), QStringLiteral("Gestern"));
    QCOMPARE(library::groupTitle(library::NoteGroup::ThisWeek), QStringLiteral("Diese Woche"));
    QCOMPARE(library::groupTitle(library::NoteGroup::LastWeek), QStringLiteral("Letzte Woche"));
    QCOMPARE(library::groupTitle(library::NoteGroup::Older), QStringLiteral("Älter"));
}

Note LibraryTest::noteWith(const QString &content)
{
    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.content = content;
    return note;
}

Note LibraryTest::noteWith(const QString &content, const QString &isoDateTime)
{
    Note note;
    note.createdAt = at(isoDateTime);
    note.content = content;
    return note;
}

QStringList LibraryTest::rowsOf(const NoteListModel &model)
{
    QStringList rows;
    for (int row = 0; row < model.rowCount(); ++row) {
        const QModelIndex index = model.index(row);
        rows.append((index.data(NoteListModel::GroupHeaderRole).toBool() ? QStringLiteral("Kopf: ")
                                                                        : QStringLiteral("Notiz: "))
                    + index.data(Qt::DisplayRole).toString());
    }
    return rows;
}

QModelIndex LibraryTest::noteRow(const QListView *list, int noteIndex)
{
    NoteListModel *model = modelOf(list);
    const int row = model->rowOfNote(noteIndex);
    Q_ASSERT(row >= 0);

    return model->index(row);
}

int LibraryTest::narrowWidth()
{
    return QFontMetrics(QFont()).horizontalAdvance(QStringLiteral("MMMMMMMMMM"));
}

void LibraryTest::leavesThePreviewEmptyForASingleLine()
{
    // Wireframe 3b, case 3: a note of one short line has nothing to preview.
    // The entry keeps its height all the same — the delegate reserves the row,
    // not the text.
    const library::EntryText entry =
        library::subjectAndPreview(QStringLiteral("Reifen wechseln lassen"), QFont(), 10 * narrowWidth());

    QCOMPARE(entry.subject, QStringLiteral("Reifen wechseln lassen"));
    QVERIFY2(entry.preview.isEmpty(), qPrintable(entry.preview));
}

void LibraryTest::wrapsALongFirstLineIntoThePreview()
{
    // Wireframe 3b, case 2: the regular note from the capture window — one
    // thought, no line break. The subject is then the first wrapped line and
    // the preview its continuation, elided at the end of the text.
    const QString long_ = QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten, "
                                         "als Cronjob auf dem NAS einrichten und danach einmal wiederherstellen");

    const library::EntryText entry = library::subjectAndPreview(long_, QFont(), narrowWidth());

    QVERIFY2(long_.startsWith(entry.subject), qPrintable(entry.subject));
    QVERIFY2(!entry.subject.contains(QChar(0x2026)), qPrintable(entry.subject));
    QVERIFY2(entry.preview.endsWith(QChar(0x2026)), qPrintable(entry.preview));

    // The preview continues where the subject stopped, it does not start over.
    QVERIFY2(long_.mid(entry.subject.length()).trimmed().startsWith(entry.preview.chopped(1)),
             qPrintable(entry.preview));

    // The subject is never wrapped: it fits into one line of the given width.
    QVERIFY(QFontMetrics(QFont()).horizontalAdvance(entry.subject) <= narrowWidth());
}

void LibraryTest::splitsSubjectAndPreviewAtTheLineBreak()
{
    // Wireframe 3b, case 3: with a line break, the break separates.
    const library::EntryText entry =
        library::subjectAndPreview(QStringLiteral("Einkauf Samstag\nMehl, Hefe, Zitronen"),
                                   QFont(),
                                   10 * narrowWidth());

    QCOMPARE(entry.subject, QStringLiteral("Einkauf Samstag"));
    QCOMPARE(entry.preview, QStringLiteral("Mehl, Hefe, Zitronen"));
}

void LibraryTest::readsFurtherLineBreaksAsSeparators()
{
    // The preview stays on one line, so further breaks become a separator —
    // and an empty line in between is a break, not an entry of its own.
    const library::EntryText entry =
        library::subjectAndPreview(QStringLiteral("Einkauf Samstag\nMehl\n\nHefe\nZitronen"),
                                   QFont(),
                                   10 * narrowWidth());

    QCOMPARE(entry.subject, QStringLiteral("Einkauf Samstag"));
    QCOMPARE(entry.preview, QStringLiteral("Mehl · Hefe · Zitronen"));
}

void LibraryTest::hasNoTextForAnEmptyNote()
{
    QVERIFY(library::subjectAndPreview(QString(), QFont(), narrowWidth()).subject.isEmpty());

    const library::EntryText blank = library::subjectAndPreview(QStringLiteral("   \n "), QFont(), narrowWidth());
    QVERIFY(blank.subject.isEmpty());
    QVERIFY(blank.preview.isEmpty());
}

void LibraryTest::listsNotesWithTheirTimestamp()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    NoteListModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.noteCount(), 0);

    model.setNotes({noteWith(QStringLiteral("heute gedacht"), QStringLiteral("2026-07-31T14:32:00")),
                    noteWith(QStringLiteral("gestern gedacht"), QStringLiteral("2026-07-30T21:48:00"))},
                   now);

    // Two notes, two heads — the rows of the list are no longer its notes.
    QCOMPARE(model.noteCount(), 2);
    QCOMPARE(model.rowCount(), 4);

    QCOMPARE(model.index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Heute"));
    QCOMPARE(model.index(1).data(Qt::DisplayRole).toString(), QStringLiteral("heute gedacht"));

    // The head carries the day, so the entry carries the time alone.
    QCOMPARE(model.index(1).data(NoteListModel::TimestampRole).toString(), QStringLiteral("14:32"));
    QCOMPARE(model.index(3).data(NoteListModel::TimestampRole).toString(), QStringLiteral("21:48"));
    // A head has no timestamp of its own.
    QVERIFY(model.index(0).data(NoteListModel::TimestampRole).toString().isEmpty());

    QCOMPARE(model.noteAt(3).content, QStringLiteral("gestern gedacht"));
    QCOMPARE(model.noteIndexAt(3), 1);
    QCOMPARE(model.rowOfNote(1), 3);

    // A head row holds no note, and a row outside the list is a question,
    // not a crash.
    QVERIFY(model.noteAt(0).content.isEmpty());
    QCOMPARE(model.noteIndexAt(0), -1);
    QVERIFY(model.noteAt(4).content.isEmpty());
    QVERIFY(!model.index(4).data(Qt::DisplayRole).isValid());
}

void LibraryTest::writesAHeadOverEachGroupAndNoneOverAnEmptyOne()
{
    // A Monday: nothing can fall into "Diese Woche", and the group is left out
    // altogether rather than drawn empty (wireframe 3b). "Gestern" holds a
    // single note and gets its head like any other group.
    const QDateTime monday = at(QStringLiteral("2026-08-03T10:00:00"));

    NoteListModel model;
    model.setNotes({noteWith(QStringLiteral("von heute"), QStringLiteral("2026-08-03T09:00:00")),
                    noteWith(QStringLiteral("von gestern"), QStringLiteral("2026-08-02T18:00:00")),
                    noteWith(QStringLiteral("von letzter Woche"), QStringLiteral("2026-07-30T09:00:00")),
                    noteWith(QStringLiteral("von davor"), QStringLiteral("2026-07-10T09:00:00"))},
                   monday);

    QCOMPARE(rowsOf(model),
             QStringList({QStringLiteral("Kopf: Heute"),
                          QStringLiteral("Notiz: von heute"),
                          QStringLiteral("Kopf: Gestern"),
                          QStringLiteral("Notiz: von gestern"),
                          QStringLiteral("Kopf: Letzte Woche"),
                          QStringLiteral("Notiz: von letzter Woche"),
                          QStringLiteral("Kopf: Älter"),
                          QStringLiteral("Notiz: von davor")}));

    // An empty library has no group and therefore no head (wireframe 2c).
    model.setNotes({}, monday);
    QCOMPARE(model.rowCount(), 0);
}

void LibraryTest::keepsTheHeadsOutOfReachOfTheSelection()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    NoteListModel model;
    model.setNotes({noteWith(QStringLiteral("heute gedacht"), QStringLiteral("2026-07-31T14:32:00"))}, now);

    // A head is a row of the same list, but no item: neither selectable nor
    // enabled, so the view walks past it (wireframe 3b).
    QCOMPARE(model.flags(model.index(0)), Qt::NoItemFlags);
    QVERIFY(model.flags(model.index(1)).testFlag(Qt::ItemIsSelectable));
    QVERIFY(model.flags(model.index(1)).testFlag(Qt::ItemIsEnabled));
}

void LibraryTest::takesAndReinsertsANote()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    NoteListModel model;
    model.setNotes({noteWith(QStringLiteral("eins"), QStringLiteral("2026-07-31T15:00:00")),
                    noteWith(QStringLiteral("zwei"), QStringLiteral("2026-07-31T14:00:00")),
                    noteWith(QStringLiteral("drei"), QStringLiteral("2026-07-31T13:00:00"))},
                   now);

    const Note removed = model.noteAt(model.rowOfNote(1));
    QCOMPARE(removed.content, QStringLiteral("zwei"));

    model.takeNote(1);

    QCOMPARE(model.noteCount(), 2);
    QCOMPARE(model.noteAt(model.rowOfNote(1)).content, QStringLiteral("drei"));
    // One group, one head — the head stays as long as notes stand under it.
    QCOMPARE(model.rowCount(), 3);

    // Undo puts the note back where it was, not at the end.
    model.insertNote(1, removed);

    QCOMPARE(model.noteCount(), 3);
    QCOMPARE(model.noteAt(model.rowOfNote(1)).content, QStringLiteral("zwei"));
    QCOMPARE(model.noteAt(model.rowOfNote(2)).content, QStringLiteral("drei"));
}

void LibraryTest::dropsTheHeadWithTheLastNoteOfItsGroup()
{
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));

    NoteListModel model;
    model.setNotes({noteWith(QStringLiteral("von heute"), QStringLiteral("2026-07-31T14:32:00")),
                    noteWith(QStringLiteral("von gestern"), QStringLiteral("2026-07-30T21:48:00"))},
                   now);

    QSignalSpy removed(&model, &QAbstractItemModel::rowsRemoved);
    model.takeNote(1);

    // The last note of "Gestern" takes its head with it — head and note are
    // two adjacent rows and go in one removal.
    QCOMPARE(rowsOf(model), QStringList({QStringLiteral("Kopf: Heute"), QStringLiteral("Notiz: von heute")}));
    QCOMPARE(removed.size(), 1);
    QCOMPARE(removed.first().at(1).toInt(), 2);
    QCOMPARE(removed.first().at(2).toInt(), 3);

    QSignalSpy inserted(&model, &QAbstractItemModel::rowsInserted);
    model.insertNote(1, noteWith(QStringLiteral("von gestern"), QStringLiteral("2026-07-30T21:48:00")));

    // Undo brings both back, in the same place.
    QCOMPARE(rowsOf(model),
             QStringList({QStringLiteral("Kopf: Heute"),
                          QStringLiteral("Notiz: von heute"),
                          QStringLiteral("Kopf: Gestern"),
                          QStringLiteral("Notiz: von gestern")}));
    QCOMPARE(inserted.size(), 1);
    QCOMPARE(inserted.first().at(1).toInt(), 2);
    QCOMPARE(inserted.first().at(2).toInt(), 3);
}

void LibraryTest::groupsAgainOnTheNewReferenceTime()
{
    // The window that stood open over night regroups when it is looked at
    // again — no midnight timer (wireframe 3b).
    NoteListModel model;
    model.setNotes({noteWith(QStringLiteral("gestern Abend"), QStringLiteral("2026-07-31T21:48:00"))},
                   at(QStringLiteral("2026-07-31T22:00:00")));

    QCOMPARE(model.index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Heute"));
    QCOMPARE(model.index(1).data(NoteListModel::TimestampRole).toString(), QStringLiteral("21:48"));

    model.regroup(at(QStringLiteral("2026-08-01T09:00:00")));

    QCOMPARE(model.index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));
    QCOMPARE(model.index(1).data(NoteListModel::TimestampRole).toString(), QStringLiteral("21:48"));
    QCOMPARE(model.noteCount(), 1);
}

qint64 LibraryTest::storedNote(const QString &content)
{
    Note note = noteWith(content);
    // Each note a second older than the one before it, so the list order is
    // the order the notes were added in.
    note.createdAt = note.createdAt.addSecs(-m_storedNotes++);

    const std::optional<qint64> id = m_store->addNote(note);
    Q_ASSERT(id.has_value());
    return *id;
}

qint64 LibraryTest::storedNote(const QString &content, const QString &isoDateTime)
{
    const std::optional<qint64> id = m_store->addNote(noteWith(content, isoDateTime));
    Q_ASSERT(id.has_value());
    return *id;
}

QStringList LibraryTest::visibleLabels(const QWidget &window)
{
    QStringList texts;
    const QList<QLabel *> labels = window.findChildren<QLabel *>();
    for (const QLabel *label : labels) {
        if (label->isVisible() && !label->text().isEmpty()) {
            texts.append(label->text());
        }
    }
    return texts;
}

QListView *LibraryTest::listOf(const QWidget &window)
{
    QListView *list = window.findChild<QListView *>();
    Q_ASSERT(list);
    return list;
}

NoteListModel *LibraryTest::modelOf(const QListView *list)
{
    auto *model = qobject_cast<NoteListModel *>(list->model());
    Q_ASSERT(model);
    return model;
}

QAction *LibraryTest::actionNamed(const QWidget &window, const QString &text)
{
    const QList<QAction *> actions = window.actions();
    for (QAction *action : actions) {
        if (action->text() == text) {
            return action;
        }
    }
    return nullptr;
}

void LibraryTest::keepsTheGracePeriodOfFiveSeconds()
{
    // SPEC 9 names the period; the tests below shorten it to keep the suite
    // quick, so the value itself needs a test of its own.
    QCOMPARE(PendingDeletion::DefaultGracePeriodSeconds, 5);
}

void LibraryTest::deletesTheNoteWhenTheGracePeriodRunsOut()
{
    const qint64 id = storedNote(QStringLiteral("geht weg"));
    PendingDeletion deletion(m_store.get(), 1);
    QSignalSpy committed(&deletion, &PendingDeletion::committed);

    deletion.request(id);

    // Still there while the period runs.
    QVERIFY(deletion.isPending());
    QVERIFY(m_store->note(id).has_value());

    QVERIFY(committed.wait(3000));
    QCOMPARE(committed.first().first().toLongLong(), id);
    QVERIFY(!deletion.isPending());
    QVERIFY(!m_store->note(id).has_value());
}

void LibraryTest::countsTheRemainingSecondsDown()
{
    PendingDeletion deletion(m_store.get(), 2);
    QSignalSpy remaining(&deletion, &PendingDeletion::remainingChanged);

    deletion.request(storedNote(QStringLiteral("zählt herunter")));

    // The message shows the full period right away, then the last second.
    QCOMPARE(remaining.size(), 1);
    QCOMPARE(remaining.first().first().toInt(), 2);

    QVERIFY(QTest::qWaitFor([&remaining] { return remaining.size() == 2; }, 3000));
    QCOMPARE(remaining.last().first().toInt(), 1);
}

void LibraryTest::keepsTheNoteWhenTheDeletionIsUndone()
{
    const qint64 id = storedNote(QStringLiteral("bleibt doch"));
    PendingDeletion deletion(m_store.get(), 1);
    QSignalSpy reverted(&deletion, &PendingDeletion::reverted);
    QSignalSpy committed(&deletion, &PendingDeletion::committed);

    deletion.request(id);
    deletion.undo();

    QCOMPARE(reverted.size(), 1);
    QCOMPARE(reverted.first().first().toLongLong(), id);
    QVERIFY(!deletion.isPending());
    QVERIFY(m_store->note(id).has_value());

    // The timer is off: the note survives the period it would have run for.
    QTest::qWait(1500);
    QCOMPARE(committed.size(), 0);
    QVERIFY(m_store->note(id).has_value());

    // Nothing pending, nothing to undo.
    deletion.undo();
    QCOMPARE(reverted.size(), 1);
}

void LibraryTest::carriesOutTheFirstDeletionWhenASecondArrives()
{
    const qint64 first = storedNote(QStringLiteral("zuerst gelöscht"));
    const qint64 second = storedNote(QStringLiteral("danach gelöscht"));
    PendingDeletion deletion(m_store.get(), 5);
    QSignalSpy committed(&deletion, &PendingDeletion::committed);
    QSignalSpy remaining(&deletion, &PendingDeletion::remainingChanged);

    // The window keeps its message standing as long as something is pending,
    // so the second request has to have taken over by the time the first
    // deletion is reported.
    connect(&deletion, &PendingDeletion::committed, this, [&deletion] {
        QVERIFY(deletion.isPending());
    });

    deletion.request(first);
    deletion.request(second);

    // The first deletion happened at once, the period started over for the
    // second one — one message, never a stack.
    QCOMPARE(committed.size(), 1);
    QCOMPARE(committed.first().first().toLongLong(), first);
    QVERIFY(!m_store->note(first).has_value());

    QCOMPARE(remaining.size(), 2);
    QCOMPARE(remaining.last().first().toInt(), 5);
    QVERIFY(deletion.isPending());
    QVERIFY(m_store->note(second).has_value());
}

void LibraryTest::carriesOutThePendingDeletionOnFlush()
{
    const qint64 id = storedNote(QStringLiteral("Fenster wird geschlossen"));
    PendingDeletion deletion(m_store.get(), 5);
    QSignalSpy committed(&deletion, &PendingDeletion::committed);

    deletion.request(id);
    deletion.flush();

    QCOMPARE(committed.size(), 1);
    QVERIFY(!m_store->note(id).has_value());

    // Flushing with nothing pending is a no-op, not a second deletion.
    deletion.flush();
    QCOMPARE(committed.size(), 1);
}

void LibraryTest::showsOnlyTheListPlaceholderWhenNothingIsStored()
{
    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QStringList visible = visibleLabels(window);

    QVERIFY2(visible.contains(QStringLiteral("Noch keine Notizen")), qPrintable(visible.join(QLatin1Char('|'))));
    QVERIFY(visible.contains(QStringLiteral("Mit Meta+N einen Gedanken festhalten.")));

    // Wireframe 2c: an empty window must not say the same thing twice.
    QVERIFY2(!visible.contains(QStringLiteral("Keine Notiz ausgewählt")),
             qPrintable(visible.join(QLatin1Char('|'))));
    QVERIFY(!listOf(window)->isVisible());
}

void LibraryTest::asksForASelectionWhileNothingIsSelected()
{
    storedNote(QStringLiteral("etwas Gedachtes"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QStringList visible = visibleLabels(window);

    QVERIFY2(visible.contains(QStringLiteral("Keine Notiz ausgewählt")), qPrintable(visible.join(QLatin1Char('|'))));
    QVERIFY(visible.contains(QStringLiteral("Zum Lesen links eine Notiz auswählen.")));
    QVERIFY(!visible.contains(QStringLiteral("Noch keine Notizen")));
    QVERIFY(listOf(window)->isVisible());
}

void LibraryTest::readsTheSelectedNote()
{
    storedNote(QStringLiteral("die neuere Notiz"));
    storedNote(QStringLiteral("die ältere Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 1));

    auto *reader = window.findChild<QTextBrowser *>();
    QVERIFY(reader);
    QVERIFY(reader->isVisible());
    QCOMPARE(reader->toPlainText(), QStringLiteral("die ältere Notiz"));
    QVERIFY(!visibleLabels(window).contains(QStringLiteral("Keine Notiz ausgewählt")));
}

void LibraryTest::movesTheSelectionToTheFollowingNote()
{
    storedNote(QStringLiteral("eins"));
    storedNote(QStringLiteral("zwei"));
    storedNote(QStringLiteral("drei"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 1));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    QCOMPARE(modelOf(list)->noteCount(), 2);
    QCOMPARE(list->currentIndex(), noteRow(list, 1));
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("drei"));
}

void LibraryTest::movesTheSelectionBackwardsAfterTheLastNote()
{
    storedNote(QStringLiteral("eins"));
    storedNote(QStringLiteral("zwei"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 1));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    // No following entry, so the preceding one takes the selection.
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("eins"));
}

void LibraryTest::fallsBackToTheEmptyStateAfterTheLastNoteIsDeleted()
{
    storedNote(QStringLiteral("die einzige Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    // The last note takes its group head with it — the empty library shows no
    // heading over nothing (wireframe 3b).
    QCOMPARE(list->model()->rowCount(), 0);

    const QStringList visible = visibleLabels(window);
    QVERIFY2(visible.contains(QStringLiteral("Noch keine Notizen")), qPrintable(visible.join(QLatin1Char('|'))));
    QVERIFY(!visible.contains(QStringLiteral("Keine Notiz ausgewählt")));
}

void LibraryTest::carriesOutTheDeletionWhenTheWindowCloses()
{
    const qint64 id = storedNote(QStringLiteral("beim Schließen weg"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    // The grace period is still running, the note is still in the store.
    QVERIFY(m_store->note(id).has_value());

    window.close();

    QVERIFY(!m_store->note(id).has_value());
}

void LibraryTest::walksTheListWithTheArrowKeys()
{
    storedNote(QStringLiteral("die neuere Notiz"));
    storedNote(QStringLiteral("die ältere Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    QVERIFY(!list->currentIndex().isValid());

    // The first key press has to land on the first entry, not skip past it —
    // and not on the group head above it either.
    QTest::keyClick(list, Qt::Key_Down);
    QCOMPARE(list->currentIndex(), noteRow(list, 0));

    auto *reader = window.findChild<QTextBrowser *>();
    QVERIFY(reader);
    QCOMPARE(reader->toPlainText(), QStringLiteral("die neuere Notiz"));

    QTest::keyClick(list, Qt::Key_Down);
    QCOMPARE(list->currentIndex(), noteRow(list, 1));
    QCOMPARE(reader->toPlainText(), QStringLiteral("die ältere Notiz"));

    QTest::keyClick(list, Qt::Key_Up);
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
}

void LibraryTest::walksPastTheGroupHeadsWithTheArrowKeys()
{
    // One note per group, so every step of the way crosses a group boundary.
    storedNote(QStringLiteral("von heute"), QStringLiteral("2026-07-31T14:32:00"));
    storedNote(QStringLiteral("von gestern"), QStringLiteral("2026-07-30T21:48:00"));
    storedNote(QStringLiteral("von dieser Woche"), QStringLiteral("2026-07-28T09:00:00"));
    storedNote(QStringLiteral("von letzter Woche"), QStringLiteral("2026-07-23T09:00:00"));

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    QCOMPARE(list->model()->rowCount(), 8);

    // Down the whole list: the selection walks from note to note without ever
    // stopping on a head, although a head sits before each of them.
    QStringList walked;
    for (int step = 0; step < 4; ++step) {
        QTest::keyClick(list, Qt::Key_Down);
        QVERIFY2(!list->currentIndex().data(NoteListModel::GroupHeaderRole).toBool(),
                 qPrintable(QStringLiteral("Zeile %1 ist ein Kopf").arg(list->currentIndex().row())));
        walked.append(list->currentIndex().data(Qt::DisplayRole).toString());
    }

    QCOMPARE(walked,
             QStringList({QStringLiteral("von heute"),
                          QStringLiteral("von gestern"),
                          QStringLiteral("von dieser Woche"),
                          QStringLiteral("von letzter Woche")}));

    // At the end of the list the key does nothing; it does not fall onto a
    // head either.
    QTest::keyClick(list, Qt::Key_Down);
    QCOMPARE(list->currentIndex(), noteRow(list, 3));

    // And the same way back up.
    for (int step = 0; step < 3; ++step) {
        QTest::keyClick(list, Qt::Key_Up);
        QVERIFY(!list->currentIndex().data(NoteListModel::GroupHeaderRole).toBool());
    }
    QCOMPARE(list->currentIndex(), noteRow(list, 0));

    // Neither can the mouse pick a head: it is not an item of this list.
    QVERIFY(!modelOf(list)->flags(modelOf(list)->index(0)).testFlag(Qt::ItemIsSelectable));

    // Entf on a head deletes nothing — it never holds the selection, and the
    // action asks the model for a note rather than for a row.
    QCOMPARE(modelOf(list)->noteIndexAt(0), -1);
}

void LibraryTest::bringsTheHeadOfTheNewGroupIntoView_data()
{
    // Head and entry together need room, so the assurance has a condition:
    // the list has to be at least as tall as both of them. The second row asks
    // for a window flatter than the layout allows — it settles at its minimum
    // of 166 px, which leaves 125 px of list for a head of 47 px and an entry
    // of 65 px. The condition therefore holds at every size the window can
    // take, and nothing has to be added to the SPEC for it.
    QTest::addColumn<QSize>("windowSize");

    QTest::newRow("900x600") << QSize(900, 600);
    QTest::newRow("so flach wie möglich") << QSize(900, 150);
}

void LibraryTest::bringsTheHeadOfTheNewGroupIntoView()
{
    QFETCH(QSize, windowSize);

    // Two full groups, so that the boundary lies outside the first screenful
    // and the list has to be scrolled at all.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(windowSize);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    QCOMPARE(modelOf(list)->noteCount(), 16);
    // A list that fits into the window would let this pass without scrolling.
    QVERIFY2(list->verticalScrollBar()->maximum() > 0,
             qPrintable(QStringLiteral("Liste passt ganz ins Bild, der Fall tritt nicht ein")));

    // From the end of the list upwards to the first note of "Gestern". Going
    // up is what puts the entry against the upper edge — and its head just
    // outside it, unless the window pulls the head along.
    list->setCurrentIndex(noteRow(list, 15));
    for (int step = 0; step < 7; ++step) {
        QTest::keyClick(list, Qt::Key_Up);
    }

    const QModelIndex selected = noteRow(list, 8);
    QCOMPARE(list->currentIndex(), selected);

    const QModelIndex head = modelOf(list)->index(selected.row() - 1);
    QVERIFY(head.data(NoteListModel::GroupHeaderRole).toBool());
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));

    // The head of the new group is in the picture — the selection never stands
    // without its heading (wireframe 3b, case 4) …
    QVERIFY2(list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Kopf bei y=%1, Viewport %2 px hoch")
                            .arg(list->visualRect(head).y())
                            .arg(list->viewport()->height())));

    // … and the selected entry is whole, not cut off at an edge (PO decision
    // of 01.08.2026).
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(selected)),
             qPrintable(QStringLiteral("Auswahl y=%1 h=%2, Viewport %3 px hoch")
                            .arg(list->visualRect(selected).y())
                            .arg(list->visualRect(selected).height())
                            .arg(list->viewport()->height())));

    // The heads scroll with the list rather than sticking to the top: the head
    // of "Heute" has left the picture at this point.
    const QRect firstHead = list->visualRect(modelOf(list)->index(0));
    QVERIFY2(!list->viewport()->rect().intersects(firstHead),
             qPrintable(QStringLiteral("Kopf „Heute\" klebt bei y=%1").arg(firstHead.y())));
}

void LibraryTest::undoesTheDeletionByKeyboard()
{
    const qint64 id = storedNote(QStringLiteral("kommt zurück"));
    storedNote(QStringLiteral("bleibt sowieso"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();
    QCOMPARE(modelOf(list)->noteCount(), 1);

    QTest::keyClick(&window, Qt::Key_Z, Qt::ControlModifier);

    // The note is back in its old place and selected again.
    QCOMPARE(modelOf(list)->noteCount(), 2);
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("kommt zurück"));
    QVERIFY(m_store->note(id).has_value());
}

void LibraryTest::bringsBackTheHeadWhenTheDeletionIsUndone()
{
    // Two groups of one note each: deleting either of them empties its group.
    storedNote(QStringLiteral("von heute"), QStringLiteral("2026-07-31T14:32:00"));
    storedNote(QStringLiteral("von gestern"), QStringLiteral("2026-07-30T21:48:00"));

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 1));
    QCOMPARE(rowsOf(*modelOf(list)),
             QStringList({QStringLiteral("Kopf: Heute"),
                          QStringLiteral("Notiz: von heute"),
                          QStringLiteral("Kopf: Gestern"),
                          QStringLiteral("Notiz: von gestern")}));

    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    // The head of "Gestern" goes with its last note …
    QCOMPARE(rowsOf(*modelOf(list)),
             QStringList({QStringLiteral("Kopf: Heute"), QStringLiteral("Notiz: von heute")}));
    // … and the selection falls back to the preceding note, never onto a head.
    QCOMPARE(list->currentIndex(), noteRow(list, 0));

    QTest::keyClick(&window, Qt::Key_Z, Qt::ControlModifier);

    // The undo brings note and head back in the same place.
    QCOMPARE(rowsOf(*modelOf(list)),
             QStringList({QStringLiteral("Kopf: Heute"),
                          QStringLiteral("Notiz: von heute"),
                          QStringLiteral("Kopf: Gestern"),
                          QStringLiteral("Notiz: von gestern")}));
    QCOMPARE(list->currentIndex(), noteRow(list, 1));
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("von gestern"));
}

void LibraryTest::regroupsWhenTheWindowIsActivated()
{
    storedNote(QStringLiteral("gestern Abend gedacht"), QStringLiteral("2026-07-31T21:48:00"));

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T22:00:00")));
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Heute"));

    // The window stood open past midnight. Setting the reference time alone
    // changes nothing — there is no timer that regroups on its own
    // (wireframe 3b).
    window.setReferenceTime(at(QStringLiteral("2026-08-01T09:00:00")));
    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Heute"));

    // Looking at the window again does: the activation regroups the list, and
    // the note keeps its selection across the regrouping.
    QWidget elsewhere;
    elsewhere.show();
    elsewhere.activateWindow();
    QTRY_VERIFY(!window.isActiveWindow());

    window.activateWindow();
    QTRY_VERIFY(window.isActiveWindow());

    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("gestern Abend gedacht"));
}

void LibraryTest::deletesWithTheDeleteKey()
{
    const qint64 id = storedNote(QStringLiteral("per Taste gelöscht"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);

    QCOMPARE(modelOf(list)->noteCount(), 0);

    // The store still has it: the key starts the grace period, it does not
    // skip it.
    QVERIFY(m_store->note(id).has_value());
}

void LibraryTest::showsTheRemainingTimeInTheMessage()
{
    storedNote(QStringLiteral("mit Frist gelöscht"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *message = window.findChild<KMessageWidget *>();
    QVERIFY(message);
    QVERIFY(!message->isVisible());

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);

    // The window uses the period SPEC 9 names, and the message says how much
    // of it is left.
    QCOMPARE(message->text(), QStringLiteral("Notiz gelöscht — noch 5 s"));
    QTRY_VERIFY(message->isVisible());
    QCOMPARE(message->messageType(), KMessageWidget::Warning);
    QVERIFY(!message->isCloseButtonVisible());
}

void LibraryTest::keepsOneMessageWhenASecondNoteIsDeleted()
{
    const qint64 first = storedNote(QStringLiteral("zuerst gelöscht"));
    storedNote(QStringLiteral("danach gelöscht"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *message = window.findChild<KMessageWidget *>();
    QVERIFY(message);
    QListView *list = listOf(window);

    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);
    QTest::keyClick(list, Qt::Key_Delete);

    // One message, counting from the start again — never a stack of them.
    QCOMPARE(window.findChildren<KMessageWidget *>().size(), 1);
    QCOMPARE(message->text(), QStringLiteral("Notiz gelöscht — noch 5 s"));
    QVERIFY(message->isVisible());

    // The first deletion was carried out on the spot.
    QVERIFY(!m_store->note(first).has_value());

    // Undo brings back the second note, the one still counting down.
    QTest::keyClick(&window, Qt::Key_Z, Qt::ControlModifier);
    QCOMPARE(modelOf(list)->noteCount(), 1);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("danach gelöscht"));
}

void LibraryTest::closesWithTheStandardShortcut()
{
    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QTest::keySequence(&window, KStandardShortcut::close().first());

    QVERIFY(!window.isVisible());
}

void LibraryTest::showsTheSearchFieldWithoutItsFunction()
{
    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *search = window.findChild<QLineEdit *>();
    QVERIFY(search);
    QVERIFY(search->isVisible());
    QVERIFY(!search->isEnabled());
    QCOMPARE(search->placeholderText(), QStringLiteral("Volltextsuche …"));

    // The disabled field cannot show a tooltip of its own, so its wrapper
    // carries the one the story asks for.
    QCOMPARE(search->parentWidget()->toolTip(),
             QStringLiteral("Die Volltextsuche steht noch nicht zur Verfügung."));
}

void LibraryTest::doesNotReadTheStoreAgainWhileADeletionIsCountingDown()
{
    storedNote(QStringLiteral("wird gelöscht"));
    storedNote(QStringLiteral("bleibt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);
    QCOMPARE(modelOf(list)->noteCount(), 1);

    // ShowLibrary() on an open window brings it to the front. Reading the
    // store again would fetch the note that is still counting down back into
    // the list.
    window.showLibrary();

    QCOMPARE(modelOf(list)->noteCount(), 1);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("bleibt"));
}

void LibraryTest::readsTheStoreAgainWhenTheOpenWindowIsShownAgain()
{
    storedNote(QStringLiteral("die neuere Notiz"));
    storedNote(QStringLiteral("die ältere Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 1));

    // Meta+N while the library is open: the note captured meanwhile is the
    // newest one and takes the top row, so the selected note moves down.
    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    window.showLibrary();

    QCOMPARE(modelOf(list)->noteCount(), 3);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("gerade festgehalten"));

    // The selection follows the note, not the row it used to sit in.
    QCOMPARE(list->currentIndex(), noteRow(list, 2));
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("die ältere Notiz"));

    auto *reader = window.findChild<QTextBrowser *>();
    QVERIFY(reader);
    QCOMPARE(reader->toPlainText(), QStringLiteral("die ältere Notiz"));
}

void LibraryTest::leavesTheFocusAloneWhenTheOpenWindowIsShownAgain()
{
    storedNote(QStringLiteral("wird gelesen"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    // The window that was not on screen puts the focus into the list.
    QListView *list = listOf(window);
    QCOMPARE(window.focusWidget(), list);

    list->setCurrentIndex(noteRow(list, 0));
    auto *reader = window.findChild<QTextBrowser *>();
    QVERIFY(reader);
    reader->setFocus();
    QCOMPARE(window.focusWidget(), reader);

    // The tray brings the open window to the front; the reading pane keeps the
    // focus the user put there.
    window.showLibrary();

    QCOMPARE(window.focusWidget(), reader);
}

void LibraryTest::keepsTheListWideEnoughForThePreview()
{
    LibraryWindow window(m_store.get());

    auto *splitter = window.findChild<QSplitter *>();
    QVERIFY(splitter);

    // Two lines of preview need room; the splitter must not squeeze the list
    // down to the width of its placeholder text.
    QCOMPARE(splitter->widget(0)->minimumWidth(), 220);
}

void LibraryTest::keepsTheHeaderAtTheTopAndTheRestForTheNotes_data()
{
    // Two sizes, because a missing stretch factor only shows in the height the
    // layout has left over: at the sizeHint of the window there is none.
    QTest::addColumn<QSize>("windowSize");

    QTest::newRow("900x600") << QSize(900, 600);
    QTest::newRow("1200x800") << QSize(1200, 800);
}

void LibraryTest::keepsTheHeaderAtTheTopAndTheRestForTheNotes()
{
    QFETCH(QSize, windowSize);

    storedNote(QStringLiteral("die Liste braucht die Resthöhe"));

    LibraryWindow window(m_store.get());
    window.resize(windowSize);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QCOMPARE(window.size(), windowSize);

    auto *search = window.findChild<QLineEdit *>();
    QVERIFY(search);
    // The search field sits in the header through its tooltip wrapper.
    QWidget *header = search->parentWidget()->parentWidget();
    QVERIFY(header);
    auto *splitter = window.findChild<QSplitter *>();
    QVERIFY(splitter);

    // Wireframe 2b draws the header as a narrow bar along the top edge …
    QCOMPARE(header->mapTo(&window, QPoint()).y(), 0);
    QVERIFY2(header->height() <= header->sizeHint().height(),
             qPrintable(QStringLiteral("Kopfzeile %1 px hoch, sizeHint %2 px")
                            .arg(header->height())
                            .arg(header->sizeHint().height())));

    // … list and reading pane begin right below it …
    QCOMPARE(splitter->mapTo(&window, QPoint()).y(), header->height());

    // … and the rest of the window is theirs. Anything less means the surplus
    // height went into empty space (customer finding of 01.08.2026).
    QVERIFY2(splitter->height() >= window.height() * 3 / 4,
             qPrintable(QStringLiteral("Splitter %1 px hoch in einem %2 px hohen Fenster")
                            .arg(splitter->height())
                            .arg(window.height())));

    // Down to the bottom edge, at that: the check above alone would let a strip
    // of empty space at the lower end pass (UI review of 01.08.2026).
    QCOMPARE(splitter->mapTo(&window, QPoint()).y() + splitter->height(), window.height());

    // The field belongs into the header, not into the middle of the window.
    QVERIFY2(search->mapTo(&window, QPoint()).y() < 40,
             qPrintable(QStringLiteral("Suchfeld bei y=%1").arg(search->mapTo(&window, QPoint()).y())));

    // The other axis, same class of mistake: wireframe 2b gives the list a
    // fixed width and the reading pane the rest, so the same number has to come
    // out at both window widths.
    QCOMPARE(splitter->widget(0)->width(), 300);
}

void LibraryTest::alignsTheGroupHeadsWithTheEntryTimestamps_data()
{
    // The measurements of wireframe 3a are checked at two window sizes, like
    // the room split of 2b: a number that only holds at one width holds by
    // accident.
    QTest::addColumn<QSize>("windowSize");

    QTest::newRow("900x600") << QSize(900, 600);
    QTest::newRow("1200x800") << QSize(1200, 800);
}

void LibraryTest::alignsTheGroupHeadsWithTheEntryTimestamps()
{
    QFETCH(QSize, windowSize);

    storedNote(QStringLiteral("eine Notiz mit zwei Zeilen\nund einer Vorschau darunter"),
               QStringLiteral("2026-07-31T14:32:00"));
    storedNote(QStringLiteral("ein Einzeiler"), QStringLiteral("2026-07-31T09:41:00"));
    storedNote(QStringLiteral("von gestern"), QStringLiteral("2026-07-30T21:48:00"));

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(windowSize);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    NoteListModel *model = modelOf(list);

    const QModelIndex firstHead = model->index(0);
    const QModelIndex secondHead = model->index(3);
    QVERIFY(firstHead.data(NoteListModel::GroupHeaderRole).toBool());
    QVERIFY(secondHead.data(NoteListModel::GroupHeaderRole).toBool());

    // The head text starts where the timestamp of an entry starts, 12 px from
    // the left edge of the list (wireframe 3a).
    //
    // No test can see where a delegate put its ink without measuring pixels,
    // so the alignment is not held by this check but by the delegate itself:
    // head, timestamp, subject and preview are drawn by one single function,
    // and the edge is worked out in one single place. What is checked here is
    // the number that place uses.
    const QRect headRect = list->visualRect(firstHead);
    const QRect entryRect = list->visualRect(noteRow(list, 0));
    QCOMPARE(NoteListDelegate::textLeft(headRect), NoteListDelegate::textLeft(entryRect));
    QCOMPARE(NoteListDelegate::textLeft(entryRect) - list->viewport()->rect().x(), 12);

    // The first head sits close under the upper edge, every following one
    // keeps the larger distance that separates it from the group above
    // (wireframe 3a: 6 px against 14 px above the text).
    QVERIFY2(headRect.height() < list->visualRect(secondHead).height(),
             qPrintable(QStringLiteral("erster Kopf %1 px, zweiter %2 px")
                            .arg(headRect.height())
                            .arg(list->visualRect(secondHead).height())));
    QCOMPARE(list->visualRect(secondHead).height() - headRect.height(), 8);

    // Every entry is as tall as every other, the single-line note included:
    // its preview row stays empty rather than shrinking the entry.
    const int entryHeight = entryRect.height();
    for (int note = 1; note < model->noteCount(); ++note) {
        QCOMPARE(list->visualRect(noteRow(list, note)).height(), entryHeight);
    }

    // The heads take room of their own — they do not eat into the entries.
    QVERIFY2(entryHeight > headRect.height(), qPrintable(QStringLiteral("Eintrag %1 px").arg(entryHeight)));
}

void LibraryTest::putsTheMessageBetweenTheHeaderAndTheNotes()
{
    storedNote(QStringLiteral("mit Frist gelöscht"));

    LibraryWindow window(m_store.get());
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);

    auto *message = window.findChild<KMessageWidget *>();
    QVERIFY(message);
    // animatedShow() grows the widget out of nothing, so the geometry only
    // settles once that animation has run its course.
    QTRY_VERIFY(message->isVisible() && !message->isShowAnimationRunning());

    auto *search = window.findChild<QLineEdit *>();
    QVERIFY(search);
    QWidget *header = search->parentWidget()->parentWidget();
    auto *splitter = window.findChild<QSplitter *>();
    QVERIFY(splitter);

    // Wireframe 2c: the message is a band between header and content, and it
    // takes its room from neither of them being pushed out of place.
    QCOMPARE(message->mapTo(&window, QPoint()).y(), header->height());
    QCOMPARE(splitter->mapTo(&window, QPoint()).y(), header->height() + message->height());
    QVERIFY2(splitter->height() >= window.height() * 3 / 4,
             qPrintable(QStringLiteral("Splitter %1 px hoch in einem %2 px hohen Fenster")
                            .arg(splitter->height())
                            .arg(window.height())));
    QCOMPARE(splitter->mapTo(&window, QPoint()).y() + splitter->height(), window.height());

    // Wireframe 2b draws text and „Rückgängig" side by side in one row. With
    // word wrap KMessageWidget puts the button underneath instead and the band
    // grows by half; the rows are compared rather than the pixel heights,
    // because those belong to the theme.
    QLabel *text = nullptr;
    for (QLabel *label : message->findChildren<QLabel *>()) {
        if (label->text() == message->text()) {
            text = label;
        }
    }
    QToolButton *undo = nullptr;
    for (QToolButton *button : message->findChildren<QToolButton *>()) {
        if (button->text() == QStringLiteral("Rückgängig")) {
            undo = button;
        }
    }
    QVERIFY(text);
    QVERIFY(undo);

    const int textTop = text->mapTo(message, QPoint()).y();
    const int undoTop = undo->mapTo(message, QPoint()).y();
    QVERIFY2(textTop < undoTop + undo->height() && undoTop < textTop + text->height(),
             qPrintable(QStringLiteral("Text y=%1 h=%2, Knopf y=%3 h=%4 — nicht in einer Zeile")
                            .arg(textTop)
                            .arg(text->height())
                            .arg(undoTop)
                            .arg(undo->height())));
}

void LibraryTest::keepsTheWindowSizeForTheNextSession()
{
    const QSize chosen(700, 480);

    {
        LibraryWindow window(m_store.get());
        window.showLibrary();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        window.resize(chosen);
        window.close();
    }

    LibraryWindow reopened(m_store.get());

    QCOMPARE(reopened.size(), chosen);
}

void LibraryTest::carriesOutTheDeletionWhenTheApplicationQuits()
{
    const qint64 id = storedNote(QStringLiteral("beim Beenden weg"));
    const qint64 kept = storedNote(QStringLiteral("bleibt sowieso"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    // The grace period is still running, the note is still in the store.
    QVERIFY(m_store->note(id).has_value());

    // Qt 6 closes all windows before quitting (QCoreApplication::quit docs),
    // so the close event is what carries the pending deletion out on D-Bus
    // Quit() (SPEC 2.3). This test pins that Qt behaviour: should a later Qt
    // stop closing windows on quit, SPEC 9 ("deleted for good once the
    // period is over") would silently break — and this test catches it.
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    qApp->exec();

    QVERIFY(!m_store->note(id).has_value());

    // The store still answers after the loop has ended — without this, the
    // check above would pass on a store that has stopped reading.
    QVERIFY(m_store->note(kept).has_value());
}

QTEST_MAIN(LibraryTest)

#include "librarytest.moc"
