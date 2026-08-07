/**
 * Messsonde des UI/UX-Reviews zu Sprint 9 (#100 und #101).
 *
 * Eigene Bilder, eigene Messung — die Bilder der Umsetzungsstränge ersetzen die
 * des Reviews nicht (DoD 3, Beschluss B3). Die Sonde linkt gegen die übersetzten
 * Bibliotheken des Projekts, damit sie den gelieferten Code zeigt und keinen
 * Nachbau davon.
 *
 * Sie kennt vier Betriebsarten:
 *
 *   bibliothek  <Zielverzeichnis> <Präfix> [auswahl|hover|keine]
 *       Das Bibliotheksfenster im Normalfall von Zeichnung 3a. Schreibt das
 *       Bild, zwei Ausschnitte (Gruppengrenze und Notizgrenze, 1:1 und
 *       vierfach) und einen Messbericht: wo die Ansicht ihre Zeilen hat, wo im
 *       Bild Linien stehen, wie weit sie reichen und wie stark sie sich vom
 *       Listengrund abheben.
 *
 *   erfassung   <Zielverzeichnis> <Präfix>
 *       Das Erfassungsfenster mit Text. Schreibt Bild und einen Schnitt durch
 *       die Fenstermitte: jeder Farbwechsel von oben nach unten, mit Deckung.
 *
 *   sitzung-bibliothek <Sekunden>
 *   sitzung-erfassung  <Sekunden>
 *       Für den Beleg aus der angemeldeten Sitzung (B21). Legt ein eigenes
 *       Hintergrundfenster mit magentafarbenem Rahmen unter das Prüffenster,
 *       damit die Bildschirmaufnahme daran zugeschnitten werden kann und vom
 *       Schreibtisch des Kunden kein Bildpunkt ins öffentliche Repository
 *       gerät. Der Schatten fällt dabei auf eine Fläche, die die Sonde selbst
 *       gezeichnet hat, und kommt so ins Bild.
 *
 * QT_SCALE_FACTOR gehört in den Offscreen-Lauf und nicht in den Sitzungslauf:
 * Unter Wayland multipliziert die Variable mit der Sitzungsskalierung.
 */

#include "capture/capturewindow.h"

#include <KWindowShadow>
#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QHash>
#include <QLineEdit>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScreen>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>
#include <QTimer>

namespace
{

QDateTime friday()
{
    return QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate);
}

void addNote(Store &store, const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = QDateTime::fromString(isoDateTime, Qt::ISODate);
    if (!store.addNote(note).has_value()) {
        qFatal("Notiz ließ sich nicht speichern");
    }
}

/** Die Notizmenge des Normalfalls — dieselbe wie in `libraryshots`, Bild 1. */
void fillNormalCase(Store &store)
{
    addNote(store,
            QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten — "
                           "als Cronjob auf dem NAS einrichten und danach einmal wiederherstellen"),
            QStringLiteral("2026-07-31T14:32:00"));
    addNote(store,
            QStringLiteral("Idee für Denkzettel — Bündel-Export erst vorschlagen, wenn mindestens "
                           "fünf Notizen zum selben Thema da sind, sonst wird der Vault zugemüllt"),
            QStringLiteral("2026-07-31T11:05:00"));
    addNote(store, QStringLiteral("journalctl -u whisperd --since today"),
            QStringLiteral("2026-07-30T21:48:00"));
    addNote(store, QStringLiteral("Mara wegen Wochenende anrufen,\nKuchen nicht vergessen"),
            QStringLiteral("2026-07-28T09:00:00"));
    addNote(store, QStringLiteral("Kategorien-Prompt: Beispiele mitgeben, sonst rät das Modell"),
            QStringLiteral("2026-07-23T09:00:00"));
    addNote(store, QStringLiteral("Tray-Icon im dunklen Theme testen"),
            QStringLiteral("2026-07-10T09:00:00"));
}

