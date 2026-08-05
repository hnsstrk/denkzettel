/*
 * Beleg zu #85, AK 2 — der Kontrast beider Textklassen aus einer Aufnahme der
 * **angemeldeten Sitzung**, über einem benannten Grund.
 *
 * Abgeleitet von `sonden/sitzungsgrund.cpp` der Vorprüfung, mit einem
 * Unterschied, der die Sonde erst zu einem Beleg der Umsetzung macht: Jene
 * rechnet gegen eine **fest verdrahtete** Themeschrift (35,38,41) — sie sollte
 * ja beantworten, was eine Umsetzung brächte, die es noch nicht gab. Diese hier
 * liest die Farben, die das gebaute Fenster **wirklich trägt**: die Farbe des
 * Notiztextes aus der Palette des Textfeldes und die der gedämpften Klasse aus
 * der Palette eines der beiden Kleintexte. Was gemessen wird, ist damit das
 * Ergebnis des Codes und nicht die Annahme des Messenden.
 *
 * Der Aufbau, und beide Punkte sind gemessen und nicht bedacht:
 *   1. Der Grund ist ein **gewöhnliches** Fenster in Bildschirmgröße und
 *      ausdrücklich kein Vollbildfenster — ein Vollbildfenster legt dieser
 *      Compositor über das Erfassungsfenster, und gemessen würde der Grund
 *      über der Hülle statt unter ihr.
 *   2. Vor jeder Zahl steht die Selbstprüfung des Aufbaus: Bei **gesperrtem**
 *      Bildschirm liefert `spectacle -f` ein durchweg schwarzes Bild mit
 *      Rückgabe 0, und die Sonde meldete dann „das Fenster hebt sich nirgends
 *      ab" — ein Satz über den Rollladen, der aussieht wie einer über das
 *      Fenster.
 *
 * Zwei Aufnahmen je Lauf, weil die Story beide Zustände berührt: das leere
 * Fenster (der Platzhaltertext gehört seit #85 zur gedämpften Klasse) und das
 * beschriebene.
 *
 * Aufruf: sitzungsbeleg <Zielordner> <theme> <grundname> <r> <g> <b>
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QLabel>
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

/** Ein gewöhnliches Fenster in Bildschirmgröße, in einer benannten Farbe. */
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

/** Mittlere Farbe eines kleinen Blocks — hält Rauschen und Schriftränder heraus. */
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

/**
 * Alle Bildpunkte, in denen sich zwei Aufnahmen unterscheiden.
 *
 * Das ist **nicht** dasselbe wie „das Fenster": Wenn das Erfassungsfenster
 * erscheint, ändert sich auch die Fensterleiste, weil dort ein Eintrag
 * dazukommt. Wer aus allen Unterschieden ein einziges Rechteck bildet, bekommt
 * eines, das vom Fenster bis in die Leiste reicht — und misst dann mitten im
 * blanken Grund. Gemessen am 05.08.2026, erster Lauf dieser Sonde: 2210x1274
 * statt der erwarteten 960x291, und die Zahl daraus sah aus wie ein Befund über
 * die Hülle. Deshalb zerlegt `fensterbereich()` unten die Maske in
 * zusammenhängende Teile.
 */
QList<bool> unterschiedsmaske(const QImage &ohne, const QImage &mit, int &groesster)
{
    QList<bool> maske(qsizetype(mit.width()) * mit.height(), false);
    groesster = 0;

    for (int y = 0; y < mit.height(); ++y) {
        for (int x = 0; x < mit.width(); ++x) {
            const QColor c1 = ohne.pixelColor(x, y);
            const QColor c2 = mit.pixelColor(x, y);
            const int abstand = qAbs(c1.red() - c2.red()) + qAbs(c1.green() - c2.green())
                + qAbs(c1.blue() - c2.blue());
            groesster = qMax(groesster, abstand);
            maske[qsizetype(y) * mit.width() + x] = abstand > 12;
        }
    }

    return maske;
}

/**
 * Der zusammenhängende Teil der Maske, der das Fenster ist — erkannt an seiner
 * **Größe**, die von außen bekannt ist.
 *
 * Das Erkennungsmerkmal kommt ausdrücklich nicht aus derselben Messung: Die
 * erwartete Kantenlänge ist die Fenstergröße mal dem Bildpunktverhältnis, und
 * beide stehen am Fenster. Ein Teil, der weit kleiner oder mehr als doppelt so
 * groß ist, ist etwas anderes — die Fensterleiste, ein Zeiger, ein Nachziehen
 * des Grundes.
 *
 * Die untere Schranke liegt bei drei Vierteln und nicht bei eins, und auch das
 * ist gemessen: Über **schwarzem** Grund hebt sich der äußere Rand der Hülle
 * von `cachyos-emerald-light` nicht ab, und das Teilstück fällt auf 941x272 bei
 * erwarteten 960x291. Bei einer Schranke von eins hätte die Sonde gemeldet, das
 * Fenster sei unsichtbar — ein Befund über die Schranke, der aussieht wie einer
 * über das Theme.
 */
