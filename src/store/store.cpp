#include "store/store.h"

#include "store/searchquery.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QVariant>

#include <sqlite3.h>

#include <cstring>

// third_party/spellfix/spellfix.c, compiled into this library with
// SQLITE_CORE=1 (src/CMakeLists.txt). The entry point keeps C linkage; the
// api-routines argument stays a null pointer, which is what a statically
// linked extension passes.
struct sqlite3_api_routines;
extern "C" int sqlite3_spellfix_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);

namespace
{
/**
 * Ordered schema migrations; entry i upgrades the database to version i + 1.
 *
 * The statements carry no IF NOT EXISTS on purpose: a migration that runs
 * twice is a bug in the version bookkeeping and should fail loudly.
 */
const QList<QStringList> &migrations()
{
    static const QList<QStringList> steps = {
        // Version 1 — M1 schema (SPEC 5.1). notes_fts, embeddings, proposals
        // and proposal_notes arrive with their milestones.
        {
            QStringLiteral("CREATE TABLE meta ("
                           "  key TEXT PRIMARY KEY,"
                           "  value TEXT NOT NULL)"),
            QStringLiteral("CREATE TABLE notes ("
                           "  id INTEGER PRIMARY KEY,"
                           "  created_at TEXT NOT NULL,"
                           "  type TEXT NOT NULL CHECK (type IN ('text', 'audio')),"
                           "  content TEXT NOT NULL,"
                           "  audio_path TEXT,"
                           "  audio_duration_s INTEGER,"
                           "  category TEXT,"
                           "  state TEXT NOT NULL CHECK (state IN ('neu', 'transkribiert', 'analysiert')),"
                           "  needs_reembed INTEGER NOT NULL DEFAULT 0,"
                           "  analysis_attempts INTEGER NOT NULL DEFAULT 0,"
                           "  analysis_last_error TEXT)"),
            QStringLiteral("CREATE TABLE tags ("
                           "  note_id INTEGER NOT NULL REFERENCES notes(id),"
                           "  tag TEXT NOT NULL,"
                           "  PRIMARY KEY (note_id, tag))"),
        },
        // Version 2 — full-text index (SPEC 5.1, 6). The index keeps no text of
        // its own: `content='notes'` points it at the notes table, so the note
        // text exists once and cannot drift between two copies.
        {
            // `trigram` indexes every three-character sequence, which is what
            // makes „grafieren" find „fotografieren" (user decision
            // 01.08.2026). It costs index size — measured at roughly six times
            // a unicode61 index — and it cannot represent anything shorter
            // than three characters; Store::search() covers that gap.
            QStringLiteral("CREATE VIRTUAL TABLE notes_fts USING fts5("
                           "  content,"
                           "  content='notes',"
                           "  content_rowid='id',"
                           "  tokenize='trigram remove_diacritics 1')"),
            // The three triggers of the external-content pattern. A deletion is
            // written as a 'delete' command carrying the *old* text: FTS5 needs
            // it to find the index entries it has to take out. Passing the new
            // text instead leaves the old words findable — which is what
            // StoreTest::keepsSearchIndexInSync() watches for.
            QStringLiteral("CREATE TRIGGER notes_fts_after_insert AFTER INSERT ON notes BEGIN"
                           "  INSERT INTO notes_fts (rowid, content) VALUES (new.id, new.content);"
                           " END"),
            QStringLiteral("CREATE TRIGGER notes_fts_after_delete AFTER DELETE ON notes BEGIN"
                           "  INSERT INTO notes_fts (notes_fts, rowid, content)"
                           "  VALUES ('delete', old.id, old.content);"
                           " END"),
            QStringLiteral("CREATE TRIGGER notes_fts_after_update AFTER UPDATE ON notes BEGIN"
                           "  INSERT INTO notes_fts (notes_fts, rowid, content)"
                           "  VALUES ('delete', old.id, old.content);"
                           "  INSERT INTO notes_fts (rowid, content) VALUES (new.id, new.content);"
                           " END"),
            // Notes written before this migration predate the triggers; without
            // the rebuild they would stay invisible to the search for good.
            QStringLiteral("INSERT INTO notes_fts (notes_fts) VALUES ('rebuild')"),
        },
        // Version 3 — the transcription queue (SPEC 5.1, 12). The columns are
        // the ones SPEC 5.1 names and no others; what a reader wants to know
        // is answered by them together:
        //   no row                         — done, the transcript is the note
        //                                    (state 'transkribiert')
        //   row, attempts below the limit  — outstanding: waiting or running
        //   row, attempts at the limit     — given up on; the note keeps its
        //                                    audio without a transcript, and
        //                                    last_error says why
        // A column for "running" would be a fourth state nobody can write
        // truthfully: a daemon killed mid-run cannot clear it, and the row
        // would say "running" for ever (SPEC 12: the queue survives restarts).
        // For the same reason the **count** and not the reason separates the
        // last two rows above: the count is written before the run and the
        // reason after it, so a killed daemon leaves an attempt counted with
        // nothing said, and read by the reason that row would pass for one
        // still waiting. Transcriber::start() fills the reason in through
        // noteInterruptedTranscribeJobs().
        {
            // ON DELETE CASCADE, and not for tidiness: with `PRAGMA
            // foreign_keys = ON` a job row still pointing at the note would
            // make Store::removeNote() fail outright — deleting an audio note
            // whose transcription is queued is the ordinary case, not an edge.
            QStringLiteral("CREATE TABLE transcribe_jobs ("
                           "  note_id INTEGER PRIMARY KEY REFERENCES notes(id) ON DELETE CASCADE,"
                           "  enqueued_at TEXT NOT NULL,"
                           "  attempts INTEGER NOT NULL DEFAULT 0,"
                           "  last_error TEXT)"),
        },
        // Version 4 — what the classification of SPEC 7.2 writes beyond the
        // columns version 1 already brought (issue #14). Category, tags, state
        // and the two error columns stood there from the start; the task fields
        // did not, and SPEC 5.1 names no place for them.
        //
        // **`is_todo` has no column of its own**, although the JSON schema of
        // SPEC 7.2 carries the flag: it is exactly `task IS NOT NULL`. SPEC 7.4
        // makes a suggestion out of the extracted fields, so what says a note is
        // a task is a description to make one from — a model that sets the flag
        // and hands over nothing has named no task, and readClassification()
        // writes none while the category and the tags stand as they came. Two
        // columns whose truth has to agree would be a second place for the same
        // fact and the first thing to drift.
        //
        // One JSON text and not five columns, because that is what the fields
        // are on the way out as well: SPEC 5.1 gives `proposals.payload` the
        // same shape, and M5 hands this text on rather than assembling it
        // again. What may stand in it is decided in readClassification(), not
        // by the model.
        {
            QStringLiteral("ALTER TABLE notes ADD COLUMN task TEXT"),
        },
        // Version 5 — the embeddings of SPEC 5.1, which step 2 of the analysis
        // run writes and the topic clustering of SPEC 7.3 reads (issue #28).
        //
        // One row per note and no history: what is asked for is the vector of
        // the text as it stands now, and an edited note replaces its own
        // (SPEC 9 sets `needs_reembed`, setEmbedding() clears it again).
        //
        // `model` beside the vector, because a vector only means anything
        // against vectors of the **same** model: SPEC 7.1 makes the embedding
        // model a setting, and the 0.60 of SPEC 7.3 is calibrated for one of
        // them. Whoever changes the model gets a corpus that is embedded
        // again, note by note — see notesToEmbed(), which is what compares the
        // name. Mixed silently, the clustering would put notes together for no
        // reason anybody could look up.
        //
        // ON DELETE CASCADE for the reason the transcription queue has it: with
        // `PRAGMA foreign_keys = ON` a row still pointing at the note would make
        // Store::removeNote() fail outright, and SPEC 5.1 asks for the
        // embedding to go with the note in the same transaction. Left behind,
        // it would also be clustered — a bundle carrying a note that is not
        // there any more.
        {
            QStringLiteral("CREATE TABLE embeddings ("
                           "  note_id INTEGER PRIMARY KEY REFERENCES notes(id) ON DELETE CASCADE,"
                           "  model TEXT NOT NULL,"
                           "  vector BLOB NOT NULL)"),
        },
        // Version 6 — the suggestions of SPEC 5.1, which step 3 of the analysis
        // run writes and the review of SPEC 9 reads (issue #29). The columns are
        // the ones SPEC 5.1 names and no others.
        //
        // **One payload column and not a set of columns per kind**: the two
        // kinds carry different things — a bundle a title and the Markdown of
        // the collective note (SPEC 8.1), a task the five fields of SPEC 7.2 —
        // and the task text is handed on from `notes.task`, which migration 4
        // writes as one JSON object for exactly this. Columns for both would
        // leave half of them NULL in every row and would need a second column
        // saying which half means anything.
        //
        // **There is no 'accepted' status**, and SPEC 8.1 says why: accepting
        // and discarding both end with the suggestion deleted, the difference
        // being that accepting exports first. A suggestion is a pass-through,
        // not a history — so the two values SPEC 5.1 names are the two there
        // are.
        //
        // ON DELETE CASCADE on **both** sides of proposal_notes. Towards the
        // note for the reason the transcription queue and the embeddings carry
        // it: with `PRAGMA foreign_keys = ON` a link row still pointing at the
        // note would make Store::removeNote() fail outright, and SPEC 5.1 asks
        // for the references to go with the note in the same transaction.
        // Towards the proposal because SPEC 8.1 removes a suggestion together
        // with its references, and a link left behind would hold a note in a
        // bundle nobody can open any more.
        {
            QStringLiteral("CREATE TABLE proposals ("
                           "  id INTEGER PRIMARY KEY,"
                           "  kind TEXT NOT NULL CHECK (kind IN ('bundle', 'task')),"
                           "  created_at TEXT NOT NULL,"
                           "  status TEXT NOT NULL CHECK (status IN ('offen', 'zurueckgestellt')),"
                           "  payload TEXT NOT NULL)"),
            // The pair is the primary key, as it is in `tags`: a note stands in
            // one suggestion once, and a run that linked it twice would put the
            // same note twice into the same collective note.
            QStringLiteral("CREATE TABLE proposal_notes ("
                           "  proposal_id INTEGER NOT NULL REFERENCES proposals(id) ON DELETE CASCADE,"
                           "  note_id INTEGER NOT NULL REFERENCES notes(id) ON DELETE CASCADE,"
                           "  PRIMARY KEY (proposal_id, note_id))"),
        },
        // Version 7 — the context stamp of SPEC 5.1 and 13 (issue #47): the
        // title of the window that was active before the capture window took
        // the focus, and the application id beside it.
        //
        // **Two columns and not one**, and SPEC 5.1 carries the reasoning: the
        // title is what the user reads, the application id is what the
        // classification of SPEC 7 keys on — a note from a terminal is
        // probably a command-line note whatever its title says. Two facts
        // about one moment, so the argument against `is_todo` above does not
        // apply here.
        //
        // Both nullable and both NULL for every note that already stands in
        // the database: ALTER TABLE ADD COLUMN fills the existing rows with
        // NULL, which is exactly the state „no origin" — nothing to migrate,
        // nothing to lose. The switch of SPEC 13 is off by default, so a
        // database whose owner never turns it on keeps two columns of NULL.
        {
            QStringLiteral("ALTER TABLE notes ADD COLUMN origin TEXT"),
            QStringLiteral("ALTER TABLE notes ADD COLUMN origin_app TEXT"),
        },
        // Version 8 — what the tolerant search of SPEC 6 needs beside the
        // trigram index (issue #69): a list of the words the corpus holds, and
        // a table of German edit costs. The spellfix1 table itself is **not**
        // here — it is built in `temp`, see Store::prepareCorrections().
        //
        // **A second index and not the existing one**, because the trigram
        // index has no words in it: its vocabulary is three-character
        // fragments, and a correction offered out of it would be a fragment.
        // `unicode61` is the tokenizer that cuts at word boundaries, and it is
        // read for nothing else — no query touches `notes_words`, only the
        // `fts5vocab` view of it.
        //
        // **`remove_diacritics 0`, and that is a condition rather than a
        // preference** (spike #62): with 1 the word list would read „prufen",
        // the correction would hand the user a word that stands in no note,
        // and the second pass would search for it and find nothing. The word
        // list has to be spelled the way the notes are.
        {
            QStringLiteral("CREATE VIRTUAL TABLE notes_words USING fts5("
                           "  content,"
                           "  content='notes',"
                           "  content_rowid='id',"
                           "  tokenize='unicode61 remove_diacritics 0')"),
            // The three triggers of migration 2, for the same reason: a
            // deletion is written as a 'delete' command carrying the *old*
            // text. With the new text the old words stay in the list, and the
            // correction offers a word nobody wrote any more.
            QStringLiteral("CREATE TRIGGER notes_words_after_insert AFTER INSERT ON notes BEGIN"
                           "  INSERT INTO notes_words (rowid, content) VALUES (new.id, new.content);"
                           " END"),
            QStringLiteral("CREATE TRIGGER notes_words_after_delete AFTER DELETE ON notes BEGIN"
                           "  INSERT INTO notes_words (notes_words, rowid, content)"
                           "  VALUES ('delete', old.id, old.content);"
                           " END"),
            QStringLiteral("CREATE TRIGGER notes_words_after_update AFTER UPDATE ON notes BEGIN"
                           "  INSERT INTO notes_words (notes_words, rowid, content)"
                           "  VALUES ('delete', old.id, old.content);"
                           "  INSERT INTO notes_words (rowid, content) VALUES (new.id, new.content);"
                           " END"),
            // Every note written before this migration, as migration 2 does it.
            QStringLiteral("INSERT INTO notes_words (notes_words) VALUES ('rebuild')"),
            // One row per distinct word — the vocabulary the correction picks
            // from.
            QStringLiteral("CREATE VIRTUAL TABLE notes_words_vocab USING fts5vocab('notes_words', 'row')"),
            // The German rules of SPEC 6. The four column names are the ones
            // spellfix1 reads; the table name is what `edit_cost_table=` is
            // pointed at.
            //
            // **Both directions of every rule**, because the cost is asked of
            // the way from the typed word to the stored one, and the user
            // types „prüfen" as readily as „pruefen".
            //
            // **Cost 1 and not 0**: at 0 „pruefen" and „prüfen" would be the
            // same word to editdist3, and the ranking could not tell the
            // spelling variant from a second word standing at the same
            // distance. 1 is as close to free as this table goes while staying
            // a difference.
            QStringLiteral("CREATE TABLE editcost ("
                           "  iLang INTEGER NOT NULL,"
                           "  cFrom TEXT NOT NULL,"
                           "  cTo TEXT NOT NULL,"
                           "  iCost INTEGER NOT NULL)"),
            QStringLiteral("INSERT INTO editcost (iLang, cFrom, cTo, iCost) VALUES"
                           " (0, 'ue', 'ü', 1), (0, 'ü', 'ue', 1),"
                           " (0, 'ae', 'ä', 1), (0, 'ä', 'ae', 1),"
                           " (0, 'oe', 'ö', 1), (0, 'ö', 'oe', 1),"
                           " (0, 'ss', 'ß', 1), (0, 'ß', 'ss', 1)"),
        },
    };
    return steps;
}

/**
 * The vector as SPEC 5.1 stores it: a float32 array, in this machine's byte
 * order.
 *
 * The database lies under the user's home directory and is read by the process
 * that wrote it (SPEC 5.1) — a copy onto a machine of the other byte order
 * would have to bring its own converter, and every other column would need one
 * too.
 */
QByteArray vectorToBlob(const QList<float> &vector)
{
    return {reinterpret_cast<const char *>(vector.constData()), vector.size() * qsizetype(sizeof(float))};
}

/**
 * The other direction, and **the size is counted in elements** — the one place
 * bytes and dimensions get confused without a word.
 *
 * A remainder means the BLOB was not written by the function above; it is cut
 * off rather than read past the end.
 */
QList<float> vectorFromBlob(const QByteArray &blob)
{
    const qsizetype count = blob.size() / qsizetype(sizeof(float));
    QList<float> vector(count);
    std::memcpy(vector.data(), blob.constData(), size_t(count) * sizeof(float));
    return vector;
}

QString typeToText(Note::Type type)
{
    return type == Note::Type::Audio ? QStringLiteral("audio") : QStringLiteral("text");
}

Note::Type typeFromText(const QString &text)
{
    return text == QLatin1String("audio") ? Note::Type::Audio : Note::Type::Text;
}

QString stateToText(Note::State state)
{
    switch (state) {
    case Note::State::Transcribed:
        return QStringLiteral("transkribiert");
    case Note::State::Analysed:
        return QStringLiteral("analysiert");
    case Note::State::New:
        break;
    }
    return QStringLiteral("neu");
}

Note::State stateFromText(const QString &text)
{
    if (text == QLatin1String("transkribiert")) {
        return Note::State::Transcribed;
    }
    if (text == QLatin1String("analysiert")) {
        return Note::State::Analysed;
    }
    return Note::State::New;
}

QString kindToText(Proposal::Kind kind)
{
    return kind == Proposal::Kind::Task ? QStringLiteral("task") : QStringLiteral("bundle");
}

Proposal::Kind kindFromText(const QString &text)
{
    return text == QLatin1String("task") ? Proposal::Kind::Task : Proposal::Kind::Bundle;
}

/** The two German values SPEC 5.1 writes, as `notes.state` writes its own. */
QString statusToText(Proposal::Status status)
{
    return status == Proposal::Status::Deferred ? QStringLiteral("zurueckgestellt") : QStringLiteral("offen");
}

Proposal::Status statusFromText(const QString &text)
{
    return text == QLatin1String("zurueckgestellt") ? Proposal::Status::Deferred : Proposal::Status::Open;
}

/**
 * Binds a text that must not be NULL, empty or not.
 *
 * Qt's SQLite driver binds a **null** QString as NULL, and a default
 * constructed QString is null — so a voice note, which carries no text until
 * its transcript arrives, would run into the NOT NULL constraint of `content`
 * and never be stored at all (measured 2026-08-28 on issue #22). An empty text
 * is a text of length nought here, not an absent one.
 */
QVariant plainText(const QString &value)
{
    return value.isNull() ? QVariant(QString(QLatin1String(""))) : QVariant(value);
}

/** Binds an empty string as SQL NULL. */
QVariant nullableText(const QString &value)
{
    return value.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : QVariant(value);
}

QVariant nullableInt(const std::optional<int> &value)
{
    return value.has_value() ? QVariant(*value) : QVariant(QMetaType(QMetaType::Int));
}

/** Timestamps keep the local offset: date filters (SPEC 6) mean local days. */
QString timestampToText(const QDateTime &timestamp)
{
    return timestamp.toString(Qt::ISODateWithMs);
}

QDateTime timestampFromText(const QString &text)
{
    return QDateTime::fromString(text, Qt::ISODateWithMs);
}

/** Column list shared by the single-note and the list query. */
QString noteColumns()
{
    return QStringLiteral("id, created_at, type, content, audio_path, audio_duration_s,"
                          " category, state, needs_reembed, analysis_attempts, analysis_last_error, task,"
                          " origin, origin_app");
}

/** Shortest term the trigram index can represent (SPEC 6). */
constexpr qsizetype trigramLength = 3;

/**
 * The `sqlite3*` behind a Qt connection, or nullptr if the driver is not
 * QSQLITE.
 *
 * The one instance of SQLite in the process: the Qt driver links the shared
 * `libsqlite3` and spellfix.c is compiled against the same one, which is the
 * first of the five conditions of spike #62.
 */
sqlite3 *sqliteHandle(const QSqlDatabase &db)
{
    const QVariant handle = db.driver()->handle();
    if (!handle.isValid() || qstrcmp(handle.typeName(), "sqlite3*") != 0) {
        return nullptr;
    }
    return *static_cast<sqlite3 *const *>(handle.constData());
}

/**
 * Most a correction of SPEC 6 may cost, in the units of `editdist3`.
 *
 * spellfix1 counts **costs**, not edits: an ordinary substitution is 150 and
 * an insertion or a deletion 100, while the `editcost` table of migration 8
 * puts the German umlaut rules at 1 apiece. So 150 means „any number of
 * umlaut spellings, and at most one ordinary typo" — which is exactly the two
 * cases the story is for, „pruefen" (cost 1) and „prüfem" (cost 150).
 *
 * A ceiling read as a number of edits does not work here and that was
 * measured in spike #62: a threshold of 2 or less admits the spelling
 * variants and locks every typo out.
 */
constexpr int correctionCostCeiling = 150;

/**
 * The German folding of SPEC 6: „ü" and „ue" become the same text.
 *
 * It decides whether a correction is one the user has to be told about. Whoever
 * types „pruefen" and gets „prüfen" has not mistyped anything — they wrote on a
 * keyboard without umlauts, which is what SPEC 6 already answers silently for
 * „bucher" → „Bücher". Whoever types „prüfem" has, and the library says so.
 *
 * A comparison and not a cost, on the UX decision of 30.08.2026: the boundary
 * between the two cases is a spelling rule with four entries, and a number
 * beside it would have to be calibrated and maintained.
 */
QString germanFold(const QString &word)
{
    QString folded = word.toCaseFolded();
    folded.replace(QStringLiteral("ü"), QStringLiteral("ue"));
    folded.replace(QStringLiteral("ö"), QStringLiteral("oe"));
    folded.replace(QStringLiteral("ä"), QStringLiteral("ae"));
    folded.replace(QStringLiteral("ß"), QStringLiteral("ss"));
    return folded;
}

/**
 * Wraps a search term as a LIKE pattern that matches it anywhere.
 *
 * A term out of the parser is a word of letters and digits **or** a phrase the
 * user put in quotation marks, and a phrase carries whatever was typed. So the
 * two wildcards of LIKE have to be taken out of the term — without that,
 * searching for the phrase „100%" would find every note (SPEC 6). What cannot
 * occur in either kind is a quotation mark: the parser eats those as
 * delimiters, which is why the FTS5 phrase further down needs no escaping.
 */
QString likePattern(const QString &term)
{
    QString escaped = term;
    escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    escaped.replace(QLatin1Char('%'), QLatin1String("\\%"));
    escaped.replace(QLatin1Char('_'), QLatin1String("\\_"));
    return QStringLiteral("%%%1%%").arg(escaped);
}

/** Reads the current row of a query built from noteColumns(). */
Note noteFromQuery(const QSqlQuery &query)
{
    Note note;
    note.id = query.value(QStringLiteral("id")).toLongLong();
    note.createdAt = timestampFromText(query.value(QStringLiteral("created_at")).toString());
    note.type = typeFromText(query.value(QStringLiteral("type")).toString());
    note.content = query.value(QStringLiteral("content")).toString();
    note.audioPath = query.value(QStringLiteral("audio_path")).toString();
    const QVariant duration = query.value(QStringLiteral("audio_duration_s"));
    if (!duration.isNull()) {
        note.audioDurationS = duration.toInt();
    }
    note.category = query.value(QStringLiteral("category")).toString();
    note.state = stateFromText(query.value(QStringLiteral("state")).toString());
    note.needsReembed = query.value(QStringLiteral("needs_reembed")).toInt() != 0;
    note.analysisAttempts = query.value(QStringLiteral("analysis_attempts")).toInt();
    note.analysisLastError = query.value(QStringLiteral("analysis_last_error")).toString();
    note.task = query.value(QStringLiteral("task")).toString();
    note.origin = query.value(QStringLiteral("origin")).toString();
    note.originApp = query.value(QStringLiteral("origin_app")).toString();
    return note;
}
}

