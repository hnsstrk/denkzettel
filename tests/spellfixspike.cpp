/*
 * Prototyp zum Spike #62 — kein Produktivcode.
 *
 * Answers the four questions of issue #62 by measurement instead of reasoning:
 * whether spellfix1 registers at the handle the Qt SQLite driver hands out,
 * which of the two vocabulary sources costs what on the real schema, and
 * whether an editdist3 cost table finds the two customer cases (#51, #52).
 *
 * Built only with -DDENKZETTEL_SPIKE_SPELLFIX=ON; the report of the spike lives
 * in docs/scrum/reviews/spike-62-spellfix1/.
 */

#include "store/store.h"

#include <QElapsedTimer>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>
#include <QVariant>

#include <sqlite3.h>

#include <memory>

// spellfix.c is compiled into denkzettelstore (see src/CMakeLists.txt). Its
// entry point keeps C linkage; the api-routines argument stays a null pointer,
// which is what a statically linked extension passes.
struct sqlite3_api_routines;
extern "C" int sqlite3_spellfix_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);

namespace
{
/** Words the corpus is built from — German fragments, umlauts and ß included. */
const char *const seedWords[] = {
    "abend",    "ablage",     "anruf",     "antwort",   "arbeit",    "aufgabe",   "auswahl",
    "bahn",     "bericht",    "besprechung", "bestellung", "bild",    "buch",      "bücher",
    "büro",     "dienst",     "druck",     "einkauf",   "eintrag",   "entwurf",   "ergebnis",
    "erinnerung", "fahrt",    "fehler",    "feier",     "fenster",   "film",      "frage",
    "frist",    "führung",    "garten",    "gebäude",   "gedanke",   "gerät",     "geschäft",
    "gespräch", "größe",      "haus",      "heizung",   "hinweis",   "idee",      "kalender",
    "karte",    "kasse",      "konto",     "kosten",    "küche",     "kunde",     "kurs",
    "lager",    "leitung",    "liste",     "lösung",    "magazin",   "mappe",     "maschine",
    "meldung",  "messung",    "miete",     "mittag",    "monat",     "motor",     "muster",
    "nachricht", "notiz",     "ordner",    "paket",     "papier",    "plan",      "platz",
    "post",     "praxis",     "preis",     "projekt",   "prüfen",    "prüfstand", "prüfung",
    "quelle",   "raum",       "rechnung",  "regel",     "reise",     "rezept",    "richtung",
    "sache",    "schlüssel",  "schrank",   "schreiben", "schule",    "sitzung",   "software",
    "sommer",   "spiel",      "sprache",   "stadt",     "straße",    "strom",     "stunde",
    "suche",    "technik",    "termin",    "text",      "thema",     "tisch",     "treffen",
    "übung",    "uhr",        "umzug",     "unterlage", "urlaub",    "vertrag",   "video",
    "vorlage",  "vortrag",    "wagen",     "wand",      "wartung",   "wasser",    "werkstatt",
    "wetter",   "woche",      "wohnung",   "wunsch",    "zahlung",   "zeit",      "zettel",
    "ziel",     "zimmer",     "zug",       "zustand",
};

/** Notes in the corpus — the scale SPEC 6 states its index numbers for. */
constexpr int corpusNotes = 20000;
/** Words per note; times the note count this is the token total. */
constexpr int wordsPerNote = 25;

/** The sqlite3 handle behind a Qt connection, or nullptr if the driver is not QSQLITE. */
sqlite3 *sqliteHandle(const QSqlDatabase &db)
{
    const QVariant handle = db.driver()->handle();
    if (!handle.isValid() || qstrcmp(handle.typeName(), "sqlite3*") != 0) {
        return nullptr;
    }
    return *static_cast<sqlite3 *const *>(handle.constData());
}

QString megabytes(qint64 bytes)
{
    return QStringLiteral("%1 MiB").arg(double(bytes) / (1024.0 * 1024.0), 0, 'f', 2);
}
}

class SpellfixSpike : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    /** Prüffrage 1: registration at QSqlDriver::handle(). */
    void registersAtTheQtDriverHandle();

    /** Prüffrage 3: the two customer cases, each proven on its own. */
    void correctsTheCustomerCases_data();
    void correctsTheCustomerCases();

    /** Prüffrage 2: size of the two vocabulary sources on the real schema. */
    void measuresBothVocabularySources();

