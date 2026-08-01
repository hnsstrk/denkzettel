#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QListView>
#include <QTemporaryDir>
#include <QTest>

/**
 * Writes the picture series of the grouped note list for the handover
 * (DoD 2, wireframes 3a and 3b).
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

/** Walks the selection up `steps` rows, as the arrow key does. */
void walkUp(QListView *list, int steps)
{
    for (int step = 0; step < steps; ++step) {
        QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        QCoreApplication::sendEvent(list, &up);
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
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

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
        window.resize(900, 600);
        window.showLibrary();
        QTest::qWait(200);
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
        window.resize(900, 600);
        window.showLibrary();
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
        window.resize(900, 600);
        window.showLibrary();
        QTest::qWait(200);
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
        window.resize(900, 600);
        window.showLibrary();
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
        window.resize(900, 600);
        window.showLibrary();
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
        window.resize(900, 600);
        window.showLibrary();
        QTest::qWait(200);
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
        window.resize(900, 600);
        window.showLibrary();
        QTest::qWait(200);

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
        window.resize(900, 600);
        window.showLibrary();
        QTest::qWait(200);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
        QTest::qWait(100);
        walkUp(list, 9);
        shoot(window, directory, QStringLiteral("08-heilung-kopf-mitten-in-kleiner-gruppe.png"));
    }

    return 0;
}
