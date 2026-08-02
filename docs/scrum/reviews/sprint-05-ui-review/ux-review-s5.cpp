// UI-Review Sprint 5 (#57, #58, #66, #67) — eigenes Prüfmittel des UI/UX-Reviews.
// Kein Projekt-Code: linkt nur gegen die gebaute Bibliothek denkzettelui und
// rendert das echte LibraryWindow offscreen (DoD 3, Retro-Beschluss B3).
//
// Zwei Strecken:
//   A  #58 — Schemawechsel am STEHENDEN Fenster, mit sichtbarem Detailbereich.
//      Neben den zwei Breeze-Schemata eine Signalfarbe: Ein Label, das der
//      Rolle folgt, wird magenta; ein eingefrorenes bliebe grau. Das ist der
//      Nachweis, den ein Bildpaar in der falschen Richtung nicht erbringt.
//   B  #57 — Rollwert-Szenen, die im Szenenprogramm des S8-Reviews NICHT
//      vorkommen: teilweise sichtbare Zeile, Klick in derselben Gruppe,
//      Tastendruck NACH einem Klick, Klick auf Kopf und Leerraum.
//
// Aufruf: QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
//         ux-review-s5 <Zielverzeichnis> -style breeze

#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QHash>
#include <QKeyEvent>
#include <QLabel>
#include <QListView>
#include <QMouseEvent>
#include <QPushButton>
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

void addNote(Store &store, const QString &content, const QString &iso)
{
    Note note;
    note.content = content;
    note.createdAt = at(iso);
    if (!store.addNote(note).has_value()) {
        out() << "Notiz liess sich nicht speichern\n";
    }
}

/** Acht Notizen eines Tages, stündlich ab 08:00 — Bestand des S8-Reviews. */
void addGroup(Store &store, const QString &day, int count, const QString &what)
{
    for (int index = 0; index < count; ++index) {
        addNote(store,
                QStringLiteral("Notiz %1 von %2 — Text, der über eine Zeile hinausgeht und "
                               "in der Liste umbricht, damit die Zeilenhöhe stimmt.")
                        .arg(index + 1)
                        .arg(what),
                QStringLiteral("%1T%2:00:00").arg(day, QString::number(8 + index).rightJustified(2, QLatin1Char('0'))));
    }
}

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
    palette.setColor(QPalette::PlaceholderText, placeholder);
    palette.setColor(QPalette::Highlight, QColor(0x3d, 0xae, 0xe9));
    palette.setColor(QPalette::HighlightedText, dark ? QColor(0xfc, 0xfc, 0xfc) : QColor(0xff, 0xff, 0xff));
    palette.setColor(QPalette::Link, dark ? QColor(0x1d, 0x99, 0xf3) : QColor(0x29, 0x80, 0xb9));

    return palette;
}

/** Dieselbe helle Palette, aber die Platzhalterfarbe ist unverwechselbar. */
QPalette signalPalette()
{
    QPalette palette = breezePalette(false);
    palette.setColor(QPalette::PlaceholderText, QColor(0xff, 0x00, 0xff));

    return palette;
}

void applyScheme(const QPalette &palette)
{
    qApp->setPalette(palette);
    // Qt stellt die Palette per gepostetem Ereignis zu (Stolperstelle aus #54,
    // im Issue #58 ausdrücklich genannt).
    QCoreApplication::processEvents();
    settle(120);
}

QLabel *labelWithText(QWidget &window, const QString &text)
{
    const QList<QLabel *> labels = window.findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->isVisible() && label->text() == text) {
            return label;
        }
    }
    return nullptr;
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

int distance(QRgb a, QRgb b)
{
    return qAbs(qRed(a) - qRed(b)) + qAbs(qGreen(a) - qGreen(b)) + qAbs(qBlue(a) - qBlue(b));
}

/**
 * Liest Hintergrund- und Textfarbe eines Labels AUS DEM BILD des Fensters.
 *
 * Nicht aus der Palette: gefragt ist, was gemalt wurde, nicht was zugesichert
 * ist. Der Ausschnitt kommt aus der Widget-Geometrie, nicht aus abgezählten
 * Koordinaten — so hängt die Messung an keinem Layout-Stand.
 */