Store::Store(const QString &databasePath)
    : m_databasePath(databasePath)
    , m_connectionName(QStringLiteral("denkzettel-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
}

Store::~Store()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    // The handle has to go out of scope before the connection can be removed.
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

QString Store::defaultDatabasePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/denkzettel.db");
}

bool Store::open()
{
    m_lastError.clear();
    const QString directory = QFileInfo(m_databasePath).absolutePath();
    if (!QDir().mkpath(directory)) {
        m_lastError = QStringLiteral("Creating directory %1 failed").arg(directory);
        return false;
    }

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(m_databasePath);
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery pragma(m_db);
    if (!pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"))) {
        m_lastError = pragma.lastError().text();
        return false;
    }

    m_spellfixReady = registerSpellfix();

    return migrate();
}

bool Store::registerSpellfix()
{
    // The registration applies **per connection** and not per database file
    // (spike #62), which is why it stands here and not in a migration: every
    // Store that opens this file registers the extension again on its own
    // handle.
    sqlite3 *const db = sqliteHandle(m_db);
    if (db == nullptr || sqlite3_spellfix_init(db, nullptr, nullptr) != SQLITE_OK) {
        return false;
    }

    // Read back at the service and not trusted: `SQLITE_OK` says the call
    // returned, this query says the function is there. Without the extension
    // it fails with „no such function".
    QSqlQuery probe(m_db);
    return probe.exec(QStringLiteral("SELECT editdist3('prüfem', 'prüfen')")) && probe.next();
}

