// Messung 4 — Der unsichtbare Teil von #68, am echten NoteListDelegate.
//
// Claim under test: after a font change the list is only half updated. The
// group head keeps its size for good, the note entry grows by its two text
// lines alone — the timestamp line inside it contributes the same pixels as
// before. Both follow from notelistdelegate.cpp:32, where the small font comes
// from QFontDatabase::systemFont() rather than from the view.
//
// The relayout is measured as well, so that it is not mistaken for the cure:
// doItemsLayout() changes nothing. The stale font is the cause, not the layout.
//
// Measured against the real delegate, not a rebuilt copy of its formula: a test
// setup in which the fault cannot occur is no test.

#include "store/note.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDateTime>
#include <QListView>
#include <QStyleOptionViewItem>
#include <QTextStream>

namespace
{
constexpr int ListWidth = 300;

QTextStream out(stdout);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    out << "Messung 4 — Zeilenhöhen der Bibliotheksliste nach einer Schriftänderung\n";
    out << "======================================================================\n\n";

    NoteListModel model;
    Note note;
    note.id = 1;
    note.createdAt = QDateTime::currentDateTime();
    note.content = QStringLiteral("Ein Gedanke über Straßenbahnen");
    model.setNotes({note}, QDateTime::currentDateTime());

    QListView view;
    auto *delegate = new NoteListDelegate(&view);
    view.setItemDelegate(delegate);
    view.setModel(&model);
    view.resize(ListWidth, 400);
    view.show();
    QCoreApplication::processEvents();

    // Row 0 is the group head the model puts in front of the note, row 1 the
    // note itself.
    const QModelIndex head = model.index(0);
    const QModelIndex entry = model.index(1);

    auto measure = [&](const QString &tag) {
        QStyleOptionViewItem option;
        option.rect = QRect(0, 0, ListWidth, 0);
        option.font = view.font();

        out << tag << "  Schrift der Liste=" << view.font().pointSizeF() << " pt"
            << "  Gruppenkopf=" << delegate->sizeHint(option, head).height() << " px"
            << "  Eintrag=" << delegate->sizeHint(option, entry).height() << " px"
            << "  gezeichnet=" << view.visualRect(entry).height() << " px\n";
        out.flush();
    };

    measure(QStringLiteral("Start                "));

    QFont bigger = app.font();
    bigger.setPointSizeF(app.font().pointSizeF() * 2);
    app.setFont(bigger);
    QCoreApplication::processEvents();
    measure(QStringLiteral("Anwendungsschrift 2x "));

    view.doItemsLayout();
    QCoreApplication::processEvents();
    measure(QStringLiteral("nach doItemsLayout   "));

    out << "\nBefund: Der Gruppenkopf steht still — er wird ganz aus groupHeadFont()\n"
           "gerechnet, also aus systemFont(). Der Eintrag wächst nur um seine beiden\n"
           "Textzeilen; die Zeitstempelzeile darin steuert vorher wie nachher dieselben\n"
           "Pixel bei. Eine Bibliothek nach Schriftwechsel zeigt damit alte\n"
           "Überschriften über gewachsenen Notizen. Das Relayout heilt es nicht.\n";

    out.flush();
    return 0;
}
