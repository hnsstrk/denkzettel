// Eigene Bildprüfung des UI/UX-Reviews zur Sprint-2-Heilung (Issue #7).
// Kein Projekt-Code: linkt nur gegen die gebaute Bibliothek denkzettelui und
// rendert das echte LibraryWindow offscreen (DoD 3, Retro-Beschluss B3).
//
// Gegenüber dem Helfer des Entwicklers zusätzlich: Leerzustand 2 aus
// Wireframe 2c (Notizen vorhanden, keine ausgewählt) und die Messung der
// Splitter-Aufteilung Liste/Detail.
//
// Bauen und laufen lassen:
//   g++ -std=c++20 -fPIC $(pkg-config --cflags Qt6Widgets) \
//       -I<worktree>/src ux-shot-s5.cpp \
//       build-ux/lib/libdenkzettelui.a build-ux/lib/libdenkzettelstore.a \
//       $(pkg-config --libs Qt6Widgets Qt6Sql) \
//       -lKF6ConfigCore -lKF6ConfigGui -lKF6I18n -lKF6WidgetsAddons \
//       -lKF6WindowSystem -o ux-shot-s5
//   QT_QPA_PLATFORM=offscreen ./ux-shot-s5 <Zielverzeichnis> -style breeze

#include "store/store.h"
#include "ui/librarywindow.h"

#include <KMessageWidget>

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QListView>
#include <QSplitter>
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

/** Misst die Bereiche, die Wireframe 2b über die Raumaufteilung zeichnet. */
void report(const char *label, LibraryWindow &window)
{
    auto *search = window.findChild<QLineEdit *>();
    auto *splitter = window.findChild<QSplitter *>();
    auto *message = window.findChild<KMessageWidget *>();
    QWidget *header = search ? search->parentWidget()->parentWidget() : nullptr;

    printf("%s — Fenster %dx%d\n", label, window.width(), window.height());
    if (header) {
        printf("  Kopfzeile: y=%d h=%d (sizeHint %d)\n",
               header->mapTo(&window, QPoint()).y(), header->height(), header->sizeHint().height());
    }
    if (search) {
        printf("  Suchfeld : y=%d h=%d aktiv=%d Tooltip=\"%s\"\n",
               search->mapTo(&window, QPoint()).y(), search->height(), int(search->isEnabled()),
               qPrintable(search->parentWidget()->toolTip()));
    }
    if (message && message->isVisible()) {
        printf("  Meldung  : y=%d h=%d Text=\"%s\" Schliessknopf=%d\n",
               message->mapTo(&window, QPoint()).y(), message->height(),
               qPrintable(message->text()), int(message->isCloseButtonVisible()));
    }
    if (splitter) {
        printf("  Splitter : y=%d h=%d -> %.0f %% der Fensterhöhe, Rest unten %d px\n",
               splitter->mapTo(&window, QPoint()).y(), splitter->height(),
               100.0 * splitter->height() / window.height(),
               window.height() - (splitter->mapTo(&window, QPoint()).y() + splitter->height()));
        printf("  Aufteilung: Liste %d px / Detail %d px (Minimum Liste %d px)\n",
               splitter->widget(0)->width(), splitter->widget(1)->width(),
               splitter->widget(0)->minimumWidth());
    }
}

void shoot(LibraryWindow &window, const QString &directory, const QString &name)
{
    const QString path = QStringLiteral("%1/ux-%2.png").arg(directory, name);
    if (!window.grab().save(path)) {
        printf("  Bild %s konnte nicht geschrieben werden\n", qPrintable(path));
    }
}

void fill(Store &store)
{
    const QStringList texts{
        QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten — als Cronjob auf dem NAS einrichten"),
        QStringLiteral("Idee für Denkzettel — Bündel-Export erst vorschlagen, wenn mindestens fünf Notizen zum selben Thema da sind, sonst wird der Vault zugemüllt"),
        QStringLiteral("journalctl -u whisperd --since today"),
        QStringLiteral("Mara wegen Wochenende anrufen, Kuchen nicht vergessen"),
    };
    for (int i = 0; i < texts.size(); ++i) {
        Note note;
        note.createdAt = QDateTime::currentDateTime().addSecs(-3600 * (i + 1));
        note.content = texts.at(i);
        store.addNote(note);
    }
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Aufruf: ux-shot-s5 <Zielverzeichnis> [-style breeze]\n");
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString directory = QString::fromLocal8Bit(argv[1]);

    Store store(sandbox.filePath(QStringLiteral("ux.db")));
    if (!store.open()) {
        printf("Store konnte nicht geöffnet werden: %s\n", qPrintable(store.lastError()));
        return 1;
    }

    // Wireframe 2c, Leerzustand 1 — noch keine Notiz vorhanden.
    {
        LibraryWindow window(&store);
        window.resize(900, 600);
        window.showLibrary();
        settle(300);
        report("Leerzustand 1 (leere Bibliothek) 900x600", window);
        shoot(window, directory, QStringLiteral("leer-1-900x600"));
    }

    fill(store);

    // Wireframe 2b — Normalfall in zwei Fenstergrößen, mit Auswahl.
    for (const QSize &size : {QSize(900, 600), QSize(1200, 800)}) {
        LibraryWindow window(&store);
        window.resize(size);
        window.showLibrary();
        settle(300);

        auto *list = window.findChild<QListView *>();
        list->setCurrentIndex(list->model()->index(0, 0));
        settle(150);

        report(qPrintable(QStringLiteral("Normalfall %1x%2").arg(size.width()).arg(size.height())), window);
        shoot(window, directory, QStringLiteral("normal-%1x%2").arg(size.width()).arg(size.height()));
    }

    // Wireframe 2c, Leerzustand 2 — Notizen vorhanden, keine ausgewählt.
    {
        LibraryWindow window(&store);
        window.resize(900, 600);
        window.showLibrary();
        settle(300);
        report("Leerzustand 2 (nichts ausgewählt) 900x600", window);
        shoot(window, directory, QStringLiteral("leer-2-900x600"));
    }

    // Wireframe 2b/2c — Meldungszustand: Löschen startet die Frist.
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

        report("Meldungszustand 900x600", window);
        shoot(window, directory, QStringLiteral("meldung-900x600"));
    }

    return 0;
}