bool Store::correctionsReady() const
{
    return m_spellfixReady;
}

bool Store::prepareCorrections() const
{
    if (!m_spellfixReady) {
        return false;
    }

    // The spellfix1 table lives in `temp` and is built when it is first
    // needed. It is derived data with no way of keeping itself current —
    // spellfix1 offers no hook a trigger could hang on — so a copy in the
    // database file would drift away from the notes: offering words nobody
    // wrote any more, missing the ones just written, and saying so nowhere.
    //
    // What says it is out of date is SQLite's own row counter for this
    // connection. It counts every INSERT, UPDATE and DELETE, the ones the FTS
    // triggers make included, so no write to a note can slip past it — which
    // a list of call sites in this file could. It counts a little too much
    // (a tag, a job row and a schema version move it as well) and that costs
    // one rebuild that would not have been needed. It does not see a **second
    // process** writing the same file; the daemon is single-instance
    // (SPEC 2.3) and there is no second writer.
    sqlite3 *const db = sqliteHandle(m_db);
    if (m_correctionsReady && sqlite3_total_changes(db) == m_correctionsBuiltAt) {
        return true;
    }

    // ponytail: the build reads the whole word list. Ceiling: **62 ms** at
    // 20,000 notes and a list of 17,030 words (measured 30.08.2026, machine at
    // load 0.3 — 114 ms for the first search that finds nothing against 52 ms
    // for every one after it). It is paid on that first search and again on
    // the first such search after a write; while a search has hits the second
    // pass of SPEC 6 does not run at all. The way up is an incremental update,
    // which needs a word list of our own instead of fts5vocab: spellfix1 takes
    // INSERT and DELETE, but nothing tells us which words a changed note added
    // and which it took away.
    static const QStringList steps = {
        QStringLiteral("DROP TABLE IF EXISTS temp.notes_spellfix"),
        QStringLiteral("CREATE VIRTUAL TABLE temp.notes_spellfix USING spellfix1"),
        QStringLiteral("INSERT INTO temp.notes_spellfix (word) SELECT term FROM notes_words_vocab"),
        // The German rules of migration 8. The name is unqualified because
        // spellfix1 builds the query itself; SQLite resolves it against
        // `temp` first and then against `main`, where the table stands.
        QStringLiteral("INSERT INTO temp.notes_spellfix (command) VALUES ('edit_cost_table=editcost')"),
    };
    for (const QString &statement : steps) {
        QSqlQuery query(m_db);
        if (!query.exec(statement)) {
            // Not into `m_lastError`: the correction is a widening of the
            // search, and its failure is not the search's. Written there it
            // would leave a **successful** search reporting an error, because
            // search() clears the field on the way in and nothing clears it
            // again afterwards. The journal is where a fault of this kind
            // belongs (CLAUDE.md, "Before handover").
            qWarning("the correction of SPEC 6 is unavailable: %s", qUtf8Printable(query.lastError().text()));
            return false;
        }
    }

    m_correctionsReady = true;
    // Read **after** the build: filling the table is itself a few thousand
    // row changes, and taken beforehand the mark would be stale the moment it
    // is written — every following search would rebuild.
    m_correctionsBuiltAt = sqlite3_total_changes(db);
    return true;
}