void readColours(const QPixmap &picture, QWidget &window, QLabel *label, const QString &what)
{
    if (!label) {
        out() << "  " << what << ": Label nicht gefunden\n";
        return;
    }

    const QPoint topLeft = label->mapTo(&window, QPoint(0, 0));
    const QImage patch = picture.copy(QRect(topLeft, label->size())).toImage();

    QHash<QRgb, int> counts;
    for (int y = 0; y < patch.height(); ++y) {
        for (int x = 0; x < patch.width(); ++x) {
            ++counts[patch.pixel(x, y)];
        }
    }

    QRgb background = 0;
    int most = -1;
    for (auto it = counts.cbegin(); it != counts.cend(); ++it) {
        if (it.value() > most) {
            most = it.value();
            background = it.key();
        }
    }

    QRgb ink = background;
    int farthest = 0;
    for (auto it = counts.cbegin(); it != counts.cend(); ++it) {
        const int gap = distance(it.key(), background);
        if (gap > farthest) {
            farthest = gap;
            ink = it.key();
        }
    }

    out() << QStringLiteral("  %1: Hintergrund %2, Text %3 (Text „%4“)\n")
                     .arg(what,
                          QColor(background).name(),
                          QColor(ink).name(),
                          label->text().left(40));
}

void shoot(QWidget &window, const QString &directory, const QString &name)
{
    settle(150);
    if (!window.grab().save(QStringLiteral("%1/%2.png").arg(directory, name))) {
        out() << "  Bild " << name << " liess sich nicht schreiben\n";
    } else {
        out() << "  geschrieben: " << name << ".png\n";
    }
}

QModelIndex headOf(QListView *list, int row)
{
    for (int above = row; above >= 0; --above) {
        const QModelIndex index = list->model()->index(above, 0);
        if (!index.flags().testFlag(Qt::ItemIsSelectable)) {
            return index;
        }
    }
    return QModelIndex();
}

