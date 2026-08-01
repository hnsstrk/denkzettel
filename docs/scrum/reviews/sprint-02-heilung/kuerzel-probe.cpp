// Was liefert KGlobalAccel::globalShortcut(component, action) wirklich?
#include <KGlobalAccel>

#include <QAction>
#include <QGuiApplication>
#include <QKeySequence>
#include <cstdio>

static void show(const char *label, const QString &component, const QString &action)
{
    const QList<QKeySequence> keys = KGlobalAccel::self()->globalShortcut(component, action);
    printf("%-28s %s / %s -> %lld Eintraege", label, qPrintable(component), qPrintable(action),
           (long long)keys.size());
    for (const QKeySequence &k : keys) {
        printf("  [%s]", qPrintable(k.toString()));
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    app.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));

    show("fremd, bekannt", QStringLiteral("org.kde.spectacle.desktop"), QStringLiteral("RecordScreen"));
    show("fremd, kwin", QStringLiteral("kwin"), QStringLiteral("Window Close"));
    show("denkzettel, vor Register", QStringLiteral("org.denkzettel.Denkzettel.desktop"),
         QStringLiteral("show_capture"));

    QAction action(QStringLiteral("Capture"));
    action.setObjectName(QStringLiteral("show_capture"));
    action.setProperty("componentName", QStringLiteral("org.denkzettel.Denkzettel.desktop"));
    action.setProperty("componentDisplayName", QStringLiteral("Denkzettel"));
    const bool ok = KGlobalAccel::setGlobalShortcut(&action, QKeySequence(Qt::META | Qt::Key_N));
    printf("setGlobalShortcut -> %s\n", ok ? "true" : "false");

    show("denkzettel, nach Register", QStringLiteral("org.denkzettel.Denkzettel.desktop"),
         QStringLiteral("show_capture"));
    printf("shortcut(action) -> %lld Eintraege\n",
           (long long)KGlobalAccel::self()->shortcut(&action).size());
    return 0;
}