QString Store::correctionFor(const QString &term) const
{
    QSqlQuery query(m_db);
    // `top = 1` asks for the single best candidate; without it spellfix1
    // answers with twenty. The ceiling is a constraint of the query rather
    // than a comparison afterwards, so spellfix1 stops looking once it is
    // exceeded.
    query.prepare(QStringLiteral("SELECT word FROM temp.notes_spellfix"
                                 " WHERE word MATCH :term AND top = 1 AND distance <= :ceiling"));
    // Condition 5 of spike #62: the word list is what the `unicode61`
    // tokenizer wrote, and that is lower case. A term in any other spelling
    // would be measured against a vocabulary spelled differently.
    query.bindValue(QStringLiteral(":term"), term.toCaseFolded());
    query.bindValue(QStringLiteral(":ceiling"), correctionCostCeiling);
    if (!query.exec() || !query.next()) {
        return {};
    }
    return query.value(0).toString();
}

bool Store::migrate()
{
    if (m_db.tables().contains(QStringLiteral("meta"))) {
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral("SELECT value FROM meta WHERE key = 'schema_version'"));
        if (!query.exec()) {
            m_lastError = query.lastError().text();
            return false;
        }
        if (query.next()) {
            m_schemaVersion = query.value(0).toInt();
        }
    }

    const QList<QStringList> &steps = migrations();
    for (int index = m_schemaVersion; index < steps.size(); ++index) {
        if (!m_db.transaction()) {
            m_lastError = m_db.lastError().text();
            return false;
        }

        for (const QString &statement : steps.at(index)) {
            QSqlQuery query(m_db);
            if (!query.exec(statement)) {
                m_lastError = query.lastError().text();
                m_db.rollback();
                return false;
            }
        }

        QSqlQuery version(m_db);
        version.prepare(
            QStringLiteral("INSERT INTO meta (key, value) VALUES ('schema_version', :version)"
                           " ON CONFLICT(key) DO UPDATE SET value = excluded.value"));
        version.bindValue(QStringLiteral(":version"), QString::number(index + 1));
        if (!version.exec()) {
            m_lastError = version.lastError().text();
            m_db.rollback();
            return false;
        }

        if (!m_db.commit()) {
            m_lastError = m_db.lastError().text();
            m_db.rollback();
            return false;
        }

        m_schemaVersion = index + 1;
    }

    return true;
}

QString Store::lastError() const
{
    return m_lastError;
}

int Store::schemaVersion() const
{
    return m_schemaVersion;
}

QString Store::audioDirectory() const
{
    return QFileInfo(m_databasePath).absolutePath() + QStringLiteral("/audio");
}

QString Store::rescuedDirectory() const
{
    return QFileInfo(m_databasePath).absolutePath() + QStringLiteral("/rescued");
}

std::optional<qint64> Store::addNote(const Note &note)
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("INSERT INTO notes (created_at, type, content, audio_path, audio_duration_s,"
                       " category, state, needs_reembed, analysis_attempts, analysis_last_error, task,"
                       " origin, origin_app)"
                       " VALUES (:created_at, :type, :content, :audio_path, :audio_duration_s,"
                       " :category, :state, :needs_reembed, :analysis_attempts, :analysis_last_error, :task,"
                       " :origin, :origin_app)"));
    query.bindValue(QStringLiteral(":created_at"), timestampToText(note.createdAt));
    query.bindValue(QStringLiteral(":type"), typeToText(note.type));
    query.bindValue(QStringLiteral(":content"), plainText(note.content));
    query.bindValue(QStringLiteral(":audio_path"), nullableText(note.audioPath));
    query.bindValue(QStringLiteral(":audio_duration_s"), nullableInt(note.audioDurationS));
    query.bindValue(QStringLiteral(":category"), nullableText(note.category));
    query.bindValue(QStringLiteral(":state"), stateToText(note.state));
    query.bindValue(QStringLiteral(":needs_reembed"), note.needsReembed ? 1 : 0);
    query.bindValue(QStringLiteral(":analysis_attempts"), note.analysisAttempts);
    query.bindValue(QStringLiteral(":analysis_last_error"), nullableText(note.analysisLastError));
    query.bindValue(QStringLiteral(":task"), nullableText(note.task));
    query.bindValue(QStringLiteral(":origin"), nullableText(note.origin));
    query.bindValue(QStringLiteral(":origin_app"), nullableText(note.originApp));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }

    const qint64 id = query.lastInsertId().toLongLong();
    Q_EMIT noteAdded(id);

    return id;
}

bool Store::updateNote(const Note &note)
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("UPDATE notes SET created_at = :created_at, type = :type, content = :content,"
                       " audio_path = :audio_path, audio_duration_s = :audio_duration_s,"
                       " category = :category, state = :state, needs_reembed = :needs_reembed,"
                       " analysis_attempts = :analysis_attempts, analysis_last_error = :analysis_last_error,"
                       " task = :task, origin = :origin, origin_app = :origin_app"
                       " WHERE id = :id"));
    query.bindValue(QStringLiteral(":created_at"), timestampToText(note.createdAt));
    query.bindValue(QStringLiteral(":type"), typeToText(note.type));
    query.bindValue(QStringLiteral(":content"), plainText(note.content));
    query.bindValue(QStringLiteral(":audio_path"), nullableText(note.audioPath));
    query.bindValue(QStringLiteral(":audio_duration_s"), nullableInt(note.audioDurationS));
    query.bindValue(QStringLiteral(":category"), nullableText(note.category));
    query.bindValue(QStringLiteral(":state"), stateToText(note.state));
    query.bindValue(QStringLiteral(":needs_reembed"), note.needsReembed ? 1 : 0);
    query.bindValue(QStringLiteral(":analysis_attempts"), note.analysisAttempts);
    query.bindValue(QStringLiteral(":analysis_last_error"), nullableText(note.analysisLastError));
    query.bindValue(QStringLiteral(":task"), nullableText(note.task));
    query.bindValue(QStringLiteral(":origin"), nullableText(note.origin));
    query.bindValue(QStringLiteral(":origin_app"), nullableText(note.originApp));
    query.bindValue(QStringLiteral(":id"), note.id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        m_lastError = QStringLiteral("Notiz %1 existiert nicht").arg(note.id);
        return false;
    }

    return true;
}

