#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"
#include "ui/pendingdeletion.h"
#include "ui/timestampformat.h"

#include <KLocalizedString>

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QApplication>
#include <QLocale>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QTimer>

#include <memory>

// Why eight QFETCH lines below carry NOLINTNEXTLINE(misc-const-correctness),
// the one reason for all of them (issue #76): QFETCH is a macro that declares
// the variable itself -
//
//     #define QFETCH(Type, name) Type name = *static_cast<Type *>(...)
//
// so `QFETCH(QSize, windowSize)` expands to `QSize windowSize = ...`. There is
// no place to put a const short of giving up QFETCH, and QFETCH is the QTest
// idiom for data-driven tests. Switching misc-const-correctness off for tests/
// was considered and dropped: it would have dropped the const findings this
// issue heals as well.

namespace
{
/**
 * Answers away every guard dialog that turns up while it lives, and remembers
 * whether one did.
 *
 * For the paths on which no dialog may appear. Without it a wrongly raised
 * dialog would hang the test inside exec() instead of failing it, and a suite
 * that hangs is worse than one that goes red — measured on 02.08.2026, when
 * the counter-check of the guard ran into the timeout instead of into an
 * assertion.
 */
class DialogWatch
{
public:
    DialogWatch()
    {
        QObject::connect(&m_timer, &QTimer::timeout, &m_timer, [this] {
            // Any modal dialog, not a QMessageBox: since issue #66 the guard is
            // a KMessageDialog, and it is a plain QDialog.
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialog) {
                m_appeared = true;
                dialog->reject();
            }
        });
        m_timer.start(10);
    }

    bool appeared() const
    {
        return m_appeared;
    }

private:
    QTimer m_timer;
    bool m_appeared = false;
};

/**
 * The selectable row the lower edge cuts through, or -1 if none does.
 *
 * Looked up instead of written down: which row is clipped depends on the roll
 * value, and a test that fixes one can land on the very value at which the
 * fault stays below its threshold (issue #71, reproduction of 05.08.2026).
 * Only the lower edge ever cuts a row — under ScrollPerItem the list always
 * sets down flush at the top.
 */
int bottomClippedRow(const QListView *list)
{
    for (int row = 0; row < list->model()->rowCount(); ++row) {
        const QModelIndex index = list->model()->index(row, 0);
        if (!index.flags().testFlag(Qt::ItemIsSelectable)) {
            continue;
        }
        const QRect rect = list->visualRect(index);
        if (rect.top() < list->viewport()->height() && rect.bottom() > list->viewport()->height()) {
            return row;
        }
    }
    return -1;
}

/**
 * The rows the selection covers right now, as "3" or "none" or "3,4".
 *
 * Kept as text so a failure names what was marked instead of only that a count
 * came out wrong — after the fault of #71 the answer is sometimes "none".
 */
QString selectedRows(const QListView *list)
{
    const QModelIndexList selected = list->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        return QStringLiteral("none");
    }
    QStringList rows;
    for (const QModelIndex &index : selected) {
        rows << QString::number(index.row());
    }
    return rows.join(QLatin1Char(','));
}
}

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

    void switchesOnTheCalendarDayNotAfterTwentyFourHours();
    void keepsTheYearFourDigitsAcrossYears();

    void sortsIntoTheFiveGroupsWithTheFirstMatchWinning();
    void countsTheWeekAsACalendarWeek();
    void leavesTheWeekToTheLocale();
    void showsTheSameDateAndTimeFormInEveryGroup();
    void sortsATimestampFromTheFutureIntoToday();

    void listsNotesWithTheirTimestamp();
    void takesAndReinsertsANote();
    void dropsTheHeadWithTheLastNoteOfItsGroup();
    void groupsAgainOnTheNewReferenceTime();

    void keepsTheGracePeriodOfFiveSeconds();
    void deletesTheNoteWhenTheGracePeriodRunsOut();
    void keepsTheNoteWhenTheDeletionIsUndone();
    void carriesOutTheFirstDeletionWhenASecondArrives();
    void carriesOutThePendingDeletionOnFlush();

    void carriesOutTheDeletionWhenTheWindowCloses();
    void bringsTheHeadOfTheNewGroupIntoView_data();
    void bringsTheHeadOfTheNewGroupIntoView();
    void bringsTheHeadAlongForANoteInTheMiddleOfASmallGroup();
    void leavesTheHeadOutsideWhereItCannotFitWithTheSelection();
    void staysPutWhileTheSelectionMovesWithinItsGroup();
    void bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready();
    void leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked();
    void keepsTheHeadFetchAfterAClickThatSelectedNothing();
    void bringsTheHeadAlongWhenTheSelectionReachesTheFirstNoteOfItsGroup();
    void leavesThePictureWhereItIsWhenTheFirstNoteOfAGroupIsClicked();
    void selectsTheClippedRowThatWasClickedAndLeavesThePictureWhereItIs();
    void dropsTheMarkOfAPressThatSelectedNothingWhenItEnds();
    void bringsBackTheHeadWhenTheDeletionIsUndone();
    void regroupsWhenTheWindowIsActivated();
    void staysPutWhenTheWindowIsActivatedWithoutADayChange();
    void filtersTheListWithTheSearchField();
    void carriesOutAPendingDeletionWhenTheSearchChanges();
    void doesNotReadTheStoreAgainWhileADeletionIsCountingDown();
    void readsTheStoreAgainWhenTheOpenWindowIsShownAgain();
    void showsANoteCapturedWhileTheWindowStoodOpen();
    void waitsWithTheNewNoteWhileADeletionIsCountingDown();
    void takesUpTheWaitingNoteWhenTheDeletionIsCarriedOut();
    void takesUpTheWaitingNoteWhenTheDeletionIsUndone();
    void keepsTheReadingPlaceWhenANoteArrives();
    void takesUpANewNoteOnlyWhenItMatchesTheRunningSearch();
    void keepsTheEditorWhenANoteArrives();

    void putsTheCursorAtTheEndWithoutSelectingTheText();
    void savesTheChangedTextWithTheButton();
    void savesTheChangedTextWithControlEnter();
    void keepsCategoryTagsAndStateWhileSaving();
    void marksTheSavedNoteForANewEmbedding();
    void findsTheSavedTextInTheSearchIndex();
    void keepsTheAudioFileWhenTheTranscriptIsEdited();
    void refusesToSaveAnEmptyText();
    void keepsTheSelectionOnTheEditedNoteWhileTheDialogAsks();
    void asksBeforeUnsavedChangesAreLost_data();
    void asksBeforeUnsavedChangesAreLost();
    void keepsTheEditorWhenTheListIsRebuiltUnderIt();
    void keepsTheEditorWhenTheWindowIsActivatedAgain();

    // Qt emits aboutToQuit once per process, so the test of the quit path has
    // to be the last one of this class.
    void carriesOutTheDeletionWhenTheApplicationQuits();

private:
    static QDateTime at(const QString &isoDateTime);
    static QLocale german();
    static QLocale american();
    static Note noteWith(const QString &content);
    static Note noteWith(const QString &content, const QString &isoDateTime);

    /** The rows of `model` as "Kopf: …" and "Notiz: …", for whole-list checks. */
    static QStringList rowsOf(const NoteListModel &model);

    /** The row the note `noteIndex` sits in, as an index of `list`. */
    static QModelIndex noteRow(const QListView *list, int noteIndex);

    /** Adds a note to the store; the first one added is the newest. */
    qint64 storedNote(const QString &content);

    /** Adds a note of a fixed age, for the tests that look at the groups. */
    qint64 storedNote(const QString &content, const QString &isoDateTime);

    /** Fills in what the analysis run fills in — category, tags and state. */
    void analysed(qint64 id, const QString &category, const QStringList &tags);

    /** Texts of the labels the window shows right now. */
    static QStringList visibleLabels(const QWidget &window);

    /** The search field of the header. */
    static QLineEdit *searchOf(const QWidget &window);

    static QListView *listOf(const QWidget &window);
    static NoteListModel *modelOf(const QListView *list);
    static QAction *actionNamed(const QWidget &window, const QString &text);

    /** The button carrying `text`, or nullptr if the window shows none. */
    static QPushButton *buttonNamed(const QWidget &window, const QString &text);

    /** The reading pane and the text field of the edit state (wireframe 2a). */
    static QTextBrowser *readerOf(const QWidget &window);
    static QPlainTextEdit *editorOf(const QWidget &window);

    /**
     * The guard dialog once it stands, or nullptr — for the lambdas that run
     * inside its modal loop (wireframe 2a, state C).
     *
     * It asks the application which window is modal rather than the library
     * which dialog it built: under the KDE platform theme those are not the
     * same object for a QMessageBox, which is why the guard is a
     * KMessageDialog since issue #66.
     */
    static QDialog *waitForGuardDialog();

    /**
     * The answers `dialog` offers.
     *
     * Out of its QDialogButtonBox rather than out of all its children: a
     * KMessageDialog also carries a hidden „do not ask again“ checkbox, and
     * that is a QAbstractButton too — measured on 02.08.2026, when a test
     * clicked it instead of an answer and waited for a dialog that nobody had
     * answered.
     */
    static QList<QAbstractButton *> dialogAnswerButtons(QDialog *dialog);

    /** The answer of `dialog` labelled `label`, or nullptr. */
    static QAbstractButton *dialogButton(QDialog *dialog, const QString &label);

    /** The labels of the answers `dialog` offers. */
    static QStringList dialogAnswers(QDialog *dialog);

    /**
     * Clicks the answer labelled `label` in the guard dialog that the next
     * action opens (wireframe 2a, state C).
     *
     * The dialog is modal and runs an event loop of its own, so the answer has
     * to be queued before the action that opens it — from inside that loop
     * there is no other way back into the test.
     *
     * By label, not by role: the label is what the user reads and acts on. A
     * test that picks its button by the role it also expects to find would
     * click whatever sits in the „discard“ slot and never notice two answers
     * having swapped places.
     */
    static void answerNextDialog(const QString &label);

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    int m_storedNotes = 0;

};

