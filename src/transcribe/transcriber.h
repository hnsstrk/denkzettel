#pragma once

#include <QLatin1StringView>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string_view>

class QTemporaryDir;
class Store;

/**
 * What SPEC 12 lets the user choose, and the defaults it names.
 *
 * Here and not in the settings, because the dependency only runs one way:
 * `denkzettelsettings` links `denkzetteltranscribe`, not the other way round —
 * the same reason `ollama::` lives in analysis/ollamaprovider.h. And because
 * what a size *means* is the file name Transcriber::modelPath() builds from
 * it: whoever adds a size adds it in one place and the naming follows.
 */
namespace whisper
{
/** Where the package `whisper-cpp` puts the program (SPEC 12). */
inline constexpr QLatin1StringView DefaultProgram("/usr/bin/whisper-cli");

/**
 * The other program of the same pipeline, which converts the recording before
 * whisper.cpp ever sees it (SPEC 12, 15).
 *
 * Written down here although no page offers a field for it: it is a setting
 * all the same — `[Transcription] FfmpegProgram` — and the tool detection of
 * SPEC 2.5 has to name the same default the queue reads (issue #17).
 */
inline constexpr QLatin1StringView DefaultFfmpegProgram("/usr/bin/ffmpeg");

/**
 * The five model sizes of SPEC 12, smallest first.
 *
 * The order is stored: the settings page offers them as a combo box, and
 * KConfigDialogManager keeps such a box by its **index**. An entry put in
 * between moves every value after it and changes what a written
 * `denkzettelrc` means, without a sound.
 */
inline constexpr std::array<QLatin1StringView, 5> Sizes{
    QLatin1StringView("tiny"),
    QLatin1StringView("base"),
    QLatin1StringView("small"),
    QLatin1StringView("medium"),
    QLatin1StringView("large-v3"),
};

/** The default of SPEC 12, as an index into Sizes. */
inline constexpr int DefaultSize = 2;
// Through std::string_view because QLatin1StringView::operator==() is not
// constexpr — the point is only that the index and the list cannot drift
// apart when somebody puts a sixth size in.
static_assert(std::string_view(Sizes.at(DefaultSize).data(), Sizes.at(DefaultSize).size())
                  == "small",
              "SPEC 12 names `small` as the default model size");
}

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

    /**
     * `~/.local/share/denkzettel/models/ggml-<size>.bin` (SPEC 12), after the
     * naming scheme of the upstream GGML models.
     *
     * The setting is the **size** and the path follows from it; the settings
     * page asks this for the one question it has per size — is the file there
     * — and the download of issue #23 will write to the same answer.
     */
    static QString modelPath(const QString &size);

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
     * The two program paths of SPEC 12, read from `denkzettelrc` at
     * construction and settable afterwards, for the reason SPEC 12 gives them:
     * the automated run has no graphics card and puts a program of its own in
     * whisper-cli's place.
     *
     * **The model has no setter of its own.** It had one until issue #27 and
     * nobody ever called it; since the size is what is stored and the path
     * follows from it, reloadSettings() is the one road from the configuration
     * into the model, and a second one would only drift from it. The automated
     * run needs none — its stand-in ignores `-m`, and what the model is
     * called is asserted through modelPath().
     */
    void setFfmpegProgram(const QString &program);
    void setWhisperProgram(const QString &program);

    /**
     * The same two, as the configuration currently names them.
     *
     * For the tool detection of SPEC 2.5 (issue #17), which has to ask about
     * the programs this queue would really start. Read from here rather than
     * out of `denkzettelrc` a second time: the group, the two key names and
     * their defaults have exactly one place, and it is reloadSettings().
     */
    QString ffmpegProgram() const;
    QString whisperProgram() const;

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

public Q_SLOTS:
    /**
     * Re-reads the settings of SPEC 12 out of `denkzettelrc`.
     *
     * The daemon holds one transcriber for the whole session, and the settings
     * dialog writes underneath it — without this the program and the model
     * chosen there would only take hold at the next start (SPEC 13, issue
     * #27). A job that is already running keeps what it was started with; what
     * is read here reaches the next one.
     */
    void reloadSettings();

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

/**
 * Turns the `ModelPath` of a version before issue #27 into the `ModelSize`
 * this one reads, once at the start of the daemon.
 *
 * **Before the first read of the group**, and that is not a matter of taste:
 * the Transcriber takes its model in the constructor, and the settings
 * dialog's first Apply writes every item of the skeleton at once — so a
 * fallback further in would be overwritten by the default `small` at the first
 * click, and until then the queue would run against a model the user never
 * chose. Measured 29.08.2026 on the built daemon: with `ModelPath` naming
 * `ggml-medium.bin` it started `whisper-cli -m …/ggml-small.bin`, and nothing
 * said so.
 *
 * A path whose file name is `ggml-<size>.bin` for one of `whisper::Sizes` is
 * that size, and the old key goes. A path that is anything else names a model
 * this program cannot express: the size falls back to the default and
 * **`ModelPath` stays where it is** — a path set by hand is a deliberate act
 * and is not taken away silently. The key is then the whole of the state, and
 * the page "Voice notes" reports it for as long as it stands (customer
 * decision, 29.08.2026).
 */
void migrateModelPath();