/**
 * Eine Datenlage mit gefüllten Gruppen — der Normalfall hat vier Gruppen mit
 * je einer Notiz, und zwischen zwei Notizen derselben Gruppe steht die
 * eingerückte Linie nur dort, wo es zwei gibt. Für die Frage, ob sich die
 * Rangfolge Notiz/Gruppe aus der Länge des Striches liest, braucht das Bild
 * beide Linien nebeneinander.
 */
void fillRhythm(Store &store)
{
    addNote(store, QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten"),
            QStringLiteral("2026-07-31T14:32:00"));
    addNote(store, QStringLiteral("Idee für Denkzettel — Bündel-Export erst ab fünf Notizen vorschlagen"),
            QStringLiteral("2026-07-31T11:05:00"));
    addNote(store, QStringLiteral("Kaffeefilter, Hafermilch, Zitronen"), QStringLiteral("2026-07-31T08:14:00"));
    addNote(store, QStringLiteral("journalctl -u whisperd --since today"),
            QStringLiteral("2026-07-30T21:48:00"));
    addNote(store, QStringLiteral("Rückruf Werkstatt: Termin für den Reifenwechsel"),
            QStringLiteral("2026-07-30T15:20:00"));
    addNote(store, QStringLiteral("Vortragsfolien auf zwölf Seiten kürzen"),
            QStringLiteral("2026-07-30T09:02:00"));
    addNote(store, QStringLiteral("Mara wegen Wochenende anrufen,\nKuchen nicht vergessen"),
            QStringLiteral("2026-07-28T09:00:00"));
    addNote(store, QStringLiteral("Kategorien-Prompt: Beispiele mitgeben, sonst rät das Modell"),
            QStringLiteral("2026-07-27T18:40:00"));
}

QListView *listOf(QWidget &window)
{
    auto *list = window.findChild<QListView *>();
    Q_ASSERT(list);
    return list;
}

