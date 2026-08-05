// Messsonde zur Gestaltungsfrage vom 06.08.2026 (Issues #100 und #101):
// Womit lassen sich die Einträge der Bibliotheksliste voneinander trennen?
//
// Die Sonde beantwortet vier Fragen und rät bei keiner:
//
//   1. Auf welcher Palettenrolle steht die Liste — angemeldet und gezeichnet?
//      Beides wird getrennt erhoben: `backgroundRole()` des Viewports sagt,
//      was angemeldet ist; ein Bildpunkt aus dem gegriffenen Fenster sagt,
//      was gezeichnet wurde. Gehen die beiden auseinander, zählt der Bildpunkt.
//   2. Wie groß ist der Unterschied zwischen `AlternateBase` und dem
//      **gezeichneten** Grund? Die HIG empfehlen abwechselnde Zeilenfarben;
//      ob sie auf diesem Bildschirm sichtbar sind, entscheidet diese Zahl.
//      Zum Vergleich steht daneben, was dieselbe Empfehlung bei einer Liste
//      auf `Base` einbrächte.
//   3. Welche Rolle trüge eine Trennlinie, und mit welchem Kontrast? Geprüft
//      werden die vier Rollen, an denen die Fußzeilen-Linie am 01.08.2026
//      gescheitert ist, plus die Mischung aus Textfarbe und Grund im
//      Verhältnis `frameContrast` des Schemas — das Verfahren, mit dem
//      Kirigami seine Trennlinien färbt.
//   4. Hängt der gezeichnete Grund am Stil? Dieselbe Messung läuft unter
//      `breeze` und unter `fusion`.
//
// Kein Projektcode: die Sonde linkt gegen die im Bauplatz gebaute
// `libdenkzettelui.a` und ändert nichts.
//
// Aufruf: listengrund <Stil> <Zielverzeichnis|-> <Pfad zur .colors> …

#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>

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

/** Relative Luminanz nach WCAG 2.1, aus sRGB. */
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

