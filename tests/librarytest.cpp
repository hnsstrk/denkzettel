#include "store/store.h"
#include "ui/elidedlines.h"
#include "ui/librarywindow.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"
#include "ui/pendingdeletion.h"
#include "ui/timestampformat.h"

#include <KLocalizedString>
#include <KMessageWidget>
#include <KStandardShortcut>

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QIcon>
#include <KStandardGuiItem>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListView>
#include <QApplication>
#include <QLocale>
#include <QPlainTextEdit>
#include <QPushButton>
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
    void bringsTheHeadAlongForANoteInTheMiddleOfASmallGroup();
    void leavesTheHeadOutsideWhereItCannotFitWithTheSelection();
    void staysPutWhileTheSelectionMovesWithinItsGroup();
    void bringsTheHeadAlongEvenWhenTheNoteIsInViewAlready();
    void leavesThePictureWhereItIsWhenAVisibleNoteOfAnotherGroupIsClicked();
    void keepsTheHeadFetchAfterAClickThatSelectedNothing();
    void bringsBackTheHeadWhenTheDeletionIsUndone();
    void regroupsWhenTheWindowIsActivated();
    void staysPutWhenTheWindowIsActivatedWithoutADayChange();
    void undoesTheDeletionByKeyboard();
    void deletesWithTheDeleteKey();
    void showsTheRemainingTimeInTheMessage();
    void keepsOneMessageWhenASecondNoteIsDeleted();
    void closesWithTheStandardShortcut();
    void showsTheSearchField();
    void filtersTheListWithTheSearchField();
    void groupsTheSearchResultsLikeTheLibrary();
    void showsTheEmptyStateWhenNothingMatches();
    void restoresTheFullListWhenTheSearchFieldIsCleared();
    void carriesOutAPendingDeletionWhenTheSearchChanges();
    void doesNotReadTheStoreAgainWhileADeletionIsCountingDown();
    void readsTheStoreAgainWhenTheOpenWindowIsShownAgain();
    void leavesTheFocusAloneWhenTheOpenWindowIsShownAgain();
    void keepsTheListWideEnoughForThePreview();
    void keepsTheHeaderAtTheTopAndTheRestForTheNotes_data();
    void keepsTheHeaderAtTheTopAndTheRestForTheNotes();
    void keepsTheMeasuresOfTheGroupedList_data();
    void keepsTheMeasuresOfTheGroupedList();
    void putsTheMessageBetweenTheHeaderAndTheNotes();
    void keepsTheWindowSizeForTheNextSession();
    void textsFollowAColourSchemeChange();

    void opensTheEditorWithTheButton();
    void opensTheEditorWithF2();
    void leavesTheWordSelectionToTheDoubleClick();
    void putsTheCursorAtTheEndWithoutSelectingTheText();
    void showsCategoryAndTagsAsPlainDisplayWhileEditing();
    void putsTheEditingBadgeWhereTheButtonsStand();
    void savesTheChangedTextWithTheButton();
    void savesTheChangedTextWithControlEnter();
    void keepsCategoryTagsAndStateWhileSaving();
    void marksTheSavedNoteForANewEmbedding();
    void findsTheSavedTextInTheSearchIndex();
    void keepsTheAudioFileWhenTheTranscriptIsEdited();
    void refusesToSaveAnEmptyText();
    void leavesTheEditorWithoutAskingWhenNothingWasChanged();
    void namesTheThreeAnswersOfTheGuardDialog();
    void showsTheWarningSymbolInTheGuardDialog();
    void namesTheSymbolsOfTheDetailButtons();
    void namesTheSymbolOfTheUndoAction();
    void keepsTheSelectionOnTheEditedNoteWhileTheDialogAsks();
    void namesTheEditedNoteWithoutBreakingTheSentence_data();
    void namesTheEditedNoteWithoutBreakingTheSentence();
    void saysWhyTheSearchFieldRestsWhileEditing();
    void asksBeforeUnsavedChangesAreLost_data();
    void asksBeforeUnsavedChangesAreLost();
    void keepsTheEditorWhenTheListIsRebuiltUnderIt();
    void keepsTheEditorWhenTheWindowIsActivatedAgain();
    void keepsTheSavedNoteInTheResultListUntilTheSearchChanges();
    void keepsTheMeasuresOfTheEditState_data();
    void keepsTheMeasuresOfTheEditState();

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

    /** The label carrying `text`, or nullptr. */
    static QLabel *labelNamed(const QWidget &window, const QString &text);

    /** The reading pane and the text field of the edit state (wireframe 2a). */
    static QTextBrowser *readerOf(const QWidget &window);
    static QPlainTextEdit *editorOf(const QWidget &window);

    /** The page reader and editor sit on — the right half of the splitter. */
    static QWidget *detailOf(const QWidget &window);

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

    // Symbols come from the system theme, and the bare offscreen platform
    // brings no platform theme that would name one. Without this the check on
    // the dialog buttons could not tell „no symbol was asked for“ from „there
    // was no theme to ask“ — and it is the first of the two that it is meant
    // to catch. Breeze is the icon stock SPEC 15 builds on.
    // Measured on 02.08.2026: the theme name alone is not enough. Test mode
    // rewrites the XDG data locations, and the icon loader looks for themes
    // under those — so `QIcon::fromTheme()` came back null although breeze was
    // installed. The system icon directory has to be named as well.
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
    const std::optional<Note> stored = m_store->note(id);
    Q_ASSERT(stored.has_value());

    Note note = *stored;
    note.category = category;
    note.state = Note::State::Analysed;
    Q_ASSERT(m_store->updateNote(note));
    Q_ASSERT(m_store->setTags(id, tags));
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