double luminance(const QColor &colour)
{
    const auto channel = [](double value) {
        return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(colour.redF()) + 0.7152 * channel(colour.greenF()) + 0.0722 * channel(colour.blueF());
}

double contrast(const QColor &one, const QColor &other)
{
    const double a = luminance(one);
    const double b = luminance(other);
    return (std::max(a, b) + 0.05) / (std::min(a, b) + 0.05);
}

QString show(const QColor &colour)
{
    return QStringLiteral("%1,%2,%3 (a%4)")
        .arg(colour.red())
        .arg(colour.green())
        .arg(colour.blue())
        .arg(colour.alpha());
}

/** Speichert einen Ausschnitt zweimal: in Originalgröße und vierfach vergrößert. */
void cutOut(const QImage &picture, const QRect &area, const QString &directory, const QString &name)
{
    const QImage piece = picture.copy(area);
    if (!piece.save(QDir(directory).filePath(name + QStringLiteral(".png")))) {
        qFatal("Ausschnitt ließ sich nicht schreiben: %s", qUtf8Printable(name));
    }
    const QImage large = piece.scaled(piece.size() * 4, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    if (!large.save(QDir(directory).filePath(name + QStringLiteral("-4x.png")))) {
        qFatal("Vergrößerung ließ sich nicht schreiben: %s", qUtf8Printable(name));
    }
}

/**
 * Die häufigste Farbe eines Bereichs — der Listengrund, gegen den alles andere
 * gemessen wird.
 */
QRgb commonColour(const QImage &picture, const QRect &area)
{
    QHash<QRgb, int> tally;
    for (int y = area.top(); y <= area.bottom(); ++y) {
        for (int x = area.left(); x <= area.right(); ++x) {
            ++tally[picture.pixel(x, y)];
        }
    }
    QRgb best = 0;
    int most = -1;
    for (auto it = tally.cbegin(); it != tally.cend(); ++it) {
        if (it.value() > most) {
            most = it.value();
            best = it.key();
        }
    }
    return best;
}

/** Beschreibt eine Bildzeile, die überwiegend von einer einzigen Fremdfarbe belegt ist. */
struct Streak {
    int y = 0;
    int from = 0;
    int to = 0;
    QRgb colour = 0;
    int coverage = 0;
};

/**
 * Sucht im Listenbereich nach Zeilen, die über weite Strecken eine einzige
 * andere Farbe als den Grund tragen — so sieht eine Haarlinie im Bild aus.
 * Textzeilen fallen durch, weil ihre Bildpunkte in vielen Farben und mit
 * Lücken stehen.
 */
QList<Streak> findStreaks(const QImage &picture, const QRect &area, QRgb ground)
{
    QList<Streak> found;
    for (int y = area.top(); y <= area.bottom(); ++y) {
        QHash<QRgb, int> tally;
        int first = -1;
        int last = -1;
        for (int x = area.left(); x <= area.right(); ++x) {
            const QRgb pixel = picture.pixel(x, y);
            if (pixel == ground) {
                continue;
            }
            ++tally[pixel];
            if (first < 0) {
                first = x;
            }
            last = x;
        }
        if (first < 0) {
            continue;
        }
        QRgb dominant = 0;
        int most = 0;
        for (auto it = tally.cbegin(); it != tally.cend(); ++it) {
            if (it.value() > most) {
                most = it.value();
                dominant = it.key();
            }
        }
        // Eine Linie belegt ihre Strecke lückenlos in einer Farbe; Text tut das
        // nicht. Die Schwelle liegt bei drei Vierteln der Breite des Bereichs.
        if (most * 4 < area.width() * 3) {
            continue;
        }
        found.append(Streak{y, first - area.left(), last - area.left(), dominant, most});
    }
    return found;
}

int deviceOf(int logical, qreal ratio)
{
    return qRound(logical * ratio);
}

void shootLibrary(const QString &directory, const QString &prefix, const QString &mode, const QString &data)
{
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        qFatal("Store ließ sich nicht öffnen");
    }
    if (data == QLatin1String("rhythmus")) {
        fillRhythm(store);
    } else {
        fillNormalCase(store);
    }

    LibraryWindow window(&store);
    window.setReferenceTime(friday());
    window.resize(900, 600);
    window.showLibrary();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("Fenster kam nicht auf den Schirm");
    }

    QListView *list = listOf(window);
    if (mode.startsWith(QLatin1String("suche:"))) {
        // Die Trefferliste zeigt dieselbe Gliederung wie die volle Liste
        // (Zeichnung 2c, AK 6 von #101) — Gruppen ohne Treffer fallen weg, und
        // an der neuen ersten Gruppe darf keine Linie stehen.
        auto *field = window.findChild<QLineEdit *>();
        Q_ASSERT(field);
        field->setText(mode.mid(6));
        QTest::qWait(400);
        list->clearSelection();
        list->setCurrentIndex(QModelIndex());
    } else if (mode == QLatin1String("auswahl")) {
        list->setCurrentIndex(list->model()->index(2, 0));
    } else if (mode == QLatin1String("hover")) {
        list->clearSelection();
        list->setCurrentIndex(QModelIndex());
        // Zeile 2 überfahren: die erste Notiz unter dem Kopf „Heute".
        const QRect row = list->visualRect(list->model()->index(2, 0));
        const QPoint at = row.center();
        list->setAttribute(Qt::WA_Hover, true);
        list->viewport()->setAttribute(Qt::WA_Hover, true);
        QMouseEvent move(QEvent::MouseMove, at, list->viewport()->mapToGlobal(at),
                         Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(list->viewport(), &move);
    } else {
        list->clearSelection();
        list->setCurrentIndex(QModelIndex());
    }
    QTest::qWait(400);

    const QPixmap grabbed = window.grab();
    const QImage picture = grabbed.toImage();
    const qreal ratio = grabbed.devicePixelRatio();
    if (!picture.save(QDir(directory).filePath(prefix + QStringLiteral(".png")))) {
        qFatal("Bild ließ sich nicht schreiben");
    }

    QFile report(QDir(directory).filePath(prefix + QStringLiteral(".txt")));
    if (!report.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qFatal("Bericht ließ sich nicht schreiben");
    }
    QTextStream out(&report);
    out << "Bibliothek — " << prefix << "\n";
    out << "Bildgröße " << picture.width() << "x" << picture.height()
        << " Gerätebildpunkte, Verhältnis " << ratio << "\n\n";

    QWidget *viewport = list->viewport();
    const QPoint viewportTopLeft = viewport->mapTo(&window, QPoint(0, 0));
    const QRect viewportLogical(viewportTopLeft, viewport->size());
    const QRect viewportDevice(deviceOf(viewportLogical.x(), ratio),
                               deviceOf(viewportLogical.y(), ratio),
                               deviceOf(viewportLogical.width(), ratio),
                               deviceOf(viewportLogical.height(), ratio));
    out << "Sichtfeld der Liste im Fenster: " << viewportLogical.x() << "," << viewportLogical.y()
        << " " << viewportLogical.width() << "x" << viewportLogical.height() << " (logisch)\n";
    out << "                    im Bild:   " << viewportDevice.x() << "," << viewportDevice.y()
        << " " << viewportDevice.width() << "x" << viewportDevice.height() << "\n\n";

    out << "Zeilen der Ansicht (logisch, Fensterkoordinaten):\n";
    QList<int> groupBoundaries;
    QList<int> noteBoundaries;
    for (int row = 0; row < list->model()->rowCount(); ++row) {
        const QModelIndex index = list->model()->index(row, 0);
        const QRect rect = list->visualRect(index);
        const bool head = index.data(NoteListModel::GroupHeaderRole).toBool();
        const int top = viewportLogical.y() + rect.top();
        const int bottom = viewportLogical.y() + rect.bottom();
        out << QStringLiteral("  %1 %2 y=%3..%4 h=%5  %6\n")
                   .arg(row, 2)
                   .arg(head ? QStringLiteral("Kopf ") : QStringLiteral("Notiz"))
                   .arg(top, 4)
                   .arg(bottom, 4)
                   .arg(rect.height(), 3)
                   .arg(index.data(Qt::DisplayRole).toString().left(38));
        if (head && row > 0) {
            groupBoundaries.append(top);
        }
        if (!head) {
            const QModelIndex below = list->model()->index(row + 1, 0);
            if (below.isValid() && !below.data(NoteListModel::GroupHeaderRole).toBool()) {
                noteBoundaries.append(bottom);
            }
        }
    }

    const QRgb ground = commonColour(picture, viewportDevice);
    out << "\nListengrund: " << show(QColor::fromRgb(ground)) << "\n\n";

    // Nur der textfreie Streifen rechts wird abgetastet: dort steht keine
    // Schrift, und eine Linie, die bis dorthin reicht, zeigt sich als
    // lückenloser Lauf.
    const QList<Streak> streaks = findStreaks(picture, viewportDevice, ground);
    out << "Durchgehende Farbläufe im Sichtfeld (Bildzeile, Strecke ab linkem Sichtfeldrand):\n";
    for (const Streak &streak : streaks) {
        const QColor colour = QColor::fromRgb(streak.colour);
        out << QStringLiteral("  y=%1  x=%2..%3  Breite %4 von %5  Farbe %6  Kontrast %7 : 1\n")
                   .arg(streak.y, 4)
                   .arg(streak.from, 4)
                   .arg(streak.to, 4)
                   .arg(streak.to - streak.from + 1, 4)
                   .arg(viewportDevice.width(), 4)
                   .arg(show(colour), -22)
                   .arg(contrast(colour, QColor::fromRgb(ground)), 0, 'f', 2);
    }

    // Zwei Ausschnitte: eine Gruppengrenze und eine Notizgrenze, jeweils mit
    // reichlich Luft darüber und darunter, damit das Auge den Rhythmus sieht
    // und nicht nur den Strich.
    if (!groupBoundaries.isEmpty()) {
        const int y = deviceOf(groupBoundaries.first(), ratio);
        const QRect area(viewportDevice.x(), std::max(0, y - deviceOf(46, ratio)),
                         viewportDevice.width(), deviceOf(92, ratio));
        cutOut(picture, area, directory, prefix + QStringLiteral("-gruppengrenze"));
        out << "\nAusschnitt Gruppengrenze um y=" << y << "\n";
    }
    if (!noteBoundaries.isEmpty()) {
        const int y = deviceOf(noteBoundaries.first(), ratio);
        const QRect area(viewportDevice.x(), std::max(0, y - deviceOf(46, ratio)),
                         viewportDevice.width(), deviceOf(92, ratio));
        cutOut(picture, area, directory, prefix + QStringLiteral("-notizgrenze"));
        out << "Ausschnitt Notizgrenze um y=" << y << "\n";
    }

    // Der ganze Listenstreifen, damit der Rhythmus in einem Bild steht.
    cutOut(picture, viewportDevice, directory, prefix + QStringLiteral("-liste"));
}

/**
 * Schreibt eine `plasmarc` mit dem gewünschten Desktop-Theme in den Sandkasten.
 * Ohne Eintrag greift derselbe Rückfall wie beim Kunden, dessen `plasmarc`
 * keinen Namen trägt: `default`.
 */
void chooseTheme(const QString &configurationPath, const QString &theme)
{
    // Das Farbschema des Kunden wandert in den Sandkasten: Die Theme-Grafik
    // wird aus `kdeglobals` umgefärbt, und ein leeres Verzeichnis machte aus
    // einem dunklen Schreibtisch ein helles Bild.
    const QString real = QDir::homePath() + QStringLiteral("/.config/kdeglobals");
    if (QFile::exists(real)) {
        QFile::copy(real, QDir(configurationPath).filePath(QStringLiteral("kdeglobals")));
    }
    if (theme.isEmpty()) {
        return;
    }
    QFile file(QDir(configurationPath).filePath(QStringLiteral("plasmarc")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qFatal("plasmarc ließ sich nicht schreiben");
    }
    QTextStream out(&file);
    out << "[Theme]\nname=" << theme << "\n";
}

void shootCapture(const QString &directory, const QString &prefix, const QString &theme)
{
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());
    chooseTheme(configuration.path(), theme);

    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        qFatal("Store ließ sich nicht öffnen");
    }

    CaptureWindow window(&store);
    auto *text = window.findChild<QPlainTextEdit *>();
    Q_ASSERT(text);
    // Der leere Zustand gehört mitgeprüft: Der Platzhaltertext steht seit dem
    // Feld auf der Feldfläche und nicht mehr auf der Hülle.
    if (!prefix.contains(QLatin1String("leer"))) {
        text->setPlainText(QStringLiteral("Der Eingabebereich soll als solcher erkennbar sein"));
    }
    window.show();
    QCoreApplication::processEvents();
    QTest::qWait(300);

    const QPixmap grabbed = window.grab();
    const QImage picture = grabbed.toImage();
    const qreal ratio = grabbed.devicePixelRatio();
    if (!picture.save(QDir(directory).filePath(prefix + QStringLiteral(".png")))) {
        qFatal("Bild ließ sich nicht schreiben");
    }

    QFile report(QDir(directory).filePath(prefix + QStringLiteral(".txt")));
    if (!report.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qFatal("Bericht ließ sich nicht schreiben");
    }
    QTextStream out(&report);
    out << "Erfassungsfenster — " << prefix << "\n";
    out << "Bildgröße " << picture.width() << "x" << picture.height()
        << " Gerätebildpunkte, Verhältnis " << ratio << "\n";
    const QRect textLogical(text->mapTo(&window, QPoint(0, 0)), text->size());
    out << "Textfeld im Fenster: " << textLogical.x() << "," << textLogical.y() << " "
        << textLogical.width() << "x" << textLogical.height() << " (logisch)\n\n";

    const int column = picture.width() / 2;
    out << "Schnitt durch die Fenstermitte (x=" << column << "), jeder Farbwechsel:\n";
    QRgb previous = 0;
    bool first = true;
    for (int y = 0; y < picture.height(); ++y) {
        const QRgb pixel = picture.pixel(column, y);
        if (first || pixel != previous) {
            out << QStringLiteral("  y=%1  %2\n").arg(y, 4).arg(show(QColor::fromRgba(pixel)));
            previous = pixel;
            first = false;
        }
    }

    const int line = deviceOf(textLogical.y() + textLogical.height() / 2, ratio);
    out << "\nSchnitt quer durch die Feldmitte (y=" << line << "), jeder Farbwechsel bis x=" << picture.width() / 3
        << ":\n";
    first = true;
    for (int x = 0; x < picture.width() / 3; ++x) {
        const QRgb pixel = picture.pixel(x, line);
        if (first || pixel != previous) {
            out << QStringLiteral("  x=%1  %2\n").arg(x, 4).arg(show(QColor::fromRgba(pixel)));
            previous = pixel;
            first = false;
        }
    }

    // Die Abhebung des Feldes gegen die Hülle, an zwei Punkten abgelesen, die
    // sicher in der jeweiligen Fläche liegen: über dem Feld steht der App-Name,
    // links daneben die Hülle.
    const QColor hull = QColor::fromRgba(picture.pixel(deviceOf(4, ratio), line));
    const QColor field = QColor::fromRgba(picture.pixel(column, line));
    out << "\nHülle bei x=" << deviceOf(4, ratio) << ": " << show(hull) << "\n";
    out << "Feld  bei x=" << column << ": " << show(field) << "\n";
    out << "Kontrast Feld gegen Hülle, ohne Untergrund gerechnet: "
        << QString::number(contrast(field, hull), 'f', 2) << " : 1\n";
    // Beide Flächen decken nicht voll; der Seheindruck hängt am Untergrund.
    for (const QColor &under : {QColor(Qt::black), QColor(Qt::white), QColor(0x31, 0x36, 0x3b)}) {
        const auto over = [&under](const QColor &top) {
            const double a = top.alphaF();
            return QColor::fromRgbF(top.redF() * a + under.redF() * (1 - a),
                                    top.greenF() * a + under.greenF() * (1 - a),
                                    top.blueF() * a + under.blueF() * (1 - a));
        };
        out << "  über " << show(under) << ": "
            << QString::number(contrast(over(field), over(hull)), 'f', 2) << " : 1\n";
    }

    cutOut(picture, QRect(0, 0, picture.width(), std::min(picture.height(), deviceOf(70, ratio))), directory,
           prefix + QStringLiteral("-kopf"));
}

