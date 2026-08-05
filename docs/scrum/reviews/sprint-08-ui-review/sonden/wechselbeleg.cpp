/*
 * UI-Review Sprint 8 — die beiden Übergänge der Story #85 **im Bild**.
 *
 * AK 5 (Theme-Wechsel) und AK 7 (Schemawechsel) sind Zusicherungen über einen
 * Weg, nicht über einen Zustand: An **einem stehenden Fenster** soll die
 * Textfarbe dem neuen Desktop-Theme folgen — und beim Wechsel des *Farbschemas*
 * gerade nicht mitwandern, solange das Theme eine eigene `colors`-Datei hat.
 *
 * Der Strang hat beides über Prüfsätze belegt. Was hier dazukommt, ist der
 * Sichtbeleg: dieselbe Folge an einem Fenster in der angemeldeten Sitzung, vier
 * Aufnahmen, dazwischen kein Neuanlegen. Ein Fenster je Zustand — wie es die
 * Belegsonde des Strangs baut — könnte die Zusicherung nicht von einer
 * Umsetzung unterscheiden, die die Farbe **nur beim Anlegen** richtig setzt.
 *
 * Das Farbschema wird über `qApp->setPalette()` gewechselt, wie im Prüfsatz
 * `themeTextColoursOutlastAColourSchemeChange()`. Die Einstellungen des Kunden
 * werden dabei nicht angefasst — diese Sonde schreibt nichts nach `~/.config`.
 * Als Ersatzschema dient ein grelles Magenta: Ein Fehler an dieser Stelle wäre
 * dann im Bild nicht zu übersehen, während ein zweites glaubwürdiges Grau ihn
 * verstecken könnte.
 *
 * Aufruf: wechselbeleg <Zielordner>
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
#include <QTextCursor>
#include <QTextStream>
#include <QTimer>
#include <QWidget>

namespace
{
QTextStream out(stdout);

QString farbe(const QColor &c)
{
    return QStringLiteral("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue());
}

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

/**
 * Das Fenster im Vollbild, gefunden über den Unterschied zweier Aufnahmen.
 *
 * Warum überhaupt gesucht wird, obwohl das Fenster seine Geometrie kennt: Unter
 * Wayland meldet `QWidget::x()/y()` **nicht** die Lage auf dem Bildschirm. Der
 * erste Lauf dieser Sonde hat aus `0,0` geschnitten und vier weiße Bilder
 * abgelegt — Aufnahmen, die entstanden sind, etwas zeigen und nichts belegen.
 * Die Größe dagegen ist von außen bekannt und dient als Erkennungsmerkmal; das
 * Verfahren ist das der Belegsonde des Strangs (dort ausführlich begründet).
 */
