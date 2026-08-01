// Eigene Bildprüfung zur Zugabe des UI-Reviews: die Trefferliste der Suche
// (S6/#8) mit der Gliederung aus #46 und der Leerzustand „Keine Treffer".
// Kein Projekt-Code: linkt nur gegen die gebaute Bibliothek denkzettelui und
// rendert das echte LibraryWindow offscreen (DoD 3, Retro-Beschluss B3).
//
// Zwei Fragen stehen hinter den Bildern:
//   * Wirkt die Gliederung in einer gefilterten Liste noch, wenn Gruppen
//     dazwischen ausfallen (Treffer nur in „Heute" und „Älter")?
//   * Trägt sie noch, wenn fast jede Gruppe nur einen Treffer beisteuert und
//     die Liste damit halb aus Köpfen besteht?
//
// Bauen und laufen lassen wie ux-shot-s5a.cpp; Bezugszeitpunkt fest auf
// Fr 31.07.2026 16:20, damit die Bilder kalenderunabhängig sind.

#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QListView>
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
    const QString path = QStringLiteral("%1/s6-%2.png").arg(directory, name);
    if (!window.grab().save(path)) {
        out() << "  Bild " << path << " konnte nicht geschrieben werden\n";
    }
}

/** Was die Liste gerade zeigt — Köpfe und Notizen in ihrer Reihenfolge. */
void reportRows(QListView *list, const QString &label)
{
    out() << "\n[" << label << "]\n";

    int heads = 0;
    int notes = 0;
    for (int row = 0; row < list->model()->rowCount(); ++row) {
        const QModelIndex index = list->model()->index(row, 0);
        const bool head = index.data(NoteListModel::GroupHeaderRole).toBool();
        head ? ++heads : ++notes;
        out() << QStringLiteral("  %1 %2 %3\n")
                     .arg(head ? QStringLiteral("KOPF ") : QStringLiteral("Notiz"))
                     .arg(index.data(NoteListModel::TimestampRole).toString(), -14)
                     .arg(index.data(Qt::DisplayRole).toString().left(48));
    }
    out() << QStringLiteral("  → %1 Köpfe, %2 Notizen, Liste sichtbar=%3\n")
                 .arg(heads)
                 .arg(notes)
                 .arg(list->isVisible() ? QStringLiteral("ja") : QStringLiteral("nein"));
}

qint64 add(Store &store, const QString &iso, const QString &content)
{
    Note note;
    note.createdAt = at(iso);
    note.content = content;
    return store.addNote(note).value_or(-1);
}

/**
 * Bestand über alle fünf Gruppen. „Backup" trifft nur ganz oben und ganz unten
 * (Lücke in der Mitte), „Vortrag" trifft in jeder Gruppe genau einmal.
 */
