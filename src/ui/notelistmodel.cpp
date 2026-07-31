#include "ui/notelistmodel.h"

#include "ui/timestampformat.h"

#include <QDateTime>
#include <QLocale>

NoteListModel::NoteListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void NoteListModel::setNotes(const QList<Note> &notes)
{
    beginResetModel();
    m_notes = notes;
    endResetModel();
}

Note NoteListModel::noteAt(int row) const
{
    if (row < 0 || row >= m_notes.size()) {
        return {};
    }
    return m_notes.at(row);
}

int NoteListModel::rowOf(qint64 noteId) const
{
    // A note that was never stored carries the default id, and so does the
    // empty note of a row outside the list — looking for it finds nothing.
    if (noteId < 0) {
        return -1;
    }

    for (int row = 0; row < m_notes.size(); ++row) {
        if (m_notes.at(row).id == noteId) {
            return row;
        }
    }

    return -1;
}

void NoteListModel::takeRow(int row)
{
    if (row < 0 || row >= m_notes.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_notes.removeAt(row);
    endRemoveRows();
}

void NoteListModel::insertNote(int row, const Note &note)
{
    const int target = qBound(0, row, m_notes.size());

    beginInsertRows(QModelIndex(), target, target);
    m_notes.insert(target, note);
    endInsertRows();
}

int NoteListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_notes.size();
}

QVariant NoteListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_notes.size()) {
        return {};
    }

    const Note &note = m_notes.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return note.content;
    case TimestampRole:
        return library::relativeTimestamp(note.createdAt, QDateTime::currentDateTime(), QLocale());
    default:
        return {};
    }
}
