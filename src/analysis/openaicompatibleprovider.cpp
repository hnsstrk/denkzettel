#include "analysis/openaicompatibleprovider.h"

#include "store/keystore.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTimer>

#include <utility>

namespace
{
/** The frame prefix of a server-sent event, and the one this reader keeps. */
constexpr QByteArrayView DataPrefix("data:");
/** What closes an OpenAI-compatible stream. */
constexpr QByteArrayView StreamEnd("[DONE]");
}

OpenAiCompatibleAnswer readOpenAiCompatibleReply(QLatin1StringView service,
                                                 QNetworkReply::NetworkError transport,
                                                 const QString &transportMessage,
                                                 int httpStatus,
                                                 const QByteArray &body)
{
    // Once, because every sentence below needs it and i18n() takes a QString.
    const QString name(service);

    if (transport == QNetworkReply::TimeoutError) {
        return {{}, i18n("%1 did not answer within the time limit.", name)};
    }

    // The total limit of SPEC 7.1, and this is what an abort() on a running
    // reply produces — measured on 2026-08-30 against a real server for
    // OllamaProvider, and it is Qt's value rather than the service's, so it
    // carries here (see the header for what does not).
    if (transport == QNetworkReply::OperationCanceledError) {
        return {{}, i18n("%1 took longer over this call than it is allowed.", name)};
    }

    // A refusal that arrives as one JSON document, which is what a request
    // rejected before the stream starts looks like. Its own sentence beats the
    // status code: an unknown model, a spent quota and a rejected key are three
    // different things to the user and one HTTP number.
    //
    // **It is not the only shape a refusal takes** — the review of 30.08.2026
    // measured the other one against this very function: the service can put
    // the error *inside* the stream, as a `data:` frame carrying `error`, and
    // read as an ordinary frame that came out "the answer broke off" or
    // "carried no text" — the service's own sentence lost, which is the whole
    // point of this branch. The frame loop below therefore asks the same
    // question again.
    const QString refusal = QJsonDocument::fromJson(body)
                                .object()
                                .value(QLatin1String("error"))
                                .toObject()
                                .value(QLatin1String("message"))
                                .toString();
    if (!refusal.isEmpty()) {
        return {{}, i18n("%1 refused the request: %2", name, refusal)};
    }

    if (transport != QNetworkReply::NoError && httpStatus == 0) {
        return {{}, i18n("%1 could not be reached: %2", name, transportMessage)};
    }

    if (httpStatus != 0 && (httpStatus < 200 || httpStatus > 299)) {
        return {{}, i18n("%1 answered with HTTP status %2.", name, httpStatus)};
    }

    // The stream. Every frame is a line of its own, so a connection that goes
    // away at a frame boundary leaves a body in which every line parses and the
    // answer is half there — which is why `[DONE]` decides and not the parser
    // (the reasoning of OllamaProvider's `done`, issue #121).
    QString text;
    bool finished = false;
    bool readable = false;
    const QList<QByteArray> lines = body.split('\n');
    for (const QByteArray &raw : lines) {
        const QByteArray line = raw.trimmed();
        // A line beginning with a colon is a comment, and both services send
        // those as a keep-alive while a model thinks — they are what keeps the
        // silence limit above from biting on a slow answer. Every other field
        // of the protocol (`event:`, `id:`, `retry:`) is none of our business.
        if (line.isEmpty() || !line.startsWith(DataPrefix)) {
            continue;
        }
        const QByteArray payload = line.sliced(DataPrefix.size()).trimmed();
        if (payload == StreamEnd) {
            finished = true;
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument frame = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !frame.isObject()) {
            return {{}, i18n("%1 sent an unreadable answer.", name)};
        }
        readable = true;

        // **The service's own sentence, wherever it stands.** A stream that
        // turns into an error mid-way carries it in a frame like any other, and
        // without this the loop would go on collecting empty deltas and end in
        // "broke off" — a sentence about the transport over a refusal the
        // service spelled out (review of 30.08.2026).
        const QString streamed = frame.object()
                                     .value(QLatin1String("error"))
                                     .toObject()
                                     .value(QLatin1String("message"))
                                     .toString();
        if (!streamed.isEmpty()) {
            return {{}, i18n("%1 refused the request: %2", name, streamed)};
        }

        // `delta.content` and not `message.content`: a streamed choice carries
        // the piece, not the whole. What a thinking model reasons stands in
        // `reasoning` beside it and is no part of the answer, the way it is not
        // for Ollama (CLAUDE.md, finding 45).
        const QJsonArray choices = frame.object().value(QLatin1String("choices")).toArray();
        if (!choices.isEmpty()) {
            text += choices.constBegin()
                        ->toObject()
                        .value(QLatin1String("delta"))
                        .toObject()
                        .value(QLatin1String("content"))
                        .toString();
        }
    }

    if (!readable && !finished) {
        return {{}, i18n("%1 sent an unreadable answer.", name)};
    }

    if (!finished) {
        return {{}, i18n("%1's answer broke off.", name)};
    }

    if (text.isEmpty()) {
        return {{}, i18n("%1's answer carried no text.", name)};
    }

    return {text, {}};
}

