#include "analysis/ollamaprovider.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

// No namespace of its own any more: the decision which errors are worth
// repeating used to live here, and every one of its entries fell to a
// measurement (issue #13, 2026-08-29). What is left is in send() below.

OllamaAnswer readOllamaReply(OllamaCall call,
                             QNetworkReply::NetworkError transport,
                             const QString &transportMessage,
                             int httpStatus,
                             const QByteArray &body)
{
    // The timeout of SPEC 7.1, and it arrives as TimeoutError — measured on
    // 2026-08-29 against a real Ollama with the limit set to 5 ms. Mapping only
    // the OperationCanceledError that Qt's documentation of setTransferTimeout
    // names left this case falling through to "could not be reached:
    // Zeitüberschreitung", the transport's sentence over a limit of ours.
    //
    // OperationCanceledError stays beside it and **nothing in this program
    // reaches it today**: it takes an abort() or a close() on a running reply,
    // and there is none in the tree. It is not asserted anywhere for that
    // reason (CLAUDE.md, finding 40). What would reach it is a cancel button on
    // a running analysis run, or a Qt that starts doing what its own
    // documentation says — and either way the sentence is the right one.
    if (transport == QNetworkReply::TimeoutError || transport == QNetworkReply::OperationCanceledError) {
        return {{}, {}, i18n("Ollama did not answer within the time limit."), AiFailure::Unreachable};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    const QJsonObject answer = document.object();

    // What Ollama says about itself beats what the status code says about the
    // transfer: a model that is not pulled arrives as an HTTP 404 whose body
    // names the model.
    const QString refusal = answer.value(QLatin1String("error")).toString();
    if (!refusal.isEmpty()) {
        return {{}, {}, i18n("Ollama refused the request: %1", refusal), AiFailure::Refused};
    }

    if (transport != QNetworkReply::NoError && httpStatus == 0) {
        return {{}, {}, i18n("Ollama could not be reached: %1", transportMessage), AiFailure::Unreachable};
    }

    if (httpStatus != 0 && (httpStatus < 200 || httpStatus > 299)) {
        return {{}, {}, i18n("Ollama answered with HTTP status %1.", httpStatus), AiFailure::Refused};
    }

    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {{}, {}, i18n("Ollama sent an unreadable answer: %1", parseError.errorString()), AiFailure::Refused};
    }

    if (call == OllamaCall::Chat) {
        const QString text = answer.value(QLatin1String("message")).toObject().value(QLatin1String("content")).toString();
        if (text.isEmpty()) {
            return {{}, {}, i18n("Ollama's answer carried no text."), AiFailure::Refused};
        }
        return {text, {}, {}};
    }

    // `/api/embed` takes a list of inputs and answers with a list of vectors.
    // One text goes in, so the first one is the one asked for.
    const QJsonArray vectors = answer.value(QLatin1String("embeddings")).toArray();
    const QJsonArray first = vectors.isEmpty() ? QJsonArray() : vectors.first().toArray();
    if (first.isEmpty()) {
        return {{}, {}, i18n("Ollama's answer carried no embedding."), AiFailure::Refused};
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
    m_url = QUrl(group.readEntry("OllamaUrl", QString(ollama::DefaultUrl)));
    m_chatModel = group.readEntry("ChatModel", QString(ollama::DefaultChatModel));
    m_embeddingModel = group.readEntry("EmbeddingModel", QString(ollama::DefaultEmbeddingModel));
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
    send(id, OllamaCall::Chat, QStringLiteral("/api/chat"), body);
    return id;
}

int OllamaProvider::embed(const QString &text)
{
    const QJsonObject body{
        {QLatin1String("model"), m_embeddingModel},
        {QLatin1String("input"), text},
    };

    const int id = nextRequestId();
    send(id, OllamaCall::Embed, QStringLiteral("/api/embed"), body);
    return id;
}

void OllamaProvider::testReachability()
{
    QNetworkRequest request(m_url.resolved(QUrl(QStringLiteral("/api/tags"))));
    request.setTransferTimeout(m_timeout);

    QNetworkReply *reply = m_network.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        // Only the transport is asked, and readOllamaReply() is deliberately
        // not called on it: that function reads an answer of `/api/chat` or
        // `/api/embed` and would call the list of models "an answer that
        // carried no text". A server that says anything at all — a status, a
        // refusal, a body of its own — is a server that is there, and that is
        // the whole of the question this endpoint is asked (issue #17).
        Q_EMIT reachabilityTested(reply->error() == QNetworkReply::NoError);
    });
}

void OllamaProvider::send(int id, OllamaCall call, const QString &path, const QJsonObject &body)
{
    QNetworkRequest request(m_url.resolved(QUrl(path)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    // Qt's own limit rather than a QTimer of ours (SPEC 7.1). It measures a
    // stretch without transfer activity, which for `stream: false` is the whole
    // answer: Ollama sends nothing until the model is done.
    request.setTransferTimeout(m_timeout);

    QNetworkReply *reply = m_network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, id, call] {
        reply->deleteLater();
        const OllamaAnswer answer = readOllamaReply(call,
                                                    reply->error(),
                                                    reply->errorString(),
                                                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                                                    // An aborted reply is closed, and reading it anyway
                                                    // only earns a "device not open" from QIODevice.
                                                    reply->isOpen() ? reply->readAll() : QByteArray());

        if (call == OllamaCall::Chat) {
            Q_EMIT chatFinished(id, answer.text, answer.error);
        } else {
            Q_EMIT embedFinished(id, answer.vector, answer.error, answer.failure);
        }
    });
}
