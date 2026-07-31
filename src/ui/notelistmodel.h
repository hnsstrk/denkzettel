#pragma once

#include "store/note.h"

#include <QAbstractListModel>
#include <QList>

/**
 * The notes of the library list, newest first (SPEC 9).
 *
 * The model holds the notes it was given; it never reads or writes the store
 * itself. That keeps the pending deletion honest: the note leaves the list the
 * moment it is deleted, while the row is still recoverable for the undo.
 */
class NoteListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        /** The timestamp as the list shows it, see library::relativeTimestamp. */
        TimestampRole = Qt::UserRole,
    };

    explicit NoteListModel(QObject *parent = nullptr);

    void setNotes(const QList<Note> &notes);

    /** The note in `row`; an empty note for a row outside the list. */
    Note noteAt(int row) const;

    /** Removes a row without touching the store. */
    void takeRow(int row);

    /** Puts a note back where takeRow() removed it. */
    void insertNote(int row, const Note &note);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<Note> m_notes;
};
