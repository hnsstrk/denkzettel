#pragma once

#include <QDateTime>
#include <QString>

#include <cstdint>
#include <optional>

/**
 * A single note as stored in the `notes` table (SPEC 5.1).
 *
 * Empty strings stand for SQL NULL: an empty category means the note has not
 * been analysed yet, an empty audioPath means there is no audio file.
 */
struct Note {
    enum class Type : std::uint8_t {
        Text,
        Audio,
    };

    enum class State : std::uint8_t {
        New,
        Transcribed,
        Analysed,
    };

    qint64 id = -1;
    QDateTime createdAt;
    Type type = Type::Text;
    QString content;
    QString audioPath; //< relative to the store's audio directory
    std::optional<int> audioDurationS;
    QString category;
    State state = State::New;
    bool needsReembed = false;
    int analysisAttempts = 0;
    QString analysisLastError;
};

/**
 * The stem a note's own files are named after: its ISO timestamp with the
 * colons of the hour replaced by hyphens.
 *
 * The form the store writes into `created_at` (SPEC 5.1), so the name of a
 * file is the timestamp of the note and not a second rendering of it. The
 * milliseconds come along and settle the only collision two notes of the same
 * second could have.
 *
 * With one departure, and SPEC 4 carries it (user decision 2026-08-28, issue
 * #20): the colons become hyphens. FAT and exFAT forbid `:` in a name, and the
 * full export of SPEC 8.3 writes note and audio file onto whatever the user
 * points it at — a USB stick is the case it exists for. Nobody reading the
 * name notices the difference; a copy that fails on a stick is noticed at once.
 *
 * It sits here rather than beside either caller because both of them name the
 * same file: the recording writes `<stem>.ogg`, the export writes `<stem>.md`
 * next to a copy of it, and the two would drift apart written twice.
 */
inline QString noteFileStem(const QDateTime &createdAt)
{
    QString stem = createdAt.toString(Qt::ISODateWithMs);
    stem.replace(QLatin1Char(':'), QLatin1Char('-'));
    return stem;
}