std::optional<Note> Store::note(qint64 id) const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT %1 FROM notes WHERE id = :id").arg(noteColumns()));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }

    if (!query.next()) {
        m_lastError = QStringLiteral("Notiz %1 existiert nicht").arg(id);
        return std::nullopt;
    }

    return noteFromQuery(query);
}

QList<Note> Store::notes() const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // The id breaks ties: two notes of the same millisecond would otherwise
    // change places between two reads, and the list would jump.
    query.prepare(QStringLiteral("SELECT %1 FROM notes ORDER BY created_at DESC, id DESC").arg(noteColumns()));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return {};
    }

    QList<Note> notes;
    while (query.next()) {
        notes.append(noteFromQuery(query));
    }
    return notes;
}

QList<Note> Store::search(const QString &text, QStringList *correctedTo) const
{
    if (correctedTo != nullptr) {
        correctedTo->clear();
    }

    m_lastError.clear();
    const SearchQuery parsed = parseSearchQuery(text);
    if (parsed.isEmpty()) {
        return notes();
    }

    // Not const: it is handed back on two roads below, and a const local
    // cannot be moved out of.
    QList<Note> literal = notesMatching(parsed);
    // **The second pass runs on nothing found and not on few found**
    // (UX decision 30.08.2026): every hit of the first pass is a hit on what
    // the user really typed, and putting hits on another word beside them
    // would push their own result out of the list. The trigram tokenizer finds
    // parts of words at any position, so „p", „pr" and „prüfe" all have hits
    // while a correct word is being typed — the correction therefore fires at
    // the moment a wrong word is finished, and not on the way there.
    if (!literal.isEmpty() || !prepareCorrections()) {
        return literal;
    }

    SearchQuery corrected = parsed;
    QStringList replaced;
    bool changed = false;
    for (QString &term : corrected.terms) {
        // A phrase carries spaces and is searched for as it stands; the word
        // list holds single words, so there is nothing to look a phrase up in.
        // Terms below three characters take the substring route, where a
        // correction would turn „ad" into some word of the corpus.
        if (term.size() < trigramLength || term.contains(QLatin1Char(' '))) {
            continue;
        }
        const QString candidate = correctionFor(term);
        if (candidate.isEmpty() || candidate.compare(term, Qt::CaseInsensitive) == 0) {
            continue;
        }
        // Only a real typo is reported. „pruefen" and „prüfen" fold to the
        // same text, and then nothing happened the user has to be told about.
        if (germanFold(candidate) != germanFold(term)) {
            replaced.append(candidate);
        }
        term = candidate;
        changed = true;
    }
    if (!changed) {
        return literal;
    }

    const QList<Note> found = notesMatching(corrected);
    if (found.isEmpty()) {
        // Nothing was gained, so nothing is said: the library keeps its "No
        // matches" page with the sentence it has always carried.
        return found;
    }
    if (correctedTo != nullptr) {
        *correctedTo = replaced;
    }
    return found;
}

QList<Note> Store::notesMatching(const SearchQuery &parsed) const
{
    // Terms of three characters and more go through the trigram index. Shorter
    // ones cannot be in it — a trigram is three characters by definition — and
    // take a plain substring comparison instead, so that „KI" or „PO" find
    // something instead of silently nothing (SPEC 6).
    //
    // ponytail: that comparison is a `LIKE '%…%'`, which no index can serve.
    // Ceiling: a query in which **not one** term reaches three characters reads
    // every row of `notes` — 3 ms at 20,000 notes against the index route's
    // 9 ms (SPEC 6), and it grows with the corpus rather than with the number
    // of hits. A query that carries a long term as well does not pay it:
    // EXPLAIN QUERY PLAN on SQLite 3.53.4 gives `SEARCH notes USING INTEGER
    // PRIMARY KEY` under the FTS subquery for the mixed query and a bare
    // `SCAN notes` only for the all-short one, so there the LIKE sees the rows
    // the index has already picked. The way up is not a second index: FTS5
    // brings no tokenizer that holds anything shorter than three characters,
    // and `trigram` takes no size argument (`tokenize='trigram 2'` dies in the
    // tokenizer constructor). It would be a tokenizer of our own, registered
    // with `xCreateTokenizer_v2()` on the `fts5_api` that `SELECT fts5(?1)`
    // hands out to a pointer bound as `fts5_api_ptr` — on the `sqlite3*` from
    // `QSqlDriver::handle()` — plus a second index and a migration for it, and
    // plus a build dependency this project does not have today: `fts5_api` is
    // declared in `sqlite3.h`, and SQLite arrives here only through Qt's
    // driver. That the path is walkable at all rests on that driver linking the
    // system library rather than a copy of its own, so the tokenizer would be
    // registered in the same instance the queries run in. Measure before
    // building that.
    //
    // A phrase takes the same two roads as a word, and by its length like one:
    // „Backup prüfen" is long enough for the index, and the trigram tokenizer
    // holds the space between the two words like any other character, so the
    // FTS5 phrase finds exactly that sequence. Only a phrase under three
    // characters — `"KI"` — falls to the LIKE route, and there it is a
    // substring like everything else.
    QStringList phrases;
    QStringList shortTerms;
    for (const QString &term : parsed.terms) {
        if (term.size() >= trigramLength) {
            phrases.append(QStringLiteral("\"%1\"").arg(term));
        } else {
            shortTerms.append(term);
        }
    }

    // Every component of SPEC 6 is one condition, and they are ANDed — text,
    // tags, category, type and the two date boundaries alike.
    QStringList conditions;
    if (!phrases.isEmpty()) {
        conditions.append(QStringLiteral("id IN (SELECT rowid FROM notes_fts WHERE notes_fts MATCH :match)"));
    }
    for (qsizetype index = 0; index < shortTerms.size(); ++index) {
        // ESCAPE, because a phrase may carry a percent sign or an underscore.
        conditions.append(QStringLiteral("content LIKE :short%1 ESCAPE '\\'").arg(index));
    }
    for (qsizetype index = 0; index < parsed.tags.size(); ++index) {
        // Two tags are two conditions, so the note has to carry both.
        //
        // ponytail: COLLATE NOCASE folds ASCII case and nothing else, so
        // `tag:Buecher` finds the tag `buecher` while `tag:BÜCHER` does not
        // find `bücher`. Ceiling and upgrade path are the ones the short-term
        // route already carries (SPEC 6); the tags the analysis writes are
        // lower case to begin with (SPEC 7.2).
        conditions.append(
            QStringLiteral("id IN (SELECT note_id FROM tags WHERE tag = :tag%1 COLLATE NOCASE)").arg(index));
    }
    for (qsizetype index = 0; index < parsed.categories.size(); ++index) {
        conditions.append(QStringLiteral("category = :kat%1 COLLATE NOCASE").arg(index));
    }
    for (qsizetype index = 0; index < parsed.types.size(); ++index) {
        conditions.append(QStringLiteral("type = :typ%1").arg(index));
    }
    // `created_at` is ISO 8601, so the boundary compares as text: every
    // timestamp of the 1st of July begins with „2026-07-01" and is longer than
    // it, and therefore greater. Both operators get the first day of what was
    // typed, and these two comparisons are the whole difference: `vor:` leaves
    // the named day out, `nach:` takes it in.
    if (parsed.before.isValid()) {
        conditions.append(QStringLiteral("created_at < :before"));
    }
    if (parsed.after.isValid()) {
        conditions.append(QStringLiteral("created_at >= :after"));
    }

    QSqlQuery query(m_db);
    // The rows are picked by id rather than joined: `notes` and `notes_fts`
    // both have a column named `content`, which a join would make ambiguous.
    // The order is the library's, not FTS5's relevance ranking — the result
    // list keeps the day grouping of the note list (SPEC 9).
    query.prepare(QStringLiteral("SELECT %1 FROM notes WHERE %2"
                                 " ORDER BY created_at DESC, id DESC")
                      .arg(noteColumns(), conditions.join(QStringLiteral(" AND "))));
    if (!phrases.isEmpty()) {
        // A sequence of phrases is an AND to FTS5.
        query.bindValue(QStringLiteral(":match"), phrases.join(QLatin1Char(' ')));
    }
    for (qsizetype index = 0; index < shortTerms.size(); ++index) {
        query.bindValue(QStringLiteral(":short%1").arg(index), likePattern(shortTerms.at(index)));
    }
    for (qsizetype index = 0; index < parsed.tags.size(); ++index) {
        query.bindValue(QStringLiteral(":tag%1").arg(index), parsed.tags.at(index));
    }
    for (qsizetype index = 0; index < parsed.categories.size(); ++index) {
        query.bindValue(QStringLiteral(":kat%1").arg(index), parsed.categories.at(index));
    }
    for (qsizetype index = 0; index < parsed.types.size(); ++index) {
        query.bindValue(QStringLiteral(":typ%1").arg(index), parsed.types.at(index));
    }
    if (parsed.before.isValid()) {
        query.bindValue(QStringLiteral(":before"), parsed.before.toString(Qt::ISODate));
    }
    if (parsed.after.isValid()) {
        query.bindValue(QStringLiteral(":after"), parsed.after.toString(Qt::ISODate));
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return {};
    }

    QList<Note> found;
    while (query.next()) {
        found.append(noteFromQuery(query));
    }
    return found;
}

