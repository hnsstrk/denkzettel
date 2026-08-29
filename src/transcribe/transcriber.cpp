#include "transcribe/transcriber.h"

#include "store/store.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryDir>
#include <QTimer>

// The child of a daemon that is gone has to go too, and Linux has the lever
// for it. No portability guard: this program is a Plasma-on-Wayland tool and
// is built nowhere else.
#include <algorithm>

#include <csignal>
#include <sys/prctl.h>
#include <unistd.h>

namespace
{
/**
 * The name of the working directory of a job, and it is load-bearing twice: it
 * says in the runtime directory what the directory belongs to, and
 * sweepAbandonedWork() finds the leftovers of a killed daemon by exactly this
 * pattern.
 */
QString workPrefix()
{
    return QStringLiteral("denkzettel-transcribe-");
}

/** What the child process said last, for the error the job records. */
QString lastLineOf(const QByteArray &output)
{
    constexpr qsizetype limit = 200;
    const QStringList lines = QString::fromUtf8(output).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return {};
    }
    return lines.constLast().trimmed().left(limit);
}
}

QString reasonWithoutDirectories(QString reason)
{
    static const QRegularExpression directories(QStringLiteral("(?:^|(?<=\\s))/\\S*/"));
    return reason.remove(directories);
}

void migrateModelPath()
{
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    const QString path = group.readEntry("ModelPath", QString());
    if (path.isEmpty()) {
        return;
    }
    // Only the file name, and only the naming Transcriber::modelPath() builds:
    // where the file lies is this program's business, which size it is was the
    // user's. A path under a directory of their own that carries the ordinary
    // name is therefore the same model and migrates like any other.
    const QString name = QFileInfo(path).fileName();
    for (const QLatin1StringView size : whisper::Sizes) {
        if (name == QLatin1String("ggml-") + size + QLatin1String(".bin")) {
            group.writeEntry("ModelSize", QString(size));
            group.deleteEntry("ModelPath");
            group.sync();
            return;
        }
    }
    // No size of ours. Nothing is written and nothing is deleted — the size
    // stays at its default because the key is absent, and `ModelPath` stands
    // on as the record of what was set. See the header for why.
}

Transcriber::Transcriber(Store *store, QObject *parent)
    : QObject(parent)
    , m_store(store)
{
    reloadSettings();

    // Every note that reaches the database runs through Store::noteAdded, and
    // that is why the queue is filled from here rather than from whoever makes
    // the recording: the capture window and the D-Bus entry point put their
    // audio notes in on the same road.
    connect(m_store, &Store::noteAdded, this, [this](qint64 noteId) {
        const std::optional<Note> note = m_store->note(noteId);
        // A transcript that is already there is not fetched a second time; the
        // full import of SPEC 8.3 brings notes that carry both.
        if (!note.has_value() || note->type != Note::Type::Audio || !note->content.isEmpty()) {
            return;
        }
        if (!m_store->enqueueTranscription(noteId)) {
            qWarning("Queueing note %lld for transcription failed: %s",
                     noteId,
                     qUtf8Printable(m_store->lastError()));
            return;
        }
        takeNextJob();
    });

    // The limit of SPEC 12, and it is on the whole job: it starts where the job
    // is taken out and stops in endJob(), which every road out runs through.
    // What it must not be hung on is a stretch without output — Transcriber
    // reads stderr only in the failure path, and whether whisper-cli says
    // anything at all while it recognises is unmeasured (issue #113).
    m_deadline.setSingleShot(true);
    connect(&m_deadline, &QTimer::timeout, this, &Transcriber::giveUp);

    // **The destructor is not what ends a run.** Qt handles no SIGTERM, so a
    // logout or a shutdown takes the daemon away without running a line of
    // ours — and whisper-cli kept running on the graphics card afterwards,
    // with no window and no service left that it belongs to (measured
    // 2026-08-29). The kernel is what notices the parent going, whatever the
    // parent was killed with: PR_SET_PDEATHSIG survives the exec below and
    // needs no signal handler of our own.
    //
    // The second line closes the one gap of the first: between fork and here
    // the parent may already be gone, and the signal would then never be sent
    // again. Everything in this block runs in the forked child before exec, so
    // only async-signal-safe calls belong in it — these three are.
    m_process.setChildProcessModifier([parent = ::getpid()] {
        ::prctl(PR_SET_PDEATHSIG, SIGKILL);
        if (::getppid() != parent) {
            ::_exit(EXIT_FAILURE);
        }
    });

    connect(&m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        // Idle is the process the destructor killed; it owes nobody an answer.
        if (m_step == Step::Idle) {
            return;
        }
        const QString program = m_step == Step::Converting ? m_ffmpegProgram : m_whisperProgram;
        if (status != QProcess::NormalExit) {
            fail(i18n("%1 was killed", program));
            return;
        }
        // Three different things, and none of them stands for the others: the
        // return code, what came back on stderr, and whether there is a JSON
        // file with text in it. The code is read here, the JSON in
        // collectTranscript() — stderr only ever furnishes the reason.
        if (exitCode != 0) {
            // What the program said last goes to the log and not into the
            // tooltip. Measured 2026-08-29: in three failure cases of ffmpeg
            // and one of whisper-cli the last line carries no path — ffmpeg
            // names the audio file one line ABOVE it ("Error opening input
            // file /home/…"), whisper-cli names its backend libraries and the
            // graphics card there. The branch is clean today by the ordering
            // of somebody else's messages, and which line is the last is not
            // ours to decide. So the reason names the program and its code,
            // like every other failure path here (SPEC 14: tooltip quiet, log
            // detailed).
            const QString said = lastLineOf(m_process.readAllStandardError());
            if (!said.isEmpty()) {
                qWarning("%s said: %s", qUtf8Printable(program), qUtf8Printable(said));
            }
            fail(i18n("%1 ended with code %2", program, exitCode));
            return;
        }
        if (m_step == Step::Converting) {
            // Through the event loop and not straight on: starting the next
            // process inside the finished handler of the last one re-enters
            // QProcess, and a queue of failing jobs would recurse instead of
            // looping.
            QTimer::singleShot(0, this, &Transcriber::transcribe);
            return;
        }
        collectTranscript();
    });

    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        // Only this one, and it is the one that has no finished() behind it: a
        // program that never started emits no exit code. A crash arrives above
        // as a return status.
        if (error == QProcess::FailedToStart && m_step != Step::Idle) {
            fail(i18n("%1 could not be started",
                      m_step == Step::Converting ? m_ffmpegProgram : m_whisperProgram));
        }
    });
}

