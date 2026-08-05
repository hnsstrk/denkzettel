/**
 * Messsonde 3 zu #83 — wirkt der Weichzeichner? (AK 5, AK 6)
 *
 * `enableBlurBehind()` und `enableBackgroundContrast()` sind **void**, und der
 * einzige lesbare Rückgabewert — `isEffectAvailable()` — lügt vor der ersten
 * Anmeldung. Aus dem eigenen Prozess ist also nichts zurückzulesen. Bleibt der
 * Blick von außen, und der geht nur über eine Aufnahme des **zusammengesetzten
 * Bildes**: `spectacle -a` (nur das Fenster) liefert wie `QWidget::grab()` die
 * eigene Fläche, in der von hinter der Hülle nichts steht.
 *
 * Aufbau, datensparsam:
 *   1. ein eigenes Vollbild-Schachbrett verdeckt den Schreibtisch vollständig;
 *   2. darüber das **echte** Erfassungsfenster (Lauf `an`) oder ein bloßes
 *      Widget, das dieselbe Theme-Grafik zeichnet und **nichts** anmeldet
 *      (Lauf `aus`) — das ist der Gegenlauf;
 *   3. `spectacle -f`, Vollbildpuffer sofort löschen, nur den Ausschnitt um die
 *      Hülle behalten.
 *
 * Der Beleg ist der Unterschied: hinter einer angemeldeten Hülle läuft das
 * Schachbrett über, hinter einer nicht angemeldeten bleiben seine Kanten hart.
 * Gemessen als Zahl der harten Helligkeitssprünge und als Spannweite auf einer
 * festen Prüflinie durch die Hülle.
 *
 * Der Lauf `groesse` beantwortet die offene Frage von AK 5: Ändert eine
 * **späte** Anmeldung die Region eines bereits laufenden Weichzeichners? Er
 * wächst das Fenster nach der ersten Anmeldung auf acht Zeilen und misst, ob
 * der untere Teil mitverwischt.
 *
 * Zwei weitere Läufe brauchen dieselbe Maschinerie — Untergrund, Aufnahme,
 * Ausschnitt — und stehen deshalb hier statt in einer eigenen Sonde:
 *
 *   `ecke`     AK 13: Steht am Bogen ein Bildpunkt, der heller ist als der
 *              ungeschattete Grund? Untergrund ist dann eine glatte Fläche.
 *   `krunner`  AK 12: das Erfassungsfenster und KRunner nebeneinander. Die
 *              **Beurteilung** dieses Bildes ist Sache der Kundenabnahme; diese
 *              Sonde beschafft nur die Grundlage.
 *
 * Aufruf: weichzeichner <Zielverzeichnis> <an|aus|groesse|wiederzeigen|ecke|krunner>
 *         (nur in der angemeldeten Sitzung sinnvoll)
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QImage>
#include <QPainter>
#include <QDBusInterface>
#include <QPlainTextEdit>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QWidget>

namespace
{
constexpr int CheckerStep = 12;

/** Der Grund des Laufs `ecke`: eine Fläche ohne jede Struktur. */
const QColor PlainGround(128, 128, 128);

/**
 * Vollbild-Untergrund: verdeckt den Schreibtisch des Kunden vollständig, sodass
 * in der Aufnahme nur Inhalt dieser Sonde steht.
 *
 * Schachbrett für die Weichzeichner-Läufe (harte Kanten, an denen sich ein
 * Weichzeichner überhaupt messen lässt), eine glatte Fläche für den Lauf
 * `ecke` — dort ist jede Struktur im Grund eine Störung.
 */
class Backdrop : public QWidget
{
public:
    explicit Backdrop(bool plain)
        : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint)
        , m_plain(plain)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        if (m_plain) {
            painter.fillRect(rect(), PlainGround);
            return;
        }
        for (int y = 0; y < height(); y += CheckerStep) {
            for (int x = 0; x < width(); x += CheckerStep) {
                const bool dark = ((x / CheckerStep) + (y / CheckerStep)) % 2 == 0;
                painter.fillRect(x, y, CheckerStep, CheckerStep, dark ? Qt::black : Qt::white);
            }
        }
    }

private:
    bool m_plain;
};

