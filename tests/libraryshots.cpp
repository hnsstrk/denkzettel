#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QListView>
#include <QPushButton>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>

/**
 * Writes the picture series of the grouped note list for the handover
 * (wireframes 3a and 3b).
 *
 * Not a test — a picture maker. It is built and run by hand, so it stays out
 * of `add_test()`: a picture nobody looks at proves nothing, and a failing
 * screenshot writer must not turn the suite red.
 *
 * Run it with QT_QPA_PLATFORMTHEME=kde. Without it Qt falls back to a
 * substitute font whose sizes are not the ones the running application uses —
 * heads and timestamps then look heavier than the note text, and a picture
 * that misstates its own type sizes is worse than none.
 *
 * Usage: QT_QPA_PLATFORMTHEME=kde libraryshots <target directory>
 */
namespace
{
/** The Friday every picture but the Monday one is taken on. */
QDateTime friday()
{
    return QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate);
}

QDateTime at(const QString &isoDateTime)
{
    return QDateTime::fromString(isoDateTime, Qt::ISODate);
}

// Healing this means changing the signature or introducing a type of its own,
// which is design rather than tidying up (issue #76). The one case a mix-up
// would be visible in - placeholderPage() in the empty library - gets a test
// assurance instead, as issue #88.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void addNote(Store &store, const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = at(isoDateTime);
    if (!store.addNote(note).has_value()) {
        qFatal("Notiz ließ sich nicht speichern");
    }
}

QListView *listOf(QWidget &window)
{
    auto *list = window.findChild<QListView *>();
    Q_ASSERT(list);
    return list;
}

QPushButton *buttonNamed(QWidget &window, const QString &text)
{
    const QList<QPushButton *> buttons = window.findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        if (button->text() == text) {
            return button;
        }
    }
    return nullptr;
}

/**
 * The two colour schemes of the scheme-change series (issue #58).
 *
 * The four colours that carry the picture are the measured ones of Breeze
 * Light and Breeze Dark, taken from the UX investigation of 01.08.2026
 * (`Palettenmessung vom 01.08.2026`); the rest is
 * derived from them, because a picture in which only the text colour changes
 * would show a light window with dark text and prove nothing.
 */
QPalette breezePalette(bool dark)
{
    const QColor window = dark ? QColor(0x20, 0x23, 0x26) : QColor(0xef, 0xf0, 0xf1);
    const QColor base = dark ? QColor(0x14, 0x16, 0x18) : QColor(0xff, 0xff, 0xff);
    const QColor text = dark ? QColor(0xfc, 0xfc, 0xfc) : QColor(0x23, 0x26, 0x29);
    const QColor placeholder = dark ? QColor(0xa1, 0xa9, 0xb1) : QColor(0x70, 0x7d, 0x8a);

    QPalette palette;
    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, window);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, window);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::ToolTipBase, base);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::PlaceholderText, placeholder);
    palette.setColor(QPalette::Highlight, QColor(0x3d, 0xae, 0xe9));
    palette.setColor(QPalette::HighlightedText, dark ? QColor(0xfc, 0xfc, 0xfc) : QColor(0xff, 0xff, 0xff));
    palette.setColor(QPalette::Link, dark ? QColor(0x1d, 0x99, 0xf3) : QColor(0x29, 0x80, 0xb9));

    return palette;
}

/** Hands the new scheme to the standing window, as a scheme change does. */
void applyScheme(const QPalette &palette)
{
    qApp->setPalette(palette);
    // Qt delivers the palette through a posted event. Without letting it
    // through, the picture would be taken before the window has seen it — and
    // an unhealed window would look healed (issue #58, AK 3).
    QCoreApplication::processEvents();
}

/** Walks the selection up `steps` rows, as the arrow key does. */
void walkUp(QListView *list, int steps)
{
    for (int step = 0; step < steps; ++step) {
        QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        QCoreApplication::sendEvent(list, &up);
    }
}