void LibraryTest::initTestCase()
{
    // The window stores its size through KSharedConfig. Without the test mode
    // that write would land in the user's denkzettelrc.
    QStandardPaths::setTestModeEnabled(true);

    // NoteListModel and LibraryWindow read their locale out of QLocale(), the
    // system one, so three checks below would compare a different string in
    // every session: the entry reads "31.07.2026 14:32" under de_DE and under
    // the C locale of the CI container, "7/31/2026 2:32 PM" under en_US
    // (measured 20.08.2026 - without this line listsNotesWithTheirTimestamp(),
    // groupsAgainOnTheNewReferenceTime() and
    // leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked() go
    // red there). Pinned to German, the form those checks were written for.
    // Which arrangement which locale produces is not decided here but in the
    // timestamp checks, which are handed their locale as an argument.
    //
    // The English strings expected below are pinned from two sides. The domain
    // is set as it is in main.cpp, or every i18nc() call here warns that
    // translation will not work — 1643 lines of it in one run, enough to bury
    // any other warning. And LANGUAGE=en_US in tests/CMakeLists.txt names the
    // source language, for which no catalogue exists, so i18nc() hands back the
    // msgid. Without that pin an installed German catalogue reaches the checks
    // through XDG_DATA_DIRS and they compare German against English.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));
    QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));

    // Symbols come from the system theme, and the bare offscreen platform
    // brings no platform theme that would name one. Test mode rewrites the XDG
    // data locations and the icon loader looks for themes under those, so the
    // system icon directory has to be named as well (measured 02.08.2026).
    // Breeze is the icon stock SPEC 15 builds on.
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral("/usr/share/icons"));
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName(QStringLiteral("breeze"));
    }
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

/**
 * The second locale of the timestamp checks. The date forms are the object of
 * the check - month before day, no ordinal period, slashes instead of periods
 * - and the week beginning on Sunday comes with it (leavesTheWeekToTheLocale).
 */
QLocale LibraryTest::american()
{
    return QLocale(QLocale::English, QLocale::UnitedStates);
}

void LibraryTest::switchesOnTheCalendarDayNotAfterTwentyFourHours()
{
    // Twenty minutes old, but written on the previous calendar day.
    const QDateTime now = at(QStringLiteral("2026-07-31T00:10:00"));
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-30T23:50:00")), now, german()),
             library::NoteGroup::Yesterday);

    // Almost 24 hours old, but the same calendar day.
    const QDateTime lateEvening = at(QStringLiteral("2026-07-31T23:30:00"));
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-31T00:05:00")), lateEvening, german()),
             library::NoteGroup::Today);
}

void LibraryTest::keepsTheYearFourDigitsAcrossYears()
{
    // QLocale::ShortFormat alone would not give this: it writes "28.07.25" in
    // German and "7/28/25" in English, and a note from last year must not
    // read like one from this year (issue #108).
    const QDateTime lastYear = at(QStringLiteral("2025-07-28T09:00:45"));

    QCOMPARE(library::entryTimestamp(lastYear, german()), QStringLiteral("28.07.2025 09:00"));
    QCOMPARE(library::relativeTimestamp(lastYear, german()), QStringLiteral("Montag, 28.07.2025 09:00:45"));
    QCOMPARE(library::entryTimestamp(lastYear, american()), QStringLiteral("7/28/2025 9:00 AM"));
    QCOMPARE(library::relativeTimestamp(lastYear, american()), QStringLiteral("Monday, 7/28/2025 9:00:45 AM"));
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
    // The Monday probe of wireframe 3b: the day before belongs to "Yesterday"
    // although it lies in the previous calendar week — the first matching
    // group wins …
    const QDateTime monday = at(QStringLiteral("2026-08-03T10:00:00"));

    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-08-02T18:00:00")), monday, german()),
             library::NoteGroup::Yesterday);

    // … and the Thursday before is three days old and still "Last week".
    // Under the rolling seven days it had read "Do., 30. Juli".
    QCOMPARE(library::noteGroup(at(QStringLiteral("2026-07-30T09:00:00")), monday, german()),
             library::NoteGroup::LastWeek);

    // Nothing can fall into "This week" on a Monday; the list draws no head
    // over an empty group.
    for (int hoursBack = 1; hoursBack < 24 * 14; ++hoursBack) {
        const library::NoteGroup group =
            library::noteGroup(monday.addSecs(-3600LL * hoursBack), monday, german());
        QVERIFY2(group != library::NoteGroup::ThisWeek,
                 qPrintable(QStringLiteral("%1 hours back falls into “This week”").arg(hoursBack)));
    }
}

void LibraryTest::leavesTheWeekToTheLocale()
{
    // SPEC 9 names Monday because that is what the German locale says; the
    // rule itself is QLocale::firstDayOfWeek. In a locale whose week starts on
    // Sunday, that Sunday is the first day of "This week" — otherwise the
    // group would contradict the calendar the rest of the system draws.
    QCOMPARE(german().firstDayOfWeek(), Qt::Monday);
    QCOMPARE(american().firstDayOfWeek(), Qt::Sunday);

    const QDateTime wednesday = at(QStringLiteral("2026-07-29T16:00:00"));
    const QDateTime sunday = at(QStringLiteral("2026-07-26T09:00:00"));

    QCOMPARE(library::noteGroup(sunday, wednesday, german()), library::NoteGroup::LastWeek);
    QCOMPARE(library::noteGroup(sunday, wednesday, american()), library::NoteGroup::ThisWeek);
}

void LibraryTest::showsTheSameDateAndTimeFormInEveryGroup()
{
    // One timestamp from each of the five groups a Friday, 31 July, sorts
    // them into (SPEC 9) — entry and detail-pane form no longer depend on
    // which one a note falls into (issue #108).
    const QDateTime today = at(QStringLiteral("2026-07-31T14:32:07"));
    const QDateTime yesterday = at(QStringLiteral("2026-07-30T21:48:19"));
    const QDateTime thisWeek = at(QStringLiteral("2026-07-29T09:00:33"));
    const QDateTime lastWeek = at(QStringLiteral("2026-07-23T09:00:45"));
    const QDateTime older = at(QStringLiteral("2026-07-19T09:00:01"));

    // The list carries date and time, no seconds — the same form whichever
    // group the note is in.
    QCOMPARE(library::entryTimestamp(today, german()), QStringLiteral("31.07.2026 14:32"));
    QCOMPARE(library::entryTimestamp(yesterday, german()), QStringLiteral("30.07.2026 21:48"));
    QCOMPARE(library::entryTimestamp(thisWeek, german()), QStringLiteral("29.07.2026 09:00"));
    QCOMPARE(library::entryTimestamp(lastWeek, german()), QStringLiteral("23.07.2026 09:00"));
    QCOMPARE(library::entryTimestamp(older, german()), QStringLiteral("19.07.2026 09:00"));

    // The detail pane stands under no head and carries weekday, date and time
    // with seconds — again the same form in every group, and neither "Today"
    // nor "Yesterday" appears there any more.
    QCOMPARE(library::relativeTimestamp(today, german()), QStringLiteral("Freitag, 31.07.2026 14:32:07"));
    QCOMPARE(library::relativeTimestamp(yesterday, german()), QStringLiteral("Donnerstag, 30.07.2026 21:48:19"));
    QCOMPARE(library::relativeTimestamp(thisWeek, german()), QStringLiteral("Mittwoch, 29.07.2026 09:00:33"));
    QCOMPARE(library::relativeTimestamp(lastWeek, german()), QStringLiteral("Donnerstag, 23.07.2026 09:00:45"));
    QCOMPARE(library::relativeTimestamp(older, german()), QStringLiteral("Sonntag, 19.07.2026 09:00:01"));

    // The same in English, on two of the five: month before day, no leading
    // zero, twelve-hour clock with AM/PM, and the weekday name in English —
    // the group logic itself is already covered above and in the noteGroup()
    // checks, so this only has to show the arrangement, not repeat it.
    QCOMPARE(library::entryTimestamp(today, american()), QStringLiteral("7/31/2026 2:32 PM"));
    QCOMPARE(library::entryTimestamp(older, american()), QStringLiteral("7/19/2026 9:00 AM"));
    QCOMPARE(library::relativeTimestamp(today, american()), QStringLiteral("Friday, 7/31/2026 2:32:07 PM"));
    QCOMPARE(library::relativeTimestamp(older, american()), QStringLiteral("Sunday, 7/19/2026 9:00:01 AM"));
}

