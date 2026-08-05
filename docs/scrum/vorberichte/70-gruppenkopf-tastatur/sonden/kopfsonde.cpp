// Vorprüfung #70 — Sonde: Wo genau steht der Gruppenkopf nach den vier
// Bewegungen, um die es in den Akzeptanzkriterien geht?
//
// Gemessen wird der Rollwert (verticalScrollBar()->value()) vor und nach der
// Eingabe sowie die y-Lage des Kopfes im Viewport — nicht das Endbild. Die
// Sonde ändert keine Zeile Produktivcode; sie misst den Stand, gegen den #70
// gebaut würde.

#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QDir>
#include <QListView>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

static QTextStream out(stdout);

static QDateTime at(const QString &iso)
{
    return QDateTime::fromString(iso, Qt::ISODate);
}

static QListView *listOf(QWidget &window)
{
    return window.findChild<QListView *>();
}

static NoteListModel *modelOf(QListView *list)
{
    return qobject_cast<NoteListModel *>(list->model());
}

/** Zeile der n-ten Notiz (0-basiert, neueste zuerst) als QModelIndex. */
static QModelIndex noteRow(QListView *list, int noteIndex)
{
    return modelOf(list)->index(modelOf(list)->rowOfNote(noteIndex));
}

/** Kopfzeile über der Notiz — dieselbe Suche, die groupHeadOf() im Code führt. */
static QModelIndex headOf(QListView *list, const QModelIndex &note)
{
    for (int row = note.row() - 1; row >= 0; --row) {
        const QModelIndex candidate = modelOf(list)->index(row);
        if (candidate.data(NoteListModel::GroupHeaderRole).toBool()) {
            return candidate;
        }
    }
    return {};
}

static QString rectOf(QListView *list, const QModelIndex &index)
{
    const QRect r = list->visualRect(index);
    return QStringLiteral("y=%1 h=%2 unten=%3").arg(r.y()).arg(r.height()).arg(r.bottom());
}

static bool wholeInView(QListView *list, const QModelIndex &index)
{
    return list->viewport()->rect().contains(list->visualRect(index));
}

