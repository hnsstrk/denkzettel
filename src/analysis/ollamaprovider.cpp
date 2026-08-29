#include "analysis/ollamaprovider.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace
{
/**
 * Whether a second attempt can come out differently from the first.
 *
 * ponytail: transport-level failures and the timeout, nothing the server said.
 * An answer Ollama gave — a model that is not pulled, a malformed request —
 * comes out the same on the second try, and the retry would only double the
 * wait before the user is told. The ceiling is a genuinely transient HTTP 5xx,
 * which is not retried here; the way up is to add the status to this decision
 * once such a case has been seen.
 */
bool isWorthRetrying(QNetworkReply::NetworkError transport)
{
    switch (transport) {
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::RemoteHostClosedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
    case QNetworkReply::OperationCanceledError:
    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::NetworkSessionFailedError:
    case QNetworkReply::UnknownNetworkError:
        return true;
    default:
        return false;
    }
}
}

OllamaAnswer readOllamaReply(OllamaCall call,
                             QNetworkReply::NetworkError transport,
                             const QString &transportMessage,
                             int httpStatus,
                             const QByteArray &body)
{
    // The timeout of SPEC 7.1, and it arrives as TimeoutError — measured on
    // 2026-08-29 against a real Ollama with the limit set to 5 ms. Qt's
    // documentation of setTransferTimeout names OperationCanceledError, which
    // an abort of ours would produce; both are the same statement to the user,
    // and mapping only the documented one left this case falling through to
    // "could not be reached: Zeitüberschreitung".
    if (transport == QNetworkReply::TimeoutError || transport == QNetworkReply::OperationCanceledError) {
        return {{}, {}, i18n("Ollama did not answer within the time limit.")};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    const QJsonObject answer = document.object();

    // What Ollama says about itself beats what the status code says about the
    // transfer: a model that is not pulled arrives as an HTTP 404 whose body
    // names the model.
    const QString refusal = answer.value(QLatin1String("error")).toString();
    if (!refusal.isEmpty()) {
        return {{}, {}, i18n("Ollama refused the request: %1", refusal)};
    }

    if (transport != QNetworkReply::NoError && httpStatus == 0) {
        return {{}, {}, i18n("Ollama could not be reached: %1", transportMessage)};
    }

    if (httpStatus != 0 && (httpStatus < 200 || httpStatus > 299)) {
        return {{}, {}, i18n("Ollama answered with HTTP status %1.", httpStatus)};
    }

    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {{}, {}, i18n("Ollama sent an unreadable answer: %1", parseError.errorString())};
    }

    if (call == OllamaCall::Chat) {
        const QString text = answer.value(QLatin1String("message")).toObject().value(QLatin1String("content")).toString();
        if (text.isEmpty()) {
            return {{}, {}, i18n("Ollama's answer carried no text.")};
        }
        return {text, {}, {}};
    }

    // `/api/embed` takes a list of inputs and answers with a list of vectors.
    // One text goes in, so the first one is the one asked for.
    const QJsonArray vectors = answer.value(QLatin1String("embeddings")).toArray();
    const QJsonArray first = vectors.isEmpty() ? QJsonArray() : vectors.first().toArray();
    if (first.isEmpty()) {
        return {{}, {}, i18n("Ollama's answer carried no embedding.")};
    }

    QList<double> vector;
    vector.reserve(first.size());
    for (const auto &component : first) {
        vector.append(component.toDouble());
    }
    return {{}, vector, {}};
}

OllamaProvider::OllamaProvider(QObject *parent)
    : AiProvider(parent)
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    m_url = QUrl(group.readEntry("OllamaUrl", QStringLiteral("http://localhost:11434")));
    m_chatModel = group.readEntry("ChatModel", QStringLiteral("qwen3:8b"));
    m_embeddingModel = group.readEntry("EmbeddingModel", QStringLiteral("bge-m3"));
}

void OllamaProvider::setUrl(const QUrl &url)
{
    m_url = url;
}

void OllamaProvider::setChatModel(const QString &model)
{
    m_chatModel = model;
}

QString OllamaProvider::chatModel() const
{
    return m_chatModel;
}

void OllamaProvider::setEmbeddingModel(const QString &model)
{
    m_embeddingModel = model;
}

QString OllamaProvider::embeddingModel() const
{
    return m_embeddingModel;
}

void OllamaProvider::setTimeout(std::chrono::milliseconds timeout)
{
    m_timeout = timeout;
}

int OllamaProvider::chat(const QString &prompt)
{
    const QJsonObject message{
        {QLatin1String("role"), QLatin1String("user")},
        {QLatin1String("content"), prompt},
    };
    const QJsonObject body{
        {QLatin1String("model"), m_chatModel},
        {QLatin1String("messages"), QJsonArray{message}},
        {QLatin1String("stream"), false},
    };

    const int id = nextRequestId();
    send(id, OllamaCall::Chat, QStringLiteral("/api/chat"), body, 0);
    return id;
}

int OllamaProvider::embed(const QString &text)
{
    const QJsonObject body{
        {QLatin1String("model"), m_embeddingModel},
        {QLatin1String("input"), text},
    };

    const int id = nextRequestId();
    send(id, OllamaCall::Embed, QStringLiteral("/api/embed"), body, 0);
    return id;
}

void OllamaProvider::send(int id, OllamaCall call, const QString &path, const QJsonObject &body, int attempt)
{
    QNetworkRequest request(m_url.resolved(QUrl(path)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    // Qt's own limit rather than a QTimer of ours (SPEC 7.1). It measures a
    // stretch without transfer activity, which for `stream: false` is the whole
    // answer: Ollama sends nothing until the model is done.
    request.setTransferTimeout(m_timeout);

    QNetworkReply *reply = m_network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, id, call, path, body, attempt] {
        reply->deleteLater();
        const OllamaAnswer answer = readOllamaReply(call,
                                                    reply->error(),
                                                    reply->errorString(),
                                                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                                                    // An aborted reply is closed, and reading it anyway
                                                    // only earns a "device not open" from QIODevice.
                                                    reply->isOpen() ? reply->readAll() : QByteArray());

        if (!answer.error.isEmpty() && attempt == 0 && isWorthRetrying(reply->error())) {
            send(id, call, path, body, attempt + 1);
            return;
        }

        if (call == OllamaCall::Chat) {
            Q_EMIT chatFinished(id, answer.text, answer.error);
        } else {
            Q_EMIT embedFinished(id, answer.vector, answer.error);
        }
    });
}
