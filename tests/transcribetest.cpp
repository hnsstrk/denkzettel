#include "capture/audiorecorder.h"
#include "store/store.h"
#include "transcribe/modeldownload.h"
#include "transcribe/transcriber.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QAudioBuffer>
#include <QAudioFormat>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>

#include <chrono>
#include <cmath>
#include <memory>
#include <numbers>

/**
 * The transcription queue (SPEC 12, issue #22).
 *
 * **whisper.cpp is not run here and no model is loaded.** Its place is taken by
 * a program of its own, which SPEC 12 asks the path to be configurable for: the
 * automated run has no graphics card, and 466 MiB of model weights per run are
 * not what a build server should download. What is checked here is what breaks
 * without anybody seeing it — the schema, the queue over a restart, the count
 * of attempts, the encoding of the transcript and the temporary WAV. That
 * whisper.cpp really transcribes German is the acceptance criterion on the
 * development machine and is proven by a run there, not by this file.
 *
 * `ffmpeg` on the other hand is the real one. It is the step SPEC 12 keeps
 * explicitly, and the WAV it produces is measured here out of the file the
 * stand-in was handed: 16 kHz, one channel, held against numbers this file did
 * not compute.
 */
class TranscribeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void transcribesAnAudioNoteItWasHandedByTheStore();
    void takesUpAJobLeftBehindByAKilledRun();
    void pausesAfterTwoAttemptsAndKeepsTheJobLine();
    void reportsThePauseOnlyWhenTheAttemptsAreUsedUp();
    void reportsAJobThatWasGivenUpOnAtTheNextStart();
    void namesAReasonForARunThatWasNeverAnsweredFor();
    void givesUpOnARunThatHangs();
    void namesTheProgramWhenItIsNotInstalledAtAll();
    void stripsTheDirectoriesOffAReasonBeforeItIsForwarded();
    void clearsTheWorkingDirectoryOfAKilledRun();
    void deletingANoteTakesItsJobWithIt();
    void aChangedSettingReachesTheRunningQueue();
    void takesOverAModelPathOfTheVersionBefore();
    void fallsBackToTheDefaultForASizeItDoesNotKnow();
    void namesTheSettingsPageWhenTheModelIsMissing();
    void fetchesAModelAndWritesItWhenTheChecksumAgrees();
    void leavesNothingBehindWhenTheDownloadIsCancelled();
    void leavesNothingBehindWhenTheConnectionBreaks();
    void leavesNothingBehindWhenWhatArrivesIsNotTheModel();

private:
    QString databasePath() const;
    /**
     * A voice note in the store: a real Opus recording under the name of its
     * timestamp, the note without a transcript.
     *
     * Never the microphone (the user's instruction of 28.08.2026). The
     * recording is a tone this file computes, and it goes through
     * AudioRecorder::startEncoder(), which opens no device — the same road
     * capturetest takes, and the file is the one a user's recording is.
     */
    qint64 addVoiceNote();
    /**
     * Writes the stand-in for whisper-cli and returns its path: it copies the
     * WAV it was handed to `keptWav()` and writes the JSON that whisper-cli
     * would write.
     */
    QString writeWhisperStub();
    /**
     * Writes a stand-in that names itself and the model it was handed, and
     * returns its path.
     *
     * The transcript it writes is `<name> <the -m argument>`, so what the note
     * holds afterwards says **which** program ran and **which** model the
     * settings had reached it with — the two halves of issue #27's first
     * acceptance criterion, read off one string.
     */
    QString writeMarkingStub(const QString &name);
    /** Runs one voice note through the queue and answers with its transcript. */
    QString transcriptOfOneNote(Transcriber &transcriber);
    /**
     * Writes a stand-in for whisper-cli that never comes back, and returns its
     * path: it notes its own process id and then hangs.
     */
    QString writeHangingStub();
    QString hangingPidFile() const;
    /**
     * The process ids the hanging stand-in has noted, oldest first.
     *
     * The ids come from the children themselves. `pgrep -f` on the path of the
     * stand-in would find the test's own process too, because its command line
     * carries that path (finding 30 of CLAUDE.md).
     */
    QList<qint64> hangingChildren() const;
    QString keptWav() const;
    /** Working directories of a job — none of them may survive its job. */
    static QStringList leftOverWorkingDirectories();
    /** Where Transcriber::modelPath() puts the models of this run. */
    static QDir modelDirectory();
    /**
     * Everything the models directory holds for `size`, the model itself and
     * anything beside it — an empty list is what "no half file" means.
     */
    static QStringList modelFiles(const QString &size);
    /** Puts a file where the model of `size` belongs, so a job can run. */
    static void placeModel(const QString &size);

    /** The runtime directory of this run, see initTestCase(). */
    std::unique_ptr<QTemporaryDir> m_runtime;
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
};

namespace
{
/** One buffer of a 440 Hz tone at 48 kHz mono, continuing at `phase`. */
QAudioBuffer tone(qint64 &phase, int frames)
{
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    format.setSampleFormat(QAudioFormat::Int16);

    QByteArray samples(static_cast<qsizetype>(frames) * 2, Qt::Uninitialized);
    auto *value = reinterpret_cast<qint16 *>(samples.data());
    for (int i = 0; i < frames; ++i, ++phase) {
        const double t = static_cast<double>(phase) / 48000.0;
        value[i] = static_cast<qint16>(12000.0 * std::sin(2.0 * std::numbers::pi * 440.0 * t));
    }
    return {samples, format};
}

/** The channel count of a 44-byte WAV header: two little-endian bytes at 22. */
quint16 wavChannels(const QByteArray &header)
{
    return static_cast<quint16>(static_cast<quint8>(header.at(22))
                                | (static_cast<quint8>(header.at(23)) << 8));
}

/** The sample rate of a 44-byte WAV header: four little-endian bytes at 24. */
quint32 wavSampleRate(const QByteArray &header)
{
    quint32 rate = 0;
    for (int i = 27; i >= 24; --i) {
        rate = (rate << 8) | static_cast<quint8>(header.at(i));
    }
    return rate;
}
}

