// Probe zum Kundenbefund vom 01.08.2026 („Capture passt nicht zum KDE-Theme").
// Kein Projekt-Code: der Helfer linkt gegen denkzettelcapture und zeichnet das
// Fenster unter mehreren Farbschemata, um die Frage zu trennen — geht die
// Farbe mit, und was bleibt beim Themewechsel stehen?
//
// Aufruf: capture-theme-probe <Zielverzeichnis> <Name>=<Pfad zur .colors> …

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KColorScheme>
#include <KSharedConfig>

#include <QApplication>
#include <QElapsedTimer>
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

QString hex(const QPalette &palette, QPalette::ColorRole role)
{
    return palette.color(QPalette::Active, role).name();
}
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Aufruf: capture-theme-probe <Zielverzeichnis> <Name>=<Pfad zur .colors> …\n");
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

    printf("Stil: %s\n\n", qPrintable(app.style()->objectName()));

    for (int i = 2; i < argc; ++i) {
        const QString argument = QString::fromLocal8Bit(argv[i]);
        const QString name = argument.section(QLatin1Char('='), 0, 0);
        const QString path = argument.section(QLatin1Char('='), 1);

        KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
        app.setPalette(KColorScheme::createApplicationPalette(scheme));

        // Frisches Fenster je Schema: subtleLabel() friert die Palette beim Bau
        // ein (capturewindow.cpp:32–34).
        CaptureWindow window(&store);
        window.show();
        settle(200);

        auto *text = window.findChild<QPlainTextEdit *>();
        const QPalette windowPalette = window.palette();
        const QPalette textPalette = text->palette();

        printf("%-14s Fenster(Window)=%s  Textfeld(Base)=%s  Text=%s  Platzhalter=%s\n",
               qPrintable(name),
               qPrintable(hex(windowPalette, QPalette::Window)),
               qPrintable(hex(textPalette, QPalette::Base)),
               qPrintable(hex(textPalette, QPalette::Text)),
               qPrintable(hex(textPalette, QPalette::PlaceholderText)));
        printf("%-14s Fenstergröße leer: %dx%d\n", qPrintable(name), window.width(), window.height());

        window.grab().save(QStringLiteral("%1/capture-leer-%2.png").arg(directory, name));

        text->setPlainText(
            QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten — als Cronjob auf dem NAS?"));
        settle(200);
        printf("%-14s Fenstergröße getippt: %dx%d\n\n", qPrintable(name), window.width(), window.height());
        window.grab().save(QStringLiteral("%1/capture-getippt-%2.png").arg(directory, name));
    }

    return 0;
}
