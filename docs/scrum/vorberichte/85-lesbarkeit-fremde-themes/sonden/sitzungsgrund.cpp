/*
 * Sonde der Vorprüfung zu #85, Bearbeiter A (UX) — der Kern von AK 2.
 *
 * Frage: Was gilt bei 2,7 % Deckung **wirklich**? Die Zahlen des Issues sind
 * gegen eine deckende Fläche gerechnet, die es dort nicht gibt; in Wahrheit
 * steht der Text auf dem Bildschirmhintergrund. Diese Sonde misst ihn aus einer
 * Aufnahme der **angemeldeten Sitzung** über einem **benannten Grund**.
 *
 * Aufbau, nach dem Bauplan von #83 (`sonden/weichzeichnerbeleg.cpp`):
 *   1. ein eigenes Vollbildfenster in einer benannten Farbe — es verdeckt den
 *      Schreibtisch des Kunden vollständig, sodass in der Aufnahme nur Inhalt
 *      dieser Sonde steht;
 *   2. eine Aufnahme **ohne** das Erfassungsfenster;
 *   3. dasselbe mit dem echten `CaptureWindow` darüber;
 *   4. der Unterschied beider Aufnahmen findet das Fenster — unabhängig davon,
 *      ob man es sieht. Genau das ist hier der Punkt: ein Fenster, das man
 *      nicht findet, weil es nichts abhebt, wäre der Befund selbst.
 *
 * Gespeichert wird nur der Ausschnitt um das Fenster; die Vollbildpuffer werden
 * sofort gelöscht.
 *
 * Aufruf: sitzungsgrund <Zielordner> <theme> <grundname> <r> <g> <b>
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProcess>
#include <QScreen>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QWidget>

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
    return QStringLiteral("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue());
}

/** Vollbild in einer benannten Farbe. */
class Grund : public QWidget
{
public:
    explicit Grund(const QColor &farbe)
        : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint)
        , m_farbe(farbe)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter maler(this);
        maler.fillRect(rect(), m_farbe);
    }

private:
    QColor m_farbe;
};

void warte(int ms)
{
    QEventLoop schleife;
    QTimer::singleShot(ms, &schleife, &QEventLoop::quit);
    schleife.exec();
}

/** Eine Vollbildaufnahme, die der Aufrufer gleich wieder löscht. */
QImage vollbild(const QString &datei)
{
    QFile::remove(datei);
    QProcess spectacle;
    spectacle.start(QStringLiteral("spectacle"),
                    {QStringLiteral("-f"), QStringLiteral("-b"), QStringLiteral("-n"),
                     QStringLiteral("-o"), datei});
    if (!spectacle.waitForFinished(20000)) {
        return {};
    }
    QImage bild(datei);
    QFile::remove(datei);
    return bild;
}

