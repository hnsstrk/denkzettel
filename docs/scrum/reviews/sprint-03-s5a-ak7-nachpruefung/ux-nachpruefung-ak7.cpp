// Nachprüfung der AK-7-Heilung (#46) durch das UI/UX-Review.
// Kein Projekt-Code: linkt nur gegen die gebaute Bibliothek denkzettelui und
// rendert das echte LibraryWindow offscreen (DoD 3, Retro-Beschluss B3).
//
// Gemessen wird der Scrollwert vor und nach jedem Tastendruck, nicht nur der
// Endzustand: Der Schaden eines zu weit greifenden Vorscrollens ist der
// Sprung, und den stellt der nachfolgende scrollTo(selection) im Endbild
// wieder glatt.
//
// Bauen und laufen lassen wie ux-shot-s5a.cpp, Bezugszeitpunkt fest auf
// Fr 31.07.2026 16:00.

#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QAction>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QListView>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

namespace
{
QTextStream &out()
{
    static QTextStream stream(stdout);
    return stream;
}

void settle(int milliseconds)
{
    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < milliseconds) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
}

QDateTime at(const QString &iso)
{
    return QDateTime::fromString(iso, Qt::ISODate);
}

void shoot(QWidget &window, const QString &directory, const QString &name)
{
    const QString path = QStringLiteral("%1/ak7-%2.png").arg(directory, name);
    if (!window.grab().save(path)) {
        out() << "  Bild " << path << " konnte nicht geschrieben werden\n";
    }
}

/** Der Kopf der Gruppe, in der `row` steht — dieselbe Suche wie im Fenster. */
QModelIndex headOf(QListView *list, int row)
{
    for (int above = row - 1; above >= 0; --above) {
        const QModelIndex candidate = list->model()->index(above, 0);
        if (candidate.data(NoteListModel::GroupHeaderRole).toBool()) {
            return candidate;
        }
    }
    return {};
}

