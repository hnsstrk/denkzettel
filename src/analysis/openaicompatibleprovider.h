#pragma once

#include "analysis/aiprovider.h"

#include <QJsonObject>
#include <QLatin1StringView>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QUrl>

#include <chrono>

/**
 * What tells one OpenAI-compatible service from another — and it is four
 * strings and nothing else.
 *
 * openrouter.ai and OpenAI speak the same protocol: that is openrouter's own
 * selling point, and it is why this file carries **one** client instead of two
 * (issue #39). Everything the two services differ in stands here; everything
 * they share stands below and is written once. The alternative was measured on
 * the diff and rejected: a second copy of the reply reader would have to be
 * corrected twice, and that reader has already been corrected twice in review
 * (the refusal inside the stream, and the key the settings page wiped).
 */
struct AiService {
    /** What the user reads in a sentence about this service. */
    QLatin1StringView name;
    /**
     * What a vector out of this service is kept under in the store
     * (AiProvider::serviceId(), issue #130) — the same string `[AI] Provider`
     * carries for this choice.
     *
     * A field of its own and not `name` or `keyName`: those two are a display
     * string and a wallet entry, and renaming either of them is a cosmetic act
     * that would silently devalue every stored vector.
     */
    QLatin1StringView id;
    /** The chat endpoint. Not a setting: the service is the service. */
    QLatin1StringView endpoint;
    /** The embedding endpoint, for the same reason (SPEC 7.1, issue #130). */
    QLatin1StringView embedEndpoint;
    /** The entry KeyStore keeps this service's key under (SPEC 5.2, issue #37). */
    QLatin1StringView keyName;
    /** The `[AI]` key in `denkzettelrc` that carries this service's chat model. */
    QLatin1StringView modelKey;
    /** The `[AI]` key that carries this service's embedding model (issue #130). */
    QLatin1StringView embeddingModelKey;
};

/** What a finished reply says: the text, or the reason there is none. */
struct OpenAiCompatibleAnswer {
    /** What the assistant wrote, put back together out of the stream. */
    QString text;
    /** Empty exactly when `text` carries the answer. */
    QString error;
};

/** The same for an embedding call, and it carries the failure kind with it. */
struct OpenAiCompatibleEmbedding {
    /** The vector the service made of the text. */
    QList<double> vector;
    /** Empty exactly when `vector` carries the embedding. */
    QString error;
    /** Which kind of failure it was — the embedding run decides on it. */
    AiFailure failure = AiFailure::None;
};

