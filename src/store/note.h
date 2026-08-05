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
