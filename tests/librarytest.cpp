#include "store/store.h"
#include "ui/elidedlines.h"
#include "ui/librarywindow.h"
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
    void usesTheWeekdayFormWithinTheLastSevenDays();
    void usesTheAbsoluteDateBeyondSevenDays();

    void keepsShortTextOnOneLine();
    void elidesTextBeyondTwoLines();
    void foldsLineBreaksIntoThePreview();
    void hasNoLinesForEmptyText();

    void listsNotesWithTheirTimestamp();
    void takesAndReinsertsARow();

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
    void putsTheMessageBetweenTheHeaderAndTheNotes();
    void keepsTheWindowSizeForTheNextSession();

    // Qt emits aboutToQuit once per process, so the test of the quit path has
    // to be the last one of this class.
    void carriesOutTheDeletionWhenTheApplicationQuits();

private:
    static QDateTime at(const QString &isoDateTime);
    static QLocale german();
    static Note noteWith(const QString &content);

    /** Width of ten wide characters — enough for a few words, not for many. */
    static int narrowWidth();

    /** Adds a note to the store; the first one added is the newest. */
    qint64 storedNote(const QString &content);

    /** Texts of the labels the window shows right now. */
    static QStringList visibleLabels(const QWidget &window);

    static QListView *listOf(const QWidget &window);
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
    list->setCurrentIndex(list->model()->index(1, 0));

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
    list->setCurrentIndex(list->model()->index(1, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    QCOMPARE(list->model()->rowCount(), 2);
    QCOMPARE(list->currentIndex().row(), 1);
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
    list->setCurrentIndex(list->model()->index(1, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

    // No following entry, so the preceding one takes the selection.
    QCOMPARE(list->currentIndex().row(), 0);
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("eins"));
}

void LibraryTest::fallsBackToTheEmptyStateAfterTheLastNoteIsDeleted()
{
    storedNote(QStringLiteral("die einzige Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(list->model()->index(0, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();

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
    list->setCurrentIndex(list->model()->index(0, 0));
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

    // The first key press has to land on the first entry, not skip past it.
    QTest::keyClick(list, Qt::Key_Down);
    QCOMPARE(list->currentIndex().row(), 0);

    auto *reader = window.findChild<QTextBrowser *>();
    QVERIFY(reader);
    QCOMPARE(reader->toPlainText(), QStringLiteral("die neuere Notiz"));

    QTest::keyClick(list, Qt::Key_Down);
    QCOMPARE(list->currentIndex().row(), 1);
    QCOMPARE(reader->toPlainText(), QStringLiteral("die ältere Notiz"));

    QTest::keyClick(list, Qt::Key_Up);
    QCOMPARE(list->currentIndex().row(), 0);
}

void LibraryTest::undoesTheDeletionByKeyboard()
{
    const qint64 id = storedNote(QStringLiteral("kommt zurück"));
    storedNote(QStringLiteral("bleibt sowieso"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(list->model()->index(0, 0));
    actionNamed(window, QStringLiteral("Löschen"))->trigger();
    QCOMPARE(list->model()->rowCount(), 1);

    QTest::keyClick(&window, Qt::Key_Z, Qt::ControlModifier);

    // The note is back in its old place and selected again.
    QCOMPARE(list->model()->rowCount(), 2);
    QCOMPARE(list->currentIndex().row(), 0);
    QCOMPARE(list->currentIndex().data(Qt::DisplayRole).toString(), QStringLiteral("kommt zurück"));
    QVERIFY(m_store->note(id).has_value());
}

void LibraryTest::deletesWithTheDeleteKey()
{
    const qint64 id = storedNote(QStringLiteral("per Taste gelöscht"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(list->model()->index(0, 0));
    QTest::keyClick(list, Qt::Key_Delete);

    QCOMPARE(list->model()->rowCount(), 0);

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
    list->setCurrentIndex(list->model()->index(0, 0));
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

    list->setCurrentIndex(list->model()->index(0, 0));
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
    QCOMPARE(list->model()->rowCount(), 1);
    QCOMPARE(list->model()->index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("danach gelöscht"));
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
    list->setCurrentIndex(list->model()->index(0, 0));
    QTest::keyClick(list, Qt::Key_Delete);
    QCOMPARE(list->model()->rowCount(), 1);

    // ShowLibrary() on an open window brings it to the front. Reading the
    // store again would fetch the note that is still counting down back into
    // the list.
    window.showLibrary();

    QCOMPARE(list->model()->rowCount(), 1);
    QCOMPARE(list->model()->index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("bleibt"));
}

void LibraryTest::readsTheStoreAgainWhenTheOpenWindowIsShownAgain()
{
    storedNote(QStringLiteral("die neuere Notiz"));
    storedNote(QStringLiteral("die ältere Notiz"));

    LibraryWindow window(m_store.get());
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(list->model()->index(1, 0));

    // Meta+N while the library is open: the note captured meanwhile is the
    // newest one and takes the top row, so the selected note moves down.
    Note captured = noteWith(QStringLiteral("gerade festgehalten"));
    captured.createdAt = captured.createdAt.addSecs(60);
    QVERIFY(m_store->addNote(captured).has_value());

    window.showLibrary();

    QCOMPARE(list->model()->rowCount(), 3);
    QCOMPARE(list->model()->index(0, 0).data(Qt::DisplayRole).toString(),
             QStringLiteral("gerade festgehalten"));

    // The selection follows the note, not the row it used to sit in.
    QCOMPARE(list->currentIndex().row(), 2);
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

    list->setCurrentIndex(list->model()->index(0, 0));
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

void LibraryTest::putsTheMessageBetweenTheHeaderAndTheNotes()
{
    storedNote(QStringLiteral("mit Frist gelöscht"));

    LibraryWindow window(m_store.get());
    window.resize(900, 600);
    window.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QListView *list = listOf(window);
    list->setCurrentIndex(list->model()->index(0, 0));
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
    list->setCurrentIndex(list->model()->index(0, 0));
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