private:
    /** Runs one statement, failing the test with the SQLite message on error. */
    void run(const QString &statement);
    QVariant scalar(const QString &statement);
    /** Bytes the pages of every object matching the GLOB take up (dbstat). */
    qint64 objectBytes(const QString &nameGlob);
    /** Fills `notes` through the real schema, so the FTS triggers fire as usual. */
    void buildCorpus();
    /** Feeds `spellfix1` from a SELECT delivering one word column. */
    void fillSpellfixFrom(const QString &table, const QString &select);
    /** Best correction spellfix1 offers for `term`, with its distance. */
    QPair<QString, int> bestMatch(const QString &table, const QString &term);

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    QSqlDatabase m_db;
    QString m_connectionName;
    int m_seedCount = 0;
    int m_vocabularySize = 0;
};

void SpellfixSpike::initTestCase()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    // The real schema, created by the real migrations — measuring against a
    // hand-written copy of it would measure the copy.
    m_store = std::make_unique<Store>(m_dir->path() + QStringLiteral("/denkzettel.db"));
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));
    QCOMPARE(m_store->schemaVersion(), 2);

    // A second connection to the same file: Store keeps its own handle private,
    // and the spike must not change Store to get at it.
    m_connectionName = QStringLiteral("spellfixspike-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(m_dir->path() + QStringLiteral("/denkzettel.db"));
    QVERIFY2(m_db.open(), qPrintable(m_db.lastError().text()));

    // Every slot below needs the extension; that it *arrives* is the subject of
    // registersAtTheQtDriverHandle(), which proves it on a connection of its own.
    QCOMPARE(sqlite3_spellfix_init(sqliteHandle(m_db), nullptr, nullptr), SQLITE_OK);

    buildCorpus();
}

void SpellfixSpike::cleanupTestCase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
    m_store.reset();
    m_dir.reset();
}

void SpellfixSpike::run(const QString &statement)
{
    QSqlQuery query(m_db);
    if (!query.exec(statement)) {
        QFAIL(qPrintable(QStringLiteral("%1\n  bei: %2").arg(query.lastError().text(), statement.left(400))));
    }
}

QVariant SpellfixSpike::scalar(const QString &statement)
{
    QSqlQuery query(m_db);
    if (!query.exec(statement) || !query.next()) {
        return {};
    }
    return query.value(0);
}

qint64 SpellfixSpike::objectBytes(const QString &nameGlob)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT coalesce(sum(pgsize), 0) FROM dbstat WHERE name GLOB :glob"));
    query.bindValue(QStringLiteral(":glob"), nameGlob);
    if (!query.exec() || !query.next()) {
        return -1;
    }
    return query.value(0).toLongLong();
}