QRect fensterbereich(const QList<bool> &maske, const QSize &bild, const QSize &erwartet,
                     QRect &groesstesTeilstueck)
{
    QList<bool> gesehen = maske;
    QRect bester;
    groesstesTeilstueck = QRect();

    for (int y0 = 0; y0 < bild.height(); ++y0) {
        for (int x0 = 0; x0 < bild.width(); ++x0) {
            if (!gesehen.at(qsizetype(y0) * bild.width() + x0)) {
                continue;
            }

            QList<QPoint> stapel{QPoint(x0, y0)};
            gesehen[qsizetype(y0) * bild.width() + x0] = false;
            int links = x0;
            int rechts = x0;
            int oben = y0;
            int unten = y0;

            while (!stapel.isEmpty()) {
                const QPoint p = stapel.takeLast();
                links = qMin(links, p.x());
                rechts = qMax(rechts, p.x());
                oben = qMin(oben, p.y());
                unten = qMax(unten, p.y());

                const QPoint nachbarn[4] = {QPoint(p.x() - 1, p.y()), QPoint(p.x() + 1, p.y()),
                                            QPoint(p.x(), p.y() - 1), QPoint(p.x(), p.y() + 1)};
                for (const QPoint &n : nachbarn) {
                    if (n.x() < 0 || n.y() < 0 || n.x() >= bild.width() || n.y() >= bild.height()) {
                        continue;
                    }
                    const qsizetype i = qsizetype(n.y()) * bild.width() + n.x();
                    if (gesehen.at(i)) {
                        gesehen[i] = false;
                        stapel.append(n);
                    }
                }
            }

            const QRect teil(links, oben, rechts - links + 1, unten - oben + 1);
            // Auch mitgeschrieben, wenn er nicht passt: Ein Fehlschlag, der nur
            // „nichts gefunden" sagt, lässt offen, ob das Fenster unsichtbar
            // war oder der Aufbau nicht stand.
            if (teil.width() * teil.height() > groesstesTeilstueck.width() * groesstesTeilstueck.height()) {
                groesstesTeilstueck = teil;
            }
            const bool passt = 4 * teil.width() >= 3 * erwartet.width()
                && 4 * teil.height() >= 3 * erwartet.height()
                && teil.width() <= 2 * erwartet.width() && teil.height() <= 2 * erwartet.height();
            if (passt && teil.width() * teil.height() > bester.width() * bester.height()) {
                bester = teil;
            }
        }
    }

    return bester;
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QStringList a = app.arguments();
    if (a.size() < 7) {
        out << "Aufruf: sitzungsbeleg <Zielordner> <theme> <grundname> <r> <g> <b>\n";
        out.flush();
        return 2;
    }
    const QString ordner = a.at(1);
    const QString theme = a.at(2);
    const QString grundname = a.at(3);
    const QColor grundfarbe(a.at(4).toInt(), a.at(5).toInt(), a.at(6).toInt());
    QDir().mkpath(ordner);

    out << "=== #85 AK 2 — Sitzungsbeleg über benanntem Grund ===\n";
    out << "Plattform  : " << app.platformName() << "\n";
    out << "Theme      : " << theme << "\n";
    out << "Grund      : " << grundname << " (" << farbe(grundfarbe) << ")\n";
    // Der Auswahlpfad gehört zu jeder Zahl: seit #83 hat jedes Theme zwei
    // Flächen, und welche gilt, entscheidet genau diese Frage an KWin.
    const bool weichzeichnet = capture::sessionBlursBehindWindows();
    out << "Auswahlpfad: " << (weichzeichnet ? "durchscheinend (Sitzung weichzeichnet)"
                                             : "opaque (Sitzung weichzeichnet nicht)")
        << "\n";

    if (app.platformName() != QLatin1String("wayland")) {
        out << "Ohne angemeldete Sitzung hat diese Sonde keinen Gegenstand — übersprungen.\n";
        out.flush();
        return 0;
    }

    QTemporaryDir fluechtig;

    Grund grund(grundfarbe);
    grund.resize(app.primaryScreen()->size());
    grund.show();
    warte(900);

    const QImage ohne = vollbild(fluechtig.filePath(QStringLiteral("_ohne.png")));
    if (ohne.isNull()) {
        out << "Keine Aufnahme entstanden — der Beleg lässt sich so nicht führen.\n";
        out.flush();
        return 2;
    }

    // Die Selbstprüfung des Aufbaus, und sie ist kein Zierat: siehe Kopf.
    const QColor gesehen = block(ohne, ohne.width() / 2, ohne.height() / 2, 8);
    out << "Grund sichtbar: " << (grund.isVisible() ? "ja" : "nein") << ", " << grund.width() << "x"
        << grund.height() << "; Bildmitte der ersten Aufnahme: " << farbe(gesehen) << "\n";
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
    auto *feld = fenster.findChild<QPlainTextEdit *>();
    const QList<QLabel *> kleintexte = fenster.findChildren<QLabel *>();
    if (!feld || kleintexte.isEmpty()) {
        out << "Fenster ohne Textfeld oder ohne Kleintexte — abgebrochen.\n";
        out.flush();
        return 1;
    }

    // Zuerst der Leerzustand: der Platzhaltertext gehört seit #85 zur
    // gedämpften Klasse und wird aus derselben Rolle gezeichnet.
    fenster.showCapture();
    warte(1600);
    const QImage leerBild = vollbild(fluechtig.filePath(QStringLiteral("_leer.png")));
    if (leerBild.isNull() || leerBild.size() != ohne.size()) {
        out << "Die Aufnahme des leeren Fensters fehlt — sie trägt die Flächenzahl,\n";
        out << "weil in der beschriebenen an der Messstelle Schrift steht. Abgebrochen.\n";
        out.flush();
        return 2;
    }

    feld->setPlainText(QStringLiteral("Denkzettel — Lesbarkeit unter fremden Themes"));
    warte(600);

    out << "Fenster sichtbar: " << (fenster.isVisible() ? "ja" : "nein") << ", Geometrie "
        << fenster.width() << "x" << fenster.height() << " an " << fenster.x() << ","
        << fenster.y() << "\n";

    const QImage mit = vollbild(fluechtig.filePath(QStringLiteral("_mit.png")));
    if (mit.isNull() || mit.size() != ohne.size()) {
        out << "Zweite Aufnahme fehlt oder hat eine andere Größe — abgebrochen.\n";
        out.flush();
        return 2;
    }

    // Das Fenster ist ein zusammenhängender Teil des Unterschieds beider
    // Aufnahmen — nicht der ganze Unterschied (siehe fensterbereich()). Der Weg
    // über den Unterschied ist der einzige, der auch dann trägt, wenn das
    // Fenster kaum etwas abhebt, und das ist bei 3,5 % Deckung der Normalfall.
    int groesster = 0;
    const QList<bool> maske = unterschiedsmaske(ohne, mit, groesster);
    out << "Aufnahmen " << mit.width() << "x" << mit.height() << ", größter Unterschied "
        << groesster << " Zählschritte\n";

    // Die erwartete Kantenlänge kommt vom Fenster selbst und nicht aus der
    // Aufnahme: Beide Seiten desselben Vergleichs dürfen nicht von derselben
    // Messung stammen, sonst kann er auf beiden Seiten falsch sein und trotzdem
    // „stimmt" melden.
    const qreal verhaeltnis = fenster.devicePixelRatioF();
    const QSize erwartet(qRound(fenster.width() * verhaeltnis),
                         qRound(fenster.height() * verhaeltnis));
    out << "Bildpunktverhältnis des Fensters: " << verhaeltnis << ", erwartete Kantenlänge "
        << erwartet.width() << "x" << erwartet.height() << "\n";

    QRect groesstesTeilstueck;
    const QRect gefunden = fensterbereich(maske, mit.size(), erwartet, groesstesTeilstueck);
    if (!gefunden.isValid()) {
        out << "Kein zusammenhängender Teil in der erwarteten Größe gefunden.\n";
        out << "Größtes Teilstück überhaupt: " << groesstesTeilstueck.width() << "x"
            << groesstesTeilstueck.height() << " an " << groesstesTeilstueck.x() << ","
            << groesstesTeilstueck.y() << "\n";
        out << "Über diesem Grund hebt sich das Fenster nirgends auf seiner ganzen Fläche ab.\n";
        out << "Das ist selbst der Befund und keine Panne des Aufbaus: Bei 3,5 % Deckung\n";
        out << "steht die Hülle praktisch nicht zwischen Text und Grund. Keine Zahl geraten.\n";
        out.flush();
        return 0;
    }
    out << "Fenster samt Schatten gefunden: " << gefunden.width() << "x" << gefunden.height()
        << " an " << gefunden.x() << "," << gefunden.y() << "\n";

    const QImage ausschnitt = mit.copy(gefunden);
    const QString ziel =
        QDir(ordner).filePath(QStringLiteral("fenster-%1-%2.png").arg(theme, grundname));
    ausschnitt.save(ziel);
    out << "Ausschnitt abgelegt: " << ziel << "\n";
    const QString zielLeer =
        QDir(ordner).filePath(QStringLiteral("fenster-%1-%2-leer.png").arg(theme, grundname));
    leerBild.copy(gefunden).save(zielLeer);
    out << "Leerzustand abgelegt : " << zielLeer << "\n";

    // Gemessen wird in der **Mitte** des gefundenen Teils. Sie liegt im Fenster
    // und nicht im Schatten oder daneben: Das Teilstück hat zwischen drei
    // Vierteln und dem Doppelten der Kantenlänge des Fensters (Schranke oben),
    // der Schatten umgibt das Fenster gleichmäßig, also ist die Mitte des
    // umschließenden Rechtecks vom Fenster bedeckt. Das Verhältnis steht in der
    // Ausgabe, damit ein Leser es nicht glauben muss.
    const QPoint mitte = gefunden.center();
    out << "Messpunkt " << mitte.x() << "," << mitte.y() << " (Mitte des Teilstücks); "
        << "Teilstück zu Fenster: " << QString::number(1.0 * gefunden.width() / erwartet.width(), 'f', 2)
        << " breit, " << QString::number(1.0 * gefunden.height() / erwartet.height(), 'f', 2)
        << " hoch\n";

    // Die Fläche aus der Aufnahme des **leeren** Fensters: In der beschriebenen
    // steht an dieser Stelle Text, und dann misst man die Schrift statt des
    // Grundes, auf dem sie steht.
    const int kante = 12;
    const QColor flaeche = block(leerBild, mitte.x() - kante / 2, mitte.y() - kante / 2, kante);
    const QColor grundgemessen = block(ohne, mitte.x() - kante / 2, mitte.y() - kante / 2, kante);

    out << "\nGrund an derselben Stelle ohne Fenster : " << farbe(grundgemessen) << "\n";
    out << "Zusammengesetzte Fläche im Fenster     : " << farbe(flaeche) << "\n";

    // Und jetzt die Farben, die das Fenster **trägt** — nicht die, die jemand
    // erwartet hat.
    const QColor notiz = feld->palette().color(QPalette::Text);
    const QColor gedaempft =
        kleintexte.first()->palette().color(kleintexte.first()->foregroundRole());
    const capture::ThemeTextColours themefarben = capture::themeTextColoursOf(theme);

    out << "\nWoher die Farben kommen:\n";
    out << "  colors-Datei des Themes : "
        << (themefarben.normal.isValid() ? QStringLiteral("ja") : QStringLiteral("nein")) << "\n";
    out << "  Notiztext des Fensters  : " << farbe(notiz) << "\n";
    out << "  Kleintext des Fensters  : " << farbe(gedaempft) << "\n";
    out << "  Schemaschrift (Palette) : " << farbe(fenster.palette().color(QPalette::WindowText))
        << "\n";
    out << "  Schema gedämpft         : "
        << farbe(fenster.palette().color(QPalette::PlaceholderText)) << "\n";

    out << "\nKontrast gegen die zusammengesetzte Fläche:\n";
    out << "  Notiztext : " << QString::number(kontrast(notiz, flaeche), 'f', 2) << ":1\n";
    out << "  Kleintext : " << QString::number(kontrast(gedaempft, flaeche), 'f', 2) << ":1"
        << (kontrast(gedaempft, flaeche) < 4.5 ? QStringLiteral("   — unter 4,5:1 (Grenze, #84)")
                                               : QString())
        << "\n";
    out << "  Zum Vergleich, was vor #85 gestanden hätte:\n";
    out << "    Schemaschrift : "
        << QString::number(kontrast(fenster.palette().color(QPalette::WindowText), flaeche), 'f', 2)
        << ":1\n";
    out << "    Schema gedämpft: "
        << QString::number(kontrast(fenster.palette().color(QPalette::PlaceholderText), flaeche),
                           'f', 2)
        << ":1\n";

    out.flush();
    return 0;
}
