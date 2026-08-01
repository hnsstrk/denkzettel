// Gegenprobe zum Kundensatz „Wenn ich ein anderes Theme verwende muss es sich
// anpassen": Der Daemon haelt das Capture-Fenster dauerhaft vorgehalten
// (SPEC 2.1), es wird also genau einmal gebaut. Wechselt der Nutzer danach das
// Farbschema, bekommt die Anwendung eine neue Palette — die Frage ist, welche
// Teile des Fensters ihr folgen.
//
// Aufruf: capture-themeswitch <Zielverzeichnis> <Schema beim Bau> <Schema danach>

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KColorScheme>
#include <KSharedConfig>

#include <QApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleFactory>
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

void applyScheme(QApplication &app, const QString &path)
{
    KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    app.setPalette(KColorScheme::createApplicationPalette(scheme));
}

void report(const QString &phase, QWidget *window)
{
    const auto labels = window->findChildren<QLabel *>();
    auto *text = window->findChild<QPlainTextEdit *>();

    printf("%-8s Fensterhintergrund   = %s\n",
           qPrintable(phase),
           qPrintable(window->palette().color(QPalette::Active, QPalette::Window).name()));
    printf("%-8s Textfeld-Hintergrund = %s   Textfarbe = %s\n",
           qPrintable(phase),
           qPrintable(text->palette().color(QPalette::Active, QPalette::Base).name()),
           qPrintable(text->palette().color(QPalette::Active, QPalette::Text).name()));
    for (QLabel *label : labels) {
        printf("%-8s Kleintext \"%s\" = %s\n",
               qPrintable(phase),
               qPrintable(label->text()),
               qPrintable(label->palette().color(QPalette::Active, QPalette::WindowText).name()));
    }
}
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("Aufruf: capture-themeswitch <Zielverzeichnis> <Schema beim Bau> <Schema danach>\n");
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create(QStringLiteral("breeze")));
    const QString directory = QString::fromLocal8Bit(argv[1]);

    Store store(sandbox.filePath(QStringLiteral("probe.db")));
    if (!store.open()) {
        printf("Store konnte nicht geöffnet werden: %s\n", qPrintable(store.lastError()));
        return 1;
    }

    // Wie der Dienststart: Schema steht, Fenster wird einmal gebaut.
    applyScheme(app, QString::fromLocal8Bit(argv[2]));
    CaptureWindow window(&store);
    window.show();
    settle(300);
    report(QStringLiteral("vorher"), &window);
    window.grab().save(QStringLiteral("%1/wechsel-1-vorher.png").arg(directory));

    // Wie ein Themewechsel in den Systemeinstellungen: die Anwendung bekommt
    // eine neue Palette, das Fenster lebt weiter.
    printf("\n--- Farbschema gewechselt, Fenster bleibt bestehen ---\n\n");
    applyScheme(app, QString::fromLocal8Bit(argv[3]));
    settle(300);
    report(QStringLiteral("nachher"), &window);
    window.grab().save(QStringLiteral("%1/wechsel-2-nachher.png").arg(directory));

    // Gegenprobe: ein frisch gebautes Fenster unter demselben neuen Schema —
    // so saehe es aus, wenn der Dienst nach dem Wechsel neu startete.
    CaptureWindow neu(&store);
    neu.show();
    settle(300);
    printf("\n");
    report(QStringLiteral("neu"), &neu);
    neu.grab().save(QStringLiteral("%1/wechsel-3-neu-gebaut.png").arg(directory));

    return 0;
}