Transcriber::~Transcriber()
{
    // The step first: the kill below travels back through finished(), and a
    // job that is being taken apart must not be failed and counted a second
    // time on the way out.
    m_step = Step::Idle;
    if (m_process.state() != QProcess::NotRunning) {
        m_process.kill();
        m_process.waitForFinished(1000);
    }
    // And the WAV goes with it. QTemporaryDir deletes what it holds; on this
    // road that is the whole point of it being one (SPEC 12, acceptance
    // criterion of issue #22).
    m_work.reset();
}

QString Transcriber::modelPath(const QString &size)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/models/ggml-") + size + QStringLiteral(".bin");
}

void Transcriber::reloadSettings()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    m_ffmpegProgram = group.readEntry("FfmpegProgram", QString(whisper::DefaultFfmpegProgram));
    m_whisperProgram = group.readEntry("WhisperProgram", QString(whisper::DefaultProgram));
    // The setting is the **size** and not a path (SPEC 12). Up to issue #27 a
    // full `ModelPath` stood here; a size cannot be offered as a list of five
    // while the file holds a file name, and the download of issue #23 needs
    // the same mapping to know what it is fetching. What a `ModelPath` of that
    // time becomes stands in migrateModelPath().
    //
    // Held against the list rather than pasted into a file name: this is where
    // a hand-written denkzettelrc enters, and a typo would otherwise become a
    // path that leads nowhere — two failed attempts and the tray in its error
    // state, for a value the dialog could never have produced.
    const QString size = group.readEntry("ModelSize", QString());
    const bool known = std::find(whisper::Sizes.begin(), whisper::Sizes.end(), size)
        != whisper::Sizes.end();
    m_modelSize = known ? size : QString(whisper::Sizes.at(whisper::DefaultSize));
    m_modelPath = modelPath(m_modelSize);
}

QString Transcriber::workingRoot()
{
    // Measured 2026-08-29: QStandardPaths::setTestModeEnabled() does **not**
    // redirect this one — it stays /run/user/<uid> for a test run too. So this
    // separates users and sessions and nothing else; a check that must not
    // reach the daemon of the session it runs in gives itself an
    // XDG_RUNTIME_DIR of its own, as transcribetest does.
    const QString runtime = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    return runtime.isEmpty() ? QDir::tempPath() : runtime;
}

void Transcriber::setFfmpegProgram(const QString &program)
{
    m_ffmpegProgram = program;
}

void Transcriber::setWhisperProgram(const QString &program)
{
    m_whisperProgram = program;
}

QString Transcriber::ffmpegProgram() const
{
    return m_ffmpegProgram;
}

QString Transcriber::whisperProgram() const
{
    return m_whisperProgram;
}


void Transcriber::setTimeout(std::chrono::milliseconds timeout)
{
    // Takes hold with the next job. A deadline that is already running is not
    // moved: the limit a job was taken out under is the one it is measured by.
    m_timeout = timeout;
}

