#include "transcribe/transcriber.h"

#include "store/store.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryDir>
#include <QTimer>

// The child of a daemon that is gone has to go too, and Linux has the lever
// for it. No portability guard: this program is a Plasma-on-Wayland tool and
// is built nowhere else.
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

Transcriber::Transcriber(Store *store, QObject *parent)
    : QObject(parent)
    , m_store(store)
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    m_ffmpegProgram = group.readEntry("FfmpegProgram", QStringLiteral("/usr/bin/ffmpeg"));
    m_whisperProgram = group.readEntry("WhisperProgram", QStringLiteral("/usr/bin/whisper-cli"));
    m_modelPath = group.readEntry("ModelPath", defaultModelPath());

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
            fail(QStringLiteral("%1 was killed").arg(program));
            return;
        }
        // Three different things, and none of them stands for the others: the
        // return code, what came back on stderr, and whether there is a JSON
        // file with text in it. The code is read here, the JSON in
        // collectTranscript() — stderr only ever furnishes the reason.
        if (exitCode != 0) {
            const QString said = lastLineOf(m_process.readAllStandardError());
            fail(said.isEmpty() ? QStringLiteral("%1 ended with code %2").arg(program).arg(exitCode)
                                : QStringLiteral("%1 ended with code %2: %3").arg(program).arg(exitCode).arg(said));
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
            fail(QStringLiteral("%1 could not be started")
                     .arg(m_step == Step::Converting ? m_ffmpegProgram : m_whisperProgram));
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

QString Transcriber::defaultModelPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/models/ggml-small.bin");
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

void Transcriber::setModelPath(const QString &path)
{
    m_modelPath = path;
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
    m_step = Step::Converting;

    const std::optional<Note> note = m_store->note(m_noteId);
    if (!note.has_value() || note->audioPath.isEmpty()) {
        fail(QStringLiteral("The note carries no audio file"));
        return;
    }
    const QString audioFile = m_store->audioDirectory() + QLatin1Char('/') + note->audioPath;
    if (!QFile::exists(audioFile)) {
        fail(QStringLiteral("The audio file %1 is missing").arg(note->audioPath));
        return;
    }

    m_work = std::make_unique<QTemporaryDir>(workingRoot() + QLatin1Char('/') + workPrefix()
                                             + QStringLiteral("XXXXXX"));
    if (!m_work->isValid()) {
        fail(QStringLiteral("No working directory: %1").arg(m_work->errorString()));
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
        fail(QStringLiteral("%1 wrote no transcript").arg(m_whisperProgram));
        return;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(json.readAll(), &error);
    if (document.isNull()) {
        fail(QStringLiteral("The transcript is no JSON: %1").arg(error.errorString()));
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
        fail(QStringLiteral("%1 returned no text").arg(m_whisperProgram));
        return;
    }

    if (!m_store->completeTranscription(m_noteId, transcript)) {
        fail(m_store->lastError());
        return;
    }

    Q_EMIT transcribed(m_noteId);
    endJob();
}

void Transcriber::fail(const QString &reason)
{
    qWarning("Transcribing note %lld failed: %s", m_noteId, qUtf8Printable(reason));
    if (!m_store->failTranscribeJob(m_noteId, reason)) {
        qWarning("Noting the failure failed: %s", qUtf8Printable(m_store->lastError()));
    }
    Q_EMIT failed(m_noteId, reason);
    endJob();
}

void Transcriber::endJob()
{
    m_step = Step::Idle;
    m_noteId = -1;
    // Success, failure and a job that never got as far as its programs all end
    // here, and that is what makes the temporary WAV go on every road.
    m_work.reset();
    QTimer::singleShot(0, this, &Transcriber::takeNextJob);
}