void LibraryTest::sortsATimestampFromTheFutureIntoToday()
{
    // A clock jump is no group of its own — the note is the newest one there
    // is and goes to the top (SPEC 9).
    const QDateTime now = at(QStringLiteral("2026-07-31T16:00:00"));
    const QDateTime ahead = at(QStringLiteral("2026-08-04T09:00:00"));

    QCOMPARE(library::noteGroup(ahead, now, german()), library::NoteGroup::Today);
}

Note LibraryTest::noteWith(const QString &content)
{
    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.content = content;
    return note;
}

// Healing this means changing the signature or introducing a type of its own,
// which is design rather than tidying up (issue #76). The one case a mix-up
// would be visible in - placeholderPage() in the empty library - gets a test
// assurance instead, as issue #88.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
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
    const NoteListModel *model = modelOf(list);
    const int row = model->rowOfNote(noteIndex);
    Q_ASSERT(row >= 0);

    return model->index(row);
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

    QCOMPARE(model.index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Today"));
    QCOMPARE(model.index(1).data(Qt::DisplayRole).toString(), QStringLiteral("heute gedacht"));

    // The entry carries date and time, whichever group its head names
    // (issue #108) — the default locale is German, set in initTestCase().
    QCOMPARE(model.index(1).data(NoteListModel::TimestampRole).toString(), QStringLiteral("31.07.2026 14:32"));
    QCOMPARE(model.index(3).data(NoteListModel::TimestampRole).toString(), QStringLiteral("30.07.2026 21:48"));
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

    // The last note of "Yesterday" takes its head with it — head and note are
    // two adjacent rows and go in one removal.
    QCOMPARE(rowsOf(model), QStringList({QStringLiteral("Kopf: Today"), QStringLiteral("Notiz: von heute")}));
    QCOMPARE(removed.size(), 1);
    QCOMPARE(removed.first().at(1).toInt(), 2);
    QCOMPARE(removed.first().at(2).toInt(), 3);

    QSignalSpy inserted(&model, &QAbstractItemModel::rowsInserted);
    model.insertNote(1, noteWith(QStringLiteral("von gestern"), QStringLiteral("2026-07-30T21:48:00")));

    // Undo brings both back, in the same place.
    QCOMPARE(rowsOf(model),
             QStringList({QStringLiteral("Kopf: Today"),
                          QStringLiteral("Notiz: von heute"),
                          QStringLiteral("Kopf: Yesterday"),
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

    QCOMPARE(model.index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Today"));
    QCOMPARE(model.index(1).data(NoteListModel::TimestampRole).toString(), QStringLiteral("31.07.2026 21:48"));

    model.regroup(at(QStringLiteral("2026-08-01T09:00:00")));

    // Only the head changes — the entry names its own date and time, not the
    // group, so regrouping leaves it untouched (issue #108).
    QCOMPARE(model.index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));
    QCOMPARE(model.index(1).data(NoteListModel::TimestampRole).toString(), QStringLiteral("31.07.2026 21:48"));
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

QLineEdit *LibraryTest::searchOf(const QWidget &window)
{
    QLineEdit *search = window.findChild<QLineEdit *>();
    Q_ASSERT(search);
    return search;
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

void LibraryTest::analysed(qint64 id, const QString &category, const QStringList &tags)
{
    // The analysis run of M3 writes these fields; in M2 nothing does, so the
    // test bench fills them in. That is what the check "unchanged after
    // saving" needs — against empty fields it could never fail (issue #11, K3).
    //
    // QVERIFY2 and not Q_ASSERT, measured with issue #99. Qt's CMake package
    // defines QT_NO_DEBUG for every build type but Debug, and under it
    // `Q_ASSERT(cond)` expands to `static_cast<void>(false && (cond))`: the
    // condition is compiled and never evaluated. The two writes below therefore
    // stopped happening as soon as anyone built Release — and the two checks
    // that read the category back afterwards reported a data loss that only the
    // bench had caused. QVERIFY2 is never compiled out and says who failed.
    //
    // The same holds for the read above it: without a live check `*stored`
    // would dereference an empty optional in exactly those builds.
    const std::optional<Note> stored = m_store->note(id);
    QVERIFY2(stored.has_value(), qPrintable(m_store->lastError()));

    Note note = *stored;
    note.category = category;
    note.state = Note::State::Analysed;
    QVERIFY2(m_store->updateNote(note), qPrintable(m_store->lastError()));
    QVERIFY2(m_store->setTags(id, tags), qPrintable(m_store->lastError()));
}

QPushButton *LibraryTest::buttonNamed(const QWidget &window, const QString &text)
{
    const QList<QPushButton *> buttons = window.findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        if (button->text() == text) {
            return button;
        }
    }
    return nullptr;
}

QTextBrowser *LibraryTest::readerOf(const QWidget &window)
{
    auto *reader = window.findChild<QTextBrowser *>();
    Q_ASSERT(reader);
    return reader;
}

QPlainTextEdit *LibraryTest::editorOf(const QWidget &window)
{
    auto *editor = window.findChild<QPlainTextEdit *>();
    Q_ASSERT(editor);
    return editor;
}

QDialog *LibraryTest::waitForGuardDialog()
{
    QDialog *dialog = nullptr;
    for (int attempt = 0; attempt < 200 && !dialog; ++attempt) {
        dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) {
            QTest::qWait(10);
        }
    }

    // …and then until it is really on the screen. A dialog that merely exists
    // is not yet the dialog the user meets: focus is handed out at that later
    // moment, and with the focus goes the default answer. Measured on
    // 02.08.2026 — read one turn of the event loop earlier, the default read
    // „Save“ and settled on „Cancel“ afterwards. The assertion on it
    // was green and meant nothing.
    // A dialog that never reaches the screen is no dialog for the caller
    // either — it gets nullptr and says so through its own assertion.
    if (dialog && !QTest::qWaitForWindowExposed(dialog)) {
        return nullptr;
    }

    return dialog;
}

QList<QAbstractButton *> LibraryTest::dialogAnswerButtons(QDialog *dialog)
{
    auto *box = dialog->findChild<QDialogButtonBox *>();
    return box ? box->buttons() : QList<QAbstractButton *>();
}

QAbstractButton *LibraryTest::dialogButton(QDialog *dialog, const QString &label)
{
    const QList<QAbstractButton *> buttons = dialogAnswerButtons(dialog);
    for (QAbstractButton *button : buttons) {
        if (KLocalizedString::removeAcceleratorMarker(button->text()) == label) {
            return button;
        }
    }
    return nullptr;
}

QStringList LibraryTest::dialogAnswers(QDialog *dialog)
{
    QStringList labels;
    const QList<QAbstractButton *> buttons = dialogAnswerButtons(dialog);
    for (const QAbstractButton *button : buttons) {
        labels.append(KLocalizedString::removeAcceleratorMarker(button->text()));
    }
    return labels;
}

void LibraryTest::answerNextDialog(const QString &label)
{
    QTimer::singleShot(0, qApp, [label] {
        QDialog *dialog = waitForGuardDialog();
        QVERIFY2(dialog, "The guard dialog never appeared");

        QAbstractButton *button = dialogButton(dialog, label);
        QVERIFY2(button,
                 qPrintable(QStringLiteral("The guard dialog offers no answer “%1”, but %2")
                                .arg(label, dialogAnswers(dialog).join(QLatin1Char('|')))));
        button->click();
    });
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

void LibraryTest::keepsTheNoteWhenTheDeletionIsUndone()
{
    const qint64 id = storedNote(QStringLiteral("bleibt doch"));
    PendingDeletion deletion(m_store.get(), 1);
    QSignalSpy reverted(&deletion, &PendingDeletion::reverted);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
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
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy committed(&deletion, &PendingDeletion::committed);

    deletion.request(id);
    deletion.flush();

    QCOMPARE(committed.size(), 1);
    QVERIFY(!m_store->note(id).has_value());

    // Flushing with nothing pending is a no-op, not a second deletion.
    deletion.flush();
    QCOMPARE(committed.size(), 1);
}

void LibraryTest::carriesOutTheDeletionWhenTheWindowCloses()
{
    const qint64 id = storedNote(QStringLiteral("beim Schließen weg"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Delete"))->trigger();

    // The grace period is still running, the note is still in the store.
    QVERIFY(m_store->note(id).has_value());

    window.close();

    QVERIFY(!m_store->note(id).has_value());
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
    QTest::newRow("as flat as possible") << QSize(900, 150);
}

void LibraryTest::bringsTheHeadOfTheNewGroupIntoView()
{
    // NOLINTNEXTLINE(misc-const-correctness) - QFETCH declares it, see the head of this file
    QFETCH(QSize, windowSize);

    // Three groups, the middle one holding a single note: entering it from
    // below puts the selection right under its head, which is what lets this
    // hold at the flattest window too. The outer groups are full, so the
    // boundary lies outside the first screenful and the list has to scroll.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    storedNote(QStringLiteral("von gestern"), QStringLiteral("2026-07-30T21:48:00"));
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(windowSize);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    QCOMPARE(modelOf(list)->noteCount(), 17);
    // A list that fits into the window would let this pass without scrolling.
    QVERIFY2(list->verticalScrollBar()->maximum() > 0,
             qPrintable(QStringLiteral("The list fits into the picture whole — the case does not occur")));

    // From the end of the list upwards across the boundary into "Yesterday".
    // Going up is what puts the entry against the upper edge — and its head
    // just outside it, unless the window pulls the head along.
    list->setCurrentIndex(noteRow(list, 16));
    for (int step = 0; step < 8; ++step) {
        QTest::keyClick(list, Qt::Key_Up);
    }

    const QModelIndex selected = noteRow(list, 8);
    QCOMPARE(list->currentIndex(), selected);

    const QModelIndex head = modelOf(list)->index(selected.row() - 1);
    QVERIFY(head.data(NoteListModel::GroupHeaderRole).toBool());
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));

    // The head of the new group is in the picture, whole rather than half cut
    // off — the selection never stands without its heading (wireframe 3b,
    // case 4) …
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1, viewport %2 px high")
                            .arg(list->visualRect(head).y())
                            .arg(list->viewport()->height())));

    // … and the selected entry is whole, not cut off at an edge (design decision
    // of 01.08.2026).
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(selected)),
             qPrintable(QStringLiteral("Selection y=%1 h=%2, viewport %3 px high")
                            .arg(list->visualRect(selected).y())
                            .arg(list->visualRect(selected).height())
                            .arg(list->viewport()->height())));

    // The heads scroll with the list rather than sticking to the top: the head
    // of "Today" has left the picture at this point.
    const QRect firstHead = list->visualRect(modelOf(list)->index(0));
    QVERIFY2(!list->viewport()->rect().intersects(firstHead),
             qPrintable(QStringLiteral("Head “Today” sticks at y=%1").arg(firstHead.y())));
}