bool Store::removeNote(qint64 id)
{
    m_lastError.clear();
    // Read the audio reference while the row still exists, so the file that is
    // removed further down is the one the deleted note pointed at.
    const std::optional<Note> stored = note(id);
    if (!stored.has_value()) {
        return false;
    }

    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery removeTags(m_db);
    removeTags.prepare(QStringLiteral("DELETE FROM tags WHERE note_id = :id"));
    removeTags.bindValue(QStringLiteral(":id"), id);
    if (!removeTags.exec()) {
        m_lastError = removeTags.lastError().text();
        m_db.rollback();
        return false;
    }

    QSqlQuery removeNote(m_db);
    removeNote.prepare(QStringLiteral("DELETE FROM notes WHERE id = :id"));
    removeNote.bindValue(QStringLiteral(":id"), id);
    if (!removeNote.exec()) {
        m_lastError = removeNote.lastError().text();
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    // Database and file system cannot be committed together. The database is
    // the authority, so the file goes last: an interruption in between leaves
    // an orphaned audio file — never a note pointing at a missing file. The
    // orphan sweep (T7) cleans those up later.
    if (!stored->audioPath.isEmpty()) {
        const QString file = audioDirectory() + QLatin1Char('/') + stored->audioPath;
        if (QFile::exists(file) && !QFile::remove(file)) {
            // The note is gone either way; deleting it did not fail.
            qWarning("Deleting the audio file %s failed", qUtf8Printable(file));
        }
    }

    return true;
}

void Store::sweepOrphanedAudio()
{
    m_lastError.clear();

    QSqlQuery referencedQuery(m_db);
    // IS NOT NULL and not `<> ''`: a note without audio binds the column as
    // NULL (nullableText() above), and every comparison against NULL is NULL.
    if (!referencedQuery.exec(
            QStringLiteral("SELECT audio_path FROM notes WHERE audio_path IS NOT NULL"))) {
        // Without the list of what is referenced every file looks orphaned, so
        // nothing goes. Self-healing may cost a stray file, never a recording.
        m_lastError = referencedQuery.lastError().text();
        qWarning("Reading the referenced audio files failed, nothing swept: %s",
                 qUtf8Printable(m_lastError));
        return;
    }

    QSet<QString> referenced;
    while (referencedQuery.next()) {
        referenced.insert(referencedQuery.value(0).toString());
    }

    const QDir directory(audioDirectory());
    const QStringList present = directory.entryList(QDir::Files);
    for (const QString &name : present) {
        if (referenced.contains(name)) {
            continue;
        }
        if (QFile::remove(directory.filePath(name))) {
            qInfo("Removed the orphaned audio file %s", qUtf8Printable(name));
        } else {
            qWarning("Deleting the orphaned audio file %s failed", qUtf8Printable(name));
        }
    }
}

bool Store::replaceTags(qint64 noteId, const QStringList &tags)
{
    QSqlQuery clear(m_db);
    clear.prepare(QStringLiteral("DELETE FROM tags WHERE note_id = :note_id"));
    clear.bindValue(QStringLiteral(":note_id"), noteId);
    if (!clear.exec()) {
        m_lastError = clear.lastError().text();
        return false;
    }

    QSqlQuery insert(m_db);
    insert.prepare(QStringLiteral("INSERT INTO tags (note_id, tag) VALUES (:note_id, :tag)"));
    for (const QString &tag : tags) {
        insert.bindValue(QStringLiteral(":note_id"), noteId);
        insert.bindValue(QStringLiteral(":tag"), tag);
        if (!insert.exec()) {
            m_lastError = insert.lastError().text();
            return false;
        }
    }

    return true;
}

bool Store::setTags(qint64 noteId, const QStringList &tags)
{
    m_lastError.clear();
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    if (!replaceTags(noteId, tags)) {
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return true;
}

QList<Note> Store::unanalysedNotes() const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // Oldest first — a run works through what has been waiting longest, and the
    // id breaks the tie the way the note list does.
    query.prepare(QStringLiteral("SELECT %1 FROM notes"
                                 " WHERE state != :state AND TRIM(content) != ''"
                                 " ORDER BY created_at, id")
                      .arg(noteColumns()));
    query.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return {};
    }

    QList<Note> notes;
    while (query.next()) {
        notes.append(noteFromQuery(query));
    }
    return notes;
}

bool Store::completeAnalysis(qint64 noteId, const QString &category, const QStringList &tags, const QString &task)
{
    m_lastError.clear();
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery write(m_db);
    write.prepare(QStringLiteral("UPDATE notes SET category = :category, state = :state, task = :task,"
                                 " analysis_attempts = 0, analysis_last_error = NULL"
                                 " WHERE id = :id"));
    write.bindValue(QStringLiteral(":category"), nullableText(category));
    write.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));
    write.bindValue(QStringLiteral(":task"), nullableText(task));
    write.bindValue(QStringLiteral(":id"), noteId);
    if (!write.exec()) {
        m_lastError = write.lastError().text();
        m_db.rollback();
        return false;
    }
    if (write.numRowsAffected() == 0) {
        m_lastError = QStringLiteral("Notiz %1 existiert nicht").arg(noteId);
        m_db.rollback();
        return false;
    }

    if (!replaceTags(noteId, tags)) {
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return true;
}

std::optional<int> Store::failAnalysis(qint64 noteId, const QString &error)
{
    m_lastError.clear();
    QSqlQuery write(m_db);
    write.prepare(QStringLiteral("UPDATE notes SET analysis_attempts = analysis_attempts + 1,"
                                 " analysis_last_error = :error WHERE id = :id"));
    write.bindValue(QStringLiteral(":error"), error);
    write.bindValue(QStringLiteral(":id"), noteId);
    if (!write.exec()) {
        m_lastError = write.lastError().text();
        return std::nullopt;
    }
    if (write.numRowsAffected() == 0) {
        m_lastError = QStringLiteral("Notiz %1 existiert nicht").arg(noteId);
        return std::nullopt;
    }

    QSqlQuery read(m_db);
    read.prepare(QStringLiteral("SELECT analysis_attempts FROM notes WHERE id = :id"));
    read.bindValue(QStringLiteral(":id"), noteId);
    if (!read.exec() || !read.next()) {
        m_lastError = read.lastError().text();
        return std::nullopt;
    }
    return read.value(0).toInt();
}

QList<Note> Store::notesToEmbed(const QString &model) const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // Oldest first, like the classification run reads its notes.
    query.prepare(QStringLiteral("SELECT %1 FROM notes"
                                 " LEFT JOIN embeddings ON embeddings.note_id = notes.id"
                                 " WHERE state = :state AND TRIM(content) != ''"
                                 " AND (embeddings.note_id IS NULL OR embeddings.model != :model"
                                 "      OR needs_reembed = 1)"
                                 " ORDER BY created_at, id")
                      .arg(noteColumns()));
    query.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));
    query.bindValue(QStringLiteral(":model"), model);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return {};
    }

    QList<Note> notes;
    while (query.next()) {
        notes.append(noteFromQuery(query));
    }
    return notes;
}

