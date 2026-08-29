#pragma once

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QString>

#include <cstdint>

/**
 * Why a call carried no answer: because the backend never gave one, or because
 * it gave one and refused this request.
 *
 * The two are one sentence to the user and two different facts to a run. An
 * unreachable backend says nothing about the note that happened to be first in
 * the queue — the next note fares exactly the same, so the run stops and
 * nothing is counted against anybody. A refusal came out of a server that
 * answered: whatever it choked on, it choked on **this** text, and a note that
 * is refused twice is skipped rather than handed over for ever (SPEC 7.2).
 *
 * Only embedFinished() carries it. The classification of SPEC 7.2 counts every
 * failure against the note (issue #14, Classifier::fail()); what tells the
 * embedding run apart from it is that a corpus embedded again after every
 * outage would have burnt its counters through in two of them.
 */
enum class AiFailure : std::uint8_t {
    /** The call carried an answer. */
    None,
    /** No answer at all: a timeout, a refused connection, a transport error. */
    Unreachable,
    /** The backend answered, and what it answered was a refusal of this text. */
    Refused,
};

/**
 * What an AI backend can do for Denkzettel, and nothing beyond it (SPEC 7.1).
 *
 * Two capabilities: `chat()` turns a prompt into text — the classification of
 * SPEC 7.2 reads its JSON out of that text — and `embed()` turns a text into
 * the vector the clustering of SPEC 7.3 compares. Both are asynchronous,
 * because each of them is an HTTP call that may take seconds and the daemon
 * has one event loop, which the two windows live in as well.
 *
 * An interface with a single implementation is what this project otherwise
 * does not build. It is built here because SPEC 7.1 names three backends —
 * Ollama, openrouter.ai and OpenAI — of which only the first is due in v1, and
 * because the analysis pipeline of SPEC 7.2 is checked against a stand-in
 * rather than against a running model (SPEC 16: "prompt/JSON schema
 * processing, provider mocked").
 *
 * **Every call answers under an id of its own.** Without it two answers
 * arriving in the other order would be indistinguishable, and the analysis run
 * of SPEC 7.2 asks once per note.
 *
 * **Asynchronous means through the event loop**, never from inside chat() or
 * embed(): the caller has not seen the id yet while its own call is still on
 * the stack, and testConnection() below would miss its answer.
 */
class AiProvider : public QObject
{
    Q_OBJECT

public:
    explicit AiProvider(QObject *parent = nullptr);
    ~AiProvider() override;

    AiProvider(const AiProvider &) = delete;
    AiProvider &operator=(const AiProvider &) = delete;

    /** Sends `prompt`; the answer comes as chatFinished() under the id returned. */
    virtual int chat(const QString &prompt) = 0;

    /** Turns `text` into a vector; the answer comes as embedFinished(). */
    virtual int embed(const QString &text) = 0;

    /**
     * One mini `chat` call and one `embed` call, answered by
     * connectionTested() with their latencies or with the first error
     * (SPEC 7.1).
     *
     * It lives here and not in the implementation because the procedure is the
     * same for every backend; what differs is what chat() and embed() talk to.
     * The button that presses it comes with the settings dialog (issue #16) —
     * as a method it is callable from a check and from the tool detection of
     * SPEC 2.5 alike.
     *
     * ponytail: one test at a time — a second call while the first is still
     * out abandons the first. The upgrade path is the id the two calls above
     * already carry.
     */
    void testConnection();

Q_SIGNALS:
    /** `error` is empty exactly when `answer` carries the model's text. */
    void chatFinished(int id, const QString &answer, const QString &error);

    /**
     * `error` is empty exactly when `vector` carries the embedding, and
     * `failure` says which kind of failure it was (AiFailure above).
     *
     * The fourth argument is what the embedding run of SPEC 7.2 decides on. A
     * listener that does not care takes the first three: Qt's connect() allows
     * a slot with fewer arguments, which is what testConnection() below does.
     */
    void embedFinished(int id, const QList<double> &vector, const QString &error, AiFailure failure);

    /**
     * The answer to testConnection(): both latencies in milliseconds, or the
     * error of whichever call failed first — and then both latencies are -1.
     */
    void connectionTested(qint64 chatMilliseconds, qint64 embedMilliseconds, const QString &error);

protected:
    /** The id the next chat() or embed() is to answer under. */
    int nextRequestId();

private:
    /** Runs from a call of the connection test going out until it is answered. */
    QElapsedTimer m_testClock;
    int m_testChatId = -1;
    int m_testEmbedId = -1;
    qint64 m_testChatMilliseconds = -1;
    int m_lastRequestId = 0;
};
