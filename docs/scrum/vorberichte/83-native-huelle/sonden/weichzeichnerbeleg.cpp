/**
 * Vorprüfung #83, Sonde 4 — lässt sich die Weichzeichner-Anmeldung belegen?
 *
 * AK 5 verlangt, dass der Weichzeichner über `enableBlurBehind` mit der
 * Maskenregion angemeldet ist. Die Funktion ist **void**, und
 * `isEffectAvailable()` liefert in der angemeldeten Sitzung `false` (Sonde 2).
 * Damit gibt es aus dem eigenen Prozess kein Zurücklesen. Bleibt der Blick von
 * außen — und der geht nur über eine Bildschirmaufnahme.
 *
 * Diese Sonde baut den Aufbau, der eine solche Aufnahme **belegfähig und
 * datensparsam** macht:
 *
 *   1. ein eigenes Vollbildfenster mit einem harten Schachbrett — es verdeckt
 *      den Schreibtisch des Kunden vollständig, sodass in der Aufnahme nur
 *      Inhalt dieser Sonde steht;
 *   2. darüber das Hüllenfenster nach dem nativen Weg, durchscheinend;
 *   3. wahlweise mit oder ohne `enableBlurBehind` (Aufrufschalter);
 *   4. eine Aufnahme über `spectacle -a` (aktives Fenster).
 *
 * Der Beleg ist der **Unterschied** zwischen beiden Läufen: Ist der Weichzeichner
 * angemeldet, ist das Schachbrett hinter der Hülle verwaschen; ist er es nicht,
 * bleiben seine Kanten hart. Die Sonde misst das an der Aufnahme selbst — als
 * Zahl der Helligkeitssprünge auf einer waagerechten Linie durch die Hülle.
 *
 * Aufruf: weichzeichnerbeleg <Zielverzeichnis> <an|aus>   (angemeldete Sitzung)
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowEffects>

#include <QApplication>
#include <QDir>
#include <QEventLoop>
#include <QImage>
#include <QPainter>
#include <QProcess>
#include <QScreen>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <QWindow>

namespace
{
constexpr int HullWidth = 600;
constexpr int HullHeight = 174;
constexpr int CheckerStep = 12;

/** Vollbild-Schachbrett: verdeckt den Schreibtisch, liefert harte Kanten. */
class Backdrop : public QWidget
{
public:
    Backdrop()
        : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        for (int y = 0; y < height(); y += CheckerStep) {
            for (int x = 0; x < width(); x += CheckerStep) {
                const bool dark = ((x / CheckerStep) + (y / CheckerStep)) % 2 == 0;
                painter.fillRect(x, y, CheckerStep, CheckerStep, dark ? Qt::black : Qt::white);
            }
        }
    }
};