QLabel *LibraryTest::labelNamed(const QWidget &window, const QString &text)
{
    const QList<QLabel *> labels = window.findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->text() == text) {
            return label;
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

QWidget *LibraryTest::detailOf(const QWidget &window)
{
    // Reader and editor share one stack; the page below it is the detail pane.
    QWidget *page = readerOf(window)->parentWidget()->parentWidget();
    Q_ASSERT(page);
    return page;
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
    // „Speichern“ and settled on „Abbrechen“ afterwards. The assertion on it
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
    for (QAbstractButton *button : buttons) {
        labels.append(KLocalizedString::removeAcceleratorMarker(button->text()));
    }
    return labels;
}

void LibraryTest::answerNextDialog(const QString &label)
{
    QTimer::singleShot(0, qApp, [label] {
        QDialog *dialog = waitForGuardDialog();
        QVERIFY2(dialog, "Der Wächterdialog ist nicht erschienen");

        QAbstractButton *button = dialogButton(dialog, label);
        QVERIFY2(button,
                 qPrintable(QStringLiteral("Der Wächterdialog bietet keine Antwort „%1“, sondern %2")
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
             qPrintable(QStringLiteral("Liste passt ganz ins Bild, der Fall tritt nicht ein")));

    // From the end of the list upwards across the boundary into "Gestern".
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
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));

    // The head of the new group is in the picture, whole rather than half cut
    // off — the selection never stands without its heading (wireframe 3b,
    // case 4) …
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
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

void LibraryTest::staysPutWhileTheSelectionMovesWithinItsGroup()
{
    // AK 7 and wireframe 3b, case 4 both say "springt die Auswahl über eine
    // Gruppengrenze". Within one group nothing is fetched, however far the
    // head may have scrolled away: the user rolled the list to where he wanted
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

    // The selection sits in "Gestern", and the user has rolled the list so
    // that the entry stands at the top with its head above the upper edge.
    list->setCurrentIndex(noteRow(list, 9));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));

    list->verticalScrollBar()->setValue(noteRow(list, 9).row());
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Kopf bei y=%1 — der Fall verlangt ihn außerhalb")
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
    // (PO decision of 01.08.2026, taken back after the case was measured).
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

    // Selection in "Letzte Woche", then the list rolled so that the last note
    // of "Gestern" stands in the picture while its head does not.
    list->setCurrentIndex(noteRow(list, 11));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));

    list->verticalScrollBar()->setValue(noteRow(list, 8).row());
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Kopf bei y=%1 — der Fall verlangt ihn außerhalb")
                            .arg(list->visualRect(head).y())));

    // The row the selection is about to reach is already in the picture, whole.
    const QModelIndex target = noteRow(list, 10);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(target)),
             qPrintable(QStringLiteral("Zielzeile y=%1 — der Fall verlangt sie ganz im Bild")
                            .arg(list->visualRect(target).y())));

    QTest::keyClick(list, Qt::Key_Up);

    // It crossed a group boundary, so the head is in the picture afterwards —
    // never mind that nothing had to be scrolled for the note itself.
    QCOMPARE(list->currentIndex(), target);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Kopf bei y=%1, Viewport %2 px hoch")
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
    // the customer reported it, and the measurement said why: the row he
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
    // of "Gestern" to leave the picture at all.
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
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));

    list->verticalScrollBar()->setValue(noteRow(list, 8).row());
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));

    const QModelIndex target = noteRow(list, 10);
    QVERIFY(list->viewport()->rect().contains(list->visualRect(target)));

    const int rolledTo = list->verticalScrollBar()->value();
    const int targetBefore = list->visualRect(target).y();

    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(target).center());

    QCOMPARE(list->currentIndex(), target);

    // The picture has not moved: the roll value is the one the user had rolled
    // to, and the row he pointed at is still under his finger.
    QCOMPARE(list->verticalScrollBar()->value(), rolledTo);
    QCOMPARE(list->visualRect(target).y(), targetBefore);

    // The head stays outside, and that is the price of it. The day is not lost
    // with it: the reading pane carries the full timestamp of what was picked,
    // so what the head would have said stands in the window anyway (UX note on
    // issue #57).
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(head)),
             qPrintable(QStringLiteral("Kopf bei y=%1").arg(list->visualRect(head).y())));
    QVERIFY(visibleLabels(window).contains(QStringLiteral("Gestern 09:00")));
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
    // Woche", the head of "Gestern" rolled out of the picture, the row the key
    // is about to reach already in view.
    list->setCurrentIndex(noteRow(list, 11));
    const QModelIndex head = modelOf(list)->index(noteRow(list, 8).row() - 1);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));

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
    QVERIFY2(visibleHead.isValid(), "Kein Gruppenkopf im Bild — der Fall tritt nicht ein");

    const QModelIndex before = list->currentIndex();
    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualRect(visibleHead).center());
    QCOMPARE(list->currentIndex(), before);

    QTest::keyClick(list, Qt::Key_Up);

    // The key crossed a group boundary, so the head is in the picture — the
    // click before it changed nothing and counts for nothing.
    QCOMPARE(list->currentIndex(), target);
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Kopf bei y=%1").arg(list->visualRect(head).y())));
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
    // "Gestern" — its head is two rows above the selection, not one.
    list->setCurrentIndex(noteRow(list, 18));
    for (int step = 0; step < 9; ++step) {
        QTest::keyClick(list, Qt::Key_Up);
    }

    const QModelIndex selected = noteRow(list, 9);
    QCOMPARE(list->currentIndex(), selected);
    QCOMPARE(selected.data(Qt::DisplayRole).toString(), QStringLiteral("von gestern, 10 Uhr"));

    const QModelIndex head = modelOf(list)->index(selected.row() - 2);
    QCOMPARE(head.data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));
    QVERIFY2(!modelOf(list)->index(selected.row() - 1).data(NoteListModel::GroupHeaderRole).toBool(),
             "Der Fall verlangt eine Notiz, über der kein Kopf steht");

    // Head and selection fit into the list together, so both are in the
    // picture, whole.
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(head)),
             qPrintable(QStringLiteral("Kopf bei y=%1, Viewport %2 px hoch")
                            .arg(list->visualRect(head).y())
                            .arg(list->viewport()->height())));
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(selected)),
             qPrintable(QStringLiteral("Auswahl y=%1 h=%2, Viewport %3 px hoch")
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
             "Der Fall verlangt eine Zielzeile, die schon ganz im Bild steht");

    const int scrolledTo = list->verticalScrollBar()->value();
    QTest::keyClick(list, Qt::Key_Up);
    QCOMPARE(list->verticalScrollBar()->value(), scrolledTo);

    QCOMPARE(list->currentIndex(), selected);

    // The head is further away than the list is tall …
    const int span = list->visualRect(selected).bottom() - list->visualRect(head).top();
    QVERIFY2(span > list->viewport()->height(),
             qPrintable(QStringLiteral("Kopf und Auswahl umspannen %1 px, Viewport %2 px — der Fall "
                                       "tritt nicht ein")
                            .arg(span)
                            .arg(list->viewport()->height())));

    // … so it stays outside, and the selection keeps the picture, whole.
    QVERIFY(!list->viewport()->rect().intersects(list->visualRect(head)));
    QVERIFY2(list->viewport()->rect().contains(list->visualRect(selected)),
             qPrintable(QStringLiteral("Auswahl y=%1 h=%2, Viewport %3 px hoch")
                            .arg(list->visualRect(selected).y())
                            .arg(list->visualRect(selected).height())
                            .arg(list->viewport()->height())));
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

    // The selection is in "Gestern", well below row 0 …
    const QModelIndex selected = noteRow(list, 12);
    list->setCurrentIndex(selected);
    QVERIFY(selected.row() > 0);

    // … and the user has rolled the list back to the top, away from it.
    list->verticalScrollBar()->setValue(0);
    QVERIFY2(!list->viewport()->rect().intersects(list->visualRect(selected)),
             "Der Fall verlangt die Auswahl außerhalb des Bildes");
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