void SpellfixSpike::buildCorpus()
{
    QElapsedTimer timer;
    timer.start();

    run(QStringLiteral("CREATE TABLE spike_seed (id INTEGER PRIMARY KEY, w TEXT NOT NULL)"));
    QSqlQuery insertSeed(m_db);
    insertSeed.prepare(QStringLiteral("INSERT INTO spike_seed (w) VALUES (:w)"));
    QVERIFY(m_db.transaction());
    for (const char *const word : seedWords) {
        insertSeed.bindValue(QStringLiteral(":w"), QString::fromUtf8(word));
        QVERIFY2(insertSeed.exec(), qPrintable(insertSeed.lastError().text()));
    }
    QVERIFY(m_db.commit());
    m_seedCount = scalar(QStringLiteral("SELECT count(*) FROM spike_seed")).toInt();
    QVERIFY(m_seedCount > 0);

    // The vocabulary: every seed, plus every ordered pair of seeds glued
    // together. German notes are full of compounds, and a vocabulary of a few
    // hundred words would make both variants look cheaper than they are.
    // The pairing (a, b) → id is injective, so no word is generated twice.
    const int total = m_seedCount + m_seedCount * m_seedCount;
    run(QStringLiteral("CREATE TABLE spike_word (id INTEGER PRIMARY KEY, w TEXT NOT NULL)"));
    run(QStringLiteral("WITH RECURSIVE k(i) AS (VALUES(0) UNION ALL SELECT i + 1 FROM k WHERE i < %1)"
                       " INSERT INTO spike_word (id, w)"
                       " SELECT i + 1,"
                       "   CASE WHEN i < %2 THEN (SELECT w FROM spike_seed WHERE id = i + 1)"
                       "   ELSE (SELECT w FROM spike_seed WHERE id = ((i - %2) / %2) + 1)"
                       "     || (SELECT w FROM spike_seed WHERE id = ((i - %2) % %2) + 1)"
                       "   END"
                       " FROM k")
            .arg(total - 1)
            .arg(m_seedCount));
    m_vocabularySize = scalar(QStringLiteral("SELECT count(DISTINCT w) FROM spike_word")).toInt();
    QCOMPARE(m_vocabularySize, total);

    // The notes themselves, written through `notes` so the FTS5 triggers of
    // migration 2 fill the trigram index exactly as they do in the product.
    // The word index is squared before scaling, which makes short words far more
    // frequent than long ones — a flat draw would give every compound the same
    // weight as „zeit" and overstate the vocabulary a real corpus reaches.
    run(QStringLiteral(
            "WITH RECURSIVE"
            "  n(i) AS (VALUES(1) UNION ALL SELECT i + 1 FROM n WHERE i < %1),"
            "  p(j) AS (VALUES(1) UNION ALL SELECT j + 1 FROM p WHERE j < %2)"
            " INSERT INTO notes (created_at, type, content, state)"
            " SELECT strftime('%Y-%m-%dT%H:%M:%f', 1767225600 + i * 97, 'unixepoch'), 'text',"
            "   (SELECT group_concat(("
            "      SELECT w FROM spike_word WHERE id = 1 + cast("
            "        ((((i * 7919 + j * 104729) * 2654435761) % 65536) / 65536.0)"
            "        * ((((i * 7919 + j * 104729) * 2654435761) % 65536) / 65536.0) * %3 AS INTEGER)"
            "    ), ' ') FROM p),"
            "   'neu'"
            " FROM n")
            .arg(corpusNotes)
            .arg(wordsPerNote)
            .arg(m_vocabularySize));

    QCOMPARE(scalar(QStringLiteral("SELECT count(*) FROM notes")).toInt(), corpusNotes);
    qInfo().noquote() << QStringLiteral("Korpus: %1 Notizen, %2 Wörter je Notiz, %3 Wörter im Vokabular,"
                                        " %4 Zeichen Rohtext, gebaut in %5 s")
                             .arg(corpusNotes)
                             .arg(wordsPerNote)
                             .arg(m_vocabularySize)
                             .arg(scalar(QStringLiteral("SELECT sum(length(content)) FROM notes")).toLongLong())
                             .arg(double(timer.elapsed()) / 1000.0, 0, 'f', 1);
}

void SpellfixSpike::registersAtTheQtDriverHandle()
{
    // A connection of its own, so the state before the registration is really
    // the state before it — on m_db the extension is already in.
    const QString name = QStringLiteral("spellfixproof-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    {
        QSqlDatabase probe = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
        probe.setDatabaseName(m_dir->path() + QStringLiteral("/denkzettel.db"));
        QVERIFY2(probe.open(), qPrintable(probe.lastError().text()));

        sqlite3 *const handle = sqliteHandle(probe);
        QVERIFY2(handle != nullptr, "QSqlDriver::handle() liefert kein sqlite3*");

        // Without this half the proof below would also pass if SQLite had
        // brought editdist3 along by itself.
        QSqlQuery before(probe);
        QVERIFY(!before.exec(QStringLiteral("SELECT editdist3('prüfem', 'prüfen')")));
        qInfo().noquote() << QStringLiteral("vor der Registrierung: %1").arg(before.lastError().databaseText());

        QCOMPARE(sqlite3_spellfix_init(handle, nullptr, nullptr), SQLITE_OK);

        // The registration is read back at the service, not trusted: SQLITE_OK
        // says the call returned, the query says the function is there.
        QSqlQuery after(probe);
        QVERIFY2(after.exec(QStringLiteral("SELECT editdist3('prüfem', 'prüfen'), sqlite_version()")),
                 qPrintable(after.lastError().text()));
        QVERIFY(after.next());
        QVERIFY(after.value(0).canConvert<int>());
        qInfo().noquote() << QStringLiteral("nach der Registrierung: editdist3('prüfem', 'prüfen') = %1,"
                                            " SQLite im Prozess: %2")
                                 .arg(after.value(0).toInt())
                                 .arg(after.value(1).toString());

        // The virtual table is the part the correction actually needs; a
        // registration that only brings the functions would not carry the story.
        QSqlQuery module(probe);
        QVERIFY2(module.exec(QStringLiteral("CREATE VIRTUAL TABLE temp.probe_sfx USING spellfix1")),
                 qPrintable(module.lastError().text()));
        QVERIFY(module.exec(QStringLiteral("INSERT INTO temp.probe_sfx (word) VALUES ('prüfen')")));
        QVERIFY(module.exec(QStringLiteral("SELECT word FROM temp.probe_sfx WHERE word MATCH 'prufen'")));
        QVERIFY(module.next());
        QCOMPARE(module.value(0).toString(), QStringLiteral("prüfen"));

        probe.close();
    }
    QSqlDatabase::removeDatabase(name);
}