void TranscribeTest::initTestCase()
{
    // As in main.cpp. Since issue #24 the reason a job failed with is a string
    // the user reads, so it goes through i18n() — and without the domain every
    // one of those calls warns that translation will not work. The English
    // wording the checks below compare stays in place because
    // tests/CMakeLists.txt pins LANGUAGE to the source language, for which no
    // catalogue exists.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // The transcriber reads its two program paths and its model out of
    // denkzettelrc. Test mode points QStandardPaths at a directory of the
    // test's own, so no setting of the developer's reaches this run — and none
    // of this run reaches theirs.
    QStandardPaths::setTestModeEnabled(true);

    // And a runtime directory of its own, because test mode does **not**
    // redirect that one: measured 2026-08-29, it stays /run/user/<uid> here as
    // it is for the daemon the user has running beside this. start() sweeps
    // abandoned working directories there, so without this line a `ctest` run
    // would take the WAV away from a transcription in progress — and the
    // daemon's next start would take ours.
    //
    // 0700 and owned by us, or QStandardPaths refuses the directory and warns.
    m_runtime = std::make_unique<QTemporaryDir>();
    QVERIFY(m_runtime->isValid());
    QVERIFY(QFile::setPermissions(m_runtime->path(),
                                  QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
    qputenv("XDG_RUNTIME_DIR", m_runtime->path().toUtf8());
    QCOMPARE(Transcriber::workingRoot(), m_runtime->path());

    // A file where the models of this run belong, for the two sizes the cases
    // below let the queue run with. Since issue #23 a job that finds no model
    // stops before whisper-cli with the sentence that names the settings page,
    // so without these the checks would all measure that one sentence. The
    // download cases work on `base`, which is deliberately not here.
    modelDirectory().removeRecursively();
    placeModel(QStringLiteral("small"));
    placeModel(QStringLiteral("tiny"));
    placeModel(QStringLiteral("medium"));
}

void TranscribeTest::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));
}

void TranscribeTest::cleanup()
{
    m_store.reset();
    m_dir.reset();
}

QString TranscribeTest::databasePath() const
{
    return m_dir->filePath(QStringLiteral("denkzettel.db"));
}

QString TranscribeTest::keptWav() const
{
    return m_dir->filePath(QStringLiteral("handed-over.wav"));
}

QString TranscribeTest::hangingPidFile() const
{
    return m_dir->filePath(QStringLiteral("hanging.pids"));
}

QList<qint64> TranscribeTest::hangingChildren() const
{
    QFile file(hangingPidFile());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QList<qint64> pids;
    const QList<QByteArray> lines = file.readAll().split('\n');
    for (const QByteArray &line : lines) {
        bool ok = false;
        const qint64 pid = line.trimmed().toLongLong(&ok);
        if (ok && pid > 0) {
            pids.append(pid);
        }
    }
    return pids;
}

qint64 TranscribeTest::addVoiceNote()
{
    const QDateTime createdAt = QDateTime::currentDateTime();
    AudioRecorder recorder(m_store->audioDirectory());
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&recorder, &AudioRecorder::finished);
    if (!recorder.startEncoder(createdAt)) {
        qWarning("%s", qUtf8Printable(recorder.lastError()));
        return -1;
    }
    qint64 phase = 0;
    for (int buffers = 3; buffers > 0; --buffers) {
        recorder.encode(tone(phase, 4800)); // 100 ms at 48 kHz
        QTest::qWait(10);
    }
    recorder.stop();
    if (!QTest::qWaitFor([&finished] { return finished.count() == 1; }, 5000)) {
        return -1;
    }

    Note note;
    note.createdAt = createdAt;
    note.type = Note::Type::Audio;
    // Empty, and it stays empty until there is a transcript: a placeholder
    // written in here would go into the search index, into the export and in
    // front of the analysis (SPEC 6, 8.3, 7). What the library shows in the
    // meantime is its own business.
    note.content.clear();
    note.audioPath = AudioRecorder::fileNameFor(createdAt);
    note.audioDurationS = 0;
    note.state = Note::State::New;

    const std::optional<qint64> id = m_store->addNote(note);
    if (!id.has_value()) {
        qWarning("addNote: %s", qUtf8Printable(m_store->lastError()));
    }
    return id.value_or(-1);
}

QString TranscribeTest::writeWhisperStub()
{
    // NOLINTNEXTLINE(misc-const-correctness) - returned below, see rule 1 in .clang-tidy
    QString path = m_dir->filePath(QStringLiteral("whisper-stub.sh"));
    QFile stub(path);
    if (!stub.open(QIODevice::WriteOnly)) {
        return {};
    }
    // It reads the two arguments the transcriber's contract with whisper-cli
    // rests on — `-f` and `-of` — and answers on the road whisper-cli answers
    // on: a file `<-of>.json`. The transcript carries umlauts and an ß on
    // purpose; the road from the JSON through SQLite into the note is where a
    // wrong encoding would show and nowhere else.
    stub.write("#!/bin/sh\n"
               "while [ $# -gt 0 ]; do\n"
               "  case \"$1\" in\n"
               "    -f) wav=\"$2\"; shift 2 ;;\n"
               "    -of) out=\"$2\"; shift 2 ;;\n"
               "    *) shift ;;\n"
               "  esac\n"
               "done\n"
               "cp \"$wav\" \"");
    stub.write(keptWav().toUtf8());
    stub.write("\"\n"
               "cat > \"$out.json\" <<'JSON'\n"
               "{ \"transcription\": [\n"
               "  { \"text\": \" Milch und Brötchen kaufen,\" },\n"
               "  { \"text\": \" die Straße ist gesperrt.\" } ] }\n"
               "JSON\n");
    stub.close();
    if (!stub.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        return {};
    }
    return path;
}

QString TranscribeTest::writeMarkingStub(const QString &name)
{
    // NOLINTNEXTLINE(misc-const-correctness) - returned below, see rule 1 in .clang-tidy
    QString path = m_dir->filePath(name + QStringLiteral(".sh"));
    QFile stub(path);
    if (!stub.open(QIODevice::WriteOnly)) {
        return {};
    }
    // The heredoc delimiter is unquoted on purpose, so that `$model` — the
    // argument the transcriber built out of the model size — is expanded into
    // the transcript.
    stub.write("#!/bin/sh\n"
               "while [ $# -gt 0 ]; do\n"
               "  case \"$1\" in\n"
               "    -m) model=\"$2\"; shift 2 ;;\n"
               "    -of) out=\"$2\"; shift 2 ;;\n"
               "    *) shift ;;\n"
               "  esac\n"
               "done\n"
               "cat > \"$out.json\" <<JSON\n"
               "{ \"transcription\": [ { \"text\": \" ");
    stub.write(name.toUtf8());
    stub.write(" $model\" } ] }\n"
               "JSON\n");
    stub.close();
    if (!stub.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        return {};
    }
    return path;
}

