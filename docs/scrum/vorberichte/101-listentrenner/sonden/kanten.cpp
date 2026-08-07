// Messsonde zur Vorprüfung von Issue #101 (Trennlinien in der Bibliothek).
//
// Sie beantwortet die Fragen, die der PO der Vorprüfung vorgelegt hat, und
// rät bei keiner:
//
//   1. Kommt ein `QStyledItemDelegate` dieser Liste an die Auswahl der
//      NACHBARZEILE heran? Gemessen wird, ob `option.widget` gesetzt ist, ob
//      es sich auf `QAbstractItemView` casten lässt und ob dessen
//      `selectionModel()` die Nachbarzeile beantwortet.
//   2. Zeichnet die Ansicht die Nachbarzeile neu, wenn die Auswahl umspringt?
//      Davon hängt ab, ob eine Linie, die von der Nachbarauswahl abhängt,
//      stehenbleibt, wo sie verschwinden müsste. Gezählt wird je Zeile, wie
//      oft `paint()` gerufen wird.
//   3. Liegen die Zeilenrechtecke lückenlos aneinander? Nur dann sind „letzte
//      Bildpunktzeile der oberen Notiz" und „oberste Bildpunktzeile des
//      Kopfes" benachbarte Bildpunktzeilen.
//   4. Was macht die Skalierung des Kunden aus einer Haarlinie? Ausgegeben
//      werden `devicePixelRatio` des gegriffenen Bildes und die Bildpunkt-
//      breite, die eine 1-px-Linie darin belegt.
//
// Kein Projektcode wird geändert: die Sonde linkt gegen die im Bauplatz
// gebaute `libdenkzettelui.a`.
//
// Aufruf: kanten <Zielverzeichnis|->

#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QHash>
#include <QImage>
#include <QItemSelectionModel>
#include <QListView>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <cstdio>

namespace
{
void settle(int milliseconds)
{
    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < milliseconds) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
}

void addNote(Store &store, const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = QDateTime::fromString(isoDateTime, Qt::ISODate);
    if (!store.addNote(note).has_value()) {
        qFatal("Notiz ließ sich nicht speichern");
    }
}

/**
 * Zählt die Aufrufe von `paint()` je Zeile und schreibt beim ersten Aufruf
 * auf, was der Delegate über sich und seine Nachbarn erfahren kann.
 *
 * Sie erbt vom Produktions-Delegate und ruft ihn, damit Zeilenhöhen und
 * Malwege dieselben bleiben — gemessen wird die Ansicht, nicht ein Nachbau.
 */
class Zaehler : public NoteListDelegate
{
public:
    using NoteListDelegate::NoteListDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        ++counts[index.row()];

        if (!probed) {
            probed = true;
            widgetSet = option.widget != nullptr;
            widgetClass = option.widget ? QString::fromLatin1(option.widget->metaObject()->className())
                                        : QStringLiteral("(null)");
            const auto *view = qobject_cast<const QAbstractItemView *>(option.widget);
            viewCast = view != nullptr;
            selectionReachable = view && view->selectionModel() != nullptr;
            if (selectionReachable) {
                // Die eigentliche Frage: Weiß der Delegate, ob die Zeile UNTER
                // ihm ausgewählt ist? Ohne sie ist AK 3 nicht zu bauen.
                const QModelIndex below = index.sibling(index.row() + 1, index.column());
                neighbourAnswered = true;
                neighbourSelected = below.isValid() && view->selectionModel()->isSelected(below);
            }
            parentIsView = qobject_cast<const QAbstractItemView *>(parent()) != nullptr;
        }

        NoteListDelegate::paint(painter, option, index);
    }

    mutable QHash<int, int> counts;
    mutable bool probed = false;
    mutable bool widgetSet = false;
    mutable QString widgetClass;
    mutable bool viewCast = false;
    mutable bool selectionReachable = false;
    mutable bool neighbourAnswered = false;
    mutable bool neighbourSelected = false;
    mutable bool parentIsView = false;
};
}

