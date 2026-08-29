#include "transcribe/modeldownload.h"

#include "transcribe/transcriber.h"

#include <KLocalizedString>

#include <QDir>
#include <QFileInfo>
#include <QLatin1StringView>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>

#include <algorithm>
#include <array>
#include <chrono>

namespace
{
/** What the upstream `models/README.md` says about one of `whisper::Sizes`. */
struct Model {
    /** The SHA-1 of the file, as SPEC 12 names it — no checksum of ours. */
    QLatin1StringView sha1;
    /**
     * How long the file is.
     *
     * Read off the answer of the address below on 29.08.2026 (`x-linked-size`
     * of an HTTP HEAD), because the question the user is asked before a
     * download names the size and is asked before anything has been fetched.
     */
    qint64 bytes;
};

/**
 * The five of `whisper::Sizes`, in that order — the same order a written
 * `denkzettelrc` hangs on, and the static_assert below is what keeps the two
 * lists the same length.
 */
constexpr std::array<Model, whisper::Sizes.size()> Models{{
    {QLatin1StringView("bd577a113a864445d4c299885e0cb97d4ba92b5f"), 77691713},
    {QLatin1StringView("465707469ff3a37a2b9b8d8f89f2f99de7299dac"), 147951465},
    {QLatin1StringView("55356645c2b361a969dfd0ef2c5a50d530afd8d5"), 487601967},
    {QLatin1StringView("fd9727b6e1217c2f614f9b698455c4ffd82463b4"), 1533763059},
    {QLatin1StringView("ad82bf6a9043ceed055076d0fd39f5f186ff8062"), 3095033483},
}};

static_assert(Models.size() == whisper::Sizes.size(),
              "every model size of SPEC 12 carries its checksum and its length");

/** Which of `whisper::Sizes` this is, or -1 for a name from nowhere. */
int indexOf(const QString &size)
{
    const auto found = std::find(whisper::Sizes.cbegin(), whisper::Sizes.cend(), size);
    if (found == whisper::Sizes.cend()) {
        return -1;
    }
    return static_cast<int>(std::distance(whisper::Sizes.cbegin(), found));
}

/** The directory Transcriber::modelPath() puts the models in. */
QDir modelDirectory()
{
    return QFileInfo(Transcriber::modelPath(QStringLiteral("tiny"))).absoluteDir();
}
}

ModelDownload::ModelDownload(QObject *parent)
    : QObject(parent)
{
}

ModelDownload::~ModelDownload() = default;

QUrl ModelDownload::sourceFor(const QString &size)
{
    if (indexOf(size) < 0) {
        return {};
    }
    // The address of the upstream `models/download-ggml-model.sh` — `src` and
    // `pfx` of that script, put together. Not one of ours and not guessed:
    // whoever changes it reads the script again.
    return QUrl(QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-%1.bin")
                    .arg(size));
}

QString ModelDownload::checksumFor(const QString &size)
{
    const int index = indexOf(size);
    return index < 0 ? QString() : QString(Models.at(index).sha1);
}

qint64 ModelDownload::bytesFor(const QString &size)
{
    const int index = indexOf(size);
    return index < 0 ? 0 : Models.at(index).bytes;
}

bool ModelDownload::isRunning() const
{
    return m_reply != nullptr;
}

QString ModelDownload::runningSize() const
{
    return isRunning() ? m_size : QString();
}

void ModelDownload::start(const QString &size)
{
    start(size, sourceFor(size), checksumFor(size));
}