QString TranscribeTest::writeHangingStub()
{
    // NOLINTNEXTLINE(misc-const-correctness) - returned below, see rule 1 in .clang-tidy
    QString path = m_dir->filePath(QStringLiteral("whisper-hanging.sh"));
    QFile stub(path);
    if (!stub.open(QIODevice::WriteOnly)) {
        return {};
    }
    // Appended and not overwritten: every run of the stand-in leaves its line,
    // so the check reads the id of the run it means instead of whichever run
    // wrote last.
    //
    // `exec` and not `sleep 30 &`: the stand-in becomes the sleeping process
    // instead of starting one beside itself. A background child would outlive
    // the kill that hits the shell, and the check would then be measuring the
    // shell and not the run.
    stub.write("#!/bin/sh\necho $$ >> \"");
    stub.write(hangingPidFile().toUtf8());
    stub.write("\"\nexec sleep 30\n");
    stub.close();
    if (!stub.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        return {};
    }
    return path;
}

QStringList TranscribeTest::leftOverWorkingDirectories()
{
    // Asked of the same place the code writes to, and that place is the run's
    // own (initTestCase) — never the one the user's daemon works in.
    return QDir(Transcriber::workingRoot())
        .entryList({QStringLiteral("denkzettel-transcribe-*")}, QDir::Dirs);
}

void TranscribeTest::transcribesAnAudioNoteItWasHandedByTheStore()
{
    const QString stub = writeWhisperStub();
    QVERIFY(!stub.isEmpty());

    Transcriber transcriber(m_store.get());
    transcriber.setWhisperProgram(stub);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy transcribed(&transcriber, &Transcriber::transcribed);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&transcriber, &Transcriber::failed);

    // The road into the queue is the store's own signal: nobody has to tell the
    // transcriber about a note.
    const qint64 id = addVoiceNote();
    QVERIFY(id > 0);

    QVERIFY2(QTest::qWaitFor([&transcribed, &failed] { return transcribed.count() + failed.count() == 1; }, 30000),
             failed.isEmpty() ? "nothing happened at all" : qPrintable(failed.first().at(1).toString()));
    QCOMPARE(failed.count(), 0);
    QCOMPARE(transcribed.count(), 1);

    const std::optional<Note> note = m_store->note(id);
    QVERIFY(note.has_value());
    QCOMPARE(note->content, QStringLiteral("Milch und Brötchen kaufen, die Straße ist gesperrt."));
    QCOMPARE(note->state, Note::State::Transcribed);
    // The audio stays where it is: the note remains playable, transcript or not.
    QVERIFY(QFile::exists(m_store->audioDirectory() + QLatin1Char('/') + note->audioPath));

    // The finished job leaves the queue — that is what tells it from one that
    // failed for good, which keeps its line.
    QVERIFY(!m_store->transcribeJob(id).has_value());

    // And the transcript is findable. It reaches the index through the trigger
    // of schema version 2 and through nothing else; an UPDATE that went past it
    // would leave the note unfindable for ever without a word.
    const QList<Note> found = m_store->search(QStringLiteral("Straße"));
    QCOMPARE(found.size(), 1);
    QCOMPARE(found.first().id, id);

    // What the stand-in was handed, read out of the file it kept. 16000 and 1
    // are the numbers of SPEC 12, not numbers this run computed — the recording
    // is 48 kHz, so a conversion that did not happen would read 48000 here.
    QFile handed(keptWav());
    QVERIFY2(handed.open(QIODevice::ReadOnly), qPrintable(handed.fileName()));
    const QByteArray header = handed.read(44);
    QCOMPARE(header.left(4), QByteArrayLiteral("RIFF"));
    QCOMPARE(header.mid(8, 4), QByteArrayLiteral("WAVE"));
    QCOMPARE(wavChannels(header), quint16(1));
    QCOMPARE(wavSampleRate(header), 16000U);

    // The WAV was temporary and is gone with the job, together with the
    // directory it lay in.
    QCOMPARE(leftOverWorkingDirectories(), QStringList());
}

void TranscribeTest::takesUpAJobLeftBehindByAKilledRun()
{
    const QString stub = writeWhisperStub();
    QVERIFY(!stub.isEmpty());

    const qint64 id = addVoiceNote();
    QVERIFY(id > 0);
    QVERIFY2(m_store->enqueueTranscription(id), qPrintable(m_store->lastError()));

    // The state a daemon killed in the middle of the run leaves behind: the job
    // is taken, its attempt is counted, and no answer will ever come.
    const std::optional<TranscribeJob> taken = m_store->takeTranscribeJob();
    QVERIFY(taken.has_value());
    QCOMPARE(taken->noteId, id);
    QCOMPARE(taken->attempts, 1);

    // The restart: a new store on the same file, as the next start of the
    // daemon opens it.
    m_store.reset();
    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    Transcriber transcriber(m_store.get());
    transcriber.setWhisperProgram(stub);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy transcribed(&transcriber, &Transcriber::transcribed);
    // Nothing hands the job over here: start() finds it in the database, and
    // only there. A queue kept in memory would be empty at this point.
    transcriber.start();

    QVERIFY(QTest::qWaitFor([&transcribed] { return transcribed.count() == 1; }, 30000));
    const std::optional<Note> note = m_store->note(id);
    QVERIFY(note.has_value());
    QCOMPARE(note->state, Note::State::Transcribed);
    QVERIFY(!m_store->transcribeJob(id).has_value());
    QCOMPARE(leftOverWorkingDirectories(), QStringList());
}

void TranscribeTest::pausesAfterTwoAttemptsAndKeepsTheJobLine()
{
    Transcriber transcriber(m_store.get());
    // The error path without a graphics card: a program that is there, starts,
    // and ends with a code of its own (SPEC 12, and the finding of issue #19).
    transcriber.setWhisperProgram(QStringLiteral("/bin/false"));
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&transcriber, &Transcriber::failed);

    const qint64 id = addVoiceNote();
    QVERIFY(id > 0);

    QVERIFY(QTest::qWaitFor([&failed] { return failed.count() == Store::transcribeAttemptLimit; }, 30000));

    // The WAV of a failed attempt goes the same way as that of a finished one.
    // Asked here and not at the end of the case: start() below sweeps
    // abandoned directories, and after it this comparison would be green
    // whether the job cleared up after itself or not.
    QCOMPARE(leftOverWorkingDirectories(), QStringList());

    // And no third attempt. start() is asked a second time on purpose: the
    // pause has to hold against somebody taking the queue up again, which
    // every restart of the daemon does.
    transcriber.start();
    QTest::qWait(500);
    QCOMPARE(failed.count(), Store::transcribeAttemptLimit);

    // The job line stays, with the reason in it. Without it the library could
    // not tell a transcription that is still outstanding from one that was
    // given up on — `notes.state` reads 'neu' in both cases (SPEC 5.1).
    const std::optional<TranscribeJob> job = m_store->transcribeJob(id);
    QVERIFY(job.has_value());
    QCOMPARE(job->attempts, Store::transcribeAttemptLimit);
    QVERIFY(!job->lastError.isEmpty());

    // Nothing is lost: the note keeps its audio and no transcript (SPEC 12).
    const std::optional<Note> note = m_store->note(id);
    QVERIFY(note.has_value());
    QVERIFY(note->content.isEmpty());
    QCOMPARE(note->state, Note::State::New);
    QVERIFY(QFile::exists(m_store->audioDirectory() + QLatin1Char('/') + note->audioPath));
}

