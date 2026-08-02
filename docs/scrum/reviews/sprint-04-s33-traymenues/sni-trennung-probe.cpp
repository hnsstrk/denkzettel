// Messung zum ersten Schritt von Issue #60 (S33): Trägt ein eigenes
// Linksklick-Menü unter Plasma/Wayland?
//
// Getrennte Menüs heißen ItemIsMenu=false plus eigenes QMenu im
// activateRequested-Handler. Das Rechtsklick-Menü zeichnet dann weiterhin
// plasmashell (DBusMenu), das Linksklick-Menü zeichnen wir selbst — und unter
// Wayland darf ein Klient seine Fensterlage nicht bestimmen. Ob das Menü
// trotzdem am Symbol aufgeht, ist die Frage dieser Probe.
//
// Bauen:
//   g++ -std=c++20 -fPIC sni-trennung-probe.cpp -o <ziel> \
//       $(pkg-config --cflags --libs Qt6Widgets) \
//       -I/usr/include/KF6/KStatusNotifierItem -lKF6StatusNotifierItem
//
// Aufrufen:  sni-trennung-probe [popup|exec|fenster]
//   popup   — QMenu::popup(pos), der übliche Weg eines Tray-Menüs
//   exec    — QMenu::exec(pos), derselbe Weg mit eigener Ereignisschleife
//   fenster — das Menü als gewöhnliches Fenster an die Stelle gesetzt;
//             die Gegenprobe, ob überhaupt irgendein selbst gezeichnetes
//             Fenster dort landet, wo es hin soll
//
// Messen (Probe läuft, Dienstname aus dem StatusNotifierWatcher):
//   qdbus6 <dienst> /StatusNotifierItem org.kde.StatusNotifierItem.Activate X Y
//
// Die Probe meldet jede Aktivierung samt übergebener Position nach stdout und
// sagt, wohin Qt das Menü zu setzen glaubt. Wo es wirklich steht, entscheidet
// das Bild: Unter Wayland kennt ein Klient seine eigene Fensterlage nicht, die
// Zahl aus QWidget::geometry() ist deshalb kein Beleg.

#include <KStatusNotifierItem>

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QIcon>
#include <QMenu>
#include <QMetaEnum>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#include <cstdio>

namespace
{
QElapsedTimer g_clock;
}

namespace
{
/** Menu entry for a feature that is not implemented yet. */
void addStub(QMenu *menu, const QString &text, const QString &iconName)
{
    QAction *action = menu->addAction(QIcon::fromTheme(iconName), text);
    action->setEnabled(false);
}

/** The left click menu of the story, so the picture shows the real thing. */
QMenu *buildWorkMenu()
{
    auto *menu = new QMenu();
    menu->addAction(QIcon::fromTheme(QStringLiteral("document-edit")),
                    QStringLiteral("Notiz erfassen\tMeta+N"));
    addStub(menu, QStringLiteral("Sprachnotiz aufnehmen"), QStringLiteral("audio-input-microphone"));
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme(QStringLiteral("view-list-text")), QStringLiteral("Bibliothek öffnen"));
    addStub(menu, QStringLiteral("Jetzt analysieren"), QStringLiteral("system-run"));
    addStub(menu, QStringLiteral("Vorschläge"), QStringLiteral("tools-wizard"));
    return menu;
}

void report(const char *label, QMenu *menu)
{
    const QRect geometry = menu->geometry();
    const QRect frame = menu->frameGeometry();
    printf("  [%5lld ms] %-12s geometry=(%d,%d) %dx%d · frame=(%d,%d) %dx%d · sizeHint=%dx%d · sichtbar=%d\n",
           static_cast<long long>(g_clock.elapsed()), label, geometry.x(), geometry.y(), geometry.width(),
           geometry.height(), frame.x(), frame.y(), frame.width(), frame.height(), menu->sizeHint().width(),
           menu->sizeHint().height(), menu->isVisible());
    if (QWindow *window = menu->windowHandle()) {
        const QRect windowGeometry = window->geometry();
        printf("  %-12s Fenster: (%d,%d) %dx%d · sichtbar=%d · Typ=0x%x · Elternfenster=%p · Bildschirm=%s\n",
               label, windowGeometry.x(), windowGeometry.y(), windowGeometry.width(),
               windowGeometry.height(), window->isVisible(),
               static_cast<unsigned>(window->flags() & Qt::WindowType_Mask),
               static_cast<void *>(window->transientParent()),
               window->screen() ? qPrintable(window->screen()->name()) : "(keiner)");
    } else {
        printf("  %-12s Fenster: keins\n", label);
    }
    fflush(stdout);
}

