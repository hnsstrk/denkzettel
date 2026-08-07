/*
 * Sonde zur Vorprüfung von #101, Messung B (07.08.2026).
 *
 * Prüffrage 1: Erreicht `option.widget` den Delegate einer QListView? Davon
 *   hängt ab, ob der Delegate überhaupt an das `selectionModel` herankommt —
 *   die Kopplung, die AK 3 („keine Linie an den Kanten der ausgewählten
 *   Zeile") verlangt.
 *
 * Prüffrage 2: Welche Zeilen malt die Ansicht neu, wenn die Auswahl von einer
 *   Zeile zur nächsten wandert? Eine Linie an der **Unterkante** einer Zeile,
 *   die von der Auswahl der **Nachbarzeile** abhängt, steht und fällt damit:
 *   Wird die Zeile über der neu ausgewählten nicht neu gemalt, bleibt ihre
 *   Linie stehen, obwohl sie fort sein müsste.
 *
 * Prüffrage 3: Deckt das Rechteck, das der Delegate bekommt, die volle Breite
 *   des Sichtfeldes ab — auch dann, wenn ein Rollbalken steht?
 *
 * Nachbau statt Produktivcode: Diese Sonde kennt weder NoteListModel noch
 * NoteListDelegate. Gemessen wird das Verhalten von QListView, und das hängt
 * an keiner der beiden Klassen.
 */

#include <QApplication>
#include <QListView>
#include <QPainter>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTest>
#include <QTextStream>

#include <map>

namespace
{
std::map<int, int> g_paints;
bool g_widgetSeen = false;
bool g_widgetMissing = false;
std::map<int, QRect> g_rects;

class CountingDelegate : public QStyledItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        g_paints[index.row()] += 1;
        g_rects[index.row()] = option.rect;
        if (option.widget != nullptr) {
            g_widgetSeen = true;
        } else {
            g_widgetMissing = true;
        }
        QStyledItemDelegate::paint(painter, option, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(index)
        return QSize(option.rect.width(), 60);
    }
};

QString countLine()
{
    QString text;
    for (const auto &[row, count] : g_paints) {
        text += QStringLiteral("Zeile %1: %2×  ").arg(row).arg(count);
    }
    return text.isEmpty() ? QStringLiteral("(keine)") : text;
}

void reset()
{
    g_paints.clear();
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    QStandardItemModel model;
    for (int row = 0; row < 12; ++row) {
        model.appendRow(new QStandardItem(QStringLiteral("Zeile %1").arg(row)));
    }

    QListView list;
    CountingDelegate delegate;
    list.setModel(&model);
    list.setItemDelegate(&delegate);
    list.setSelectionMode(QAbstractItemView::SingleSelection);
    list.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list.setFrameShape(QFrame::NoFrame);
    list.resize(300, 240); // kleiner als 12 × 60 px: der Rollbalken steht
    list.show();
    QTest::qWait(200);

    out << "## Prüffrage 1 — option.widget im Delegate\n";
    out << (g_widgetSeen ? "gesetzt: ja\n" : "gesetzt: nein\n");
    out << (g_widgetMissing ? "mindestens einmal nullptr: ja\n" : "mindestens einmal nullptr: nein\n");
    out << QStringLiteral("qobject_cast auf QAbstractItemView: %1\n\n")
               .arg(g_widgetSeen ? QStringLiteral("prüfbar, siehe unten") : QStringLiteral("entfällt"));

    out << "## Prüffrage 3 — Rechteck des Delegate gegenüber dem Sichtfeld\n";
    out << QStringLiteral("Breite der Ansicht: %1, Breite des Sichtfeldes: %2\n")
               .arg(list.width())
               .arg(list.viewport()->width());
    out << QStringLiteral("Rollbalken sichtbar: %1\n")
               .arg(list.verticalScrollBar()->isVisible() ? QStringLiteral("ja") : QStringLiteral("nein"));
    if (g_rects.count(0) > 0) {
        out << QStringLiteral("Rechteck der Zeile 0: x=%1 Breite=%2 (rechte Kante %3)\n\n")
                   .arg(g_rects[0].x())
                   .arg(g_rects[0].width())
                   .arg(g_rects[0].right());
    }

    // Für Prüffrage 2 wird die Liste hoch genug gemacht, dass jede geprüfte
    // Zeile sichtbar ist: Ein Rollvorgang malt das ganze Sichtfeld neu und
    // verdeckte damit genau das, was gemessen werden soll.
    list.resize(300, 800);
    QTest::qWait(200);

    out << "## Prüffrage 2 — Neumalung bei Auswahlwechsel\n";
    out << QStringLiteral("Sichtfeld %1 px hoch, Zeilen 60 px — es wird nicht gerollt.\n\n")
               .arg(list.viewport()->height());

    // Jeder Schritt beginnt bei `from` und geht nach `to`. Zwei Zeilen sind
    // dabei die interessanten:
    //   - die Zeile über der **neuen** Auswahl: Ihre Linie muss verschwinden.
    //   - die Zeile über der **alten** Auswahl: Ihre Linie muss zurückkommen.
    // Wird eine der beiden nicht neu gemalt, bleibt das Bild falsch stehen.
    const auto step = [&](int from, int to) {
        list.setCurrentIndex(model.index(from, 0));
        QTest::qWait(150);
        reset();
        list.setCurrentIndex(model.index(to, 0));
        QTest::qWait(150);

        const bool aboveNew = to > 0 && g_paints.count(to - 1) > 0;
        const bool aboveOld = from > 0 && g_paints.count(from - 1) > 0;
        out << QStringLiteral("Auswahl %1 → %2: gemalt [%3]\n").arg(from).arg(to).arg(countLine());
        out << QStringLiteral("    Zeile über der neuen Auswahl (%1) neu gemalt: %2\n")
                   .arg(to - 1)
                   .arg(aboveNew ? QStringLiteral("ja") : QStringLiteral("NEIN"));
        out << QStringLiteral("    Zeile über der alten Auswahl (%1) neu gemalt: %2\n")
                   .arg(from - 1)
                   .arg(aboveOld ? QStringLiteral("ja") : QStringLiteral("NEIN"));
    };

    step(2, 3); // ein Schritt abwärts
    step(3, 2); // ein Schritt aufwärts
    step(1, 5); // weiter Sprung abwärts
    step(5, 1); // weiter Sprung aufwärts

    // Prüffrage 4 entscheidet über das Prüfmittel: Kann ein Bildbeleg einen
    // stehengebliebenen Strich überhaupt zeigen? `grab()` ist der Weg, den
    // alle fünf Bildläufer dieses Projekts nehmen.
    out << "\n## Prüffrage 4 — was grab() malt\n";
    list.setCurrentIndex(model.index(5, 0));
    QTest::qWait(150);
    reset();
    list.setCurrentIndex(model.index(1, 0));
    QTest::qWait(150);
    const QString afterSelection = countLine();
    reset();
    const QPixmap picture = list.grab();
    out << "nach dem Auswahlwechsel 5 → 1 gemalt: " << afterSelection << "\n";
    out << "beim anschließenden grab() gemalt:   " << countLine() << "\n";
    out << QStringLiteral("Bildgröße: %1×%2\n").arg(picture.width()).arg(picture.height());

    out.flush();
    return 0;
}
