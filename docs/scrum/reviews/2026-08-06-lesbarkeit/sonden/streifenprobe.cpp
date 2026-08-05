// Zweite Messsonde vom 06.08.2026: Was zeichnet `alternatingRowColors` in
// DIESER Liste wirklich?
//
// Die erste Sonde (`listengrund`) rechnet Kontraste aus Palettenwerten. Diese
// hier rechnet nichts, sondern schaltet den Schalter an und liest die Farbe
// aus dem gegriffenen Bild — Zeile für Zeile, am linken Rand, wo kein Text
// steht. Damit fällt die Frage weg, welche der beiden Paletten (die der
// Ansicht oder die des Viewports) am Ende gewinnt: gemessen wird, was zu
// sehen ist.
//
// Zwei Dinge kommen dabei mit heraus, nach denen niemand gefragt hat und die
// über die Empfehlung mitentscheiden:
//   * Ob der Gruppenkopf mitgestreift wird — er ist eine Zeile des Modells
//     wie jede andere.
//   * Ob die Streifen dem Wechsel der Notiz folgen oder dem Zeilenzähler,
//     der die Köpfe mitzählt.
//
// Kein Projektcode: die Sonde linkt gegen die im Bauplatz gebaute
// `libdenkzettelui.a` und ändert nichts.
//
// Aufruf: streifenprobe <Zielverzeichnis|-> <Pfad zur .colors> …

#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <KColorScheme>
#include <KSharedConfig>

#include <QAbstractItemModel>
#include <QApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImage>
#include <QListView>
#include <QStandardPaths>
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

