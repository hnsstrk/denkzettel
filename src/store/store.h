#pragma once

#include "store/note.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include <optional>

/**
 * Access to the SQLite database (SPEC 5.1).
 *
 * One instance owns one database connection; the path is passed in so tests
 * can work on a temporary file instead of the user's database.
 *
 * All methods report failure through their return value; the reason is
 * available from lastError().
 */
class Store
{
public:
    explicit Store(const QString &databasePath);
    ~Store();

    Store(const Store &) = delete;
    Store &operator=(const Store &) = delete;

    /** `~/.local/share/denkzettel/denkzettel.db` for the running application. */
    static QString defaultDatabasePath();

    /** Opens the database, creating directory, file and schema as needed. */
    bool open();

    QString lastError() const;

    /** Schema version recorded in `meta`, 0 before open(). */
    int schemaVersion() const;

    /** Directory holding the audio files referenced by Note::audioPath. */
    QString audioDirectory() const;

    /** Inserts a note and returns its new id. */
    std::optional<qint64> addNote(const Note &note);

    /** Writes all fields of an existing note, identified by note.id. */
    bool updateNote(const Note &note);

    std::optional<Note> note(qint64 id) const;

    /** All notes, newest first — the order the library lists them in (SPEC 9). */
    QList<Note> notes() const;

    /** Deletes note and tags in one transaction, then its audio file. */
    bool removeNote(qint64 id);

    /** Replaces all tags of a note. */
    bool setTags(qint64 noteId, const QStringList &tags);

    QStringList tags(qint64 noteId) const;

private:
    bool migrate();

    QString m_databasePath;
    QString m_connectionName;
    QSqlDatabase m_db;
    int m_schemaVersion = 0;
    mutable QString m_lastError;
};
