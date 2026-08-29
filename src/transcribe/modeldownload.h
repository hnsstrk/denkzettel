#pragma once

#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>

#include <memory>

class QNetworkReply;
class QSaveFile;

/**
 * Fetches one GGML model of SPEC 12 into
 * `~/.local/share/denkzettel/models/ggml-<size>.bin`.
 *
 * **One download at a time, and it outlives the settings dialog.** The daemon
 * holds one of these for the whole session and hands it to the dialog, the way
 * it hands over the shortcuts: a file of gigabytes must not be thrown away
 * because a window was closed, and the only way to stop it is the button the
 * page shows while it runs (UX decision, 29.08.2026).
 *
 * **Nothing half-written can survive this class.** The bytes go into a
 * QSaveFile, which writes beside the model and renames onto it in one step —
 * so a cancel, a broken connection and a killed daemon leave the same state,
 * and it is the state before the download. The model file itself only ever
 * comes into being complete, and only after the SHA-1 of SPEC 12 has agreed:
 * without that check a proxy's error page, an HTML login form or a truncated
 * body would be renamed onto the model and whisper.cpp would be the first to
 * notice.
 *
 * A killed daemon leaves nothing either, and that was measured rather than
 * assumed (29.08.2026): while a download runs, the models directory holds no
 * new file at all — QSaveFile opens its temporary through `O_TMPFILE`, an
 * inode with no name in any directory, and only commit() links it in. The
 * kernel frees it when the process dies.
 *
 * ponytail: no sweep for leftovers. Ceiling — a file system without
 * `O_TMPFILE` makes QSaveFile fall back to a **named** temporary beside the
 * model, and a killed daemon would leave one of those, invisible to everything
 * that looks for the model and as big as it. The way up is to remove
 * `ggml-*.bin.*` in start(); nothing here can measure that case, because the
 * file system under `~/.local/share` on this machine does support O_TMPFILE.
 */
class ModelDownload : public QObject
{
    Q_OBJECT

public:
    explicit ModelDownload(QObject *parent = nullptr);
    ~ModelDownload() override;

    ModelDownload(const ModelDownload &) = delete;
    ModelDownload &operator=(const ModelDownload &) = delete;

    /**
     * Where the model of `size` lies upstream, what its file has to hash to,
     * and how many bytes that is.
     *
     * The address is the one the upstream `models/download-ggml-model.sh`
     * fetches from, the hash and the length are what stands in the upstream
     * `models/README.md` — SPEC 12 says both explicitly, and neither is a
     * checksum of ours. Measured on 29.08.2026: the address answers 302 → 200
     * for every one of the five sizes.
     *
     * The byte count is here because the question asked before a download
     * names it, and that question is asked before a single byte has been
     * fetched — a Content-Length can only answer it once the run is under way.
     * An unknown size answers 0 and an empty string.
     */
    static QUrl sourceFor(const QString &size);
    static QString checksumFor(const QString &size);
    static qint64 bytesFor(const QString &size);

    /** Fetches `size` from sourceFor(size); does nothing while one runs. */
    void start(const QString &size);

    /**
     * The same from another address and against another checksum, which is
     * what the automated run fetches from: the CI has no way out to the
     * internet, and a 466 MB model is not a test fixture. Same reason SPEC 12
     * makes the two program paths settings.
     */
    void start(const QString &size, const QUrl &from, const QString &sha1);

    /** Gives up the running download and leaves nothing of it behind. */
    void cancel();

    bool isRunning() const;
    /** Which size is being fetched, empty when nothing runs. */
    QString runningSize() const;

Q_SIGNALS:
    /**
     * How much of the model is written, and how much the answer says there is
     * — 0 for as long as it has not said.
     *
     * Not QNetworkReply::downloadProgress passed on: that signal did not fire
     * once in a measured transfer of 4.2 MB (Qt 6.11.2, 29.08.2026).
     */
    void progress(qint64 received, qint64 total);

    /**
     * The download is over, one way or another.
     *
     * `error` is empty exactly when the model lies complete under
     * Transcriber::modelPath(size) — a cancelled download reports the sentence
     * for it here rather than through a signal of its own, so the page has one
     * place to put a message and not two.
     */
    void finished(const QString &size, const QString &error);

private:
    void collect(QNetworkReply *reply);
    void complete();
    /** Drops what has been written and reports `reason`. */
    void giveUp(const QString &reason);

    QNetworkAccessManager m_network;
    QNetworkReply *m_reply = nullptr;
    std::unique_ptr<QSaveFile> m_file;
    QCryptographicHash m_hash{QCryptographicHash::Sha1};
    QString m_size;
    QString m_expected;
    /** How much has been written, for the progress — see collect(). */
    qint64 m_received = 0;
};