void LibraryTest::showsTheSearchField()
{
    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QLineEdit *search = searchOf(window);
    QVERIFY(search->isVisible());
    QCOMPARE(search->placeholderText(), QStringLiteral("Volltextsuche …"));

    // S5 left the field switched off and explained that in a tooltip. This
    // story switches it on, so neither may be left over.
    QVERIFY(search->isEnabled());
    const QList<QWidget *> widgets = window.findChildren<QWidget *>();
    for (const QWidget *widget : widgets) {
        QVERIFY2(!widget->toolTip().contains(QStringLiteral("steht noch nicht zur Verfügung")),
                 qPrintable(widget->toolTip()));
    }
}

void LibraryTest::filtersTheListWithTheSearchField()
{
    storedNote(QStringLiteral("Bücher über Straßenbahnen ansehen"));
    storedNote(QStringLiteral("Backup der Fotos prüfen"));
    storedNote(QStringLiteral("Milch kaufen"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
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

void LibraryTest::groupsTheSearchResultsLikeTheLibrary()
{
    // Two notes of today, one of a day the grouping puts elsewhere.
    storedNote(QStringLiteral("Backup heute früh"), QStringLiteral("2026-07-31T08:00:00"));
    storedNote(QStringLiteral("Milch kaufen"), QStringLiteral("2026-07-31T07:00:00"));
    storedNote(QStringLiteral("Backup vom Vortag"), QStringLiteral("2026-07-30T09:00:00"));

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    NoteListModel *model = modelOf(listOf(window));
    QCOMPARE(rowsOf(*model),
             QStringList({QStringLiteral("Kopf: Heute"),
                          QStringLiteral("Notiz: Backup heute früh"),
                          QStringLiteral("Notiz: Milch kaufen"),
                          QStringLiteral("Kopf: Gestern"),
                          QStringLiteral("Notiz: Backup vom Vortag")}));

    // The result list carries the same heads — and the group whose only note
    // dropped out of the results loses its head with it.
    searchOf(window)->setText(QStringLiteral("Backup"));
    QCOMPARE(rowsOf(*model),
             QStringList({QStringLiteral("Kopf: Heute"),
                          QStringLiteral("Notiz: Backup heute früh"),
                          QStringLiteral("Kopf: Gestern"),
                          QStringLiteral("Notiz: Backup vom Vortag")}));

    searchOf(window)->setText(QStringLiteral("Milch"));
    QCOMPARE(rowsOf(*model),
             QStringList({QStringLiteral("Kopf: Heute"), QStringLiteral("Notiz: Milch kaufen")}));
}

void LibraryTest::showsTheEmptyStateWhenNothingMatches()
{
    storedNote(QStringLiteral("Milch kaufen"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    searchOf(window)->setText(QStringLiteral("Fahrrad"));

    const QStringList visible = visibleLabels(window);
    QVERIFY2(visible.contains(QStringLiteral("Keine Treffer")), qPrintable(visible.join(QLatin1Char('|'))));
    QVERIFY(visible.contains(QStringLiteral("Den Suchbegriff ändern oder das Feld leeren.")));

    // The list itself is out of sight, and the detail area stays empty — the
    // list column says it once, as in wireframe 2c.
    QVERIFY(!listOf(window)->isVisible());
    QVERIFY2(!visible.contains(QStringLiteral("Keine Notiz ausgewählt")),
             qPrintable(visible.join(QLatin1Char('|'))));

    // The empty library says something else: no notes at all is not the same
    // as none that match.
    QVERIFY(!visible.contains(QStringLiteral("Noch keine Notizen")));
}

void LibraryTest::restoresTheFullListWhenTheSearchFieldIsCleared()
{
    storedNote(QStringLiteral("Bücher ansehen"));
    storedNote(QStringLiteral("Milch kaufen"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);

    searchOf(window)->setText(QStringLiteral("Fahrrad"));
    QCOMPARE(modelOf(list)->noteCount(), 0);
    QVERIFY(!list->isVisible());

    searchOf(window)->clear();

    QCOMPARE(modelOf(list)->noteCount(), 2);
    QVERIFY(list->isVisible());
    QVERIFY(!visibleLabels(window).contains(QStringLiteral("Keine Treffer")));
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

    QWidget *header = searchOf(window)->parentWidget();
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
    const QPoint searchTopLeft = searchOf(window)->mapTo(&window, QPoint());
    QVERIFY2(searchTopLeft.y() < 40, qPrintable(QStringLiteral("Suchfeld bei y=%1").arg(searchTopLeft.y())));

    // The other axis, same class of mistake: wireframe 2b gives the list a
    // fixed width and the reading pane the rest, so the same number has to come
    // out at both window widths.
    QCOMPARE(splitter->widget(0)->width(), 300);
}

void LibraryTest::keepsTheMeasuresOfTheGroupedList_data()
{
    // The measurements of wireframe 3a are checked at two window sizes, like
    // the room split of 2b: a number that only holds at one width holds by
    // accident.
    QTest::addColumn<QSize>("windowSize");

    QTest::newRow("900x600") << QSize(900, 600);
    QTest::newRow("1200x800") << QSize(1200, 800);
}

void LibraryTest::keepsTheMeasuresOfTheGroupedList()
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

    // Text starts 12 px from the left edge of the list (wireframe 3a).
    //
    // That head and entry start at the same place is deliberately not checked
    // here: both would ask the same function about rectangles of the same
    // width, and the comparison could never fail. Where a delegate puts its
    // ink is invisible to a test that does not count pixels, so the alignment
    // is held by the delegate instead — head, timestamp, subject and preview
    // go through one single drawing function, and the edge is worked out in
    // one single place. What this checks is the number that place uses.
    const QRect headRect = list->visualRect(firstHead);
    const QRect entryRect = list->visualRect(noteRow(list, 0));
    QCOMPARE(NoteListDelegate::textLeft(entryRect) - list->viewport()->rect().x(), 12);

    // The first head sits close under the upper edge, every following one
    // keeps the larger distance that separates it from the group above
    // (wireframe 3a: 6 px against 14 px above the text).
    QVERIFY2(headRect.height() < list->visualRect(secondHead).height(),
             qPrintable(QStringLiteral("erster Kopf %1 px, zweiter %2 px")
                            .arg(headRect.height())
                            .arg(list->visualRect(secondHead).height())));
    QCOMPARE(list->visualRect(secondHead).height() - headRect.height(), 8);

    // Not checked here, and not checkable without measuring pixels: that the
    // subject is drawn in text colour, the preview dimmed, both in
    // HighlightedText while selected, and neither in bold. Like the left text
    // edge above, that is held by the delegate having a single painting
    // function (notelistdelegate.cpp) — and by the picture series of the UI
    // review, which is what a colour is decided on anyway.

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

    QWidget *header = searchOf(window)->parentWidget();
    QVERIFY(header);
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

void LibraryTest::textsFollowAColourSchemeChange()
{
    // The daemon builds this window at start and keeps it (SPEC 2.1, main.cpp),
    // so a colour scheme changed underneath it reaches a window that is already
    // standing — and stays there until the daemon is restarted. Every text has
    // to follow it (issue #58, the second site of issue #54).
    //
    // Asked of every label rather than of the two known ones: the point of the
    // fault is that it spreads by copying, and a check that names its sites
    // cannot see the next copy.
    storedNote(QStringLiteral("eine Notiz von gestern"), QStringLiteral("2026-07-30T11:00:00"));

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    const QPalette startPalette = qApp->palette();

    QPalette switched = startPalette;
    switched.setColor(QPalette::WindowText, QColor(0x23, 0x26, 0x29));
    switched.setColor(QPalette::PlaceholderText, QColor(0x70, 0x7d, 0x8a));
    switched.setColor(QPalette::Link, QColor(0x2d, 0x7d, 0x9a));
    qApp->setPalette(switched);

    // Qt hands the new palette to the widgets through a posted event; without a
    // running event loop the test has to let it through itself. Without this the
    // test stays red after the correction and looks like a failed fix
    // (CaptureTest::textsFollowAColourSchemeChange, issue #54).
    QCoreApplication::processEvents();

    // The message band paints its own colours by message type — that is
    // KMessageWidget's business, not this window's.
    auto *message = window.findChild<KMessageWidget *>();
    QVERIFY(message);

    const QList<QLabel *> labels = window.findChildren<QLabel *>();
    QVERIFY(!labels.isEmpty());

    for (QLabel *label : labels) {
        if (message->isAncestorOf(label)) {
            continue;
        }

        // What the label paints with: its own palette, read through its role.
        // A colour written into that palette once would still stand here.
        QVERIFY2(label->palette().color(label->foregroundRole())
                     == switched.color(label->foregroundRole()),
                 qPrintable(QStringLiteral("„%1“ malt in %2, das Schema sagt %3")
                                .arg(label->text(),
                                     label->palette().color(label->foregroundRole()).name(),
                                     switched.color(label->foregroundRole()).name())));
    }

    qApp->setPalette(startPalette);
}

void LibraryTest::opensTheEditorWithTheButton()
{
    storedNote(QStringLiteral("Transkript mit einem Hörfehler"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    // Wireframe 2a, state A: the way in is a visible button, not a hidden key.
    QPushButton *edit = buttonNamed(window, QStringLiteral("Bearbeiten"));
    QVERIFY2(edit, "Der Detailbereich zeigt keine Schaltfläche „Bearbeiten“");
    QVERIFY(edit->isVisible());

    edit->click();

    QVERIFY(editorOf(window)->isVisible());
    QVERIFY(!readerOf(window)->isVisible());
    QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("Transkript mit einem Hörfehler"));
}

void LibraryTest::opensTheEditorWithF2()
{
    storedNote(QStringLiteral("Transkript mit einem Hörfehler"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));

    QTest::keyClick(list, Qt::Key_F2);

    QVERIFY(editorOf(window)->isVisible());
    QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("Transkript mit einem Hörfehler"));
}

void LibraryTest::leavesTheWordSelectionToTheDoubleClick()
{
    storedNote(QStringLiteral("Vault statt Fold"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    QTextBrowser *reader = readerOf(window);
    QTest::mouseDClick(reader->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(6, 6));

    // Wireframe 2a: a double click picks a word — the common way of copying
    // something out of a note. A third way into the editor is not worth it.
    QVERIFY2(!editorOf(window)->isVisible(), "Der Doppelklick hat das Bearbeiten geöffnet");
    QVERIFY2(reader->textCursor().hasSelection(), "Die Wortauswahl ist verlorengegangen");
}

void LibraryTest::putsTheCursorAtTheEndWithoutSelectingTheText()
{
    const QString content = QStringLiteral("Transkript mit einem Hörfehler");
    storedNote(content);

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();

    // Wireframe 2a: the cursor stands at the end and nothing is selected —
    // the first keystroke must not be able to overwrite the note.
    const QTextCursor cursor = editorOf(window)->textCursor();
    QVERIFY2(!cursor.hasSelection(), "Der ganze Text ist markiert");
    QCOMPARE(cursor.position(), static_cast<int>(content.size()));
}

void LibraryTest::showsCategoryAndTagsAsPlainDisplayWhileEditing()
{
    const qint64 id = storedNote(QStringLiteral("Idee für Denkzettel"));
    analysed(id,
             QStringLiteral("Software-Ideen"),
             QStringList({QStringLiteral("software-idee"), QStringLiteral("denkzettel")}));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    // While reading, the edit state's rows are away — they belong to state B.
    QVERIFY(!labelNamed(window, QStringLiteral("Kategorie"))->isVisible());

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();

    const QStringList visible = visibleLabels(window);
    QVERIFY2(visible.contains(QStringLiteral("Software-Ideen")), qPrintable(visible.join(QLatin1Char('|'))));
    // In the order the store hands them out, which is alphabetical.
    QVERIFY2(visible.contains(QStringLiteral("denkzettel · software-idee")),
             qPrintable(visible.join(QLatin1Char('|'))));
    QVERIFY(visible.contains(QStringLiteral("Kategorie")));
    QVERIFY(visible.contains(QStringLiteral("Tags")));

    // Wireframe 2a: as plain display, deliberately not as greyed-out input
    // fields — those would promise an editing that does not exist. The note
    // text is the only input field of the pane.
    const QList<QLineEdit *> fields = detailOf(window)->findChildren<QLineEdit *>();
    QVERIFY2(fields.isEmpty(), "Kategorie oder Tags stehen in einem Eingabefeld");
    QCOMPARE(detailOf(window)->findChildren<QPlainTextEdit *>().size(), 1);
}

void LibraryTest::putsTheEditingBadgeWhereTheButtonsStand()
{
    storedNote(QStringLiteral("Idee für Denkzettel"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    QPushButton *edit = buttonNamed(window, QStringLiteral("Bearbeiten"));
    QPushButton *remove = buttonNamed(window, QStringLiteral("Löschen"));
    QVERIFY(edit);
    QVERIFY(remove);
    QVERIFY(edit->isVisible());
    QVERIFY(remove->isVisible());
    QVERIFY(!visibleLabels(window).contains(QStringLiteral("wird bearbeitet")));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();

    // Wireframe 2a, state B: the head says that the note is being edited where
    // the two buttons stand while reading.
    QVERIFY2(visibleLabels(window).contains(QStringLiteral("wird bearbeitet")),
             qPrintable(visibleLabels(window).join(QLatin1Char('|'))));
    QVERIFY(!edit->isVisible());
    QVERIFY(!remove->isVisible());

    // With the button gone the key must not delete either — the note under the
    // editor is not up for deletion.
    QVERIFY(!actionNamed(window, QStringLiteral("Löschen"))->isEnabled());
}

void LibraryTest::savesTheChangedTextWithTheButton()
{
    const qint64 id = storedNote(QStringLiteral("sonst wird der Fold zugemüllt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("sonst wird der Vault zugemüllt"));

    buttonNamed(window, QStringLiteral("Speichern"))->click();

    QCOMPARE(m_store->note(id)->content, QStringLiteral("sonst wird der Vault zugemüllt"));

    // The answer is the way back into the reading state — and nothing else:
    // no success message, no band under the header (wireframe 2a).
    QVERIFY(!editorOf(window)->isVisible());
    QVERIFY(readerOf(window)->isVisible());
    QCOMPARE(readerOf(window)->toPlainText(), QStringLiteral("sonst wird der Vault zugemüllt"));
    QVERIFY(buttonNamed(window, QStringLiteral("Bearbeiten"))->isVisible());

    auto *message = window.findChild<KMessageWidget *>();
    QVERIFY(message);
    QVERIFY2(!message->isVisible(), "Nach dem Speichern erscheint eine Erfolgsmeldung");

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

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
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

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("Idee für Denkzettel, überarbeitet"));
    buttonNamed(window, QStringLiteral("Speichern"))->click();

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

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("Idee für Denkzettel, überarbeitet"));
    buttonNamed(window, QStringLiteral("Speichern"))->click();

    // SPEC 9 / 7.2: the embedding ages with the text, so the next analysis run
    // renews it — and only it.
    QVERIFY2(m_store->note(id)->needsReembed, "needs_reembed steht nach dem Speichern nicht auf 1");
}

void LibraryTest::findsTheSavedTextInTheSearchIndex()
{
    storedNote(QStringLiteral("sonst wird der Fold zugemüllt"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("sonst wird der Vault zugemüllt"));
    buttonNamed(window, QStringLiteral("Speichern"))->click();

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

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("Transkript ohne Hörfehler"));
    buttonNamed(window, QStringLiteral("Speichern"))->click();

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

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    QPlainTextEdit *editor = editorOf(window);
    editor->setPlainText(QStringLiteral("   \n  "));

    // Wireframe 2a: an empty field is no valid state to save. Deleting runs
    // over the delete action, not over emptying the field.
    QVERIFY2(!buttonNamed(window, QStringLiteral("Speichern"))->isEnabled(),
             "„Speichern“ ist bei leerem Feld auslösbar");

    QTest::keyClick(editor, Qt::Key_Return, Qt::ControlModifier);

    QCOMPARE(m_store->note(id)->content, QStringLiteral("bleibt so stehen"));
    QVERIFY2(editor->isVisible(), "Strg+Enter hat den leeren Text gespeichert");

    // Filling it again makes the button live once more.
    editor->setPlainText(QStringLiteral("doch etwas"));
    QVERIFY(buttonNamed(window, QStringLiteral("Speichern"))->isEnabled());
}

void LibraryTest::leavesTheEditorWithoutAskingWhenNothingWasChanged()
{
    storedNote(QStringLiteral("unverändert"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    QVERIFY(editorOf(window)->isVisible());

    // Nothing to lose, so nothing to ask about: Esc goes straight back. Were a
    // dialog to come up here, exec() would hang this test.
    QTest::keyClick(editorOf(window), Qt::Key_Escape);

    QVERIFY(!editorOf(window)->isVisible());
    QVERIFY(readerOf(window)->isVisible());

    // The button beside it is the same action and answers the same way.
    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    QVERIFY(editorOf(window)->isVisible());
    buttonNamed(window, QStringLiteral("Abbrechen"))->click();
    QVERIFY(!editorOf(window)->isVisible());
}

void LibraryTest::namesTheThreeAnswersOfTheGuardDialog()
{
    storedNote(QStringLiteral("wird geändert"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("geändert"));

    // The dialog is read out of the running window rather than described from
    // memory — and out of the window the *user* gets: activeModalWidget(), not
    // the object this window built. Under QT_QPA_PLATFORMTHEME=kde those were
    // two different objects as long as the guard was a QMessageBox, and the
    // one on screen carried none of the symbols set on ours (issue #66).
    QStringList labels;
    QStringList icons;
    QString defaultAnswer;
    QTimer::singleShot(0, qApp, [&labels, &icons, &defaultAnswer] {
        QDialog *dialog = waitForGuardDialog();
        QVERIFY(dialog);

        const QList<QAbstractButton *> buttons = dialogAnswerButtons(dialog);
        QVERIFY(!buttons.isEmpty());
        for (QAbstractButton *button : buttons) {
            labels.append(KLocalizedString::removeAcceleratorMarker(button->text()));
            icons.append(button->icon().name());
            if (auto *push = qobject_cast<QPushButton *>(button); push && push->isDefault()) {
                defaultAnswer = KLocalizedString::removeAcceleratorMarker(button->text());
            }
        }
        buttons.constFirst()->click();
    });

    QTest::keyClick(editorOf(window), Qt::Key_Escape);

    QCOMPARE(labels.size(), 3);
    QVERIFY2(labels.contains(QStringLiteral("Speichern")), qPrintable(labels.join(QLatin1Char('|'))));
    QVERIFY(labels.contains(QStringLiteral("Verwerfen")));
    QVERIFY(labels.contains(QStringLiteral("Abbrechen")));

    // Three similarly long German words side by side are told apart fastest by
    // their symbol, and the destructive one carries the marking KDE gives a
    // destructive action (UI review of 02.08.2026, finding 5). The names are
    // asked for rather than the pixels: whether the theme resolves them is the
    // theme's business, and an empty name means no symbol was ever asked for.
    // The three names are the ones the drawing lays down (wireframe 2a, table
    // „Symbole an den Schaltflächen“, state C).
    QCOMPARE(icons.at(labels.indexOf(QStringLiteral("Speichern"))), QStringLiteral("document-save"));
    QCOMPARE(icons.at(labels.indexOf(QStringLiteral("Verwerfen"))), QStringLiteral("edit-delete"));
    QCOMPARE(icons.at(labels.indexOf(QStringLiteral("Abbrechen"))), QStringLiteral("dialog-cancel"));

    // The default answer is „Speichern“, and it has to be said out loud: the
    // KDE build puts the default on the cancel button (measured 02.08.2026,
    // PO decision F3). Saving meets the likely intention of someone who has
    // just typed — and it is the answer that loses nothing either way.
    QCOMPARE(defaultAnswer, QStringLiteral("Speichern"));
}

void LibraryTest::showsTheWarningSymbolInTheGuardDialog()
{
    storedNote(QStringLiteral("wird geändert"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("geändert"));

    // Wireframe 2a, state C: a dialog about losing work carries the warning
    // symbol. It is the one symbol the platform's substitute dialog did have,
    // and the story that heals the missing symbols must not take the largest
    // one away (PO decision of 02.08.2026 on the handover finding).
    //
    // The measurement is the picture label, not a name: the symbol sits in a
    // QLabel, and a pixmap has no name to ask for. Empty or hidden means the
    // dialog shows none — which is exactly what KMessageDialog does when no
    // icon is handed to it, its documentation notwithstanding.
    QSize symbol;
    QTimer::singleShot(0, qApp, [&symbol] {
        QDialog *dialog = waitForGuardDialog();
        QVERIFY(dialog);

        const QList<QLabel *> labels = dialog->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->isVisible() && !label->pixmap().isNull()) {
                symbol = label->pixmap().size();
            }
        }

        QAbstractButton *cancel = dialogButton(dialog, QStringLiteral("Abbrechen"));
        QVERIFY(cancel);
        cancel->click();
    });

    QTest::keyClick(editorOf(window), Qt::Key_Escape);

    QVERIFY2(!symbol.isEmpty(),
             "Der Wächterdialog zeigt kein Warnsymbol — kein sichtbares Etikett trägt ein Bild");
}

void LibraryTest::namesTheSymbolsOfTheDetailButtons()
{
    storedNote(QStringLiteral("trägt Symbole"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    // Wireframe 2a, table „Symbole an den Schaltflächen“: what is laid down is
    // the *name*, not the drawn shape — the graphic comes from the icon theme
    // and changes with it. An empty name means no symbol was ever asked for,
    // which is what the customer saw („Die Bibliothek hat keine Icons“, #67).
    QPushButton *edit = buttonNamed(window, QStringLiteral("Bearbeiten"));
    QPushButton *remove = buttonNamed(window, QStringLiteral("Löschen"));
    QVERIFY(edit);
    QVERIFY(remove);
    QCOMPARE(edit->icon().name(), QStringLiteral("document-edit"));
    QCOMPARE(remove->icon().name(), QStringLiteral("edit-delete"));

    // The symbol steps beside the label, never in its place (KDE HIG). A
    // symbol-only button would not appear anywhere in this window.
    QCOMPARE(edit->text(), QStringLiteral("Bearbeiten"));
    QCOMPARE(remove->text(), QStringLiteral("Löschen"));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();

    // State B, the edit footer. Same two symbols the guard dialog uses for the
    // same two answers — the window says the same thing in both places.
    QPushButton *save = buttonNamed(window, QStringLiteral("Speichern"));
    QPushButton *cancel = buttonNamed(window, QStringLiteral("Abbrechen"));
    QVERIFY(save);
    QVERIFY(cancel);
    QCOMPARE(save->icon().name(), QStringLiteral("document-save"));
    QCOMPARE(cancel->icon().name(), QStringLiteral("dialog-cancel"));
    QCOMPARE(save->text(), QStringLiteral("Speichern"));
    QCOMPARE(cancel->text(), QStringLiteral("Abbrechen"));
}

void LibraryTest::namesTheSymbolOfTheUndoAction()
{
    storedNote(QStringLiteral("wird gelöscht"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    QTest::keyClick(list, Qt::Key_Delete);

    auto *message = window.findChild<KMessageWidget *>();
    QVERIFY(message);
    QTRY_VERIFY(message->isVisible() && !message->isShowAnimationRunning());

    // The fourth labelled control of the library and the only one outside the
    // detail pane (wireframe 2b). Read off the button KMessageWidget builds
    // rather than off the QAction behind it: the button is what is on screen,
    // and it is the button that would go without a symbol if the widget stopped
    // passing it on.
    QToolButton *undo = nullptr;
    const QList<QToolButton *> buttons = message->findChildren<QToolButton *>();
    for (QToolButton *button : buttons) {
        if (button->text() == QStringLiteral("Rückgängig")) {
            undo = button;
        }
    }
    QVERIFY(undo);
    QCOMPARE(undo->icon().name(), QStringLiteral("edit-undo"));
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
    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("halb getippt"));

    QModelIndex selectedWhileAsking;
    QTimer::singleShot(0, qApp, [&selectedWhileAsking, list] {
        QDialog *dialog = waitForGuardDialog();
        QVERIFY(dialog);

        selectedWhileAsking = list->currentIndex();

        QAbstractButton *cancel = dialogButton(dialog, QStringLiteral("Abbrechen"));
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

void LibraryTest::namesTheEditedNoteWithoutBreakingTheSentence_data()
{
    // The timestamp goes into the sentence in the form its group gives it, and
    // the sentence has to carry every one of them — „Die Notiz von Heute 11:05
    // wurde geändert“ carried none (UI review of 02.08.2026, finding 3).
    //
    // Both sentences stand in one text since issue #66: KMessageDialog has no
    // informative text beside the main one. The question keeps its place at the
    // top, the explanation below it, and the blank line between them does what
    // the second field did (wireframe 2a, state C).
    QTest::addColumn<QString>("written");
    QTest::addColumn<QString>("expected");

    QTest::newRow("heute") << QStringLiteral("2026-07-31T11:05:00")
                           << QStringLiteral("Änderungen speichern?\n\nDie bearbeitete Notiz (Heute 11:05) "
                                             "hat ungespeicherte Änderungen. Ohne Speichern gehen sie "
                                             "verloren.");
    QTest::newRow("älter") << QStringLiteral("2026-07-19T09:00:00")
                           << QStringLiteral("Änderungen speichern?\n\nDie bearbeitete Notiz (19.07.2026) "
                                             "hat ungespeicherte Änderungen. Ohne Speichern gehen sie "
                                             "verloren.");
}

void LibraryTest::namesTheEditedNoteWithoutBreakingTheSentence()
{
    QFETCH(QString, written);
    QFETCH(QString, expected);

    storedNote(QStringLiteral("wird bearbeitet"), written);

    LibraryWindow window(m_store.get());
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));
    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("halb getippt"));

    // Every line the dialog shows, so the comparison below is an equality
    // against the whole message and not a search for the words it expects.
    QStringList shown;
    QTimer::singleShot(0, qApp, [&shown] {
        QDialog *dialog = waitForGuardDialog();
        QVERIFY(dialog);

        const QList<QLabel *> labels = dialog->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (!label->text().isEmpty()) {
                shown.append(label->text());
            }
        }

        QAbstractButton *cancel = dialogButton(dialog, QStringLiteral("Abbrechen"));
        QVERIFY(cancel);
        cancel->click();
    });

    QTest::keyClick(editorOf(window), Qt::Key_Escape);

    QVERIFY2(shown.contains(expected), qPrintable(shown.join(QStringLiteral(" ⏎ "))));
}

void LibraryTest::saysWhyTheSearchFieldRestsWhileEditing()
{
    storedNote(QStringLiteral("wird bearbeitet"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));
    QVERIFY(searchOf(window)->isEnabled());
    QVERIFY(searchOf(window)->toolTip().isEmpty());

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();

    // The KDE HIG argue against controls that are switched off without a
    // visible reason. That the field rests is plain to see, why it rests was
    // not (UI review of 02.08.2026, finding 4).
    QVERIFY(!searchOf(window)->isEnabled());
    QVERIFY2(!searchOf(window)->toolTip().isEmpty(), "Das ruhende Suchfeld sagt nicht, warum");

    // Nothing was changed, so Esc goes straight back — and the hint goes with
    // the state that explains it.
    QTest::keyClick(editorOf(window), Qt::Key_Escape);

    QVERIFY(searchOf(window)->isEnabled());
    QVERIFY2(searchOf(window)->toolTip().isEmpty(), "Die Kurzhilfe bleibt am wieder freien Feld stehen");
}

void LibraryTest::asksBeforeUnsavedChangesAreLost_data()
{
    // Three ways out of the edit state and three answers to each — the matrix
    // of wireframe 2a, state C. Leaving one column out would leave one way of
    // losing a correction unwatched.
    //
    // The answers are named by their label, not by the button role behind them
    // (issue #66). What the wireframe fixes is their *meaning* — „Speichern“
    // writes and carries the act out, „Verwerfen“ carries it out without
    // writing, „Abbrechen“ stays in the editor — while roles and order belong
    // to the platform. A matrix that both clicks and expects by role would
    // survive two answers swapping their labels.
    QTest::addColumn<QString>("trigger");
    QTest::addColumn<QString>("answer");

    for (const QString &trigger :
         {QStringLiteral("auswahlwechsel"), QStringLiteral("fensterschliessen"), QStringLiteral("esc")}) {
        for (const QString &answer :
             {QStringLiteral("Speichern"), QStringLiteral("Verwerfen"), QStringLiteral("Abbrechen")}) {
            QTest::newRow(qPrintable(trigger + QLatin1Char('-') + answer.toLower())) << trigger << answer;
        }
    }
}

void LibraryTest::asksBeforeUnsavedChangesAreLost()
{
    QFETCH(QString, trigger);
    QFETCH(QString, answer);

    const qint64 edited = storedNote(QStringLiteral("erste Notiz"));
    storedNote(QStringLiteral("zweite Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(noteRow(list, 0));
    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("erste Notiz, berichtigt"));

    answerNextDialog(answer);

    if (trigger == QStringLiteral("auswahlwechsel")) {
        list->setCurrentIndex(noteRow(list, 1));
    } else if (trigger == QStringLiteral("fensterschliessen")) {
        window.close();
    } else {
        QTest::keyClick(editorOf(window), Qt::Key_Escape);
    }

    // „Speichern“ writes, „Verwerfen“ and „Abbrechen“ leave the note alone.
    const QString stored = m_store->note(edited)->content;
    if (answer == QStringLiteral("Speichern")) {
        QCOMPARE(stored, QStringLiteral("erste Notiz, berichtigt"));
    } else {
        QCOMPARE(stored, QStringLiteral("erste Notiz"));
    }

    // „Speichern“ and „Verwerfen“ carry the triggering act out, „Abbrechen“
    // stays in the edit state and takes the act back.
    const bool carriedOut = answer != QStringLiteral("Abbrechen");

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

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("halb getippt"));

    // Meta+N while the library stands open: the new note takes the top row and
    // pushes the edited one down. Its row number changes, the note does not —
    // and a question about unsaved changes here would be a question about
    // nothing.
    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    DialogWatch watch;
    window.showLibrary();
    QTest::qWait(100);

    // The row really did move — otherwise this test would prove nothing.
    QCOMPARE(modelOf(list)->noteCount(), 2);
    QVERIFY2(list->currentIndex() != before, "Die Zeile ist gar nicht verrutscht");

    QVERIFY2(!watch.appeared(), "Der Wächterdialog kam, obwohl die Notiz dieselbe geblieben ist");
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
    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("halb getippt"));

    // The window stood open past midnight and is looked at again: the
    // activation regroups the list, and the regrouping rebuilds every row
    // under the editor. The note stays the same one, so the question must not
    // come up — a user coming back from another window would otherwise be
    // asked about a change he has not finished making.
    window.setReferenceTime(at(QStringLiteral("2026-08-01T09:00:00")));

    DialogWatch watch;

    QWidget elsewhere;
    elsewhere.show();
    elsewhere.activateWindow();
    QTRY_VERIFY(!window.isActiveWindow());

    window.activateWindow();
    QTRY_VERIFY(window.isActiveWindow());
    QTest::qWait(100);

    // The regrouping really did happen — otherwise nothing was rebuilt here.
    QCOMPARE(modelOf(list)->index(0).data(Qt::DisplayRole).toString(), QStringLiteral("Gestern"));

    QVERIFY2(!watch.appeared(), "Der Wächterdialog kam beim Zurückkommen ans Fenster");
    QVERIFY(editorOf(window)->isVisible());
    QCOMPARE(editorOf(window)->toPlainText(), QStringLiteral("halb getippt"));
    QCOMPARE(list->currentIndex(), noteRow(list, 0));
}

void LibraryTest::keepsTheSavedNoteInTheResultListUntilTheSearchChanges()
{
    // A list long enough to be rolled: only then can a jump show at all.
    for (int number = 0; number < 20; ++number) {
        storedNote(QStringLiteral("Straßenbahn Nummer %1").arg(number, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(m_store.get());
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    searchOf(window)->setText(QStringLiteral("Straßenbahn"));
    QCOMPARE(modelOf(list)->noteCount(), 20);

    list->setCurrentIndex(noteRow(list, 10));
    QTest::qWait(50);
    const int rolledTo = list->verticalScrollBar()->value();
    const QStringList before = rowsOf(*modelOf(list));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    editorOf(window)->setPlainText(QStringLiteral("Fahrkarte Nummer 10"));
    buttonNamed(window, QStringLiteral("Speichern"))->click();

    // The saved note no longer matches the running term, and it stays all the
    // same: it keeps its row, its selection and the roll value (issue #11, K2).
    QCOMPARE(modelOf(list)->noteCount(), 20);
    QCOMPARE(list->currentIndex(), noteRow(list, 10));
    QCOMPARE(noteRow(list, 10).data(Qt::DisplayRole).toString(), QStringLiteral("Fahrkarte Nummer 10"));
    QCOMPARE(list->verticalScrollBar()->value(), rolledTo);

    QStringList expected = before;
    expected[expected.indexOf(QStringLiteral("Notiz: Straßenbahn Nummer 10"))] =
        QStringLiteral("Notiz: Fahrkarte Nummer 10");
    QCOMPARE(rowsOf(*modelOf(list)), expected);

    // The next change of the term reads the store again, and there it is gone.
    searchOf(window)->setText(QStringLiteral("Straßenbahn Nummer"));
    QCOMPARE(modelOf(list)->noteCount(), 19);
}

void LibraryTest::keepsTheMeasuresOfTheEditState_data()
{
    // Two window sizes, as for the reading state: a room split that only holds
    // at one height holds by accident.
    QTest::addColumn<QSize>("windowSize");

    QTest::newRow("900x600") << QSize(900, 600);
    QTest::newRow("1200x800") << QSize(1200, 800);
}

void LibraryTest::keepsTheMeasuresOfTheEditState()
{
    QFETCH(QSize, windowSize);

    const qint64 id = storedNote(QStringLiteral("Idee für Denkzettel"));
    analysed(id, QStringLiteral("Software-Ideen"), QStringList({QStringLiteral("software-idee")}));

    LibraryWindow window(m_store.get());
    window.resize(windowSize);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QCOMPARE(window.size(), windowSize);

    listOf(window)->setCurrentIndex(noteRow(listOf(window), 0));

    QWidget *detail = detailOf(window);
    // Reader and editor share this stack, so it is the one thing that can be
    // measured in both states.
    QWidget *stack = readerOf(window)->parentWidget();
    QTest::qWait(50);
    const int readingTop = stack->mapTo(detail, QPoint()).y();
    const int readingHeight = stack->height();

    // A button carries the width of its label (KDE HIG) and must not grow with
    // the window. The stack that fixed finding 1 made them grow: it asks for
    // the surplus width itself and passes it on to the buttons inside it —
    // 80 px of label in a 194 px button at 1200 px window width (UI review of
    // 02.08.2026, second pass, finding 7).
    //
    // Not the stretch on its editing page, although that was the reading the
    // finding came with: taking the stretch out changes none of these numbers
    // (measured 02.08.2026). What holds the width is the horizontal
    // QSizePolicy::Maximum on the stack — see librarywindow.cpp, buildDetail().
    QPushButton *edit = buttonNamed(window, QStringLiteral("Bearbeiten"));
    QPushButton *remove = buttonNamed(window, QStringLiteral("Löschen"));
    QVERIFY(edit);
    QVERIFY(remove);
    QVERIFY2(edit->width() == edit->sizeHint().width(),
             qPrintable(QStringLiteral("„Bearbeiten“ ist %1 px breit, seine natürliche Breite ist %2 px")
                            .arg(edit->width())
                            .arg(edit->sizeHint().width())));
    QVERIFY2(remove->width() == remove->sizeHint().width(),
             qPrintable(QStringLiteral("„Löschen“ ist %1 px breit, seine natürliche Breite ist %2 px")
                            .arg(remove->width())
                            .arg(remove->sizeHint().width())));

    actionNamed(window, QStringLiteral("Bearbeiten"))->trigger();
    QTest::qWait(50);

    // Wireframe 2a puts the badge where the buttons stand, in the same row —
    // and a mode change must not shift the content: the eye rests on the spot
    // that is to be corrected when F2 is pressed, and it would travel with the
    // text. The head row is the one that shrank, from button height to label
    // height (UI review of 02.08.2026, finding 1).
    QVERIFY2(stack->mapTo(detail, QPoint()).y() == readingTop,
             qPrintable(QStringLiteral("Notiztext beginnt beim Lesen bei y=%1, beim Bearbeiten bei y=%2")
                            .arg(readingTop)
                            .arg(stack->mapTo(detail, QPoint()).y())));
    QLabel *badge = labelNamed(window, QStringLiteral("wird bearbeitet"));
    QWidget *meta = labelNamed(window, QStringLiteral("Kategorie"))->parentWidget();
    QWidget *footer =
        labelNamed(window, QStringLiteral("Esc bricht ab · Strg+Enter speichert"))->parentWidget();
    QVERIFY(badge);
    QVERIFY(meta);
    QVERIFY(footer);

    const auto top = [detail](QWidget *widget) {
        return widget->mapTo(detail, QPoint()).y();
    };
    const auto bottom = [&top](QWidget *widget) {
        return top(widget) + widget->height();
    };

    // Wireframe 2a, state B, from top to bottom: head, text field, category
    // and tags, then the row of buttons.
    QVERIFY2(bottom(badge) <= top(stack),
             qPrintable(QStringLiteral("Kopfzeile endet bei %1, Textfeld beginnt bei %2")
                            .arg(bottom(badge))
                            .arg(top(stack))));
    QVERIFY2(bottom(stack) <= top(meta),
             qPrintable(QStringLiteral("Textfeld endet bei %1, Merkmalszeile beginnt bei %2")
                            .arg(bottom(stack))
                            .arg(top(meta))));
    QVERIFY2(bottom(meta) <= top(footer),
             qPrintable(QStringLiteral("Merkmalszeile endet bei %1, Fußzeile beginnt bei %2")
                            .arg(bottom(meta))
                            .arg(top(footer))));

    // The button row sits on the lower edge of the pane and the note text gets
    // the surplus height — otherwise the pane ends in a field of empty space,
    // the mistake the header row of the window already made once (customer
    // finding of 01.08.2026).
    QVERIFY2(detail->height() - bottom(footer) <= 12,
             qPrintable(QStringLiteral("Unter der Fußzeile bleiben %1 px").arg(detail->height() - bottom(footer))));
    QVERIFY2(stack->height() >= detail->height() / 2,
             qPrintable(QStringLiteral("Textfeld %1 px hoch in einem %2 px hohen Bereich")
                            .arg(stack->height())
                            .arg(detail->height())));

    // Both extra rows stay one row high — they must not grow into the text.
    QVERIFY2(meta->height() <= meta->sizeHint().height(),
             qPrintable(QStringLiteral("Merkmalszeile %1 px hoch").arg(meta->height())));
    QVERIFY2(footer->height() <= footer->sizeHint().height(),
             qPrintable(QStringLiteral("Fußzeile %1 px hoch").arg(footer->height())));

    // The text field is shorter while editing — by exactly the two rows that
    // came in below it and their spacing, and by nothing else. The head row
    // above it is expressly not part of that difference; that one has to keep
    // its height, which is what the y-coordinate above holds.
    const int spacing = detail->layout()->spacing();
    QCOMPARE(readingHeight - stack->height(), meta->height() + footer->height() + 2 * spacing);

    // The other axis: editing does not move the split between list and pane.
    auto *splitter = window.findChild<QSplitter *>();
    QVERIFY(splitter);
    QCOMPARE(splitter->widget(0)->width(), 300);
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
