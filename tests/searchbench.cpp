/*
 * What a broad search costs at a large corpus (issue #78).
 *
 * No add_test(): this measures, it does not assert. It builds its own corpus
 * in a temporary directory — the user's database is never touched — and prints
 * a table.
 *
 * Every number comes with the control that says whether the result count is
 * what drives it: on one and the same corpus a term that matches every note
 * stands beside a term that matches fifty. Without that pair a slow run would
 * only say "20,000 notes are slow", not "the unbounded result list is".
 *
 * Run it the way the pictures are run, so the type is the user's:
 *   QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       ./build/bin/searchbench
 */

#include "platform/systemfonts.h"
#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QLineEdit>
#include <QListView>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>
#include <QVariant>

#include <algorithm>
#include <cstdio>
#include <utility>

namespace
{
/**
 * The reference time every grouping is measured against, so the run repeats.
 *
 * In a function rather than beside one: a QDateTime at namespace scope would be
 * built before main(), and clazy says so (`non-pod-global-static`).
 */
const QDateTime &now()
{
    static const QDateTime reference =
        QDateTime::fromString(QStringLiteral("2026-08-28T12:00:00.000"), Qt::ISODateWithMs);
    return reference;
}

/** In every note, so a three-character term matches the whole corpus. */
constexpr const char *BroadWord = "Notiz";

/** In every four-hundredth note — the control with the same corpus behind it. */
constexpr const char *RareWord = "Kolibri";

QStringList wordPool()
{
    return QStringLiteral("Backup Termin Rechnung Skizze Gedanke Straßenbahn Besprechung Größe Einkauf"
                          " Fotografieren Werkstatt Anruf Angebot Vertrag Rückruf Küche Übergabe Prüfung"
                          " Ablage Kalender Entwurf Notizbuch Werkzeug Auswertung Umzug Schlüssel")
        .split(QLatin1Char(' '));
}

/**
 * A note of roughly the length the SPEC's index measurement assumes — about
 * 190 characters, a subject line and two lines under it.
 */
QString noteText(QRandomGenerator *random, const QStringList &pool, int index)
{
    QStringList lines;
    for (int line = 0; line < 3; ++line) {
        QStringList words;
        const int count = 6 + static_cast<int>(random->bounded(5));
        for (int word = 0; word < count; ++word) {
            words.append(pool.at(static_cast<qsizetype>(random->bounded(static_cast<quint32>(pool.size())))));
        }
        lines.append(words.join(QLatin1Char(' ')));
    }

    lines[0] = QStringLiteral("%1 %2").arg(QString::fromLatin1(BroadWord), lines.at(0));
    if (index % 400 == 0) {
        lines[1] = QStringLiteral("%1 %2").arg(QString::fromLatin1(RareWord), lines.at(1));
    }

    return lines.join(QLatin1Char('\n'));
}

/**
 * Fills the store's database with `count` notes over a second connection and
 * in one transaction.
 *
 * Store::addNote() writes one transaction per note; twenty thousand of those
 * are minutes of fsync and measure nothing this bench asks about. The rows are
 * the same rows — the FTS index is filled by the triggers of the schema, not
 * by addNote().
 */
void seed(const QString &databasePath, int count)
{
    const QString connection = QStringLiteral("searchbench-seed");
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        db.setDatabaseName(databasePath);
        if (!db.open()) {
            qFatal("seed: %s", qPrintable(db.lastError().text()));
        }

        QRandomGenerator random(42U);
        const QStringList pool = wordPool();

        db.transaction();
        QSqlQuery insert(db);
        insert.prepare(QStringLiteral("INSERT INTO notes (created_at, type, content, state)"
                                      " VALUES (:created_at, 'text', :content, 'neu')"));
        // Three years back from the reference time, so the list carries every
        // group the window knows and a few hundred day heads.
        const QDateTime oldest = now().addDays(-1095);
        for (int index = 0; index < count; ++index) {
            const QDateTime created = oldest.addSecs(static_cast<qint64>(index) * (1095LL * 86400LL) / count);
            insert.bindValue(QStringLiteral(":created_at"), created.toString(Qt::ISODateWithMs));
            insert.bindValue(QStringLiteral(":content"), noteText(&random, pool, index));
            if (!insert.exec()) {
                qFatal("seed: %s", qPrintable(insert.lastError().text()));
            }
        }
        if (!db.commit()) {
            qFatal("seed commit: %s", qPrintable(db.lastError().text()));
        }
    }
    QSqlDatabase::removeDatabase(connection);
}

