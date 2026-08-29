#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

#include <cstdint>

/**
 * One row of `proposals` together with the notes it stands for (SPEC 5.1).
 *
 * What step 3 of the analysis run produces (SPEC 7.3, 7.4) and what the review
 * of SPEC 9 offers: a bundle of notes that belong to one topic, or one task
 * taken out of a note. Nothing is carried out here — SPEC 7.4 rules out an
 * automatic `task add`, and SPEC 8.1 an export that nobody confirmed.
 *
 * It stands here beside Note for the reason NoteEmbedding does: the store is
 * what hands it out and the analysis is what asks for it, so put in the
 * analysis the store would depend on it the wrong way round.
 */
struct Proposal {
    enum class Kind : std::uint8_t {
        /** Notes of one topic, for the collective note of SPEC 8.1. */
        Bundle,
        /** The task fields of one note, for Taskwarrior (SPEC 8.2). */
        Task,
    };

    /**
     * The two states SPEC 5.1 names, and there is no third.
     *
     * "Accepted" would be one: accepting and discarding both end with the row
     * deleted, and only accepting exports first (SPEC 8.1).
     */
    enum class Status : std::uint8_t {
        Open,
        /** "Later" — the notes go back into the corpus of the next run. */
        Deferred,
    };

    qint64 id = -1;
    Kind kind = Kind::Bundle;
    QDateTime createdAt;
    Status status = Status::Open;

    /**
     * The fields of this suggestion as a JSON object (SPEC 5.1).
     *
     * A bundle carries `title` and `markdown`, a task the fields of SPEC 7.2 —
     * and that text is `notes.task` handed on unchanged, which is what
     * migration 4 writes it as one object for.
     */
    QString payload;

    /** The notes this suggestion stands for, oldest first. */
    QList<qint64> noteIds;
};
