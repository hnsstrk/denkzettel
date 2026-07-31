#include "store/store.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
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
        // Version 1 — M1 schema (SPEC 5.1). notes_fts, embeddings, proposals,
        // proposal_notes and transcribe_jobs arrive with their milestones.
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
                          " category, state, needs_reembed, analysis_attempts, analysis_last_error");
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
    const QString directory = QFileInfo(m_databasePath).absolutePath();
    if (!QDir().mkpath(directory)) {
        m_lastError = QStringLiteral("Verzeichnis %1 lässt sich nicht anlegen").arg(directory);
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
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("INSERT INTO notes (created_at, type, content, audio_path, audio_duration_s,"
                       " category, state, needs_reembed, analysis_attempts, analysis_last_error)"
                       " VALUES (:created_at, :type, :content, :audio_path, :audio_duration_s,"
                       " :category, :state, :needs_reembed, :analysis_attempts, :analysis_last_error)"));
    query.bindValue(QStringLiteral(":created_at"), timestampToText(note.createdAt));
    query.bindValue(QStringLiteral(":type"), typeToText(note.type));
    query.bindValue(QStringLiteral(":content"), note.content);
    query.bindValue(QStringLiteral(":audio_path"), nullableText(note.audioPath));
    query.bindValue(QStringLiteral(":audio_duration_s"), nullableInt(note.audioDurationS));
    query.bindValue(QStringLiteral(":category"), nullableText(note.category));
    query.bindValue(QStringLiteral(":state"), stateToText(note.state));
    query.bindValue(QStringLiteral(":needs_reembed"), note.needsReembed ? 1 : 0);
    query.bindValue(QStringLiteral(":analysis_attempts"), note.analysisAttempts);
    query.bindValue(QStringLiteral(":analysis_last_error"), nullableText(note.analysisLastError));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }

    return query.lastInsertId().toLongLong();
}

bool Store::updateNote(const Note &note)
{
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("UPDATE notes SET created_at = :created_at, type = :type, content = :content,"
                       " audio_path = :audio_path, audio_duration_s = :audio_duration_s,"
                       " category = :category, state = :state, needs_reembed = :needs_reembed,"
                       " analysis_attempts = :analysis_attempts, analysis_last_error = :analysis_last_error"
                       " WHERE id = :id"));
    query.bindValue(QStringLiteral(":created_at"), timestampToText(note.createdAt));
    query.bindValue(QStringLiteral(":type"), typeToText(note.type));
    query.bindValue(QStringLiteral(":content"), note.content);
    query.bindValue(QStringLiteral(":audio_path"), nullableText(note.audioPath));
    query.bindValue(QStringLiteral(":audio_duration_s"), nullableInt(note.audioDurationS));
    query.bindValue(QStringLiteral(":category"), nullableText(note.category));
    query.bindValue(QStringLiteral(":state"), stateToText(note.state));
    query.bindValue(QStringLiteral(":needs_reembed"), note.needsReembed ? 1 : 0);
    query.bindValue(QStringLiteral(":analysis_attempts"), note.analysisAttempts);
    query.bindValue(QStringLiteral(":analysis_last_error"), nullableText(note.analysisLastError));
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

bool Store::removeNote(qint64 id)
{
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
            qWarning("Audiodatei %s ließ sich nicht löschen", qUtf8Printable(file));
        }
    }

    return true;
}

bool Store::setTags(qint64 noteId, const QStringList &tags)
{
    if (!m_db.transaction()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    QSqlQuery clear(m_db);
    clear.prepare(QStringLiteral("DELETE FROM tags WHERE note_id = :note_id"));
    clear.bindValue(QStringLiteral(":note_id"), noteId);
    if (!clear.exec()) {
        m_lastError = clear.lastError().text();
        m_db.rollback();
        return false;
    }

    QSqlQuery insert(m_db);
    insert.prepare(QStringLiteral("INSERT INTO tags (note_id, tag) VALUES (:note_id, :tag)"));
    for (const QString &tag : tags) {
        insert.bindValue(QStringLiteral(":note_id"), noteId);
        insert.bindValue(QStringLiteral(":tag"), tag);
        if (!insert.exec()) {
            m_lastError = insert.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        m_lastError = m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    return true;
}

QStringList Store::tags(qint64 noteId) const
{
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