/** Resident set size of this process in MiB. */
double residentMiB()
{
    QFile status(QStringLiteral("/proc/self/status"));
    if (!status.open(QIODevice::ReadOnly)) {
        return -1;
    }
    // Not atEnd(): a file under /proc reports size 0, and QFile calls that the
    // end before the first line is read.
    while (true) {
        const QString line = QString::fromLatin1(status.readLine());
        if (line.isEmpty()) {
            break;
        }
        if (line.startsWith(QLatin1String("VmRSS:"))) {
            // "VmRSS:\t   12345 kB" — the separator is a tab, not a space.
            return line.mid(6).remove(QLatin1String("kB")).trimmed().toDouble() / 1024.0;
        }
    }
    return -1;
}

template<typename Work>
double medianMs(int runs, Work work)
{
    QList<double> times;
    for (int run = 0; run < runs; ++run) {
        QElapsedTimer timer;
        timer.start();
        work();
        times.append(static_cast<double>(timer.nsecsElapsed()) / 1e6);
    }
    std::sort(times.begin(), times.end());
    return times.at(times.size() / 2);
}

/**
 * The same rows over a connection of this bench's own — once with a `LIMIT`,
 * once without, and the columns read by index instead of by name.
 *
 * Two questions in one function. What would a `LIMIT` on Store::search() buy?
 * And how much of the unlimited time is SQLite at all — `QSqlQuery::value(QString)`
 * looks the name up in the record for every column of every row, eleven times
 * per note.
 */
double rawQueryMs(const QString &databasePath, const char *match, int limit, bool byName)
{
    const QString connection = QStringLiteral("searchbench-raw");
    double result = 0;
    int rows = 0;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        db.setDatabaseName(databasePath);
        db.open();
        const QStringList columns = QStringLiteral("id created_at type content audio_path audio_duration_s"
                                                   " category state needs_reembed analysis_attempts"
                                                   " analysis_last_error")
                                        .split(QLatin1Char(' '));
        result = medianMs(5, [&] {
            QSqlQuery query(db);
            query.prepare(QStringLiteral("SELECT %1 FROM notes WHERE id IN"
                                         " (SELECT rowid FROM notes_fts WHERE notes_fts MATCH :match)"
                                         " ORDER BY created_at DESC, id DESC%2")
                              .arg(columns.join(QStringLiteral(", ")),
                                   limit > 0 ? QStringLiteral(" LIMIT :limit") : QString()));
            query.bindValue(QStringLiteral(":match"), QStringLiteral("\"%1\"").arg(QString::fromLatin1(match)));
            if (limit > 0) {
                query.bindValue(QStringLiteral(":limit"), limit);
            }
            query.exec();
            rows = 0;
            while (query.next()) {
                for (int column = 0; column < columns.size(); ++column) {
                    const QVariant value = byName ? query.value(columns.at(column)) : query.value(column);
                    (void)value.toString();
                }
                ++rows;
            }
        });
    }
    QSqlDatabase::removeDatabase(connection);
    if (rows == 0) {
        qFatal("query returned no row");
    }
    return result;
}

void line(const QString &text)
{
    QTextStream(stdout) << text << '\n';
    fflush(stdout);
}