void TranscribeTest::reportsThePauseOnlyWhenTheAttemptsAreUsedUp()
{
    Transcriber transcriber(m_store.get());
    transcriber.setWhisperProgram(QStringLiteral("/bin/false"));

    // The **order** of the two signals is what is checked here, so both are
    // written into one list. A spy per signal only counts, and a count cannot
    // tell "paused after the second failure" from "paused after the first" —
    // which is the whole difference between a tray state that stands and one
    // that comes up for a moment and clears itself again (SPEC 12, issue #24).
    QStringList road;
    connect(&transcriber, &Transcriber::failed, this, [&road] {
        road.append(QStringLiteral("failed"));
    });
    connect(&transcriber, &Transcriber::paused, this, [&road](qint64, const QString &reason) {
        road.append(reason.isEmpty() ? QStringLiteral("paused, no reason") : QStringLiteral("paused"));
    });

    const qint64 id = addVoiceNote();
    QVERIFY(id > 0);

    // Waited for by the number of ends the queue can still produce and not by
    // isBusy(), which is false in the gap between two attempts as well
    // (CLAUDE.md, finding 32).
    QVERIFY(QTest::qWaitFor([&road] { return road.count() >= 3; }, 30000));
    QTest::qWait(500);

    const QStringList expected{QStringLiteral("failed"), QStringLiteral("failed"), QStringLiteral("paused")};
    QCOMPARE(road, expected);
    // And the reason the tray is handed is the one the job line keeps.
    const std::optional<TranscribeJob> job = m_store->transcribeJob(id);
    QVERIFY(job.has_value());
    QVERIFY(!job->lastError.isEmpty());
}

void TranscribeTest::reportsAJobThatWasGivenUpOnAtTheNextStart()
{
    const qint64 givenUp = addVoiceNote();
    QVERIFY(givenUp > 0);
    const qint64 stillWaiting = addVoiceNote();
    QVERIFY(stillWaiting > 0);
    QVERIFY2(m_store->enqueueTranscription(givenUp), qPrintable(m_store->lastError()));
    QVERIFY2(m_store->enqueueTranscription(stillWaiting), qPrintable(m_store->lastError()));

    // Two notes and not one, and that is the control: the second one has a
    // failed attempt behind it too, and it is the **newer** of the two. A
    // start that reported everything with a counted attempt would name it, not
    // the note that is really out of attempts.
    const QString reason = QStringLiteral("/usr/bin/whisper-cli ended with code 1");
    for (int attempt = 1; attempt <= Store::transcribeAttemptLimit; ++attempt) {
        const std::optional<TranscribeJob> taken = m_store->takeTranscribeJob();
        QVERIFY(taken.has_value());
        // Asserted, not assumed: which note the queue hands out decides what
        // the rest of this case builds, and the order is the store's business.
        QCOMPARE(taken->noteId, givenUp);
        QCOMPARE(taken->attempts, attempt);
        QVERIFY2(m_store->failTranscribeJob(givenUp, reason), qPrintable(m_store->lastError()));
    }
    const std::optional<TranscribeJob> once = m_store->takeTranscribeJob();
    QVERIFY(once.has_value());
    QCOMPARE(once->noteId, stillWaiting);
    QCOMPARE(once->attempts, 1);
    QVERIFY2(m_store->failTranscribeJob(stillWaiting, QStringLiteral("the first attempt failed")),
             qPrintable(m_store->lastError()));

    // The restart: a new store on the same file, as the next start of the
    // daemon opens it.
    m_store.reset();
    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    Transcriber transcriber(m_store.get());
    transcriber.setWhisperProgram(QStringLiteral("/bin/false"));
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy paused(&transcriber, &Transcriber::paused);
    transcriber.start();

    // Read straight away and without an event loop in between: what start()
    // finds in the database it says on the spot, and the attempt it takes up
    // for the second note runs in the event loop and cannot reach this line.
    QCOMPARE(paused.count(), 1);
    QCOMPARE(paused.first().at(0).toLongLong(), givenUp);
    QCOMPARE(paused.first().at(1).toString(), reason);
}

void TranscribeTest::namesAReasonForARunThatWasNeverAnsweredFor()
{
    const qint64 id = addVoiceNote();
    QVERIFY(id > 0);
    QVERIFY2(m_store->enqueueTranscription(id), qPrintable(m_store->lastError()));

    // What a daemon that is killed leaves behind: takeTranscribeJob() counts
    // the attempt before the run, and neither failTranscribeJob() nor
    // completeTranscription() ever follows. SIGTERM runs no destructor of ours.
    for (int attempt = 1; attempt <= Store::transcribeAttemptLimit; ++attempt) {
        const std::optional<TranscribeJob> taken = m_store->takeTranscribeJob();
        QVERIFY(taken.has_value());
        QCOMPARE(taken->attempts, attempt);
    }
    const std::optional<TranscribeJob> unreadable = m_store->transcribeJob(id);
    QVERIFY(unreadable.has_value());
    // Asserted while it stands, and it is what makes the check a check: with
    // the repair taken out the line below comes out empty instead.
    QVERIFY(unreadable->lastError.isEmpty());

    m_store.reset();
    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    Transcriber transcriber(m_store.get());
    transcriber.setWhisperProgram(QStringLiteral("/bin/false"));
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy paused(&transcriber, &Transcriber::paused);
    transcriber.start();

    QCOMPARE(paused.count(), 1);
    QCOMPARE(paused.first().at(0).toLongLong(), id);
    // Without the reason the tooltip would read "Transcription failed:" and
    // break off — the unreadable state issue #22 left behind on purpose.
    QVERIFY2(!paused.first().at(1).toString().isEmpty(), "the pause was reported without a reason");

    const std::optional<TranscribeJob> repaired = m_store->transcribeJob(id);
    QVERIFY(repaired.has_value());
    QCOMPARE(repaired->lastError, paused.first().at(1).toString());
    // And no attempt was added by the repair: the count is written before a
    // run and is not the thing that was missing.
    QCOMPARE(repaired->attempts, Store::transcribeAttemptLimit);
}

