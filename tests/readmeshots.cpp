#include "capture/capturewindow.h"
#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <QApplication>
#include <QDir>
#include <QListView>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>

/**
 * Writes the two pictures the README carries: the capture window and the
 * library.
 *
 * Not a test — a picture maker, like `libraryshots` and `editshots`, and out
 * of `add_test()` for the same reason: a picture nobody looks at proves
 * nothing, and a failing screenshot writer must not turn the suite red.
 *
 * Every note in here is invented and about nothing in particular. The
 * repository is public, and the pictures the README shows are the first thing
 * a stranger sees — they must not show anybody's own notes (customer's
 * instruction of 04.08.2026). The store is a temporary one, as in the other
 * benches; the user's database is never opened.
 *
 * Run it with QT_QPA_PLATFORMTHEME=kde. Without it Qt falls back to a
 * substitute font whose sizes are not the ones the running application uses —
 * heads and timestamps then look heavier than the note text.
 *
 * QT_SCALE_FACTOR=2 is what makes the pictures readable in the README: GitHub
 * fits an image into about 800 px of column width, so a picture taken at 1x
 * arrives downscaled and blurred. At 2x the window is the same window, only
 * measured in twice as many pixels.
 *
 * Usage: QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=2 \
 *            readmeshots <target directory>
 */
namespace
{
/** The Friday the pictures are taken on, as in the other benches. */
QDateTime friday()
{
    return QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate);
}

QDateTime at(const QString &isoDateTime)
{
    return QDateTime::fromString(isoDateTime, Qt::ISODate);
}

void addNote(Store &store, const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = at(isoDateTime);
    note.type = Note::Type::Text;
    note.state = Note::State::New;
    if (!store.addNote(note).has_value()) {
        qFatal("Notiz ließ sich nicht speichern");
    }
}

/**
 * Breeze Dark, the scheme the pictures it replaces were taken under.
 *
 * The four colours that carry the picture are the measured ones of the UX
 * investigation of 01.08.2026
 * (`docs/scrum/reviews/2026-08-01-capture-theme/palette.txt`), as in
 * `libraryshots`; the rest is derived from them. Without setting a scheme the
 * bench would draw its own default — the configuration directory is a fresh
 * temporary one on purpose, so there is no kdeglobals to read a scheme from.
 */
QPalette breezeDark()
{
    const QColor window(0x20, 0x23, 0x26);
    const QColor base(0x14, 0x16, 0x18);
    const QColor text(0xfc, 0xfc, 0xfc);
    const QColor placeholder(0xa1, 0xa9, 0xb1);

    QPalette palette;
    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, window);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, window);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::ToolTipBase, base);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::PlaceholderText, placeholder);
    palette.setColor(QPalette::Highlight, QColor(0x3d, 0xae, 0xe9));
    palette.setColor(QPalette::HighlightedText, QColor(0xfc, 0xfc, 0xfc));
    palette.setColor(QPalette::Link, QColor(0x1d, 0x99, 0xf3));

    return palette;
}

void shoot(QWidget &window, const QString &directory, const QString &name)
{
    QTest::qWait(300);

    const QPixmap picture = window.grab();
    if (!picture.save(directory + QLatin1Char('/') + name)) {
        qFatal("Bild %s ließ sich nicht schreiben", qUtf8Printable(name));
    }
    // The pixel size stands in the log because that is what decides whether the
    // picture survives the README column: it is the window size times the scale
    // factor, and a forgotten QT_SCALE_FACTOR is not visible in the picture.
    qInfo("geschrieben: %s (%dx%d px)", qUtf8Printable(name), picture.width(), picture.height());
}
}

int main(int argc, char **argv)
{
    // As in the other benches: the pictures are to show the state as shipped,
    // not the window size and splitter position whoever runs this has stored.
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    const QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
    QApplication::setPalette(breezeDark());

    if (argc < 2) {
        qFatal("Aufruf: readmeshots <Zielverzeichnis>");
    }
    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    // 1 — the capture window with a thought half written down, the picture the
    // README opens with (wireframe 1).
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        if (!store.open()) {
            qFatal("Store: %s", qUtf8Printable(store.lastError()));
        }

        CaptureWindow window(&store);
        window.showCapture();
        if (!QTest::qWaitForWindowExposed(&window)) {
            qFatal("Erfassungsfenster kam nicht auf den Schirm");
        }

        auto *text = window.findChild<QPlainTextEdit *>();
        Q_ASSERT(text);
        text->setPlainText(QStringLiteral(
            "Wanderung am Samstag: Karte ausdrucken, Brotzeit einpacken — und am Freitagabend "
            "noch einmal nach dem Wetter sehen."));
        // Where the cursor stands after typing. setPlainText() puts it back to
        // the first character, and the picture would then show a state the
        // running window never reaches.
        text->moveCursor(QTextCursor::End);

        shoot(window, directory, QStringLiteral("erfassungsfenster.png"));
    }

    // 2 — the library: five day groups on the left, a note open on the right
    // (wireframes 2b and 3a).
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        if (!store.open()) {
            qFatal("Store: %s", qUtf8Printable(store.lastError()));
        }

        addNote(store,
                QStringLiteral("Fahrradschlauch ist wieder platt. Flickzeug ist alle — im Baumarkt "
                               "zwei Ersatzschläuche mitnehmen, 28 Zoll, und die kleine Luftpumpe "
                               "gleich dazu.\n"
                               "Wenn der Mantel innen rau ist, muss der auch raus."),
                QStringLiteral("2026-07-31T15:04:00"));
        addNote(store, QStringLiteral("Kürbissuppe braucht mehr Ingwer"),
                QStringLiteral("2026-07-31T12:20:00"));
        addNote(store,
                QStringLiteral("Idee: die Fotos vom Sommer als kleines Fotobuch drucken lassen.\n"
                               "Ein Bild pro Woche reicht völlig, sonst wird das nie fertig."),
                QStringLiteral("2026-07-31T09:12:00"));
        addNote(store, QStringLiteral("Podcast über Schlafphasen weiterhören,\nab Minute 40"),
                QStringLiteral("2026-07-30T21:38:00"));
        addNote(store,
                QStringLiteral("Für den Umzug bei der Werkstatt nachfragen, ob der Anhänger übers "
                               "Wochenende frei ist"),
                QStringLiteral("2026-07-28T16:47:00"));
        addNote(store, QStringLiteral("Bildband über Straßenbahnen — hat die Bücherei den noch?"),
                QStringLiteral("2026-07-23T10:30:00"));
        addNote(store, QStringLiteral("Bücherkisten nie schwerer packen als 15 kg. Nie wieder."),
                QStringLiteral("2026-07-10T19:55:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        // Big enough for all five day groups to stand in the list at once: the
        // caption of this picture is about the grouping, and a group cut off by
        // the lower edge is exactly what it must not show.
        window.resize(1000, 760);
        window.showLibrary();
        if (!QTest::qWaitForWindowExposed(&window)) {
            qFatal("Bibliothek kam nicht auf den Schirm");
        }

        // Row 0 is the head „Heute", row 1 its first note: the reading pane is
        // to carry text, not the „no note selected" page — that is the half of
        // the window the picture is captioned for.
        auto *list = window.findChild<QListView *>();
        Q_ASSERT(list);
        list->setCurrentIndex(list->model()->index(1, 0));

        shoot(window, directory, QStringLiteral("bibliothek.png"));
    }

    return 0;
}
