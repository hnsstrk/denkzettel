#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

#include <chrono>
#include <cstdint>
#include <memory>

class QTemporaryDir;
class Store;

/**
 * The transcription queue and the two programs it runs (SPEC 12).
 *
 * One job at a time — there is one graphics card, and two whisper.cpp runs
 * beside each other would share it. The queue itself is the table
 * `transcribe_jobs` and nothing is held in memory: whatever a crash interrupts
 * is in the database when the next start reads it, and start() picks it up.
 *
 * Each job runs `ffmpeg` over the recording into a temporary 16 kHz mono WAV
 * and hands that to `whisper-cli`; the JSON it writes becomes the note's
 * content and its state 'transkribiert'. The conversion stays even though the
 * packaged whisper-cli reads Opus by itself — it does so only because the
 * packager linked it against libavformat, and SPEC 12 says why that must not
 * be depended on.
 *
 * **Nothing here reports to the user directly.** A routine run is silent (SPEC
 * 14: no notification for routine runs); a job that has used up its attempts
 * announces itself through paused(), which the tray turns into its error state
 * (SPEC 10, issue #24). There is no "running" state and none is wanted — see
 * the schema comment in store.cpp for why nobody could write one truthfully.
 */
class Transcriber : public QObject
{
    Q_OBJECT

public:
    /** `store` outlives the transcriber and is not owned by it. */
    explicit Transcriber(Store *store, QObject *parent = nullptr);
    ~Transcriber() override;

    Transcriber(const Transcriber &) = delete;
    Transcriber &operator=(const Transcriber &) = delete;

    /** `~/.local/share/denkzettel/models/ggml-small.bin` (SPEC 12). */
    static QString defaultModelPath();

    /**
     * Where the temporary WAV of a job lies, and where start() sweeps up what
     * a killed run left behind.
     *
     * The runtime directory and not `/tmp`: it belongs to one user and one
     * session (`/run/user/<uid>`), so the sweep cannot reach the files of
     * another user or another login. `/tmp` is shared by everybody on the
     * machine, and a sweep there took a directory away from a run beside it
     * (measured 2026-08-29).
     *
     * A container without `XDG_RUNTIME_DIR` has none — there `/tmp` has to do,
     * and that is the automated run, which shares the machine with nobody.
     */
    static QString workingRoot();

    /**
     * The three settings of SPEC 12, read from `denkzettelrc` at construction
     * and settable afterwards.
     *
     * The program paths are settable for the reason SPEC 12 gives them: the
     * automated run has no graphics card and no model, and puts a program of
     * its own in that place.
     */
    void setFfmpegProgram(const QString &program);
    void setWhisperProgram(const QString &program);
    void setModelPath(const QString &path);

    /**
     * How long a whole job may take before it is given up on (SPEC 12).
     *
     * The default is the five minutes the customer chose on 29.08.2026, and it
     * covers the run and not a stretch of it without output: whether
     * `whisper-cli` writes anything at all while it recognises is unmeasured,
     * and a limit on something unmeasured would be a guess (issue #113).
     *
     * Settable for the same reason the program paths above are: a check that
     * waits five minutes for the limit to bite is one nobody runs.
     */
    void setTimeout(std::chrono::milliseconds timeout);

    /**
     * Takes up the queue and returns at once — the work runs in the event loop.
     *
     * Calling it while a job is running does nothing: the next job is taken
     * when the running one is answered for.
     */
    void start();

    /** Whether a job is being worked on right now. */
    bool isBusy() const;

Q_SIGNALS:
    /** The transcript is written and the job is out of the queue. */
    void transcribed(qint64 noteId);

    /**
     * The attempt failed; the note keeps its audio and no transcript, and the
     * job row keeps the reason (SPEC 12).
     *
     * One per attempt, the ones that are retried included — whoever wants the
     * end of the road wants paused() below.
     */
    void failed(qint64 noteId, const QString &reason);

    /**
     * The job is given up on: the attempts of SPEC 12 are used up, and this
     * note will not be handed out again.
     *
     * That is what the tray shows as its error state (SPEC 10, issue #24), and
     * why it is a signal of its own beside failed(): a first attempt that
     * failed is followed by a second one, and an error state that came up for
     * it would go away by itself a moment later.
     *
     * start() emits it too, for a job that was already given up on before this
     * process began — after a restart the tray stands where the database says,
     * not at "no trouble so far".
     */
    void paused(qint64 noteId, const QString &reason);

private:
    /** Which of the two programs the running process is. */
    enum class Step : std::uint8_t {
        Idle,
        Converting,
        Transcribing,
    };

    /** Removes what a killed run of an earlier daemon left under /tmp. */
    void sweepAbandonedWork();
    void takeNextJob();
    void convert(const QString &audioFile);
    void transcribe();
    /** Reads the JSON whisper-cli wrote and finishes the job with it. */
    void collectTranscript();
    /** Kills the run that has outlasted m_timeout and fails its job. */
    void giveUp();
    void fail(const QString &reason);
    /** Ends the job, drops the temporary files and goes on to the next one. */
    void endJob();

    Store *m_store;
    QProcess m_process;
    /** Runs from the job being taken out until endJob(), and only then. */
    QTimer m_deadline;
    std::chrono::milliseconds m_timeout = std::chrono::minutes(5);
    /** Holds the WAV and the JSON of the running job, and only of that one. */
    std::unique_ptr<QTemporaryDir> m_work;
    QString m_ffmpegProgram;
    QString m_whisperProgram;
    QString m_modelPath;
    qint64 m_noteId = -1;
    /** How often the running job has been handed out, its own run counted. */
    int m_attempts = 0;
    Step m_step = Step::Idle;
};

/**
 * The reason a job failed with, as it may leave the machine (SPEC 10, issue
 * #115).
 *
 * The job records the program with its full path, and the log and the tray
 * tooltip show it that way — a notification is a different matter, because it
 * can be forwarded, and the program path is a setting (SPEC 12) that may lie
 * in the user's home. The name of the program carries the whole statement; the
 * directory says only where this machine keeps it.
 *
 * ponytail: a filter over the finished sentence, not a reason built from parts.
 * It takes the directories off a run without blanks that begins with `/`, so
 * "/home/<name>/tools/whisper-cli ended with code 2" comes out as "whisper-cli
 * ended with code 2". **The ceiling is the blank**: a program path with one in
 * it keeps what stands after the last blank ("…/my tools/whisper-cli ended
 * with code 2" → "my tools/whisper-cli ended with code 2"), and a relative
 * path stays whole, because it names no home. Neither is the ordinary case —
 * the programs of SPEC 12 are packaged ones under `/usr/bin` — and both leave
 * the statement readable. Where that stops being enough, the way up is to
 * carry the program name beside the reason instead of parsing the sentence
 * apart again.
 */
QString reasonWithoutDirectories(QString reason);