/** Die Mischung, mit der Kirigami seine Trennlinien färbt. */
QColor mixed(const QColor &ground, const QColor &text, double share)
{
    return QColor::fromRgbF(ground.redF() * (1 - share) + text.redF() * share,
                            ground.greenF() * (1 - share) + text.greenF() * share,
                            ground.blueF() * (1 - share) + text.blueF() * share);
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

const char *roleName(QPalette::ColorRole role)
{
    switch (role) {
    case QPalette::Window:
        return "Window";
    case QPalette::Base:
        return "Base";
    case QPalette::AlternateBase:
        return "AlternateBase";
    case QPalette::Button:
        return "Button";
    case QPalette::NoRole:
        return "NoRole";
    default:
        return "(andere)";
    }
}

struct Row {
    QString name;
    double alternateOnPainted = 0;
    double alternateOnBase = 0;
    double mid = 0;
    double midlight = 0;
    double dark = 0;
    double shadow = 0;
    double kirigami = 0;
};
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("Aufruf: listengrund <Stil> <Zielverzeichnis|-> <Pfad zur .colors> …\n");
        return 2;
    }

    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);
    const QString style = QString::fromLocal8Bit(argv[1]);
    app.setStyle(QStyleFactory::create(style));
    const QString directory = QString::fromLocal8Bit(argv[2]);

    Store store(sandbox.filePath(QStringLiteral("probe.db")));
    if (!store.open()) {
        printf("Store konnte nicht geöffnet werden: %s\n", qPrintable(store.lastError()));
        return 1;
    }
    addNote(store, QStringLiteral("Backup prüfen\nprune-Policy, monatliche Snapshots behalten"),
            QStringLiteral("2026-07-31T14:20:00"));
    addNote(store, QStringLiteral("Zahnarzt anrufen\nTermin für September, am besten vormittags"),
            QStringLiteral("2026-07-31T09:05:00"));
    addNote(store, QStringLiteral("Idee: Denkzettel-Export\nMarkdown mit Frontmatter, ein Ordner je Monat"),
            QStringLiteral("2026-07-30T21:40:00"));

    printf("Stil: %s\n", qPrintable(app.style()->objectName()));
    printf("Gemessen am gebauten LibraryWindow — angemeldete Rolle, gezeichneter Bildpunkt.\n\n");

    printf("%-22s %-9s %-9s %-9s %-9s %-9s  %s\n",
           "Schema", "Window", "Base", "AltBase", "gezeichn.", "= Rolle", "angemeldet");
    printf("%s\n", QByteArray(96, '-').constData());

    QList<Row> rows;

    for (int i = 3; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        const QString name = QFileInfo(path).baseName();

        KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
        app.setPalette(KColorScheme::createApplicationPalette(scheme));

        LibraryWindow window(&store);
        window.setReferenceTime(QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate));
        window.resize(900, 600);
        window.showLibrary();
        settle(200);

        auto *list = window.findChild<QListView *>();
        if (!list) {
            printf("%-22s — keine Liste gefunden\n", qPrintable(name));
            continue;
        }
        const QPalette palette = list->palette();
        const QColor windowColor = palette.color(QPalette::Active, QPalette::Window);
        const QColor baseColor = palette.color(QPalette::Active, QPalette::Base);
        const QColor alternate = palette.color(QPalette::Active, QPalette::AlternateBase);
        const QColor windowText = palette.color(QPalette::Active, QPalette::WindowText);
        const QColor textColor = palette.color(QPalette::Active, QPalette::Text);

        // Ein Punkt im unteren Bereich der Liste, links vom Text und unter dem
        // letzten Eintrag: dort liegt weder Schrift noch Auswahl, also zeigt
        // er den blanken Grund.
        const QImage picture = window.grab().toImage();
        const QPoint spot =
            list->mapTo(&window, QPoint(6, list->height() - 12)) * picture.devicePixelRatio();
        const QColor painted = picture.pixelColor(spot);

        QString matches = QStringLiteral("(keine)");
        if (painted == windowColor) {
            matches = QStringLiteral("Window");
        } else if (painted == baseColor) {
            matches = QStringLiteral("Base");
        } else if (painted == alternate) {
            matches = QStringLiteral("AltBase");
        }

        const double frameContrast =
            KConfigGroup(scheme, QStringLiteral("General")).readEntry("frameContrast", 0.20);
        // Die Linie mischt mit der Textfarbe, die auf dem gezeichneten Grund
        // steht — auf Window ist das WindowText, auf Base ist es Text.
        const QColor lineText = (painted == baseColor) ? textColor : windowText;
        const QColor kirigami = mixed(painted, lineText, frameContrast);

        Row row;
        row.name = name;
        row.alternateOnPainted = contrast(alternate, painted);
        row.alternateOnBase = contrast(alternate, baseColor);
        row.mid = contrast(palette.color(QPalette::Active, QPalette::Mid), painted);
        row.midlight = contrast(palette.color(QPalette::Active, QPalette::Midlight), painted);
        row.dark = contrast(palette.color(QPalette::Active, QPalette::Dark), painted);
        row.shadow = contrast(palette.color(QPalette::Active, QPalette::Shadow), painted);
        row.kirigami = contrast(kirigami, painted);
        rows.append(row);

        printf("%-22s %-9s %-9s %-9s %-9s %-9s  %s / autoFill=%s\n",
               qPrintable(name),
               qPrintable(windowColor.name()),
               qPrintable(baseColor.name()),
               qPrintable(alternate.name()),
               qPrintable(painted.name()),
               qPrintable(matches),
               roleName(list->viewport()->backgroundRole()),
               list->viewport()->autoFillBackground() ? "ja" : "nein");

        if (directory != QLatin1String("-")) {
            window.grab().save(QStringLiteral("%1/liste-%2-%3.png").arg(directory, style, name));
        }
    }

    printf("\n\nKontraste gegen den GEZEICHNETEN Grund\n");
    printf("%-22s %9s %9s %8s %8s %8s %8s %8s\n",
           "Schema", "AltBase", "(auf Base)", "Mid", "Midlight", "Dark", "Shadow", "Kirigami");
    printf("%s\n", QByteArray(96, '-').constData());
    Row worst;
    worst.alternateOnPainted = worst.alternateOnBase = worst.mid = worst.midlight = worst.dark =
        worst.shadow = worst.kirigami = 1e9;
    for (const Row &row : rows) {
        printf("%-22s %7.2f:1 %7.2f:1 %6.2f:1 %6.2f:1 %6.2f:1 %6.2f:1 %6.2f:1\n",
               qPrintable(row.name), row.alternateOnPainted, row.alternateOnBase, row.mid,
               row.midlight, row.dark, row.shadow, row.kirigami);
        worst.alternateOnPainted = std::min(worst.alternateOnPainted, row.alternateOnPainted);
        worst.alternateOnBase = std::min(worst.alternateOnBase, row.alternateOnBase);
        worst.mid = std::min(worst.mid, row.mid);
        worst.midlight = std::min(worst.midlight, row.midlight);
        worst.dark = std::min(worst.dark, row.dark);
        worst.shadow = std::min(worst.shadow, row.shadow);
        worst.kirigami = std::min(worst.kirigami, row.kirigami);
    }
    printf("%s\n", QByteArray(96, '-').constData());
    printf("%-22s %7.2f:1 %7.2f:1 %6.2f:1 %6.2f:1 %6.2f:1 %6.2f:1 %6.2f:1\n",
           "schlechtester Fall", worst.alternateOnPainted, worst.alternateOnBase, worst.mid,
           worst.midlight, worst.dark, worst.shadow, worst.kirigami);

    int identical = 0;
    int belowThree = 0;
    for (const Row &row : rows) {
        if (row.alternateOnPainted < 1.005) {
            ++identical;
        }
        if (row.alternateOnPainted < 1.03) {
            ++belowThree;
        }
    }
    printf("\nAlternateBase gegen den gezeichneten Grund: %d von %lld Schemata ohne messbaren\n"
           "Unterschied, %d von %lld unter 1,03:1.\n",
           identical, static_cast<long long>(rows.size()), belowThree,
           static_cast<long long>(rows.size()));

    return 0;
}
