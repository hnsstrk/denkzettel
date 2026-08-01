// Zeigt das echte CaptureWindow in einer verschachtelten KWin-Sitzung, damit
// der Kundenbefund im Desktop-Zusammenhang beurteilt werden kann: Ecken,
// Kontur, Schatten und Nachbarschaft zu einem gewöhnlichen KDE-Fenster.
// Der Daemon selbst taugt dafür nicht — er ist Einzelinstanz und liefe der
// laufenden Installation ins Gehege.
//
// Aufruf: capture-show [<Pfad zur .colors>] [<Text>]

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KColorScheme>
#include <KSharedConfig>

#include <QApplication>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <cstdio>

int main(int argc, char **argv)
{
    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    // Wie main.cpp:26 — die Anwendungskennung, an der KWin das Fenster erkennt.
    app.setDesktopFileName(QStringLiteral("org.denkzettel.Denkzettel"));

    if (argc > 1) {
        KSharedConfigPtr scheme = KSharedConfig::openConfig(QString::fromLocal8Bit(argv[1]), KConfig::SimpleConfig);
        app.setPalette(KColorScheme::createApplicationPalette(scheme));
    }

    Store store(sandbox.filePath(QStringLiteral("probe.db")));
    if (!store.open()) {
        printf("Store konnte nicht geöffnet werden: %s\n", qPrintable(store.lastError()));
        return 1;
    }

    CaptureWindow window(&store);
    window.showCapture();

    if (argc > 2) {
        window.findChild<QPlainTextEdit *>()->setPlainText(QString::fromLocal8Bit(argv[2]));
    }

    return app.exec();
}
