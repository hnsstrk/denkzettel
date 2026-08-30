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

/** What a finished openrouter reply says: the text, or the reason there is none. */
struct OpenRouterAnswer {
    /** What the assistant wrote, put back together out of the stream. */
    QString text;
    /** Empty exactly when `text` carries the answer. */
    QString error;
};

/**
 * The one place an openrouter reply becomes an answer or a comprehensible
 * error — readOllamaReply()'s counterpart for the OpenAI-compatible API
 * (SPEC 7.1, issue #38).
 *
 * A pure function and not a method, so the mapping can be checked without a
 * server: every case below is one an `aitest` function hands in directly. The
 * order the cases are tried in is the order of what the user needs told, and it
 * is the Ollama one:
 *
 * 1. **Nothing arrived for 30 s** — the silence limit of SPEC 7.1.
 * 2. **The call ran past 5 minutes** — the total limit, which is an abort() of
 *    ours on a running reply.
 * 3. **openrouter itself refused**, and then its own sentence is the best one
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
 * ponytail: **every value below is written from openrouter's documentation and
 * not one of them has been measured**, because there is no API key for the
 * acceptance — the customer confirmed it on 30.08.2026 and the check therefore
 * runs against a stand-in (issue #38). What a stand-in cannot buy is finding
 * 40: this project's own precedent is `setTransferTimeout()`, documented to
 * abort with `OperationCanceledError` while a real Ollama answers
 * `TimeoutError`, with a green test that proved nothing because the code and
 * the check had read the same page. Unmeasured here, in this order:
 *
 * - that `stream: true` really produces `data: ` frames closed by
 *   `data: [DONE]`, and that the text stands in `choices[0].delta.content`;
 * - that a refusal really carries `error.message`, and which HTTP statuses it
 *   comes with;
 * - that a bitten transfer timeout over TLS really arrives as `TimeoutError`
 *   — measured for Ollama over plain HTTP on 2026-08-30, not for this service;
 * - that openrouter's keep-alive comment lines really begin with `:`, which is
 *   what is supposed to keep the silence limit from biting on a slow model.
 *
 * `OperationCanceledError` is the one value here that does **not** need a live
 * run: it is what Qt's own abort() produces, measured on 2026-08-30 against a
 * real server, and nothing about the service can change it.
 *
 * Upgrade path: one live run per value, once the customer has a key, and the
 * value goes into the check out of that run rather than out of this comment.
 */
OpenRouterAnswer readOpenRouterReply(QNetworkReply::NetworkError transport,
                                     const QString &transportMessage,
                                     int httpStatus,
                                     const QByteArray &body);

/**
 * The defaults of SPEC 7.1 for this backend, and the one place they stand —
 * `ollama::` beside it carries the same reasoning.
 */
namespace openrouter
{
/** The OpenAI-compatible chat endpoint. Not a setting: the service is the service. */
inline constexpr QLatin1StringView Endpoint("https://openrouter.ai/api/v1/chat/completions");

/**
 * The model asked when `denkzettelrc` says nothing.
 *
 * **Not settled by SPEC 7.1, which names no model for this service**, and it
 * is a decision with a price tag: every call is billed. Reported as open with
 * issue #38 — a cheap model that answers JSON is what the classification of
 * SPEC 7.2 needs, and this is the implementer's choice until the customer
 * makes one.
 */
inline constexpr QLatin1StringView DefaultChatModel("openai/gpt-4o-mini");

/** The entry KeyStore keeps this service's key under (SPEC 5.2, issue #37). */
inline constexpr QLatin1StringView KeyName("openrouter");
}