/** Ein Tastendruck samt Scrollwert davor und danach. */
void press(QListView *list, Qt::Key key, const QString &was)
{
    const int before = list->verticalScrollBar()->value();
    const int rowBefore = list->currentIndex().row();

    QKeyEvent down(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::sendEvent(list, &down);
    QKeyEvent up(QEvent::KeyRelease, key, Qt::NoModifier);
    QApplication::sendEvent(list, &up);
    settle(80);

    const int after = list->verticalScrollBar()->value();
    const int row = list->currentIndex().row();
    const QModelIndex head = headOf(list, row);
    const QRect headRect = head.isValid() ? list->visualRect(head) : QRect();
    const QRect selected = list->visualRect(list->model()->index(row, 0));
    const QRect port = list->viewport()->rect();

    out() << QStringLiteral("  %1: Zeile %2 → %3, Rollwert %4 → %5 (%6%7)\n")
                 .arg(was)
                 .arg(rowBefore)
                 .arg(row)
                 .arg(before)
                 .arg(after)
                 .arg(after - before >= 0 ? QStringLiteral("+") : QString())
                 .arg(after - before);
    out() << QStringLiteral("      Kopf \"%1\" y=%2 h=%3 → %4 · Auswahl y=%5 h=%6 → %7\n")
                 .arg(head.isValid() ? head.data(Qt::DisplayRole).toString() : QStringLiteral("—"))
                 .arg(headRect.y())
                 .arg(headRect.height())
                 .arg(head.isValid() && port.contains(headRect) ? QStringLiteral("IM BILD")
                                                                : QStringLiteral("draussen"))
                 .arg(selected.y())
                 .arg(selected.height())
                 .arg(port.contains(selected) ? QStringLiteral("ganz sichtbar")
                                              : QStringLiteral("ANGESCHNITTEN"));
}

qint64 add(Store &store, const QString &iso, const QString &content)
{
    Note note;
    note.createdAt = at(iso);
    note.content = content;
    return store.addNote(note).value_or(-1);
}

void addGroup(Store &store, const QString &day, int from, int to, const QString &label)
{
    for (int hour = from; hour < to; ++hour) {
        add(store,
            QStringLiteral("%1T%2:00:00").arg(day).arg(hour, 2, 10, QLatin1Char('0')),
            QStringLiteral("Von %1, %2 Uhr — eine Notiz mit genug Text für zwei Zeilen Vorschau")
                .arg(label)
                .arg(hour));
    }
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        out() << "Aufruf: ux-nachpruefung-ak7 <Zielverzeichnis> [-style breeze]\n";
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString directory = QString::fromLocal8Bit(argv[1]);

    // --- Bestand A: zwei grosse Gruppen (je acht Notizen) -----------------
    Store big(sandbox.filePath(QStringLiteral("gross.db")));
    if (!big.open()) {
        out() << "Store: " << big.lastError() << "\n";
        return 1;
    }
    addGroup(big, QStringLiteral("2026-07-31"), 8, 16, QStringLiteral("heute"));
    addGroup(big, QStringLiteral("2026-07-30"), 8, 16, QStringLiteral("gestern"));

    // --- Bestand B: grosse, kleine und mittlere Gruppe ---------------------
    Store mixed(sandbox.filePath(QStringLiteral("gemischt.db")));
    if (!mixed.open()) {
        out() << "Store: " << mixed.lastError() << "\n";
        return 1;
    }
    addGroup(mixed, QStringLiteral("2026-07-31"), 8, 14, QStringLiteral("heute"));
    addGroup(mixed, QStringLiteral("2026-07-30"), 8, 11, QStringLiteral("gestern"));
    addGroup(mixed, QStringLiteral("2026-07-20"), 8, 12, QStringLiteral("letzter Woche"));

    const auto openWindow = [](Store &store, LibraryWindow &window, QListView *&list) {
        window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
        window.resize(900, 600);
        window.showLibrary();
        settle(300);
        list = window.findChild<QListView *>();
        list->setFocus();
        (void)store;
    };

    // === N1 — abwärts über die Grenze ====================================
    {
        LibraryWindow window(&big);
        QListView *list = nullptr;
        openWindow(big, window, list);
        out() << "\n=== N1 abwärts über die Gruppengrenze (grosse Gruppen) ===\n";
        out() << "Viewport " << list->viewport()->width() << "x" << list->viewport()->height() << "\n";

        list->setCurrentIndex(list->model()->index(1, 0));
        settle(150);
        for (int step = 0; step < 7; ++step) {
            press(list, Qt::Key_Down, QStringLiteral("Ab  "));
        }
        shoot(window, directory, QStringLiteral("n1-vor-der-grenze"));
        press(list, Qt::Key_Down, QStringLiteral("Ab (Grenze)"));
        shoot(window, directory, QStringLiteral("n1-abwaerts-ueber-die-grenze"));
    }

    // === N2/N3 — aufwärts in eine grosse Gruppe ==========================
    {
        LibraryWindow window(&big);
        QListView *list = nullptr;
        openWindow(big, window, list);
        out() << "\n=== N2/N3 aufwärts in die grosse Gruppe ===\n";

        list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
        settle(150);
        for (int step = 0; step < 7; ++step) {
            press(list, Qt::Key_Up, QStringLiteral("Auf "));
        }
        shoot(window, directory, QStringLiteral("n2-aufwaerts-erste-notiz-der-gruppe"));

        out() << "  -- ein Schritt weiter: letzte Notiz der grossen Gruppe (N3) --\n";
        press(list, Qt::Key_Up, QStringLiteral("Auf (Grenze)"));
        shoot(window, directory, QStringLiteral("n3-aufwaerts-grosse-gruppe"));
    }

    // === N4/N5 — aufwärts in eine kleine Gruppe ==========================
    {
        LibraryWindow window(&mixed);
        QListView *list = nullptr;
        openWindow(mixed, window, list);
        out() << "\n=== N4 aufwärts in die kleine Gruppe (drei Notizen) ===\n";

        list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
        settle(150);
        for (int step = 0; step < 4; ++step) {
            press(list, Qt::Key_Up, QStringLiteral("Auf "));
        }
        shoot(window, directory, QStringLiteral("n4-kleine-gruppe-letzte-notiz"));

        out() << "  -- weiter auf die mittlere Notiz der kleinen Gruppe (N5) --\n";
        press(list, Qt::Key_Up, QStringLiteral("Auf "));
        shoot(window, directory, QStringLiteral("n5-kleine-gruppe-mittlere-notiz"));
    }

    // === N6 — mit dem Rad weggescrollt, dann Pfeiltaste ===================
    // Der Fall, den kein Test der Heilung abdeckt: Der Kopf ist aus dem Bild
    // gescrollt, ohne dass die Auswahl sich bewegt hat. Der nächste
    // Tastendruck bleibt innerhalb derselben Gruppe.
    {
        LibraryWindow window(&big);
        QListView *list = nullptr;
        openWindow(big, window, list);
        out() << "\n=== N6 Rad weggescrollt, dann Pfeil ab (innerhalb der Gruppe) ===\n";

        // Auswahl auf die fünfte Notiz von "Heute", ohne Scrollbedarf.
        list->setCurrentIndex(list->model()->index(5, 0));
        settle(150);
        out() << "  Ausgangslage: Rollwert " << list->verticalScrollBar()->value() << ", Kopf y="
              << list->visualRect(list->model()->index(0, 0)).y() << "\n";
        shoot(window, directory, QStringLiteral("n6a-ausgangslage"));

        // Wie zwei Raddrehungen: der Kopf verlässt das Bild nach oben, die
        // Auswahl bleibt sichtbar und unverändert. Genau das kann die
        // Tastaturnavigation allein nicht herstellen.
        list->verticalScrollBar()->setValue(2);
        settle(150);
        out() << "  Nach dem Raddreh: Rollwert " << list->verticalScrollBar()->value() << ", Kopf y="
              << list->visualRect(list->model()->index(0, 0)).y() << " (Auswahl unverändert Zeile "
              << list->currentIndex().row() << ")\n";
        shoot(window, directory, QStringLiteral("n6b-nach-dem-raddreh"));

        press(list, Qt::Key_Down, QStringLiteral("Ab  "));
        shoot(window, directory, QStringLiteral("n6c-nach-dem-tastendruck"));
    }

    // === N8 — derselbe Fall, so weit weggescrollt wie die Bedingung erlaubt
    // Der Kopf steht knapp innerhalb dessen, was die Passt-Bedingung noch
    // durchlässt. Hier wird der gegenläufige Sprung am grössten.
    {
        LibraryWindow window(&big);
        QListView *list = nullptr;
        openWindow(big, window, list);
        out() << "\n=== N8 Rad bis an die Grenze der Passt-Bedingung, dann Pfeil ab ===\n";

        list->setCurrentIndex(list->model()->index(6, 0));
        settle(150);
        list->verticalScrollBar()->setValue(6);
        settle(150);
        const QRect head = list->visualRect(list->model()->index(0, 0));
        out() << "  Nach dem Raddreh: Rollwert " << list->verticalScrollBar()->value() << ", Kopf y="
              << head.y() << ", Auswahl Zeile " << list->currentIndex().row() << " y="
              << list->visualRect(list->currentIndex()).y() << "\n";
        shoot(window, directory, QStringLiteral("n8a-vor-dem-tastendruck"));

        press(list, Qt::Key_Down, QStringLiteral("Ab  "));
        shoot(window, directory, QStringLiteral("n8b-nach-dem-tastendruck"));
    }

    // === N10 — Grenzübertritt auf eine schon sichtbare Zeile ==============
    // Der Prüfsatz gegen den Rückfall: Auch wenn die Zielzeile ohnehin ganz
    // im Bild steht, muss der Kopf ihrer Gruppe danach sichtbar sein.
    {
        LibraryWindow window(&mixed);
        QListView *list = nullptr;
        openWindow(mixed, window, list);
        out() << "\n=== N10 Grenzübertritt auf eine schon sichtbare Zeile ===\n";

        list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
        settle(150);
        for (int step = 0; step < 3; ++step) {
            QKeyEvent key(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
            QApplication::sendEvent(list, &key);
            settle(60);
        }

        const QModelIndex target = list->model()->index(10, 0);
        const QRect before = list->visualRect(target);
        out() << "  Zielzeile 10 vor dem Tastendruck: y=" << before.y() << " h=" << before.height() << " → "
              << (list->viewport()->rect().contains(before) ? QStringLiteral("SCHON GANZ SICHTBAR")
                                                            : QStringLiteral("nicht ganz sichtbar"))
              << "\n";
        press(list, Qt::Key_Up, QStringLiteral("Auf (Grenze)"));
        shoot(window, directory, QStringLiteral("n10-grenze-auf-sichtbare-zeile"));
    }

    // === N11 — Mausklick in eine andere Gruppe auf eine sichtbare Zeile ====
    // Bewusst offengelassener Fall (PO-Entscheidung): protokolliert, nicht
    // als Fehler gewertet.
    {
        LibraryWindow window(&big);
        QListView *list = nullptr;
        openWindow(big, window, list);
        out() << "\n=== N11 Mausklick in eine andere Gruppe (bewusst offen) ===\n";

        // Auswahl in "Gestern", geklickt wird in "Heute" — dessen Kopf ist
        // nach oben aus dem Bild gescrollt. Nur so trifft der Fall.
        list->setCurrentIndex(list->model()->index(11, 0));
        settle(150);
        list->verticalScrollBar()->setValue(6);
        settle(150);

        // Eine sichtbare Zeile suchen, deren eigener Gruppenkopf draussen ist.
        int target = -1;
        for (int row = 0; row < list->model()->rowCount(); ++row) {
            const QModelIndex index = list->model()->index(row, 0);
            if (index.data(NoteListModel::GroupHeaderRole).toBool()) {
                continue;
            }
            const QModelIndex head = headOf(list, row);
            const QRect rect = list->visualRect(index);
            if (list->viewport()->rect().contains(rect) && head.isValid()
                && !list->viewport()->rect().contains(list->visualRect(head))) {
                target = row;
                break;
            }
        }
        if (target < 0) {
            out() << "  keine passende Zeile gefunden\n";
        } else {
            const QRect rect = list->visualRect(list->model()->index(target, 0));
            const int before = list->verticalScrollBar()->value();
            out() << "  Klickziel Zeile " << target << " y=" << rect.y() << " (sichtbar), Rollwert " << before
                  << ", Kopf y=" << list->visualRect(headOf(list, target)).y() << "\n";
            shoot(window, directory, QStringLiteral("n11a-vor-dem-klick"));

            const QPoint point = rect.center();
            QMouseEvent down(QEvent::MouseButtonPress, point, list->viewport()->mapToGlobal(point),
                             Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(list->viewport(), &down);
            QMouseEvent up(QEvent::MouseButtonRelease, point, list->viewport()->mapToGlobal(point),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(list->viewport(), &up);
            settle(150);

            out() << "  Nach dem Klick: Auswahl Zeile " << list->currentIndex().row() << ", Rollwert " << before
                  << " → " << list->verticalScrollBar()->value() << ", Kopf y="
                  << list->visualRect(headOf(list, list->currentIndex().row())).y() << "\n";
            shoot(window, directory, QStringLiteral("n11b-nach-dem-klick"));
        }
    }

    // === N12 — Löschen und Undo an der Gruppengrenze ======================
    // Hier bricht ein Gruppenvergleich, der an Zeilennummern hängt: der
    // Neuaufbau nach takeNote/insertNote verschiebt sie.
    {
        Store single(sandbox.filePath(QStringLiteral("einzeln.db")));
        if (!single.open()) {
            out() << "Store: " << single.lastError() << "\n";
            return 1;
        }
        addGroup(single, QStringLiteral("2026-07-31"), 8, 12, QStringLiteral("heute"));
        add(single, QStringLiteral("2026-07-30T21:48:00"),
            QStringLiteral("Einzige Notiz von gestern — mit ihr geht ihr Kopf"));
        addGroup(single, QStringLiteral("2026-07-20"), 8, 12, QStringLiteral("letzter Woche"));

        LibraryWindow window(&single);
        QListView *list = nullptr;
        openWindow(single, window, list);
        out() << "\n=== N12 Löschen und Undo an der Gruppengrenze ===\n";

        // Auswahl auf die einzige Notiz von "Gestern" (Zeile 6).
        list->setCurrentIndex(list->model()->index(6, 0));
        settle(200);
        out() << "  Vor dem Löschen: Auswahl Zeile " << list->currentIndex().row() << ", Rollwert "
              << list->verticalScrollBar()->value() << ", Zeilen " << list->model()->rowCount() << "\n";
        shoot(window, directory, QStringLiteral("n12a-vor-dem-loeschen"));

        for (QAction *action : window.actions()) {
            if (action->text() == QStringLiteral("Löschen")) {
                action->trigger();
            }
        }
        settle(300);
        out() << "  Nach dem Löschen: Auswahl Zeile " << list->currentIndex().row() << ", Rollwert "
              << list->verticalScrollBar()->value() << ", Zeilen " << list->model()->rowCount() << ", Kopf \""
              << (headOf(list, list->currentIndex().row()).isValid()
                      ? headOf(list, list->currentIndex().row()).data(Qt::DisplayRole).toString()
                      : QStringLiteral("—"))
              << "\" y=" << list->visualRect(headOf(list, list->currentIndex().row())).y() << "\n";
        shoot(window, directory, QStringLiteral("n12b-nach-dem-loeschen"));

        for (QAction *action : window.actions()) {
            if (action->text() == QStringLiteral("Rückgängig")) {
                action->trigger();
            }
        }
        settle(300);
        out() << "  Nach dem Undo   : Auswahl Zeile " << list->currentIndex().row() << ", Rollwert "
              << list->verticalScrollBar()->value() << ", Zeilen " << list->model()->rowCount() << ", Kopf \""
              << (headOf(list, list->currentIndex().row()).isValid()
                      ? headOf(list, list->currentIndex().row()).data(Qt::DisplayRole).toString()
                      : QStringLiteral("—"))
              << "\" y=" << list->visualRect(headOf(list, list->currentIndex().row())).y() << "\n";
        shoot(window, directory, QStringLiteral("n12c-nach-dem-undo"));
    }

    // === N7 — abwärts durch eine grosse Gruppe, Zeile für Zeile ===========
    {
        LibraryWindow window(&big);
        QListView *list = nullptr;
        openWindow(big, window, list);
        out() << "\n=== N7 abwärts durch die grosse Gruppe, Rollwert je Schritt ===\n";

        list->setCurrentIndex(list->model()->index(1, 0));
        settle(150);
        for (int step = 0; step < 8; ++step) {
            press(list, Qt::Key_Down, QStringLiteral("Ab  "));
        }
    }

    out().flush();
    return 0;
}