void TranscribeTest::givesUpOnARunThatHangs()
{
    const QString hanging = writeHangingStub();
    QVERIFY(!hanging.isEmpty());

    // Two notes, because the second half of the criterion is that the queue
    // goes on afterwards — one note could only ever show the same job again.
    const qint64 first = addVoiceNote();
    QVERIFY(first > 0);
    const qint64 second = addVoiceNote();
    QVERIFY(second > 0);
    QVERIFY2(m_store->enqueueTranscription(first), qPrintable(m_store->lastError()));
    QVERIFY2(m_store->enqueueTranscription(second), qPrintable(m_store->lastError()));

    // The control, and the case rests on it: the same hanging run under the
    // five minutes the customer chose. They do not pass here, so nothing ends —
    // that is the state issue #113 describes and the state a normal run is in.
    {
        Transcriber standing(m_store.get());
        standing.setWhisperProgram(hanging);
        // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
        QSignalSpy failed(&standing, &Transcriber::failed);
        standing.start();

        QVERIFY(QTest::qWaitFor([this] { return !hangingChildren().isEmpty(); }, 30000));
        QTest::qWait(1000);
        QCOMPARE(failed.count(), 0);
        // Alive, and asserted while it is: without this line the run below
        // would be green over a child that had never started.
        QVERIFY(QFile::exists(QStringLiteral("/proc/%1").arg(hangingChildren().constFirst())));
    }

    QVERIFY(QFile::remove(hangingPidFile()));

    Transcriber transcriber(m_store.get());
    transcriber.setWhisperProgram(hanging);
    // Long enough to tell the limit from the start of the run, short enough
    // that a ctest run waits for it. Five minutes are what the daemon uses and
    // what the control above ran under.
    transcriber.setTimeout(std::chrono::milliseconds(500));
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&transcriber, &Transcriber::failed);
    transcriber.start();

    QVERIFY(QTest::qWaitFor([this] { return !hangingChildren().isEmpty(); }, 30000));
    const qint64 child = hangingChildren().constFirst();
    QVERIFY(QFile::exists(QStringLiteral("/proc/%1").arg(child)));

    QVERIFY(QTest::qWaitFor([&failed] { return failed.count() >= 1; }, 30000));
    QCOMPARE(failed.first().at(0).toLongLong(), first);
    QVERIFY2(failed.first().at(1).toString().contains(QStringLiteral("did not finish")),
             qPrintable(failed.first().at(1).toString()));
    // The child is gone with the job, and it is gone before the failure is
    // announced: fail() runs after the kill has been waited for. PR_SET_PDEATHSIG
    // does not reach this — the daemon is alive and gave the job up itself.
    QVERIFY(!QFile::exists(QStringLiteral("/proc/%1").arg(child)));

    // And the queue moves on: the next job is a different note, not the one
    // that was given up on.
    QVERIFY(QTest::qWaitFor([&failed] { return failed.count() >= 2; }, 30000));
    QCOMPARE(failed.at(1).at(0).toLongLong(), second);

    // The reason reaches the job line. Without it the attempt is counted and
    // nothing says why — the unreadable state issue #22 left behind.
    const std::optional<TranscribeJob> job = m_store->transcribeJob(first);
    QVERIFY(job.has_value());
    QCOMPARE(job->attempts, Store::transcribeAttemptLimit);
    QVERIFY(!job->lastError.isEmpty());

    // Three runs and no more: the first note had one attempt left, the second
    // had both. Waited for by that number and not by isBusy(), which is false
    // for a moment between two jobs as well — caught there, the checks below
    // would be measuring a queue that goes on running under them.
    QVERIFY(QTest::qWaitFor([&failed] { return failed.count() >= 3; }, 30000));
    QVERIFY(QTest::qWaitFor([&transcriber] { return !transcriber.isBusy(); }, 30000));
    QCOMPARE(leftOverWorkingDirectories(), QStringList());
    // No run of the stand-in survived its job.
    const QList<qint64> children = hangingChildren();
    QCOMPARE(children.size(), 3);
    for (const qint64 pid : children) {
        QVERIFY2(!QFile::exists(QStringLiteral("/proc/%1").arg(pid)),
                 qPrintable(QStringLiteral("process %1 outlived its job").arg(pid)));
    }
}

void TranscribeTest::namesTheProgramWhenItIsNotInstalledAtAll()
{
    Transcriber transcriber(m_store.get());
    // whisper.cpp is an **optional** dependency of the package (SPEC 15, issue
    // #41): the program is meant to stay usable without it. That is a different
    // road than the case above — a program that is not there never starts, so
    // QProcess emits errorOccurred and no finished() ever follows. Without the
    // handler on that signal the job would neither fail nor be counted, and the
    // queue would stand still with nothing said anywhere.
    const QString absent = m_dir->filePath(QStringLiteral("whisper-cli-not-installed"));
    QVERIFY(!QFile::exists(absent));
    transcriber.setWhisperProgram(absent);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&transcriber, &Transcriber::failed);

    const qint64 id = addVoiceNote();
    QVERIFY(id > 0);

    QVERIFY(QTest::qWaitFor([&failed] { return failed.count() == Store::transcribeAttemptLimit; }, 30000));

    // The reason names the program that is missing. Without the name the user
    // looks for the fault in their recording.
    const std::optional<TranscribeJob> job = m_store->transcribeJob(id);
    QVERIFY(job.has_value());
    QVERIFY2(job->lastError.contains(absent), qPrintable(job->lastError));

    // And nothing is lost: the note keeps its audio and stays playable, exactly
    // as after a program that ran and failed.
    const std::optional<Note> note = m_store->note(id);
    QVERIFY(note.has_value());
    QVERIFY(note->content.isEmpty());
    QCOMPARE(note->state, Note::State::New);
    QVERIFY(QFile::exists(m_store->audioDirectory() + QLatin1Char('/') + note->audioPath));
}

void TranscribeTest::stripsTheDirectoriesOffAReasonBeforeItIsForwarded()
{
    // The job records the program with its whole path and the case above
    // asserts that it does; a notification carries the same statement without
    // it (SPEC 10, issue #115), because it can be forwarded and the path is a
    // setting that may lie in the user's home. This is a filter over the error
    // texts of somebody else's programs, and it failing is nothing anybody sees
    // but the user, on their own machine, with their own configured path.
    QCOMPARE(reasonWithoutDirectories(QStringLiteral("/home/someone/tools/whisper-cli ended with code 2")),
             QStringLiteral("whisper-cli ended with code 2"));
    QCOMPARE(reasonWithoutDirectories(QStringLiteral("/usr/bin/ffmpeg could not be started")),
             QStringLiteral("ffmpeg could not be started"));

    // A reason that never carried a path is handed on word for word — the
    // other failure paths of SPEC 12 are not to be nibbled at.
    QCOMPARE(reasonWithoutDirectories(QStringLiteral("The audio file 20260829-091500.ogg is missing")),
             QStringLiteral("The audio file 20260829-091500.ogg is missing"));
    QCOMPARE(reasonWithoutDirectories(QStringLiteral("The run was interrupted")),
             QStringLiteral("The run was interrupted"));

    // And the ceiling the declaration names, written down so that it stays a
    // decision instead of becoming a surprise: a blank inside the path leaves
    // what follows the last one standing, and a relative path stays whole
    // because it names no home. Both keep the statement readable.
    QCOMPARE(reasonWithoutDirectories(QStringLiteral("/home/someone/my tools/whisper-cli ended with code 2")),
             QStringLiteral("my tools/whisper-cli ended with code 2"));
    QCOMPARE(reasonWithoutDirectories(QStringLiteral("bin/whisper-cli could not be started")),
             QStringLiteral("bin/whisper-cli could not be started"));
}

