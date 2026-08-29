#pragma once

#include "analysis/aiprovider.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QUrl>

#include <chrono>
#include <cstdint>

/** Which of the two endpoints of SPEC 7.1 a reply belongs to. */
enum class OllamaCall : std::uint8_t {
    Chat,
    Embed,
};

/** What a finished Ollama reply says: the value, or the reason there is none. */
struct OllamaAnswer {
    /** `/api/chat`: what the assistant wrote. Empty for an embedding. */
    QString text;
    /** `/api/embed`: the one vector asked for. Empty for a chat. */
    QList<double> vector;
    /** Empty exactly when the call carried an answer. */
    QString error;
};

/**
 * The one place a reply becomes an answer or a comprehensible error, and it
 * reads the body rather than believing the status (issue #13).
 *
 * A pure function and not a method, so the mapping can be checked without a
 * server: every case below is one an `aitest` function hands in directly.
 * The order the cases are tried in is the order of what the user needs told:
 *
 * 1. **The transfer ran out of time** — the timeout of SPEC 7.1 bit. It
 *    arrives as `TimeoutError`, measured, and as `OperationCanceledError` in
 *    Qt's documentation of `setTransferTimeout`; both mean the same thing here.
 * 2. **Ollama itself refused**, and then its own sentence is the best one
 *    there is: a model that is not pulled arrives as HTTP 404 with
 *    `{"error":"model \"x\" not found"}`, and "HTTP 404" alone would leave the
 *    user looking for a network fault.
 * 3. **The connection failed** — no answer, so `transportMessage` carries it.
 * 4. **An HTTP status that is not a success**, with nothing readable in the
 *    body to say why.
 * 5. **The body is not JSON**, which is what a proxy or a wrong port answering
 *    with HTML looks like.
 * 6. **The JSON is fine and the field is missing** — the case a return value
 *    of `true` would have hidden.
 */
OllamaAnswer readOllamaReply(OllamaCall call,
                             QNetworkReply::NetworkError transport,
                             const QString &transportMessage,
                             int httpStatus,
                             const QByteArray &body);

/**
 * Ollama over its HTTP API (SPEC 7.1): `/api/chat` and `/api/embed`.
 *
 * The address and the two models are settings out of `denkzettelrc` (SPEC
 * 5.2), read at construction and settable afterwards — the settings dialog of
 * SPEC 13 writes them, and a check points them at a port that is not listening.
 *
 * Non-streaming on purpose: SPEC 7.2 wants one JSON document per note, not a
 * token trickle, and with `stream: false` the transfer timeout below is a
 * deadline for the whole answer.
 */
class OllamaProvider : public AiProvider
{
    Q_OBJECT

public:
    explicit OllamaProvider(QObject *parent = nullptr);

    /** `http://localhost:11434` unless `denkzettelrc` says otherwise. */
    void setUrl(const QUrl &url);
    /** SPEC 7.1: `qwen3:8b`. */
    void setChatModel(const QString &model);
    QString chatModel() const;
    /** SPEC 7.1: `bge-m3`, and in v1 every embedding comes from here. */
    void setEmbeddingModel(const QString &model);
    QString embeddingModel() const;

    /**
     * The 30 s of SPEC 7.1, settable because a check that waits half a minute
     * for the limit to bite is one nobody runs.
     */
    void setTimeout(std::chrono::milliseconds timeout);

    int chat(const QString &prompt) override;
    int embed(const QString &text) override;

private:
    /**
     * Posts one request and answers, retrying once where a second attempt can
     * come out differently (SPEC 7.1: "a timeout (30 s) and one retry").
     *
     * `attempt` counts from 0. Everything the retry needs travels in the
     * arguments, so no request is held anywhere: a reply that outlives its
     * provider is deleted with it.
     */
    void send(int id, OllamaCall call, const QString &path, const QJsonObject &body, int attempt);

    QNetworkAccessManager m_network;
    QUrl m_url;
    QString m_chatModel;
    QString m_embeddingModel;
    std::chrono::milliseconds m_timeout = std::chrono::seconds(30);
};
