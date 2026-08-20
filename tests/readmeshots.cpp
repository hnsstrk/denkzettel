#include "capture/capturewindow.h"
#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QListView>
#include <QLocale>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>

/**
 * Writes the two pictures the README carries: the capture window and the
 * library.
 *
 * Not a test — a picture maker, and out of `add_test()` for that reason: a
 * picture nobody looks at proves nothing, and a failing screenshot writer must
 * not turn the suite red.
 *
 * Every note in here is invented and about nothing in particular. The
 * repository is public, and the pictures the README shows are the first thing
 * a stranger sees — they must not show anybody's own notes (user's
 * instruction of 04.08.2026). The store is a temporary one; the user's database
 * is never opened.
 *
 * One run writes **one** language set: `README.md` carries the English one,
 * `README.de.md` the German one from `docs/images/de/`. Which one comes out is
 * decided by the environment, see germanPictures().
 *
 * Run it with QT_QPA_PLATFORMTHEME=kde. Without it Qt falls back to a
 * substitute font whose sizes are not the ones the running application uses —
 * heads and timestamps then look heavier than the note text.
 *
 * QT_SCALE_FACTOR is what makes the pictures readable in the README: GitHub
 * fits an image into about 800 px of column width, so a picture taken at 1x
 * arrives downscaled and blurred. Above 1x it is the same window, only measured
 * in more pixels; the pictures in the repository are taken at the user's
 * scaling of 1.5.
 *
 * Usage: QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *            readmeshots <target directory>
 *
 * That call alone is not enough for a picture one can use. The runner throws
 * away its own configuration directory (see main() below) and therefore finds
 * no `plasmarc`, so the capture window draws the shell of the default desktop
 * theme — a light one — while this file sets a dark palette, and the note text
 * comes out light on light. The README section "Screenshots" carries the two
 * calls that produce the pictures actually committed here, with the desktop
 * theme and the message catalogue set.
 */
namespace
{
/** The Friday the pictures are taken on. */
QDateTime friday()
{
    return QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate);
}

QDateTime at(const QString &isoDateTime)
{
    return QDateTime::fromString(isoDateTime, Qt::ISODate);
}

// Healing this means changing the signature or introducing a type of its own,
// which is design rather than tidying up (issue #76). The one case a mix-up
// would be visible in - placeholderPage() in the empty library - gets a test
// assurance instead, as issue #88.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void addNote(Store &store, const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = at(isoDateTime);
    note.type = Note::Type::Text;
    note.state = Note::State::New;
    if (!store.addNote(note).has_value()) {
        qFatal("The note could not be stored");
    }
}

/**
 * Whether this run writes the German set of pictures.
 *
 * Three things have to agree in one picture, and each has its own source: the
 * interface strings come from the message catalogue (`LANGUAGE`), the
 * timestamps come from `QLocale` (`LANG`/`LC_ALL`), and the invented notes
 * below come from here. Read from the locale so that the notes follow the
 * timestamps; whoever starts the runner sets both variables together, see the
 * README section on the pictures. Getting this wrong produces the worst kind of
 * picture — one that looks plausible and shows an English window with German
 * notes standing in it.
 */
bool germanPictures()
{
    return QLocale().language() == QLocale::German;
}

/** The half-written thought the capture window shows. */
QString captureText()
{
    if (germanPictures()) {
        return QStringLiteral(
            "Wanderung am Samstag: Karte ausdrucken, Brotzeit einpacken — und am Freitagabend "
            "noch einmal nach dem Wetter sehen.");
    }
    return QStringLiteral(
        "Hike on Saturday: print the map, pack something to eat — and take another look at the "
        "weather on Friday evening.");
}

/** One entry of the invented sample set: what it says and when it was written. */
struct SampleNote {
    QString content;
    QString isoDateTime;
};

/**
 * The notes the library picture shows, seven of them across five day groups.
 *
 * Both sets are invented and about nothing in particular. The repository is
 * public and these pictures are the first thing a stranger sees — they must not
 * show anybody's own notes (user's instruction of 04.08.2026).
 *
 * The two sets are not translations of each other but counterparts: what
 * carries the picture is the *shape* — one long note that fills the reading
 * pane, one that stays on a single line, two that run into a second line the
 * list has to elide. A set that loses that shape shows a different window.
 */