void clickAt(QListView *list, const QPoint &point)
{
    QMouseEvent down(QEvent::MouseButtonPress, point, list->viewport()->mapToGlobal(point),
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(list->viewport(), &down);
    QMouseEvent up(QEvent::MouseButtonRelease, point, list->viewport()->mapToGlobal(point),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(list->viewport(), &up);
    settle(150);
}

/** Derselbe Klick, aber zwischen Druck und Loslassen wird gemessen. */
void clickAtWatching(QListView *list, const QPoint &point)
{
    const auto state = [list](const QString &when) {
        const QModelIndexList selected = list->selectionModel()->selectedIndexes();
        out() << QStringLiteral("  %1: Rollwert %2, aktuelle Zeile %3, markiert %4\n")
                         .arg(when)
                         .arg(list->verticalScrollBar()->value())
                         .arg(list->currentIndex().row())
                         .arg(selected.isEmpty() ? QStringLiteral("keine")
                                                 : QString::number(selected.first().row()));
    };

    state(QStringLiteral("vor dem Druck"));

    QMouseEvent down(QEvent::MouseButtonPress, point, list->viewport()->mapToGlobal(point),
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(list->viewport(), &down);
    settle(150);
    state(QStringLiteral("nach dem Druck"));

    QMouseEvent up(QEvent::MouseButtonRelease, point, list->viewport()->mapToGlobal(point),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(list->viewport(), &up);
    settle(150);
    state(QStringLiteral("nach dem Loslassen"));
}

void pressKey(QListView *list, Qt::Key key)
{
    QKeyEvent down(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::sendEvent(list, &down);
    QKeyEvent up(QEvent::KeyRelease, key, Qt::NoModifier);
    QApplication::sendEvent(list, &up);
    settle(120);
}

void reportRow(QListView *list, const QString &what, int before)
{
    const int row = list->currentIndex().row();
    const QModelIndex head = headOf(list, row);
    const QRect headRect = head.isValid() ? list->visualRect(head) : QRect();
    const QRect selected = list->visualRect(list->model()->index(row, 0));

    out() << QStringLiteral("  %1: Auswahl Zeile %2, Rollwert %3 → %4, Kopf y=%5 (%6), Auswahl y=%7 (%8)\n")
                     .arg(what)
                     .arg(row)
                     .arg(before)
                     .arg(list->verticalScrollBar()->value())
                     .arg(headRect.y())
                     .arg(list->viewport()->rect().contains(headRect) ? QStringLiteral("im Bild")
                                                                      : QStringLiteral("draussen"))
                     .arg(selected.y())
                     .arg(list->viewport()->rect().contains(selected) ? QStringLiteral("ganz sichtbar")
                                                                      : QStringLiteral("angeschnitten"));
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        out() << "Aufruf: ux-review-s5 <Zielverzeichnis> [-style breeze]\n";
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
    const QString directory = QString::fromLocal8Bit(argv[1]);

    // ===== Strecke A — #58, Schemawechsel am stehenden Fenster =============
    {
        applyScheme(breezePalette(false));

        Store store(sandbox.filePath(QStringLiteral("schema.db")));
        if (!store.open()) {
            out() << "Store: " << store.lastError() << "\n";
            return 1;
        }
        addNote(store, QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten"),
                QStringLiteral("2026-07-31T14:32:00"));
        addNote(store, QStringLiteral("Transkript: Idee für Denkzettel — den Bündel-Export erst vorschlagen, "
                                      "wenn mindestens fünf Notizen zum selben Thema da sind."),
                QStringLiteral("2026-07-31T11:05:00"));
        addNote(store, QStringLiteral("journalctl -u whisperd --since today"),
                QStringLiteral("2026-07-30T21:48:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
        window.resize(900, 600);
        window.showLibrary();
        settle(300);

        auto *list = window.findChild<QListView *>();
        // Zeilen: Kopf „Heute“, zwei Notizen, Kopf „Gestern“, deren Notiz.
        list->setCurrentIndex(list->model()->index(4, 0));
        settle(200);

        out() << "\n=== A  #58 Schemawechsel, ein Fenster, kein Neuaufbau ===\n";
        out() << "helles Schema, Lesezustand:\n";
        QPixmap picture = window.grab();
        readColours(picture, window, labelWithText(window, QStringLiteral("Gestern 21:48")),
                    QStringLiteral("Zeitstempel des Detailbereichs"));
        shoot(window, directory, QStringLiteral("s5-58-a-hell-lesen"));

        applyScheme(breezePalette(true));
        out() << "nach dem Wechsel auf dunkel, dasselbe Fenster:\n";
        picture = window.grab();
        readColours(picture, window, labelWithText(window, QStringLiteral("Gestern 21:48")),
                    QStringLiteral("Zeitstempel des Detailbereichs"));
        shoot(window, directory, QStringLiteral("s5-58-b-dunkel-lesen"));

        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        settle(200);
        out() << "Bearbeiten-Zustand unter dunkel:\n";
        picture = window.grab();
        readColours(picture, window,
                    labelWithText(window, QStringLiteral("Esc bricht ab · Strg+Enter speichert")),
                    QStringLiteral("Fusszeilenhinweis"));
        readColours(picture, window, labelWithText(window, QStringLiteral("Kategorie")),
                    QStringLiteral("Beschriftung „Kategorie“"));
        shoot(window, directory, QStringLiteral("s5-58-c-dunkel-bearbeiten"));

        // Der eigentliche Nachweis: eine Farbe, die in keinem Schema vorkommt.
        // Folgt das Label der Rolle, ist es magenta; ein eingefrorenes bliebe
        // in der Farbe des vorigen Schemas stehen.
        applyScheme(signalPalette());
        out() << "Signalfarbe #ff00ff auf PlaceholderText:\n";
        picture = window.grab();
        readColours(picture, window,
                    labelWithText(window, QStringLiteral("Esc bricht ab · Strg+Enter speichert")),
                    QStringLiteral("Fusszeilenhinweis"));
        readColours(picture, window, labelWithText(window, QStringLiteral("Gestern 21:48")),
                    QStringLiteral("Zeitstempel des Detailbereichs"));
        shoot(window, directory, QStringLiteral("s5-58-d-signalfarbe"));

        applyScheme(breezePalette(false));
        out() << "zurück auf hell:\n";
        picture = window.grab();
        readColours(picture, window,
                    labelWithText(window, QStringLiteral("Esc bricht ab · Strg+Enter speichert")),
                    QStringLiteral("Fusszeilenhinweis"));
        shoot(window, directory, QStringLiteral("s5-58-e-hell-bearbeiten"));
    }

    // ===== Strecke B — #57, Wege, die das S8-Szenenprogramm nicht hat ======
    {
        applyScheme(breezePalette(true));

        Store store(sandbox.filePath(QStringLiteral("gross.db")));
        if (!store.open()) {
            out() << "Store: " << store.lastError() << "\n";
            return 1;
        }
        addGroup(store, QStringLiteral("2026-07-31"), 8, QStringLiteral("heute"));
        addGroup(store, QStringLiteral("2026-07-30"), 8, QStringLiteral("gestern"));

        const auto openWindow = [&store](LibraryWindow &window, QListView *&list) {
            window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
            window.resize(900, 600);
            window.showLibrary();
            settle(300);
            list = window.findChild<QListView *>();
            list->setFocus();
        };

        // --- Z1: Klick auf eine nur teilweise sichtbare Zeile --------------
        {
            LibraryWindow window(&store);
            QListView *list = nullptr;
            openWindow(window, list);
            out() << "\n=== B1 Klick auf eine ANGESCHNITTENE Zeile ===\n";

            list->setCurrentIndex(list->model()->index(11, 0));
            settle(150);
            list->verticalScrollBar()->setValue(6);
            settle(150);

            // Die unterste Zeile, die noch angeschnitten im Bild steht.
            int target = -1;
            for (int row = 0; row < list->model()->rowCount(); ++row) {
                const QModelIndex index = list->model()->index(row, 0);
                if (!index.flags().testFlag(Qt::ItemIsSelectable)) {
                    continue;
                }
                const QRect rect = list->visualRect(index);
                if (rect.top() < list->viewport()->height() && rect.bottom() > list->viewport()->height()) {
                    target = row;
                    break;
                }
            }
            if (target < 0) {
                out() << "  keine angeschnittene Zeile gefunden\n";
            } else {
                const QRect rect = list->visualRect(list->model()->index(target, 0));
                out() << "  Zielzeile " << target << ": y=" << rect.y() << " h=" << rect.height()
                      << ", Bildhöhe " << list->viewport()->height() << "\n";
                const int before = list->verticalScrollBar()->value();
                shoot(window, directory, QStringLiteral("s5-57-b1a-vor-dem-klick"));
                clickAt(list, QPoint(rect.center().x(), rect.top() + 5));
                reportRow(list, QStringLiteral("nach dem Klick"), before);
                shoot(window, directory, QStringLiteral("s5-57-b1b-nach-dem-klick"));
            }
        }

        // --- Z2: Klick in derselben Gruppe ---------------------------------
        {
            LibraryWindow window(&store);
            QListView *list = nullptr;
            openWindow(window, list);
            out() << "\n=== B2 Klick auf eine sichtbare Zeile DERSELBEN Gruppe ===\n";

            list->setCurrentIndex(list->model()->index(4, 0));
            settle(150);
            list->verticalScrollBar()->setValue(3);
            settle(150);
            const int before = list->verticalScrollBar()->value();

            const QRect rect = list->visualRect(list->model()->index(6, 0));
            if (list->viewport()->rect().contains(rect)) {
                clickAt(list, rect.center());
                reportRow(list, QStringLiteral("nach dem Klick"), before);
            } else {
                out() << "  Zeile 6 steht nicht ganz im Bild (y=" << rect.y() << ")\n";
            }
        }

        // --- Z3: Tastendruck NACH einem Klick über die Gruppengrenze -------
        {
            LibraryWindow window(&store);
            QListView *list = nullptr;
            openWindow(window, list);
            out() << "\n=== B3 Erst klicken, dann per Taste über die Grenze ===\n";

            list->setCurrentIndex(list->model()->index(7, 0));
            settle(150);
            const QRect rect = list->visualRect(list->model()->index(7, 0));
            const int before = list->verticalScrollBar()->value();
            clickAt(list, rect.center());
            reportRow(list, QStringLiteral("Klick auf die eigene Zeile"), before);

            const int beforeKey = list->verticalScrollBar()->value();
            pressKey(list, Qt::Key_Down);
            pressKey(list, Qt::Key_Down);
            reportRow(list, QStringLiteral("zweimal Pfeil ab (Grenzübertritt)"), beforeKey);
            shoot(window, directory, QStringLiteral("s5-57-b3-taste-nach-klick"));
        }

        // --- Z4: Klick auf einen Gruppenkopf, dann Taste --------------------
        {
            LibraryWindow window(&store);
            QListView *list = nullptr;
            openWindow(window, list);
            out() << "\n=== B4 Klick auf einen Gruppenkopf (wählt nichts), dann Taste ===\n";

            list->setCurrentIndex(list->model()->index(7, 0));
            settle(150);
            const QModelIndex head = headOf(list, 7);
            const QRect headRect = list->visualRect(head);
            if (headRect.isValid() && list->viewport()->rect().intersects(headRect)) {
                clickAt(list, headRect.center());
                out() << "  Auswahl nach dem Kopfklick: Zeile " << list->currentIndex().row() << "\n";
            } else {
                out() << "  Kopf steht nicht im Bild — Klick entfällt\n";
            }

            const int before = list->verticalScrollBar()->value();
            pressKey(list, Qt::Key_Down);
            pressKey(list, Qt::Key_Down);
            reportRow(list, QStringLiteral("zweimal Pfeil ab (Grenzübertritt)"), before);
            shoot(window, directory, QStringLiteral("s5-57-b4-taste-nach-kopfklick"));
        }

        // --- Z6: Derselbe Fall wie Z1, Schritt für Schritt gemessen --------
        // Rutscht die Liste zwischen Druck und Loslassen, liegt beim
        // Loslassen eine andere Zeile unter dem Zeiger.
        {
            LibraryWindow window(&store);
            QListView *list = nullptr;
            openWindow(window, list);
            out() << "\n=== B6 Klick auf eine angeschnittene Zeile, Schritt für Schritt ===\n";

            list->setCurrentIndex(list->model()->index(11, 0));
            settle(150);
            list->verticalScrollBar()->setValue(6);
            settle(150);

            int target = -1;
            for (int row = 0; row < list->model()->rowCount(); ++row) {
                const QModelIndex index = list->model()->index(row, 0);
                if (!index.flags().testFlag(Qt::ItemIsSelectable)) {
                    continue;
                }
                const QRect rect = list->visualRect(index);
                if (rect.top() < list->viewport()->height() && rect.bottom() > list->viewport()->height()) {
                    target = row;
                    break;
                }
            }
            if (target < 0) {
                out() << "  keine angeschnittene Zeile gefunden\n";
            } else {
                const QRect rect = list->visualRect(list->model()->index(target, 0));
                out() << "  Zielzeile " << target << ": y=" << rect.y() << " h=" << rect.height() << "\n";
                clickAtWatching(list, QPoint(rect.center().x(), rect.top() + 5));
                out() << "  Zeile unter dem Zeiger am Ende: "
                      << list->indexAt(QPoint(rect.center().x(), rect.top() + 5)).row() << "\n";
                shoot(window, directory, QStringLiteral("s5-57-b6-nach-dem-loslassen"));
            }
        }

        // --- Z5: Klick in den Leerraum unter der Liste, dann Taste ----------
        {
            LibraryWindow window(&store);
            QListView *list = nullptr;
            openWindow(window, list);
            out() << "\n=== B5 Klick in den Leerraum, dann Taste ===\n";

            list->setCurrentIndex(list->model()->index(7, 0));
            settle(150);
            list->verticalScrollBar()->setValue(list->verticalScrollBar()->maximum());
            settle(150);

            const int lastRow = list->model()->rowCount() - 1;
            const QRect lastRect = list->visualRect(list->model()->index(lastRow, 0));
            if (lastRect.bottom() + 10 < list->viewport()->height()) {
                clickAt(list, QPoint(list->viewport()->width() / 2, lastRect.bottom() + 8));
                out() << "  Auswahl nach dem Leerraumklick: Zeile " << list->currentIndex().row() << "\n";
            } else {
                out() << "  kein Leerraum unter der letzten Zeile — Klick entfällt\n";
            }

            list->setCurrentIndex(list->model()->index(7, 0));
            settle(150);
            const int before = list->verticalScrollBar()->value();
            pressKey(list, Qt::Key_Down);
            pressKey(list, Qt::Key_Down);
            reportRow(list, QStringLiteral("zweimal Pfeil ab (Grenzübertritt)"), before);
        }
    }

    out() << "\nfertig\n";
    out().flush();

    return 0;
}