/** One corpus, from building it to the shown window. */
void measure(int count)
{
    const QTemporaryDir directory;
    const QString databasePath = directory.filePath(QStringLiteral("denkzettel.db"));

    Store store(databasePath);
    if (!store.open()) {
        qFatal("open: %s", qPrintable(store.lastError()));
    }
    seed(databasePath, count);

    const double afterSeed = residentMiB();
    const double databaseMiB = static_cast<double>(QFile(databasePath).size()) / (1024.0 * 1024.0);

    line(QString());
    line(QStringLiteral("=== %1 notes ===").arg(count));
    line(QStringLiteral("Database on disk: %1 MiB").arg(databaseMiB, 0, 'f', 1));

    // --- the store on its own -------------------------------------------
    int all = 0;
    const double emptyMs = medianMs(5, [&] {
        all = static_cast<int>(store.search(QString()).size());
    });
    int broad = 0;
    const double broadMs = medianMs(5, [&] {
        broad = static_cast<int>(store.search(QString::fromLatin1(BroadWord)).size());
    });
    int rare = 0;
    const double rareMs = medianMs(5, [&] {
        rare = static_cast<int>(store.search(QString::fromLatin1(RareWord)).size());
    });
    int oneChar = 0;
    const double oneCharMs = medianMs(5, [&] {
        oneChar = static_cast<int>(store.search(QStringLiteral("n")).size());
    });
    int twoChar = 0;
    const double twoCharMs = medianMs(5, [&] {
        twoChar = static_cast<int>(store.search(QStringLiteral("no")).size());
    });

    line(QStringLiteral("Store::search"));
    line(QStringLiteral("  empty (all notes)       %1 hits  %2 ms").arg(all, 6).arg(emptyMs, 8, 'f', 1));
    line(QStringLiteral("  \"%1\" (trigram, broad) %2 hits  %3 ms")
             .arg(QString::fromLatin1(BroadWord))
             .arg(broad, 6)
             .arg(broadMs, 8, 'f', 1));
    line(QStringLiteral("  \"%1\" (control)       %2 hits  %3 ms")
             .arg(QString::fromLatin1(RareWord))
             .arg(rare, 6)
             .arg(rareMs, 8, 'f', 1));
    line(QStringLiteral("  \"n\"  (LIKE, 1 character)%1 hits  %2 ms").arg(oneChar, 6).arg(oneCharMs, 8, 'f', 1));
    line(QStringLiteral("  \"no\" (LIKE, 2 characters)%1 hits %2 ms").arg(twoChar, 6).arg(twoCharMs, 8, 'f', 1));
    line(QStringLiteral("  same query, columns by name             %1 ms")
             .arg(rawQueryMs(databasePath, BroadWord, 0, true), 8, 'f', 1));
    line(QStringLiteral("  same query, columns by index            %1 ms")
             .arg(rawQueryMs(databasePath, BroadWord, 0, false), 8, 'f', 1));
    line(QStringLiteral("  same query with LIMIT 200               %1 ms")
             .arg(rawQueryMs(databasePath, BroadWord, 200, true), 8, 'f', 1));

    const double afterQuery = residentMiB();

    // --- the whole way into the window ----------------------------------
    LibraryWindow window(&store);
    window.setReferenceTime(now());
    window.resize(1100, 700);

    auto *search = window.findChild<QLineEdit *>();
    auto *list = window.findChild<QListView *>();

    // A repaint after the keystroke, because the view lays its rows out
    // lazily: without it the measurement would stop before the work the user
    // waits for. It is synchronous, so what it costs is inside the number.
    const auto type = [&](const QString &text) {
        search->setText(text);
        QCoreApplication::processEvents();
        list->viewport()->repaint();
    };

    // Hidden first, so the second and third run measure the same road as the
    // first: on a window that is already up, showLibrary() reads the store and
    // leaves the row layout to the next paint, which would fall outside the
    // clock.
    const double showMs = medianMs(3, [&] {
        window.hide();
        window.showLibrary();
        if (!QTest::qWaitForWindowExposed(&window)) {
            qFatal("window never became visible");
        }
        list->viewport()->repaint();
    });

    line(QStringLiteral("LibraryWindow (store + model + list, until the picture stands)"));
    line(QStringLiteral("  showLibrary(), whole library          %1 ms").arg(showMs, 8, 'f', 1));

    // Single transitions, the state before each one set outside the clock: a
    // measurement that clears the field first would carry two list builds and
    // read as twice the cost of one.
    type(QString::fromLatin1(RareWord));
    const double toBroadMs = medianMs(3, [&] {
        type(QString::fromLatin1(BroadWord));
        type(QString::fromLatin1(RareWord));
    }) ;
    type(QString::fromLatin1(BroadWord));
    const double toRareMs = medianMs(3, [&] {
        type(QString::fromLatin1(RareWord));
        type(QString::fromLatin1(BroadWord));
    });
    type(QString::fromLatin1(RareWord));
    const double toAllMs = medianMs(3, [&] {
        type(QString());
        type(QString::fromLatin1(RareWord));
    });

    line(QStringLiteral("  50 hits -> all hits and back         %1 ms").arg(toBroadMs, 8, 'f', 1));
    line(QStringLiteral("  all hits -> 50 hits and back         %1 ms").arg(toRareMs, 8, 'f', 1));
    line(QStringLiteral("  50 hits -> empty field and back      %1 ms").arg(toAllMs, 8, 'f', 1));

    const double withWindow = residentMiB();

    // --- where that time goes -------------------------------------------
    // The same list once without a view behind it and once with one: the
    // difference is what the row layout of QListView costs, and only that
    // difference can be bought with a LIMIT on the query.
    const QList<Note> everything = store.search(QString());
    NoteListModel bare;
    const double modelMs = medianMs(3, [&] {
        bare.setNotes(everything, now());
    });

    NoteListModel attached;
    QListView plain;
    plain.setModel(&attached);
    plain.setItemDelegate(new NoteListDelegate(&plain));
    plain.resize(600, 700);
    plain.show();
    if (!QTest::qWaitForWindowExposed(&plain)) {
        qFatal("list never became visible");
    }
    const double viewMs = medianMs(3, [&] {
        attached.setNotes(everything, now());
        QCoreApplication::processEvents();
        plain.viewport()->repaint();
    });

    const int rows = attached.rowCount();
    const double readFontMs = medianMs(3, [&] {
        for (int row = 0; row < rows; ++row) {
            const QFont font = platform::smallestReadableFont();
            (void)QFontMetrics(font).height();
        }
    });
    const QFont held = platform::smallestReadableFont();
    const double heldFontMs = medianMs(3, [&] {
        for (int row = 0; row < rows; ++row) {
            (void)QFontMetrics(held).height();
        }
    });

    line(QStringLiteral("Where the time goes (%1 rows, group heads counted in)").arg(rows));
    line(QStringLiteral("  Store::search on its own             %1 ms").arg(emptyMs, 8, 'f', 1));
    line(QStringLiteral("  setNotes() without a view            %1 ms").arg(modelMs, 8, 'f', 1));
    line(QStringLiteral("  setNotes() on a shown QListView      %1 ms").arg(viewMs, 8, 'f', 1));
    line(QStringLiteral("  %1x smallestReadableFont() + metrics %2 ms").arg(rows, 6).arg(readFontMs, 8, 'f', 1));
    line(QStringLiteral("  %1x metrics alone, font held         %2 ms").arg(rows, 6).arg(heldFontMs, 8, 'f', 1));

    line(QStringLiteral("Memory: after filling %1 MiB, after the query %2 MiB, with the window %3 MiB")
             .arg(afterSeed, 0, 'f', 1)
             .arg(afterQuery, 0, 'f', 1)
             .arg(withWindow, 0, 'f', 1));

    window.close();
}
}

int main(int argc, char **argv)
{
    const QApplication app(argc, argv);

    // The same line main.cpp carries, and here for the same reason as there:
    // it is what puts the watch on `kdeglobals`, and only under that watch do
    // the two font functions hold their answer instead of reading the file
    // once per row (issues #68, #110). A bench without it would measure a
    // configuration the application does not run in.
    platform::followSystemFonts(qApp);

    QList<int> sizes;
    for (int argument = 1; argument < argc; ++argument) {
        sizes.append(QString::fromLocal8Bit(argv[argument]).toInt());
    }
    if (sizes.isEmpty()) {
        sizes = {200, 2000, 20000};
    }

    for (const int size : std::as_const(sizes)) {
        measure(size);
    }

    return 0;
}
