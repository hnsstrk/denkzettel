#include "capture/audiorecorder.h"
#include "store/store.h"
#include "transcribe/transcriber.h"

#include <QAudioBuffer>
#include <QAudioFormat>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
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
    void givesUpOnARunThatHangs();
    void namesTheProgramWhenItIsNotInstalledAtAll();
    void clearsTheWorkingDirectoryOfAKilledRun();
    void deletingANoteTakesItsJobWithIt();

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

QTEST_GUILESS_MAIN(TranscribeTest)

#include "transcribetest.moc"
