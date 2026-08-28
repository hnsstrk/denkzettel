#include "capture/audiorecorder.h"

#include <QAudioBuffer>
#include <QAudioSource>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QTimer>
#include <QUrl>

namespace
{
constexpr int SampleRate = 48000;
}

AudioRecorder::AudioRecorder(QString audioDirectory, QObject *parent)
    : QObject(parent)
    , m_audioDirectory(std::move(audioDirectory))
{
    m_format.setSampleRate(SampleRate);
    m_format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    m_format.setSampleFormat(QAudioFormat::Int16);

    m_session.setAudioBufferInput(&m_input);
    m_session.setRecorder(&m_recorder);

    QMediaFormat mediaFormat;
    mediaFormat.setFileFormat(QMediaFormat::Ogg);
    mediaFormat.setAudioCodec(QMediaFormat::AudioCodec::Opus);
    m_recorder.setMediaFormat(mediaFormat);
    m_recorder.setAudioChannelCount(1);
    m_recorder.setAudioSampleRate(SampleRate);

    connect(&m_input, &QAudioBufferInput::readyToSendAudioBuffer, this, &AudioRecorder::flush);
    connect(&m_recorder, &QMediaRecorder::recorderStateChanged, this, &AudioRecorder::handleRecorderState);
    connect(&m_recorder, &QMediaRecorder::errorOccurred, this, [this](QMediaRecorder::Error, const QString &message) {
        handleRecorderError(message);
    });
}

AudioRecorder::~AudioRecorder()
{
    // **Every** state that still owes an answer, and Stopping belongs to them:
    // since stop() waits for the tail of the queue, Ctrl+Enter and the window
    // closing in the same turn leave a recording whose muxer has not finished.
    // A destroyed object emits nothing, so no note will ever be made out of
    // that file — and a file nothing points at is the orphan of SPEC 8.1. It
    // goes, whichever of the three states we are in.
    //
    // Unlinking underneath the open muxer is what POSIX is for: the handle
    // writes on into an inode that is already nameless and goes when the
    // encoder below closes it.
    if (m_state == State::Idle) {
        return;
    }
    // Idle first, so the stop below cannot come back through
    // handleRecorderState() as a finished recording.
    m_state = State::Idle;
    m_recorder.stop();
    removeFile();
}

void AudioRecorder::removeFile()
{
    // What the recorder says it wrote, not what it was told to write. Measured
    // 2026-08-28: handed a directory in place of the file name, QMediaRecorder
    // makes up a name inside it (`record_0001.ogv`) — deleting the path we
    // asked for would leave the recording standing.
    const QUrl written = m_recorder.actualLocation();
    const QString file = written.isEmpty() ? filePath() : written.toLocalFile();
    if (QFile::exists(file) && !QFile::remove(file)) {
        qWarning("Deleting the audio file %s failed", qUtf8Printable(file));
    }
}

QString AudioRecorder::fileNameFor(const QDateTime &createdAt)
{
    // The form the store writes into `created_at` (SPEC 5.1), so the name of
    // the file is the timestamp of the note and not a second rendering of it.
    // The milliseconds come along and settle the only collision two recordings
    // could have.
    //
    // With one departure, and SPEC 4 carries it: the colons of the hour become
    // hyphens. FAT and exFAT forbid `:` in a name, and the full export of SPEC
    // 8.3 writes the audio files onto whatever the user points it at — a USB
    // stick is the case it exists for. Nobody reading the name notices the
    // difference; a copy that fails on a stick is noticed at once.
    QString name = createdAt.toString(Qt::ISODateWithMs);
    name.replace(QLatin1Char(':'), QLatin1Char('-'));
    return name + QStringLiteral(".ogg");
}

void AudioRecorder::setMaximumDuration(qint64 milliseconds)
{
    m_maximumDuration = milliseconds;
}

qint64 AudioRecorder::maximumDuration() const
{
    return m_maximumDuration;
}

QString AudioRecorder::filePath() const
{
    return m_audioDirectory + QLatin1Char('/') + m_fileName;
}