void SpellfixSpike::fillSpellfixFrom(const QString &table, const QString &select)
{
    run(QStringLiteral("CREATE VIRTUAL TABLE %1 USING spellfix1").arg(table));
    run(QStringLiteral("INSERT INTO %1 (word) %2").arg(table, select));
}

QPair<QString, int> SpellfixSpike::bestMatch(const QString &table, const QString &term)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT word, distance FROM %1 WHERE word MATCH :term AND top = 1").arg(table));
    query.bindValue(QStringLiteral(":term"), term);
    if (!query.exec() || !query.next()) {
        return {QString(), -1};
    }
    return {query.value(0).toString(), query.value(1).toInt()};
}

void SpellfixSpike::correctsTheCustomerCases_data()
{
    QTest::addColumn<QString>("typed");
    QTest::addColumn<QString>("expected");

    // The two cases are listed separately on purpose: spellfix1 transliterates
    // internally, and one of them could pass on the transliteration alone while
    // the other needs the cost table. Deriving one from the other would hide it.
    QTest::newRow("#51 Schreibvariante pruefen") << QStringLiteral("pruefen") << QStringLiteral("prüfen");
    QTest::newRow("#52 Tippfehler prüfem") << QStringLiteral("prüfem") << QStringLiteral("prüfen");
}

void SpellfixSpike::correctsTheCustomerCases()
{
    QFETCH(QString, typed);
    QFETCH(QString, expected);

    static bool prepared = false;
    if (!prepared) {
        prepared = true;
        run(QStringLiteral("CREATE TABLE editcost (iLang INT, cFrom TEXT, cTo TEXT, iCost INT)"));
        // Both directions: the rule turns the *typed* term into the stored word,
        // and a user types „prüfen" as readily as „pruefen".
        run(QStringLiteral("INSERT INTO editcost (iLang, cFrom, cTo, iCost) VALUES"
                           " (0, 'ue', 'ü', 1), (0, 'ü', 'ue', 1),"
                           " (0, 'ae', 'ä', 1), (0, 'ä', 'ae', 1),"
                           " (0, 'oe', 'ö', 1), (0, 'ö', 'oe', 1),"
                           " (0, 'ss', 'ß', 1), (0, 'ß', 'ss', 1)"));

        // Two vocabularies: a small one, where the case is plain to see, and the
        // corpus vocabulary, where the word has 17000 competitors. A proof on the
        // small list alone would not show whether the hit survives company.
        fillSpellfixFrom(QStringLiteral("klein_sfx"),
                         QStringLiteral("VALUES ('prüfen'), ('prüfung'), ('prüfstand'), ('drucken'),"
                                        " ('rufen'), ('greifen'), ('brüten'), ('prügeln')"));
        fillSpellfixFrom(QStringLiteral("gross_sfx"), QStringLiteral("SELECT w FROM spike_word"));
        for (const QString &table : {QStringLiteral("klein_sfx"), QStringLiteral("gross_sfx")}) {
            run(QStringLiteral("INSERT INTO %1 (command) VALUES ('edit_cost_table=editcost')").arg(table));
        }
    }

    for (const QString &table : {QStringLiteral("klein_sfx"), QStringLiteral("gross_sfx")}) {
        const QPair<QString, int> match = bestMatch(table, typed);
        qInfo().noquote() << QStringLiteral("%1: „%2\" → „%3\" (Distanz %4)")
                                 .arg(table, typed, match.first)
                                 .arg(match.second);
        QCOMPARE(match.first, expected);
    }
}

