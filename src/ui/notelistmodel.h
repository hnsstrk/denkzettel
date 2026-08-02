#pragma once

#include "store/note.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QString>

/**
 * The notes of the library list, newest first, grouped like an inbox
 * (SPEC 9, wireframe 3a): each group of notes carries a head row above it.
 *
 * A row is therefore either a group head or a note, and row numbers are no
 * longer note numbers. Everything that means a note — deleting, undoing,
 * following the selection — counts in note indices; `rowOfNote()` and
 * `noteIndexAt()` translate between the two.
 *
 * The model holds the notes it was given; it never reads or writes the store
 * itself. That keeps the pending deletion honest: the note leaves the list the
 * moment it is deleted, while the row is still recoverable for the undo.
 *
 * The point in time the grouping is measured against comes from the caller and
 * is not read off the clock here: without it every test of a group would
 * depend on the day it runs on.
 */
class NoteListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        /** The timestamp as the list shows it, see library::entryTimestamp. */
        TimestampRole = Qt::UserRole,
        /** True on the rows that carry a group heading rather than a note. */
        GroupHeaderRole,
    };

    explicit NoteListModel(QObject *parent = nullptr);

    /** Takes the notes and groups them as of `now`. */
    void setNotes(const QList<Note> &notes, const QDateTime &now);

    /** Groups the notes it already holds again, as of `now`. */
    void regroup(const QDateTime &now);

    /** How many notes the list holds — `rowCount()` counts the heads as well. */
    int noteCount() const;

    /** The note in `row`; an empty note for a head row or a row outside the list. */
    Note noteAt(int row) const;

    /** Where `row` sits in the note order; -1 for a head row. */
    int noteIndexAt(int row) const;

    /** The row showing the note `noteIndex`, or -1 if there is none. */
    int rowOfNote(int noteIndex) const;

    /** The row holding the note `noteId`, or -1 if the list has none. */
    int rowOf(qint64 noteId) const;

    /**
     * Removes a note without touching the store. Was it the last one of its
     * group, its head goes with it.
     */
    void takeNote(int noteIndex);

    /** Puts a note back where takeNote() removed it, with its head if needed. */
    void insertNote(int noteIndex, const Note &note);

    /**
     * Writes an edited note over the one in `noteIndex`, keeping row and
     * grouping (SPEC 9).
     *
     * Editing leaves `created_at` alone, so no note can move to another group;
     * and reading the store again is exactly what must not happen here — a
     * note whose new text drops it out of the running result list stays in
     * sight until the search term changes (issue #11, K2).
     */
    void replaceNote(int noteIndex, const Note &note);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    /** A row of the list: a group head, or one of the notes below it. */
    struct Row {
        /** The note this row shows; -1 on a group head. */
        int note = -1;
        /** The heading; empty on a note row. */
        QString title;
    };

    static QList<Row> buildRows(const QList<Note> &notes, const QDateTime &now);
    static int rowOfNoteIn(const QList<Row> &rows, int noteIndex);

    QList<Note> m_notes;
    QList<Row> m_rows;
    QDateTime m_now;
};
