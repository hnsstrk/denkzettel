// Messsonde zur Vorprüfung von Issue #71 — „Klick auf eine angeschnittene
// Zeile wählt die Nachbarzeile" (UI-Review S5, Befund B2).
//
// Die Sonde misst, sie heilt nicht. Sie beantwortet drei Fragen:
//
//   1. Tritt der Fehler auf — reproduzierbar, in Zahlen, oben wie unten
//      angeschnitten?
//   2. Hängt er an derselben Ursache wie #57 (Gruppenkopf-Vorlauf, geheilt
//      über den Mausdruck-Merker) oder an einer anderen?
//   3. Welche der beiden im UI-Review genannten Lesarten trägt — und was
//      kostet sie? Dafür laufen vier Gegenproben an einem **blanken**
//      QListView mit demselben Modell und demselben Delegate.
//
// Die Sonde legt sich eine eigene Datenbank an und rührt weder die Notizen
// noch die Fenstergeometrie des Kunden an; `pruefen.sh` setzt ihr zusätzlich
// ein eigenes XDG_CONFIG_HOME.

#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QListView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTextStream>

namespace
{

QTextStream &out()
{
    static QTextStream stream(stdout);
    return stream;
}

void settle(int ms)
{
    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}

bool waitFor(const std::function<bool()> &done, int timeoutMs = 5000)
{
    QElapsedTimer clock;
    clock.start();
    while (!done() && clock.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    return done();
}

QString selectedRows(QListView *list)
{
    const QModelIndexList selected = list->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        return QStringLiteral("keine");
    }
    QStringList rows;
    for (const QModelIndex &index : selected) {
        rows << QString::number(index.row());
    }
    return rows.join(QLatin1Char(','));
}

/**
 * Die Zeile, die am unteren Rand angeschnitten steht: oben im Bild, unten
 * darüber hinaus. Köpfe zählen nicht — die Maus kann sie nicht wählen.
 */
int bottomClippedRow(QListView *list)
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

/** Dasselbe am oberen Rand: unten im Bild, oben darüber hinaus. */
int topClippedRow(QListView *list)
{
    for (int row = 0; row < list->model()->rowCount(); ++row) {
        const QModelIndex index = list->model()->index(row, 0);
        if (!index.flags().testFlag(Qt::ItemIsSelectable)) {
            continue;
        }
        const QRect rect = list->visualRect(index);
        if (rect.top() < 0 && rect.bottom() > 0) {
            return row;
        }
    }
    return -1;
}

struct ClickResult {
    int targetRow = -1;
    int rowUnderCursorBefore = -1;
    int rowUnderCursorAfter = -1;
    int currentAfter = -1;
    QString selectedAfter;
    int scrollBefore = 0;
    int scrollAfter = 0;
    int targetYBefore = 0;
    int targetYAfter = 0;
    int shift = 0;
};

/**
 * Ein Klick auf `point`, Schritt für Schritt gemessen. Zwischen Druck und
 * Loslassen wird ausgegeben, weil genau dort die Liste rutscht.
 */
ClickResult clickWatching(QListView *list, int targetRow, const QPoint &point)
{
    ClickResult result;
    result.targetRow = targetRow;

    const QModelIndex target = list->model()->index(targetRow, 0);
    result.rowUnderCursorBefore = list->indexAt(point).row();
    result.scrollBefore = list->verticalScrollBar()->value();
    result.targetYBefore = list->visualRect(target).y();

    const auto state = [list](const QString &when) {
        out() << QStringLiteral("    %1: Rollwert %2, aktuelle Zeile %3, markiert %4\n")
                     .arg(when, -18)
                     .arg(list->verticalScrollBar()->value())
                     .arg(list->currentIndex().row())
                     .arg(selectedRows(list));
    };

    state(QStringLiteral("vor dem Druck"));

    QMouseEvent down(QEvent::MouseButtonPress, point, list->viewport()->mapToGlobal(point),
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(list->viewport(), &down);
    settle(120);
    state(QStringLiteral("nach dem Druck"));

    QMouseEvent up(QEvent::MouseButtonRelease, point, list->viewport()->mapToGlobal(point),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(list->viewport(), &up);
    settle(120);
    state(QStringLiteral("nach dem Loslassen"));

    result.rowUnderCursorAfter = list->indexAt(point).row();
    result.currentAfter = list->currentIndex().row();
    result.selectedAfter = selectedRows(list);
    result.scrollAfter = list->verticalScrollBar()->value();
    result.targetYAfter = list->visualRect(target).y();
    result.shift = result.targetYAfter - result.targetYBefore;

    out() << QStringLiteral("    Zeile unter dem Zeiger: vorher %1, nachher %2\n")
                 .arg(result.rowUnderCursorBefore)
                 .arg(result.rowUnderCursorAfter);
    out() << QStringLiteral("    Zielzeile y: vorher %1, nachher %2 (Versatz %3 px)\n")
                 .arg(result.targetYBefore)
                 .arg(result.targetYAfter)
                 .arg(result.shift);
    out() << QStringLiteral("    ERGEBNIS: aktuelle Zeile %1, markiert %2 — %3\n")
                 .arg(result.currentAfter)
                 .arg(result.selectedAfter)
                 .arg(result.selectedAfter == QString::number(result.currentAfter)
                          ? QStringLiteral("stimmen überein")
                          : QStringLiteral("GEHEN AUSEINANDER"));
    return result;
}

QList<Note> makeNotes()
{
    QList<Note> notes;
    const auto add = [&notes](const QString &text, const QString &iso) {
        Note note;
        note.id = notes.size() + 1;
        note.createdAt = QDateTime::fromString(iso, Qt::ISODate);
        note.content = text;
        notes << note;
    };
    for (int hour = 8; hour < 16; ++hour) {
        add(QStringLiteral("von heute, %1 Uhr\nzweite Zeile").arg(hour),
            QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        add(QStringLiteral("von gestern, %1 Uhr\nzweite Zeile").arg(hour),
            QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    // Die Notizen kommen neueste zuerst in die Liste; der Store sortiert. Für
    // das blanke Modell drehen wir sie hier um.
    QList<Note> newestFirst;
    for (int i = notes.size() - 1; i >= 0; --i) {
        newestFirst << notes.at(i);
    }
    return newestFirst;
}

QDateTime referenceNow()
{
    return QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate);
}

/**
 * Ein blanker QListView mit demselben Modell und demselben Delegate — ohne
 * LibraryWindow, ohne dessen currentChanged-Weg. Die Gegenprobe hängt daran,
 * dass hier alles gleich ist außer der einen Verbindung.
 */
struct BareView {
    NoteListModel *model = nullptr;
    QListView *list = nullptr;
};

BareView makeBareView(QWidget *&owner)
{
    auto *list = new QListView;
    auto *model = new NoteListModel(list);
    model->setNotes(makeNotes(), referenceNow());
    list->setModel(model);
    list->setItemDelegate(new NoteListDelegate(list));
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setFrameShape(QFrame::NoFrame);
    list->resize(300, 552);
    list->show();
    owner = list;
    waitFor([list] { return list->isVisible(); });
    settle(300);
    return {model, list};
}

/** Rollt so, dass unten eine Zeile angeschnitten steht. */
int rollUntilClipped(QListView *list, int startValue)
{
    for (int value = startValue; value <= list->verticalScrollBar()->maximum(); ++value) {
        list->verticalScrollBar()->setValue(value);
        settle(60);
        if (bottomClippedRow(list) >= 0) {
            return value;
        }
    }
    return -1;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    out() << "== Sonde #71 — Klick auf eine angeschnittene Zeile ==\n";
    out() << "Plattform: " << QGuiApplication::platformName() << "\n\n";

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("sonde71.sqlite")));
    if (!store.open()) {
        out() << "Datenbank ließ sich nicht öffnen: " << store.lastError() << "\n";
        return 1;
    }
    const QList<Note> notes = makeNotes();
    for (int i = notes.size() - 1; i >= 0; --i) {
        Note note = notes.at(i);
        note.id = 0;
        store.addNote(note);
    }

    // ---------------------------------------------------------------- A ----
    out() << "=== A Geometrie der Liste ===\n";
    {
        LibraryWindow window(&store);
        window.setReferenceTime(referenceNow());
        window.resize(900, 600);
        window.showLibrary();
        if (!waitFor([&window] { return window.isVisible(); })) {
            out() << "Fenster kam nicht auf den Schirm\n";
            return 1;
        }
        settle(400);

        auto *list = window.findChild<QListView *>();
        out() << "  Viewport: " << list->viewport()->width() << "x" << list->viewport()->height() << "\n";
        out() << "  Rollmodus senkrecht: "
              << (list->verticalScrollMode() == QAbstractItemView::ScrollPerItem ? "ScrollPerItem"
                                                                                 : "ScrollPerPixel")
              << "\n";
        out() << "  Rollbereich: 0.." << list->verticalScrollBar()->maximum()
              << ", Einzelschritt " << list->verticalScrollBar()->singleStep() << "\n";
        for (int row = 0; row < qMin(4, list->model()->rowCount()); ++row) {
            const QModelIndex index = list->model()->index(row, 0);
            out() << "  Zeile " << row << ": h=" << list->visualRect(index).height()
                  << (index.flags().testFlag(Qt::ItemIsSelectable) ? " (Notiz)" : " (Kopf)") << "\n";
        }
        out() << "\n";
    }

    // ---------------------------------------------------------------- B ----
    out() << "=== B Unten angeschnitten, im LibraryWindow, Gruppengrenze wird überschritten ===\n";
    {
        LibraryWindow window(&store);
        window.setReferenceTime(referenceNow());
        window.resize(900, 600);
        window.showLibrary();
        waitFor([&window] { return window.isVisible(); });
        settle(400);

        auto *list = window.findChild<QListView *>();
        list->setCurrentIndex(list->model()->index(1, 0));
        settle(150);
        const int rolled = rollUntilClipped(list, 6);
        const int target = bottomClippedRow(list);
        if (target < 0) {
            out() << "  keine angeschnittene Zeile gefunden — Aufbau untauglich\n";
            return 1;
        }
        const QRect rect = list->visualRect(list->model()->index(target, 0));
        out() << "  Rollwert " << rolled << ", Zielzeile " << target << ": y=" << rect.y()
              << " h=" << rect.height() << ", sichtbar " << (list->viewport()->height() - rect.y())
              << " px von " << rect.height() << "\n";
        clickWatching(list, target, QPoint(rect.center().x(), rect.top() + 5));
        out() << "\n";
    }

    // ---------------------------------------------------------------- C ----
    out() << "=== C Unten angeschnitten, im LibraryWindow, OHNE Gruppengrenze ===\n";
    out() << "  (Vorauswahl liegt in derselben Gruppe wie das Ziel — der Kopf-Vorlauf\n"
             "   aus #57 käme hier auch ohne Mausdruck-Merker nicht in Frage.)\n";
    {
        LibraryWindow window(&store);
        window.setReferenceTime(referenceNow());
        window.resize(900, 600);
        window.showLibrary();
        waitFor([&window] { return window.isVisible(); });
        settle(400);

        auto *list = window.findChild<QListView *>();
        const int rolled = rollUntilClipped(list, 6);
        const int target = bottomClippedRow(list);
        if (target < 0) {
            out() << "  keine angeschnittene Zeile gefunden\n";
        } else {
            // Vorauswahl auf die Zeile direkt über dem Ziel: gleiche Gruppe.
            list->setCurrentIndex(list->model()->index(target - 1, 0));
            settle(200);
            list->verticalScrollBar()->setValue(rolled);
            settle(200);
            const int again = bottomClippedRow(list);
            const QRect rect = list->visualRect(list->model()->index(again, 0));
            out() << "  Rollwert " << rolled << ", Vorauswahl " << list->currentIndex().row()
                  << ", Zielzeile " << again << ": y=" << rect.y() << " h=" << rect.height() << "\n";
            clickWatching(list, again, QPoint(rect.center().x(), rect.top() + 5));
        }
        out() << "\n";
    }

    // ---------------------------------------------------------------- D ----
    out() << "=== D Gibt es überhaupt eine oben angeschnittene Zeile? ===\n";
    out() << "  (Jeder Rollwert wird durchgegangen. ScrollPerItem setzt die Liste je\n"
             "   Zeile ab, also könnte der obere Rand immer bündig sein.)\n";
    {
        LibraryWindow window(&store);
        window.setReferenceTime(referenceNow());
        window.resize(900, 600);
        window.showLibrary();
        waitFor([&window] { return window.isVisible(); });
        settle(400);

        auto *list = window.findChild<QListView *>();
        int found = 0;
        for (int value = 0; value <= list->verticalScrollBar()->maximum(); ++value) {
            list->verticalScrollBar()->setValue(value);
            settle(40);
            const int row = topClippedRow(list);
            if (row >= 0) {
                ++found;
                out() << "  Rollwert " << value << ": Zeile " << row << " oben angeschnitten\n";
            }
        }
        out() << "  ERGEBNIS: " << found << " von " << (list->verticalScrollBar()->maximum() + 1)
              << " Rollwerten zeigen eine oben angeschnittene Zeile\n\n";
    }

    // ---------------------------------------------------------------- I ----
    out() << "=== I Streuung: jeder Rollwert einzeln geklickt (LibraryWindow) ===\n";
    out() << "  Rollwert | Zielzeile | sichtbar | Versatz | aktuell | markiert | Befund\n";
    {
        LibraryWindow window(&store);
        window.setReferenceTime(referenceNow());
        window.resize(900, 600);
        window.showLibrary();
        waitFor([&window] { return window.isVisible(); });
        settle(400);

        auto *list = window.findChild<QListView *>();
        int mismatches = 0;
        int cases = 0;
        for (int value = 0; value <= list->verticalScrollBar()->maximum(); ++value) {
            // Erst die Auswahl weit weglegen, dann rollen. Ohne das bliebe die
            // aktuelle Zeile bei zwei aufeinanderfolgenden Rollwerten dieselbe,
            // currentChanged feuerte nicht, und der Aufbau könnte den Fehler
            // gar nicht zeigen — er sähe aus wie ein Freispruch.
            list->setCurrentIndex(list->model()->index(1, 0));
            settle(60);
            list->verticalScrollBar()->setValue(value);
            settle(60);
            const int target = bottomClippedRow(list);
            if (target < 0) {
                out() << QStringLiteral("  %1 | — keine angeschnittene Zeile\n").arg(value, 8);
                continue;
            }
            ++cases;
            const QRect rect = list->visualRect(list->model()->index(target, 0));
            const int visible = list->viewport()->height() - rect.y();
            const QPoint point(rect.center().x(), rect.top() + qMin(5, visible - 1));
            const int yBefore = rect.y();

            QMouseEvent down(QEvent::MouseButtonPress, point, list->viewport()->mapToGlobal(point),
                             Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(list->viewport(), &down);
            QMouseEvent up(QEvent::MouseButtonRelease, point, list->viewport()->mapToGlobal(point),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(list->viewport(), &up);
            settle(120);

            const int current = list->currentIndex().row();
            const QString marked = selectedRows(list);
            const int shift = list->visualRect(list->model()->index(target, 0)).y() - yBefore;
            const bool apart = marked != QString::number(current);
            if (apart) {
                ++mismatches;
            }
            out() << QStringLiteral("  %1 | %2 | %3 px | %4 px | %5 | %6 | %7\n")
                         .arg(value, 8)
                         .arg(target, 9)
                         .arg(visible, 6)
                         .arg(shift, 5)
                         .arg(current, 7)
                         .arg(marked, 8)
                         .arg(apart ? QStringLiteral("AUSEINANDER") : QStringLiteral("stimmt"));
        }
        out() << "  ERGEBNIS: " << mismatches << " von " << cases
              << " angeschnittenen Fällen gehen auseinander\n\n";
    }

    // ---------------------------------------------------------------- E ----
    out() << "=== E Gegenprobe: blanker QListView, kein currentChanged-scrollTo ===\n";
    {
        QWidget *owner = nullptr;
        BareView view = makeBareView(owner);
        const int rolled = rollUntilClipped(view.list, 6);
        const int target = bottomClippedRow(view.list);
        if (target < 0) {
            out() << "  keine angeschnittene Zeile gefunden\n";
        } else {
            const QRect rect = view.list->visualRect(view.list->model()->index(target, 0));
            out() << "  Rollwert " << rolled << ", Zielzeile " << target << ": y=" << rect.y()
                  << " h=" << rect.height() << "\n";
            clickWatching(view.list, target, QPoint(rect.center().x(), rect.top() + 5));
        }
        delete owner;
        out() << "\n";
    }

    // ---------------------------------------------------------------- F ----
    out() << "=== F Positive Kontrolle: derselbe blanke View MIT synchronem scrollTo ===\n";
    {
        QWidget *owner = nullptr;
        BareView view = makeBareView(owner);
        QObject::connect(view.list->selectionModel(), &QItemSelectionModel::currentChanged,
                         view.list, [list = view.list](const QModelIndex &index, const QModelIndex &) {
                             if (index.isValid()) {
                                 list->scrollTo(index, QAbstractItemView::EnsureVisible);
                             }
                         });
        const int rolled = rollUntilClipped(view.list, 6);
        const int target = bottomClippedRow(view.list);
        if (target < 0) {
            out() << "  keine angeschnittene Zeile gefunden\n";
        } else {
            const QRect rect = view.list->visualRect(view.list->model()->index(target, 0));
            out() << "  Rollwert " << rolled << ", Zielzeile " << target << ": y=" << rect.y() << "\n";
            clickWatching(view.list, target, QPoint(rect.center().x(), rect.top() + 5));
        }
        delete owner;
        out() << "\n";
    }

    // ---------------------------------------------------------------- G ----
    out() << "=== G Lesart 1: scrollTo nachgereicht (QueuedConnection) ===\n";
    {
        QWidget *owner = nullptr;
        BareView view = makeBareView(owner);
        QObject::connect(
            view.list->selectionModel(), &QItemSelectionModel::currentChanged, view.list,
            [list = view.list](const QModelIndex &index, const QModelIndex &) {
                if (index.isValid()) {
                    list->scrollTo(index, QAbstractItemView::EnsureVisible);
                }
            },
            Qt::QueuedConnection);
        const int rolled = rollUntilClipped(view.list, 6);
        const int target = bottomClippedRow(view.list);
        if (target < 0) {
            out() << "  keine angeschnittene Zeile gefunden\n";
        } else {
            const QRect rect = view.list->visualRect(view.list->model()->index(target, 0));
            out() << "  Rollwert " << rolled << ", Zielzeile " << target << ": y=" << rect.y() << "\n";
            clickWatching(view.list, target, QPoint(rect.center().x(), rect.top() + 5));
        }
        delete owner;
        out() << "\n";
    }

    // ---------------------------------------------------------------- H ----
    out() << "=== H Nebenfrage: ändert ScrollPerPixel etwas? (synchrones scrollTo) ===\n";
    out() << "  Geklickt wird am oberen UND am unteren Rand des sichtbaren Streifens,\n"
             "   über einen Fächer von Rollwerten. Ein Aufbau, der nur eine Stelle\n"
             "   trifft, könnte den Fehler verfehlen statt ihn auszuschließen.\n";
    out() << "  Rollwert | Zielzeile | sichtbar | Stelle | Versatz | aktuell | markiert | Befund\n";
    {
        QWidget *owner = nullptr;
        BareView view = makeBareView(owner);
        view.list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        settle(200);
        QObject::connect(view.list->selectionModel(), &QItemSelectionModel::currentChanged,
                         view.list, [list = view.list](const QModelIndex &index, const QModelIndex &) {
                             if (index.isValid()) {
                                 list->scrollTo(index, QAbstractItemView::EnsureVisible);
                             }
                         });
        int mismatches = 0;
        int cases = 0;
        for (int value = 10; value <= view.list->verticalScrollBar()->maximum(); value += 37) {
            for (int edge = 0; edge < 2; ++edge) {
                // Siehe Abschnitt I: ohne das Weglegen der Auswahl feuert
                // currentChanged beim zweiten Klick nicht mehr.
                view.list->setCurrentIndex(view.list->model()->index(1, 0));
                settle(60);
                view.list->verticalScrollBar()->setValue(value);
                settle(60);
                const int target = bottomClippedRow(view.list);
                if (target < 0) {
                    continue;
                }
                ++cases;
                const QRect rect = view.list->visualRect(view.list->model()->index(target, 0));
                const int visible = view.list->viewport()->height() - rect.y();
                const int inRow = edge == 0 ? qMin(3, visible - 1) : visible - 1;
                const QPoint point(rect.center().x(), rect.top() + inRow);
                const int yBefore = rect.y();

                QMouseEvent down(QEvent::MouseButtonPress, point,
                                 view.list->viewport()->mapToGlobal(point), Qt::LeftButton,
                                 Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(view.list->viewport(), &down);
                QMouseEvent up(QEvent::MouseButtonRelease, point,
                               view.list->viewport()->mapToGlobal(point), Qt::LeftButton,
                               Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(view.list->viewport(), &up);
                settle(100);

                const int current = view.list->currentIndex().row();
                const QString marked = selectedRows(view.list);
                const int shift =
                    view.list->visualRect(view.list->model()->index(target, 0)).y() - yBefore;
                const bool apart = marked != QString::number(current);
                if (apart) {
                    ++mismatches;
                }
                out() << QStringLiteral("  %1 | %2 | %3 px | %4 | %5 px | %6 | %7 | %8\n")
                             .arg(value, 8)
                             .arg(target, 9)
                             .arg(visible, 6)
                             .arg(edge == 0 ? QStringLiteral("oben ") : QStringLiteral("unten"))
                             .arg(shift, 5)
                             .arg(current, 7)
                             .arg(marked, 8)
                             .arg(apart ? QStringLiteral("AUSEINANDER") : QStringLiteral("stimmt"));
            }
        }
        out() << "  ERGEBNIS: " << mismatches << " von " << cases << " Fällen gehen auseinander\n";
        delete owner;
        out() << "\n";
    }

    // ---------------------------------------------------------------- J ----
    out() << "=== J Was ein Wechsel des Rollmodus die Tests kostet ===\n";
    out() << "  Vier Testaufbauten in tests/librarytest.cpp setzen den Rollwert auf eine\n"
             "   ZEILENNUMMER (:1406, :1459, :1532, :1595 — `setValue(noteRow(...).row())`).\n"
             "   Was dieselbe Zahl in beiden Rollmodi bedeutet, steht hier.\n";
    {
        LibraryWindow window(&store);
        window.setReferenceTime(referenceNow());
        window.resize(900, 600);
        window.showLibrary();
        waitFor([&window] { return window.isVisible(); });
        settle(400);

        auto *list = window.findChild<QListView *>();
        const QModelIndex head = list->model()->index(9, 0);
        out() << "  Prüfzeile 9 ist ein " << (head.flags().testFlag(Qt::ItemIsSelectable) ? "Notizeintrag" : "Gruppenkopf")
              << "\n";
        for (int mode = 0; mode < 2; ++mode) {
            list->setVerticalScrollMode(mode == 0 ? QAbstractItemView::ScrollPerItem
                                                  : QAbstractItemView::ScrollPerPixel);
            settle(200);
            list->verticalScrollBar()->setValue(8);
            settle(150);
            out() << QStringLiteral("  %1: Rollbereich 0..%2, nach setValue(8) steht Zeile 9 bei y=%3 — %4\n")
                         .arg(mode == 0 ? QStringLiteral("ScrollPerItem ") : QStringLiteral("ScrollPerPixel"))
                         .arg(list->verticalScrollBar()->maximum())
                         .arg(list->visualRect(head).y())
                         .arg(list->viewport()->rect().intersects(list->visualRect(head))
                                  ? QStringLiteral("IM BILD")
                                  : QStringLiteral("außerhalb"));
        }
        out() << "\n";
    }

    out() << "== Ende der Sonde ==\n";
    out().flush();
    return 0;
}
