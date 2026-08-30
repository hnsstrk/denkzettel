#include "analysis/ollamaprovider.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTimer>

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
    // The silence limit of SPEC 7.1, and it arrives as TimeoutError — measured
    // on 2026-08-29 against a real Ollama with the limit set to 5 ms, and again
    // on 2026-08-30 with the limit at 5 s, where the same call came back
    // `TimeoutError (4)` after 5,260 ms. Mapping only the
    // OperationCanceledError that Qt's documentation of setTransferTimeout
    // names left this case falling through to "could not be reached:
    // Zeitüberschreitung", the transport's sentence over a limit of ours.
    if (transport == QNetworkReply::TimeoutError) {
        return {{}, {}, i18n("Ollama did not answer within the time limit."), AiFailure::Unreachable};
    }

    // The total limit of SPEC 7.1 beside it (see send()), and this is the value
    // an abort() on a running reply really produces: measured on 2026-08-30
    // against a real Ollama, a streamed call aborted after 3 s came back
    // `OperationCanceledError (5)` at 2,849 ms with HTTP 200 and an unreadable
    // body, while the same program with its timer set past the end of the
    // answer ran 38,537 ms to `NoError` — the control that says the value
    // belongs to the abort and not to the call.
    //
    // Until 2026-08-30 this value stood beside the one above with a comment
    // saying nothing in this program could reach it. The 5 minute limit reaches
    // it, so it gets a sentence of its own: the server did answer here, it only
    // took longer than one call may take, and "did not answer" would send the
    // user looking for a server that is running perfectly well.
    if (transport == QNetworkReply::OperationCanceledError) {
        return {{}, {}, i18n("Ollama took longer over this call than it is allowed."), AiFailure::Unreachable};
    }

    // `/api/chat` answers with one JSON object per line (`stream: true`, see
    // chat() below). **`/api/embed` does not stream and is read as the one
    // document it is** — put through the same split it would refuse a body it
    // read before, because a line is not a JSON unit: measured in review on
    // 2026-08-30, `{\n  "embeddings": [[0.5, 0.25]]\n}` came back as
    // "unreadable answer: unterminated object". Ollama prints compact today, so
    // nothing bit; what bit is that the change reached further than the road it
    // was made for.
    //
    // `trimmed()` takes the closing newline off, and it leaves an empty body an
    // empty line: that way an answer of nothing keeps reaching the "unreadable"
    // case below with a parser error to name, rather than with no error at all.
    QJsonParseError parseError;
    QList<QJsonObject> answers;
    const QList<QByteArray> lines =
        call == OllamaCall::Chat ? body.trimmed().split('\n') : QList<QByteArray>{body};
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
    // names the model. Every line that was read is asked, because a stream that
    // goes wrong half way through carries its reason in the last one — and a
    // body in which any line is unreadable has none of them, because the loop
    // above throws the lot away and the case lands on "unreadable answer"
    // instead of on the refusal. Named rather than fixed (review 2026-08-30):
    // both sentences are true of that body, and the one that survives is about
    // the half that arrived broken.
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
        // **A stream is an answer only once Ollama has said it is finished.**
        // Every line of a stream is valid JSON on its own, so a connection that
        // goes away at a line boundary leaves a body that parses cleanly and
        // carries half an answer — HTTP 200, no parse error, and nothing in it
        // to say the rest is missing. Unstreamed the case could not arise: a
        // document cut off anywhere has no closing brace and the parser refuses
        // it. Found in review on 2026-08-30, and it was the streaming that made
        // it possible.
        //
        // `done` and not the transport error, because `done` is the server's
        // own assurance and catches every road a stream can break off on — a
        // proxy closing cleanly, a server restarting mid-answer, a
        // Content-Length that ends early — while the transport error catches
        // one of them (CLAUDE.md, finding 62: name both ends of an assurance).
        // Unreachable and not Refused: nothing was refused, the transfer
        // stopped. `done_reason` is deliberately not read beside it — "stop",
        // "length" and "load" are all finished answers.
        if (!answers.constLast().value(QLatin1String("done")).toBool()) {
            return {{}, {}, i18n("Ollama's answer broke off."), AiFailure::Unreachable};
        }

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

void OllamaProvider::setCallLimit(std::chrono::milliseconds limit)
{
    m_callLimit = limit;
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
        // 2026-08-30 against Ollama 0.32.15 and `qwen3:8b`: reading the
        // 5.2 GB blob at 5.9 GB/s of cold throughput takes under a second, and
        // an 18 GB model about 3.1 s.
        //
        // The load is the smaller half. What decides the time is how many
        // tokens the model reasons for: at a throughput measured constant to
        // 1.2 % (92.5 to 93.6 tokens per second over five runs) the 30 s hold
        // about 2,800 output tokens, and identical input produced 477 to 1,385
        // of them in those five runs and 730 to 4,300 in four with another
        // note — 18.1 s to 46.9 s of wall time for one and the same note.
        // Unstreamed the limit therefore fell on a note for what the sampling
        // did, not for anything about the note.
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
    // over 30 s still costs the note an attempt. Reckoned at under a second and
    // about 3.1 s here (see chat()); a machine slow enough to break that is the
    // upgrade path — the classification run then reads AiFailure the way
    // Embedder does and stops the run instead of counting, which is road 2 of
    // issue #121. The same upgrade path covers the total limit below.
    request.setTransferTimeout(m_timeout);

    QNetworkReply *reply = m_network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    // **The total limit of SPEC 7.1**, and it exists because the limit above no
    // longer bounds a call: a server that sends one byte every 29 s holds the
    // analysis run for ever, and nothing else in the tree would end it —
    // AnalysisScheduler carries the interval between runs, not a bound on one
    // (decision 30.08.2026, customer). The point is not to be tight. Healthy
    // calls reach 46.9 s, and a bound near that would repeat issue #121 one
    // storey up; the point is to turn a silent hang into an error the attempt
    // counter of SPEC 7.2 can deal with.
    //
    // Bound to the reply as its context object, so the answer arriving first
    // takes the timer with it — there is nothing to cancel by hand and nothing
    // that can fire on a reply that is gone. The timer is a coarse one, which
    // may go off up to 5 % early: measured 2026-08-30, a 3,000 ms limit fired
    // at 2,849 ms. On five minutes that is a quarter of a minute, and a bound
    // this generous is not the place to spend a precise timer.
    QTimer::singleShot(m_callLimit, reply, [reply] {
        reply->abort();
    });

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
