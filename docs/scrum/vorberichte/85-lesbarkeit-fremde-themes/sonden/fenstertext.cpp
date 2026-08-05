/*
 * Sonde der Vorprüfung zu #85, Bearbeiter A (UX).
 *
 * Frage: Was tut das **gebaute Fenster** von heute (Stand nach #83) mit seinen
 * beiden Textklassen, wenn das Desktop-Theme wechselt?
 *
 * Gemessen wird je Theme, am laufenden `CaptureWindow`:
 *   - die Farbe des Notiztextes (`QPalette::Text` des Textfeldes),
 *   - die Farbe der gedämpften Kleintexte (Rolle `PlaceholderText` der beiden
 *     `QLabel` — Anwendungsname und Fußzeile),
 *   - die Fläche, die das Fenster in seiner Mitte zeichnet,
 *   - der Kontrast beider Klassen gegen diese Fläche, deckend gerechnet.
 *
 * Das ist die Ausgangslage, gegen die #85 AK 1 und AK 5 zu messen sind: heute
 * ändert ein Theme-Wechsel die Fläche und lässt die Schrift stehen.
 *
 * Schreibt nur in ein eigenes Verzeichnis, nichts in die Einstellungen des
 * Kunden. `QStandardPaths::setTestModeEnabled(true)` hält auch das Lesen von
 * `plasmarc` aus dem Weg — das Theme wird hier ausdrücklich übergeben.
 *
 * Aufruf: fenstertext <theme> [<theme> …]
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QLabel>
#include <QPalette>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

#include <cmath>

namespace
{
QTextStream out(stdout);

double kanal(int c)
{
    const double v = c / 255.0;
    return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
}

double leuchtdichte(const QColor &c)
{
    return 0.2126 * kanal(c.red()) + 0.7152 * kanal(c.green()) + 0.0722 * kanal(c.blue());
}

double kontrast(const QColor &a, const QColor &b)
{
    const double la = leuchtdichte(a);
    const double lb = leuchtdichte(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

QString farbe(const QColor &c)
{
    return QStringLiteral("%1,%2,%3/a%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QStringList themes = app.arguments().mid(1);

    QStandardPaths::setTestModeEnabled(true);
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store nicht offen: " << store.lastError() << "\n";
        return 1;
    }

    CaptureWindow fenster(&store);
    const QList<QLabel *> kleintexte = fenster.findChildren<QLabel *>();
    auto *feld = fenster.findChild<QPlainTextEdit *>();

    out << "Anwendungspalette: WindowText " << farbe(fenster.palette().color(QPalette::WindowText))
        << "  PlaceholderText " << farbe(fenster.palette().color(QPalette::PlaceholderText)) << "\n";
    out << "Kleintexte gefunden: " << kleintexte.size() << "\n\n";

    for (const QString &theme : themes) {
        fenster.reloadDesktopTheme(theme);
        fenster.resize(600, fenster.sizeHint().height());

        const QImage bild = fenster.grab().toImage().convertToFormat(QImage::Format_ARGB32);
        const QColor flaeche = bild.pixelColor(bild.width() / 2, bild.height() / 2);
        const QColor deckend(flaeche.red(), flaeche.green(), flaeche.blue());

        const QColor notiz = feld->palette().color(QPalette::Text);
        const QColor gedaempft = kleintexte.isEmpty()
            ? QColor()
            : kleintexte.first()->palette().color(kleintexte.first()->foregroundRole());

        out << "==== " << theme << "\n";
        out << "  Fläche in der Mitte : " << farbe(flaeche) << "   Deckung "
            << QString::number(100.0 * flaeche.alpha() / 255.0, 'f', 1) << " %\n";
        out << "  Notiztext           : " << farbe(notiz) << "   Kontrast deckend "
            << QString::number(kontrast(notiz, deckend), 'f', 2) << ":1\n";
        out << "  Kleintext (gedämpft): " << farbe(gedaempft) << "   Kontrast deckend "
            << QString::number(kontrast(gedaempft, deckend), 'f', 2) << ":1\n";
        out << "  Fenstergröße        : " << bild.width() << "x" << bild.height() << "\n\n";
    }

    out.flush();
    return 0;
}