int main(int argc, char **argv)
{
    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString directory =
        argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("-");

    Store store(sandbox.filePath(QStringLiteral("probe.db")));
    if (!store.open()) {
        printf("Store konnte nicht geöffnet werden: %s\n", qPrintable(store.lastError()));
        return 1;
    }
    // Drei Gruppen, die mittlere mit drei Notizen: nur so gibt es zugleich
    // eine Gruppengrenze, eine Notizgrenze und eine letzte Notiz je Gruppe.
    addNote(store, QStringLiteral("Backup prüfen\nprune-Policy, monatliche Snapshots behalten"),
            QStringLiteral("2026-07-31T14:20:00"));
    addNote(store, QStringLiteral("Zahnarzt anrufen\nTermin für September, am besten vormittags"),
            QStringLiteral("2026-07-30T21:40:00"));
    addNote(store, QStringLiteral("Mara anrufen\nWochenende, Kuchen nicht vergessen"),
            QStringLiteral("2026-07-30T18:10:00"));
    addNote(store, QStringLiteral("Idee: Denkzettel-Export\nMarkdown mit Frontmatter, ein Ordner je Monat"),
            QStringLiteral("2026-07-30T09:00:00"));
    addNote(store, QStringLiteral("Kategorien-Prompt\nBeispiele mitgeben, sonst rät das Modell"),
            QStringLiteral("2026-07-23T11:30:00"));

    LibraryWindow window(&store);
    window.setReferenceTime(QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate));
    window.resize(900, 600);
    window.showLibrary();
    settle(300);

    auto *list = window.findChild<QListView *>();
    if (!list) {
        printf("Keine Liste gefunden\n");
        return 1;
    }

    auto *zaehler = new Zaehler(list);
    list->setItemDelegate(zaehler);
    list->viewport()->update();
    settle(300);

    printf("== 1. Was der Delegate sehen kann ==\n");
    printf("paint() überhaupt gerufen           : %s\n", zaehler->probed ? "ja" : "NEIN");
    printf("option.widget gesetzt               : %s (%s)\n",
           zaehler->widgetSet ? "ja" : "nein", qPrintable(zaehler->widgetClass));
    printf("cast auf QAbstractItemView          : %s\n", zaehler->viewCast ? "ja" : "nein");
    printf("selectionModel() erreichbar         : %s\n", zaehler->selectionReachable ? "ja" : "nein");
    printf("Auswahl der Nachbarzeile beantwortet: %s\n", zaehler->neighbourAnswered ? "ja" : "nein");
    printf("QObject-Elter ist die Ansicht       : %s\n", zaehler->parentIsView ? "ja" : "nein");

    auto *model = qobject_cast<NoteListModel *>(list->model());
    if (!model) {
        printf("Kein NoteListModel an der Ansicht\n");
        return 1;
    }

    printf("\n== 2. Zeilen der Liste ==\n");
    printf("%-5s %-6s %-8s %-8s %-7s %s\n", "Zeile", "Art", "y", "unten", "Höhe", "Nachbar unten");
    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex index = model->index(row);
        const QRect rect = list->visualRect(index);
        const bool head = index.data(NoteListModel::GroupHeaderRole).toBool();
        const QModelIndex below = model->index(row + 1);
        const QString next = row + 1 < model->rowCount()
            ? (below.data(NoteListModel::GroupHeaderRole).toBool() ? QStringLiteral("Kopf")
                                                                  : QStringLiteral("Notiz"))
            : QStringLiteral("(Ende)");
        printf("%-5d %-6s %-8d %-8d %-7d %s\n", row, head ? "Kopf" : "Notiz", rect.y(),
               rect.bottom(), rect.height(), qPrintable(next));
    }

    printf("\nLücken zwischen den Zeilenrechtecken (spacing=%d):\n", list->spacing());
    bool contiguous = true;
    for (int row = 1; row < model->rowCount(); ++row) {
        const int gap = list->visualRect(model->index(row)).y()
            - list->visualRect(model->index(row - 1)).bottom() - 1;
        if (gap != 0) {
            contiguous = false;
            printf("  Zeile %d → %d: %d px Lücke\n", row - 1, row, gap);
        }
    }
    printf("  %s\n", contiguous ? "keine — die Rechtecke stoßen lückenlos aneinander"
                                : "Lücken vorhanden (siehe oben)");

    printf("\nSeitliche Lage: Viewport x=%d, Breite=%d, Textkante=%d (also %d px vom linken Rand)\n",
           list->viewport()->rect().x(), list->viewport()->width(),
           NoteListDelegate::textLeft(list->visualRect(model->index(1))),
           NoteListDelegate::textLeft(list->visualRect(model->index(1)))
               - list->viewport()->rect().x());

    printf("\n== 3. Wer wird neu gezeichnet, wenn die Auswahl umspringt? ==\n");
    printf("Entscheidend sind die OBEREN NACHBARN von alter und neuer Auswahl: an ihrer\n"
           "Unterkante kommt beziehungsweise geht die Linie. Werden sie nicht neu\n"
           "gezeichnet, bleibt eine Linie stehen, die verschwinden müsste.\n\n");

    // Vier Sprünge, darunter zwei, bei denen ein oberer Nachbar AUSSERHALB der
    // Strecke zwischen alter und neuer Auswahl liegt — nur die entscheiden.
    const QList<QPair<int, int>> jumps = {{1, 3}, {3, 5}, {5, 3}, {3, 7}, {4, 5}, {4, 7}, {5, 4}, {5, 3}};
    for (const auto &jump : jumps) {
        const int from = jump.first;
        const int to = jump.second;
        list->setCurrentIndex(model->index(from));
        settle(200);

        zaehler->counts.clear();
        list->setCurrentIndex(model->index(to));
        settle(200);

        QList<int> painted = zaehler->counts.keys();
        std::sort(painted.begin(), painted.end());
        QStringList names;
        for (int row : painted) {
            names.append(QString::number(row));
        }
        const int spanLow = qMin(from, to);
        const int spanHigh = qMax(from, to);
        const bool aboveNewOutside = to - 1 < spanLow || to - 1 > spanHigh;
        const bool aboveOldOutside = from - 1 < spanLow || from - 1 > spanHigh;
        printf("Sprung %d → %d: neu gezeichnet [%s]\n", from, to,
               qPrintable(names.join(QStringLiteral(", "))));
        printf("   oberer Nachbar der neuen Auswahl (Zeile %d, %s der Strecke): %s\n", to - 1,
               aboveNewOutside ? "außerhalb" : "innerhalb",
               zaehler->counts.contains(to - 1) ? "gezeichnet" : "NICHT gezeichnet");
        printf("   oberer Nachbar der alten Auswahl (Zeile %d, %s der Strecke): %s\n", from - 1,
               aboveOldOutside ? "außerhalb" : "innerhalb",
               zaehler->counts.contains(from - 1) ? "gezeichnet" : "NICHT gezeichnet");
    }

    // Und die Gegenprobe, von der abhängt, WIE sich das nachweisen lässt:
    // Zeichnet ein `grab()` alles neu? Dann zeigt ein gegriffenes Standbild
    // eine stehengebliebene Linie gerade NICHT.
    list->setCurrentIndex(model->index(4));
    settle(200);
    zaehler->counts.clear();
    list->setCurrentIndex(model->index(5));
    settle(200);
    const int nachSprung = zaehler->counts.size();
    window.grab();
    printf("\nGegenprobe zur Belegform: nach dem Sprung 4 → 5 waren %d Zeilen neu gezeichnet,\n"
           "nach einem zusätzlichen window.grab() sind es %d von %d.\n",
           nachSprung, zaehler->counts.size(), model->rowCount());
    printf("→ Ein gegriffenes Standbild %s die stehengebliebene Linie.\n",
           zaehler->counts.size() >= model->rowCount() ? "VERDECKT" : "zeigt");

    printf("\n== 4. Wie breit ist „die volle Breite der Liste“? ==\n");
    printf("AK 2 verlangt die Gruppenlinie „über die volle Breite, einschließlich x = 0 und\n"
           "x = Breite−1“. Sobald ein Rollbalken steht, sind Listenbreite und Zeilenbreite\n"
           "nicht mehr dieselbe Zahl — und in einem gegriffenen Fensterbild ist beides\n"
           "wieder etwas anderes.\n");
    auto breiten = [&](const char *lage) {
        printf("%-28s Liste %3d · Viewport %3d · Zeile %3d · Rollbalken %s\n", lage, list->width(),
               list->viewport()->width(), list->visualRect(model->index(1)).width(),
               list->verticalScrollBar()->isVisible() ? "sichtbar" : "aus");
    };
    breiten("Fenster 900×600:");
    window.resize(900, 220);
    settle(200);
    breiten("Fenster 900×220:");
    window.resize(900, 600);
    settle(200);

    printf("\n== 5. Skalierung ==\n");
    const QImage picture = window.grab().toImage();
    printf("QT_SCALE_FACTOR=%s, devicePixelRatio des Bildes: %.2f\n",
           qgetenv("QT_SCALE_FACTOR").isEmpty() ? "(nicht gesetzt)" : qgetenv("QT_SCALE_FACTOR").constData(),
           picture.devicePixelRatio());
    printf("Bild %d×%d Bildpunkte bei %d×%d logischen Punkten Fenster\n", picture.width(),
           picture.height(), window.width(), window.height());
    printf("Eine Linie von 1 logischem Punkt belegt im Bild %.2f Bildpunktzeilen.\n",
           picture.devicePixelRatio());

    if (directory != QLatin1String("-")) {
        const QString scale = qgetenv("QT_SCALE_FACTOR").isEmpty()
            ? QStringLiteral("1")
            : QString::fromLocal8Bit(qgetenv("QT_SCALE_FACTOR"));
        window.grab().save(QStringLiteral("%1/kanten-vorher-%2.png").arg(directory, scale));
    }

    return 0;
}