OpenAiCompatibleEmbedding readOpenAiCompatibleEmbedding(QLatin1StringView service,
                                                        QNetworkReply::NetworkError transport,
                                                        const QString &transportMessage,
                                                        int httpStatus,
                                                        const QByteArray &body)
{
    const QString name(service);

    // The two limits of SPEC 7.1, in the order and with the wording the chat
    // reader above carries them: the silence limit arrives as TimeoutError, and
    // an abort() of ours on a running reply as OperationCanceledError. Both are
    // Unreachable — the note was not refused, the transfer ended — so the run
    // stops instead of spending the note an attempt (aiprovider.h).
    if (transport == QNetworkReply::TimeoutError) {
        return {{}, i18n("%1 did not answer within the time limit.", name), AiFailure::Unreachable};
    }
    if (transport == QNetworkReply::OperationCanceledError) {
        return {{}, i18n("%1 took longer over this call than it is allowed.", name), AiFailure::Unreachable};
    }

    // The service's own sentence beats the status code, for the reason it does
    // above: an unknown model, a spent quota and a rejected key are three
    // different things to the user and one HTTP number. Only one shape here,
    // because this call does not stream and a refusal has nowhere else to
    // stand.
    const QJsonDocument document = QJsonDocument::fromJson(body);
    const QString refusal =
        document.object().value(QLatin1String("error")).toObject().value(QLatin1String("message")).toString();
    if (!refusal.isEmpty()) {
        return {{}, i18n("%1 refused the request: %2", name, refusal), AiFailure::Refused};
    }

    if (transport != QNetworkReply::NoError && httpStatus == 0) {
        return {{}, i18n("%1 could not be reached: %2", name, transportMessage), AiFailure::Unreachable};
    }

    if (httpStatus != 0 && (httpStatus < 200 || httpStatus > 299)) {
        return {{}, i18n("%1 answered with HTTP status %2.", name, httpStatus), AiFailure::Refused};
    }

    if (!document.isObject()) {
        return {{}, i18n("%1 sent an unreadable answer.", name), AiFailure::Refused};
    }

    // `data` is a list because the endpoint takes a list of inputs; one text
    // goes in, so the first entry is the one asked for — the same reading
    // readOllamaReply() gives `/api/embed`.
    const QJsonArray data = document.object().value(QLatin1String("data")).toArray();
    const QJsonArray first =
        data.isEmpty() ? QJsonArray() : data.constBegin()->toObject().value(QLatin1String("embedding")).toArray();
    if (first.isEmpty()) {
        return {{}, i18n("%1's answer carried no embedding.", name), AiFailure::Refused};
    }

    QList<double> vector;
    vector.reserve(first.size());
    for (const auto &component : first) {
        vector.append(component.toDouble());
    }
    return {vector, {}, AiFailure::None};
}