/**
 * Der Lauf `ecke` (AK 13): Steht am Bogen ein Bildpunkt, der **heller** ist als
 * der ungeschattete Grund weit draußen?
 *
 * Das war der Kernsatz des Kunden zur Sprint-6-Abnahme — „der weiße
 * Hintergrund ragt über den Rahmen hinaus". Messbar ist er nur an einer
 * Aufnahme des Bildschirms: Schatten und Grund liegen außerhalb des Fensters,
 * `QWidget::grab()` sieht beide nie.
 */
int cornerReport(QTextStream &out, QImage &screen, const QString &directory, int left, int top)
{
    out << "Linke obere Ecke der Fläche: x=" << left << ", y=" << top << "\n";

    const int groundX = qMax(0, left - 250);
    const int groundY = qMax(0, top - 250);
    const int ground = qGray(screen.pixel(groundX, groundY));
    out << "Ungeschatteter Grund weit draußen (x=" << groundX << ", y=" << groundY
        << "): " << ground << "\n\n";

    out << "Waagerechte Schnitte durch den Bogen, von außen nach innen:\n";
    int brightest = 0;
    int brightestX = 0;
    int brightestY = 0;
    for (int y = top; y < top + 8 && y < screen.height(); ++y) {
        QStringList values;
        for (int x = qMax(0, left - 20); x < left + 12 && x < screen.width(); ++x) {
            const int grey = qGray(screen.pixel(x, y));
            values << QStringLiteral("%1").arg(grey, 4);
            if (grey > brightest) {
                brightest = grey;
                brightestX = x;
                brightestY = y;
            }
        }
        out << QStringLiteral("  Zeile %1: %2\n").arg(y, 5).arg(values.join(QString()));
    }

    out << "\nHellster Bildpunkt am Bogen: " << brightest << " bei x=" << brightestX << ", y="
        << brightestY << "\n";
    out << "Grund außerhalb des Schattens: " << ground << "\n";
    if (brightest > ground) {
        out << "**BEFUND: Am Bogen steht ein Bildpunkt, der heller ist als der Grund "
               "(+"
            << brightest - ground
            << "). Die bisherige Erklärung ist damit widerlegt; der Befund geht als eigenes\n"
               "Issue an den Product Owner zurück und blockiert die Abnahme dieser Story "
               "nicht.**\n";
    } else {
        out << "Befund: Kein Bildpunkt am Bogen ist heller als der Grund ("
            << brightest - ground << "). AK 13 ist erfüllt.\n";
    }

    screen.copy(qMax(0, left - 40), qMax(0, top - 40), 120, 120)
        .scaled(600, 600, Qt::KeepAspectRatio, Qt::FastTransformation)
        .save(QDir(directory).filePath(QStringLiteral("ecke-am-bildschirm.png")));
    screen.copy(qMax(0, left - 60), qMax(0, top - 60), 1100, 420)
        .save(QDir(directory).filePath(QStringLiteral("fenster-am-bildschirm.png")));
    return 0;
}

/**
 * Der Gegenlauf: dieselbe Grafik, dieselbe Größe, **keine** Anmeldung.
 *
 * Er ist bewusst kein Erfassungsfenster: Das Erfassungsfenster meldet immer an,
 * und einen Schalter dafür gäbe es im Erzeugnis nur für diese Messung. Was der
 * Vergleich damit trennt, ist die Anmeldung — nicht die Zeichnung.
 */
class UnregisteredHull : public QWidget
{
public:
    UnregisteredHull(const QString &theme, const QSize &size)
        : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint)
        , m_imageSet(theme, QStringLiteral("plasma/desktoptheme"))
    {
        setAttribute(Qt::WA_TranslucentBackground);
        m_frame.setColorSet(KSvg::Svg::Window);
        m_frame.setImageSet(&m_imageSet);
        m_frame.setImagePath(QStringLiteral("dialogs/background"));
        m_frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        resize(size);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.drawPixmap(0, 0, m_frame.framePixmap());
    }

    void resizeEvent(QResizeEvent *) override
    {
        m_frame.setDevicePixelRatio(devicePixelRatioF());
        m_frame.resizeFrame(size());
    }

private:
    KSvg::ImageSet m_imageSet;
    KSvg::FrameSvg m_frame;
};