bool Store::setEmbedding(qint64 noteId, const QString &model, const QList<float> &vector)
{
    m_lastError.clear();
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery write(m_db);
    write.prepare(QStringLiteral("INSERT INTO embeddings (note_id, model, vector)"
                                 " VALUES (:id, :model, :vector)"
                                 " ON CONFLICT(note_id) DO UPDATE SET"
                                 " model = excluded.model, vector = excluded.vector"));
    write.bindValue(QStringLiteral(":id"), noteId);
    write.bindValue(QStringLiteral(":model"), model);
    write.bindValue(QStringLiteral(":vector"), vectorToBlob(vector));
    if (!write.exec()) {
        m_lastError = write.lastError().text();
        m_db.rollback();
        return false;
    }

    // In the same transaction, because that is what makes the two agree: the
    // flag says "the vector is older than the text", and a vector written with
    // the flag left standing would be embedded again in every run to come.
    QSqlQuery clear(m_db);
    clear.prepare(QStringLiteral("UPDATE notes SET needs_reembed = 0 WHERE id = :id"));
    clear.bindValue(QStringLiteral(":id"), noteId);
    if (!clear.exec()) {
        m_lastError = clear.lastError().text();
        m_db.rollback();
        return false;
    }
    if (clear.numRowsAffected() == 0) {
        m_lastError = QStringLiteral("Notiz %1 existiert nicht").arg(noteId);
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return true;
}

QList<NoteEmbedding> Store::embeddings(const QString &model) const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // **Unexported means still there**: SPEC 8.1 deletes the notes of a
    // confirmed export in the same transaction, so what stands in the table is
    // what has not been exported. Analysed is a column, and the model is what
    // keeps two vector spaces from being compared with each other.
    query.prepare(QStringLiteral("SELECT embeddings.note_id, embeddings.vector FROM embeddings"
                                 " JOIN notes ON notes.id = embeddings.note_id"
                                 " WHERE embeddings.model = :model AND notes.state = :state"
                                 " ORDER BY notes.created_at, notes.id"));
    query.bindValue(QStringLiteral(":model"), model);
    query.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return {};
    }

    QList<NoteEmbedding> embeddings;
    while (query.next()) {
        embeddings.append({query.value(0).toLongLong(), vectorFromBlob(query.value(1).toByteArray())});
    }
    return embeddings;
}

std::optional<qint64> Store::addProposal(const Proposal &proposal)
{
    m_lastError.clear();
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return std::nullopt;
    }

    QSqlQuery write(m_db);
    write.prepare(QStringLiteral("INSERT INTO proposals (kind, created_at, status, payload)"
                                 " VALUES (:kind, :created_at, :status, :payload)"));
    write.bindValue(QStringLiteral(":kind"), kindToText(proposal.kind));
    write.bindValue(QStringLiteral(":created_at"), timestampToText(proposal.createdAt));
    write.bindValue(QStringLiteral(":status"), statusToText(proposal.status));
    write.bindValue(QStringLiteral(":payload"), plainText(proposal.payload));
    if (!write.exec()) {
        m_lastError = write.lastError().text();
        m_db.rollback();
        return std::nullopt;
    }

    const qint64 id = write.lastInsertId().toLongLong();
    for (const qint64 noteId : proposal.noteIds) {
        QSqlQuery link(m_db);
        link.prepare(QStringLiteral("INSERT INTO proposal_notes (proposal_id, note_id)"
                                    " VALUES (:proposal_id, :note_id)"));
        link.bindValue(QStringLiteral(":proposal_id"), id);
        link.bindValue(QStringLiteral(":note_id"), noteId);
        if (!link.exec()) {
            m_lastError = link.lastError().text();
            m_db.rollback();
            return std::nullopt;
        }
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return std::nullopt;
    }

    return id;
}

QList<Note> Store::notesForTaskProposals() const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // TRIM(task) != '' beside IS NOT NULL: completeAnalysis() binds an empty
    // task as NULL, and this is the condition that says "the note is a task"
    // (SPEC 5.1) — a text of nought length reaching it from anywhere else must
    // not become a suggestion carrying no description.
    query.prepare(QStringLiteral("SELECT %1 FROM notes"
                                 " WHERE state = :state AND task IS NOT NULL AND TRIM(task) != ''"
                                 " AND NOT EXISTS ("
                                 "   SELECT 1 FROM proposal_notes"
                                 "   JOIN proposals ON proposals.id = proposal_notes.proposal_id"
                                 "   WHERE proposal_notes.note_id = notes.id AND proposals.kind = 'task')"
                                 " ORDER BY created_at, id")
                      .arg(noteColumns()));
    query.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return {};
    }

    QList<Note> notes;
    while (query.next()) {
        notes.append(noteFromQuery(query));
    }
    return notes;
}

QList<Proposal> Store::proposals() const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // One query with the links joined on, and the rows read in one pass: the
    // suggestions of a run are a handful, and a second query per suggestion
    // would be one round trip each for a list nobody scrolls.
    //
    // The notes come back **oldest first**, and that is by the note's own
    // timestamp rather than by its id: a note carries the moment it was
    // captured (SPEC 5.1), so a row written later can hold an earlier moment,
    // and ordering by the id would hand the review a bundle in an order the
    // collective note beside it does not have. Both joins are LEFT ones,
    // because a suggestion without notes has to keep its row.
    if (!query.exec(QStringLiteral("SELECT proposals.id, kind, proposals.created_at, status, payload, note_id"
                                   " FROM proposals"
                                   " LEFT JOIN proposal_notes ON proposal_notes.proposal_id = proposals.id"
                                   " LEFT JOIN notes ON notes.id = proposal_notes.note_id"
                                   " ORDER BY proposals.created_at, proposals.id,"
                                   " notes.created_at, note_id"))) {
        m_lastError = query.lastError().text();
        return {};
    }

    QList<Proposal> proposals;
    while (query.next()) {
        const qint64 id = query.value(0).toLongLong();
        if (proposals.isEmpty() || proposals.constLast().id != id) {
            Proposal proposal;
            proposal.id = id;
            proposal.kind = kindFromText(query.value(1).toString());
            proposal.createdAt = timestampFromText(query.value(2).toString());
            proposal.status = statusFromText(query.value(3).toString());
            proposal.payload = query.value(4).toString();
            proposals.append(proposal);
        }
        // NULL for a suggestion without a single note — what the LEFT JOIN is
        // there for. It cannot come out of addProposal(), and a row hand-written
        // into the database must not make the whole list unreadable.
        if (!query.value(5).isNull()) {
            proposals.last().noteIds.append(query.value(5).toLongLong());
        }
    }
    return proposals;
}

bool Store::removeProposal(qint64 id)
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM proposals WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Store::enqueueTranscription(qint64 noteId)
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // A note that is already queued stays as it is, attempts included: the
    // road into the queue is Store::noteAdded, and a second start of the
    // daemon must not hand a job that has already failed twice a fresh life.
    query.prepare(QStringLiteral("INSERT INTO transcribe_jobs (note_id, enqueued_at)"
                                 " VALUES (:note_id, :enqueued_at)"
                                 " ON CONFLICT(note_id) DO NOTHING"));
    query.bindValue(QStringLiteral(":note_id"), noteId);
    query.bindValue(QStringLiteral(":enqueued_at"), timestampToText(QDateTime::currentDateTime()));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

std::optional<TranscribeJob> Store::takeTranscribeJob()
{
    m_lastError.clear();
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return std::nullopt;
    }

    QSqlQuery query(m_db);
    // Oldest first, and the id breaks the tie the way the note list does.
    query.prepare(QStringLiteral("SELECT note_id, enqueued_at, attempts, last_error FROM transcribe_jobs"
                                 " WHERE attempts < :limit ORDER BY enqueued_at, note_id LIMIT 1"));
    query.bindValue(QStringLiteral(":limit"), transcribeAttemptLimit);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_db.rollback();
        return std::nullopt;
    }
    if (!query.next()) {
        m_db.rollback();
        return std::nullopt;
    }

    TranscribeJob job;
    job.noteId = query.value(0).toLongLong();
    job.enqueuedAt = timestampFromText(query.value(1).toString());
    job.attempts = query.value(2).toInt() + 1;
    job.lastError = query.value(3).toString();

    QSqlQuery count(m_db);
    count.prepare(QStringLiteral("UPDATE transcribe_jobs SET attempts = :attempts WHERE note_id = :note_id"));
    count.bindValue(QStringLiteral(":attempts"), job.attempts);
    count.bindValue(QStringLiteral(":note_id"), job.noteId);
    if (!count.exec()) {
        m_lastError = count.lastError().text();
        m_db.rollback();
        return std::nullopt;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return std::nullopt;
    }

    return job;
}