/**
 * openrouter.ai over its OpenAI-compatible HTTP API (SPEC 7.1): chat, and
 * chat only.
 *
 * **The two capabilities are kept apart** (SPEC 7.1, issue #38). `embed()`
 * below answers with the sentence saying so and canEmbed() reports it, so
 * nothing asks this service for a vector and no missing local Ollama can look
 * like a remote API failure. Since the customer decision of 29.08.2026 SPEC 7.1
 * allows both capabilities per provider; building openrouter's embedding side
 * is issue #130 and expressly not this story.
 *
 * The model is a setting out of `denkzettelrc` (`[AI] OpenRouterModel`), read
 * at construction and re-read on reloadSettings(). **The key is not**: SPEC 5.2
 * forbids it in a configuration file, so it comes from `KeyStore` and from
 * nowhere else, and the first call is what fetches it.
 *
 * **The chat call streams**, for the reason OllamaProvider's does (issue #121):
 * unstreamed, the 30 s of SPEC 7.1 bound the whole answer, and a reasoning
 * model over a remote API is exactly the case that broke — measured there at
 * 18.1 s to 46.9 s for one and the same note. Streamed, the limit is what
 * SPEC 7.1 means by it, and the 5 minutes of setCallLimit() are what ends a
 * call that trickles for ever.
 *
 * **No retry of ours**, and here the reason is money rather than time:
 * `QNetworkAccessManager` already repeats a closed connection by itself (the
 * one retry SPEC 7.1 asks for, measured on 2026-08-29), and a second request
 * of ours would be a second generation and a second bill for one job.
 */
class OpenRouterProvider : public AiProvider
{
    Q_OBJECT

public:
    explicit OpenRouterProvider(QObject *parent = nullptr);

    /** `openrouter::DefaultChatModel` unless `denkzettelrc` says otherwise. */
    void setChatModel(const QString &model);
    QString chatModel() const;

    /**
     * The key, set past `KeyStore` — for the settings page, which tests what
     * stands on the form rather than what stands in the wallet, and for a check,
     * which has no wallet at all (CLAUDE.md, finding 77: in a headless session
     * `kwalletd6` asks the user before it opens anything).
     */
    void setKey(const QString &key);

    /** `openrouter::Endpoint` unless a check points it at a stand-in. */
    void setUrl(const QUrl &url);

    /** The 30 s of silence of SPEC 7.1; settable for the reason it is on Ollama. */
    void setTimeout(std::chrono::milliseconds timeout);

    /** The 5 minutes one call may take all told (SPEC 7.1, decision 30.08.2026). */
    void setCallLimit(std::chrono::milliseconds limit);

    int chat(const QString &prompt) override;

    /**
     * Answers with the sentence that this service is not asked for vectors —
     * through the event loop, like every other answer here.
     *
     * `AiFailure::Unreachable` and not `Refused`: nothing about the note was
     * refused, and a run that counted this against the note would burn its two
     * attempts of SPEC 7.2 on a call nobody should have made.
     */
    int embed(const QString &text) override;

    /** False: this backend does chat and nothing else, see the class comment. */
    bool canEmbed() const override;

public Q_SLOTS:
    /**
     * Re-reads `[AI] OpenRouterModel` out of `denkzettelrc` and forgets the key.
     *
     * The key is forgotten and not re-read here: a user who has just put a new
     * one in the settings would otherwise go on being billed against the old
     * one until the daemon restarts, and re-reading it now would open the wallet
     * for a key that may never be needed. The next call fetches it.
     */
    void reloadSettings();

private:
    /** One call waiting for the wallet to answer. */
    struct Waiting {
        int id;
        QString prompt;
    };

    /** Posts one request and answers it. Once — see the class comment. */
    void post(int id, const QString &prompt);
    /** Answers everything in m_waiting with `error`, or sends it. */
    void releaseWaiting(const QString &error);

    QNetworkAccessManager m_network;
    QUrl m_url = QUrl(QString(openrouter::Endpoint));
    QString m_model;
    QString m_key;
    /** False until the wallet has answered once, or setKey() has been called. */
    bool m_keyKnown = false;
    /** True while a readKey() is out, so the wallet is asked once for many calls. */
    bool m_keyAsked = false;
    QList<Waiting> m_waiting;
    std::chrono::milliseconds m_timeout = std::chrono::seconds(30);
    std::chrono::milliseconds m_callLimit = std::chrono::minutes(5);
};
