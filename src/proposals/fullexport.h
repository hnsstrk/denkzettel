#pragma once

#include <QDate>
#include <QString>
#include <QStringList>

class Store;

/**
 * What one run of exportAllNotes() left behind.
 *
 * `noteCount` and `audioCount` are **read back off the disk** rather than
 * counted by the loop that writes: "the export folder is complete" is the
 * acceptance criterion of issue #36, and a counter raised beside every write
 * says only how often the loop went round. The number the user reads has to
 * come from the folder it talks about.
 */
struct FullExportResult {
    /** The created folder; empty when nothing was created. */
    QString directory;
    /** `.md` files standing in `directory` afterwards. */
    int noteCount = 0;
    /** Files standing in `directory/audio` afterwards. */
    int audioCount = 0;
    /**
     * One line per note that is **not** in the folder at all.
     *
     * Told apart from `incomplete` because a rescue path is read for what to do
     * next, and the two ask for different things: a note that is missing has to
     * be fetched out of the database another way, a note without its recording
     * is there and readable. "Incomplete" for both would name the milder of the
     * two for the worse case.
     */
    QStringList missing;
    /** One line per note that stands in the folder without its recording. */
    QStringList incomplete;
    /** Why nothing was exported; empty when the run went through. */
    QString error;

    bool ok() const
    {
        return error.isEmpty();
    }
};

/**
 * Writes every note of `store` into `parentDirectory/denkzettel-export-<date>/`
 * — one `.md` per note plus an `audio/` subfolder with the original files
 * (SPEC 8.3).
 *
 * **Purely reading.** It asks the store for notes(), tags() and
 * audioDirectory() and nothing else, and it copies the audio files rather than
 * moving them; the corpus comes out of the run as it went in.
 *
 * `date` names the folder. It is passed in so a check can hold the name
 * against a literal instead of against the same clock the code read.
 */
FullExportResult exportAllNotes(const Store &store,
                                const QString &parentDirectory,
                                const QDate &date = QDate::currentDate());
