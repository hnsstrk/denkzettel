#pragma once

#include "store/note.h"

#include <QList>
#include <QObject>
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
 *
 * It announces what it takes in (noteAdded), and that is why it is a QObject:
 * every road a note travels into the database runs through addNote() — the
 * capture window and the D-Bus method AddNote() today, whatever comes with the
 * transcription tomorrow. A window that listens here hears all of them; one
 * that listened to the capture window would hear one of them (issue #105).
 */
class Store : public QObject
{
    Q_OBJECT

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
     * The terms are ANDed, and each of them matches **anywhere inside a word**:
     * „foto", „grafieren" and „bahn" all find „fotografieren" resp.
     * „Straßenbahnen". Prefix matching is part of that, not a rule of its own.
     *
     * The tokenizer folds diacritics, so „bucher" finds „Bücher"; it does not
     * fold ß, so „strassenbahn" does not find „Straßenbahn" (S30).
     *
     * Terms of **one or two characters** cannot be in a trigram index and are
     * compared as substrings instead, so „KI" finds „KI-Pipeline". That route
     * ignores upper and lower case for ASCII only: it does not fold diacritics
     * („u" does not find „ü") and does not fold case beyond ASCII („ü" does
     * not find „Ü"). Both limits end at three characters.
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

Q_SIGNALS:
    /**
     * A note has been written and carries the id `id` (issue #105).
     *
     * Emitted after the insert has succeeded, so a listener that reads the
     * store again finds the note. It says that something was added, not what:
     * the library reads its own list back and applies its own search term to
     * it, and a signal carrying the note would tempt it to skip that.
     */
    void noteAdded(qint64 id);

private:
    bool migrate();

    QString m_databasePath;
    QString m_connectionName;
    QSqlDatabase m_db;
    int m_schemaVersion = 0;
    mutable QString m_lastError;
};