void wait(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

/** Zahl der harten Helligkeitssprünge auf einer waagerechten Linie. */
int hardSteps(const QImage &image, int y, int from, int to)
{
    int steps = 0;
    int previous = -1;
    for (int x = from; x < to && x < image.width(); ++x) {
        const int value = qGray(image.pixel(x, y));
        if (previous >= 0 && qAbs(value - previous) > 60) {
            ++steps;
        }
        previous = value;
    }
    return steps;
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    const QString directory = app.arguments().value(1, QStringLiteral("."));
    const QString mode = app.arguments().value(2, QStringLiteral("an"));
    QDir().mkpath(directory);

    out << "=== #83, Sonde 3: wirkt der Weichzeichner? ===\n";
    out << "Plattform     : " << app.platformName() << "\n";
    out << "Lauf          : " << mode << "\n";

    if (app.platformName() != QLatin1String("wayland")) {
        out << "Ohne angemeldete Sitzung hat diese Sonde keinen Gegenstand — übersprungen.\n";
        return 0;
    }
    out << "Weichzeichnende Sitzung: "
        << (capture::sessionBlursBehindWindows() ? "ja" : "nein") << "\n";

    const bool plainGround = mode == QLatin1String("ecke") || mode == QLatin1String("krunner");
    Backdrop backdrop(plainGround);
    backdrop.showFullScreen();
    wait(800);

    const QTemporaryDir tmp;
    Store store(tmp.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen\n";
        return 1;
    }

    QSize hullSize;
    std::unique_ptr<CaptureWindow> window;
    std::unique_ptr<UnregisteredHull> plain;

    if (mode == QLatin1String("aus")) {
        // Erst das echte Fenster bauen, um seine Maße zu erfahren, dann es
        // wieder fallenlassen: Der Gegenlauf soll dieselbe Fläche haben.
        CaptureWindow sizer(&store);
        hullSize = sizer.size();
        plain = std::make_unique<UnregisteredHull>(QStringLiteral("default"), hullSize);
        plain->show();
        plain->raise();
        plain->activateWindow();
        out << "Gegenlauf: dieselbe Grafik, **keine** Anmeldung.\n";
    } else {
        window = std::make_unique<CaptureWindow>(&store);
        window->showCapture();
        hullSize = window->size();
        out << "Das echte Erfassungsfenster, showCapture() — Anmeldung steht in present().\n";
    }

    wait(1200);

    if (mode == QLatin1String("wiederzeigen") && window) {
        // Jedes Zeigen zerstört die Wayland-Oberfläche und mappt eine neue. Was
        // an der alten hing, ist damit fort — der Lauf prüft, ob die Anmeldung
        // das zweite Öffnen überlebt.
        window->hide();
        QCoreApplication::processEvents();
        wait(300);
        window->showCapture();
        wait(1200);
        out << "Nach hide()/showCapture(): zweite Öffnung.\n";
    }

    if (mode == QLatin1String("groesse") && window) {
        // Die offene Frage von AK 5: ändert eine **späte** Anmeldung die Region
        // eines laufenden Weichzeichners?
        window->findChild<QPlainTextEdit *>()->setPlainText(
            QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
        QCoreApplication::processEvents();
        wait(1200);
        hullSize = window->size();
        out << "Auf acht Zeilen gewachsen: " << hullSize.width() << "x" << hullSize.height()
            << " logisch.\n";
    }

    if (mode == QLatin1String("krunner") && window) {
        // Der Vergleich, den kein Prüfmittel dieses Projekts führen kann: unser
        // Fenster neben einer Überlagerung, die Plasma selbst zeichnet — aus
        // derselben Grafik `dialogs/background`. KRunner legt sich über
        // gewöhnliche Fenster, der Untergrund dieser Sonde bleibt darunter.
        window->findChild<QPlainTextEdit *>()->setPlainText(
            QStringLiteral("Sieht das aus wie ein Fenster von Plasma?"));
        QDBusInterface(QStringLiteral("org.kde.krunner"),
                       QStringLiteral("/App"),
                       QStringLiteral("org.kde.krunner.App"))
            .call(QStringLiteral("display"));
        wait(2000);
        out << "KRunner über org.kde.krunner.App.display() geöffnet.\n";
    }

    const QString full = QDir(directory).filePath(QStringLiteral("_vollbild-%1.png").arg(mode));
    QFile::remove(full);
    QProcess spectacle;
    spectacle.start(QStringLiteral("spectacle"),
                    {QStringLiteral("-f"),
                     QStringLiteral("-b"),
                     QStringLiteral("-n"),
                     QStringLiteral("-o"),
                     full});
    const bool done = spectacle.waitForFinished(20000);
    out << "\nspectacle -f beendet: " << (done ? "ja" : "NEIN") << ", Rückgabe "
        << spectacle.exitCode() << "\n";

    QImage screen(full);
    QFile::remove(full);
    if (screen.isNull()) {
        out << "Keine Aufnahme entstanden — der Beleg lässt sich so nicht führen.\n";
        return 2;
    }
    out << "Aufnahme " << screen.width() << "x" << screen.height()
        << " Bildpunkte (Vollbildpuffer sofort gelöscht, gespeichert wird nur der "
           "Ausschnitt)\n";

    if (mode == QLatin1String("krunner")) {
        // Gespeichert wird der kleinste Ausschnitt, der beide Fenster enthält:
        // alles, was nicht der glatte Untergrund dieser Sonde ist. Vom
        // Schreibtisch des Kunden kommt so kein Bildpunkt mit — **bis auf die
        // Kontrollleisten**, und die sind der Grund für den Filter unten: Ein
        // Vollbildfenster liegt in Plasma unter ihnen, sie stehen also im Bild.
        // Gemessen an diesem Lauf: drei Bereiche, zwei davon rund 1.000
        // Bildpunkte breit (KRunner und wir), einer über die volle Bildbreite
        // (die Leiste). Zeilen, die fast die ganze Breite füllen, gehören
        // deshalb keinem Fenster dieser Messung und bleiben draußen.
        QRect box;
        for (int y = 0; y < screen.height(); ++y) {
            int first = -1;
            int last = -1;
            for (int x = 0; x < screen.width(); ++x) {
                if (screen.pixelColor(x, y) != PlainGround) {
                    first = first < 0 ? x : first;
                    last = x;
                }
            }
            if (first < 0 || last - first > screen.width() * 9 / 10) {
                continue;
            }
            const QRect row(first, y, last - first + 1, 1);
            box = box.isNull() ? row : box.united(row);
        }
        if (box.isNull()) {
            out << "Nichts außer dem Untergrund gefunden — der Lauf trägt nichts.\n";
            return 3;
        }
        const QRect crop = box.adjusted(-30, -30, 30, 30).intersected(screen.rect());
        out << "Beide Fenster im Ausschnitt " << crop.width() << "x" << crop.height()
            << " (x=" << crop.x() << ", y=" << crop.y() << ")\n";
        screen.copy(crop).save(QDir(directory).filePath(QStringLiteral("fenster-neben-krunner.png")));
        out << "Die **Beurteilung** dieses Bildes ist Sache der Kundenabnahme (AK 12).\n";
        return 0;
    }

    // Die Hülle im Bild finden. Über dem Schachbrett kommen nur reines Schwarz
    // und reines Weiß vor, über der glatten Fläche des Laufs `ecke` nur die
    // 128; alles andere gehört dem Fenster — ob weichgezeichnet oder nicht.
    //
    // Gesucht wird der längste **zusammenhängende** solche Lauf je Zeile, und
    // er muss ungefähr so breit sein wie das Fenster. Die einfache Zählung
    // „meiste gemischte Bildpunkte" hat in einem ersten Lauf die graue
    // Kontrollleiste am Bildrand gefunden und in allen vier Läufen dieselbe
    // Zahl geliefert — eine Messung, die aussah wie ein Beleg und keiner war.
    // Dieselbe Falle noch einmal beim Lauf `ecke`: Der kleinste x- und der
    // kleinste y-Wert aller dunklen Bildpunkte ergaben eine „Ecke" bei (0, 904),
    // die nirgends am Fenster lag.
    const int expected = qRound(hullSize.width() * app.devicePixelRatio());
    // Die Schwelle für das Schachbrett liegt dicht an dessen beiden reinen
    // Werten, und das ist nötig: Das Farbschema des Kunden ist dunkel, hinter
    // einer zu 84,7 % deckenden Hülle wird aus Schwarz eine 25 und aus Weiß eine
    // 64. Bei einer Schwelle von 30 endete die Suche nach der Höhe schon nach 18
    // Bildpunkten — die Hülle galt dann als 18 statt als 278 hoch.
    const auto blended = [&screen, plainGround](int x, int y) {
        const int grey = qGray(screen.pixel(x, y));
        return plainGround ? grey < 100 : grey > 6 && grey < 250;
    };
    int hullRow = -1;
    int left = -1;
    int right = -1;
    int best = 0;
    for (int y = 0; y < screen.height(); ++y) {
        int runStart = -1;
        for (int x = 0; x <= screen.width(); ++x) {
            const bool inside = x < screen.width() && blended(x, y);
            if (inside && runStart < 0) {
                runStart = x;
            } else if (!inside && runStart >= 0) {
                const int length = x - runStart;
                // Nur Läufe in der Größenordnung des Fensters, und von denen
                // der längste: eine Leiste über die ganze Bildbreite ist keine
                // Hülle.
                if (length > best && length >= expected / 2 && length <= expected * 5 / 4) {
                    best = length;
                    hullRow = y;
                    left = runStart;
                    right = x - 1;
                }
                runStart = -1;
            }
        }
    }
    out << "Fenster erwartet " << expected << " Bildpunkte breit; längster passender Lauf: "
        << best << " Bildpunkte bei y=" << hullRow << ", x=" << left << ".." << right << "\n";

    if (hullRow < 0 || right - left < expected / 2) {
        out << "Die Hülle war im Bild nicht zu finden — der Lauf trägt nichts.\n";
        return 3;
    }

    // Wie hoch die Hülle im Bild steht: von der gefundenen Zeile aus nach oben
    // und unten, solange die Mitte der Spalte noch dem Fenster gehört. Ohne
    // diesen Schritt reichte das Messband unten über den Rand hinaus und nahm
    // das blanke Schachbrett mit — dann steht dort 0 bis 255, und der Vergleich
    // der Läufe ist wertlos (in einem ersten Lauf genau so passiert).
    const int middle = (left + right) / 2;
    int top = hullRow;
    while (top > 0 && blended(middle, top - 1)) {
        --top;
    }
    int bottom = hullRow;
    while (bottom < screen.height() - 1 && blended(middle, bottom + 1)) {
        ++bottom;
    }
    out << "Hülle im Bild: " << right - left + 1 << "x" << bottom - top + 1 << " Bildpunkte (y="
        << top << ".." << bottom << ")\n";

    if (plainGround) {
        return cornerReport(out, screen, directory, left, top);
    }

    out << "Harte Sprünge (Helligkeitssprung > 60 zwischen Nachbarn):\n";
    out << "  innerhalb der Hülle (y=" << hullRow << ", x=" << left + 40 << ".." << right - 40
        << "): " << hardSteps(screen, hullRow, left + 40, right - 40) << "\n";
    const int outside = qBound(0, hullRow - 300, screen.height() - 1);
    out << "  blankes Schachbrett darüber (y=" << outside << ", gleiche Breite): "
        << hardSteps(screen, outside, left + 40, right - 40) << "\n";

    // Die Spannweite ist die Größe, in der sich der Weichzeichner hier
    // überhaupt messen lässt: Hinter einer klaren Hülle scheint das Schachbrett
    // durch und die Helligkeit springt zwischen zwei Werten; hinter einer
    // weichgezeichneten ist alles derselbe Mittelwert.
    //
    // Gemessen wird der **linke Innenstreifen** der Hülle, nicht ihre Mitte:
    // Der Innenabstand der Zeichnung 4b hält dort 16 logische Bildpunkte frei,
    // in der Mitte steht Text. Über Text gemessen kommt die Spannweite von den
    // Buchstaben und nicht vom Grund — der Vergleich der Läufe verglich dann
    // Schrift mit Schrift (in einem ersten Lauf genau so passiert: 196 gegen
    // 39, und die 196 waren die Schrift).
    const int stripeLeft = left + 8;
    const int stripeRight = left + 26;
    const int bandTop = top + 40;
    const int bandBottom = bottom - 40;
    int low = 255;
    int high = 0;
    for (int y = bandTop; y <= bandBottom; ++y) {
        for (int x = stripeLeft; x <= stripeRight && x < screen.width(); ++x) {
            const int grey = qGray(screen.pixel(x, y));
            low = qMin(low, grey);
            high = qMax(high, grey);
        }
    }
    out << "Spannweite im linken Innenstreifen (x=" << stripeLeft << ".." << stripeRight << ", y="
        << bandTop << ".." << bandBottom << "): " << low << " bis " << high << " ("
        << high - low << ")\n";
    out << "Lesart: Viele Sprünge und eine weite Spannweite heißen scharf, wenige und eine\n"
           "enge heißen verwaschen. Der Beleg ist der Vergleich `an` gegen `aus`.\n";

    screen.copy(left - 20, qMax(0, hullRow - 120), right - left + 40, 240)
        .save(QDir(directory).filePath(QStringLiteral("weichzeichner-%1.png").arg(mode)));

    return 0;
}