bool Store::completeTranscription(qint64 noteId, const QString &transcript)
{
    m_lastError.clear();
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery write(m_db);
    // The state goes to 'transkribiert' with the text, and the FTS trigger of
    // schema version 2 carries the transcript into the search index from here.
    write.prepare(QStringLiteral("UPDATE notes SET content = :content, state = :state WHERE id = :id"));
    write.bindValue(QStringLiteral(":content"), transcript);
    write.bindValue(QStringLiteral(":state"), stateToText(Note::State::Transcribed));
    write.bindValue(QStringLiteral(":id"), noteId);
    if (!write.exec()) {
        m_lastError = write.lastError().text();
        m_db.rollback();
        return false;
    }
    if (write.numRowsAffected() == 0) {
        m_lastError = QStringLiteral("Notiz %1 existiert nicht").arg(noteId);
        m_db.rollback();
        return false;
    }

    QSqlQuery done(m_db);
    done.prepare(QStringLiteral("DELETE FROM transcribe_jobs WHERE note_id = :note_id"));
    done.bindValue(QStringLiteral(":note_id"), noteId);
    if (!done.exec()) {
        m_lastError = done.lastError().text();
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return true;
}

bool Store::failTranscribeJob(qint64 noteId, const QString &error)
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE transcribe_jobs SET last_error = :last_error WHERE note_id = :note_id"));
    query.bindValue(QStringLiteral(":last_error"), error);
    query.bindValue(QStringLiteral(":note_id"), noteId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

std::optional<TranscribeJob> Store::transcribeJob(qint64 noteId) const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT note_id, enqueued_at, attempts, last_error FROM transcribe_jobs"
                                 " WHERE note_id = :note_id"));
    query.bindValue(QStringLiteral(":note_id"), noteId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }

    TranscribeJob job;
    job.noteId = query.value(0).toLongLong();
    job.enqueuedAt = timestampFromText(query.value(1).toString());
    job.attempts = query.value(2).toInt();
    job.lastError = query.value(3).toString();
    return job;
}

std::optional<TranscribeJob> Store::pausedTranscribeJob() const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT note_id, enqueued_at, attempts, last_error FROM transcribe_jobs"
                                 " WHERE attempts >= :limit ORDER BY enqueued_at DESC, note_id DESC LIMIT 1"));
    query.bindValue(QStringLiteral(":limit"), transcribeAttemptLimit);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }

    TranscribeJob job;
    job.noteId = query.value(0).toLongLong();
    job.enqueuedAt = timestampFromText(query.value(1).toString());
    job.attempts = query.value(2).toInt();
    job.lastError = query.value(3).toString();
    return job;
}

bool Store::noteInterruptedTranscribeJobs(const QString &reason)
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    // NULL and the empty string both, and neither is theoretical: the column
    // starts out NULL, and failTranscribeJob() writes what it is handed.
    query.prepare(QStringLiteral("UPDATE transcribe_jobs SET last_error = :last_error"
                                 " WHERE attempts > 0 AND (last_error IS NULL OR last_error = '')"));
    query.bindValue(QStringLiteral(":last_error"), reason);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QStringList Store::tags(qint64 noteId) const
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT tag FROM tags WHERE note_id = :note_id ORDER BY tag"));
    query.bindValue(QStringLiteral(":note_id"), noteId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return {};
    }

    QStringList tags;
    while (query.next()) {
        tags.append(query.value(0).toString());
    }
    return tags;
}

CategoryCounts Store::categoryCounts() const
{
    m_lastError.clear();
    CategoryCounts counts;

    QSqlQuery byCategory(m_db);
    // Notes with no category yet are left out here rather than counted under an
    // empty key: they belong to no entry of the column, and until an analysis
    // run has been through them that is every note there is (SPEC 7.2).
    //
    // **Grouped over `LOWER(category)`, and that is the same reading `kat:`
    // uses** — SQLite folds ASCII case and nothing else, in `LOWER()` as in the
    // `COLLATE NOCASE` of search(), which is exactly what SPEC 6 promises. Case
    // exactly as stored, a `TODOs` out of a foreign or older database would
    // stand in the list under „TODOs" and be counted **0** beside it: the
    // filter of the library compares case-insensitively, the search finds it,
    // and only the counter would have disagreed. The key comes back folded, so
    // it meets the lower-case short forms the column asks with.
    if (!byCategory.exec(QStringLiteral("SELECT LOWER(category), COUNT(*) FROM notes"
                                        " WHERE category IS NOT NULL AND category != ''"
                                        " GROUP BY LOWER(category)"))) {
        m_lastError = byCategory.lastError().text();
        return {};
    }
    while (byCategory.next()) {
        counts.byCategory.insert(byCategory.value(0).toString(), byCategory.value(1).toInt());
    }

    // Asked of the table rather than added up from the group counts above: a
    // note without a category stands in none of them, and "All" means all.
    QSqlQuery total(m_db);
    if (!total.exec(QStringLiteral("SELECT COUNT(*) FROM notes")) || !total.next()) {
        m_lastError = total.lastError().text();
        return {};
    }
    counts.total = total.value(0).toInt();

    // What the classifier gave up on **and** what therefore carries no category
    // — the second half added on 30.08.2026 (issue #133, UX decision).
    //
    // Three of the four parts are the condition Classifier::start() skips by,
    // written once more in SQL: counting the notes it hands out would mean
    // reading every one of them. `TRIM(content) != ''` belongs to
    // unanalysedNotes(), which is where the run takes its queue from, so a note
    // without text is never handed out and never given up on either.
    //
    // The fourth, `category IS NULL OR category = ''`, is **not** the
    // classifier's: a note carrying `ideen` and three tags is not uneingeordnet,
    // whatever its attempt counter says, and counting it under that name was
    // wrong before it was ever a question of sums. That it also makes the three
    // pots disjoint by construction — a category puts a note in byCategory, no
    // category puts it in exactly one of the two rows below — is the reason it
    // is written here and not in the interface: with it,
    // `total = Sigma byCategory + unclassified + waiting` holds for **every**
    // row the database can hold, and not merely for the ones the writers happen
    // to produce today. The way to such a row is real and was measured
    // (librarytest, `aNoteThatKeepsItsCategoryAcrossALateTranscript`).
    //
    // **What this does not reach**: `Classifier::start()` still reports such a
    // note through paused(), because unanalysedNotes() asks nothing about the
    // category. The window offers a way to it all the same — it stands in the
    // row of its own category — so the guarantee this count was built for holds
    // (issue #118). The journal line in main.cpp calls it "left without a
    // category", which is the wording that is now imprecise, not the reach.
    QSqlQuery unclassified(m_db);
    unclassified.prepare(QStringLiteral("SELECT COUNT(*) FROM notes"
                                        " WHERE (category IS NULL OR category = '')"
                                        " AND state != :state AND TRIM(content) != ''"
                                        " AND analysis_attempts >= :limit"));
    unclassified.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));
    unclassified.bindValue(QStringLiteral(":limit"), analysisAttemptLimit);
    if (!unclassified.exec() || !unclassified.next()) {
        m_lastError = unclassified.lastError().text();
        return {};
    }
    counts.unclassified = unclassified.value(0).toInt();

    // What is left over once the two pots above have taken theirs (issue #133):
    // no category, and not one of the given-up notes. Written as the
    // complement of the clause above rather than as a condition of its own —
    // `state != 'analysed' AND analysis_attempts < 2` would read the same today
    // and drift apart the moment either side is touched, and the column's
    // promise is the **sum**, not the wording.
    //
    // The two share their first line word for word, and that is what makes the
    // sum exact rather than likely: within "no category" the given-up clause
    // and its negation split the notes in two, so the three pots are disjoint
    // and cover the table. Whoever edits one of the two conditions edits the
    // other in the same breath, or the sum stops holding.
    //
    // The negation carries the voice note without a transcript by itself:
    // `TRIM(content) != ''` is false for it, so `NOT(...)` is true and it lands
    // here instead of nowhere.
    QSqlQuery waiting(m_db);
    waiting.prepare(QStringLiteral("SELECT COUNT(*) FROM notes"
                                   " WHERE (category IS NULL OR category = '')"
                                   " AND NOT (state != :state AND TRIM(content) != ''"
                                   " AND analysis_attempts >= :limit)"));
    waiting.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));
    waiting.bindValue(QStringLiteral(":limit"), analysisAttemptLimit);
    if (!waiting.exec() || !waiting.next()) {
        m_lastError = waiting.lastError().text();
        return {};
    }
    counts.waiting = waiting.value(0).toInt();

    return counts;
}