/** Drei volle Gruppen: Heute 0–7, Gestern 8–15, Letzte Woche 16–23. */
static void fill(Store &store)
{
    auto add = [&store](const QString &content, const QString &iso) {
        Note note;
        note.content = content;
        note.createdAt = at(iso);
        store.addNote(note);
    };
    for (int hour = 15; hour >= 8; --hour) {
        add(QStringLiteral("von heute, %1 Uhr").arg(hour),
            QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 15; hour >= 8; --hour) {
        add(QStringLiteral("von gestern, %1 Uhr").arg(hour),
            QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 15; hour >= 8; --hour) {
        add(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
            QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QTemporaryDir tmp;
    Store store(tmp.filePath(QStringLiteral("kopfsonde.db")));
    if (!store.open()) {
        out << "Store öffnet nicht: " << store.lastError() << "\n";
        return 1;
    }
    fill(store);

    out << "Plattform: " << app.platformName()
        << "  QT_SCALE_FACTOR=" << qEnvironmentVariable("QT_SCALE_FACTOR", QStringLiteral("(nicht gesetzt)"))
        << "  Theme=" << qEnvironmentVariable("QT_QPA_PLATFORMTHEME", QStringLiteral("(nicht gesetzt)")) << "\n";

    LibraryWindow window(&store);
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window.resize(900, 600);
    window.showLibrary();
    QTest::qWaitForWindowExposed(&window);

    QListView *list = listOf(window);
    NoteListModel *model = modelOf(list);

    out << "\n== A · Maße der Liste ==\n";
    out << "Notizen: " << model->noteCount() << "  Zeilen: " << model->rowCount() << "\n";
    out << "Viewport: " << list->viewport()->height() << " px hoch, Rollbereich 0.."
        << list->verticalScrollBar()->maximum() << "\n";
    out << "Kopfzeile 0 (\"Heute\"): " << rectOf(list, model->index(0)) << "\n";
    out << "Notizzeile 1:           " << rectOf(list, model->index(1)) << "\n";
    const QModelIndex gestern = headOf(list, noteRow(list, 8));
    out << "Kopf \"Gestern\" in Zeile " << gestern.row() << ": "
        << gestern.data(Qt::DisplayRole).toString() << ", Höhe "
        << list->visualRect(gestern).height() << " px\n";

    // ------------------------------------------------------------------
    // B — Der Befund N2: aufwärts auf die erste Notiz der Gruppe „Gestern".
    // Vorgänger ist Notiz 9, also DIESELBE Gruppe: kein Grenzübertritt.
    // ------------------------------------------------------------------
    out << "\n== B · Pfeil aufwärts auf die ERSTE Notiz der Gruppe (Befund N2) ==\n";
    list->setCurrentIndex(noteRow(list, 9));
    // So gerollt, dass Notiz 9 oben steht und der Kopf „Gestern" knapp draußen
    // ist — die Lage, aus der der Nutzer eine Zeile hochgeht.
    list->verticalScrollBar()->setValue(noteRow(list, 9).row());
    qApp->processEvents();
    out << "vorher:  Rollwert=" << list->verticalScrollBar()->value()
        << "  Kopf " << rectOf(list, gestern) << "  Kopf ganz im Bild? "
        << (wholeInView(list, gestern) ? "ja" : "nein") << "\n";
    out << "         Ziel (Notiz 8) " << rectOf(list, noteRow(list, 8))
        << "  ganz im Bild? " << (wholeInView(list, noteRow(list, 8)) ? "ja" : "nein") << "\n";

    const int beforeUp = list->verticalScrollBar()->value();
    QTest::keyClick(list, Qt::Key_Up);
    qApp->processEvents();

    out << "nachher: Rollwert=" << list->verticalScrollBar()->value()
        << " (Δ " << (list->verticalScrollBar()->value() - beforeUp) << " Zeilen)\n";
    out << "         Auswahl steht auf Notizzeile " << list->currentIndex().row()
        << " (erwartet " << noteRow(list, 8).row() << ")\n";
    out << "         Kopf " << rectOf(list, gestern) << "  Kopf ganz im Bild? "
        << (wholeInView(list, gestern) ? "JA" : "NEIN — das ist der Befund") << "\n";

    // ------------------------------------------------------------------
    // C — Der Prüfsatz aus Sprint 3: Grenzübertritt auf eine schon ganz
    // sichtbare Zeile. Muss grün bleiben.
    // ------------------------------------------------------------------
    out << "\n== C · Grenzübertritt aufwärts auf eine schon sichtbare Zeile (Prüfsatz Sprint 3) ==\n";
    list->setCurrentIndex(noteRow(list, 16));
    list->verticalScrollBar()->setValue(noteRow(list, 15).row());
    qApp->processEvents();
    out << "vorher:  Rollwert=" << list->verticalScrollBar()->value() << "  Kopf \"Gestern\" "
        << rectOf(list, gestern) << "  ganz im Bild? " << (wholeInView(list, gestern) ? "ja" : "nein")
        << "\n";
    out << "         Ziel (Notiz 15, letzte der Gruppe) " << rectOf(list, noteRow(list, 15))
        << "  ganz im Bild? " << (wholeInView(list, noteRow(list, 15)) ? "ja" : "nein") << "\n";
    const int beforeCross = list->verticalScrollBar()->value();
    QTest::keyClick(list, Qt::Key_Up);
    qApp->processEvents();
    out << "nachher: Rollwert=" << list->verticalScrollBar()->value() << " (Δ "
        << (list->verticalScrollBar()->value() - beforeCross) << ")  Kopf " << rectOf(list, gestern)
        << "  ganz im Bild? " << (wholeInView(list, gestern) ? "ja" : "nein") << "\n";

    // ------------------------------------------------------------------
    // D — Bewegung INNERHALB der Gruppe auf eine Notiz, die nicht die erste
    // ist. Die Liste darf sich nicht zusätzlich bewegen (AK 4).
    // ------------------------------------------------------------------
    out << "\n== D · Bewegung innerhalb der Gruppe, Ziel ist nicht die erste Notiz (AK 4) ==\n";
    list->setCurrentIndex(noteRow(list, 11));
    list->verticalScrollBar()->setValue(noteRow(list, 9).row());
    qApp->processEvents();
    const int beforeInner = list->verticalScrollBar()->value();
    out << "vorher:  Rollwert=" << beforeInner << "  Ziel (Notiz 10) "
        << rectOf(list, noteRow(list, 10)) << "  ganz im Bild? "
        << (wholeInView(list, noteRow(list, 10)) ? "ja" : "nein") << "\n";
    QTest::keyClick(list, Qt::Key_Up);
    qApp->processEvents();
    out << "nachher: Rollwert=" << list->verticalScrollBar()->value() << " (Δ "
        << (list->verticalScrollBar()->value() - beforeInner) << ")  Auswahl auf Zeile "
        << list->currentIndex().row() << "\n";

    // ------------------------------------------------------------------
    // E — Der Klickpfad aus #57: Klick auf eine sichtbare Notiz einer anderen
    // Gruppe. Der Rollwert muss stehen bleiben (AK 3).
    // ------------------------------------------------------------------
    out << "\n== E · Klick auf eine sichtbare Notiz einer anderen Gruppe (#57, AK 3) ==\n";
    list->setCurrentIndex(noteRow(list, 16));
    list->verticalScrollBar()->setValue(noteRow(list, 8).row());
    qApp->processEvents();
    const QModelIndex clickTarget = noteRow(list, 10);
    out << "vorher:  Rollwert=" << list->verticalScrollBar()->value() << "  Kopf \"Gestern\" "
        << rectOf(list, gestern) << "  ganz im Bild? " << (wholeInView(list, gestern) ? "ja" : "nein")
        << "\n";
    out << "         Klickziel (Notiz 10) " << rectOf(list, clickTarget) << "  ganz im Bild? "
        << (wholeInView(list, clickTarget) ? "ja" : "nein") << "\n";
    const int beforeClick = list->verticalScrollBar()->value();
    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier,
                      list->visualRect(clickTarget).center());
    qApp->processEvents();
    out << "nachher: Rollwert=" << list->verticalScrollBar()->value() << " (Δ "
        << (list->verticalScrollBar()->value() - beforeClick) << ")  Auswahl auf Zeile "
        << list->currentIndex().row() << " (erwartet " << clickTarget.row() << ")\n";
    out << "         Kopf " << rectOf(list, gestern) << "  ganz im Bild? "
        << (wholeInView(list, gestern) ? "ja" : "nein") << "\n";

    // ------------------------------------------------------------------
    // F — Wie tief liegt der Kopf im Fall B unter der Kante, in Pixeln?
    // Die Zahl aus dem Issue (−35 px) wird hier gegen den heutigen Stand
    // gehalten.
    // ------------------------------------------------------------------
    out << "\n== F · Reine Zeilenmaße ==\n";
    out << "Kopfzeile hoch:  " << list->visualRect(model->index(0)).height() << " px\n";
    out << "Notizzeile hoch: " << list->visualRect(model->index(1)).height() << " px\n";
    out << "Viewport hoch:   " << list->viewport()->height() << " px\n";
    out << "Rollmodus: " << (list->verticalScrollMode() == QAbstractItemView::ScrollPerItem
                                 ? "ScrollPerItem (eine Zeile je Rollschritt)"
                                 : "ScrollPerPixel")
        << "\n";

    // ------------------------------------------------------------------
    // G — Was täte der vorgeschlagene Bauweg? Er ändert nur die BEDINGUNG,
    // unter der die beiden vorhandenen scrollTo-Zeilen laufen. Also werden
    // hier genau diese beiden Zeilen von Hand nachgefahren, im Zustand aus
    // Fall B. Kein Produktivcode geändert.
    // ------------------------------------------------------------------
    out << "\n== G · Wirkung der beiden vorhandenen scrollTo-Zeilen im Fall B ==\n";
    list->setCurrentIndex(noteRow(list, 9));
    list->verticalScrollBar()->setValue(noteRow(list, 9).row());
    qApp->processEvents();
    QTest::keyClick(list, Qt::Key_Up);
    qApp->processEvents();
    const int beforeFix = list->verticalScrollBar()->value();
    out << "Ausgangslage nach dem Tastendruck: Rollwert=" << beforeFix << "  Kopf "
        << rectOf(list, gestern) << "  Auswahl " << rectOf(list, noteRow(list, 8)) << "\n";

    // Die Passbedingung des heutigen Codes, hier von Hand ausgerechnet.
    const QRect heading = list->visualRect(gestern);
    const QRect selected = list->visualRect(noteRow(list, 8));
    out << "Passbedingung (Auswahl.unten − Kopf.oben ≤ Viewport): " << selected.bottom() << " − "
        << heading.top() << " = " << (selected.bottom() - heading.top()) << " ≤ "
        << list->viewport()->height() << " → "
        << ((selected.bottom() - heading.top() <= list->viewport()->height()) ? "erfüllt" : "verletzt")
        << "\n";

    list->scrollTo(gestern, QAbstractItemView::EnsureVisible);
    const int afterHead = list->verticalScrollBar()->value();
    list->scrollTo(noteRow(list, 8), QAbstractItemView::EnsureVisible);
    const int afterSelection = list->verticalScrollBar()->value();
    qApp->processEvents();
    out << "nach scrollTo(Kopf):     Rollwert=" << afterHead << " (Δ " << (afterHead - beforeFix)
        << ")\n";
    out << "nach scrollTo(Auswahl):  Rollwert=" << afterSelection << " (Δ "
        << (afterSelection - afterHead) << " — rollt die zweite Zeile zurück?)\n";
    out << "Ergebnis: Kopf " << rectOf(list, gestern) << "  ganz im Bild? "
        << (wholeInView(list, gestern) ? "JA" : "nein") << "\n";
    out << "          Auswahl " << rectOf(list, noteRow(list, 8)) << "  ganz im Bild? "
        << (wholeInView(list, noteRow(list, 8)) ? "ja" : "nein") << "\n";
    out << "Bewegung gegenüber heute: " << (afterSelection - beforeFix) << " Zeile(n) = "
        << list->visualRect(gestern).height() << " px\n";

    // ------------------------------------------------------------------
    // H — Verbotszone: Braucht der Bauweg einen Sichtbarkeits-Schwellwert?
    // Gemessen wird, ob scrollTo(EnsureVisible) von sich aus nichts tut,
    // wenn die Zeile schon ganz im Bild steht.
    // ------------------------------------------------------------------
    out << "\n== H · Tut scrollTo(EnsureVisible) nichts, wenn die Zeile schon im Bild steht? ==\n";
    list->verticalScrollBar()->setValue(gestern.row());
    qApp->processEvents();
    out << "Kopf " << rectOf(list, gestern) << "  ganz im Bild? "
        << (wholeInView(list, gestern) ? "ja" : "nein") << "  Rollwert="
        << list->verticalScrollBar()->value() << "\n";
    const int beforeNoop = list->verticalScrollBar()->value();
    list->scrollTo(gestern, QAbstractItemView::EnsureVisible);
    qApp->processEvents();
    out << "nach scrollTo(Kopf, EnsureVisible): Rollwert=" << list->verticalScrollBar()->value()
        << " (Δ " << (list->verticalScrollBar()->value() - beforeNoop) << ")\n";
    // Und einmal mit halb angeschnittenem Kopf: dort MUSS es rollen.
    list->verticalScrollBar()->setValue(gestern.row() + 1);
    qApp->processEvents();
    const int beforeCut = list->verticalScrollBar()->value();
    out << "Kopf angeschnitten: " << rectOf(list, gestern) << "  Rollwert=" << beforeCut << "\n";
    list->scrollTo(gestern, QAbstractItemView::EnsureVisible);
    qApp->processEvents();
    out << "nach scrollTo: Rollwert=" << list->verticalScrollBar()->value() << " (Δ "
        << (list->verticalScrollBar()->value() - beforeCut) << ")  Kopf " << rectOf(list, gestern)
        << "\n";

    // ------------------------------------------------------------------
    // I — Zweites Fenster mit kleiner Mittelgruppe: der Prüfsatz aus Sprint 3
    // in seiner echten Bauart (Grenzübertritt aufwärts auf eine ganz
    // sichtbare Zeile, Kopf und Auswahl passen zusammen ins Bild).
    // ------------------------------------------------------------------
    out << "\n== I · Prüfsatz Sprint 3 in seiner echten Bauart (kleine Mittelgruppe) ==\n";
    QTemporaryDir tmp2;
    Store store2(tmp2.filePath(QStringLiteral("kopfsonde2.db")));
    if (!store2.open()) {
        out << "Store 2 öffnet nicht\n";
        return 1;
    }
    {
        auto add = [&store2](const QString &content, const QString &iso) {
            Note note;
            note.content = content;
            note.createdAt = at(iso);
            store2.addNote(note);
        };
        for (int hour = 15; hour >= 8; --hour) {
            add(QStringLiteral("von heute, %1 Uhr").arg(hour),
                QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 11; hour >= 9; --hour) {
            add(QStringLiteral("von gestern, %1 Uhr").arg(hour),
                QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 15; hour >= 8; --hour) {
            add(QStringLiteral("von letzter Woche, %1 Uhr").arg(hour),
                QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
    }
    LibraryWindow window2(&store2);
    window2.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window2.resize(900, 600);
    window2.showLibrary();
    QTest::qWaitForWindowExposed(&window2);
    QListView *list2 = listOf(window2);
    const QModelIndex gestern2 = headOf(list2, noteRow(list2, 8));

    list2->setCurrentIndex(noteRow(list2, 11));
    list2->verticalScrollBar()->setValue(noteRow(list2, 8).row());
    qApp->processEvents();
    out << "vorher:  Rollwert=" << list2->verticalScrollBar()->value() << "  Kopf "
        << rectOf(list2, gestern2) << "  ganz im Bild? "
        << (wholeInView(list2, gestern2) ? "ja" : "nein") << "\n";
    out << "         Ziel (Notiz 10) " << rectOf(list2, noteRow(list2, 10)) << "  ganz im Bild? "
        << (wholeInView(list2, noteRow(list2, 10)) ? "ja" : "nein") << "\n";
    const int before2 = list2->verticalScrollBar()->value();
    QTest::keyClick(list2, Qt::Key_Up);
    qApp->processEvents();
    out << "nachher: Rollwert=" << list2->verticalScrollBar()->value() << " (Δ "
        << (list2->verticalScrollBar()->value() - before2) << ")  Kopf " << rectOf(list2, gestern2)
        << "  ganz im Bild? " << (wholeInView(list2, gestern2) ? "JA — Prüfsatz hält" : "nein")
        << "\n";

    // Und in demselben Fenster der Befund B: aufwärts auf Notiz 8, die erste
    // der kleinen Gruppe „Gestern".
    out << "\nBefund B im selben Fenster (aufwärts auf Notiz 8, erste der Gruppe):\n";
    list2->setCurrentIndex(noteRow(list2, 9));
    list2->verticalScrollBar()->setValue(noteRow(list2, 9).row());
    qApp->processEvents();
    const int before3 = list2->verticalScrollBar()->value();
    out << "vorher:  Rollwert=" << before3 << "  Kopf " << rectOf(list2, gestern2)
        << "  ganz im Bild? " << (wholeInView(list2, gestern2) ? "ja" : "nein") << "\n";
    QTest::keyClick(list2, Qt::Key_Up);
    qApp->processEvents();
    out << "nachher: Rollwert=" << list2->verticalScrollBar()->value() << " (Δ "
        << (list2->verticalScrollBar()->value() - before3) << ")  Kopf " << rectOf(list2, gestern2)
        << "  ganz im Bild? " << (wholeInView(list2, gestern2) ? "ja" : "NEIN — derselbe Befund")
        << "\n";

    // ------------------------------------------------------------------
    // J — Flachstes mögliche Fenster: passen Kopf und erste Notiz zusammen
    // ins Bild? Davon hängt ab, ob AK 1 ohne Bedingung gilt.
    // ------------------------------------------------------------------
    out << "\n== J · Flachstes Fenster: trägt die Passbedingung für die erste Notiz? ==\n";
    LibraryWindow flat(&store);
    flat.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    flat.resize(900, 150);
    flat.showLibrary();
    QTest::qWaitForWindowExposed(&flat);
    QListView *flatList = listOf(flat);
    const QModelIndex flatHead = headOf(flatList, noteRow(flatList, 8));
    flatList->setCurrentIndex(noteRow(flatList, 9));
    flatList->verticalScrollBar()->setValue(noteRow(flatList, 9).row());
    qApp->processEvents();
    QTest::keyClick(flatList, Qt::Key_Up);
    qApp->processEvents();
    const QRect fh = flatList->visualRect(flatHead);
    const QRect fs = flatList->visualRect(noteRow(flatList, 8));
    out << "Fensterhöhe angefordert 150, geworden " << flat.height() << "  Viewport "
        << flatList->viewport()->height() << " px\n";
    out << "Kopf " << rectOf(flatList, flatHead) << "  Auswahl "
        << rectOf(flatList, noteRow(flatList, 8)) << "\n";
    out << "Passbedingung: " << fs.bottom() << " − " << fh.top() << " = " << (fs.bottom() - fh.top())
        << " ≤ " << flatList->viewport()->height() << " → "
        << ((fs.bottom() - fh.top() <= flatList->viewport()->height()) ? "erfüllt — AK 1 gilt ohne Zusatzbedingung"
                                                                      : "VERLETZT — AK 1 braucht eine Bedingung")
        << "\n";

    // ------------------------------------------------------------------
    // K — Szene 7 des Bildläufers `libraryshots` nachgestellt: derselbe
    // Aufbau, dieselben Eingaben. Der Kommentar dort behauptet, der Kopf
    // komme mit. Gemessen wird, ob er es tut.
    // ------------------------------------------------------------------
    out << "\n== K · Szene 7 von libraryshots (tests/libraryshots.cpp:278–306) nachgestellt ==\n";
    QTemporaryDir tmp3;
    Store store3(tmp3.filePath(QStringLiteral("kopfsonde3.db")));
    if (!store3.open()) {
        out << "Store 3 öffnet nicht\n";
        return 1;
    }
    {
        auto add = [&store3](const QString &content, const QString &iso) {
            Note note;
            note.content = content;
            note.createdAt = at(iso);
            store3.addNote(note);
        };
        // Der Läufer legt sie aufsteigend an; die Liste zeigt die neueste zuerst.
        for (int hour = 8; hour < 16; ++hour) {
            add(QStringLiteral("Notiz von dieser Woche, %1 Uhr").arg(hour),
                QStringLiteral("2026-07-29T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
        for (int hour = 8; hour < 16; ++hour) {
            add(QStringLiteral("Notiz von letzter Woche, %1 Uhr").arg(hour),
                QStringLiteral("2026-07-23T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
        }
    }
    LibraryWindow window3(&store3);
    // friday() des Läufers: 31.07.2026 ist ein Freitag.
    window3.setReferenceTime(at(QStringLiteral("2026-07-31T16:00:00")));
    window3.resize(900, 600);
    window3.showLibrary();
    QTest::qWaitForWindowExposed(&window3);
    QListView *list3 = listOf(window3);
    NoteListModel *model3 = modelOf(list3);
    out << "Zeilen: " << model3->rowCount() << "  Notizen: " << model3->noteCount() << "\n";
    list3->setCurrentIndex(model3->index(model3->rowCount() - 1));
    qApp->processEvents();
    for (int step = 0; step < 7; ++step) {
        QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        QCoreApplication::sendEvent(list3, &up);
    }
    qApp->processEvents();
    const QModelIndex sel3 = list3->currentIndex();
    const QModelIndex head3 = headOf(list3, sel3);
    out << "Auswahl steht auf Zeile " << sel3.row() << ": \""
        << sel3.data(Qt::DisplayRole).toString().left(34) << "\"\n";
    out << "Ihr Kopf ist Zeile " << head3.row() << " (\"" << head3.data(Qt::DisplayRole).toString()
        << "\"), unmittelbar darüber? " << (head3.row() == sel3.row() - 1 ? "ja" : "nein") << "\n";
    out << "Kopf " << rectOf(list3, head3) << "  ganz im Bild? "
        << (wholeInView(list3, head3) ? "ja" : "NEIN") << "\n";
    out << "Der Kommentar der Szene behauptet: \"its head comes along and the entry stays whole\".\n";
    out << "Gemessen: Kopf kommt " << (wholeInView(list3, head3) ? "mit" : "NICHT mit") << ".\n";

    return 0;
}
