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

    /**
     * Notes whose text matches a free-text query, newest first (SPEC 6).
     *
     * The terms are ANDed, and each of them matches as a **prefix**: „foto"
     * finds „fotografieren", so the list narrows while the user is still
     * typing. There is no infix search — „grafieren" finds nothing.
     *
     * The tokenizer folds diacritics, so „bucher" finds „Bücher"; it does not
     * fold ß, so „strassenbahn" does not find „Straßenbahn".
     *
     * A query without any searchable term — empty, blank, pure punctuation —
     * returns the same list as notes(): clearing the search field restores the
     * full list.
     *
     * The search operators of SPEC 6 (`tag:`, `kat:`, phrases …) are not part
     * of this method; they arrive with the search parser (S7). Until then
     * every character the user types is plain search text.
     */
    QList<Note> search(const QString &text) const;

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
