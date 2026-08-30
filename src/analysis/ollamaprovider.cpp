#include "analysis/ollamaprovider.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include <utility>

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
    // 2026-08-29 against a real Ollama with the limit set to 5 ms, and again on
    // 2026-08-30 with the limit at 5 s, where the same call came back
    // `TimeoutError (4)` after 5,260 ms. Mapping only
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

    // `/api/chat` answers with one JSON object per line (`stream: true`, see
    // chat() below), `/api/embed` with a single one — and a single one is a
    // single line, so both roads run through the same split. `trimmed()` takes
    // the closing newline off, and it leaves an empty body an empty line: that
    // way an answer of nothing keeps reaching the "unreadable" case below with
    // a parser error to name, rather than with no error at all.
    QJsonParseError parseError;
    QList<QJsonObject> answers;
    const QList<QByteArray> lines = body.trimmed().split('\n');
    for (const QByteArray &line : lines) {
        const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            answers.clear();
            break;
        }
        answers.append(document.object());
    }

    // What Ollama says about itself beats what the status code says about the
    // transfer: a model that is not pulled arrives as an HTTP 404 whose body
    // names the model. Every line is asked, because a stream that goes wrong
    // half way through carries its reason in the last one.
    for (const QJsonObject &answer : std::as_const(answers)) {
        const QString refusal = answer.value(QLatin1String("error")).toString();
        if (!refusal.isEmpty()) {
            return {{}, {}, i18n("Ollama refused the request: %1", refusal), AiFailure::Refused};
        }
    }

    if (transport != QNetworkReply::NoError && httpStatus == 0) {
        return {{}, {}, i18n("Ollama could not be reached: %1", transportMessage), AiFailure::Unreachable};
    }

    if (httpStatus != 0 && (httpStatus < 200 || httpStatus > 299)) {
        return {{}, {}, i18n("Ollama answered with HTTP status %1.", httpStatus), AiFailure::Refused};
    }

    if (answers.isEmpty()) {
        return {{}, {}, i18n("Ollama sent an unreadable answer: %1", parseError.errorString()), AiFailure::Refused};
    }

    if (call == OllamaCall::Chat) {
        // The answer of a streamed call is its lines put back together, and
        // `content` is the only field taken: what a thinking model reasons
        // stands in `thinking` beside it (CLAUDE.md, finding 45) and is no more
        // part of the answer here than it was before the stream.
        QString text;
        for (const QJsonObject &answer : std::as_const(answers)) {
            text += answer.value(QLatin1String("message")).toObject().value(QLatin1String("content")).toString();
        }
        if (text.isEmpty()) {
            return {{}, {}, i18n("Ollama's answer carried no text."), AiFailure::Refused};
        }
        return {text, {}, {}};
    }

    // `/api/embed` takes a list of inputs and answers with a list of vectors.
    // One text goes in, so the first one is the one asked for. It does not
    // stream, so there is the one answer.
    const QJsonArray vectors = answers.constFirst().value(QLatin1String("embeddings")).toArray();
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

QString ollama::configuredEmbeddingModel()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    return group.readEntry("EmbeddingModel", QString(ollama::DefaultEmbeddingModel));
}

OllamaProvider::OllamaProvider(QObject *parent)
    : AiProvider(parent)
{
    reloadSettings();
}

void OllamaProvider::reloadSettings()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    m_url = QUrl(group.readEntry("OllamaUrl", QString(ollama::DefaultUrl)));
    m_chatModel = group.readEntry("ChatModel", QString(ollama::DefaultChatModel));
    m_embeddingModel = ollama::configuredEmbeddingModel();
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
        // **Streamed, and that is what makes the limit of SPEC 7.1 a limit on
        // silence rather than on thinking** (issue #121). With `stream: false`
        // Ollama sends nothing until the answer is finished, so the model load
        // and the whole generation fell into one 30 s budget and the first note
        // after a start paid an attempt for a cold server. Measured on
        // 2026-08-30 against Ollama 0.32.15 and `qwen3:8b`: loading the model
        // costs 2.3 s of silence with the page cache dropped (6.5 s for an
        // 18 GB model). The load is the smaller half — the same note through
        // this same prompt took 18.1 s, 46.9 s, 22.5 s and 45.7 s in four warm
        // runs, because how long a thinking model reasons is no property of
        // the note (2,565 to 15,054 characters of `thinking`). Two of those
        // four were over the limit, so unstreamed it failed notes by a coin
        // toss.
        //
        // That Qt's transfer timeout really starts again with every chunk was
        // measured, not read (CLAUDE.md, findings 17 and 40): with the limit at
        // 5 s the streamed call ran 12.7 s and came back `NoError`, first byte
        // after 2,511 ms, while the same call unstreamed died at 5,260 ms with
        // `TimeoutError`.
        {QLatin1String("stream"), true},
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
    // stretch without transfer activity, and since the chat call streams that
    // is what it says: 30 s in which Ollama sent nothing at all.
    //
    // ponytail: the model load is the one silent stretch left, so a load of
    // over 30 s still costs the note an attempt. Measured at 2.3 s and 6.5 s
    // here (see chat()); a machine slow enough to break that is the upgrade
    // path — the classification run then reads AiFailure the way Embedder does
    // and stops the run instead of counting, which is road 2 of issue #121.
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
