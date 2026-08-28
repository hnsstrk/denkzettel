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