/**
 * The one place an OpenAI-compatible reply becomes an answer or a
 * comprehensible error — readOllamaReply()'s counterpart for the two remote
 * services of SPEC 7.1 (issues #38 and #39).
 *
 * A pure function and not a method, so the mapping can be checked without a
 * server: every case below is one an `aitest` function hands in directly. The
 * order the cases are tried in is the order of what the user needs told, and it
 * is the Ollama one:
 *
 * 1. **Nothing arrived for 30 s** — the silence limit of SPEC 7.1.
 * 2. **The call ran past 5 minutes** — the total limit, which is an abort() of
 *    ours on a running reply.
 * 3. **The service itself refused**, and then its own sentence is the best one
 *    there is: an unknown model, a spent quota and a rejected key all arrive as
 *    `{"error":{"message":"…"}}`, and "HTTP 400" alone would leave the user
 *    looking for a network fault.
 * 4. **The connection failed** — no answer, so `transportMessage` carries it.
 * 5. **An HTTP status that is not a success**, with nothing readable in the
 *    body to say why.
 * 6. **The body is no stream** — what a proxy or a captive portal answering
 *    with HTML looks like.
 * 7. **The stream stopped before `[DONE]`** — see the case for why that is not
 *    an answer.
 * 8. **Every frame arrived and none of them carried text.**
 *
 * ponytail: **every value below is written from the two services' documentation
 * and not one of them has been measured**, because there is no API key for the
 * acceptance — the customer confirmed it for openrouter on 30.08.2026 (issue
 * #38) and again for OpenAI (issue #39), so both clients are checked against a
 * stand-in. What a stand-in cannot buy is finding 40: this project's own
 * precedent is `setTransferTimeout()`, documented to abort with
 * `OperationCanceledError` while a real Ollama answers `TimeoutError`, with a
 * green test that proved nothing because the code and the check had read the
 * same page. Unmeasured here, in this order, **and for each of the two
 * services separately** — they are two companies with two documentations, and
 * a value measured against one would say nothing about the other:
 *
 * - that `stream: true` really produces `data: ` frames closed by
 *   `data: [DONE]`, and that the text stands in `choices[0].delta.content`;
 * - that a refusal really carries `error.message`, and which HTTP statuses it
 *   comes with — **and in which of the two shapes**: as one document before the
 *   stream starts, or as a frame inside it. Both are read here, because the
 *   documentation describes both and neither has been seen;
 * - that a bitten transfer timeout over TLS really arrives as `TimeoutError`
 *   — measured for Ollama over plain HTTP on 2026-08-30, not for either of
 *   these services;
 * - that a keep-alive comment line really begins with `:`, which is what is
 *   supposed to keep the silence limit from biting on a slow model;
 * - **that the endpoints in `openrouter::Service` and `openai::Service`, the
 *   `Bearer` scheme and the shape of the request body are what these services
 *   accept at all** — `messages` as a list of objects with `role` and
 *   `content`, beside `model` and `stream`. All of it comes from the
 *   documentation. The check `theKeyAndTheModelGoOnTheWire()` proves that *we*
 *   write that header, that path and that body — it cannot prove that either
 *   service reads any of them, and no stand-in can.
 *
 * `OperationCanceledError` is the one value here that does **not** need a live
 * run: it is what Qt's own abort() produces, measured on 2026-08-30 against a
 * real server, and nothing about a service can change it.
 *
 * Upgrade path: one live run per value and per service, once the customer has a
 * key, and the value goes into the check out of that run rather than out of
 * this comment.
 */
OpenAiCompatibleAnswer readOpenAiCompatibleReply(QLatin1StringView service,
                                                 QNetworkReply::NetworkError transport,
                                                 const QString &transportMessage,
                                                 int httpStatus,
                                                 const QByteArray &body);

/**
 * The same for `/v1/embeddings` (SPEC 7.1, issue #130), and a second function
 * because the two calls answer in two shapes.
 *
 * The embedding call does **not** stream — it answers with one JSON document
 * carrying `data[0].embedding` — so the whole frame apparatus above has nothing
 * to read here, and the seven cases that are left are the transport ones plus
 * "the document carried no vector". The order is the one above, for the reason
 * it is: it is the order of what the user needs told.
 *
 * **It carries the failure kind, which the chat reader does not need**: SPEC
 * 7.2 counts a refusal against the note and stops the run on an unreachable
 * backend, and only the embedding run makes that distinction (aiprovider.h).
 * A timeout, an aborted call and a connection that failed are unreachable; a
 * document the service refused, an unreadable body and an answer without a
 * vector in it are refusals — the service answered, so it is there.
 *
 * ponytail: unmeasured against both live services for the reason the chat
 * reader is, and the list of what a key would buy is the same one, `data`,
 * `embedding` and the shape of a refusal in place of the stream fields. That
 * `POST /v1/embeddings` exists at all on both services **is** measured —
 * 401 against a 404 control on each, 04.09.2026, issue #130.
 */
OpenAiCompatibleEmbedding readOpenAiCompatibleEmbedding(QLatin1StringView service,
                                                        QNetworkReply::NetworkError transport,
                                                        const QString &transportMessage,
                                                        int httpStatus,
                                                        const QByteArray &body);

/**
 * The defaults of SPEC 7.1 for this backend, and the one place they stand —
 * `ollama::` beside it carries the same reasoning.
 *
 * **There is deliberately no default model for either service** (customer
 * decision 30.08.2026, SPEC 7.1) — hence a constant that is missing from both
 * descriptors rather than an empty one.
 *
 * The customer names two opposite reasons for reaching past Ollama: a machine
 * without the compute to run a model locally, *or* the wish for a distinctly
 * stronger one. No model serves both, so any default would quietly make the
 * choice he reserved for himself — and it would make it every 30 minutes,
 * unattended and billed. The field therefore stands empty until he fills it,
 * and an empty field is a precondition and not a fault: see
 * unmetPrecondition() below.
 */