/**
 * The selectable row the lower edge cuts through, or -1 if none does.
 *
 * Looked up rather than written down: which row is clipped depends on the roll
 * value, and one written down would go stale with the first change of a line
 * height (issue #71).
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

/** Shows the window and waits until it is really on screen. */
void open(LibraryWindow &window)
{
    window.resize(900, 600);
    window.showLibrary();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("Fenster kam nicht auf den Schirm");
    }
}

void shoot(QWidget &window, const QString &directory, const QString &name)
{
    // The message band of the deletion grows out of nothing over about half a
    // second; a shorter wait catches it half open.
    QTest::qWait(500);
    if (!window.grab().save(directory + QLatin1Char('/') + name)) {
        qFatal("Bild %s ließ sich nicht schreiben", qUtf8Printable(name));
    }
    qInfo("geschrieben: %s", qUtf8Printable(name));
}
}

int main(int argc, char **argv)
{
    // The series is to show the state as shipped, not the window size and
    // splitter position whoever takes it happens to have stored: the library
    // window restores both from denkzettelrc, and with an application name set
    // that is the real one. Pointing the configuration at an empty directory
    // keeps the pictures reproducible on any machine — and keeps this from
    // writing into the user's own file.
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    if (argc < 2) {
        qFatal("Aufruf: libraryshots <Zielverzeichnis>");
    }
    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    // 1 — the regular case of wireframe 3a: all five groups, one note read.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        addNote(store,
                QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten — "
                               "als Cronjob auf dem NAS einrichten und danach einmal wiederherstellen"),
                QStringLiteral("2026-07-31T14:32:00"));
        addNote(store,
                QStringLiteral("Idee für Denkzettel — Bündel-Export erst vorschlagen, wenn mindestens "
                               "fünf Notizen zum selben Thema da sind, sonst wird der Vault zugemüllt"),
                QStringLiteral("2026-07-31T11:05:00"));
        addNote(store, QStringLiteral("journalctl -u whisperd --since today"),
                QStringLiteral("2026-07-30T21:48:00"));
        addNote(store, QStringLiteral("Mara wegen Wochenende anrufen,\nKuchen nicht vergessen"),
                QStringLiteral("2026-07-28T09:00:00"));
        addNote(store, QStringLiteral("Kategorien-Prompt: Beispiele mitgeben, sonst rät das Modell"),
                QStringLiteral("2026-07-23T09:00:00"));
        addNote(store, QStringLiteral("Tray-Icon im dunklen Theme testen"),
                QStringLiteral("2026-07-10T09:00:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);
        listOf(window)->setCurrentIndex(listOf(window)->model()->index(2, 0));
        shoot(window, directory, QStringLiteral("01-normalfall.png"));
    }

    // 2 — the empty library (wireframe 2c): no group, and so no head.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);
        shoot(window, directory, QStringLiteral("02-leerzustand.png"));
    }

    // 3 — the message of the pending deletion. The deleted note was the only
    // one of "Gestern", so its head went with it.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        addNote(store, QStringLiteral("Reifen wechseln lassen"), QStringLiteral("2026-07-31T17:02:00"));
        addNote(store, QStringLiteral("Whisper-Warteschlange bei Suspend prüfen"),
                QStringLiteral("2026-07-30T21:48:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);
        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(3, 0));
        QKeyEvent del(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QCoreApplication::sendEvent(list, &del);
        shoot(window, directory, QStringLiteral("03-meldungszustand.png"));
    }

    // 4 — wireframe 3b, case 1: a group of exactly one note, and "Diese Woche"
    // left out because a Monday cannot fill it.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        addNote(store, QStringLiteral("Whisper-Warteschlange bei Suspend prüfen"),
                QStringLiteral("2026-08-02T21:48:00"));
        addNote(store, QStringLiteral("Mara wegen Wochenende anrufen, Kuchen nicht vergessen"),
                QStringLiteral("2026-07-30T09:00:00"));
        addNote(store, QStringLiteral("Kategorien-Prompt: Beispiele mitgeben"),
                QStringLiteral("2026-07-23T09:00:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(at(QStringLiteral("2026-08-03T10:00:00")));
        open(window);
        shoot(window, directory, QStringLiteral("04-fall1-gruppe-mit-einem-eintrag.png"));
    }

    // 5 — wireframe 3b, case 2: the long first line without a break.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        addNote(store,
                QStringLiteral("Für den Vortrag nächste Woche noch einmal durchgehen, ob die Zahlen "
                               "zur Auswertung vom Frühjahr passen — sonst neu rechnen lassen"),
                QStringLiteral("2026-07-31T09:41:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);
        shoot(window, directory, QStringLiteral("05-fall2-lange-erste-zeile.png"));
    }

    // 6 — wireframe 3b, case 3: a real line break, and the single line whose
    // preview row stays empty without the entry shrinking.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        addNote(store, QStringLiteral("Einkauf Samstag\nMehl\nHefe\nZitronen\nSahne\nButter\nEier"),
                QStringLiteral("2026-07-31T18:20:00"));
        addNote(store, QStringLiteral("Reifen wechseln lassen"), QStringLiteral("2026-07-31T17:02:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);
        listOf(window)->setCurrentIndex(listOf(window)->model()->index(2, 0));
        shoot(window, directory, QStringLiteral("06-fall3-umbruch-und-einzeiler.png"));
    }

    // 7 — wireframe 3b, case 4: the crossing while scrolling. The selection
    // walks up to the first note of "Letzte Woche"; its head comes along and
    // the entry stays whole.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Notiz von dieser Woche, %1 Uhr — Tray-Icon im dunklen Theme testen")
                        .arg(hour),
                    QStringLiteral("2026-07-29T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Notiz von letzter Woche, %1 Uhr — Kategorien-Prompt: Beispiele mitgeben")
                        .arg(hour),
                    QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
        QTest::qWait(100);
        walkUp(list, 7);
        shoot(window, directory, QStringLiteral("07-fall4-uebergang-beim-scrollen.png"));
    }

    // 8 — the case the UI review of 01.08.2026 found: the selection sits in the
    // middle of a small group, so its head is not the row above it. It fits
    // into the picture together with the selection and is fetched along.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Von heute, %1 Uhr — Tray-Icon im dunklen Theme testen").arg(hour),
                    QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 9; hour < 12; ++hour) {
            addNote(store,
                    QStringLiteral("Von gestern, %1 Uhr — Whisper-Warteschlange bei Suspend prüfen").arg(hour),
                    QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Von letzter Woche, %1 Uhr — Kategorien-Prompt: Beispiele mitgeben").arg(hour),
                    QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
        QTest::qWait(100);
        walkUp(list, 9);
        shoot(window, directory, QStringLiteral("08-heilung-kopf-mitten-in-kleiner-gruppe.png"));
    }

    // 9 — the quiet picture within a group (UI review of 01.08.2026). The user
    // rolled the list to this crossing; the arrow key has just moved the
    // selection one entry down inside "Gestern", and the crossing has stayed
    // where he put it. A picture can only show the state, not the jump that
    // did not happen — that one is held by the roll value in librarytest.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Von heute, %1 Uhr — Tray-Icon im dunklen Theme testen").arg(hour),
                    QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Von gestern, %1 Uhr — Whisper-Warteschlange bei Suspend prüfen").arg(hour),
                    QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        // Rows: head "Heute", eight notes, head "Gestern", eight notes. Rolled
        // to the fourth note of "Heute", the crossing sits in the middle of
        // the picture; the selection goes to the second note below it.
        list->setCurrentIndex(list->model()->index(12, 0));
        QTest::qWait(100);
        list->verticalScrollBar()->setValue(4);
        QTest::qWait(100);

        QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QCoreApplication::sendEvent(list, &down);
        shoot(window, directory, QStringLiteral("09-ruhiges-bild-innerhalb-der-gruppe.png"));
    }

    // 10 — the colour scheme changed under the standing window (issue #58).
    // One window, four pictures, no rebuild in between: the daemon builds this
    // window at start and keeps it (SPEC 2.1, main.cpp), so a scheme that
    // reaches it late has to reach it at all. The reading state carries the
    // timestamp of the note, the edit state „Kategorie", „Tags" and the key
    // hint — those are the QLabel places; the timestamps inside the list are
    // painted by the delegate and have always followed.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        addNote(store,
                QStringLiteral("Idee für Denkzettel — Bündel-Export erst vorschlagen, wenn mindestens "
                               "fünf Notizen zum selben Thema da sind"),
                QStringLiteral("2026-07-31T11:05:00"));
        addNote(store, QStringLiteral("journalctl -u whisperd --since today"),
                QStringLiteral("2026-07-30T21:48:00"));

        applyScheme(breezePalette(false));

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);
        // Rows: head „Heute", its note, head „Gestern", its note — the note of
        // yesterday, so the reading pane carries „Gestern 21:48".
        listOf(window)->setCurrentIndex(listOf(window)->model()->index(3, 0));
        shoot(window, directory, QStringLiteral("10a-schema-hell-lesen.png"));

        applyScheme(breezePalette(true));
        shoot(window, directory, QStringLiteral("10b-schema-dunkel-lesen.png"));

        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        shoot(window, directory, QStringLiteral("10c-schema-dunkel-bearbeiten.png"));

        applyScheme(breezePalette(false));
        shoot(window, directory, QStringLiteral("10d-schema-hell-bearbeiten.png"));
    }

    // 11 — the click on a row the lower edge cuts through (issue #71). It used
    // to move the list by a line height and mark the neighbour, or nothing at
    // all; now the row that was clicked is the row that carries the frame and
    // the reading pane shows its note. That the list did not move is held by
    // the roll value in librarytest — a picture can only show a state.
    //
    // Two pictures, because there are two states and one of them arrives late.
    // QAbstractItemView starts a delayed autoscroll on the press, and it fires
    // one double-click interval afterwards: measured on 05.08.2026 the roll
    // value stood at 6 up to 500 ms after the click and at 7 from 550 ms on,
    // with the selection on the clicked row throughout
    // (messungen/71-nachlaufender-autoscroll.txt). A single picture taken by
    // shoot() would sit right on that edge and come out differently from one
    // run to the next — which it did, twice, before this was measured.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Von heute, %1 Uhr — Tray-Icon im dunklen Theme testen").arg(hour),
                    QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 9; hour < 12; ++hour) {
            addNote(store,
                    QStringLiteral("Von gestern, %1 Uhr — Whisper-Warteschlange bei Suspend prüfen").arg(hour),
                    QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 8; hour < 16; ++hour) {
            addNote(store,
                    QStringLiteral("Von letzter Woche, %1 Uhr — Kategorien-Prompt: Beispiele mitgeben").arg(hour),
                    QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        // The selection is put somewhere else first, or the click would change
        // no current row and the case would not arise at all.
        list->setCurrentIndex(list->model()->index(1, 0));
        QTest::qWait(100);
        list->verticalScrollBar()->setValue(6);
        QTest::qWait(100);

        const int row = bottomClippedRow(list);
        if (row < 0) {
            qFatal("Keine angeschnittene Zeile — die Szene zeigt den Fall nicht");
        }
        const QRect rect = list->visualRect(list->model()->index(row, 0));
        const int rolledTo = list->verticalScrollBar()->value();
        QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier,
                          QPoint(rect.center().x(), rect.top() + 5));

        // 11a — what the click itself leaves behind: the clicked row marked,
        // still cut through, and the picture where it was. Taken without a
        // wait, so it stands well before the delayed autoscroll.
        if (list->verticalScrollBar()->value() != rolledTo) {
            qFatal("Rollwert nach dem Klick %d statt %d — die Szene zeigt nicht, was sie behauptet",
                   list->verticalScrollBar()->value(), rolledTo);
        }
        if (!window.grab().save(directory + QStringLiteral("/11a-klick-auf-angeschnittene-zeile.png"))) {
            qFatal("Bild 11a ließ sich nicht schreiben");
        }

        // 11b — and what the user sees a moment later: Qt has fetched the row
        // into full view on its own. The selection has not moved with it, which
        // is the whole point of the heal.
        QTest::qWait(1500);
        shoot(window, directory, QStringLiteral("11b-nach-dem-nachlaufenden-autoscroll.png"));
    }

    return 0;
}