void LibraryTest::staysPutWhileTheSelectionMovesWithinItsGroup()
{
    // AC 7 and wireframe 3b, case 4 both say „springt die Auswahl über eine
    // Gruppengrenze". Within one group nothing is fetched, however far the
    // head may have scrolled away: the user rolled the list to where they wanted
    // it, and one press of an arrow key must not throw that away — least of
    // all against the direction he is pressing in (UI review of 01.08.2026).
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
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    // The selection sits in "Yesterday", and the user has rolled the list so
    // that the entry stands at the top with its head above the upper edge.
    list->setCurrentIndex(noteRow(list, 9));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));

    list->verticalScrollBar()->setValue(noteRow(list, 9).row());
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1 — the case requires it outside the picture")
                            .arg(list->visualRect(head).y())));

    // The next note down is in the picture already …
    const QModelIndex target = noteRow(list, 10);
    QVERIFY(list->viewport()->rect().contains(list->visualRect(target)));

    const int rolledTo = list->verticalScrollBar()->value();
    QTest::keyClick(list, Qt::Key_Down);

    // … so the list does not move, and the crossing the user had scrolled to
    // stays where he put it.
    QCOMPARE(list->currentIndex(), target);
    QCOMPARE(list->verticalScrollBar()->value(), rolledTo);
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));
}

void LibraryTest::bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready()
{
    // The trap this guards against: it looks like a saving to fetch the head
    // only where the list has to be scrolled anyway — and it would undo the
    // whole heal. A note stands in full view while its head sits just above
    // the upper edge, and that is exactly the case the head is fetched for
    // (design decision of 01.08.2026, taken back after the case was measured).
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    // Selection in "Last week", then the list rolled so that the last note
    // of "Yesterday" stands in the picture while its head does not.
    list->setCurrentIndex(noteRow(list, 11));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));

    list->verticalScrollBar()->setValue(noteRow(list, 8).row());
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1 — the case requires it outside the picture")
                            .arg(list->visualRect(head).y())));

    // The row the selection is about to reach is already in the picture, whole.
    const QModelIndex target = noteRow(list, 10);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(target)),
             qPrintable(QStringLiteral("Target row y=%1 — the case requires it whole in the picture")
                            .arg(list->visualRect(target).y())));

    QTest::keyClick(list, Qt::Key_Up);

    // It crossed a group boundary, so the head is in the picture afterwards —
    // never mind that nothing had to be scrolled for the note itself.
    QCOMPARE(list->currentIndex(), target);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1, viewport %2 px high")
                            .arg(list->visualRect(head).y())
                            .arg(list->viewport()->height())));
    QVERIFY(list->viewport()->rect().contains(list->visualRect(target)));
}

void LibraryTest::leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked()
{
    // Pointing is not typing (issue #57). Until this story the assurance here
    // read the other way round and was called
    // `bringsTheHeadAlongWhenAVisibleNoteOfAnotherGroupIsClicked`: the mouse
    // took the same road as the arrow key, and clicking a visible note of
    // another group moved the list to fetch that group's head. Its comment
    // named the trade — "the user loses the place he was looking at and gains
    // the heading of what he picked" — and ended: "Should it grate in daily
    // use, this is the test that says where the decision was made." It grated,
    // the user reported it, and the measurement said why: the row they
    // pointed at slid 387 px out from under the cursor (UI review of
    // 01.08.2026, scenes n11a/n11b).
    //
    // What the two inputs mean is not the same. Pressing an arrow key, the user
    // moves through a list and expects it to move with him; clicking, he points
    // at a place and expects that place to stay. So the head is still fetched
    // for the key — `bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready` holds
    // that side — and no longer for the press.
    //
    // Measured is the roll value before and after the input, not the end state:
    // the scrollTo(selection) that follows makes both look right in a picture,
    // and only the movement between them tells them apart (issue #57, AK 3).
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    // A third group below, so the list can be rolled far enough for the head
    // of "Yesterday" to leave the picture at all.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    list->setCurrentIndex(noteRow(list, 2));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));

    list->verticalScrollBar()->setValue(noteRow(list, 8).row());
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));

    const QModelIndex target = noteRow(list, 10);
    QVERIFY(list->viewport()->rect().contains(list->visualRect(target)));

    const int rolledTo = list->verticalScrollBar()->value();
    const int targetBefore = list->visualRect(target).y();

    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(target).center());

    QCOMPARE(list->currentIndex(), target);

    // The picture has not moved: the roll value is the one the user had rolled
    // to, and the row they pointed at is still under their finger.
    QCOMPARE(list->verticalScrollBar()->value(), rolledTo);
    QCOMPARE(list->visualRect(target).y(), targetBefore);

    // The head stays outside, and that is the price of it. The day is not lost
    // with it: the reading pane carries the full timestamp of what was picked,
    // so what the head would have said stands in the window anyway (UX note on
    // issue #57).
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1").arg(list->visualRect(head).y())));
    QVERIFY(visibleLabels(window).contains(QStringLiteral("Donnerstag, 30.07.2026 09:00:00")));
}

void LibraryTest::keepsTheHeadFetchAfterAClickThatSelectedNothing()
{
    // The press is remembered for the selection change it causes — and a press
    // that causes none must not colour the next keystroke. A group head is the
    // case at hand: it is a row of the list, the mouse cannot pick it
    // (wireframe 3b), so clicking it changes nothing. If the mark from that
    // click were still lying around, the arrow key afterwards would be taken
    // for a mouse and the head of the group it enters would stay outside.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    // The same starting point as the arrow-key case: selection in "Letzte
    // Woche", the head of "Yesterday" rolled out of the picture, the row the key
    // is about to reach already in view.
    list->setCurrentIndex(noteRow(list, 11));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));

    list->verticalScrollBar()->setValue(noteRow(list, 8).row());
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));

    const QModelIndex target = noteRow(list, 10);
    QVERIFY(list->viewport()->rect().contains(list->visualRect(target)));

    // A head that stands in the picture, clicked.
    QModelIndex visibleHead;
    for (int row = 0; row < modelOf(list)->rowCount() && !visibleHead.isValid(); ++row) {
        const QModelIndex candidate = modelOf(list)->index(row);
        if (candidate.data(NoteListModel::GroupHeaderRole).toBool()
            && list->viewport()->rect().contains(list->visualRect(candidate))) {
            visibleHead = candidate;
        }
    }
    QVERIFY2(visibleHead.isValid(), "No group head in the picture — the case does not occur");

    const QModelIndex before = list->currentIndex();
    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(visibleHead).center());
    QCOMPARE(list->currentIndex(), before);

    QTest::keyClick(list, Qt::Key_Up);

    // The key crossed a group boundary, so the head is in the picture — the
    // click before it changed nothing and counts for nothing.
    QCOMPARE(list->currentIndex(), target);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1").arg(list->visualRect(head).y())));
}