namespace openrouter
{
inline constexpr AiService Service{QLatin1StringView("openrouter.ai"),
                                   QLatin1StringView("OpenRouter"),
                                   QLatin1StringView("https://openrouter.ai/api/v1/chat/completions"),
                                   QLatin1StringView("https://openrouter.ai/api/v1/embeddings"),
                                   QLatin1StringView("openrouter"),
                                   QLatin1StringView("OpenRouterModel"),
                                   QLatin1StringView("OpenRouterEmbeddingModel")};
}

namespace openai
{
/**
 * OpenAI over its platform API (SPEC 7.5, issue #39).
 *
 * **The key is a platform API key and nothing else.** SPEC 7.5 settled that on
 * the research of 2026-07-31: "Sign in with ChatGPT" is a pure identity
 * procedure and hands out name, e-mail address and profile picture, expressly
 * no token and no model access, and the unofficial Codex subscription route was
 * never released for third-party applications and carries no embeddings. The
 * settings page says so where the key is asked for.
 */
inline constexpr AiService Service{QLatin1StringView("OpenAI"),
                                   QLatin1StringView("OpenAI"),
                                   QLatin1StringView("https://api.openai.com/v1/chat/completions"),
                                   QLatin1StringView("https://api.openai.com/v1/embeddings"),
                                   QLatin1StringView("openai"),
                                   QLatin1StringView("OpenAiModel"),
                                   QLatin1StringView("OpenAiEmbeddingModel")};
}

/**
 * One of the two remote services of SPEC 7.1 over its OpenAI-compatible HTTP
 * API: chat **and** embedding.
 *
 * **Both capabilities, since the PO decision of 04.09.2026** (issue #130): the
 * provider is chosen once and answers both. Until then `embed()` here answered
 * with a sentence saying the vectors come from Ollama, on the premise —
 * measured 2026-07-31 and true then — that openrouter had no embedding
 * endpoint. It has: `POST /api/v1/embeddings` answers 401 where an invented
 * route answers 404, and the same holds for OpenAI (04.09.2026).
 *
 * **Two models and therefore two settings keys** (`modelKey` and
 * `embeddingModelKey`), read at construction and re-read on reloadSettings().
 * One key would carry a chat model id into the embedding call and back. **The
 * key is not a setting**: SPEC 5.2 forbids it in a configuration file, so it
 * comes from `KeyStore` under the descriptor's `keyName` and from nowhere else,
 * and the first call is what fetches it.
 *
 * **The chat call streams**, for the reason OllamaProvider's does (issue #121):
 * unstreamed, the 30 s of SPEC 7.1 bound the whole answer, and a reasoning
 * model over a remote API is exactly the case that broke — measured there at
 * 18.1 s to 46.9 s for one and the same note. Streamed, the limit is what
 * SPEC 7.1 means by it, and the 5 minutes of setCallLimit() are what ends a
 * call that trickles for ever.
 *
 * **No retry of ours**, and here the reason is money rather than time: a second
 * request would be a second generation and a second bill for one job.
 *
 * **Whether anything repeats at all is open for both of these backends**
 * (review of 30.08.2026 for openrouter, measured again for OpenAI on
 * 30.08.2026 under issue #39). That `QNetworkAccessManager` repeats a closed
 * connection by itself is measured — for `OllamaProvider`, on 2026-08-29, under
 * issue #13. Carried over to here it was a borrowed result: five stand-in forms
 * against this client produced no repetition in any of them, and the same
 * permanently closing stand-in run against the OpenAI descriptor sees **one**
 * request per call as well. So **the one retry SPEC 7.1 asks for is not
 * established for either remote service**, and it is named as open rather than
 * claimed. What *is* measured is the half that costs money: this client sends
 * one request per chat() call and no second one of its own.
 */
