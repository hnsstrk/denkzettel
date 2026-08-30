#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QFontMetrics>
#include <QListView>
#include <QSplitter>
#include <QPixmap>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>
#include <QTreeWidget>

/**
 * The pictures of issue #133: the category column with and without the row
 * "Waiting for analysis".
 *
 * Not a test and out of `add_test()`, for the reason `readmeshots` is out of
 * it: a broken picture writer must not turn the suite red. It is built with
 * the suite all the same, because a runner nobody rebuilds ages unnoticed and
 * then writes plausible pictures of an **old** state with a fresh timestamp
 * (CLAUDE.md, rule 4).
 *
 * **The numbers beside the pictures are the point, not decoration.** The tree's
 * viewport and the sidebar around it both stand on `QPalette::Window`
 * (issue #135), so the two areas share a ground and the picture cannot say
 * where one ends and the other begins — a statement about **space** therefore
 * needs the geometry printed next to it, never the picture alone. The same for
 * the counter: `text(1)` answers correctly even when the number stands outside
 * the viewport, which is exactly how this column once showed a list of
 * headings with no counters at all (finding 51). So every row is printed with
 * its counter, its width and its rectangle in device pixels, and the section
 * widths beside them.
 *
 * Every note here is invented. The repository is public and a note is personal
 * data, so no run of this may ever take its material out of the session
 * somebody is working in.
 *
 * Usage — the environment is not optional, see rule 2 and finding 28:
 *
 *   cmake --build build --target categoryshots
 *   env -u LANGUAGE LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/categoryshots docs/images/reviews
 */
namespace
{
/** The floor of `MinimumSidebarWidth` in librarywindow.cpp, which is file-local there. */
constexpr int MinimumSidebarWidth = 120;

/**
 * One note in the state the picture needs it in.
 *
 * `addNote()` writes it and `updateNote()` sets what the capture road cannot
 * produce — a used-up attempt counter, and the empty text of a voice note whose
 * transcript has not arrived.
 */
// The three fields are the columns of one note, and a type of their own for
// three values used once would be the abstraction nobody asked for.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void add(Store &store, const QString &text, const QString &iso, const QString &category,
         Note::State state, int attempts)
{
    Note note;
    note.createdAt = QDateTime::fromString(iso, Qt::ISODate);
    note.type = Note::Type::Text;
    note.content = text.isEmpty() ? QStringLiteral("placeholder") : text;
    const std::optional<qint64> id = store.addNote(note);
    if (!id.has_value()) {
        qFatal("addNote: %s", qUtf8Printable(store.lastError()));
    }
    const std::optional<Note> stored = store.note(*id);
    if (!stored.has_value()) {
        qFatal("note: %s", qUtf8Printable(store.lastError()));
    }
    Note written = *stored;
    written.content = text;
    written.category = category;
    written.state = state;
    written.analysisAttempts = attempts;
    if (!store.updateNote(written)) {
        qFatal("updateNote: %s", qUtf8Printable(store.lastError()));
    }
}

void shoot(QWidget &window, const QString &file)
{
    const QPixmap picture = window.grab();
    if (!picture.save(file)) {
        qFatal("could not write %s", qUtf8Printable(file));
    }
    qWarning("%s  %d x %d", qUtf8Printable(file), picture.width(), picture.height());
}

/**
 * Prints what the column shows, and adds it up.
 *
 * The sum is taken over the rows that are **shown**, which is the number the
 * customer counted by hand on 29.08.2026 — 23 beside "All" over rows coming to
 * 20. A counter read off a hidden row would not be in that sum, and a counter
 * standing outside the viewport would be in it while the eye cannot find it;
 * hence the widths beside every value.
 */