bool AudioRecorder::startEncoder(const QDateTime &createdAt)
{
    if (m_state != State::Idle) {
        m_lastError = QStringLiteral("A recording is already running");
        return false;
    }
    m_lastError.clear();

    if (!QDir().mkpath(m_audioDirectory)) {
        m_lastError = QStringLiteral("Creating directory %1 failed").arg(m_audioDirectory);
        return false;
    }

    // Asked of the backend that is actually loaded, and it can only be asked
    // here: the backend is a plugin, it is loaded on first use, and
    // find_package() at build time sees nothing of it. qt6-multimedia depends
    // on a virtual package with two providers, and the gstreamer one writes a
    // different set of formats. Without this the failure would follow the
    // pattern of finding 24 in CLAUDE.md — every buffer refused, no error, a
    // 127-byte file, return code 0.
    if (!m_recorder.mediaFormat().isSupported(QMediaFormat::Encode)) {
        m_lastError = QStringLiteral("This QtMultimedia backend does not encode Opus in OGG");
        return false;
    }

    m_fileName = fileNameFor(createdAt);
    m_pending.clear();
    m_pendingFrames = 0;
    m_frames = 0;
    m_state = State::Recording;
    ++m_generation;
    m_elapsed.start();
    m_recorder.setOutputLocation(QUrl::fromLocalFile(filePath()));
    m_recorder.record();

    // The wall clock needs a wakeup of its own. flush() reads it too, but
    // flush() only runs when a buffer arrives — and a device that goes silent
    // without an error delivers none, so the recording would fall through both
    // clocks and run until somebody notices. That is the forgotten recording
    // SPEC 4 names, in its purest form.
    //
    // The bound is read here and not later: a setMaximumDuration() after the
    // start moves what flush() compares against, not this wakeup.
    if (m_maximumDuration > 0) {
        QTimer::singleShot(m_maximumDuration, this, [this, generation = m_generation] {
            if (m_generation == generation && m_state == State::Recording) {
                stop();
            }
        });
    }
    return true;
}

bool AudioRecorder::start(const QDateTime &createdAt, const QAudioDevice &device)
{
    const QAudioDevice input = device.isNull() ? QMediaDevices::defaultAudioInput() : device;
    if (input.isNull()) {
        m_lastError = QStringLiteral("No audio input device");
        return false;
    }
    if (!input.isFormatSupported(m_format)) {
        m_lastError = QStringLiteral("%1 records no mono 48 kHz").arg(input.description());
        return false;
    }
    if (!startEncoder(createdAt)) {
        return false;
    }

    m_source = std::make_unique<QAudioSource>(input, m_format);
    QIODevice *stream = m_source->start();
    if (stream == nullptr) {
        m_lastError = QStringLiteral("Opening %1 failed").arg(input.description());
        // Idle before the recorder is stopped, not after: the stop travels
        // through handleRecorderState(), and a state left on Recording would
        // let it announce finished() for a recording this call is reporting as
        // failed — with an empty file under the name of a note nobody made.
        m_state = State::Idle;
        m_source.reset();
        m_recorder.stop();
        removeFile();
        m_fileName.clear();
        return false;
    }
    connect(stream, &QIODevice::readyRead, this, [this, stream] {
        encode(QAudioBuffer(stream->readAll(), m_format));
    });
    // A microphone that is unplugged stops delivering and says so nowhere
    // else: readyRead simply never comes again, and without this the window
    // would keep a still time display and save the torso on Ctrl+Enter.
    connect(m_source.get(), &QAudioSource::stateChanged, this, [this](QAudio::State state) {
        if (state == QAudio::StoppedState && m_source && m_source->error() != QAudio::NoError) {
            handleRecorderError(QStringLiteral("The audio input device stopped delivering"));
        }
    });
    return true;
}

void AudioRecorder::encode(const QAudioBuffer &buffer)
{
    if (m_state != State::Recording || !buffer.isValid() || buffer.frameCount() == 0) {
        return;
    }
    m_pending.append(buffer);
    m_pendingFrames += buffer.frameCount();
    // ponytail: one fixed bound of two seconds for the queue, in place of
    // backpressure onto the source. Past it the encoder is not behind, it is
    // stuck, and a queue that grows for a quarter of an hour is worse than a
    // gap that says so. Ceiling: on a machine where the encoder regularly
    // falls two seconds behind, the buffers dropped here are lost audio. The
    // way up is QAudioSource::suspend() while the queue is full and resume()
    // on readyToSendAudioBuffer — measure before building it. Not stop() and
    // start(): start() hands back a **new** QIODevice, the readyRead
    // connection in start() hangs on the old one, and after the first restart
    // no buffer would arrive again with nothing saying so. And not reset()
    // either — that stops and throws the device buffer away, which is the
    // opposite of what this queue is for.
    while (m_pendingFrames > 2LL * SampleRate) {
        m_pendingFrames -= m_pending.takeFirst().frameCount();
        qWarning("The encoder is not keeping up; an audio buffer was dropped");
    }
    flush();
}

void AudioRecorder::flush()
{
    // The encoder takes one buffer at a time and says so by refusing the next:
    // measured 2026-08-28, three of ten in a check that feeds faster than real
    // time. A refused buffer waits here for readyToSendAudioBuffer instead of
    // being lost — a recording short of a stretch is a fault nobody sees, and
    // sendAudioBuffer() is the only place it would show.
    while (!m_pending.isEmpty() && m_input.sendAudioBuffer(m_pending.constFirst())) {
        const qint64 frames = m_pending.takeFirst().frameCount();
        m_pendingFrames -= frames;
        m_frames += frames;
    }

    if (m_state == State::Stopping && m_pending.isEmpty()) {
        // The tail of the recording has gone in; only now may the muxer close.
        m_recorder.stop();
        return;
    }

    if (m_state != State::Recording || m_maximumDuration <= 0) {
        return;
    }
    // Both clocks, and the first one to run out ends it. The frame count says
    // how much audio is in the file; the elapsed one is the bound SPEC 4 asks
    // for, and it is the only one that still moves when the encoder does not.
    if (duration() >= m_maximumDuration || m_elapsed.elapsed() >= m_maximumDuration) {
        stop();
    }
}

