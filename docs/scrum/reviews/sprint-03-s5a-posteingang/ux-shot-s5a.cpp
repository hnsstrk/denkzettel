// Eigene Bildprüfung des UI/UX-Reviews zu Story S5a (#46, Posteingangs-Gliederung).
// Kein Projekt-Code: linkt nur gegen die gebaute Bibliothek denkzettelui und
// rendert das echte LibraryWindow offscreen (DoD 3, Retro-Beschluss B3).
//
// Neben den Bildern misst das Programm zwei Dinge, die keine Zusicherung des
// Entwicklers trägt und die nur am gemalten Bild zu haben sind:
//   * die linke Textkante jeder Zeile (Kopf vs. Zeitstempel, Wireframe 3a),
//   * die Farbe von Zeitstempel, Betreff und Vorschau, in der Auswahl und
//     daneben (AK 5, ohne Test).
// Beides wird aus dem gerasterten Bild der Liste gelesen, nicht aus dem Code.
//
// Bauen und laufen lassen:
//   g++ -std=c++20 -fPIC $(pkg-config --cflags Qt6Widgets) -I<repo>/src \
//       ux-shot-s5a.cpp build-ux/lib/libdenkzettelui.a build-ux/lib/libdenkzettelstore.a \
//       $(pkg-config --libs Qt6Widgets Qt6Sql) \
//       -lKF6ConfigCore -lKF6ConfigGui -lKF6I18n -lKF6WidgetsAddons -lKF6WindowSystem \
//       -o ux-shot-s5a
//   QT_QPA_PLATFORM=offscreen ./ux-shot-s5a <Zielverzeichnis> -style breeze

#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QImage>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QScrollBar>
#include <QSplitter>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

#include <cstdio>

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

QListView *listOf(QWidget &window)
{
    return window.findChild<QListView *>();
}

void press(QWidget *widget, Qt::Key key)
{
    QKeyEvent down(QEvent::KeyPress, key, Qt::NoModifier);
    QApplication::sendEvent(widget, &down);
    QKeyEvent up(QEvent::KeyRelease, key, Qt::NoModifier);
    QApplication::sendEvent(widget, &up);
    settle(60);
}

void shoot(QWidget &window, const QString &directory, const QString &name)
{
    const QString path = QStringLiteral("%1/s5a-%2.png").arg(directory, name);
    if (!window.grab().save(path)) {
        out() << "  Bild " << path << " konnte nicht geschrieben werden\n";
    }
}

int distance(QRgb a, QRgb b)
{
    return qAbs(qRed(a) - qRed(b)) + qAbs(qGreen(a) - qGreen(b)) + qAbs(qBlue(a) - qBlue(b));
}

/**
 * Die linke Kante der Tinte in einem waagerechten Streifen des Bildes.
 *
 * Die ersten Spalten bleiben aussen vor: dort zieht der Stil den Rahmen der
 * Auswahl, und der wäre sonst die erste gefundene „Tinte".
 */
int inkLeft(const QImage &image, int top, int height)
{
    constexpr int FrameMargin = 4;

    const int bottom = qMin(top + height, image.height());
    if (top < 0 || bottom <= top) {
        return -1;
    }
    // Hintergrund am rechten Rand des Streifens ablesen: dort steht kein Text,
    // in der Auswahl aber die Auswahlfarbe.
    const QRgb background = image.pixel(image.width() - 3, (top + bottom) / 2);

    for (int x = FrameMargin; x < image.width(); ++x) {
        for (int y = top; y < bottom; ++y) {
            if (distance(image.pixel(x, y), background) > 40) {
                return x;
            }
        }
    }
    return -1;
}

/**
 * Die kräftigste Farbe im Streifen — die Federfarbe, die dort gemalt wurde.
 *
 * Der Rand bleibt wie bei inkLeft aussen vor: neben der Auswahlfläche steht der
 * Untergrund der Liste, und der wäre der stärkste Gegensatz im Streifen.
 */
QColor inkColor(const QImage &image, int top, int height)
{
    constexpr int FrameMargin = 4;

    const int bottom = qMin(top + height, image.height());
    if (top < 0 || bottom <= top) {
        return {};
    }
    const QRgb background = image.pixel(image.width() - 3, (top + bottom) / 2);

    QRgb best = background;
    int worst = 0;
    for (int y = top; y < bottom; ++y) {
        for (int x = FrameMargin; x < image.width() - FrameMargin; ++x) {
            const int d = distance(image.pixel(x, y), background);
            if (d > worst) {
                worst = d;
                best = image.pixel(x, y);
            }
        }
    }
    return QColor(best);
}

