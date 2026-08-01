// Gegenproben des UI/UX-Reviews zur Sprint-2-Heilung. Kein Projekt-Code,
// keine Quelldatei wird geändert: beide Proben greifen zur Laufzeit in den
// Widget-Baum, wie es die Retro-Gegenprobe vom 01.08.2026 vorgemacht hat.
//
// Probe 1: Trägt der Leerzustand sein Symbol, wenn die Anwendung — anders als
//          mein Helfer — ein Fenstersymbol gesetzt hat (main.cpp:29)?
// Probe 2: Wie hoch ist das Meldungsband mit und ohne Wortumbruch? Der
//          Wireframe zeichnet ein einzeiliges Band; die Umsetzung setzt
//          setWordWrap(true) (librarywindow.cpp:153).

#include "store/store.h"
#include "ui/librarywindow.h"

#include <KMessageWidget>

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QIcon>
#include <QLayout>
#include <QListView>
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
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Aufruf: ux-probe-s5 <Zielverzeichnis> <Pfad zu sc-apps-denkzettel.svg>\n");
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString directory = QString::fromLocal8Bit(argv[1]);

    // Wie main.cpp:29 — nur aus der Datei statt aus der Qt-Ressource, die mein
    // Helfer nicht mitlinkt.
    app.setWindowIcon(QIcon(QString::fromLocal8Bit(argv[2])));

    Store store(sandbox.filePath(QStringLiteral("probe.db")));
    if (!store.open()) {
        printf("Store konnte nicht geöffnet werden: %s\n", qPrintable(store.lastError()));
        return 1;
    }

    // Probe 1 — Leerzustand mit gesetztem Fenstersymbol.
    {
        LibraryWindow window(&store);
        window.resize(900, 600);
        window.showLibrary();
        settle(300);
        printf("Probe 1: Fenstersymbol gesetzt=%d\n", int(!app.windowIcon().isNull()));
        window.grab().save(QStringLiteral("%1/ux-leer-1-mit-symbol-900x600.png").arg(directory));
    }

    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.content = QStringLiteral("Idee für Denkzettel — Bündel-Export erst vorschlagen");
    store.addNote(note);
    Note zweite;
    zweite.createdAt = QDateTime::currentDateTime().addSecs(-3600);
    zweite.content = QStringLiteral("journalctl -u whisperd --since today");
    store.addNote(zweite);

    // Probe 2 — Meldungsband, Ist und Gegenprobe ohne Wortumbruch.
    {
        LibraryWindow window(&store);
        window.resize(900, 600);
        window.showLibrary();
        settle(300);

        auto *list = window.findChild<QListView *>();
        list->setCurrentIndex(list->model()->index(0, 0));
        for (QAction *action : window.actions()) {
            if (action->text() == QStringLiteral("Löschen")) {
                action->trigger();
            }
        }
        settle(900);

        auto *message = window.findChild<KMessageWidget *>();
        printf("Probe 2: Meldungsband mit Wortumbruch : h=%d px (%.0f %% von 600)\n",
               message->height(), 100.0 * message->height() / window.height());

        message->setWordWrap(false);
        window.layout()->activate();
        settle(300);
        printf("Probe 2: Meldungsband ohne Wortumbruch: h=%d px (%.0f %% von 600)\n",
               message->height(), 100.0 * message->height() / window.height());
        printf("Probe 2: Mindestbreite des Fensters ohne Wortumbruch: %d px\n",
               window.minimumSizeHint().width());
        window.grab().save(QStringLiteral("%1/ux-meldung-ohne-umbruch-900x600.png").arg(directory));
    }

    return 0;
}