void report(const QString &name, const QWidget &window, const QTreeWidget *column)
{
    const qreal ratio = window.devicePixelRatioF();
    int all = 0;
    int belowAll = 0;
    for (int row = 0; row < column->topLevelItemCount(); ++row) {
        const QTreeWidgetItem *item = column->topLevelItem(row);
        const int count = QLocale().toInt(item->text(1));
        if (row == 0) {
            all = count;
        } else if (!item->isHidden()) {
            belowAll += count;
        }
        const QRect rect = column->visualItemRect(item);
        qWarning("%s  row %d %-24s counter=%-4s hidden=%d device y=%d..%d",
                 qUtf8Printable(name), row, qUtf8Printable(item->text(0)),
                 qUtf8Printable(item->text(1)), int(item->isHidden()),
                 int(rect.top() * ratio), int(rect.bottom() * ratio));
    }
    // The statement about space, which the picture cannot carry because the
    // sidebar and the tree's viewport share their ground colour.
    qWarning("%s  sidebar=%d tree=%d viewport=%d label=%d counter=%d", qUtf8Printable(name),
             column->parentWidget()->width(), column->width(), column->viewport()->width(),
             column->columnWidth(0), column->columnWidth(1));
    // The identity the column promises. It comes out different in the last
    // picture on purpose — that population is the one the promise fails for.
    qWarning("%s  all=%d sumOfShownRows=%d %s", qUtf8Printable(name), all, belowAll,
             all == belowAll ? "EQUAL" : "DIFFERENT");
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        qFatal("usage: categoryshots <target directory>");
    }

    // A configuration directory of its own, and a real colour scheme in it
    // **before** QApplication: without a kdeglobals the platform theme and
    // KColorScheme read two different sources, and the picture then shows a
    // fault of the runner (CLAUDE.md, finding 38).
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());
    QFile::copy(QStringLiteral("/usr/share/color-schemes/BreezeDark.colors"),
                configuration.path() + QStringLiteral("/kdeglobals"));
    QFile scheme(configuration.path() + QStringLiteral("/kdeglobals"));
    if (scheme.open(QIODevice::Append)) {
        scheme.write("\n[General]\nColorScheme=BreezeDark\n");
        scheme.close();
    }
    QFile plasma(configuration.path() + QStringLiteral("/plasmarc"));
    if (plasma.open(QIODevice::WriteOnly)) {
        plasma.write("[Theme]\nname=breeze-dark\n");
        plasma.close();
    }

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("denkzettel"));
    // Without the domain every i18n() call falls back to the source language,
    // and a German run then writes an English picture without saying so — the
    // run would look like a catalogue that was never translated.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Read back what the run really drew with, rather than trusting that the
    // variables were set (findings 28 and 38).
    qWarning("style: %s  scale: %g", qUtf8Printable(app.style()->objectName()),
             qreal(app.devicePixelRatio()));
    qWarning("palette Window %s Base %s Text %s PlaceholderText %s",
             qUtf8Printable(app.palette().color(QPalette::Window).name()),
             qUtf8Printable(app.palette().color(QPalette::Base).name()),
             qUtf8Printable(app.palette().color(QPalette::Text).name()),
             qUtf8Printable(app.palette().color(QPalette::PlaceholderText).name()));

    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    // Four populations, each one note further than the one before it, so that
    // the pictures come out **different** from one another (finding 10). The
    // counters below are what the eye is meant to add up.
    const QStringList names{QStringLiteral("133-nichts-wartet.png"),
                            QStringLiteral("133-wartet.png"),
                            QStringLiteral("133-wartet-und-aufgegeben.png"),
                            QStringLiteral("133-sprachnotiz-ohne-transkript.png")};

    for (int population = 0; population < names.size(); ++population) {
        const QTemporaryDir data;
        Store store(data.path() + QStringLiteral("/denkzettel.db"));
        if (!store.open()) {
            qFatal("store: %s", qUtf8Printable(store.lastError()));
        }

        // Classified, and therefore in a category row: the state a library is
        // in once the analysis has been through it.
        add(store, QStringLiteral("Ask about the delivery date before the end of the month."),
            QStringLiteral("2026-07-31T15:04:00"), QStringLiteral("todos"),
            Note::State::Analysed, 0);
        add(store, QStringLiteral("Idea: have the summer photos printed as a photo book."),
            QStringLiteral("2026-07-31T09:12:00"), QStringLiteral("ideen"),
            Note::State::Analysed, 0);
        add(store, QStringLiteral("journalctl -u whisperd --since today"),
            QStringLiteral("2026-07-30T21:38:00"), QStringLiteral("cli"), Note::State::Analysed, 0);

        // Waiting: no category, and attempts left. This is the note that stood
        // in "All" and in no row below it before this issue.
        if (population >= 1) {
            add(store, QStringLiteral("Never pack book boxes heavier than 15 kg."),
                QStringLiteral("2026-07-30T08:15:00"), QString(), Note::State::New, 0);
        }
        // Given up on: the attempts are used up, and the text is there. The row
        // below keeps meaning exactly this (issue #118).
        if (population >= 2) {
            add(store, QStringLiteral("Keep listening to the podcast, from minute 40"),
                QStringLiteral("2026-07-29T19:20:00"), QString(), Note::State::New,
                Store::analysisAttemptLimit);
        }
        // A voice note whose transcript has not arrived: no text to classify,
        // so `TRIM(content) != ''` keeps it out of the given-up row and it
        // belongs to the waiting ones — for a second reason than the others.
        if (population >= 3) {
            add(store, QString(), QStringLiteral("2026-07-29T08:05:00"), QString(),
                Note::State::New, Store::analysisAttemptLimit);
        }

        LibraryWindow window(&store);
        window.resize(900, 700);
        window.showLibrary();
        if (!QTest::qWaitForWindowExposed(&window)) {
            qFatal("the library never reached the screen");
        }
        window.resize(900, 700);
        QTest::qWait(200);

        const auto *column = window.findChild<QTreeWidget *>();
        if (!column) {
            qFatal("the library has no category column");
        }
        report(names.at(population), window, column);
        shoot(window, directory + QLatin1Char('/') + names.at(population));

        // The last population once more with the new row **chosen**: it is a
        // filter like any other row, and a picture of a column nobody clicked
        // says nothing about that. The note count beside it is what makes the
        // picture readable — the list has to hold exactly the notes the counter
        // counted, and a wrong set would look like a plausible list.
        if (population == names.size() - 1) {
            auto *mutableColumn = const_cast<QTreeWidget *>(column);
            QTreeWidgetItem *waiting = mutableColumn->topLevelItem(6);
            mutableColumn->setCurrentItem(waiting);
            QTest::qWait(200);
            const auto *list = window.findChild<QListView *>();
            // The **note** count and not the row count: the list carries a head
            // row per day group, so rowCount() answers one more than the column
            // counted and reads like a filter letting a stranger through.
            const auto *model = qobject_cast<NoteListModel *>(list->model());
            qWarning("133-wartet-gewaehlt.png  chosen=%s counter=%s notesInTheList=%d rows=%d",
                     qUtf8Printable(waiting->text(0)), qUtf8Printable(waiting->text(1)),
                     model ? model->noteCount() : -1, list->model()->rowCount());
            shoot(window, directory + QStringLiteral("/133-wartet-gewaehlt.png"));

            // The floor of MinimumSidebarWidth, **measured** and not read out
            // of the header configuration (finding 50: a configuration is a
            // statement of intent until somebody measures it). The claim under
            // test is that at 120 px section 1 keeps its content width while
            // the stretched label section shrinks — the opposite of the fault
            // issue #18 fixed, where the counters were pushed out of the
            // viewport.
            //
            // Both digit counts, because the four-digit one is where the old
            // definition of the constant broke: the counter of a category with
            // a thousand notes in it is wider, and it is the counter this
            // column asserts (finding 51). Written into the item rather than
            // grown out of a thousand notes — `updateCategoryCounts()` puts
            // exactly this string there through QLocale().toString().
            auto *splitter = window.findChild<QSplitter *>();
            for (const char *digits : {"123", "1234"}) {
                for (int row = 0; row < mutableColumn->topLevelItemCount(); ++row) {
                    QTreeWidgetItem *item = mutableColumn->topLevelItem(row);
                    if (!item->isHidden()) {
                        item->setText(1, QString::fromUtf8(digits));
                    }
                }
                splitter->setSizes({MinimumSidebarWidth, 300, 900 - 300 - MinimumSidebarWidth});
                mutableColumn->header()->resizeSections(QHeaderView::ResizeToContents);
                mutableColumn->header()->setSectionResizeMode(0, QHeaderView::Stretch);
                mutableColumn->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
                QTest::qWait(200);

                // The counter has to be **complete and inside the viewport**,
                // which is two statements: the section is wide enough for the
                // string, and the section ends before the viewport does.
                const QFontMetrics metrics(mutableColumn->font());
                const int needed = metrics.horizontalAdvance(QString::fromUtf8(digits));
                const int right = mutableColumn->columnWidth(0) + mutableColumn->columnWidth(1);
                qWarning("133-schmal-%s  sidebar=%d label=%d counter=%d textNeeds=%d "
                         "rightEdge=%d viewport=%d fits=%d inside=%d",
                         digits, splitter->sizes().at(0), mutableColumn->columnWidth(0),
                         mutableColumn->columnWidth(1), needed, right,
                         mutableColumn->viewport()->width(),
                         int(needed <= mutableColumn->columnWidth(1)),
                         int(right <= mutableColumn->viewport()->width()));
                shoot(window, directory + QStringLiteral("/133-schmal-") + QLatin1StringView(digits)
                          + QStringLiteral(".png"));
            }
        }
    }

    return 0;
}