void ModelDownload::start(const QString &size, const QUrl &from, const QString &sha1)
{
    if (isRunning()) {
        return;
    }
    if (from.isEmpty() || sha1.isEmpty()) {
        // A size this program does not know: sourceFor() and checksumFor()
        // answer nothing for it, and without the guard the run would open a
        // QSaveFile under `ggml-<nonsense>.bin` and send a GET on an empty
        // address.
        Q_EMIT finished(size, i18n("%1 is no model size known here", size));
        return;
    }

    m_size = size;
    m_expected = sha1;

    const QDir directory = modelDirectory();
    if (!directory.mkpath(QStringLiteral("."))) {
        Q_EMIT finished(size, i18n("%1 could not be created", directory.absolutePath()));
        return;
    }

    m_file = std::make_unique<QSaveFile>(Transcriber::modelPath(size));
    if (!m_file->open(QIODevice::WriteOnly)) {
        const QString reason = m_file->errorString();
        m_file.reset();
        Q_EMIT finished(size, reason);
        return;
    }
    m_hash.reset();
    m_received = 0;

    QNetworkRequest request(from);
    // Qt's own limit and not a timer of ours: it measures a stretch WITHOUT
    // transfer, which is what a dead connection looks like. A slow line is not
    // one — it keeps delivering — and a big model over one is exactly what
    // this must not cut off.
    request.setTransferTimeout(std::chrono::seconds(30));
    m_reply = m_network.get(request);

    connect(m_reply, &QNetworkReply::readyRead, this, [this] {
        collect(m_reply);
    });
    connect(m_reply, &QNetworkReply::finished, this, &ModelDownload::complete);
}

void ModelDownload::letGoOfTheReply()
{
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;
    // Disconnected before it is aborted, and handed to the event loop rather
    // than deleted: abort() emits the reply's finished() from inside itself,
    // so complete() would run on abort()'s own stack and report a second time
    // — and if anything hanging on our finished() spins an event loop of its
    // own (a modal dialog, a wait in a picture run), the deleteLater() below
    // is executed **while abort() is still running**. Measured 29.08.2026:
    // SIGSEGV in QObjectPrivate::maybeSignalConnected under
    // QNetworkReply::abort(), reached through the cancel button of the
    // settings page; and two finished() out of one full disk, the second one
    // with an empty size.
    reply->disconnect(this);
    reply->abort();
    reply->deleteLater();
}

void ModelDownload::collect(QNetworkReply *reply)
{
    const QByteArray chunk = reply->readAll();
    m_hash.addData(chunk);
    if (m_file->write(chunk) != chunk.size()) {
        // A disk that is full, and it has to be noticed here: commit() would
        // report it too, but only after the whole model has been pulled over
        // the line for nothing.
        const QString reason = m_file->errorString();
        if (m_reply != nullptr) {
            letGoOfTheReply();
        }
        giveUp(reason);
        return;
    }

    // Counted out of what is written rather than taken from
    // QNetworkReply::downloadProgress, and the reason is what that signal is:
    // a report every 100 ms, not one per chunk. Against the real address it
    // came 21 times in 3 seconds while readyRead delivered 15.4 MB — but a
    // body that arrives inside one such window fires it **once, at 100 %**,
    // which is what the loopback stand-in of `transcribetest` does and what a
    // fast line does to a small model. A progress line hung on it would then
    // show nothing but its own end. Both measured 29.08.2026 with Qt 6.11.2.
    m_received += chunk.size();
    Q_EMIT progress(m_received, reply->header(QNetworkRequest::ContentLengthHeader).toLongLong());
}

void ModelDownload::complete()
{
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;
    reply->deleteLater();

    if (m_file == nullptr) {
        // collect() has already given up on this one and said why.
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        giveUp(reply->errorString());
        return;
    }

    collect(reply);
    if (m_file == nullptr) {
        return;
    }

    // The SHA-1 of SPEC 12, and it is what says the file is the model: a
    // truncated body, a proxy's error page and a login form all arrive as a
    // successful transfer.
    const QString written = QString::fromLatin1(m_hash.result().toHex());
    if (written != m_expected) {
        giveUp(i18n("what arrived is not the model %1", m_size));
        return;
    }

    if (!m_file->commit()) {
        giveUp(m_file->errorString());
        return;
    }

    const QString size = m_size;
    m_file.reset();
    m_size.clear();
    Q_EMIT finished(size, QString());
}

void ModelDownload::giveUp(const QString &reason)
{
    if (m_file != nullptr) {
        // Both, and in this order: cancelWriting() marks the save as given up
        // on, the destructor takes the temporary file away with it.
        m_file->cancelWriting();
        m_file.reset();
    }

    const QString size = m_size;
    m_size.clear();
    Q_EMIT finished(size, reason);
}

void ModelDownload::cancel()
{
    if (!isRunning()) {
        return;
    }

    // The reply first, the report after it — see letGoOfTheReply(). The
    // sentence is ours rather than the reply's error value because what
    // happened here is known without asking anybody.
    letGoOfTheReply();
    giveUp(QStringLiteral("cancelled"));
}
