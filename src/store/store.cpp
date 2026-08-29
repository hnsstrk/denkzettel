#include "store/store.h"

#include "store/searchquery.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>

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
    };
    return steps;
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
                          " category, state, needs_reembed, analysis_attempts, analysis_last_error, task");
}

/** Shortest term the trigram index can represent (SPEC 6). */
constexpr qsizetype trigramLength = 3;

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

    return migrate();
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

std::optional<qint64> Store::addNote(const Note &note)
{
    m_lastError.clear();
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("INSERT INTO notes (created_at, type, content, audio_path, audio_duration_s,"
                       " category, state, needs_reembed, analysis_attempts, analysis_last_error, task)"
                       " VALUES (:created_at, :type, :content, :audio_path, :audio_duration_s,"
                       " :category, :state, :needs_reembed, :analysis_attempts, :analysis_last_error, :task)"));
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
                       " task = :task"
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

QList<Note> Store::search(const QString &text) const
{
    m_lastError.clear();
    const SearchQuery parsed = parseSearchQuery(text);
    if (parsed.isEmpty()) {
        return notes();
    }

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
    if (!byCategory.exec(QStringLiteral("SELECT category, COUNT(*) FROM notes"
                                        " WHERE category IS NOT NULL AND category != ''"
                                        " GROUP BY category"))) {
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

    // The condition Classifier::start() skips by, written once more in SQL —
    // counting the notes it hands out would mean reading every one of them.
    QSqlQuery unclassified(m_db);
    unclassified.prepare(QStringLiteral("SELECT COUNT(*) FROM notes"
                                        " WHERE state != :state AND analysis_attempts >= :limit"));
    unclassified.bindValue(QStringLiteral(":state"), stateToText(Note::State::Analysed));
    unclassified.bindValue(QStringLiteral(":limit"), analysisAttemptLimit);
    if (!unclassified.exec() || !unclassified.next()) {
        m_lastError = unclassified.lastError().text();
        return {};
    }
    counts.unclassified = unclassified.value(0).toInt();

    return counts;
}
