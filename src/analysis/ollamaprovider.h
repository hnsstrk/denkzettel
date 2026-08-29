#pragma once

#include "analysis/aiprovider.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLatin1StringView>
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
    /**
     * Which kind of failure `error` is (AiFailure), and `None` when there is
     * none.
     *
     * The line the six cases below are sorted along: the first and the third
     * are a backend that never answered, the other four came out of one that
     * did. Only the embedding run reads it, and what it decides with it is
     * whether the note is counted against or the run is stopped.
     */
    AiFailure failure = AiFailure::None;
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
 *    arrives as `TimeoutError`, measured; `OperationCanceledError`, which Qt's
 *    documentation of `setTransferTimeout` names, is mapped beside it and is
 *    unreachable in this program today (see the comment at the case).
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
 * The three defaults of SPEC 7.1, and the one place they stand.
 *
 * Three parties need the same values: the provider below, which reads
 * `denkzettelrc` at construction; the settings skeleton, which declares the
 * same keys with the same defaults; and the model boxes of the settings page,
 * which offer them as the way back. Written down three times they would drift
 * apart without a sound — a "reset to the default" that hands out a model the
 * provider never asks for.
 *
 * Here and not in the settings, because the dependency only runs one way:
 * `denkzettelsettings` links `denkzettelanalysis`, not the other way round.
 */
namespace ollama
{
inline constexpr QLatin1StringView DefaultUrl("http://localhost:11434");
inline constexpr QLatin1StringView DefaultChatModel("qwen3:8b");
inline constexpr QLatin1StringView DefaultEmbeddingModel("bge-m3");

/**
 * `[AI] EmbeddingModel` as `denkzettelrc` currently spells it, or the default
 * above where it says nothing.
 *
 * Three objects have to agree on this one name inside a run — the provider
 * that asks for the vector, the embedder that writes it beside the note and the
 * suggester that clusters what carries it — and since issue #119 all three
 * re-read it when the settings dialog has written. Read out of one function so
 * that the group and the key have one place: three `readEntry` calls would be
 * three chances to spell it differently, and two spellings are two models with
 * nothing to say so.
 */
QString configuredEmbeddingModel();
}

/**
 * Ollama over its HTTP API (SPEC 7.1): `/api/chat` and `/api/embed`.
 *
 * The address and the two models are settings out of `denkzettelrc` (SPEC
 * 5.2), read at construction, re-read on reloadSettings() and settable
 * afterwards — the settings dialog of SPEC 13 writes them, and a check points
 * them at a port that is not listening.
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

    /** `ollama::DefaultUrl` unless `denkzettelrc` says otherwise. */
    void setUrl(const QUrl &url);
    /** SPEC 7.1, `ollama::DefaultChatModel`. */
    void setChatModel(const QString &model);
    QString chatModel() const;
    /** SPEC 7.1, `ollama::DefaultEmbeddingModel`; in v1 every embedding comes from here. */
    void setEmbeddingModel(const QString &model);
    QString embeddingModel() const;

    /**
     * The 30 s of SPEC 7.1, settable because a check that waits half a minute
     * for the limit to bite is one nobody runs.
     */
    void setTimeout(std::chrono::milliseconds timeout);

    int chat(const QString &prompt) override;
    int embed(const QString &text) override;

    /**
     * Whether the server answers at all, told by reachabilityTested() below
     * (issue #17).
     *
     * **Not testConnection() of the base class**, and the difference is the
     * price. That one is the button of SPEC 7.1: the user pressed something
     * and wants the latency of a real `chat` and a real `embed`, so it loads
     * both models. The tool detection of SPEC 2.5 presses nothing — it runs at
     * every start of the daemon, and it only wants to know whether the tray
     * tooltip has to name Ollama. Measured 2026-08-29 against a running Ollama
     * on this machine: `/api/chat` with `qwen3:8b` 3.08 s and `/api/embed`
     * with `bge-m3` 1.64 s from cold, against **1 ms** for `/api/tags` — which
     * lists what is pulled and loads nothing. At the 30 s limit of SPEC 7.1
     * the bad case is a minute of login for one line of tooltip.
     *
     * `/api/tags` and not a HEAD on the root: it is the one endpoint whose
     * answer says "this is an Ollama", and it is the same road, the same
     * `QNetworkAccessManager` and the same timeout as every other call here.
     */
    void testReachability();

public Q_SLOTS:
    /**
     * Re-reads `[AI] OllamaUrl`, `ChatModel` and `EmbeddingModel` out of
     * `denkzettelrc`.
     *
     * The daemon holds one provider for the whole session and the settings
     * dialog writes underneath it (SPEC 13). Without this the address and the
     * models chosen there would only take hold at the next start, while "Test
     * connection" on the same page — which works with the value out of the
     * form — reported the new address as reachable: the check says yes and the
     * analysis run talks to the old server (issue #119). A call that is
     * already on its way keeps what it was sent with; what is read here reaches
     * the next one.
     */
    void reloadSettings();

Q_SIGNALS:
    /**
     * The answer to testReachability(): whether the transfer got through.
     *
     * A bool and not a sentence, because the tooltip names the server and
     * nothing else (SPEC 14, the quiet channel). Whoever wants to know **why**
     * presses "Test connection" in the settings, where the whole mapping of
     * readOllamaReply() is shown.
     */
    void reachabilityTested(bool reachable);

private:
    /**
     * Posts one request and answers it. **Once** — the one retry of SPEC 7.1
     * is Qt's, and a second one of ours was measured out (2026-08-29).
     *
     * Every error this class used to repeat was gone through with a stand-in
     * that fails the same way every time, and not one survived:
     *
     * - `RemoteHostClosedError` — **Qt already repeats it**, which is what
     *   SPEC 7.1 asks for. A stand-in that closes every connection after
     *   reading the request saw **3** of them with our repeat and **2**
     *   without. That is also the only genuinely transient case there is: a
     *   connection out of the keep-alive pool that the server shut while it
     *   was idle.
     * - `TimeoutError` — a limit that is granted again after it has been hit
     *   is not a limit. Against a stand-in that never answers, a limit of
     *   1000 ms produced two requests and took 1968 ms; at the 30 s SPEC 7.1
     *   names that is a minute per call and two before testConnection()
     *   speaks.
     * - `ConnectionRefusedError` — the kernel's answer that nothing is bound
     *   to that port, delivered in microseconds. Measured: two attempts, both
     *   refused, 2 ms all told. For the second to differ, the server would
     *   have to take the port inside those two milliseconds.
     * - `HostNotFoundError` — measured at two attempts as well. The second
     *   lookup is answered from the same negative cache as the first; what
     *   produces it is a name that is spelt wrong, not a network that blinked.
     * - `OperationCanceledError` — nothing in this program can produce it, see
     *   readOllamaReply() above.
     * - `TemporaryNetworkFailureError` and `NetworkSessionFailedError` — the
     *   bearer management that set them was dropped in Qt 6.
     *   `libQt6Network.so.6` carries **no** string for `QNetworkSession` or
     *   roaming at all.
     * - `UnknownNetworkError` — a catch-all says nothing about whether a
     *   second attempt can come out differently.
     *
     * What the repeat cost was not only time: SPEC 7.1 puts openrouter and
     * OpenAI beside Ollama, and there a repeated request is a second
     * generation and a second bill for one job.
     */
    void send(int id, OllamaCall call, const QString &path, const QJsonObject &body);

    QNetworkAccessManager m_network;
    QUrl m_url;
    QString m_chatModel;
    QString m_embeddingModel;
    std::chrono::milliseconds m_timeout = std::chrono::seconds(30);
};