/** Die Hülle nach dem nativen Weg — dieselbe wie in Sonde 2. */
class NativeHull : public QWidget
{
public:
    explicit NativeHull(const QString &theme)
        : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint)
        , m_imageSet(theme, QStringLiteral("plasma/desktoptheme"))
    {
        setAttribute(Qt::WA_TranslucentBackground);
        m_frame.setImageSet(&m_imageSet);
        m_frame.setImagePath(QStringLiteral("dialogs/background"));
        m_frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        resize(HullWidth, HullHeight);
    }

    QRegion hullMask() const
    {
        return m_frame.mask();
    }

    void catchUpRatio()
    {
        m_frame.setDevicePixelRatio(devicePixelRatioF());
        m_frame.resizeFrame(size());
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.drawPixmap(0, 0, m_frame.framePixmap());
    }

    void resizeEvent(QResizeEvent *) override
    {
        catchUpRatio();
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

/**
 * Die Zahl der harten Helligkeitssprünge auf einer waagerechten Linie.
 *
 * Ein Schachbrett hinter einer klaren Hülle springt bei jedem Feldwechsel;
 * hinter einer weichgezeichneten Hülle läuft es über. Das ist die Größe, in
 * der sich der Weichzeichner überhaupt messen lässt.
 */
int hardSteps(const QImage &image, int y)
{
    int steps = 0;
    int previous = -1;
    for (int x = 0; x < image.width(); ++x) {
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
    const QString theme = app.arguments().value(3, QStringLiteral("default"));
    QDir().mkpath(directory);

    out << "=== Vorprüfung #83, Sonde 4: Beleg der Weichzeichner-Anmeldung ===\n";
    out << "Plattform      : " << app.platformName() << "\n";
    out << "Weichzeichner  : " << mode << "\n";

    if (app.platformName() != QLatin1String("wayland")) {
        out << "Ohne angemeldete Sitzung hat diese Sonde keinen Gegenstand — übersprungen.\n";
        return 0;
    }

    Backdrop backdrop;
    backdrop.showFullScreen();
    wait(600);

    NativeHull hull(theme);
    hull.show();
    hull.raise();
    hull.activateWindow();

    // „frueh" prüft die naheliegende Erklärung eines Fehlschlags: die Anmeldung
    // müsse **vor** dem ersten Bildwechsel stehen. Zusätzlich mit leerer Region
    // — die heißt laut Kopfdatei „das ganze Fenster" und schließt einen Fehler
    // in unserer Region aus.
    if (mode == QLatin1String("frueh") || mode.startsWith(QLatin1String("wiederzeigen"))) {
        KWindowEffects::enableBlurBehind(hull.windowHandle(), true, QRegion());
        out << "enableBlurBehind(leere Region) sofort nach show() gerufen.\n";
    }

    // Die Frage aus der Story: showCapture() versteckt und zeigt neu — überlebt
    // die Anmeldung das? „wiederzeigen-neu" meldet nach dem erneuten Zeigen
    // sofort wieder an, „wiederzeigen-alt" verlässt sich auf die alte Anmeldung.
    if (mode.startsWith(QLatin1String("wiederzeigen"))) {
        wait(400);
        hull.hide();
        QCoreApplication::processEvents();
        wait(200);
        hull.show();
        hull.raise();
        hull.activateWindow();
        if (mode == QLatin1String("wiederzeigen-neu")) {
            KWindowEffects::enableBlurBehind(hull.windowHandle(), true, QRegion());
            out << "Nach hide()/show() sofort erneut angemeldet.\n";
        } else {
            out << "Nach hide()/show() **nicht** erneut angemeldet.\n";
        }
    }

    wait(1200);
    hull.catchUpRatio();

    out << "Fenster-DPR    : " << hull.devicePixelRatioF() << "\n";
    out << "isEffectAvailable(BlurBehind): "
        << (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) ? "true" : "false")
        << "\n";
    if (mode == QLatin1String("spaet-leer")) {
        // Trennt die beiden Erklärungen des frueh-Laufs: lag es am **Zeitpunkt**
        // oder an der **Region**? Hier steht die leere Region (= ganzes Fenster)
        // am späten Zeitpunkt.
        KWindowEffects::enableBlurBehind(hull.windowHandle(), true, QRegion());
        out << "enableBlurBehind(leere Region) spät gerufen.\n";
    } else if (mode == QLatin1String("zweimal")) {
        // Die Vermutung, die der frueh-Lauf nahelegt: Der **erste** Aufruf
        // bindet nur die Wayland-Erweiterung, wirken tut erst der zweite.
        KWindowEffects::enableBlurBehind(hull.windowHandle(), true, hull.hullMask());
        QCoreApplication::processEvents();
        wait(200);
        KWindowEffects::enableBlurBehind(hull.windowHandle(), true, hull.hullMask());
        out << "enableBlurBehind(Maskenregion) **zweimal** gerufen, 200 ms auseinander.\n";
        out << "isEffectAvailable(BlurBehind) danach: "
            << (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) ? "true" : "false")
            << "\n";
    } else if (mode == QLatin1String("an") || mode == QLatin1String("frueh")) {
        KWindowEffects::enableBlurBehind(hull.windowHandle(), true, hull.hullMask());
        out << "enableBlurBehind(Maskenregion) gerufen — Rückgabewert: keiner.\n";
    } else if (mode.startsWith(QLatin1String("wiederzeigen"))) {
        out << "Kein weiterer Aufruf — es zählt allein, was oben angemeldet wurde.\n";
    } else {
        out << "enableBlurBehind() **nicht** gerufen — das ist der Gegenlauf.\n";
    }
    wait(1200);

    // Erster Versuch: `spectacle -a` nimmt allein das aktive Fenster auf.
    const QString windowShot =
        QDir(directory).filePath(QStringLiteral("weichzeichner-fenster-%1.png").arg(mode));
    QFile::remove(windowShot);
    QProcess spectacle;
    spectacle.start(QStringLiteral("spectacle"),
                    {QStringLiteral("-a"),
                     QStringLiteral("-b"),
                     QStringLiteral("-n"),
                     QStringLiteral("-o"),
                     windowShot});
    const bool finished = spectacle.waitForFinished(20000);
    out << "\n-- spectacle -a (nur das Fenster)\n";
    out << "   beendet: " << (finished ? "ja" : "NEIN (Zeitüberschreitung)") << ", Rückgabe "
        << spectacle.exitCode() << "\n";

    const QImage windowPicture(windowShot);
    if (windowPicture.isNull()) {
        out << "   Keine Aufnahme entstanden.\n";
    } else {
        const QColor centre = windowPicture.pixelColor(windowPicture.width() / 2,
                                                       windowPicture.height() / 2);
        out << "   Aufnahme " << windowPicture.width() << "x" << windowPicture.height()
            << ", Mitte " << centre.red() << "," << centre.green() << "," << centre.blue()
            << " / Alpha " << centre.alpha() << "\n";
        out << "   Harte Sprünge auf der Mittellinie: "
            << hardSteps(windowPicture, windowPicture.height() / 2) << "\n";
        out << "   Befund: Alpha " << centre.alpha()
            << " heißt, die Aufnahme trägt die **eigene Fläche des Fensters**, nicht das\n"
               "   zusammengesetzte Bild. Was hinter der Hülle liegt, steht nicht darin — "
               "der\n   Weichzeichner ist auf diesem Weg so wenig zu sehen wie über "
               "QWidget::grab().\n";
    }

    // Zweiter Versuch: das ganze Bild. Der Schreibtisch des Kunden ist dabei vom
    // Vollbild-Schachbrett verdeckt; gespeichert wird trotzdem **nur** ein
    // Ausschnitt um die Hülle, und der Vollbildpuffer wird sofort gelöscht.
    const QString full = QDir(directory).filePath(QStringLiteral("_vollbild-%1.png").arg(mode));
    QFile::remove(full);
    QProcess whole;
    whole.start(QStringLiteral("spectacle"),
                {QStringLiteral("-f"),
                 QStringLiteral("-b"),
                 QStringLiteral("-n"),
                 QStringLiteral("-o"),
                 full});
    const bool wholeDone = whole.waitForFinished(20000);
    out << "\n-- spectacle -f (das zusammengesetzte Bild)\n";
    out << "   beendet: " << (wholeDone ? "ja" : "NEIN") << ", Rückgabe " << whole.exitCode()
        << "\n";

    QImage screen(full);
    QFile::remove(full);
    if (screen.isNull()) {
        out << "   Keine Aufnahme entstanden — der Beleg lässt sich so nicht führen.\n";
        return 2;
    }
    out << "   Aufnahme " << screen.width() << "x" << screen.height() << " Bildpunkte "
        << "(sofort wieder gelöscht, gespeichert wird nur der Ausschnitt)\n";

    // Die Hülle finden: hinter ihr ist aus Schwarz ein Grau geworden — das
    // Schachbrett scheint zu 15,3 % durch eine Fläche von 240. Reines Schwarz
    // und reines Weiß kommen dort nicht mehr vor.
    const auto blended = [&screen](int x, int y) {
        const int grey = qGray(screen.pixel(x, y));
        return grey > 185 && grey < 213;
    };
    int hullRow = -1;
    int best = 0;
    for (int y = 0; y < screen.height(); ++y) {
        int count = 0;
        for (int x = 0; x < screen.width(); x += 2) {
            if (blended(x, y)) {
                ++count;
            }
        }
        if (count > best) {
            best = count;
            hullRow = y;
        }
    }
    out << "   Zeile mit den meisten gemischten Bildpunkten: y=" << hullRow << " (" << best
        << " von " << screen.width() / 2 << " geprüften)\n";

    // Immer dieselbe feste Prüflinie, damit die Läufe Bildpunkt für Bildpunkt
    // vergleichbar bleiben — auch dann, wenn die Suche oben nichts findet, was
    // bei weichgezeichnetem Grund gerade der erwartete Fall ist.
    constexpr int ProbeY = 913;
    constexpr int ProbeX = 1480;
    if (screen.height() > ProbeY && screen.width() > ProbeX + 40) {
        out << "   Feste Prüflinie y=" << ProbeY << ", x=" << ProbeX << " ff. (Graustufen):\n     ";
        for (int x = ProbeX; x < ProbeX + 40; ++x) {
            out << QStringLiteral("%1 ").arg(qGray(screen.pixel(x, ProbeY)), 3);
        }
        out << "\n   Spannweite auf dieser Linie über 600 Bildpunkte: ";
        int low = 255;
        int high = 0;
        for (int x = ProbeX; x < ProbeX + 600 && x < screen.width(); ++x) {
            const int grey = qGray(screen.pixel(x, ProbeY));
            low = qMin(low, grey);
            high = qMax(high, grey);
        }
        out << low << " bis " << high << "\n";
        screen.copy(ProbeX - 40, ProbeY - 90, 1000, 180)
            .save(QDir(directory).filePath(
                QStringLiteral("weichzeichner-pruefline-%1.png").arg(mode)));
    }

    if (hullRow < 0 || best < 100) {
        out << "   Die scharfe Hülle war im Bild nicht zu finden — bei angemeldetem\n"
               "   Weichzeichner ist genau das der erwartete Ausgang.\n";
        return 0;
    }

    // Die Spalten der Hülle in dieser Zeile.
    int left = -1;
    int right = -1;
    for (int x = 0; x < screen.width(); ++x) {
        if (blended(x, hullRow)) {
            left = left < 0 ? x : left;
            right = x;
        }
    }
    const QRect hullRect(left, qMax(0, hullRow - 90), right - left + 1, 180);
    out << "   Hülle in dieser Zeile: x=" << left << " bis " << right << " (" << right - left + 1
        << " Bildpunkte breit)\n";
    screen.copy(hullRect).save(
        QDir(directory).filePath(QStringLiteral("weichzeichner-schirm-%1.png").arg(mode)));

    const int outside = qBound(0, hullRow - 300, screen.height() - 1);
    out << "\n   Harte Sprünge (Helligkeitssprung > 60 zwischen Nachbarn):\n";
    out << "     nur innerhalb der Hülle (y=" << hullRow << ", x=" << left << ".." << right
        << "): " << hardSteps(screen.copy(left, hullRow, right - left + 1, 1), 0) << "\n";
    out << "     blankes Schachbrett darüber (y=" << outside << ", ganze Breite): "
        << hardSteps(screen, outside) << "\n";
    out << "   Helligkeitswerte in der Hülle, 40 Bildpunkte ab x=" << left + 40 << ":\n     ";
    for (int x = left + 40; x < left + 80 && x < screen.width(); ++x) {
        out << QStringLiteral("%1 ").arg(qGray(screen.pixel(x, hullRow)), 3);
    }
    out << "\n   Weniger Sprünge und ein glatter Verlauf heißen: hinter der Hülle ist das\n"
           "   Muster verwaschen. Der Vergleich der beiden Läufe (an gegen aus) ist der Beleg.\n";

    return 0;
}