bool Transcriber::isBusy() const
{
    return m_step != Step::Idle;
}

void Transcriber::start()
{
    if (m_step != Step::Idle) {
        return;
    }
    sweepAbandonedWork();

    // A run this daemon never came back from: takeTranscribeJob() counts the
    // attempt before the run, the answer never comes, and the row is left with
    // an empty reason. Here and not in the destructor — SIGTERM never reaches
    // that one (see the comment on setChildProcessModifier above), and here
    // the reading is unambiguous: the service is single-instance (SPEC 2.3)
    // and no job of ours is running, so an empty reason on a counted attempt
    // can only belong to a run that was cut off.
    if (!m_store->noteInterruptedTranscribeJobs(i18n("The run was interrupted"))) {
        qWarning("Noting the interrupted runs failed: %s", qUtf8Printable(m_store->lastError()));
    }

    // And what the database already holds as given up on reaches the tray from
    // here: after a restart the error state has to stand where the queue
    // stands, not at "no trouble so far" (issue #24).
    const std::optional<TranscribeJob> givenUp = m_store->pausedTranscribeJob();
    if (givenUp.has_value()) {
        Q_EMIT paused(givenUp->noteId, givenUp->lastError);
    }

    takeNextJob();
}

void Transcriber::sweepAbandonedWork()
{
    // A daemon that is killed runs no destructor, so its working directory
    // stays behind with the WAV in it. What may be swept is bounded by where
    // this looks: one user, one session — see workingRoot(). Inside that the
    // daemon is single-instance (SPEC 2.3), and this runs before a job of ours
    // is running.
    const QDir temporary(workingRoot());
    const QStringList left = temporary.entryList({workPrefix() + QStringLiteral("*")}, QDir::Dirs);
    for (const QString &name : left) {
        if (!QDir(temporary.filePath(name)).removeRecursively()) {
            qWarning("Deleting the abandoned working directory %s failed", qUtf8Printable(name));
        }
    }
}

void Transcriber::takeNextJob()
{
    if (m_step != Step::Idle) {
        return;
    }

    // The attempt is counted here, in the database — see
    // Store::takeTranscribeJob() for why that is what a crash needs.
    const std::optional<TranscribeJob> job = m_store->takeTranscribeJob();
    if (!job.has_value()) {
        return;
    }
    m_noteId = job->noteId;
    // Counted by takeTranscribeJob(), this run included — it is what fail()
    // below reads to know whether the queue comes back to this note.
    m_attempts = job->attempts;
    m_step = Step::Converting;
    // Here and not at the first process: what the limit covers is the job, so
    // reading the note and making the working directory below lie inside it.
    m_deadline.start(m_timeout);

    const std::optional<Note> note = m_store->note(m_noteId);
    if (!note.has_value() || note->audioPath.isEmpty()) {
        fail(i18n("The note carries no audio file"));
        return;
    }
    const QString audioFile = m_store->audioDirectory() + QLatin1Char('/') + note->audioPath;
    if (!QFile::exists(audioFile)) {
        fail(i18n("The audio file %1 is missing", note->audioPath));
        return;
    }

    m_work = std::make_unique<QTemporaryDir>(workingRoot() + QLatin1Char('/') + workPrefix()
                                             + QStringLiteral("XXXXXX"));
    if (!m_work->isValid()) {
        // The reason reaches the user as a tooltip now (issue #24), and
        // QTemporaryDir's own wording names the path it tried — the runtime
        // directory of this login. That belongs in the log and not on the
        // panel (SPEC 14: tooltip quiet, log detailed).
        qWarning("No working directory under %s: %s",
                 qUtf8Printable(workingRoot()),
                 qUtf8Printable(m_work->errorString()));
        fail(i18n("No working directory could be created"));
        return;
    }

    convert(audioFile);
}

void Transcriber::convert(const QString &audioFile)
{
    // 16 kHz mono WAV, the one format whisper.cpp needs no library of somebody
    // else's build to read (SPEC 12). `-nostdin` because this runs in a daemon:
    // ffmpeg reads stdin for its own key commands and would take the daemon's.
    m_process.start(m_ffmpegProgram,
                    {QStringLiteral("-nostdin"),
                     QStringLiteral("-y"),
                     QStringLiteral("-loglevel"),
                     QStringLiteral("error"),
                     QStringLiteral("-i"),
                     audioFile,
                     QStringLiteral("-ar"),
                     QStringLiteral("16000"),
                     QStringLiteral("-ac"),
                     QStringLiteral("1"),
                     QStringLiteral("-c:a"),
                     QStringLiteral("pcm_s16le"),
                     m_work->filePath(QStringLiteral("audio.wav"))});
}

