// Screenshot-Helfer gegen das ECHTE LibraryWindow (Sprint-2-Stand).
// Kein Projekt-Code: liegt im Scratchpad, linkt nur gegen die gebaute
// Bibliothek denkzettelui. Zweck: nachweisen, dass ein Review mit
// offscreen-Screenshot den Layoutfehler gesehen haette.

#include "store/store.h"
#include "ui/librarywindow.h"

#include <QApplication>
#include <QLineEdit>
#include <QSplitter>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QBoxLayout>
#include <QWidget>

#include <cstdio>
#include <memory>

int main(int argc, char **argv)
{
    QTemporaryDir dataDir;
    qputenv("XDG_DATA_HOME", dataDir.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", dataDir.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);

    auto store = std::make_unique<Store>(dataDir.filePath(QStringLiteral("probe.db")));
    if (!store->open()) {
        printf("Store konnte nicht geoeffnet werden: %s\n", qPrintable(store->lastError()));
        return 1;
    }

    const QStringList texts{
        QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten — als Cronjob auf dem NAS einrichten"),
        QStringLiteral("Idee für Denkzettel — Bündel-Export erst vorschlagen, wenn mindestens fünf Notizen zum selben Thema da sind"),
        QStringLiteral("journalctl -u whisperd --since today"),
    };
    for (int i = 0; i < texts.size(); ++i) {
        Note note;
        note.createdAt = QDateTime::currentDateTime().addSecs(-3600 * (i + 1));
        note.content = texts.at(i);
        store->addNote(note);
    }

    LibraryWindow window(store.get());
    window.showLibrary();
    app.processEvents();

    auto *splitter = window.findChild<QSplitter *>();
    auto *search = window.findChild<QLineEdit *>();
    QWidget *header = search ? search->parentWidget()->parentWidget() : nullptr;

    printf("Fenster            : %dx%d\n", window.width(), window.height());
    if (header) {
        printf("Kopfzeile          : y=%d h=%d\n", header->y(), header->height());
    }
    if (search) {
        const QPoint p = search->mapTo(&window, QPoint(0, 0));
        printf("Suchfeld           : y=%d h=%d  -> Leerflaeche darueber: %d px\n", p.y(), search->height(), p.y());
    }
    if (splitter) {
        printf("Splitter (Liste/Detail): y=%d h=%d  -> beginnt bei %.0f %% der Fensterhoehe\n",
               splitter->y(), splitter->height(), 100.0 * splitter->y() / window.height());
        printf("Splitter-SizePolicy vertikal: %d  (5=Preferred, 7=Expanding)\n",
               int(splitter->sizePolicy().verticalPolicy()));
    }

    window.grab().save(QStringLiteral("%1/echt-ist.png").arg(QString::fromLatin1(argv[1])));

    // Gegenprobe am laufenden Widget-Baum — der Quellcode bleibt unangetastet:
    // entspricht layout->addWidget(m_splitter, 1) in librarywindow.cpp:162.
    if (auto *box = qobject_cast<QBoxLayout *>(window.layout()); box && splitter) {
        box->setStretchFactor(splitter, 1);
        window.layout()->activate();
        app.processEvents();

        printf("\n-- nach setStretchFactor(splitter, 1) --\n");
        if (header) {
            printf("Kopfzeile          : y=%d h=%d\n", header->y(), header->height());
        }
        if (search) {
            const QPoint p = search->mapTo(&window, QPoint(0, 0));
            printf("Suchfeld           : y=%d h=%d  -> Leerflaeche darueber: %d px\n", p.y(), search->height(), p.y());
        }
        printf("Splitter (Liste/Detail): y=%d h=%d  -> beginnt bei %.0f %% der Fensterhoehe\n",
               splitter->y(), splitter->height(), 100.0 * splitter->y() / window.height());
        window.grab().save(QStringLiteral("%1/echt-soll.png").arg(QString::fromLatin1(argv[1])));
    }
    return 0;
}