void LibraryTest::bringsTheHeadAlongWhenTheSelectionReachesTheFirstNoteOfItsGroup()
{
    // Issue #70: the arrow key walks up to the first note of a group without
    // crossing a boundary — the note above it in the same group is where it
    // comes from — and until now nothing fetched the head. Under "Today" and
    // "Yesterday" an entry carries nothing but the time, so with the head outside
    // there stands „08:00" and nothing says of which day (SPEC 9, timestamps).
    //
    // Measured against the roll value before and after the key, not against a
    // picture: the scrollTo(selection) that follows makes both cases look right
    // in a picture, and only the movement between them tells them apart.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    // The selection sits on the second note of "Last week", and the user has
    // rolled the list so that the head of the group stands above the upper
    // edge. The note the key is about to reach is in the picture already — the
    // very case in which fetching the head looks like waste and is not.
    const QModelIndex first = noteRow(list, 11);
    list->setCurrentIndex(noteRow(list, 12));
    const QModelIndex head = modelOf(list)->index(first.row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Last week"));

    list->verticalScrollBar()->setValue(first.row());
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1 — the case requires it outside the picture")
                            .arg(list->visualRect(head).y())));
    QVERIFY(list->viewport()->rect().contains(list->visualRect(first)));

    QTest::keyClick(list, Qt::Key_Up);

    QCOMPARE(list->currentIndex(), first);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1, picture %2 high")
                            .arg(list->visualRect(head).y())
                            .arg(list->viewport()->height())));
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(first)),
             qPrintable(QStringLiteral("Selection at y=%1, picture %2 high")
                            .arg(list->visualRect(first).y())
                            .arg(list->viewport()->height())));
}

void LibraryTest::leavesThePictureWhereItIsWhenTheFirstNoteOfAGroupIsClicked()
{
    // What #70 has to keep out: its new trigger must not reach the mouse. The
    // one click test with a head assertion aims at the last note of a group, so
    // the first note of one appeared in no test at all.
    //
    // Since #71 a click moves the list for nothing at all, so "no jump on a
    // click" is held there; what is measured here is that the head is not
    // fetched for a press either, and it is measured before and after the
    // click, not on the end state.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    const QModelIndex first = noteRow(list, 11);
    const QModelIndex head = modelOf(list)->index(first.row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Last week"));

    // Selection elsewhere, head rolled out of the picture, the note to be
    // clicked fully in it.
    list->setCurrentIndex(noteRow(list, 2));
    list->verticalScrollBar()->setValue(first.row());
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));
    QVERIFY(list->viewport()->rect().contains(list->visualRect(first)));

    const int rolledTo = list->verticalScrollBar()->value();
    const int firstBefore = list->visualRect(first).y();

    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(first).center());

    QCOMPARE(list->currentIndex(), first);
    QCOMPARE(list->verticalScrollBar()->value(), rolledTo);
    QCOMPARE(list->visualRect(first).y(), firstBefore);
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1 — it must not be fetched for a mouse press")
                            .arg(list->visualRect(head).y())));
}

void LibraryTest::selectsTheClippedRowThatWasClickedAndLeavesThePictureWhereItIs()
{
    // Issue #71: a click on a row the lower edge cuts through used to select
    // its neighbour. QAbstractItemView::mousePressEvent sets the current row
    // first — which runs showNote() synchronously, and its scrollTo moved the
    // list by a row — and only afterwards picks the selection from the
    // rectangle it remembered at the press. That rectangle then pointed at the
    // row which had moved into its place.
    //
    // Three ways of writing this test would have gone green without measuring
    // anything (reproduction of 05.08.2026):
    //
    //  - Without a selection set beforehand currentChanged never fires, so the
    //    scrollTo never runs and every case reports "right" (2 of 11).
    //  - A fixed roll value can be the one at which the list moves by 35 px
    //    instead of 72, leaving the click point inside the same row.
    //  - Comparing selection against current index proves nothing: both are
    //    dragged onto the wrong row together. Measured against the row that
    //    was clicked.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    const QModelIndex preselected = noteRow(list, 0);

    int checked = 0;
    for (int value = list->verticalScrollBar()->minimum(); value <= list->verticalScrollBar()->maximum();
         ++value) {
        // The selection is put somewhere else first, or the click would not
        // change the current row and nothing would be measured at all.
        list->setCurrentIndex(preselected);
        list->verticalScrollBar()->setValue(value);

        const int row = bottomClippedRow(list);
        if (row < 0) {
            continue;
        }
        const QModelIndex target = modelOf(list)->index(row);
        QVERIFY2(target != preselected,
                 qPrintable(QStringLiteral("Roll value %1: the target row is the preselected one").arg(value)));

        // The point is 5 px below the upper edge of the row, well inside the
        // strip the viewport still shows of it.
        const QRect rect = list->visualRect(target);
        const int visible = list->viewport()->height() - rect.top();
        QVERIFY2(visible >= 2,
                 qPrintable(QStringLiteral("Roll value %1: visible strip only %2 px").arg(value).arg(visible)));
        const QPoint point(rect.center().x(), rect.top() + qMin(5, visible - 1));

        const int rolledTo = list->verticalScrollBar()->value();
        const int targetBefore = rect.y();
        ++checked;

        QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, point);

        // Exactly one row is marked, and it is the one the click sat in. In 5
        // of 14 measured cases nothing at all was marked, because after the
        // list had moved a group head or the empty space lay under the cursor.
        QCOMPARE(selectedRows(list), QString::number(row));
        QCOMPARE(list->currentIndex(), target);
        QCOMPARE(readerOf(window)->toPlainText(), modelOf(list)->noteAt(row).content);

        // And the picture stands still: the row the user pointed at has not
        // moved out from under his finger (reading 2 of UI review S5, B2).
        //
        // Measured immediately, and that is a limit of what this holds:
        // QAbstractItemView starts a delayed autoscroll on the press which
        // fires one double-click interval later and does fetch the row into
        // full view — the roll value stood at 6 up to 500 ms after the click
        // and at 7 from 550 ms on, with the selection staying on the clicked
        // row (05.08.2026, reported to the PO). What #71 heals is the
        // selection; that the list is quiet holds for the press, not for the
        // second after it.
        QCOMPARE(list->verticalScrollBar()->value(), rolledTo);
        QCOMPARE(list->visualRect(target).y(), targetBefore);
    }

    QVERIFY2(checked >= 10,
             qPrintable(QStringLiteral("Only %1 clipped roll values checked — the setup measures nothing")
                            .arg(checked)));
}

void LibraryTest::dropsTheMarkOfAPressThatSelectedNothingWhenItEnds()
{
    // The mark of the press is dropped by the call it belongs to — but a press
    // that selects nothing causes no such call, and until issue #71 only a key
    // on the list took the mark away afterwards. A selection set from the
    // program in between was then taken for a mouse and got no head fetched,
    // although no mouse had been anywhere near it (#71, AK 6).
    //
    // Two things came out of measuring this on 05.08.2026, and the second is
    // why this test looks at the head and not at the selection:
    //
    //  - The mark really does stick. It still stood after the click when the
    //    deletion set its selection two calls later.
    //  - It cannot be seen on the selection, though: QAbstractItemView::
    //    setCurrentIndex scrolls to what it selects on its own, so the list
    //    follows even when this window's own scrollTo is held back. The head
    //    fetch has no such second cover, and that is where the stale mark
    //    shows.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    // The same starting point as the arrow-key case: selection in "Letzte
    // Woche", the head of "Yesterday" rolled out of the picture.
    list->setCurrentIndex(noteRow(list, 11));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));

    list->verticalScrollBar()->setValue(noteRow(list, 8).row());
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));

    // A group head that stands in the picture, clicked. The mouse cannot pick
    // it (wireframe 3b), so the press changes no selection and causes no
    // showNote — and until #71 nothing but a key on the list took its mark away
    // afterwards.
    QModelIndex visibleHead;
    for (int row = 0; row < modelOf(list)->rowCount() && !visibleHead.isValid(); ++row) {
        const QModelIndex candidate = modelOf(list)->index(row);
        if (candidate.data(NoteListModel::GroupHeaderRole).toBool()
            && list->viewport()->rect().contains(list->visualRect(candidate))) {
            visibleHead = candidate;
        }
    }
    QVERIFY2(visibleHead.isValid(), "No group head in the picture — the case does not occur");

    const QModelIndex before = list->currentIndex();
    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(visibleHead).center());
    QCOMPARE(list->currentIndex(), before);

    // Now a selection set from the program, across a group boundary. This is
    // the call reload(), regroupList(), the deletion and its undo all make, and
    // it has to fetch the head: it is no press, and a mark left over from one
    // must not make it look like one.
    list->setCurrentIndex(noteRow(list, 10));

    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1, picture %2 high")
                            .arg(list->visualRect(head).y())
                            .arg(list->viewport()->height())));
    QVERIFY(list->viewport()->rect().contains(list->visualRect(list->currentIndex())));
}

