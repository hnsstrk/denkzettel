// Misst die Geometrie des ECHTEN LibraryWindow (gegen libdenkzettelui.a),
// nicht eines Nachbaus. Kein Produktivcode, laeuft nur im Scratchpad.

#include "store/store.h"
#include "ui/librarywindow.h"

#include <KMessageWidget>

#include <QApplication>
#include <QLineEdit>
#include <QListView>
#include <QSplitter>
#include <QTemporaryDir>
#include <QTextStream>

static void dump(const QString &name, QWidget *w, QWidget *root)
{
    QTextStream out(stdout);
    if (!w) {
        out << name << ": (nicht gefunden)\n";
        return;
    }
    const QPoint p = w->mapTo(root, QPoint(0, 0));
    out << name << ": y=" << p.y() << " h=" << w->height()
        << "  vPolicy=" << int(w->sizePolicy().verticalPolicy()) << "\n";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("layoutprobe-dev"));

    QTemporaryDir dir;
    Store store(dir.path() + QStringLiteral("/probe.db"));
    if (!store.open()) {
        QTextStream(stdout) << "Store-Fehler: " << store.lastError() << "\n";
        return 1;
    }
    for (int i = 0; i < 5; ++i) {
        Note note;
        note.createdAt = QDateTime::currentDateTime().addSecs(-60 * i);
        note.content = QStringLiteral("Notiz %1").arg(i);
        store.addNote(note);
    }

    LibraryWindow window(&store);
    window.resize(900, 600);
    window.show();
    app.processEvents();
    window.resize(900, 600);
    app.processEvents();

    QTextStream out(stdout);
    out << "Fenster: " << window.width() << "x" << window.height() << "\n";
    auto *search = window.findChild<QLineEdit *>();
    dump(QStringLiteral("Suchfeld "), search, &window);
    dump(QStringLiteral("Kopfzeile"), search ? search->parentWidget()->parentWidget() : nullptr, &window);
    dump(QStringLiteral("Meldung  "), window.findChild<KMessageWidget *>(), &window);
    dump(QStringLiteral("Splitter "), window.findChild<QSplitter *>(), &window);
    dump(QStringLiteral("Liste    "), window.findChild<QListView *>(), &window);

    if (argc > 1) {
        window.grab().save(QStringLiteral("%1/echt.png").arg(QString::fromLatin1(argv[1])));
    }
    return 0;
}