void SpellfixSpike::measuresBothVocabularySources()
{
    const qint64 notesBytes = objectBytes(QStringLiteral("notes"));
    const qint64 trigramBytes = objectBytes(QStringLiteral("notes_fts*"));
    QVERIFY(notesBytes > 0);
    QVERIFY(trigramBytes > 0);

    // Variant A — a second FTS5 index, unicode61, only as a word source.
    // remove_diacritics stays 0: with 1 the vocabulary would read „prufen", and
    // the correction would offer the user a word that is in no note.
    QElapsedTimer timer;
    timer.start();
    run(QStringLiteral("CREATE VIRTUAL TABLE notes_words USING fts5("
                       "  content, content='notes', content_rowid='id',"
                       "  tokenize='unicode61 remove_diacritics 0')"));
    run(QStringLiteral("CREATE TRIGGER notes_words_ai AFTER INSERT ON notes BEGIN"
                       "  INSERT INTO notes_words (rowid, content) VALUES (new.id, new.content);"
                       " END"));
    run(QStringLiteral("CREATE TRIGGER notes_words_ad AFTER DELETE ON notes BEGIN"
                       "  INSERT INTO notes_words (notes_words, rowid, content)"
                       "  VALUES ('delete', old.id, old.content);"
                       " END"));
    run(QStringLiteral("CREATE TRIGGER notes_words_au AFTER UPDATE ON notes BEGIN"
                       "  INSERT INTO notes_words (notes_words, rowid, content)"
                       "  VALUES ('delete', old.id, old.content);"
                       "  INSERT INTO notes_words (rowid, content) VALUES (new.id, new.content);"
                       " END"));
    run(QStringLiteral("INSERT INTO notes_words (notes_words) VALUES ('rebuild')"));
    run(QStringLiteral("CREATE VIRTUAL TABLE notes_words_vocab USING fts5vocab('notes_words', 'row')"));
    const qint64 variantAms = timer.elapsed();
    const qint64 variantABytes = objectBytes(QStringLiteral("notes_words*"));
    const int variantAWords = scalar(QStringLiteral("SELECT count(*) FROM notes_words_vocab")).toInt();

    // Variant B — a word table kept by triggers. The split is a recursive CTE on
    // spaces; a reference count is what makes deletion possible at all.
    timer.restart();
    run(QStringLiteral("CREATE TABLE spike_vocab (word TEXT PRIMARY KEY, n INTEGER NOT NULL) WITHOUT ROWID"));
    const QString split = QStringLiteral(
        "WITH RECURSIVE split(rest, word) AS ("
        "  VALUES(%1.content || ' ', '')"
        "  UNION ALL"
        "  SELECT substr(rest, instr(rest, ' ') + 1), substr(rest, 1, instr(rest, ' ') - 1)"
        "    FROM split WHERE rest <> ''"
        ")");
    run(QStringLiteral("CREATE TRIGGER spike_vocab_ai AFTER INSERT ON notes BEGIN"
                       "  INSERT INTO spike_vocab (word, n) %1"
                       "  SELECT lower(word), 1 FROM split WHERE word <> ''"
                       "  ON CONFLICT(word) DO UPDATE SET n = n + 1;"
                       " END")
            .arg(split.arg(QStringLiteral("new"))));
    run(QStringLiteral("CREATE TRIGGER spike_vocab_ad AFTER DELETE ON notes BEGIN"
                       "  UPDATE spike_vocab SET n = n - 1 WHERE word IN ("
                       "    %1 SELECT lower(word) FROM split WHERE word <> '');"
                       "  DELETE FROM spike_vocab WHERE n <= 0;"
                       " END")
            .arg(split.arg(QStringLiteral("old"))));
    // Backfill for the notes that predate the trigger — the same step migration 2
    // needs its 'rebuild' for, only here it has to be written by hand.
    run(QStringLiteral("INSERT INTO spike_vocab (word, n)"
                       " WITH RECURSIVE split(id, rest, word) AS ("
                       "   SELECT id, content || ' ', '' FROM notes"
                       "   UNION ALL"
                       "   SELECT id, substr(rest, instr(rest, ' ') + 1), substr(rest, 1, instr(rest, ' ') - 1)"
                       "     FROM split WHERE rest <> ''"
                       " )"
                       " SELECT lower(word), count(*) FROM split WHERE word <> '' GROUP BY lower(word)"));
    const qint64 variantBms = timer.elapsed();
    const qint64 variantBBytes = objectBytes(QStringLiteral("spike_vocab*"));
    const int variantBWords = scalar(QStringLiteral("SELECT count(*) FROM spike_vocab")).toInt();

    // What both variants feed: the spellfix1 table. Its size is the same for
    // either source and belongs into the bill.
    timer.restart();
    fillSpellfixFrom(QStringLiteral("mess_sfx"), QStringLiteral("SELECT term FROM notes_words_vocab"));
    const qint64 spellfixMs = timer.elapsed();
    const qint64 spellfixBytes = objectBytes(QStringLiteral("mess_sfx*"));

    timer.restart();
    const QPair<QString, int> lookup = bestMatch(QStringLiteral("mess_sfx"), QStringLiteral("pruefen"));
    const qint64 lookupMs = timer.elapsed();

    // The file size is not the yardstick here: the slots before this one leave
    // their own spellfix tables in the same file. Summed objects are.
    const qint64 stockBytes = notesBytes + trigramBytes;

    qInfo().noquote() << QStringLiteral("\n--- Größen am echten Schema (dbstat, %1 Notizen) ---").arg(corpusNotes);
    qInfo().noquote() << QStringLiteral("notes (Rohtext)                 %1").arg(megabytes(notesBytes));
    qInfo().noquote() << QStringLiteral("notes_fts (trigram, Bestand)    %1").arg(megabytes(trigramBytes));
    qInfo().noquote() << QStringLiteral("A: notes_words (unicode61)      %1  — %2 Wörter, %3 s Aufbau")
                             .arg(megabytes(variantABytes))
                             .arg(variantAWords)
                             .arg(double(variantAms) / 1000.0, 0, 'f', 1);
    qInfo().noquote() << QStringLiteral("B: spike_vocab (Trigger)        %1  — %2 Wörter, %3 s Aufbau")
                             .arg(megabytes(variantBBytes))
                             .arg(variantBWords)
                             .arg(double(variantBms) / 1000.0, 0, 'f', 1);
    qInfo().noquote() << QStringLiteral("spellfix1-Tabelle (beide Wege)  %1  — %2 s Aufbau")
                             .arg(megabytes(spellfixBytes))
                             .arg(double(spellfixMs) / 1000.0, 0, 'f', 1);
    qInfo().noquote() << QStringLiteral("Bestand heute (notes+notes_fts) %1").arg(megabytes(stockBytes));
    qInfo().noquote() << QStringLiteral("  Aufschlag Weg A + spellfix1   %1  (+%2 %)")
                             .arg(megabytes(variantABytes + spellfixBytes))
                             .arg(100.0 * double(variantABytes + spellfixBytes) / double(stockBytes), 0, 'f', 1);
    qInfo().noquote() << QStringLiteral("  Aufschlag Weg B + spellfix1   %1  (+%2 %)")
                             .arg(megabytes(variantBBytes + spellfixBytes))
                             .arg(100.0 * double(variantBBytes + spellfixBytes) / double(stockBytes), 0, 'f', 1);
    qInfo().noquote() << QStringLiteral("Korrektur „pruefen\" → „%1\" in %2 ms").arg(lookup.first).arg(lookupMs);

    QVERIFY(variantABytes > 0);
    QVERIFY(variantBBytes > 0);
    QVERIFY(spellfixBytes > 0);
    // Both sources have to deliver the same words, or the comparison is between
    // two different things.
    QCOMPARE(variantAWords, variantBWords);

    // The corpus above is words separated by single spaces — a shape in which
    // the hand-written splitter of variant B cannot fail. So the numbers alone
    // would be measured on a case that never occurs. One real note settles it.
    run(QStringLiteral("CREATE TEMP TABLE vorher_a AS SELECT term FROM notes_words_vocab"));
    run(QStringLiteral("CREATE TEMP TABLE vorher_b AS SELECT word FROM spike_vocab"));
    QSqlQuery messy(m_db);
    messy.prepare(QStringLiteral("INSERT INTO notes (created_at, type, content, state)"
                                 " VALUES ('2026-08-02T12:00:00.000', 'text', :content, 'neu')"));
    messy.bindValue(QStringLiteral(":content"),
                    QStringLiteral("Rechnung prüfen: Straße 7 — „dringend\"! (siehe E-Mail vom 3.8.)"));
    QVERIFY2(messy.exec(), qPrintable(messy.lastError().text()));

    qInfo().noquote() << QStringLiteral("\n--- Wörter aus einer Notiz mit Satzzeichen ---");
    qInfo().noquote() << QStringLiteral("A (unicode61): %1")
                             .arg(scalar(QStringLiteral("SELECT group_concat(term, ' | ') FROM"
                                                        " (SELECT term FROM notes_words_vocab"
                                                        "  EXCEPT SELECT term FROM vorher_a)"))
                                      .toString());
    qInfo().noquote() << QStringLiteral("B (Trigger):   %1")
                             .arg(scalar(QStringLiteral("SELECT group_concat(word, ' | ') FROM"
                                                        " (SELECT word FROM spike_vocab"
                                                        "  EXCEPT SELECT word FROM vorher_b)"))
                                      .toString());
}

QTEST_GUILESS_MAIN(SpellfixSpike)

#include "spellfixspike.moc"