/**
 * Das Hintergrundfenster der Sitzungsläufe: eine eigene Fläche unter dem
 * Prüffenster, mit magentafarbenem Rahmen als Schnittmarke. Was die
 * Bildschirmaufnahme innerhalb dieses Rahmens zeigt, hat die Sonde gezeichnet.
 */
class Backdrop : public QWidget
{
public:
    Backdrop()
    {
        setWindowTitle(QStringLiteral("Denkzettel UI-Review — Hintergrund"));
        resize(1100, 800);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), QColor(0xf2, 0xf0, 0xeb));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0xe4, 0xe1, 0xdb));
        constexpr int Step = 10;
        for (int offset = -height(); offset < width(); offset += 2 * Step) {
            painter.drawPolygon(QPolygon({QPoint(offset, 0),
                                          QPoint(offset + Step, 0),
                                          QPoint(offset + Step + height(), height()),
                                          QPoint(offset + height(), height())}));
        }
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(0xff, 0x00, 0xff), 4));
        painter.drawRect(QRect(2, 2, width() - 4, height() - 4));
    }
};

int runSession(QApplication &app, const QString &what, int seconds)
{
    // Der Sandkasten hält die gespeicherte Fenstergröße des Kunden heraus,
    // bekommt aber sein Farbschema mit: Die Theme-Grafik färbt aus `kdeglobals`
    // um, und ohne diese Datei zeigte das Bild ein helles Fenster mit hellem
    // Text — ein Fehler der Sonde, der wie ein Fehler des Erzeugnisses aussähe.
    const QTemporaryDir configuration;
    chooseTheme(configuration.path(), QString());
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    const QTemporaryDir dir;
    auto *store = new Store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store->open()) {
        qFatal("Store ließ sich nicht öffnen");
    }

    Backdrop backdrop;
    backdrop.show();

    QWidget *probe = nullptr;
    if (what == QLatin1String("bibliothek")) {
        fillNormalCase(*store);
        auto *library = new LibraryWindow(store);
        library->setReferenceTime(friday());
        library->resize(900, 600);
        library->showLibrary();
        probe = library;
    } else {
        auto *capture = new CaptureWindow(store);
        auto *text = capture->findChild<QPlainTextEdit *>();
        // showCapture() und nicht show(): Nur auf diesem Weg bindet das Fenster
        // seinen Schatten an die frische Wayland-Fläche, und ein Bild ohne ihn
        // beliese AK 2 falsch. Der Text kommt danach, denn das Fenster kommt
        // leer herauf.
        capture->showCapture();
        text->setPlainText(QStringLiteral("Der Eingabebereich soll als solcher erkennbar sein"));
        // Ob der Schatten angelegt ist, sagt das Fenster selbst — die Zeichnung
        // lässt für ihn ausdrücklich die benannte Zusicherung zu, weil ein Bild
        // ihn nicht in jeder Lage zeigt.
        QTextStream(stdout) << "Schatten angelegt: "
                            << (capture->shadow() != nullptr ? "ja" : "nein") << "\n";
        probe = capture;
    }

    QTimer::singleShot(seconds * 1000, &app, [&app]() {
        app.quit();
    });
    const int result = app.exec();
    delete probe;
    delete store;
    return result;
}
}

