#pragma once

#include <QDateTime>
#include <QString>

#include <optional>

/**
 * A single note as stored in the `notes` table (SPEC 5.1).
 *
 * Empty strings stand for SQL NULL: an empty category means the note has not
 * been analysed yet, an empty audioPath means there is no audio file.
 */
struct Note {
    enum class Type {
        Text,
        Audio,
    };

    enum class State {
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