QList<SampleNote> sampleNotes()
{
    if (germanPictures()) {
        return {
            {QStringLiteral("Fahrradschlauch ist wieder platt. Flickzeug ist alle — im Baumarkt "
                            "zwei Ersatzschläuche mitnehmen, 28 Zoll, und die kleine Luftpumpe "
                            "gleich dazu.\n"
                            "Wenn der Mantel innen rau ist, muss der auch raus."),
             QStringLiteral("2026-07-31T15:04:00")},
            {QStringLiteral("Kürbissuppe braucht mehr Ingwer"),
             QStringLiteral("2026-07-31T12:20:00")},
            {QStringLiteral("Idee: die Fotos vom Sommer als kleines Fotobuch drucken lassen.\n"
                            "Ein Bild pro Woche reicht völlig, sonst wird das nie fertig."),
             QStringLiteral("2026-07-31T09:12:00")},
            {QStringLiteral("Podcast über Schlafphasen weiterhören,\nab Minute 40"),
             QStringLiteral("2026-07-30T21:38:00")},
            {QStringLiteral("Für den Umzug bei der Werkstatt nachfragen, ob der Anhänger übers "
                            "Wochenende frei ist"),
             QStringLiteral("2026-07-28T16:47:00")},
            {QStringLiteral("Bildband über Straßenbahnen — hat die Bücherei den noch?"),
             QStringLiteral("2026-07-23T10:30:00")},
            {QStringLiteral("Bücherkisten nie schwerer packen als 15 kg. Nie wieder."),
             QStringLiteral("2026-07-10T19:55:00")},
        };
    }

    return {
        {QStringLiteral("Bike tube is flat again. The puncture kit is empty — pick up two spare "
                        "tubes at the hardware store, 28 inch, and the small pump along with "
                        "them.\n"
                        "If the tyre is rough on the inside, that one has to come off as well."),
         QStringLiteral("2026-07-31T15:04:00")},
        {QStringLiteral("Pumpkin soup needs more ginger"), QStringLiteral("2026-07-31T12:20:00")},
        {QStringLiteral("Idea: have the summer photos printed as a small photo book.\n"
                        "One picture a week is plenty, otherwise it will never be finished."),
         QStringLiteral("2026-07-31T09:12:00")},
        {QStringLiteral("Keep listening to the podcast about sleep phases,\nfrom minute 40"),
         QStringLiteral("2026-07-30T21:38:00")},
        {QStringLiteral("Ask the garage about the move, whether the trailer is free over the "
                        "weekend"),
         QStringLiteral("2026-07-28T16:47:00")},
        {QStringLiteral("Picture book about trams — does the library still have it?"),
         QStringLiteral("2026-07-23T10:30:00")},
        {QStringLiteral("Never pack book boxes heavier than 15 kg. Never again."),
         QStringLiteral("2026-07-10T19:55:00")},
    };
}

/**
 * Breeze Dark, the scheme the pictures it replaces were taken under.
 *
 * The four colours that carry the picture are the measured ones of the UX
 * investigation of 01.08.2026
 * (`Palettenmessung vom 01.08.2026`); the rest is derived from them. Without
 * setting a scheme the runner would draw its own default — the configuration
 * directory is a fresh
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
        qFatal("Picture %s could not be written", qUtf8Printable(name));
    }
    // The pixel size stands in the log because that is what decides whether the
    // picture survives the README column: it is the window size times the scale
    // factor, and a forgotten QT_SCALE_FACTOR is not visible in the picture.
    qInfo("written: %s (%dx%d px)", qUtf8Printable(name), picture.width(), picture.height());
}
}

int main(int argc, char **argv)
{
    // As in the other benches: the pictures are to show the state as shipped,
    // not the window size and splitter position whoever runs this has stored.
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
    // As in main.cpp, and for the same reason as the palette above: the
    // pictures are to show the state as shipped. Since the source strings are
    // English, that state depends on the message catalogue — without this line
    // no catalogue is ever consulted and every picture comes out English, no
    // matter what LANG says.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));
    QApplication::setPalette(breezeDark());

    if (argc < 2) {
        qFatal("Usage: readmeshots <target directory>");
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
            qFatal("The capture window never reached the screen");
        }

        auto *text = window.findChild<QPlainTextEdit *>();
        Q_ASSERT(text);
        text->setPlainText(captureText());
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

        const QList<SampleNote> notes = sampleNotes();
        for (const SampleNote &note : notes) {
            addNote(store, note.content, note.isoDateTime);
        }

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        // Big enough for all five day groups to stand in the list at once: the
        // caption of this picture is about the grouping, and a group cut off by
        // the lower edge is exactly what it must not show.
        window.resize(1000, 760);
        window.showLibrary();
        if (!QTest::qWaitForWindowExposed(&window)) {
            qFatal("The library never reached the screen");
        }

        // Row 0 is the head "Today", row 1 its first note: the reading pane is
        // to carry text, not the "no note selected" page — that is the half of
        // the window the picture is captioned for.
        auto *list = window.findChild<QListView *>();
        Q_ASSERT(list);
        list->setCurrentIndex(list->model()->index(1, 0));

        shoot(window, directory, QStringLiteral("bibliothek.png"));
    }

    return 0;
}