int main(int argc, char **argv)
{
    // NOLINTNEXTLINE(misc-const-correctness)
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    const QStringList arguments = app.arguments();
    if (arguments.size() < 3) {
        qFatal("Aufruf: uxsonde <bibliothek|erfassung> <Verzeichnis> <Präfix> [auswahl|hover|keine]\n"
               "        uxsonde <sitzung-bibliothek|sitzung-erfassung> <Sekunden>");
    }

    const QString mode = arguments.at(1);
    if (mode == QLatin1String("sitzung-bibliothek")) {
        return runSession(app, QStringLiteral("bibliothek"), arguments.at(2).toInt());
    }
    if (mode == QLatin1String("sitzung-erfassung")) {
        return runSession(app, QStringLiteral("erfassung"), arguments.at(2).toInt());
    }

    if (arguments.size() < 4) {
        qFatal("Aufruf: uxsonde <bibliothek|erfassung> <Verzeichnis> <Präfix> [auswahl|hover|keine]");
    }
    const QString directory = arguments.at(2);
    const QString prefix = arguments.at(3);
    if (!QDir().mkpath(directory)) {
        qFatal("Zielverzeichnis ließ sich nicht anlegen");
    }

    if (mode == QLatin1String("bibliothek")) {
        shootLibrary(directory,
                     prefix,
                     arguments.size() > 4 ? arguments.at(4) : QStringLiteral("auswahl"),
                     arguments.size() > 5 ? arguments.at(5) : QStringLiteral("normal"));
        return 0;
    }
    if (mode == QLatin1String("erfassung")) {
        shootCapture(directory, prefix, arguments.size() > 4 ? arguments.at(4) : QString());
        return 0;
    }

    qFatal("Unbekannte Betriebsart: %s", qUtf8Printable(mode));
}
