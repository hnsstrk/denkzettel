// Messung 5 — Der Schatten, am laufenden Compositor.
//
// Diese Messung schließt eine Grenze, die das Planning als offen benannt hat.
// Offscreen ist der Schatten prinzipbedingt nicht zu belegen: `create()`
// liefert dort immer falsch, weil niemand da ist, dem die Kacheln gingen, und
// `QWidget::grab()` zeichnet nur das Widget — der Schatten liegt außerhalb.
// Der Ersatz war deshalb „Zusicherung Schatten angelegt" plus ein Bild vom
// Kunden.
//
// Die zweite Hälfte davon ist hier nicht mehr nötig: **Am laufenden Plasma ist
// der Rückgabewert messbar.** Das Programm baut das echte Erfassungsfenster,
// zeigt es auf der Sitzung und liest ab, was der Compositor angenommen hat.
//
// Es läuft deshalb **nicht** unter QT_QPA_PLATFORM=offscreen und **nicht** in
// pruefen.sh: Es braucht eine angemeldete Wayland-Sitzung und zeigt für ein
// paar Sekunden ein Fenster auf dem Bildschirm. Aufruf steht in LIESMICH.md.

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KLocalizedString>
#include <KWindowShadow>

#include <QApplication>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>

namespace
{
QTextStream out(stdout);

void report(const QString &tag, const CaptureWindow &window)
{
    const KWindowShadow *shadow = window.shadow();

    out << tag << "\n";
    out << "   Schattenobjekt vorhanden : " << (shadow ? "ja" : "nein") << "\n";
    if (!shadow) {
        out.flush();
        return;
    }

    out << "   vom Compositor angenommen: " << (shadow->isCreated() ? "ja" : "nein") << "\n";
    out << "   Kachel oben              : " << shadow->topTile()->image().size().width() << "x"
        << shadow->topTile()->image().size().height() << " px\n";
    out << "   Kachel oben links        : " << shadow->topLeftTile()->image().size().width() << "x"
        << shadow->topLeftTile()->image().size().height() << " px\n";
    out.flush();
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    out << "Messung 5 — Der Schatten am laufenden Compositor (#55, AK 1 und AK 7)\n";
    out << "=====================================================================\n\n";
    out << "Plattform: " << app.platformName() << "\n\n";

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen: " << store.lastError() << "\n";
        return 1;
    }

    auto *window = new CaptureWindow(&store);
    window->findChild<QPlainTextEdit *>()->setPlainText(
        QStringLiteral("Schattenprobe am laufenden Plasma — Messung 5 zu #55"));

    // Erstes Zeigen.
    window->showCapture();
    QTimer::singleShot(1500, [&] {
        report(QStringLiteral("A) Nach dem ERSTEN Zeigen"), *window);

        // Zweites Zeigen — der Punkt, den keine offscreen-Prüfung und kein
        // Standbild erreicht: showCapture() versteckt und mappt neu, die
        // Wayland-Surface verschwindet dabei. Ein Schatten, der nur einmal
        // gebunden würde, wäre hier weg.
        out << "\n   ... verstecken und erneut zeigen (Remap, SPEC 3) ...\n\n";
        out.flush();
        window->showCapture();
    });

    QTimer::singleShot(3500, [&] {
        report(QStringLiteral("B) Nach dem ZWEITEN Zeigen"), *window);
        out << "\nZu lesen: `angenommen = ja` ist der Rückgabewert von\n"
               "KWindowShadow::create(), also die Zusicherung 'Schatten angelegt' aus\n"
               "AK 7 — hier nicht als Ersatzform, sondern gemessen. Steht er in BEIDEN\n"
               "Abschnitten, ist zusätzlich die Remap-Grenze geschlossen.\n";
        out.flush();
        window->close();
        app.quit();
    });

    const int code = app.exec();
    delete window;
    return code;
}