void fill(Store &store)
{
    add(store, QStringLiteral("2026-07-31T14:32:00"),
        QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten — als Cronjob auf dem NAS"));
    add(store, QStringLiteral("2026-07-31T11:05:00"),
        QStringLiteral("Für den Vortrag nächste Woche noch einmal durchgehen, ob die Zahlen zur Auslastung stimmen"));
    add(store, QStringLiteral("2026-07-30T21:48:00"), QStringLiteral("journalctl -u whisperd --since today"));
    add(store, QStringLiteral("2026-07-30T09:15:00"),
        QStringLiteral("Vortrag: Folien auf zwölf kürzen, die Tabelle fliegt raus"));
    add(store, QStringLiteral("2026-07-28T09:12:00"),
        QStringLiteral("Mara wegen Wochenende anrufen, Kuchen nicht vergessen"));
    add(store, QStringLiteral("2026-07-28T08:00:00"),
        QStringLiteral("Vortrag beim Verein — Termin bestätigen, Beamer mitbringen"));
    add(store, QStringLiteral("2026-07-23T17:30:00"),
        QStringLiteral("Kategorien-Prompt: Beispiele mitgeben, sonst rät das Modell"));
    add(store, QStringLiteral("2026-07-23T10:00:00"),
        QStringLiteral("Vortrag im Juli — Rückmeldung der Teilnehmer einholen"));
    add(store, QStringLiteral("2026-07-12T08:05:00"),
        QStringLiteral("Backup-Platte bei Kraus abholen, vor dem Vortrag im August"));
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        out() << "Aufruf: ux-shot-s6 <Zielverzeichnis> [-style breeze]\n";
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString directory = QString::fromLocal8Bit(argv[1]);

    Store store(sandbox.filePath(QStringLiteral("suche.db")));
    if (!store.open()) {
        out() << "Store: " << store.lastError() << "\n";
        return 1;
    }
    fill(store);

    LibraryWindow window(&store);
    window.setReferenceTime(at(QStringLiteral("2026-07-31T16:20:00")));
    window.resize(900, 600);
    window.showLibrary();
    settle(300);

    auto *search = window.findChild<QLineEdit *>();
    auto *list = window.findChild<QListView *>();
    if (!search || !list) {
        out() << "Suchfeld oder Liste nicht gefunden\n";
        return 1;
    }

    out() << "Suchfeld: aktiv=" << int(search->isEnabled()) << " Löschknopf="
          << int(search->isClearButtonEnabled()) << " Platzhalter=\"" << search->placeholderText()
          << "\" Tooltip der Umhüllung=\"" << search->parentWidget()->toolTip() << "\"\n";

    reportRows(list, QStringLiteral("E0 volle Liste, kein Suchbegriff"));
    shoot(window, directory, QStringLiteral("e0-volle-liste-900x600"));

    // E1 — Treffer nur in der ersten und der letzten Gruppe: die Gliederung
    // bekommt eine Lücke.
    search->setText(QStringLiteral("backup"));
    settle(300);
    reportRows(list, QStringLiteral("E1 Suche \"backup\" — Lücke zwischen den Gruppen"));
    shoot(window, directory, QStringLiteral("e1-treffer-mit-luecke-900x600"));

    // E2 — ein Treffer je Gruppe: die halbe Liste besteht aus Köpfen.
    search->setText(QStringLiteral("vortrag"));
    settle(300);
    reportRows(list, QStringLiteral("E2 Suche \"vortrag\" — ein Treffer je Gruppe"));
    shoot(window, directory, QStringLiteral("e2-ein-treffer-je-gruppe-900x600"));

    // E3 — Auswahl in der Trefferliste: trägt der Detailbereich mit?
    list->setCurrentIndex(list->model()->index(1, 0));
    settle(200);
    shoot(window, directory, QStringLiteral("e3-auswahl-in-der-trefferliste-900x600"));

    // E4 — keine Treffer.
    search->setText(QStringLiteral("zylinderkopfdichtung"));
    settle(300);
    reportRows(list, QStringLiteral("E4 Suche ohne Treffer"));
    shoot(window, directory, QStringLiteral("e4-keine-treffer-900x600"));

    // E5 — zwei Zeichen: der trigram-Index kann sie nicht enthalten, der
    // Teilstring-Vergleich aus SPEC 6 muss sie trotzdem finden.
    search->setText(QStringLiteral("ra"));
    settle(300);
    reportRows(list, QStringLiteral("E5 Suchbegriff mit zwei Zeichen (\"ra\")"));
    shoot(window, directory, QStringLiteral("e5-zwei-zeichen-900x600"));

    // E6 — Operator aus SPEC 6, den erst S7 parst.
    search->setText(QStringLiteral("tag:backup"));
    settle(300);
    reportRows(list, QStringLiteral("E6 Operator tag:backup (S7 steht aus)"));
    shoot(window, directory, QStringLiteral("e6-operator-vor-s7-900x600"));

    // E7 — Feld geleert: der Weg zurück zur vollen Liste.
    search->clear();
    settle(300);
    reportRows(list, QStringLiteral("E7 Feld geleert"));
    shoot(window, directory, QStringLiteral("e7-feld-geleert-900x600"));

    out().flush();
    return 0;
}
