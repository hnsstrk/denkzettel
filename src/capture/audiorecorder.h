#pragma once

#include <QAudioBuffer>
#include <QAudioBufferInput>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QElapsedTimer>
#include <QList>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QObject>
#include <QString>

#include <cstdint>
#include <memory>

class QAudioSource;
class QDateTime;

/**
 * The recording of a voice note, from the microphone to the finished file
 * (SPEC 4): Opus in OGG, mono, 48 kHz, under the store's audio directory and
 * named after the note's ISO timestamp.
 *
 * **The samples run through this object, not past it.** Qt offers two ways to
 * feed a QMediaRecorder, and the choice decides more than it looks:
 * `QMediaCaptureSession::setAudioInput()` hands the device straight to the
 * encoder and nothing on the way out carries a sample — QAudioInput has
 * `device`, `volume` and `muted` and no signal at all, and QtMultimedia 6.11
 * has no successor to Qt 5's QAudioProbe. Whoever wants a level meter on that
 * road opens the microphone a second time. This class takes the other one:
 * a QAudioSource of its own reads the device and hands every buffer to a
 * QAudioBufferInput, so the samples are in our hands once, in encode().
 *
 * That is also what makes the pipeline checkable without a microphone: encode()
 * is public, and a check feeds it a signal of its own instead of the user's
 * room.
 */
class AudioRecorder : public QObject
{
    Q_OBJECT

public:
    explicit AudioRecorder(QString audioDirectory, QObject *parent = nullptr);

    /**
     * Ends a recording that is still in flight and **deletes its file** —
     * whether it was running, being stopped or being cancelled.
     *
     * A destroyed object emits nothing, so nobody is left to make a note out
     * of that file; what stayed would be the orphan of SPEC 8.1. It is the
     * path Esc takes when the window closes in the same turn, and the path
     * Ctrl+Enter takes when it does — stop() waits for the tail of the queue,
     * so the muxer is not finished the moment stop() returns.
     */
    ~AudioRecorder() override;

    AudioRecorder(const AudioRecorder &) = delete;
    AudioRecorder &operator=(const AudioRecorder &) = delete;

    /**
     * The name a note created at this moment gives its audio file: its ISO
     * timestamp with the colons replaced by hyphens (SPEC 4).
     */
    static QString fileNameFor(const QDateTime &createdAt);

    /**
     * The upper bound of SPEC 4, 15 minutes by default, in milliseconds. Zero
     * switches it off.
     *
     * **Two clocks, and the first of them to run out ends the recording.** The
     * audio in the file is one of them; the wall clock since start() is the
     * other. SPEC 4 gives the purpose — protection against a forgotten
     * recording — and that is the wall clock: an encoder that takes nothing
     * would let a recording counted in frames alone run for ever. The frame
     * count is the half a check can hold to the millisecond.
     *
     * It is settable so that a check reaches the bound without sitting out a
     * quarter of an hour for it. Set it **before** start(): the wakeup that
     * ends a recording no buffer reaches any more is armed there.
     */
    void setMaximumDuration(qint64 milliseconds);
    qint64 maximumDuration() const;

    /**
     * Starts the encoder and leaves the samples to the caller, who hands them
     * in through encode(). The file is named after `createdAt`, which is the
     * timestamp the note is stored with.
     *
     * This is the half without a microphone, and it is the half a check can
     * measure: it feeds a signal of its own instead of recording the room the
     * user sits in.
     */
    bool startEncoder(const QDateTime &createdAt);

    /**
     * startEncoder(), and on top of it the input device that feeds it. This is
     * what the recording window calls.
     *
     * `device` names the input; a null device takes the session's default
     * input.
     */
    bool start(const QDateTime &createdAt, const QAudioDevice &device = {});

    /**
     * Closes the device and finishes the file; finished() follows.
     *
     * It returns before the file is closed: what the encoder has not taken yet
     * is the end of the recording and goes in first. Keep the recorder alive
     * until the signal — destroying it in between drops the recording.
     */
    void stop();

    /**
     * Like stop(), but the file is deleted afterwards; cancelled() follows.
     *
     * It also overtakes a stop() that has not finished yet: Esc right after
     * Ctrl+Enter discards, and the recording that was on its way into a file
     * ends as a cancelled one. Nothing in SPEC 4 says the window stops taking
     * keys the moment it is asked to save.
     */
    void cancel();

    bool isRecording() const;

    /** Milliseconds of audio the encoder has taken. */
    qint64 duration() const;

    /** The file name, relative to the audio directory, empty before start(). */
    QString fileName() const;

    QString lastError() const;

    /**
     * Hands one buffer to the encoder and counts it towards the duration and
     * the limit.
     *
     * The recording loop calls this with what the device delivered. It is
     * public because the check calls it with a synthetic signal — see the class
     * comment.
     */
    void encode(const QAudioBuffer &buffer);

Q_SIGNALS:
    /** The file is written and closed. `durationSeconds` goes into the note. */
    void finished(const QString &fileName, int durationSeconds);

    /** The recording was discarded and its file is gone. */
    void cancelled();

    /**
     * The recording broke off, and its file is gone with it.
     *
     * A recorder that only wrote the reason into lastError() would leave the
     * window waiting for a finished() that never comes — on Ctrl+Enter and on
     * Esc alike. **Exactly one of these three signals follows every started
     * recording, for as long as the recorder lives**; destroying it before the
     * answer arrives is an abort, and the destructor above says what an abort
     * leaves behind.
     */
    void failed(const QString &message);

private:
    /**
     * What the recorder owes its caller. Idle is also the state a recording
     * that has already been answered for falls back into, and that is what
     * keeps a QMediaRecorder stopped on the error path from announcing a
     * finished recording nobody started.
     */
    enum class State : std::uint8_t {
        Idle,
        Recording,
        Stopping,
        Cancelling,
    };

    /** Hands the encoder everything it will take, then checks the limits. */
    void flush();
    void handleRecorderState(QMediaRecorder::RecorderState state);
    void handleRecorderError(const QString &message);
    QString filePath() const;
    void closeDevice();
    void removeFile();

    QString m_audioDirectory;
    QAudioFormat m_format;
    // Default-constructed on purpose. Measured 2026-08-28 on Qt 6.11.2: given a
    // format at construction, QAudioBufferInput refuses every buffer for the
    // whole run — sendAudioBuffer() returns false, readyToSendAudioBuffer()
    // never fires, and the recorder reports no error and leaves a 127-byte OGG
    // header behind. Without a format it takes the format from the first
    // buffer, which is the one below.
    QAudioBufferInput m_input;
    QMediaCaptureSession m_session;
    QMediaRecorder m_recorder;
    std::unique_ptr<QAudioSource> m_source;
    QList<QAudioBuffer> m_pending;
    QString m_fileName;
    /**
     * Counts the recordings this object has started, and that is what a timer
     * belongs to. QTimer::singleShot cannot be called off: a wakeup armed for
     * one recording arrives whatever has become of it in the meantime, and a
     * state alone does not tell the recordings apart — a second one can stand
     * in the very state the first one was left in.
     */
    qint64 m_generation = 0;
    QElapsedTimer m_elapsed;
    qint64 m_pendingFrames = 0;
    qint64 m_frames = 0;
    qint64 m_maximumDuration = 15LL * 60 * 1000;
    State m_state = State::Idle;
    QString m_lastError;
};