void Transcriber::transcribe()
{
    if (m_step != Step::Converting) {
        return;
    }

    // The queue never fetches a model itself — the download is a hand on the
    // settings page and the progress belongs where somebody is looking (UX
    // decision, 29.08.2026). What it owes the user is the way there: without
    // this line the missing model arrives as "whisper-cli wrote no transcript"
    // twice over and the tray goes into its error state with nothing to do
    // about it (issue #23).
    if (!QFileInfo::exists(m_modelPath)) {
        fail(i18n("Model %1 is missing. Download it under Settings \u2192 Voice notes.", m_modelSize));
        return;
    }

    m_step = Step::Transcribing;
    // `-of` without an extension: whisper-cli appends `.json` to it. Left out,
    // it would write the file beside the input — which is our directory too,
    // but only by accident.
    m_process.start(m_whisperProgram,
                    {QStringLiteral("-m"),
                     m_modelPath,
                     QStringLiteral("-f"),
                     m_work->filePath(QStringLiteral("audio.wav")),
                     QStringLiteral("-l"),
                     QStringLiteral("de"),
                     QStringLiteral("-oj"),
                     QStringLiteral("-of"),
                     m_work->filePath(QStringLiteral("transcript")),
                     QStringLiteral("-np")});
}

void Transcriber::collectTranscript()
{
    QFile json(m_work->filePath(QStringLiteral("transcript.json")));
    if (!json.open(QIODevice::ReadOnly)) {
        // Measured 2026-08-28: whisper-cli writes no JSON at all when it fails
        // to load its model — and a return code of 0 with no file is exactly
        // the case a check on the return code alone would call a success.
        fail(i18n("%1 wrote no transcript", m_whisperProgram));
        return;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(json.readAll(), &error);
    if (document.isNull()) {
        fail(i18n("The transcript is no JSON: %1", error.errorString()));
        return;
    }

    const QJsonArray segments = document.object().value(QLatin1String("transcription")).toArray();
    QString transcript;
    for (const auto &segment : segments) {
        transcript += segment.toObject().value(QLatin1String("text")).toString();
    }
    // Every segment comes with a leading blank and its own line breaks;
    // simplified() makes one text of them, which is what the note holds.
    transcript = transcript.simplified();
    if (transcript.isEmpty()) {
        // No text is no transcript. The note keeps its audio and stays
        // playable — better than a note that says 'transkribiert' and is empty.
        fail(i18n("%1 returned no text", m_whisperProgram));
        return;
    }

    if (!m_store->completeTranscription(m_noteId, transcript)) {
        // Same ground as at the working directory above: what SQLite says
        // names the database file, and that lies in the user's home directory.
        qWarning("Saving the transcript failed: %s", qUtf8Printable(m_store->lastError()));
        fail(i18n("The transcript could not be saved"));
        return;
    }

    Q_EMIT transcribed(m_noteId);
    endJob();
}

void Transcriber::giveUp()
{
    const QString program = m_step == Step::Converting ? m_ffmpegProgram : m_whisperProgram;
    // Idle over the kill and back again afterwards: killing travels back
    // through finished(), which would fail and count this job a second time.
    // Idle it must not stay — until endJob() the job is still running, and
    // isBusy() has to say so.
    const Step step = m_step;
    m_step = Step::Idle;
    if (m_process.state() != QProcess::NotRunning) {
        // PR_SET_PDEATHSIG covers the daemon dying; a job the daemon itself
        // gives up on it never reaches, so the child is killed here. kill() and
        // not terminate(): a whisper-cli busy on the graphics card is what this
        // is for, and the WAV under it goes away with endJob() either way.
        m_process.kill();
        m_process.waitForFinished(1000);
    }
    m_step = step;
    fail(i18n("%1 did not finish within %2 seconds",
              program,
              std::chrono::duration<double>(m_timeout).count()));
}

void Transcriber::fail(const QString &reason)
{
    qWarning("Transcribing note %lld failed: %s", m_noteId, qUtf8Printable(reason));
    if (!m_store->failTranscribeJob(m_noteId, reason)) {
        qWarning("Noting the failure failed: %s", qUtf8Printable(m_store->lastError()));
    }
    Q_EMIT failed(m_noteId, reason);
    // The end of the road, and only it reaches the tray: with an attempt left
    // the queue comes back to this note, and an error state raised now would
    // clear itself again a moment later (SPEC 12, issue #24).
    if (m_attempts >= Store::transcribeAttemptLimit) {
        Q_EMIT paused(m_noteId, reason);
    }
    endJob();
}

void Transcriber::endJob()
{
    m_deadline.stop();
    m_step = Step::Idle;
    m_noteId = -1;
    m_attempts = 0;
    // Success, failure and a job that never got as far as its programs all end
    // here, and that is what makes the temporary WAV go on every road.
    m_work.reset();
    QTimer::singleShot(0, this, &Transcriber::takeNextJob);
}
