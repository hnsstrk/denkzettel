// Wirkungsprobe zu den Empfehlungen. Kein Projekt-Code wird geaendert: der
// Helfer greift zur Laufzeit in den Widget-Baum, wie es die Retro-Gegenprobe
// vom 01.08.2026 vorgemacht hat.
//
// Variante A = Ist-Zustand (capturewindow.cpp:32–34 friert die Farbe ein).
// Variante B = dieselben Labels, aber ueber die Palettenrolle statt ueber eine
//              festgeschriebene Farbe.
//
// Aufruf: capture-varianten <Zielverzeichnis> <Schema beim Bau> <Schema danach>

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KColorScheme>
#include <KSharedConfig>

#include <QApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleFactory>
#include <QTemporaryDir>

#include <cmath>
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
    app.setPalette(KColorScheme::createApplicationPalette(
        KSharedConfig::openConfig(path, KConfig::SimpleConfig)));
}

double luminance(const QColor &colour)
{
    auto channel = [](double v) {
        return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(colour.redF()) + 0.7152 * channel(colour.greenF())
        + 0.0722 * channel(colour.blueF());
}

void report(const QString &variant, QWidget *window)
{
    const QColor background = window->palette().color(QPalette::Active, QPalette::Window);
    for (QLabel *label : window->findChildren<QLabel *>()) {
        const QColor text = label->palette().color(QPalette::Active, label->foregroundRole());
        const double a = std::max(luminance(background), luminance(text));
        const double b = std::min(luminance(background), luminance(text));
        printf("%-10s \"%s\"\n           Grund %s  Text %s  Kontrast %.2f:1\n",
               qPrintable(variant),
               qPrintable(label->text()),
               qPrintable(background.name()),
               qPrintable(text.name()),
               (a + 0.05) / (b + 0.05));
    }
}
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("Aufruf: capture-varianten <Zielverzeichnis> <Schema beim Bau> <Schema danach>\n");
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

    applyScheme(app, QString::fromLocal8Bit(argv[2]));

    CaptureWindow ist(&store);
    ist.show();

    CaptureWindow verbessert(&store);
    // Variante B: die festgeschriebene Farbe zuruecknehmen und stattdessen die
    // Rolle setzen, mit der gezeichnet wird — sie wird bei jedem Palettenwechsel
    // neu aus der dann gueltigen Palette gelesen.
    for (QLabel *label : verbessert.findChildren<QLabel *>()) {
        label->setPalette(QPalette());
        label->setForegroundRole(QPalette::PlaceholderText);
    }
    verbessert.show();
    settle(300);

    printf("=== beim Bau (%s) ===\n", qPrintable(QString::fromLocal8Bit(argv[2]).section(QLatin1Char('/'), -1)));
    report(QStringLiteral("A Ist"), &ist);
    report(QStringLiteral("B Rolle"), &verbessert);

    applyScheme(app, QString::fromLocal8Bit(argv[3]));
    settle(300);

    printf("\n=== nach dem Wechsel auf %s, Fenster leben weiter ===\n",
           qPrintable(QString::fromLocal8Bit(argv[3]).section(QLatin1Char('/'), -1)));
    report(QStringLiteral("A Ist"), &ist);
    report(QStringLiteral("B Rolle"), &verbessert);

    ist.grab().save(QStringLiteral("%1/variante-A-ist.png").arg(directory));
    verbessert.grab().save(QStringLiteral("%1/variante-B-rolle.png").arg(directory));

    return 0;
}