void AudioRecorder::closeDevice()
{
    if (!m_source) {
        return;
    }
    // Released first, and that order is the whole point: stop() emits
    // QAudioSource::stateChanged **synchronously**, the slot on it can end in
    // handleRecorderError(), and that calls this function again. With the
    // member already empty the second call returns at the guard above; the
    // other way round it would run to the end and the outer line would then
    // call deleteLater() on a null pointer.
    //
    // deleteLater() rather than delete: this also runs inside the readyRead
    // slot of the device the source owns when the limit ends the recording,
    // and deleting the sender's owner underneath it would take the running
    // signal emission with it.
    QAudioSource *source = m_source.release();
    source->stop();
    source->deleteLater();
}

void AudioRecorder::stop()
{
    if (m_state != State::Recording) {
        return;
    }
    m_state = State::Stopping;
    closeDevice();

    // The queue is the end of the recording, not surplus: it holds what the
    // encoder had not taken yet, and the last buffer of all is the one lying
    // in it. flush() closes the muxer as soon as it is empty.
    flush();
    if (m_state != State::Stopping || m_pending.isEmpty()) {
        return;
    }
    // And it does not wait for ever: the same two seconds, and the same
    // ceiling as in encode() above.
    QTimer::singleShot(2000, this, [this, generation = m_generation] {
        // The number, not the state: a recording that is already the second
        // one can stand in Stopping just as this one does, and this wakeup
        // would then throw away **its** tail and warn with its milliseconds.
        if (m_generation != generation || m_state != State::Stopping) {
            return;
        }
        if (!m_pending.isEmpty()) {
            qWarning("The encoder did not take the last %lld ms of the recording",
                     m_pendingFrames * 1000 / SampleRate);
            m_pending.clear();
            m_pendingFrames = 0;
        }
        m_recorder.stop();
    });
}

void AudioRecorder::cancel()
{
    // Stopping too, and not only Recording: stop() returns while the muxer is
    // still closing, so Esc right after Ctrl+Enter arrives in that state. It
    // discarded silently before — the recording was saved and cancelled()
    // never came. m_recorder.stop() below is then the second call and a
    // no-op; what changes is the state handleRecorderState() reads.
    if (m_state != State::Recording && m_state != State::Stopping) {
        return;
    }
    m_state = State::Cancelling;
    m_pending.clear();
    m_pendingFrames = 0;
    closeDevice();
    m_recorder.stop();
}

void AudioRecorder::handleRecorderError(const QString &message)
{
    m_lastError = message;
    qWarning("Recording failed: %s", qUtf8Printable(message));
    if (m_state == State::Idle) {
        return;
    }
    // Idle first: whatever the recorder does on its own way out must not come
    // back through handleRecorderState() as a finished recording.
    m_state = State::Idle;
    m_pending.clear();
    m_pendingFrames = 0;
    closeDevice();
    m_recorder.stop();
    // The torso goes with it. No note is ever made out of a failed recording,
    // so what stayed would be an orphan — and on the Esc path, where the
    // failure arrives while the recording is already being discarded, the
    // deletion in handleRecorderState() no longer runs at all.
    removeFile();
    m_fileName.clear();
    Q_EMIT failed(message);
}

void AudioRecorder::handleRecorderState(QMediaRecorder::RecorderState state)
{
    if (state != QMediaRecorder::StoppedState) {
        return;
    }
    // The muxer writes its last page on the way into this state. Before it the
    // file is neither complete nor safe to delete. A recorder that stops in
    // any other state — after a failure, or after a start that never got its
    // device — owes nobody an answer any more and says nothing.
    if (m_state == State::Cancelling) {
        m_state = State::Idle;
        removeFile();
        m_fileName.clear();
        m_frames = 0;
        Q_EMIT cancelled();
        return;
    }
    if (m_state == State::Stopping) {
        m_state = State::Idle;
        Q_EMIT finished(m_fileName, static_cast<int>(duration() / 1000));
    }
}

bool AudioRecorder::isRecording() const
{
    return m_state == State::Recording;
}

qint64 AudioRecorder::duration() const
{
    return m_frames * 1000 / SampleRate;
}

QString AudioRecorder::fileName() const
{
    return m_fileName;
}

QString AudioRecorder::lastError() const
{
    return m_lastError;
}