void LibraryTest::bringsTheHeadAlongForANoteInTheMiddleOfASmallGroup()
{
    // The case the UI review of 01.08.2026 found: the selection lands in the
    // middle of a small group, so the head is not the row above it. It would
    // fit into the picture easily — and has to be fetched, or three entries
    // stand there with a bare time and no heading, while the only heading in
    // sight belongs to the group below.
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von heute, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 9; hour < 12; ++hour) {
        storedNote(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        storedNote(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                   QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    QCOMPARE(modelOf(list)->noteCount(), 19);
    QVERIFY(list->verticalScrollBar()->maximum() > 0);

    // Up from the end of the list into the middle note of the three-note group
    // "Yesterday" — its head is two rows above the selection, not one.
    list->setCurrentIndex(noteRow(list, 18));
    for (int step = 0; step < 9; ++step) {
        QTest::keyClick(list, Qt::Key_Up);
    }

    const QModelIndex selected = noteRow(list, 9);
    QCOMPARE(list->currentIndex(), selected);
    QCOMPARE(selected.data(Qt::DisplayRole).toString(), QStringLiteral("von gestern, 10 Uhr"));

    const QModelIndex head = modelOf(list)->index(selected.row() - 2);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));
    QVERIFY2(!modelOf(list)->index(selected.row() - 1).data(NoteListModel::GroupHeaderRole).toBool(),
             "The case requires a note with no head above it");

    // Head and selection fit into the list together, so both are in the
    // picture, whole.
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Head at y=%1, viewport %2 px high")
                            .arg(list->visualRect(head).y())
                            .arg(list->viewport()->height())));
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(selected)),
             qPrintable(QStringLiteral("Selection y=%1 h=%2, viewport %3 px high")
                            .arg(list->visualRect(selected).y())
                            .arg(list->visualRect(selected).height())
                            .arg(list->viewport()->height())));
}

void LibraryTest::leavesTheHeadOutsideWhereItCannotFitWithTheSelection()
{
    // The other side of the same rule: a group taller than the list. Fetching
    // its head would push the selection out of the picture, so the head stays
    // where it is — one key press must not scroll away what the user is
    // looking at.
    for (int minute = 0; minute < 30; ++minute) {
        storedNote(QStringLiteral("von heute, Minute %1").arg(minute),
                   QStringLiteral("2026-07-31T10:%1:00").arg(minute, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    const QModelIndex head = modelOf(list)->index(0);
    QVERIFY(head.data(NoteListModel::GroupHeaderRole).toBool());

    // At the end of the list, where the head of the group is far above.
    list->setCurrentIndex(noteRow(list, 29));

    // The step goes to a row that is already in the picture, so the list must
    // not move at all: fetching the far-away head and scrolling back would
    // leave the selection at the opposite edge, with everything the user was
    // reading gone (UI review of 01.08.2026).
    const QModelIndex selected = noteRow(list, 28);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(selected)),
             "The case requires a target row that already stands whole in the picture");

    const int scrolledTo = list->verticalScrollBar()->value();
    QTest::keyClick(list, Qt::Key_Up);
    QCOMPARE(list->verticalScrollBar()->value(), scrolledTo);

    QCOMPARE(list->currentIndex(), selected);

    // The head is further away than the list is tall …
    const int span = list->visualRect(selected).bottom() - list->visualRect(head).top();
    QVERIFY2(span > list->viewport()->height(),
             qPrintable(QStringLiteral("Head and selection span %1 px, viewport %2 px — the case "
                                       "does not occur")
                            .arg(span)
                            .arg(list->viewport()->height())));

    // … so it stays outside, and the selection keeps the picture, whole.
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(selected)),
             qPrintable(QStringLiteral("Selection y=%1 h=%2, viewport %3 px high")
                            .arg(list->visualRect(selected).y())
                            .arg(list->visualRect(selected).height())
                            .arg(list->viewport()->height())));
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
             QStringList({QStringLiteral("Kopf: Today"),
                          QStringLiteral("Notiz: von heute"),
                          QStringLiteral("Kopf: Yesterday"),
                          QStringLiteral("Notiz: von gestern")}));

    actionNamed(window, QStringLiteral("Delete"))->trigger();

    // The head of "Yesterday" goes with its last note …
    QCOMPARE(rowsOf(*modelOf(list)),
             QStringList({QStringLiteral("Kopf: Today"), QStringLiteral("Notiz: von heute")}));
    // … and the selection falls back to the preceding note, never onto a head.
    QCOMPARE(list->currentIndex(), noteRow(list, 0));

    QTest::keyClick(&window, Qt::Key_Z, Qt::ControlModifier);

    // The undo brings note and head back in the same place.
    QCOMPARE(rowsOf(*modelOf(list)),
             QStringList({QStringLiteral("Kopf: Today"),
                          QStringLiteral("Notiz: von heute"),
                          QStringLiteral("Kopf: Yesterday"),
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
    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Today"));

    // The window stood open past midnight. Setting the reference time alone
    // changes nothing — there is no timer that regroups on its own
    // (wireframe 3b).
    window.setReferenceTime(at(QStringLiteral("2026-08-01T09:00:00")));
    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Today"));

    // Looking at the window again does: the activation regroups the list, and
    // the note keeps its selection across the regrouping.
    QWidget elsewhere;
    elsewhere.show();
    elsewhere.activateWindow();
    QTRY_VERIFY(!window.isActiveWindow());

    window.activateWindow();
    QTRY_VERIFY(window.isActiveWindow());

    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("gestern Abend gedacht"));
}

void LibraryTest::staysPutWhenTheWindowIsActivatedWithoutADayChange()
{
    // The counterpart of the test above, and the blind spot it left open
    // (issue #59): with the selection in row 0 the jump cannot show, because
    // there is nothing above it to scroll. So the selection sits far down here
    // and the list is rolled away from it before the window is looked at
    // again.
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
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    // The selection is in "Yesterday", well below row 0 …
    const QModelIndex selected = noteRow(list, 12);
    list->setCurrentIndex(selected);
    QVERIFY(selected.row() > 0);

    // … and the user has rolled the list back to the top, away from it.
    list->verticalScrollBar()->setValue(0);
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(selected)),
             "The case requires the selection outside the picture");
    const int rolledTo = list->verticalScrollBar()->value();

    // Alt-Tab away and back, without a day passing in between.
    QWidget elsewhere;
    elsewhere.show();
    elsewhere.activateWindow();
    QTRY_VERIFY(!window.isActiveWindow());

    window.activateWindow();
    QTRY_VERIFY(window.isActiveWindow());

    // Measured at the scroll value, not at the picture: nothing about the
    // grouping has changed, so nothing may move.
    QCOMPARE(list->verticalScrollBar()->value(), rolledTo);
    QCOMPARE(list->currentIndex(), selected);
}