double luminance(const QColor &color)
{
    auto channel = [](double value) {
        return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(color.redF()) + 0.7152 * channel(color.greenF()) + 0.0722 * channel(color.blueF());
}

double contrast(const QColor &a, const QColor &b)
{
    const double first = luminance(a);
    const double second = luminance(b);
    return (std::max(first, second) + 0.05) / (std::min(first, second) + 0.05);
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
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Aufruf: streifenprobe <Zielverzeichnis|-> <Pfad zur .colors> …\n");
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create(QStringLiteral("breeze")));
    const QString directory = QString::fromLocal8Bit(argv[1]);

    // Zwei Bestände, nicht einer. Der Streifenwechsel hängt an der Zeilenzahl,
    // und ein Gruppenkopf ist eine Zeile — ob der Rhythmus an einer
    // Gruppengrenze bricht, entscheidet also die Größe der Gruppe davor. Mit
    // einem einzigen Bestand gemessen sähe das Ergebnis nach einem festen
    // Befund aus und wäre eine Eigenschaft der Beispieldaten.
    Store ungerade(sandbox.filePath(QStringLiteral("ungerade.db")));
    Store gerade(sandbox.filePath(QStringLiteral("gerade.db")));
    if (!ungerade.open() || !gerade.open()) {
        printf("Store konnte nicht geöffnet werden\n");
        return 1;
    }
    // Heute 3 · Gestern 2 · Diese Woche 1
    addNote(ungerade, QStringLiteral("Backup prüfen\nprune-Policy, monatliche Snapshots behalten"),
            QStringLiteral("2026-07-31T14:20:00"));
    addNote(ungerade, QStringLiteral("Zahnarzt anrufen\nTermin für September, am besten vormittags"),
            QStringLiteral("2026-07-31T09:05:00"));
    addNote(ungerade, QStringLiteral("Einkauf\nMehl, Hefe, Tomaten"), QStringLiteral("2026-07-31T08:10:00"));
    addNote(ungerade, QStringLiteral("Idee: Denkzettel-Export\nMarkdown mit Frontmatter, ein Ordner je Monat"),
            QStringLiteral("2026-07-30T21:40:00"));
    addNote(ungerade, QStringLiteral("Anruf Vermieter\nHeizungsablesung, Termin bestätigen"),
            QStringLiteral("2026-07-30T11:15:00"));
    addNote(ungerade, QStringLiteral("Buchtipp notieren\nvon Kolja empfohlen, Titel nachschlagen"),
            QStringLiteral("2026-07-28T19:00:00"));

    // Heute 2 · Gestern 2 · Diese Woche 2
    addNote(gerade, QStringLiteral("Backup prüfen\nprune-Policy, monatliche Snapshots behalten"),
            QStringLiteral("2026-07-31T14:20:00"));
    addNote(gerade, QStringLiteral("Zahnarzt anrufen\nTermin für September, am besten vormittags"),
            QStringLiteral("2026-07-31T09:05:00"));
    addNote(gerade, QStringLiteral("Idee: Denkzettel-Export\nMarkdown mit Frontmatter, ein Ordner je Monat"),
            QStringLiteral("2026-07-30T21:40:00"));
    addNote(gerade, QStringLiteral("Anruf Vermieter\nHeizungsablesung, Termin bestätigen"),
            QStringLiteral("2026-07-30T11:15:00"));
    addNote(gerade, QStringLiteral("Buchtipp notieren\nvon Kolja empfohlen, Titel nachschlagen"),
            QStringLiteral("2026-07-28T19:00:00"));
    addNote(gerade, QStringLiteral("Rechnung Stadtwerke\nAbschlag anpassen, Zählerstand melden"),
            QStringLiteral("2026-07-28T10:30:00"));

    const QList<QPair<QString, Store *>> stocks = {
        {QStringLiteral("Heute 3 · Gestern 2 · Diese Woche 1"), &ungerade},
        {QStringLiteral("Heute 2 · Gestern 2 · Diese Woche 2"), &gerade},
    };

    for (int i = 2; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        const QString name = QFileInfo(path).baseName();

        KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
        app.setPalette(KColorScheme::createApplicationPalette(scheme));

        for (const auto &stock : stocks) {
        LibraryWindow window(stock.second);
        window.setReferenceTime(QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate));
        window.resize(900, 600);
        window.showLibrary();
        settle(150);

        auto *list = window.findChild<QListView *>();
        if (!list) {
            printf("%s — keine Liste gefunden\n", qPrintable(name));
            continue;
        }
        // Der Schalter, um den es geht. Er wird nur in dieser Sonde gesetzt,
        // nicht im Produktivcode.
        list->setAlternatingRowColors(true);
        // Ohne Auswahl: eine markierte Zeile überdeckt den Streifen.
        list->setCurrentIndex(QModelIndex());
        list->clearSelection();
        settle(150);

        const QImage picture = window.grab().toImage();
        const double ratio = picture.devicePixelRatio();

        printf("############ %s   (alternatingRowColors = an) — %s\n", qPrintable(name),
               qPrintable(stock.first));
        printf("%-5s %-9s %-7s %-9s\n", "Zeile", "Art", "Höhe", "Grund");

        QList<QColor> entryGrounds;
        const QAbstractItemModel *model = list->model();
        for (int row = 0; row < model->rowCount(); ++row) {
            const QModelIndex index = model->index(row, 0);
            const QRect rect = list->visualRect(index);
            if (rect.height() <= 0 || !list->viewport()->rect().intersects(rect)) {
                continue;
            }
            const bool head = index.data(NoteListModel::GroupHeaderRole).toBool();
            const QPoint spot =
                list->viewport()->mapTo(&window, QPoint(3, rect.center().y())) * ratio;
            const QColor ground = picture.pixelColor(spot);
            printf("%-5d %-9s %-7d %-9s\n", row, head ? "Kopf" : "Notiz", rect.height(),
                   qPrintable(ground.name()));
            if (!head) {
                entryGrounds.append(ground);
            }
        }

        printf("Kontrast zwischen benachbarten Notizzeilen:");
        for (int k = 1; k < entryGrounds.size(); ++k) {
            printf(" %.2f:1", contrast(entryGrounds[k - 1], entryGrounds[k]));
        }
        printf("\n\n");

        if (directory != QLatin1String("-") && stock.second == &ungerade) {
            window.grab().save(QStringLiteral("%1/streifen-%2.png").arg(directory, name));
        }
        }
    }

    return 0;
}
