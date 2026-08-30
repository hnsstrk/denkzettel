#include "analysis/openrouterprovider.h"

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

OpenRouterAnswer readOpenRouterReply(QNetworkReply::NetworkError transport,
                                     const QString &transportMessage,
                                     int httpStatus,
                                     const QByteArray &body)
{
    if (transport == QNetworkReply::TimeoutError) {
        return {{}, i18n("openrouter.ai did not answer within the time limit.")};
    }

    // The total limit of SPEC 7.1, and this is what an abort() on a running
    // reply produces — measured on 2026-08-30 against a real server for
    // OllamaProvider, and it is Qt's value rather than the service's, so it
    // carries here (see the header for what does not).
    if (transport == QNetworkReply::OperationCanceledError) {
        return {{}, i18n("openrouter.ai took longer over this call than it is allowed.")};
    }

    // A refusal is one JSON document and no stream, whatever the status says,
    // and its own sentence beats the status code: an unknown model, a spent
    // quota and a rejected key are three different things to the user and one
    // HTTP number.
    const QString refusal = QJsonDocument::fromJson(body)
                                .object()
                                .value(QLatin1String("error"))
                                .toObject()
                                .value(QLatin1String("message"))
                                .toString();
    if (!refusal.isEmpty()) {
        return {{}, i18n("openrouter.ai refused the request: %1", refusal)};
    }

    if (transport != QNetworkReply::NoError && httpStatus == 0) {
        return {{}, i18n("openrouter.ai could not be reached: %1", transportMessage)};
    }

    if (httpStatus != 0 && (httpStatus < 200 || httpStatus > 299)) {
        return {{}, i18n("openrouter.ai answered with HTTP status %1.", httpStatus)};
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
        // A line beginning with a colon is a comment, and openrouter sends
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
            return {{}, i18n("openrouter.ai sent an unreadable answer.")};
        }
        readable = true;

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
        return {{}, i18n("openrouter.ai sent an unreadable answer.")};
    }

    if (!finished) {
        return {{}, i18n("openrouter.ai's answer broke off.")};
    }

    if (text.isEmpty()) {
        return {{}, i18n("openrouter.ai's answer carried no text.")};
    }

    return {text, {}};
}

OpenRouterProvider::OpenRouterProvider(QObject *parent)
    : AiProvider(parent)
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
                if (provider != QLatin1String(openrouter::KeyName)) {
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
                    releaseWaiting(i18n("No API key for openrouter.ai is stored."
                                        " Enter it in the settings under \"AI provider\"."));
                    return;
                }

                m_key = key;
                m_keyKnown = true;
                releaseWaiting(QString());
            });
}

void OpenRouterProvider::reloadSettings()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    m_model = group.readEntry("OpenRouterModel", QString(openrouter::DefaultChatModel));
    m_keyKnown = false;
    m_key.clear();
}

void OpenRouterProvider::setChatModel(const QString &model)
{
    m_model = model;
}

QString OpenRouterProvider::chatModel() const
{
    return m_model;
}

void OpenRouterProvider::setKey(const QString &key)
{
    m_key = key;
    m_keyKnown = true;
}

void OpenRouterProvider::setUrl(const QUrl &url)
{
    m_url = url;
}

void OpenRouterProvider::setTimeout(std::chrono::milliseconds timeout)
{
    m_timeout = timeout;
}

void OpenRouterProvider::setCallLimit(std::chrono::milliseconds limit)
{
    m_callLimit = limit;
}

int OpenRouterProvider::chat(const QString &prompt)
{
    const int id = nextRequestId();

    if (m_keyKnown) {
        post(id, prompt);
        return id;
    }

    // The wallet may be standing in front of its own password dialog, so the
    // call waits rather than failing (keystore.h). One readKey() for however
    // many calls come in meanwhile: three notes in a row would otherwise be
    // three requests to a store that answers them all from the same handle.
    m_waiting.append({id, prompt});
    if (!m_keyAsked) {
        m_keyAsked = true;
        KeyStore::self()->readKey(QString(openrouter::KeyName));
    }
    return id;
}

int OpenRouterProvider::embed(const QString &text)
{
    Q_UNUSED(text)

    const int id = nextRequestId();
    // Through the event loop, because the caller has not seen the id yet while
    // its own call is still on the stack (aiprovider.h).
    QTimer::singleShot(0, this, [this, id] {
        Q_EMIT embedFinished(id,
                             {},
                             i18n("openrouter.ai is not asked for embeddings; those come from Ollama."),
                             AiFailure::Unreachable);
    });
    return id;
}

bool OpenRouterProvider::canEmbed() const
{
    return false;
}

void OpenRouterProvider::releaseWaiting(const QString &error)
{
    // Taken out first: post() below can answer synchronously on a transport
    // that fails at once, and a list being walked while it grows is the fault
    // this line is here to prevent.
    const QList<Waiting> waiting = std::exchange(m_waiting, {});
    for (const Waiting &call : waiting) {
        if (error.isEmpty()) {
            post(call.id, call.prompt);
        } else {
            Q_EMIT chatFinished(call.id, QString(), error);
        }
    }
}

void OpenRouterProvider::post(int id, const QString &prompt)
{
    const QJsonObject message{
        {QLatin1String("role"), QLatin1String("user")},
        {QLatin1String("content"), prompt},
    };
    const QJsonObject body{
        {QLatin1String("model"), m_model},
        {QLatin1String("messages"), QJsonArray{message}},
        // Streamed, and the reason is issue #121's: unstreamed, the limit below
        // measures the whole answer, and a reasoning model breaks it without
        // anything being wrong. See the class comment.
        {QLatin1String("stream"), true},
    };

    QNetworkRequest request(m_url);
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

    connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
        reply->deleteLater();
        const OpenRouterAnswer answer =
            readOpenRouterReply(reply->error(),
                                reply->errorString(),
                                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                                // An aborted reply is closed, and reading it
                                // anyway only earns a "device not open".
                                reply->isOpen() ? reply->readAll() : QByteArray());
        Q_EMIT chatFinished(id, answer.text, answer.error);
    });
}