OpenAiCompatibleProvider::OpenAiCompatibleProvider(const AiService &service, QObject *parent)
    : AiProvider(parent)
    , m_service(service)
    , m_url(QString(service.endpoint))
    , m_embedUrl(QString(service.embedEndpoint))
{
    reloadSettings();

    // Attached here and not at the first call: KeyStore opens the wallet on the
    // first *request*, so listening costs nothing and a user who never enters a
    // key is never asked for a wallet password (keystore.h).
    connect(KeyStore::self(),
            &KeyStore::keyRead,
            this,
            // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) - the signature is KeyStore::keyRead
            [this](const QString &provider, const QString &key, const QString &error) {
                // **By this instance's own entry name**, and that is what keeps
                // two of these providers apart: both listen on the one KeyStore,
                // and without this line the OpenAI client would take the
                // openrouter answer for its own — a key on the wrong wire and
                // billed to the wrong account (issue #39).
                if (provider != QLatin1String(m_service.keyName)) {
                    return;
                }
                m_keyAsked = false;

                // **`error` empty means the wallet answered**, and then an empty
                // key is a key that was never stored — not a broken store
                // (keystore.h, and the distinction was blocking twice). The two
                // are one sentence to the user and two different facts here: the
                // second one names what to do about it.
                if (!error.isEmpty()) {
                    releaseWaiting(error);
                    return;
                }
                if (key.isEmpty()) {
                    releaseWaiting(i18n("No API key for %1 is stored."
                                        " Enter it in the settings under \"AI provider\".",
                                        QString(m_service.name)));
                    return;
                }

                m_key = key;
                m_keyKnown = true;
                releaseWaiting(QString());
            });
}

void OpenAiCompatibleProvider::reloadSettings()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    // No default, and the header says why: any model here would make a choice
    // the customer reserved for himself, every 30 minutes and billed. That
    // holds for the embedding model twice over — an embedding run touches
    // **every** note (SPEC 7.1, issue #130).
    m_model = group.readEntry(QString(m_service.modelKey), QString());
    m_embeddingModel = group.readEntry(QString(m_service.embeddingModelKey), QString());
    m_keyKnown = false;
    m_key.clear();
}

void OpenAiCompatibleProvider::setChatModel(const QString &model)
{
    m_model = model;
}

QString OpenAiCompatibleProvider::chatModel() const
{
    return m_model;
}

void OpenAiCompatibleProvider::setEmbeddingModel(const QString &model)
{
    m_embeddingModel = model;
}

QString OpenAiCompatibleProvider::embeddingModel() const
{
    return m_embeddingModel;
}

QString OpenAiCompatibleProvider::serviceId() const
{
    return QString(m_service.id);
}

void OpenAiCompatibleProvider::setKey(const QString &key)
{
    m_key = key;
    m_keyKnown = true;
}

void OpenAiCompatibleProvider::setUrl(const QUrl &url)
{
    m_url = url;
}

void OpenAiCompatibleProvider::setEmbedUrl(const QUrl &url)
{
    m_embedUrl = url;
}

void OpenAiCompatibleProvider::setTimeout(std::chrono::milliseconds timeout)
{
    m_timeout = timeout;
}

void OpenAiCompatibleProvider::setCallLimit(std::chrono::milliseconds limit)
{
    m_callLimit = limit;
}

int OpenAiCompatibleProvider::chat(const QString &prompt)
{
    const int id = nextRequestId();

    // **The guided sentence instead of the transport's** (issue #38): with no
    // model set, a request would go out as `"model":""` and come back as
    // whatever the service makes of that — an HTTP status or a refusal, which
    // sends the user looking at their network for a field they never filled
    // in. Answered here rather than in the connection test alone, because this
    // is the one place every caller routes through.
    const QString missing = unmetPrecondition();
    if (!missing.isEmpty()) {
        QTimer::singleShot(0, this, [this, id, missing] {
            Q_EMIT chatFinished(id, QString(), missing);
        });
        return id;
    }

    if (m_keyKnown) {
        post(id, prompt, false);
        return id;
    }

    // The wallet may be standing in front of its own password dialog, so the
    // call waits rather than failing (keystore.h). One readKey() for however
    // many calls come in meanwhile: three notes in a row would otherwise be
    // three requests to a store that answers them all from the same handle.
    m_waiting.append({id, prompt, false});
    if (!m_keyAsked) {
        m_keyAsked = true;
        KeyStore::self()->readKey(QString(m_service.keyName));
    }
    return id;
}

int OpenAiCompatibleProvider::embed(const QString &text)
{
    const int id = nextRequestId();

    // The guided sentence for the reason chat() gives it, and here the price of
    // getting it wrong is the note's two attempts of SPEC 7.2 — hence
    // `Unreachable`: nothing about this note was refused, and the run is to
    // stop rather than count.
    const QString missing = unmetEmbeddingPrecondition();
    if (!missing.isEmpty()) {
        // Through the event loop, because the caller has not seen the id yet
        // while its own call is still on the stack (aiprovider.h).
        QTimer::singleShot(0, this, [this, id, missing] {
            Q_EMIT embedFinished(id, {}, missing, AiFailure::Unreachable);
        });
        return id;
    }

    if (m_keyKnown) {
        post(id, text, true);
        return id;
    }

    m_waiting.append({id, text, true});
    if (!m_keyAsked) {
        m_keyAsked = true;
        KeyStore::self()->readKey(QString(m_service.keyName));
    }
    return id;
}

