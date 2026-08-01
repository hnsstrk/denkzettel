// Selbst-Sichtprüfung des Entwicklers zur Heilung von Befund 2 (Issue #7,
// Beschluss B1/B3). Kein Projekt-Code: linkt nur gegen die gebaute Bibliothek
// denkzettelui und rendert das echte LibraryWindow offscreen.
//
// Bauen und laufen lassen (aus dem Wurzelverzeichnis des Arbeitsbaums):
//   g++ -std=c++20 -fPIC $(pkg-config --cflags Qt6Widgets) -Isrc \
//       docs/scrum/reviews/sprint-02-heilung/shot.cpp \
//       build/lib/libdenkzettelui.a build/lib/libdenkzettelstore.a \
//       $(pkg-config --libs Qt6Widgets Qt6Sql) \
//       -lKF6ConfigCore -lKF6ConfigGui -lKF6I18n -lKF6WidgetsAddons \
//       -lKF6WindowSystem -o build/bin/heilungshot
//   QT_QPA_PLATFORM=offscreen build/bin/heilungshot \
//       docs/scrum/reviews/sprint-02-heilung -style breeze

#include "store/store.h"
#include "ui/librarywindow.h"

#include <KMessageWidget>

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QIcon>
#include <QLineEdit>
#include <QListView>
#include <QSplitter>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <cstdio>
#include <memory>

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

void report(const char *label, LibraryWindow &window)
{
    auto *search = window.findChild<QLineEdit *>();
    auto *splitter = window.findChild<QSplitter *>();
    QWidget *header = search ? search->parentWidget()->parentWidget() : nullptr;

    printf("%s — Fenster %dx%d\n", label, window.width(), window.height());
    if (header) {
        printf("  Kopfzeile: y=%d h=%d (sizeHint %d)\n",
               header->mapTo(&window, QPoint()).y(), header->height(), header->sizeHint().height());
    }
    if (search) {
        printf("  Suchfeld : y=%d h=%d\n", search->mapTo(&window, QPoint()).y(), search->height());
    }
    if (splitter) {
        printf("  Splitter : y=%d h=%d -> %.0f %% der Fensterhöhe\n",
               splitter->mapTo(&window, QPoint()).y(), splitter->height(),
               100.0 * splitter->height() / window.height());
    }
}

void shoot(LibraryWindow &window, const QString &directory, const QString &name)
{
    const QString path = QStringLiteral("%1/%2.png").arg(directory, name);
    if (!window.grab().save(path)) {
        printf("  Bild %s konnte nicht geschrieben werden\n", qPrintable(path));
    }
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Aufruf: heilungshot <Zielverzeichnis> [-style breeze]\n");
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString directory = QString::fromLocal8Bit(argv[1]);

    // Ohne Fenstersymbol bliebe der Leerzustand ohne sein Symbol — placeholder-
    // Page() liest qApp->windowIcon(), und die Ressource steckt nur im Daemon
    // (UI-Review vom 01.08.2026, B6). Der Pfad ergibt sich aus dem Zielordner:
    // docs/scrum/reviews/<Ordner> liegt vier Ebenen unter der Wurzel.
    QDir root(directory);
    for (int up = 0; up < 4; ++up) {
        root.cdUp();
    }
    QApplication::setWindowIcon(QIcon(root.filePath(QStringLiteral("icons/sc-apps-denkzettel.svg"))));

    Store store(sandbox.filePath(QStringLiteral("heilung.db")));
    if (!store.open()) {
        printf("Store konnte nicht geöffnet werden: %s\n", qPrintable(store.lastError()));
        return 1;
    }

    // Der Leerzustand zuerst — der leere Store ist ohne weiteres Zutun da.
    {
        LibraryWindow window(&store);
        window.resize(900, 600);
        window.showLibrary();
        settle(300);
        report("Leerzustand 900x600", window);
        shoot(window, directory, QStringLiteral("heilung-leerzustand-900x600"));
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
        store.addNote(note);
    }

    for (const QSize &size : {QSize(900, 600), QSize(1200, 800)}) {
        LibraryWindow window(&store);
        window.resize(size);
        window.showLibrary();
        settle(300);

        // Wireframe 2c zeichnet zwei Leerzustaende; der zweite ist genau dieser
        // Moment — Notizen da, keine ausgewaehlt (UI-Review 01.08.2026, B7).
        if (size == QSize(900, 600)) {
            report("Ohne Auswahl 900x600", window);
            shoot(window, directory, QStringLiteral("heilung-ohne-auswahl-900x600"));
        }

        auto *list = window.findChild<QListView *>();
        list->setCurrentIndex(list->model()->index(0, 0));
        settle(100);

        const QString name = QStringLiteral("heilung-%1x%2").arg(size.width()).arg(size.height());
        report(qPrintable(QStringLiteral("Normalfall %1x%2").arg(size.width()).arg(size.height())), window);
        shoot(window, directory, name);
    }

    // Meldungszustand: Löschen startet die Frist, die Meldung fährt aus.
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
        settle(800);

        report("Meldungszustand 900x600", window);
        if (auto *message = window.findChild<KMessageWidget *>()) {
            printf("  Meldung  : y=%d h=%d sichtbar=%d\n",
                   message->mapTo(&window, QPoint()).y(), message->height(), int(message->isVisible()));
        }
        shoot(window, directory, QStringLiteral("heilung-meldung-900x600"));
    }

    return 0;
}