/** Mittlere Farbe eines kleinen Blocks — hält Rauschen und Ränder heraus. */
QColor block(const QImage &bild, int x, int y, int kante)
{
    long r = 0;
    long g = 0;
    long b = 0;
    int n = 0;
    for (int j = y; j < y + kante && j < bild.height(); ++j) {
        for (int i = x; i < x + kante && i < bild.width(); ++i) {
            const QColor c = bild.pixelColor(i, j);
            r += c.red();
            g += c.green();
            b += c.blue();
            ++n;
        }
    }
    return n == 0 ? QColor() : QColor(int(r / n), int(g / n), int(b / n));
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QStringList a = app.arguments();
    if (a.size() < 7) {
        out << "Aufruf: sitzungsgrund <Zielordner> <theme> <grundname> <r> <g> <b>\n";
        out.flush();
        return 2;
    }
    const QString ordner = a.at(1);
    const QString theme = a.at(2);
    const QString grundname = a.at(3);
    const QColor grundfarbe(a.at(4).toInt(), a.at(5).toInt(), a.at(6).toInt());
    QDir().mkpath(ordner);

    out << "=== Vorprüfung #85, Sitzungsbeleg über benanntem Grund ===\n";
    out << "Plattform : " << app.platformName() << "\n";
    out << "Theme     : " << theme << "\n";
    out << "Grund     : " << grundname << " (" << farbe(grundfarbe) << ")\n";
    if (app.platformName() != QLatin1String("wayland")) {
        out << "Ohne angemeldete Sitzung hat diese Sonde keinen Gegenstand — übersprungen.\n";
        out.flush();
        return 0;
    }

    QTemporaryDir fluechtig;

    Grund grund(grundfarbe);
    // Ausdrücklich **nicht** showFullScreen(): ein Vollbildfenster liegt bei
    // diesem Compositor in einer höheren Ebene als ein gewöhnliches, und das
    // Erfassungsfenster verschwindet darunter — gemessen, erster Lauf dieser
    // Sonde (größter Unterschied beider Aufnahmen: 0 Zählschritte). Ein
    // gewöhnliches Fenster in Bildschirmgröße verdeckt den Schreibtisch
    // genauso und lässt das Erfassungsfenster darüber.
    grund.resize(app.primaryScreen()->size());
    grund.show();
    warte(900);

    const QImage ohne = vollbild(fluechtig.filePath(QStringLiteral("_ohne.png")));
    if (ohne.isNull()) {
        out << "Keine Aufnahme entstanden — der Beleg lässt sich so nicht führen.\n";
        out.flush();
        return 2;
    }

    // Selbstprüfung des Aufbaus, und sie ist nicht Zierat: Bei **gesperrtem
    // Bildschirm** liefert `spectacle -f` ein durchweg schwarzes Bild, obwohl
    // Fenster weiter abgebildet werden und jeder Rückgabewert in Ordnung ist.
    // Ohne diese Prüfung meldet die Sonde dann „das Fenster hebt sich nirgends
    // ab" — ein Satz, der wie ein Befund aussieht und nur den Rollladen misst
    // (gemessen am 05.08.2026, LockedHint=yes).
    const QColor gesehen = block(ohne, ohne.width() / 2, ohne.height() / 2, 8);
    out << "Grund sichtbar: " << (grund.isVisible() ? "ja" : "nein") << ", " << grund.width()
        << "x" << grund.height() << "; Bildmitte der ersten Aufnahme: " << farbe(gesehen) << "\n";
    const int abweichung = qAbs(gesehen.red() - grundfarbe.red())
        + qAbs(gesehen.green() - grundfarbe.green()) + qAbs(gesehen.blue() - grundfarbe.blue());
    if (abweichung > 24) {
        out << "ABBRUCH: Die Aufnahme zeigt den Grund nicht (Abweichung " << abweichung
            << " Zählschritte).\n";
        out << "Ist der Bildschirm gesperrt? Dann ist jede Aufnahme schwarz, und jede Zahl\n";
        out << "aus ihr wäre erfunden. Prüfen mit: loginctl show-session … -p LockedHint\n";
        out.flush();
        return 3;
    }

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store nicht offen: " << store.lastError() << "\n";
        out.flush();
        return 1;
    }

    CaptureWindow fenster(&store);
    fenster.reloadDesktopTheme(theme);
    if (auto *feld = fenster.findChild<QPlainTextEdit *>()) {
        feld->setPlainText(QStringLiteral("Denkzettel"));
    }
    fenster.showCapture();
    warte(1600);

    out << "Fenster sichtbar: " << (fenster.isVisible() ? "ja" : "nein") << ", Geometrie "
        << fenster.width() << "x" << fenster.height() << " an " << fenster.x() << ","
        << fenster.y() << "\n";

    const QImage mit = vollbild(fluechtig.filePath(QStringLiteral("_mit.png")));
    if (mit.isNull() || mit.size() != ohne.size()) {
        out << "Zweite Aufnahme fehlt oder hat eine andere Größe — abgebrochen.\n";
        out.flush();
        return 2;
    }

    // Das Fenster ist der Unterschied beider Aufnahmen. Der Weg über den
    // Unterschied ist der einzige, der auch dann trägt, wenn das Fenster kaum
    // etwas abhebt — und das ist bei 2,7 % Deckung der Normalfall.
    int links = mit.width();
    int oben = mit.height();
    int rechts = -1;
    int unten = -1;
    int groesster = 0;
    for (int y = 0; y < mit.height(); ++y) {
        for (int x = 0; x < mit.width(); ++x) {
            const QColor c1 = ohne.pixelColor(x, y);
            const QColor c2 = mit.pixelColor(x, y);
            const int abstand = qAbs(c1.red() - c2.red()) + qAbs(c1.green() - c2.green())
                + qAbs(c1.blue() - c2.blue());
            groesster = qMax(groesster, abstand);
            if (abstand > 12) {
                links = qMin(links, x);
                rechts = qMax(rechts, x);
                oben = qMin(oben, y);
                unten = qMax(unten, y);
            }
        }
    }

    out << "Aufnahmen " << mit.width() << "x" << mit.height() << ", größter Unterschied "
        << groesster << " Zählschritte\n";
    if (rechts < 0) {
        out << "Das Fenster hebt sich vom Grund NIRGENDS um mehr als 12 Zählschritte ab.\n";
        out << "Das ist selbst der Befund: über diesem Grund ist es unsichtbar.\n";
        out.flush();
        return 0;
    }

    const QRect gefunden(links, oben, rechts - links + 1, unten - oben + 1);
    out << "Fenster samt Schatten gefunden: " << gefunden.width() << "x" << gefunden.height()
        << " an " << gefunden.x() << "," << gefunden.y() << "\n";

    const QImage ausschnitt = mit.copy(gefunden);
    const QString ziel =
        QDir(ordner).filePath(QStringLiteral("fenster-%1-%2.png").arg(theme, grundname));
    ausschnitt.save(ziel);
    out << "Ausschnitt abgelegt: " << ziel << "\n";

    // Gemessen wird die zusammengesetzte Fläche dort, wo Text stehen kann, aber
    // in dieser Aufnahme keiner steht: rechts oben im Fenster, unterhalb des
    // Anwendungsnamens. Ein Block statt eines Bildpunktes, damit ein
    // Schriftrand die Zahl nicht verzieht.
    const int px = gefunden.width() * 3 / 4;
    const int py = gefunden.height() / 2;
    const QColor flaeche = block(ausschnitt, px, py, 12);
    const QColor grundgemessen = block(ohne, gefunden.x() + px, gefunden.y() + py, 12);

    out << "\nGrund an derselben Stelle ohne Fenster : " << farbe(grundgemessen) << "\n";
    out << "Zusammengesetzte Fläche im Fenster     : " << farbe(flaeche) << "\n";

    const QColor themeschrift(35, 38, 41);
    const QColor schemaschrift = fenster.palette().color(QPalette::WindowText);
    const QColor gedaempft = fenster.palette().color(QPalette::PlaceholderText);

    out << "\nKontrast gegen diese Fläche:\n";
    out << "  Themeschrift  " << farbe(themeschrift) << " : "
        << QString::number(kontrast(themeschrift, flaeche), 'f', 2) << ":1\n";
    out << "  Schemaschrift " << farbe(schemaschrift) << " : "
        << QString::number(kontrast(schemaschrift, flaeche), 'f', 2) << ":1\n";
    out << "  Kleintext     " << farbe(gedaempft) << " : "
        << QString::number(kontrast(gedaempft, flaeche), 'f', 2) << ":1\n";

    out.flush();
    return 0;
}