QString OpenAiCompatibleProvider::unmetPrecondition() const
{
    if (m_model.isEmpty()) {
        return i18n("No model for %1 is set."
                    " Enter one in the settings under \"AI provider\".",
                    QString(m_service.name));
    }
    return {};
}

QString OpenAiCompatibleProvider::unmetEmbeddingPrecondition() const
{
    if (m_embeddingModel.isEmpty()) {
        return i18n("No embedding model for %1 is set."
                    " Enter one in the settings under \"AI provider\".",
                    QString(m_service.name));
    }
    return {};
}

void OpenAiCompatibleProvider::releaseWaiting(const QString &error)
{
    // Taken out first: post() below can answer synchronously on a transport
    // that fails at once, and a list being walked while it grows is the fault
    // this line is here to prevent.
    const QList<Waiting> waiting = std::exchange(m_waiting, {});
    for (const Waiting &call : waiting) {
        if (error.isEmpty()) {
            post(call.id, call.text, call.embedding);
        } else if (call.embedding) {
            // Unreachable, because a wallet that did not answer says nothing
            // about the note that happened to be first in the queue — the next
            // one fares exactly the same (aiprovider.h).
            Q_EMIT embedFinished(call.id, {}, error, AiFailure::Unreachable);
        } else {
            Q_EMIT chatFinished(call.id, QString(), error);
        }
    }
}

void OpenAiCompatibleProvider::post(int id, const QString &text, bool embedding)
{
    const QJsonObject message{
        {QLatin1String("role"), QLatin1String("user")},
        {QLatin1String("content"), text},
    };
    // **The embedding call is not streamed**, and that is the endpoint's shape
    // rather than a decision of ours: `/v1/embeddings` answers with one
    // document. The 30 s of SPEC 7.1 therefore bound the whole call here, which
    // is what they used to do for chat before issue #121 — and it is right for
    // this one, because an embedding does no reasoning and the case that broke
    // there cannot arise.
    const QJsonObject body =
        embedding ? QJsonObject{
                        {QLatin1String("model"), m_embeddingModel},
                        {QLatin1String("input"), text},
                    }
                  : QJsonObject{
                        {QLatin1String("model"), m_model},
                        {QLatin1String("messages"), QJsonArray{message}},
                        // Streamed, and the reason is issue #121's: unstreamed,
                        // the limit below measures the whole answer, and a
                        // reasoning model breaks it without anything being
                        // wrong. See the class comment.
                        {QLatin1String("stream"), true},
                    };

    QNetworkRequest request(embedding ? m_embedUrl : m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    // The key never reaches a log, an error sentence or a settings file: it is
    // written onto the request here and lives nowhere else in this class but in
    // m_key (SPEC 5.2).
    request.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + m_key.toUtf8());
    request.setTransferTimeout(m_timeout);

    QNetworkReply *reply = m_network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    // Bound to the reply as its context object, so an answer arriving first
    // takes the timer with it — measured in review on 2026-08-30 for the same
    // construction in OllamaProvider: with `qApp` as the context the lambda ran
    // on a destroyed reply and the process died with SIGSEGV.
    QTimer::singleShot(m_callLimit, reply, [reply] {
        reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, id, embedding] {
        reply->deleteLater();
        const QNetworkReply::NetworkError transport = reply->error();
        const QString transportMessage = reply->errorString();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        // An aborted reply is closed, and reading it anyway only earns a
        // "device not open".
        const QByteArray answerBody = reply->isOpen() ? reply->readAll() : QByteArray();

        if (embedding) {
            const OpenAiCompatibleEmbedding vector =
                readOpenAiCompatibleEmbedding(m_service.name, transport, transportMessage, status, answerBody);
            Q_EMIT embedFinished(id, vector.vector, vector.error, vector.failure);
            return;
        }

        const OpenAiCompatibleAnswer answer =
            readOpenAiCompatibleReply(m_service.name, transport, transportMessage, status, answerBody);
        Q_EMIT chatFinished(id, answer.text, answer.error);
    });
}