void TranscribeTest::clearsTheWorkingDirectoryOfAKilledRun()
{
    // A daemon that is killed runs no destructor. What it leaves behind is a
    // directory of this name with the WAV of the run in it, and nobody but the
    // next start ever comes back to it.
    const QString abandoned =
        Transcriber::workingRoot() + QStringLiteral("/denkzettel-transcribe-killedrun");
    QVERIFY(QDir().mkpath(abandoned));
    QFile left(abandoned + QStringLiteral("/audio.wav"));
    QVERIFY(left.open(QIODevice::WriteOnly));
    left.write(QByteArrayLiteral("RIFF"));
    left.close();

    Transcriber transcriber(m_store.get());
    transcriber.start();

    QVERIFY(!QFile::exists(left.fileName()));
    QVERIFY(!QDir(abandoned).exists());
}

void TranscribeTest::deletingANoteTakesItsJobWithIt()
{
    const qint64 id = addVoiceNote();
    QVERIFY(id > 0);
    QVERIFY2(m_store->enqueueTranscription(id), qPrintable(m_store->lastError()));

    // With `PRAGMA foreign_keys = ON` the job line would block the deletion of
    // the note it points at — deleting a voice note whose transcription is
    // still queued is the ordinary case and would fail outright.
    QVERIFY2(m_store->removeNote(id), qPrintable(m_store->lastError()));
    QVERIFY(!m_store->transcribeJob(id).has_value());
}

void TranscribeTest::aChangedSettingReachesTheRunningQueue()
{
    const QString before = writeMarkingStub(QStringLiteral("whisper-before"));
    const QString after = writeMarkingStub(QStringLiteral("whisper-after"));
    QVERIFY(!before.isEmpty());
    QVERIFY(!after.isEmpty());

    // The settings the daemon starts with. Written through the same
    // KSharedConfig the transcriber reads from, so no file name is named here
    // — under QTest that name is the binary's and not `denkzettelrc`
    // (finding 42 of CLAUDE.md).
    KConfigGroup settings(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    settings.writeEntry("WhisperProgram", before);
    settings.writeEntry("ModelSize", QStringLiteral("small"));
    settings.sync();

    Transcriber transcriber(m_store.get());
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy transcribed(&transcriber, &Transcriber::transcribed);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&transcriber, &Transcriber::failed);

    // The loud state first (finding 27): what the queue runs BEFORE anything
    // changes has to be read, or "it takes the new value" is green whatever
    // the code does.
    const qint64 first = addVoiceNote();
    QVERIFY(first > 0);
    QVERIFY2(QTest::qWaitFor([&transcribed, &failed] { return transcribed.count() + failed.count() == 1; }, 30000),
             failed.isEmpty() ? "nothing happened at all" : qPrintable(failed.first().at(1).toString()));
    QCOMPARE(failed.count(), 0);
    std::optional<Note> note = m_store->note(first);
    QVERIFY(note.has_value());
    QCOMPARE(note->content,
             QStringLiteral("whisper-before ") + Transcriber::modelPath(QStringLiteral("small")));

    // And now what the settings page does: the values change under the running
    // object. reloadSettings() is the slot main.cpp hangs on the skeleton's
    // configChanged(), and nothing else here is restarted.
    settings.writeEntry("WhisperProgram", after);
    settings.writeEntry("ModelSize", QStringLiteral("tiny"));
    settings.sync();
    transcriber.reloadSettings();

    const qint64 second = addVoiceNote();
    QVERIFY(second > 0);
    QVERIFY2(QTest::qWaitFor([&transcribed, &failed] { return transcribed.count() + failed.count() == 2; }, 30000),
             failed.isEmpty() ? "the second job never ended" : qPrintable(failed.first().at(1).toString()));
    QCOMPARE(failed.count(), 0);
    note = m_store->note(second);
    QVERIFY(note.has_value());
    // Both halves come out different: the other program, and the model path
    // the other size builds — `tiny` and not the `small` the file held when
    // this transcriber was constructed.
    QCOMPARE(note->content,
             QStringLiteral("whisper-after ") + Transcriber::modelPath(QStringLiteral("tiny")));

    // The group goes again, or the next run of this set would start with a
    // program path out of a temporary directory that is long gone.
    settings.deleteGroup();
    settings.sync();
}

/** Runs one voice note through the queue and answers with its transcript. */
QString TranscribeTest::transcriptOfOneNote(Transcriber &transcriber)
{
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy transcribed(&transcriber, &Transcriber::transcribed);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&transcriber, &Transcriber::failed);
    const qint64 id = addVoiceNote();
    if (id <= 0) {
        return QStringLiteral("the note was not added");
    }
    if (!QTest::qWaitFor([&transcribed, &failed] { return transcribed.count() + failed.count() == 1; }, 30000)) {
        return QStringLiteral("nothing happened at all");
    }
    if (!failed.isEmpty()) {
        return failed.first().at(1).toString();
    }
    const std::optional<Note> note = m_store->note(id);
    return note.has_value() ? note->content : QStringLiteral("the note is gone");
}

