#include "ui/notelistmodel.h"

#include "ui/timestampformat.h"

#include <KLocalizedString>

#include <QLocale>
#include <optional>

NoteListModel::NoteListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QList<NoteListModel::Row> NoteListModel::buildRows(const QList<Note> &notes, const QDateTime &now)
{
    QList<Row> rows;
    rows.reserve(notes.size() * 2);

    // The notes arrive newest first and the groups are ordered the same way, so
    // a group is done the moment the next note falls into another one — an
    // empty group never comes up and is never drawn (wireframe 3b).
    std::optional<library::NoteGroup> open;

    for (int index = 0; index < notes.size(); ++index) {
        const library::NoteGroup group = library::noteGroup(notes.at(index).createdAt, now, QLocale());
        if (open != group) {
            open = group;
            rows.append(Row{-1, library::groupTitle(group)});
        }
        rows.append(Row{index, QString()});
    }

    return rows;
}

void NoteListModel::setNotes(const QList<Note> &notes, const QDateTime &now)
{
    beginResetModel();
    m_notes = notes;
    m_now = now;
    m_rows = buildRows(m_notes, m_now);
    endResetModel();
}

void NoteListModel::regroup(const QDateTime &now)
{
    beginResetModel();
    m_now = now;
    m_rows = buildRows(m_notes, m_now);
    endResetModel();
}

int NoteListModel::noteCount() const
{
    return static_cast<int>(m_notes.size());
}

Note NoteListModel::noteAt(int row) const
{
    const int index = noteIndexAt(row);
    if (index < 0) {
        return {};
    }
    return m_notes.at(index);
}

int NoteListModel::noteIndexAt(int row) const
{
    if (row < 0 || row >= m_rows.size()) {
        return -1;
    }
    return m_rows.at(row).note;
}

int NoteListModel::rowOfNoteIn(const QList<Row> &rows, int noteIndex)
{
    if (noteIndex < 0) {
        return -1;
    }

    for (int row = 0; row < rows.size(); ++row) {
        if (rows.at(row).note == noteIndex) {
            return row;
        }
    }

    return -1;
}

int NoteListModel::rowOfNote(int noteIndex) const
{
    return rowOfNoteIn(m_rows, noteIndex);
}

int NoteListModel::rowOf(qint64 noteId) const
{
    // A note that was never stored carries the default id, and so does the
    // empty note of a head row — looking for it finds nothing.
    if (noteId < 0) {
        return -1;
    }

    for (int index = 0; index < m_notes.size(); ++index) {
        if (m_notes.at(index).id == noteId) {
            return rowOfNote(index);
        }
    }

    return -1;
}

void NoteListModel::takeNote(int noteIndex)
{
    if (noteIndex < 0 || noteIndex >= m_notes.size()) {
        return;
    }

    const int row = rowOfNote(noteIndex);
    // The head above goes with the note if nothing is left under it. Head and
    // note are adjacent rows, so both leave in one removal.
    const bool aloneInItsGroup =
        row > 0 && m_rows.at(row - 1).note < 0 && (row + 1 >= m_rows.size() || m_rows.at(row + 1).note < 0);
    const int first = aloneInItsGroup ? row - 1 : row;

    // Against the reference time the list was built with, not against a fresh
    // one: only then does the undo put the note back where it was. A deletion
    // is no occasion to regroup — the note that leaves does not change what
    // group the others are in.
    beginRemoveRows(QModelIndex(), first, row);
    m_notes.removeAt(noteIndex);
    m_rows = buildRows(m_notes, m_now);
    endRemoveRows();
}

void NoteListModel::insertNote(int noteIndex, const Note &note)
{
    const int target = qBound(0, noteIndex, noteCount());

    QList<Note> notes = m_notes;
    notes.insert(target, note);
    const QList<Row> rebuilt = buildRows(notes, m_now);

    // The note brings its head back along when it opens a group again — then
    // two adjacent rows appear instead of one.
    const int last = rowOfNoteIn(rebuilt, target);
    const int added = static_cast<int>(rebuilt.size() - m_rows.size());
    const int first = last - (added - 1);

    beginInsertRows(QModelIndex(), first, last);
    m_notes = notes;
    m_rows = rebuilt;
    endInsertRows();
}

void NoteListModel::replaceNote(int noteIndex, const Note &note)
{
    if (noteIndex < 0 || noteIndex >= m_notes.size()) {
        return;
    }

    m_notes[noteIndex] = note;

    const QModelIndex row = index(rowOfNote(noteIndex));
    Q_EMIT dataChanged(row, row);
}

int NoteListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

QVariant NoteListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_rows.size()) {
        return {};
    }

    const Row &row = m_rows.at(index.row());
    const bool head = row.note < 0;

    switch (role) {
    case Qt::DisplayRole:
        return head ? row.title : m_notes.at(row.note).content;
    case TimestampRole:
        return head ? QString() : library::entryTimestamp(m_notes.at(row.note).createdAt, QLocale());
    case GroupHeaderRole:
        return head;
    case AudioRole: {
        if (head || m_notes.at(row.note).type != Note::Type::Audio) {
            return QString();
        }

        // Without a length the symbol stands alone. "0:41" comes out of the
        // note, and a note that carries none would otherwise read "0:00" — a
        // number the user could believe.
        const Note &note = m_notes.at(row.note);
        return note.audioDurationS
            ? i18nc("@item:inlistbox play symbol and length of a voice note",
                    "▶ %1",
                    library::clockTime(qint64(*note.audioDurationS) * 1000))
            : QStringLiteral("▶");
    }
    default:
        return {};
    }
}

Qt::ItemFlags NoteListModel::flags(const QModelIndex &index) const
{
    // A head is a row of this list, not an item of it: the view walks past it,
    // the mouse cannot pick it, and Entf and F2 never find it (wireframe 3b).
    if (index.isValid() && index.row() < m_rows.size() && m_rows.at(index.row()).note < 0) {
        return Qt::NoItemFlags;
    }

    return QAbstractListModel::flags(index);
}
