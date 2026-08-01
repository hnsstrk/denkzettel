// Achse 2 des Kundenbefunds: „Bspw. Farben, Schriften, abgerundete Ecken."
// Nimmt das Capture-Fenster die im System eingestellte Schrift — und geht es
// mit, wenn der Nutzer sie waehrend des Betriebs aendert? Der Dienst haelt das
// Fenster dauerhaft vorgehalten (SPEC 2.1), es wird also einmal gebaut.
//
// Aufruf: capture-schriftwechsel <Zielverzeichnis>

#include "capture/capturewindow.h"
#include "store/store.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QFont>
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

QString beschreibe(const QFont &font)
{
    return QStringLiteral("%1 %2 pt").arg(font.family()).arg(font.pointSizeF());
}

void report(const QString &phase, QWidget *window)
{
    auto *text = window->findChild<QPlainTextEdit *>();
    printf("%-10s Fenster %d x %d px, Textfeld %d px hoch, Zeilenabstand %d px\n",
           qPrintable(phase),
           window->width(),
           window->height(),
           text->height(),
           text->fontMetrics().lineSpacing());
    printf("%-10s Textfeld-Schrift : %s\n", qPrintable(phase), qPrintable(beschreibe(text->font())));
    for (QLabel *label : window->findChildren<QLabel *>()) {
        printf("%-10s Kleintext-Schrift: %s   (\"%s\")\n",
               qPrintable(phase),
               qPrintable(beschreibe(label->font())),
               qPrintable(label->text().left(12)));
    }
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Aufruf: capture-schriftwechsel <Zielverzeichnis>\n");
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

    printf("Systemschrift beim Start   : %s\n", qPrintable(beschreibe(app.font())));
    printf("Kleinste lesbare Systemschrift: %s\n\n",
           qPrintable(beschreibe(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont))));

    CaptureWindow window(&store);
    window.show();
    settle(300);
    report(QStringLiteral("vorher"), &window);
    window.grab().save(QStringLiteral("%1/schrift-1-vorher.png").arg(directory));

    // Wie eine Aenderung der Schriftart in den Systemeinstellungen: die
    // Anwendung bekommt eine neue Standardschrift, das Fenster lebt weiter.
    QFont groesser(QStringLiteral("Noto Serif"), 16);
    printf("\n--- Systemschrift gewechselt auf %s, Fenster bleibt bestehen ---\n\n",
           qPrintable(beschreibe(groesser)));
    app.setFont(groesser);
    settle(400);
    report(QStringLiteral("nachher"), &window);
    window.grab().save(QStringLiteral("%1/schrift-2-nachher.png").arg(directory));

    // Gegenprobe: neu gebaut unter derselben Schrift — so saehe es aus, wenn der
    // Dienst nach der Umstellung neu startete.
    CaptureWindow neu(&store);
    neu.show();
    settle(300);
    printf("\n");
    report(QStringLiteral("neu"), &neu);
    neu.grab().save(QStringLiteral("%1/schrift-3-neu-gebaut.png").arg(directory));

    return 0;
}