void TranscribeTest::takesOverAModelPathOfTheVersionBefore()
{
    const QString stub = writeMarkingStub(QStringLiteral("whisper-taken-over"));
    QVERIFY(!stub.isEmpty());

    // What a version before issue #27 wrote, and what a hand writes: the file
    // lies where the user put it, and only its name says which model it is.
    KConfigGroup settings(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    settings.writeEntry("WhisperProgram", stub);
    settings.writeEntry("ModelPath", m_dir->filePath(QStringLiteral("ggml-medium.bin")));
    settings.deleteEntry("ModelSize");
    settings.sync();

    migrateModelPath();

    // Read out of a group opened afresh, not out of the one that did the
    // writing (finding 42's neighbourhood): what matters is what stands in the
    // file for the next start.
    {
        const KConfigGroup written(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
        QCOMPARE(written.readEntry("ModelSize", QString()), QStringLiteral("medium"));
        QVERIFY(!written.hasKey("ModelPath"));
    }

    // And it is not the file alone: the queue really runs against `medium`.
    // Without the migration this same run answers `ggml-small.bin`, which is
    // the whole of the fault it closes.
    Transcriber transcriber(m_store.get());
    QCOMPARE(transcriptOfOneNote(transcriber),
             QStringLiteral("whisper-taken-over ") + Transcriber::modelPath(QStringLiteral("medium")));

    // The other half of the decision: a path that is no size of ours is not
    // touched. It was a deliberate act of somebody who knew what they were
    // doing, and it is the whole of the state the settings page reports from —
    // so nothing is written and nothing is deleted.
    settings.writeEntry("ModelPath", QStringLiteral("/opt/whisper/one-of-my-own.bin"));
    settings.deleteEntry("ModelSize");
    settings.sync();

    migrateModelPath();

    const KConfigGroup kept(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    QCOMPARE(kept.readEntry("ModelPath", QString()), QStringLiteral("/opt/whisper/one-of-my-own.bin"));
    QVERIFY(!kept.hasKey("ModelSize"));

    settings.deleteGroup();
    settings.sync();
}

void TranscribeTest::fallsBackToTheDefaultForASizeItDoesNotKnow()
{
    const QString stub = writeMarkingStub(QStringLiteral("whisper-unknown-size"));
    QVERIFY(!stub.isEmpty());

    // A hand-written denkzettelrc reaches the queue without ever passing
    // through the dialog, and a size the dialog could never produce would
    // otherwise be pasted into a file name — a path that leads nowhere, two
    // failed attempts and the tray in its error state (SPEC 12).
    KConfigGroup settings(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    settings.writeEntry("WhisperProgram", stub);
    settings.writeEntry("ModelSize", QStringLiteral("enormous"));
    settings.sync();

    Transcriber transcriber(m_store.get());
    QCOMPARE(transcriptOfOneNote(transcriber),
             QStringLiteral("whisper-unknown-size ")
                 + Transcriber::modelPath(QString(whisper::Sizes.at(whisper::DefaultSize))));

    settings.deleteGroup();
    settings.sync();
}

QDir TranscribeTest::modelDirectory()
{
    // The run's own, through QStandardPaths test mode (initTestCase) — never
    // the ~/.local/share/denkzettel/models of whoever runs the check, where
    // their models lie.
    return QFileInfo(Transcriber::modelPath(QStringLiteral("tiny"))).absoluteDir();
}

QStringList TranscribeTest::modelFiles(const QString &size)
{
    return modelDirectory().entryList({QStringLiteral("ggml-%1.bin*").arg(size)},
                                      QDir::Files,
                                      QDir::Name);
}

void TranscribeTest::placeModel(const QString &size)
{
    const QString path = Transcriber::modelPath(size);
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    // Content nobody reads: whisper.cpp is not run here (see the class
    // comment), and what the queue asks of this file is that it is there.
    file.write(QByteArrayLiteral("not a model"));
}

namespace
{
/** 256 KiB the stand-in serves as the model. */
QByteArray modelBytes()
{
    QByteArray bytes(qsizetype(256) * 1024, Qt::Uninitialized);
    for (qsizetype i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<char>(i * 7 + 13);
    }
    return bytes;
}

QString sha1Of(const QByteArray &bytes)
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex());
}

QByteArray httpHead(qsizetype length)
{
    return "HTTP/1.1 200 OK\r\nContent-Length: " + QByteArray::number(length)
        + "\r\nConnection: close\r\n\r\n";
}
}

void TranscribeTest::namesTheSettingsPageWhenTheModelIsMissing()
{
    // The queue never fetches a model itself (UX decision, 29.08.2026), so the
    // one thing it owes the user is the way to the place that does. Without
    // this the same case says "whisper-cli wrote no transcript" twice and the
    // tray goes red with nothing to act on.
    const QString stub = writeWhisperStub();
    QVERIFY(!stub.isEmpty());
    QVERIFY(QFile::remove(Transcriber::modelPath(QStringLiteral("small"))));

    Transcriber transcriber(m_store.get());
    transcriber.setWhisperProgram(stub);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&transcriber, &Transcriber::failed);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy transcribed(&transcriber, &Transcriber::transcribed);

    QVERIFY(addVoiceNote() > 0);
    QVERIFY(QTest::qWaitFor([&failed] { return failed.count() >= 1; }, 30000));
    QCOMPARE(transcribed.count(), 0);
    QCOMPARE(failed.constFirst().at(1).toString(),
             QStringLiteral("Model small is missing. Download it under Settings → Voice notes."));
    // And whisper-cli was never started: the stand-in keeps the WAV it is
    // handed, so a run of it would leave that file behind.
    QVERIFY(!QFile::exists(keptWav()));

    placeModel(QStringLiteral("small"));
}

void TranscribeTest::fetchesAModelAndWritesItWhenTheChecksumAgrees()
{
    // The whole road, against a stand-in on the loopback interface: no name is
    // resolved and nothing leaves the machine (aitest takes the same road).
    // Two things are proven here that a run against the real address could not
    // prove any better — that the **redirect** is followed, which is what the
    // upstream answers with (302 → 200, measured 29.08.2026), and that a
    // leftover of a killed run is swept before the new file is written.
    const QByteArray body = modelBytes();

    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));
    const quint16 port = server.serverPort();

    // Shared and not captured by reference: the inner lambda outlives this
    // frame as far as anything but a reading of the code can tell, and the CI
    // fails on the clazy warning that says so (aitest.cpp holds the same note).
    auto requests = std::make_shared<int>(0);
    connect(&server, &QTcpServer::newConnection, this, [&server, requests, port, body] {
        QTcpSocket *socket = server.nextPendingConnection();
        auto request = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, socket, [socket, request, requests, port, body] {
            request->append(socket->readAll());
            if (!request->contains("\r\n\r\n")) {
                return;
            }
            ++*requests;
            if (request->startsWith("GET /ggml-base.bin ")) {
                socket->write("HTTP/1.1 302 Found\r\nLocation: http://127.0.0.1:"
                              + QByteArray::number(port)
                              + "/elsewhere\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            } else {
                socket->write(httpHead(body.size()) + body);
            }
            socket->disconnectFromHost();
        });
    });

    ModelDownload download;
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&download, &ModelDownload::finished);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy progressed(&download, &ModelDownload::progress);
    download.start(QStringLiteral("base"),
                   QUrl(QStringLiteral("http://127.0.0.1:%1/ggml-base.bin").arg(port)),
                   sha1Of(body));
    QVERIFY(finished.wait(std::chrono::seconds(20)));

    QCOMPARE(finished.constFirst().at(1).toString(), QString());
    // The progress the page shows, and it is counted out of what is written —
    // the last one says the whole file (see ModelDownload::progress).
    QVERIFY(!progressed.isEmpty());
    QCOMPARE(progressed.constLast().at(0).toLongLong(), static_cast<qint64>(body.size()));
    QCOMPARE(progressed.constLast().at(1).toLongLong(), static_cast<qint64>(body.size()));
    // Two requests, and that is the redirect: with one the client would have
    // written the 302's empty body as the model, and the checksum would have
    // been the only thing between that and the queue.
    QCOMPARE(*requests, 2);

    QFile written(Transcriber::modelPath(QStringLiteral("base")));
    QVERIFY2(written.open(QIODevice::ReadOnly), qPrintable(written.fileName()));
    QCOMPARE(written.readAll(), body);
    written.close();

    // The model and nothing beside it.
    QCOMPARE(modelFiles(QStringLiteral("base")), QStringList({QStringLiteral("ggml-base.bin")}));
    QVERIFY(QFile::remove(Transcriber::modelPath(QStringLiteral("base"))));
}

