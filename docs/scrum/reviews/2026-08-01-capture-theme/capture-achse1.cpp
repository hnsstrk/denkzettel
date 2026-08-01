// Achse 1 des Kundenbefunds, vollstaendig: Folgen alle Farbflaechen des
// Capture-Fensters der Palette des aktiven Farbschemas — Hintergrund, Textfarbe,
// Auswahlfarbe, Textcursor, Scrollbalken?
//
// Aufruf: capture-achse1 <Zielverzeichnis> <Name>=<Pfad zur .colors> …

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KColorScheme>
#include <KSharedConfig>

#include <QApplication>
#include <QElapsedTimer>
#include <QPlainTextEdit>
#include <QScrollBar>
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
        printf("Aufruf: capture-achse1 <Zielverzeichnis> <Name>=<Pfad zur .colors> …\n");
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

    for (int i = 2; i < argc; ++i) {
        const QString argument = QString::fromLocal8Bit(argv[i]);
        const QString name = argument.section(QLatin1Char('='), 0, 0);

        app.setPalette(KColorScheme::createApplicationPalette(
            KSharedConfig::openConfig(argument.section(QLatin1Char('='), 1), KConfig::SimpleConfig)));

        CaptureWindow window(&store);
        window.show();

        auto *text = window.findChild<QPlainTextEdit *>();
        // Genug Text fuer den Scrollbalken (SPEC 3: ab ~8 Zeilen) und eine
        // Auswahl, damit Auswahlfarbe und Auswahltext sichtbar werden.
        text->setPlainText(QStringLiteral(
            "restic-Backup: prune-Policy prüfen\nmonatliche Snapshots behalten\nals Cronjob auf dem NAS?\n"
            "Frage an Marek weiterleiten\nDanach Doku ergänzen\nUnd die Retro vorbereiten\n"
            "Kalendereintrag für Freitag\nRechnung Stadtwerke prüfen\nZahnarzttermin verschieben\n"
            "Buchtipp notieren: Systemdenken"));
        text->selectAll();
        settle(300);

        const QPalette p = text->palette();
        printf("%-14s Fenster=%s  Feld=%s  Text=%s\n", qPrintable(name),
               qPrintable(hex(window.palette(), QPalette::Window)),
               qPrintable(hex(p, QPalette::Base)),
               qPrintable(hex(p, QPalette::Text)));
        printf("%-14s Auswahl=%s  Auswahltext=%s  Scrollbalken sichtbar=%s\n\n",
               qPrintable(name),
               qPrintable(hex(p, QPalette::Highlight)),
               qPrintable(hex(p, QPalette::HighlightedText)),
               text->verticalScrollBar()->isVisible() ? "ja" : "nein");

        window.grab().save(QStringLiteral("%1/achse1-auswahl-%2.png").arg(directory, name));
    }

    return 0;
}