/** Logs which event ends the menu — Qt's own hiding or the compositor's. */
class EventLog : public QObject
{
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::Hide:
        case QEvent::Close:
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::PlatformSurface:
        case QEvent::Expose: {
            const char *name = QMetaEnum::fromType<QEvent::Type>().valueToKey(event->type());
            printf("  [%5lld ms] Ereignis auf %s: %s\n", static_cast<long long>(g_clock.elapsed()),
                   watched->metaObject()->className(), name ? name : "?");
            fflush(stdout);
            break;
        }
        default:
            break;
        }
        return QObject::eventFilter(watched, event);
    }
};
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("denkzettel-sni-probe"));
    app.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));
    app.setQuitOnLastWindowClosed(false);

    const QString variant = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("popup");

    auto *item = new KStatusNotifierItem(QStringLiteral("denkzettel-sni-probe"), &app);
    item->setCategory(KStatusNotifierItem::ApplicationStatus);
    item->setStatus(KStatusNotifierItem::Active);
    item->setIconByName(QStringLiteral("edit-entry"));
    item->setTitle(QStringLiteral("Probe #60"));
    item->setStandardActionsEnabled(false);

    // Right click: drawn by plasmashell over DBusMenu, as today.
    auto *adminMenu = new QMenu();
    adminMenu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), QStringLiteral("Beenden"),
                         &app, &QApplication::quit);
    item->setContextMenu(adminMenu);

    // False is what splits the clicks: the host then calls Activate on the left
    // click instead of showing the exported menu.
    item->setIsMenu(false);

    QMenu *workMenu = buildWorkMenu();
    if (variant == QStringLiteral("fenster")) {
        workMenu->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    }
    workMenu->installEventFilter(new EventLog(&app));

    QObject::connect(item, &KStatusNotifierItem::activateRequested, &app,
                     [workMenu, variant](bool active, const QPoint &pos) {
                         g_clock.restart();
                         printf("activateRequested: active=%d pos=(%d,%d) Variante=%s\n", active, pos.x(),
                                pos.y(), qPrintable(variant));
                         report("vorher", workMenu);

                         if (variant == QStringLiteral("fenster")) {
                             workMenu->resize(workMenu->sizeHint());
                             workMenu->move(pos);
                             workMenu->show();
                         } else if (variant == QStringLiteral("exec")) {
                             // exec() spins its own loop, so the report has to
                             // come from a timer that fires while it runs.
                             QTimer::singleShot(400, workMenu, [workMenu] {
                                 report("waehrend", workMenu);
                             });
                             workMenu->exec(pos);
                             report("nachher", workMenu);
                             return;
                         } else {
                             workMenu->popup(pos);
                         }

                         report("sofort", workMenu);
                         for (int delay : {50, 100, 200, 400, 1000, 2000, 4000}) {
                             QTimer::singleShot(delay, workMenu, [workMenu] {
                                 report("spaeter", workMenu);
                             });
                         }
                     });

    QObject::connect(item, &KStatusNotifierItem::secondaryActivateRequested, &app, [](const QPoint &pos) {
        printf("secondaryActivateRequested: pos=(%d,%d)\n", pos.x(), pos.y());
        fflush(stdout);
    });

    printf("Probe laeuft. ItemIsMenu=%d · Variante=%s · Eintraege=%lld\n", item->isMenu(), qPrintable(variant),
           static_cast<long long>(workMenu->actions().size()));
    fflush(stdout);

    return app.exec();
}