void TranscribeTest::leavesNothingBehindWhenTheDownloadIsCancelled()
{
    // The acceptance criterion of issue #23: cancelling leaves no half file.
    // The stand-in promises twice as much as it sends and then holds the
    // connection — that is a download in progress, and the only thing a cancel
    // can be measured on.
    const QByteArray body = modelBytes();

    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));
    const quint16 port = server.serverPort();

    connect(&server, &QTcpServer::newConnection, this, [&server, body] {
        QTcpSocket *socket = server.nextPendingConnection();
        auto request = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, socket, [socket, request, body] {
            request->append(socket->readAll());
            if (!request->contains("\r\n\r\n")) {
                return;
            }
            // Promised 8 MB, 4 MB sent and then the connection held open.
            // The 4 MB are not a round number of nothing: with 256 KiB the
            // client delivered not one byte to the reply until the connection
            // closed (measured 29.08.2026), and a cancel needs a transfer that
            // is really running.
            socket->write(httpHead(body.size() * 32));
            for (int i = 0; i < 16; ++i) {
                socket->write(body);
            }
            socket->flush();
        });
    });

    ModelDownload download;
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&download, &ModelDownload::finished);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy progressed(&download, &ModelDownload::progress);
    download.start(QStringLiteral("base"),
                   QUrl(QStringLiteral("http://127.0.0.1:%1/ggml-base.bin").arg(port)),
                   sha1Of(body));
    QVERIFY(progressed.wait(std::chrono::seconds(20)));
    QVERIFY(download.isRunning());

    // Read **while** megabytes are arriving, and this is the whole of the
    // guarantee: the directory holds nothing at all yet. QSaveFile writes into
    // an inode with no name (O_TMPFILE) and links it in at commit(), so there
    // is no half file to be seen at any moment — a plain QFile in its place
    // would show a growing ggml-base.bin here.
    QCOMPARE(modelFiles(QStringLiteral("base")), QStringList());

    download.cancel();
    // Not finished.wait(): abort() takes the reply down **inside** cancel(),
    // so the signal is already in the spy when this line is reached and a wait
    // for a new one would sit out its whole timeout (measured 29.08.2026).
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 20000);
    QVERIFY(!finished.constFirst().at(1).toString().isEmpty());

    QCOMPARE(modelFiles(QStringLiteral("base")), QStringList());
}

void TranscribeTest::leavesNothingBehindWhenTheConnectionBreaks()
{
    // The second of the three roads that have to end in the same state: not a
    // hand on a button but a line that goes away in the middle of the file.
    const QByteArray body = modelBytes();

    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));
    const quint16 port = server.serverPort();

    auto requests = std::make_shared<int>(0);
    connect(&server, &QTcpServer::newConnection, this, [&server, requests, body] {
        QTcpSocket *socket = server.nextPendingConnection();
        auto request = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, socket, [socket, request, requests, body] {
            request->append(socket->readAll());
            if (!request->contains("\r\n\r\n")) {
                return;
            }
            ++*requests;
            // Promised twice, sent once, then gone — and every time, not only
            // the first: a stand-in built to be survived measures the first
            // failure and stops (CLAUDE.md, finding 41).
            socket->write(httpHead(body.size() * 2) + body);
            socket->flush();
            socket->abort();
        });
    });

    ModelDownload download;
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&download, &ModelDownload::finished);
    download.start(QStringLiteral("base"),
                   QUrl(QStringLiteral("http://127.0.0.1:%1/ggml-base.bin").arg(port)),
                   sha1Of(body));
    QVERIFY(finished.wait(std::chrono::seconds(20)));

    QVERIFY2(!finished.constFirst().at(1).toString().isEmpty(),
             "a connection that broke off is not a finished download");
    QCOMPARE(modelFiles(QStringLiteral("base")), QStringList());
}

void TranscribeTest::leavesNothingBehindWhenWhatArrivesIsNotTheModel()
{
    // The third road, and the one nothing else can catch: the transfer runs
    // through to the end and what arrived is a login page, a proxy's error or
    // a body somebody cut short. Only the SHA-1 of SPEC 12 tells that from a
    // model — without it this is what would be renamed onto the model.
    const QByteArray body = modelBytes();

    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));
    const quint16 port = server.serverPort();

    connect(&server, &QTcpServer::newConnection, this, [&server, body] {
        QTcpSocket *socket = server.nextPendingConnection();
        auto request = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, socket, [socket, request, body] {
            request->append(socket->readAll());
            if (!request->contains("\r\n\r\n")) {
                return;
            }
            socket->write(httpHead(body.size()) + body);
            socket->disconnectFromHost();
        });
    });

    ModelDownload download;
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&download, &ModelDownload::finished);
    // Held against the checksum of the real `base`, which these 256 KiB are
    // not — the one value in this file that comes from outside it.
    download.start(QStringLiteral("base"),
                   QUrl(QStringLiteral("http://127.0.0.1:%1/ggml-base.bin").arg(port)),
                   ModelDownload::checksumFor(QStringLiteral("base")));
    QVERIFY(finished.wait(std::chrono::seconds(20)));

    QCOMPARE(finished.constFirst().at(1).toString(),
             QStringLiteral("what arrived is not the model base"));
    QCOMPARE(modelFiles(QStringLiteral("base")), QStringList());
}

QTEST_GUILESS_MAIN(TranscribeTest)

#include "transcribetest.moc"