void LibraryTest::filtersTheListWithTheSearchField()
{
    storedNote(QStringLiteral("Bücher über Straßenbahnen ansehen"));
    storedNote(QStringLiteral("Backup der Fotos prüfen"));
    storedNote(QStringLiteral("Milch kaufen"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QListView *list = listOf(window);
    QCOMPARE(modelOf(list)->noteCount(), 3);

    // Typing filters while the user types — the store decides what matches,
    // the window only passes the text on (SPEC 6).
    searchOf(window)->setText(QStringLiteral("Fotos"));
    QCOMPARE(modelOf(list)->noteCount(), 1);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Backup der Fotos prüfen"));

    // A part in the middle of a word finds it too, and so does a term spelled
    // without its umlaut — both come from the store layer.
    searchOf(window)->setText(QStringLiteral("bahn"));
    QCOMPARE(modelOf(list)->noteCount(), 1);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Bücher über Straßenbahnen ansehen"));

    searchOf(window)->setText(QStringLiteral("bucher"));
    QCOMPARE(modelOf(list)->noteCount(), 1);
}

void LibraryTest::carriesOutAPendingDeletionWhenTheSearchChanges()
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

    // A search reads the store again, and the note counting down is still in
    // it. Carrying the deletion out first is what keeps list and store in
    // agreement — the rule a second deletion and the closing window follow.
    searchOf(window)->setText(QStringLiteral("gelöscht"));

    QCOMPARE(modelOf(list)->noteCount(), 0);
    QCOMPARE(m_store->notes().size(), 1);
    QCOMPARE(m_store->notes().at(0).content, QStringLiteral("bleibt"));
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

    // A note that got into the store without announcing itself — the store is
    // silenced for the length of the write. Since issue #105 the window follows
    // the announcement by itself, and without the blocker this test would go
    // through that road and leave the one it is about unmeasured: showLibrary()
    // on the open window reads the store again, and this says so.
    //
    // The note captured meanwhile is the newest one and takes the top row, so
    // the selected note moves down.
    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    {
        const QSignalBlocker silence(m_store.get());
        QVERIFY(m_store->addNote(captured).has_value());
    }
    QCOMPARE(modelOf(list)->noteCount(), 2);

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

void LibraryTest::showsANoteCapturedWhileTheWindowStoodOpen()
{
    storedNote(QStringLiteral("die ältere Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QListView *list = listOf(window);
    QCOMPARE(modelOf(list)->noteCount(), 1);

    // The user's path (issue #105): Meta+N, type, Ctrl+Enter — and nothing
    // else. Nobody asks the library for anything, so nothing here shows it, and
    // that is the whole point: the note has to arrive on its own.
    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    QCOMPARE(modelOf(list)->noteCount(), 2);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("gerade festgehalten"));
}

void LibraryTest::waitsWithTheNewNoteWhileADeletionIsCountingDown()
{
    const qint64 deleted = storedNote(QStringLiteral("wird gelöscht"));
    storedNote(QStringLiteral("bleibt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);
    QCOMPARE(modelOf(list)->noteCount(), 1);

    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    // The deleted note is still in the store, so reading it back now would
    // fetch it into a list that is counting it down. The new note waits; the
    // list stands exactly as the deletion left it.
    QCOMPARE(modelOf(list)->noteCount(), 1);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("bleibt"));

    // And the period is not cut short for it: the note the user wrote
    // elsewhere must not spend the undo the window is still offering him.
    QVERIFY(m_store->note(deleted).has_value());
}

void LibraryTest::takesUpTheWaitingNoteWhenTheDeletionIsCarriedOut()
{
    const qint64 deleted = storedNote(QStringLiteral("wird gelöscht"));
    storedNote(QStringLiteral("bleibt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);

    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    // The full five seconds of SPEC 9, waited out rather than shortened: what
    // is measured here is that the waiting note is taken up by itself, without
    // anyone asking the window for anything.
    const NoteListModel *model = modelOf(list);
    QVERIFY2(QTest::qWaitFor([model] { return model->noteCount() == 2; }, 8000),
             qPrintable(QStringLiteral("The list has %1 notes").arg(model->noteCount())));

    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("gerade festgehalten"));
    QCOMPARE(noteRow(list, 1).data(Qt::DisplayRole).toString(), QStringLiteral("bleibt"));
    QVERIFY(!m_store->note(deleted).has_value());
}

void LibraryTest::takesUpTheWaitingNoteWhenTheDeletionIsUndone()
{
    const qint64 deleted = storedNote(QStringLiteral("wird gelöscht"));
    storedNote(QStringLiteral("bleibt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);

    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    actionNamed(window, QStringLiteral("Undo"))->trigger();

    // The undone note comes back and the waiting one comes with it — one list
    // that agrees with the store again.
    QCOMPARE(modelOf(list)->noteCount(), 3);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("gerade festgehalten"));
    QCOMPARE(noteRow(list, 1).data(Qt::DisplayRole).toString(), QStringLiteral("wird gelöscht"));
    QCOMPARE(noteRow(list, 2).data(Qt::DisplayRole).toString(), QStringLiteral("bleibt"));
    QVERIFY(m_store->note(deleted).has_value());
}

void LibraryTest::keepsTheReadingPlaceWhenANoteArrives()
{
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
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    // The user has rolled the list down and is reading a note in the middle of
    // it. Both marks are set from outside — the row they rolled to and the note
    // he picked —, so neither of them can move along with the fault.
    //
    // Picking comes first and rolling second: the selection fetches the head of
    // its group into the picture by itself (issue #70), and that would set the
    // starting condition instead of the roll value below.
    list->setCurrentIndex(noteRow(list, 10));
    list->verticalScrollBar()->setValue(noteRow(list, 9).row());
    const QString atTheTop = list->indexAt(QPoint(0, 0)).data(Qt::DisplayRole).toString();
    QCOMPARE(atTheTop, QStringLiteral("von gestern, 14 Uhr"));

    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = at(QStringLiteral("2026-07-31T15:30:00"));
    QVERIFY(m_store->addNote(captured).has_value());

    // The note is in the list …
    QCOMPARE(modelOf(list)->noteCount(), 17);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("gerade festgehalten"));

    // … and the page the user was reading has not moved under him: the same
    // row stands at the upper edge, and the same note is selected.
    QCOMPARE(list->indexAt(QPoint(0, 0)).data(Qt::DisplayRole).toString(), atTheTop);
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("von gestern, 13 Uhr"));
}

void LibraryTest::takesUpANewNoteOnlyWhenItMatchesTheRunningSearch()
{
    storedNote(QStringLiteral("Backup der Fotos prüfen"));
    storedNote(QStringLiteral("Milch kaufen"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QListView *list = listOf(window);
    searchOf(window)->setText(QStringLiteral("Backup"));
    QCOMPARE(modelOf(list)->noteCount(), 1);

    // A note that does not match the running search does not belong in a
    // result list — the term stands in the field and still says what it says.
    Note beside = noteWith(QStringLiteral("Brot holen"));
    beside.createdAt = beside.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(beside).has_value());

    QCOMPARE(modelOf(list)->noteCount(), 1);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Backup der Fotos prüfen"));

    // One that matches does, and it takes the top row like anywhere else.
    Note hit = noteWith(QStringLiteral("Backup vom Vortag kontrollieren"));
    hit.createdAt = hit.createdAt.addSecs(120);
    QVERIFY(m_store->addNote(hit).has_value());

    QCOMPARE(modelOf(list)->noteCount(), 2);
    QCOMPARE(noteRow(list, 0).data(Qt::DisplayRole).toString(),
             QStringLiteral("Backup vom Vortag kontrollieren"));
}

void LibraryTest::keepsTheEditorWhenANoteArrives()
{
    storedNote(QStringLiteral("Transkript mit einem Hörfehler"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    // No dialog may turn up here: nothing the user did is being taken away
    // from him, and a question he did not cause is worse than a stale list.
    const DialogWatch watch;

    buttonNamed(window, QStringLiteral("Edit"))->click();
    editorOf(window)->setPlainText(QStringLiteral("Transkript mit einem Hörfehler, ausgebessert"));

    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    QVERIFY(editorOf(window)->isVisible());
    QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("Transkript mit einem Hörfehler, ausgebessert"));
    QCOMPARE(modelOf(listOf(window))->noteCount(), 2);
    QVERIFY(!watch.appeared());
}

void LibraryTest::putsTheCursorAtTheEndWithoutSelectingTheText()
{
    const QString content = QStringLiteral("Transkript mit einem Hörfehler");
    storedNote(content);

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();

    // Wireframe 2a: the cursor stands at the end and nothing is selected —
    // the first keystroke must not be able to overwrite the note.
    const QTextCursor cursor = editorOf(window)->textCursor();
    QVERIFY2(!cursor.hasSelection(), "The whole text is selected");
    QCOMPARE(cursor.position(), static_cast<int>(content.size()));
}

void LibraryTest::savesTheChangedTextWithTheButton()
{
    const qint64 id = storedNote(QStringLiteral("sonst wird der Fold zugemüllt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("sonst wird der Vault zugemüllt"));

    buttonNamed(window, QStringLiteral("Save"))->click();

    QCOMPARE(m_store->note(id)->content, QStringLiteral("sonst wird der Vault zugemüllt"));

    // The answer is the way back into the reading state, and the reading pane
    // carries what was written (wireframe 2a).
    QCOMPARE(readerOf(window)->toPlainText(), QStringLiteral("sonst wird der Vault zugemüllt"));
    QVERIFY(buttonNamed(window, QStringLiteral("Edit"))->isVisible());

    // The list shows the note it now holds, not the one it was opened with.
    QCOMPARE(noteRow(listOf(window), 0).data(Qt::DisplayRole).toString(),
             QStringLiteral("sonst wird der Vault zugemüllt"));
}

void LibraryTest::savesTheChangedTextWithControlEnter()
{
    const qint64 id = storedNote(QStringLiteral("sonst wird der Fold zugemüllt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    QPlainTextEdit *editor = editorOf(window);
    editor->setPlainText(QStringLiteral("sonst wird der Vault zugemüllt"));

    QTest::keyClick(editor, Qt::Key_Return, Qt::ControlModifier);

    QCOMPARE(m_store->note(id)->content, QStringLiteral("sonst wird der Vault zugemüllt"));
    QVERIFY(!editor->isVisible());
}

void LibraryTest::keepsCategoryTagsAndStateWhileSaving()
{
    const qint64 id = storedNote(QStringLiteral("Idee für Denkzettel"));
    analysed(id,
             QStringLiteral("Software-Ideen"),
             QStringList({QStringLiteral("software-idee"), QStringLiteral("denkzettel")}));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("Idee für Denkzettel, überarbeitet"));
    buttonNamed(window, QStringLiteral("Save"))->click();

    // SPEC 9: editing keeps category, tags and state — the analysis run keeps
    // those, not the editor.
    const std::optional<Note> saved = m_store->note(id);
    QVERIFY(saved.has_value());
    QCOMPARE(saved->content, QStringLiteral("Idee für Denkzettel, überarbeitet"));
    QCOMPARE(saved->category, QStringLiteral("Software-Ideen"));
    QCOMPARE(saved->state, Note::State::Analysed);
    QCOMPARE(m_store->tags(id),
             QStringList({QStringLiteral("denkzettel"), QStringLiteral("software-idee")}));
}

void LibraryTest::marksTheSavedNoteForANewEmbedding()
{
    const qint64 id = storedNote(QStringLiteral("Idee für Denkzettel"));
    QVERIFY(!m_store->note(id)->needsReembed);

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("Idee für Denkzettel, überarbeitet"));
    buttonNamed(window, QStringLiteral("Save"))->click();

    // SPEC 9 / 7.2: the embedding ages with the text, so the next analysis run
    // renews it — and only it.
    QVERIFY2(m_store->note(id)->needsReembed, "needs_reembed is not 1 after saving");
}

void LibraryTest::findsTheSavedTextInTheSearchIndex()
{
    storedNote(QStringLiteral("sonst wird der Fold zugemüllt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("sonst wird der Vault zugemüllt"));
    buttonNamed(window, QStringLiteral("Save"))->click();

    // The full-text index follows the text; the search of S6 finds the note
    // under its new word and no longer under the old one.
    QCOMPARE(m_store->search(QStringLiteral("Vault")).size(), 1);
    QCOMPARE(m_store->search(QStringLiteral("Fold")).size(), 0);
}

void LibraryTest::keepsTheAudioFileWhenTheTranscriptIsEdited()
{
    Note spoken = noteWith(QStringLiteral("Transkript mit einem Hörfehler"));
    spoken.type = Note::Type::Audio;
    spoken.audioPath = QStringLiteral("2026/07/notiz.opus");
    spoken.audioDurationS = 41;
    const std::optional<qint64> id = m_store->addNote(spoken);
    QVERIFY(id.has_value());

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("Transkript ohne Hörfehler"));
    buttonNamed(window, QStringLiteral("Save"))->click();

    // SPEC 9: only the transcript is edited, never the recording.
    const std::optional<Note> saved = m_store->note(*id);
    QVERIFY(saved.has_value());
    QCOMPARE(saved->content, QStringLiteral("Transkript ohne Hörfehler"));
    QCOMPARE(saved->type, Note::Type::Audio);
    QCOMPARE(saved->audioPath, QStringLiteral("2026/07/notiz.opus"));
    QCOMPARE(saved->audioDurationS, std::optional<int>(41));
}

void LibraryTest::refusesToSaveAnEmptyText()
{
    const qint64 id = storedNote(QStringLiteral("bleibt so stehen"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    QPlainTextEdit *editor = editorOf(window);
    editor->setPlainText(QStringLiteral("   \n  "));

    // Wireframe 2a: an empty field is no valid state to save. Deleting runs
    // over the delete action, not over emptying the field.
    QVERIFY2(!buttonNamed(window, QStringLiteral("Save"))->isEnabled(),
             "“Save” can be triggered with an empty field");

    QTest::keyClick(editor, Qt::Key_Return, Qt::ControlModifier);

    QCOMPARE(m_store->note(id)->content, QStringLiteral("bleibt so stehen"));
    QVERIFY2(editor->isVisible(), "Ctrl+Enter saved the empty text");

    // Filling it again makes the button live once more.
    editor->setPlainText(QStringLiteral("doch etwas"));
    QVERIFY(buttonNamed(window, QStringLiteral("Save"))->isEnabled());
}

void LibraryTest::keepsTheSelectionOnTheEditedNoteWhileTheDialogAsks()
{
    storedNote(QStringLiteral("wird bearbeitet"));
    storedNote(QStringLiteral("die andere"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("halb getippt"));

    QModelIndex selectedWhileAsking;
    QTimer::singleShot(0, qApp, [&selectedWhileAsking, list] {
        QDialog *dialog = waitForGuardDialog();
        QVERIFY(dialog);

        selectedWhileAsking = list->currentIndex();

        QAbstractButton *cancel = dialogButton(dialog, QStringLiteral("Cancel"));
        QVERIFY(cancel);
        cancel->click();
    });

    list->setCurrentIndex(noteRow(list, 1));

    // The dialog asks about the note under the editor, so the list has to point
    // at that note while the question stands. currentChanged runs after the
    // selection has already jumped — without taking it back first, list and
    // question name different notes at the moment of the decision
    // (UI review of 02.08.2026, finding 2).
    QCOMPARE(selectedWhileAsking, noteRow(list, 0));
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
    QVERIFY(editorOf(window)->isVisible());
}

void LibraryTest::asksBeforeUnsavedChangesAreLost_data()
{
    // Three ways out of the edit state and three answers to each — the matrix
    // of wireframe 2a, state C. Leaving one column out would leave one way of
    // losing a correction unwatched.
    //
    // The answers are named by their label, not by the button role behind them
    // (issue #66). What the wireframe fixes is their *meaning* — „Save“
    // writes and carries the act out, „Discard“ carries it out without
    // writing, „Cancel“ stays in the editor — while roles and order belong
    // to the platform. A matrix that both clicks and expects by role would
    // survive two answers swapping their labels.
    QTest::addColumn<QString>("trigger");
    QTest::addColumn<QString>("answer");

    for (const QString &trigger :
         {QStringLiteral("auswahlwechsel"), QStringLiteral("fensterschliessen"), QStringLiteral("esc")}) {
        for (const QString &answer :
             {QStringLiteral("Save"), QStringLiteral("Discard"), QStringLiteral("Cancel")}) {
            QTest::newRow(qPrintable(trigger + QLatin1Char('-') + answer.toLower())) << trigger << answer;
        }
    }
}

void LibraryTest::asksBeforeUnsavedChangesAreLost()
{
    // NOLINTNEXTLINE(misc-const-correctness) - QFETCH declares it, see the head of this file
    QFETCH(QString, trigger);
    // NOLINTNEXTLINE(misc-const-correctness) - QFETCH declares it, see the head of this file
    QFETCH(QString, answer);

    const qint64 edited = storedNote(QStringLiteral("erste Notiz"));
    storedNote(QStringLiteral("zweite Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("erste Notiz, berichtigt"));

    answerNextDialog(answer);

    if (trigger == QStringLiteral("auswahlwechsel")) {
        list->setCurrentIndex(noteRow(list, 1));
    } else if (trigger == QStringLiteral("fensterschliessen")) {
        window.close();
    } else {
        QTest::keyClick(editorOf(window), Qt::Key_Escape);
    }

    // „Save“ writes, „Discard“ and „Cancel“ leave the note alone.
    const QString stored = m_store->note(edited)->content;
    if (answer == QStringLiteral("Save")) {
        QCOMPARE(stored, QStringLiteral("erste Notiz, berichtigt"));
    } else {
        QCOMPARE(stored, QStringLiteral("erste Notiz"));
    }

    // „Save“ and „Discard“ carry the triggering act out, „Cancel“
    // stays in the edit state and takes the act back.
    const bool carriedOut = answer != QStringLiteral("Cancel");

    if (trigger == QStringLiteral("auswahlwechsel")) {
        QCOMPARE(list->currentIndex(), noteRow(list, carriedOut ? 1 : 0));
        QCOMPARE(editorOf(window)->isVisible(), !carriedOut);
        if (!carriedOut) {
            QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("erste Notiz, berichtigt"));
        }
    } else if (trigger == QStringLiteral("fensterschliessen")) {
        QCOMPARE(window.isVisible(), !carriedOut);
        if (!carriedOut) {
            QVERIFY(editorOf(window)->isVisible());
            QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("erste Notiz, berichtigt"));
        }
    } else {
        QCOMPARE(editorOf(window)->isVisible(), !carriedOut);
        QCOMPARE(list->currentIndex(), noteRow(list, 0));
        if (!carriedOut) {
            QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("erste Notiz, berichtigt"));
        }
    }
}

void LibraryTest::keepsTheEditorWhenTheListIsRebuiltUnderIt()
{
    storedNote(QStringLiteral("wird gerade bearbeitet"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    const QModelIndex before = list->currentIndex();

    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("halb getippt"));

    // Meta+N while the library stands open: the new note takes the top row and
    // pushes the edited one down. Its row number changes, the note does not —
    // and a question about unsaved changes here would be a question about
    // nothing.
    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    DialogWatch watch;
    window.showLibrary();
    QTest::qWait(100);

    // The row really did move — otherwise this test would prove nothing.
    QCOMPARE(modelOf(list)->noteCount(), 2);
    QVERIFY2(list->currentIndex() != before, "The row did not move at all");

    QVERIFY2(!watch.appeared(), "The guard dialog came although the note stayed the same");
    QVERIFY(editorOf(window)->isVisible());
    QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("halb getippt"));
}

void LibraryTest::keepsTheEditorWhenTheWindowIsActivatedAgain()
{
    storedNote(QStringLiteral("gestern Abend gedacht"), QStringLiteral("2026-07-31T21:48:00"));

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T22:00:00")));
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Edit"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("halb getippt"));

    // The window stood open past midnight and is looked at again: the
    // activation regroups the list, and the regrouping rebuilds every row
    // under the editor. The note stays the same one, so the question must not
    // come up — a user coming back from another window would otherwise be
    // asked about a change he has not finished making.
    window.setReferenceTime(at(QStringLiteral("2026-08-01T09:00:00")));

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    DialogWatch watch;

    QWidget elsewhere;
    elsewhere.show();
    elsewhere.activateWindow();
    QTRY_VERIFY(!window.isActiveWindow());

    window.activateWindow();
    QTRY_VERIFY(window.isActiveWindow());
    QTest::qWait(100);

    // The regrouping really did happen — otherwise nothing was rebuilt here.
    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Yesterday"));

    QVERIFY2(!watch.appeared(), "The guard dialog came on returning to the window");
    QVERIFY(editorOf(window)->isVisible());
    QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("halb getippt"));
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
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
    actionNamed(window, QStringLiteral("Delete"))->trigger();

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