QRect fensterImBild(const QImage &ohne, const QImage &mit, const QSize &erwartet)
{
    const int b = mit.width();
    const int h = mit.height();
    QList<bool> offen(qsizetype(b) * h, false);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < b; ++x) {
            const QColor c1 = ohne.pixelColor(x, y);
            const QColor c2 = mit.pixelColor(x, y);
            offen[qsizetype(y) * b + x] = qAbs(c1.red() - c2.red()) + qAbs(c1.green() - c2.green())
                    + qAbs(c1.blue() - c2.blue())
                > 12;
        }
    }

    QRect bester;
    for (int y0 = 0; y0 < h; ++y0) {
        for (int x0 = 0; x0 < b; ++x0) {
            if (!offen.at(qsizetype(y0) * b + x0)) {
                continue;
            }
            QList<QPoint> stapel{QPoint(x0, y0)};
            offen[qsizetype(y0) * b + x0] = false;
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
                    if (n.x() < 0 || n.y() < 0 || n.x() >= b || n.y() >= h) {
                        continue;
                    }
                    const qsizetype i = qsizetype(n.y()) * b + n.x();
                    if (offen.at(i)) {
                        offen[i] = false;
                        stapel.append(n);
                    }
                }
            }
            const QRect teil(links, oben, rechts - links + 1, unten - oben + 1);
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
    if (a.size() < 2) {
        out << "Aufruf: wechselbeleg <Zielordner>\n";
        out.flush();
        return 2;
    }
    const QString ordner = a.at(1);
    QDir().mkpath(ordner);

    out << "=== UI-Review Sprint 8 — Theme- und Schemawechsel am stehenden Fenster ===\n";
    out << "Plattform: " << app.platformName() << "\n";
    if (app.platformName() != QLatin1String("wayland")) {
        out << "Ohne angemeldete Sitzung hat diese Sonde keinen Gegenstand.\n";
        out.flush();
        return 0;
    }

    QTemporaryDir fluechtig;
    Grund grund(QColor(255, 255, 255));
    grund.resize(app.primaryScreen()->size());
    grund.show();
    warte(900);

    const QImage ohne = vollbild(fluechtig.filePath(QStringLiteral("_ohne.png")));
    if (ohne.isNull()) {
        out << "Keine Aufnahme entstanden — der Beleg lässt sich so nicht führen.\n";
        out.flush();
        return 2;
    }

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store nicht offen: " << store.lastError() << "\n";
        out.flush();
        return 1;
    }

    CaptureWindow fenster(&store);
    fenster.reloadDesktopTheme(QStringLiteral("default"));
    auto *feld = fenster.findChild<QPlainTextEdit *>();
    const QList<QLabel *> kleintexte = fenster.findChildren<QLabel *>();
    if (!feld || kleintexte.isEmpty()) {
        out << "Fenster ohne Textfeld oder ohne Kleintexte — abgebrochen.\n";
        out.flush();
        return 1;
    }
    fenster.showCapture();
    feld->setPlainText(QStringLiteral("Denkzettel — Lesbarkeit unter fremden Themes"));
    warte(1600);

    const QSize erwartet(qRound(fenster.width() * fenster.devicePixelRatioF()),
                         qRound(fenster.height() * fenster.devicePixelRatioF()));
    out << "Fenster " << fenster.width() << "x" << fenster.height() << " bei Verhältnis "
        << fenster.devicePixelRatioF() << ", erwartete Kantenlänge " << erwartet.width() << "x"
        << erwartet.height() << "\n\n";

    // Das Fenster wird **einmal** gesucht und steht die ganze Folge über an
    // derselben Stelle — es soll sich ja gerade nichts bewegen. Alle vier
    // Ausschnitte kommen deshalb aus demselben Rechteck; ein je Zustand neu
    // gesuchtes könnte eine Verschiebung wegrechnen, statt sie zu zeigen.
    const QImage erstes = vollbild(fluechtig.filePath(QStringLiteral("_erstes.png")));
    const QRect lage = erstes.isNull() ? QRect() : fensterImBild(ohne, erstes, erwartet);
    if (!lage.isValid()) {
        out << "Das Fenster ist im Vollbild nicht zu finden — abgebrochen, statt aus der\n";
        out << "Qt-Geometrie zu schneiden: die meldet unter Wayland nicht die Bildschirmlage.\n";
        out.flush();
        return 2;
    }
    out << "Fenster samt Schatten im Vollbild: " << lage.width() << "x" << lage.height() << " an "
        << lage.x() << "," << lage.y() << "\n\n";

    const auto aufnehmen = [&](const QString &name, const QString &was) {
        const QImage bild = vollbild(fluechtig.filePath(QStringLiteral("_v.png")));
        if (bild.isNull()) {
            out << name << ": keine Aufnahme entstanden\n";
            return;
        }
        bild.copy(lage).save(QDir(ordner).filePath(QStringLiteral("wechsel-%1.png").arg(name)));
        out << name << " (" << was << ")\n";
        out << "    Notiztext  : " << farbe(feld->palette().color(QPalette::Text)) << "\n";
        out << "    Kleintext  : "
            << farbe(kleintexte.first()->palette().color(kleintexte.first()->foregroundRole()))
            << "\n";
        out << "    Schema sagt: " << farbe(fenster.palette().color(QPalette::WindowText)) << " / "
            << farbe(fenster.palette().color(QPalette::PlaceholderText)) << "\n";
        out.flush();
    };

    aufnehmen(QStringLiteral("1-default"), QStringLiteral("Theme ohne colors-Datei, Schema gilt"));

    fenster.reloadDesktopTheme(QStringLiteral("breeze-light"));
    warte(1200);
    aufnehmen(QStringLiteral("2-breeze-light"),
              QStringLiteral("AK 5: Theme-Wechsel am stehenden Fenster"));

    // AK 7: das Schema wandert, die Themefarbe soll stehen bleiben.
    const QPalette start = qApp->palette();
    QPalette grell = start;
    grell.setColor(QPalette::WindowText, QColor(255, 0, 255));
    grell.setColor(QPalette::PlaceholderText, QColor(255, 0, 255));
    qApp->setPalette(grell);
    warte(1200);
    aufnehmen(QStringLiteral("3-breeze-light-nach-schemawechsel"),
              QStringLiteral("AK 7: Schema auf Magenta, Theme hat eine colors-Datei"));

    qApp->setPalette(start);
    warte(800);

    // Wireframe 4b: „Auswahl, Cursor, Scrollbalken — unverändert aus der
    // Palette." Diese Zeile und die neue Regel treffen sich im selben Textfeld:
    // Der Text kommt jetzt aus dem Theme, seine Auswahl weiter aus dem Schema.
    // Ob das zusammen lesbar bleibt, ist eine Bildfrage und keine Rechenfrage.
    feld->selectAll();
    warte(600);
    aufnehmen(QStringLiteral("5-breeze-light-auswahl"),
              QStringLiteral("Themeschrift, Auswahl aus dem Schema"));
    QTextCursor ohneAuswahl = feld->textCursor();
    ohneAuswahl.clearSelection();
    feld->setTextCursor(ohneAuswahl);
    warte(400);

    // Und zurück — eine Farbe, die nur einmal wandert, wäre auch durch eine
    // erklärt, die einmal gesetzt und nie geräumt wird.
    fenster.reloadDesktopTheme(QStringLiteral("default"));
    warte(1200);
    aufnehmen(QStringLiteral("4-zurueck-auf-default"),
              QStringLiteral("AK 5 rückwärts: Themefarbe geräumt, Schema gilt wieder"));

    out.flush();
    return 0;
}