QString name(const QColor &color)
{
    return color.name(QColor::HexRgb);
}

/**
 * Liest jede sichtbare Zeile der Liste aus dem gerasterten Bild: Höhe, linke
 * Textkante, Farben. Die Zeilenaufteilung folgt den Maßen des Wireframes
 * (9 px oben, Zeitstempel, 3 px Zwischenraum, zwei Textzeilen).
 */
void reportRows(QListView *list, const QString &label)
{
    const QImage image = list->viewport()->grab().toImage();
    const QFontMetrics small(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    const QFontMetrics text(list->font());

    out() << "\n[" << label << "] Zeilen der Liste (Viewport " << list->viewport()->width() << "x"
          << list->viewport()->height() << ")\n";
    const QFont smallFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
    out() << "  Schrift Eintrag: " << QFontInfo(list->font()).family() << " "
          << QFontInfo(list->font()).pointSizeF() << " pt, Zeilenhöhe " << text.height()
          << ", Grundlinienabstand " << text.lineSpacing() << "\n";
    out() << "  Schrift klein  : " << QFontInfo(smallFont).family() << " " << QFontInfo(smallFont).pointSizeF()
          << " pt, Zeilenhöhe " << small.height() << ", Grundlinienabstand " << small.lineSpacing() << "\n";
    out() << "  Palette: Text=" << name(list->palette().color(QPalette::Text))
          << " PlaceholderText=" << name(list->palette().color(QPalette::PlaceholderText))
          << " PlaceholderText-Alpha=" << list->palette().color(QPalette::PlaceholderText).alpha() << " HighlightedText=" << name(list->palette().color(QPalette::HighlightedText))
          << " Highlight=" << name(list->palette().color(QPalette::Highlight)) << "\n";

    for (int row = 0; row < list->model()->rowCount(); ++row) {
        const QModelIndex index = list->model()->index(row, 0);
        const QRect rect = list->visualRect(index);
        const bool head = index.data(NoteListModel::GroupHeaderRole).toBool();
        const bool selected = list->currentIndex() == index;
        const bool visible = list->viewport()->rect().intersects(rect) && rect.height() > 0;

        out() << QStringLiteral("  Zeile %1 %2 y=%3 h=%4 %5%6")
                     .arg(row, 2)
                     .arg(head ? QStringLiteral("KOPF ") : QStringLiteral("Notiz"))
                     .arg(rect.y(), 5)
                     .arg(rect.height(), 3)
                     .arg(selected ? QStringLiteral("[Auswahl] ") : QString(),
                          visible ? QString() : QStringLiteral("[ausserhalb] "));

        if (!visible) {
            out() << index.data(Qt::DisplayRole).toString().left(30) << "\n";
            continue;
        }

        if (head) {
            const int top = rect.y() + (row == 0 ? 6 : 14);
            out() << QStringLiteral("\"%1\" Textkante x=%2 Farbe=%3 (oben %4 unten %5)\n")
                         .arg(index.data(Qt::DisplayRole).toString())
                         .arg(inkLeft(image, top, small.height()))
                         .arg(name(inkColor(image, top, small.height())))
                         .arg(row == 0 ? 6 : 14)
                         .arg(rect.height() - (row == 0 ? 6 : 14) - small.height());
            continue;
        }

        const int stampTop = rect.y() + 9;
        const int subjectTop = stampTop + small.height() + 3;
        const int previewTop = subjectTop + text.height();

        out() << QStringLiteral("\n        Zeitstempel x=%1 Farbe=%2\n")
                     .arg(inkLeft(image, stampTop, small.height()))
                     .arg(name(inkColor(image, stampTop, small.height())));
        out() << QStringLiteral("        Betreff     x=%1 Farbe=%2\n")
                     .arg(inkLeft(image, subjectTop, text.height()))
                     .arg(name(inkColor(image, subjectTop, text.height())));
        out() << QStringLiteral("        Vorschau    x=%1 Farbe=%2\n")
                     .arg(inkLeft(image, previewTop, text.height()))
                     .arg(name(inkColor(image, previewTop, text.height())));
    }
}

void reportLayout(const QString &label, LibraryWindow &window)
{
    auto *search = window.findChild<QLineEdit *>();
    auto *splitter = window.findChild<QSplitter *>();
    QWidget *header = search ? search->parentWidget()->parentWidget() : nullptr;
    QListView *list = listOf(window);

    out() << "\n[" << label << "] Fenster " << window.width() << "x" << window.height() << "\n";
    if (header) {
        out() << "  Kopfzeile: y=" << header->mapTo(&window, QPoint()).y() << " h=" << header->height() << "\n";
    }
    if (splitter) {
        out() << "  Splitter : h=" << splitter->height() << " Liste " << splitter->widget(0)->width()
              << " px / Detail " << splitter->widget(1)->width() << " px (Minimum Liste "
              << splitter->widget(0)->minimumWidth() << ")\n";
    }
    if (list) {
        out() << "  Liste    : Viewport " << list->viewport()->width() << "x" << list->viewport()->height()
              << ", Rollbalken max=" << list->verticalScrollBar()->maximum() << "\n";
    }
}

qint64 add(Store &store, const QString &iso, const QString &content)
{
    Note note;
    note.createdAt = at(iso);
    note.content = content;
    return store.addNote(note).value_or(-1);
}

/** Bestand nach Wireframe 3a — alle fünf Gruppen besetzt, Bezug Fr 31.07.2026. */
void fillInbox(Store &store)
{
    add(store, QStringLiteral("2026-07-31T14:32:00"),
        QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten — als Cronjob auf dem NAS einrichten"));
    add(store, QStringLiteral("2026-07-31T11:05:00"),
        QStringLiteral("Idee für Denkzettel — Bündel-Export erst vorschlagen, wenn mindestens fünf Notizen zum selben Thema da sind, sonst wird der Vault zugemüllt"));
    add(store, QStringLiteral("2026-07-30T21:48:00"), QStringLiteral("journalctl -u whisperd --since today"));
    add(store, QStringLiteral("2026-07-28T09:12:00"),
        QStringLiteral("Mara wegen Wochenende anrufen, Kuchen nicht vergessen"));
    add(store, QStringLiteral("2026-07-23T17:30:00"),
        QStringLiteral("Kategorien-Prompt: Beispiele mitgeben, sonst rät das Modell"));
    add(store, QStringLiteral("2026-07-12T08:05:00"),
        QStringLiteral("Reifenwechsel: Termin bei Kraus vereinbaren, vor dem Urlaub"));
}

/** Sonderfälle 3b: Montag, „Diese Woche" leer, Umbruch, Einzeiler, lange Zeile. */
void fillSpecialCases(Store &store)
{
    add(store, QStringLiteral("2026-08-03T08:30:00"),
        QStringLiteral("Einkauf Samstag\nMehl\nHefe\nZitronen\nSahne\nButter"));
    add(store, QStringLiteral("2026-08-03T07:15:00"), QStringLiteral("Reifen wechseln lassen"));
    add(store, QStringLiteral("2026-08-03T06:40:00"),
        QStringLiteral("Für den Vortrag nächste Woche noch einmal durchgehen, ob die Zahlen zur Auslastung wirklich aus dem letzten Quartal stammen"));
    add(store, QStringLiteral("2026-08-02T21:48:00"), QStringLiteral("Whisper-Warteschlange bei Suspend prüfen"));
    add(store, QStringLiteral("2026-07-30T17:30:00"),
        QStringLiteral("Kategorien-Prompt: Beispiele mitgeben, sonst rät das Modell"));
    add(store, QStringLiteral("2026-07-10T09:00:00"),
        QStringLiteral("Donnerwetter-Wort für den Vortrag suchen — irgendetwas, das lange nachhallt"));
}

/** Zwei volle Gruppen, damit die Grenze ausserhalb des ersten Bildes liegt. */
void fillTwoFullGroups(Store &store)
{
    for (int hour = 8; hour < 16; ++hour) {
        add(store,
            QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')),
            QStringLiteral("Von heute, %1 Uhr — eine Notiz mit genug Text für zwei Zeilen Vorschau").arg(hour));
    }
    for (int hour = 8; hour < 16; ++hour) {
        add(store,
            QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')),
            QStringLiteral("Von gestern, %1 Uhr — eine Notiz mit genug Text für zwei Zeilen Vorschau").arg(hour));
    }
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        out() << "Aufruf: ux-shot-s5a <Zielverzeichnis> [-style breeze]\n";
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString directory = QString::fromLocal8Bit(argv[1]);

    out() << "Stil: " << QApplication::style()->objectName() << ", Locale: " << QLocale().name()
          << ", Plattform: " << QApplication::platformName() << "\n";

    // --- Szene A: Posteingang nach Wireframe 3a, alle fünf Gruppen ---------
    {
        Store store(sandbox.filePath(QStringLiteral("a.db")));
        if (!store.open()) {
            out() << "Store A: " << store.lastError() << "\n";
            return 1;
        }
        fillInbox(store);

        for (const QSize &size : {QSize(900, 600), QSize(1200, 800)}) {
            LibraryWindow window(&store);
            window.setReferenceTime(at(QStringLiteral("2026-07-31T16:20:00")));
            window.resize(size);
            window.showLibrary();
            settle(300);

            QListView *list = listOf(window);
            // Wie in der Zeichnung: die zweite Notiz von heute ist ausgewählt.
            list->setCurrentIndex(list->model()->index(2, 0));
            settle(200);

            const QString label = QStringLiteral("A Posteingang %1x%2").arg(size.width()).arg(size.height());
            reportLayout(label, window);
            reportRows(list, label);
            shoot(window, directory, QStringLiteral("a-posteingang-%1x%2").arg(size.width()).arg(size.height()));
        }

        // Schmalste zulässige Liste (220 px) — trägt die zweizeilige Vorschau?
        {
            LibraryWindow window(&store);
            window.setReferenceTime(at(QStringLiteral("2026-07-31T16:20:00")));
            window.resize(900, 600);
            window.showLibrary();
            settle(200);

            auto *splitter = window.findChild<QSplitter *>();
            splitter->setSizes({1, 899});
            settle(200);

            QListView *list = listOf(window);
            list->setCurrentIndex(list->model()->index(2, 0));
            settle(200);

            reportLayout(QStringLiteral("A schmal (220 px)"), window);
            reportRows(list, QStringLiteral("A schmal (220 px)"));
            shoot(window, directory, QStringLiteral("a-schmal-220px"));
        }

        // Ohne Auswahl — die Farben ohne Auswahlhintergrund, Leerzustand 2.
        {
            LibraryWindow window(&store);
            window.setReferenceTime(at(QStringLiteral("2026-07-31T16:20:00")));
            window.resize(900, 600);
            window.showLibrary();
            settle(300);
            reportRows(listOf(window), QStringLiteral("A ohne Auswahl"));
            shoot(window, directory, QStringLiteral("a-ohne-auswahl-900x600"));
        }
    }

    // --- Szene B: Sonderfälle 3b, Montag, „Diese Woche" leer ---------------
    {
        Store store(sandbox.filePath(QStringLiteral("b.db")));
        if (!store.open()) {
            out() << "Store B: " << store.lastError() << "\n";
            return 1;
        }
        fillSpecialCases(store);

        LibraryWindow window(&store);
        window.setReferenceTime(at(QStringLiteral("2026-08-03T10:00:00")));
        window.resize(900, 600);
        window.showLibrary();
        settle(300);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(1, 0));
        settle(200);

        reportLayout(QStringLiteral("B Sonderfälle (Montag)"), window);
        reportRows(list, QStringLiteral("B Sonderfälle (Montag)"));
        shoot(window, directory, QStringLiteral("b-sonderfaelle-montag-900x600"));

        // Löschen der einzigen Notiz von „Gestern" — der Kopf muss mitgehen.
        for (QAction *action : window.actions()) {
            if (action->text() == QStringLiteral("Löschen")) {
                // Auswahl auf die Notiz von gestern (Notiz 3, Zeile 5).
                for (int row = 0; row < list->model()->rowCount(); ++row) {
                    const QModelIndex index = list->model()->index(row, 0);
                    if (!index.data(NoteListModel::GroupHeaderRole).toBool()
                        && index.data(Qt::DisplayRole).toString().startsWith(QStringLiteral("Whisper"))) {
                        list->setCurrentIndex(index);
                    }
                }
                settle(150);
                action->trigger();
            }
        }
        settle(400);
        reportRows(list, QStringLiteral("B nach dem Löschen der einzigen Notiz von Gestern"));
        shoot(window, directory, QStringLiteral("b-geloescht-kopf-weg-900x600"));

        for (QAction *action : window.actions()) {
            if (action->text() == QStringLiteral("Rückgängig")) {
                action->trigger();
            }
        }
        settle(400);
        reportRows(list, QStringLiteral("B nach dem Rückgängigmachen"));
        shoot(window, directory, QStringLiteral("b-undo-kopf-zurueck-900x600"));
    }

    // --- Szene C: Scrollen über die Gruppengrenze (3b, Fall 4) -------------
    {
        Store store(sandbox.filePath(QStringLiteral("c.db")));
        if (!store.open()) {
            out() << "Store C: " << store.lastError() << "\n";
            return 1;
        }
        fillTwoFullGroups(store);

        // C1: abwärts über die Grenze — von der letzten Notiz „Heute" auf die
        // erste Notiz „Gestern".
        {
            LibraryWindow window(&store);
            window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
            window.resize(900, 600);
            window.showLibrary();
            settle(300);

            QListView *list = listOf(window);
            list->setFocus();
            list->setCurrentIndex(list->model()->index(1, 0));
            settle(150);
            for (int step = 0; step < 7; ++step) {
                press(list, Qt::Key_Down);
            }
            reportRows(list, QStringLiteral("C1 vor der Grenze (letzte Notiz Heute)"));
            shoot(window, directory, QStringLiteral("c1-vor-der-grenze-900x600"));

            press(list, Qt::Key_Down);
            reportRows(list, QStringLiteral("C1 abwärts über die Grenze (erste Notiz Gestern)"));
            shoot(window, directory, QStringLiteral("c1-abwaerts-ueber-die-grenze-900x600"));
        }

        // C2: aufwärts auf die erste Notiz „Gestern" — der geprüfte Fall.
        {
            LibraryWindow window(&store);
            window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
            window.resize(900, 600);
            window.showLibrary();
            settle(300);

            QListView *list = listOf(window);
            list->setFocus();
            list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
            settle(150);
            for (int step = 0; step < 7; ++step) {
                press(list, Qt::Key_Up);
            }
            reportRows(list, QStringLiteral("C2 aufwärts auf die erste Notiz von Gestern"));
            shoot(window, directory, QStringLiteral("c2-aufwaerts-erste-notiz-der-gruppe-900x600"));

            // C3: einen Schritt weiter — auf die letzte Notiz von „Heute".
            // Hier holt die Umsetzung den Kopf nicht ins Bild.
            press(list, Qt::Key_Up);
            reportRows(list, QStringLiteral("C3 aufwärts über die Grenze (letzte Notiz Heute)"));
            shoot(window, directory, QStringLiteral("c3-aufwaerts-letzte-notiz-der-gruppe-900x600"));
        }

        // C4: kleine Gruppe — Auswahl aufwärts auf die zweite Notiz einer
        // dreiköpfigen Gruppe, deren Kopf noch ins Bild gepasst hätte.
        {
            Store small(sandbox.filePath(QStringLiteral("c4.db")));
            if (!small.open()) {
                out() << "Store C4: " << small.lastError() << "\n";
                return 1;
            }
            for (int hour = 8; hour < 14; ++hour) {
                add(small,
                    QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')),
                    QStringLiteral("Von heute, %1 Uhr — Notiz mit genug Text für zwei Zeilen Vorschau").arg(hour));
            }
            for (int hour = 8; hour < 11; ++hour) {
                add(small,
                    QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')),
                    QStringLiteral("Von gestern, %1 Uhr — Notiz mit genug Text für zwei Zeilen Vorschau").arg(hour));
            }
            for (int hour = 8; hour < 12; ++hour) {
                add(small,
                    QStringLiteral("2026-07-20T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')),
                    QStringLiteral("Von letzter Woche, %1 Uhr — Notiz mit Text für zwei Zeilen").arg(hour));
            }

            LibraryWindow window(&small);
            window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
            window.resize(900, 600);
            window.showLibrary();
            settle(300);

            QListView *list = listOf(window);
            list->setFocus();
            list->setCurrentIndex(list->model()->index(list->model()->rowCount() - 1, 0));
            settle(150);
            for (int step = 0; step < 4; ++step) {
                press(list, Qt::Key_Up);
            }
            reportRows(list, QStringLiteral("C4 aufwärts auf die letzte Notiz einer kleinen Gruppe"));
            shoot(window, directory, QStringLiteral("c4-kleine-gruppe-aufwaerts-900x600"));
        }
    }

    // --- Szene D: Leerzustand (unverändert, Wireframe 2c) ------------------
    {
        Store store(sandbox.filePath(QStringLiteral("d.db")));
        if (!store.open()) {
            out() << "Store D: " << store.lastError() << "\n";
            return 1;
        }

        LibraryWindow window(&store);
        window.setReferenceTime(at(QStringLiteral("2026-07-31T16:20:00")));
        window.resize(900, 600);
        window.showLibrary();
        settle(300);
        reportLayout(QStringLiteral("D leere Bibliothek"), window);
        shoot(window, directory, QStringLiteral("d-leere-bibliothek-900x600"));
    }

    out().flush();
    return 0;
}