class OpenAiCompatibleProvider : public AiProvider
{
    Q_OBJECT

public:
    /**
     * `service` names the endpoint, the wallet entry and the settings key —
     * `openrouter::Service` or `openai::Service`. It holds views on string
     * literals, so the copy costs nothing and outlives everything.
     */
    explicit OpenAiCompatibleProvider(const AiService &service, QObject *parent = nullptr);

    /** The service's own `[AI]` model key, and empty until the user names one. */
    void setChatModel(const QString &model);
    QString chatModel() const;

    /** The same for the embedding model, and empty for the same reason. */
    void setEmbeddingModel(const QString &model);
    QString embeddingModel() const override;

    /** The descriptor's `id`. */
    QString serviceId() const override;

    /**
     * The key, set past `KeyStore` — for the settings page, which tests what
     * stands on the form rather than what stands in the wallet, and for a check,
     * which has no wallet at all (CLAUDE.md, finding 77: in a headless session
     * `kwalletd6` asks the user before it opens anything).
     */
    void setKey(const QString &key);

    /** The descriptor's chat endpoint unless a check points it at a stand-in. */
    void setUrl(const QUrl &url);

    /** The same for the embedding endpoint. */
    void setEmbedUrl(const QUrl &url);

    /** The 30 s of silence of SPEC 7.1; settable for the reason it is on Ollama. */
    void setTimeout(std::chrono::milliseconds timeout);

    /** The 5 minutes one call may take all told (SPEC 7.1, decision 30.08.2026). */
    void setCallLimit(std::chrono::milliseconds limit);

    int chat(const QString &prompt) override;

    /**
     * One `POST` to the descriptor's `embedEndpoint`, answered by
     * embedFinished() (SPEC 7.1, issue #130).
     *
     * The same road chat() takes, key and all — the wallet is asked once for
     * however many calls come in meanwhile.
     */
    int embed(const QString &text) override;

    /**
     * Why this backend cannot be called at all — the missing model, or empty
     * when there is nothing in the way.
     *
     * **The rule is SPEC 12's, applied to the analysis run of SPEC 7.2**: a
     * missing model is a precondition not yet met, not a failed attempt
     * (decision 29.08.2026, issue #23). Without it the 30-minute run of SPEC
     * 7.2 would spend **both** attempts of every note on "model required"
     * before the user ever opened the settings page — and the notes would
     * stand in the error state with nothing wrong with them.
     *
     * Asked by Classifier::start() before a note is taken out, because taking
     * one out is what spends the attempt.
     */
    QString unmetPrecondition() const override;

    /**
     * The same for the embedding model, and asked by Embedder::start() for the
     * same reason (issue #130).
     *
     * Two questions and not one, because the two models are two settings: a
     * user who has named a chat model and no embedding model has the
     * classification running, and one answer for both would either stop that
     * over a field it does not need or send an empty model name to the service.
     */
    QString unmetEmbeddingPrecondition() const override;

public Q_SLOTS:
    /**
     * Re-reads the service's model out of `denkzettelrc` and forgets the key.
     *
     * The key is forgotten and not re-read here: a user who has just put a new
     * one in the settings would otherwise go on being billed against the old
     * one until the daemon restarts, and re-reading it now would open the wallet
     * for a key that may never be needed. The next call fetches it.
     */
    void reloadSettings();

private:
    /** One call waiting for the wallet to answer, chat or embedding. */
    struct Waiting {
        int id;
        QString text;
        bool embedding;
    };

    /** Posts one request and answers it. Once — see the class comment. */
    void post(int id, const QString &text, bool embedding);
    /** Answers everything in m_waiting with `error`, or sends it. */
    void releaseWaiting(const QString &error);

    AiService m_service;
    QNetworkAccessManager m_network;
    QUrl m_url;
    QUrl m_embedUrl;
    QString m_model;
    QString m_embeddingModel;
    QString m_key;
    /** False until the wallet has answered once, or setKey() has been called. */
    bool m_keyKnown = false;
    /** True while a readKey() is out, so the wallet is asked once for many calls. */
    bool m_keyAsked = false;
    QList<Waiting> m_waiting;
    std::chrono::milliseconds m_timeout = std::chrono::seconds(30);
    std::chrono::milliseconds m_callLimit = std::chrono::minutes(5);
};
