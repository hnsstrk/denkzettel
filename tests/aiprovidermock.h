#pragma once

#include "analysis/aiprovider.h"

#include <QTimer>

#include <chrono>

/**
 * The stand-in SPEC 16 asks for: a backend that answers what the check tells
 * it to, without a server and without a model (issue #13).
 *
 * It lives under `tests/` and not in `denkzettelanalysis` on purpose — a
 * provider that invents answers has no business in the running daemon, and a
 * class nothing but the checks construct would be shipped code that nobody
 * reaches.
 *
 * It answers **through the event loop**, as the interface requires. The two
 * delays are separate so that a check of the connection test can tell the two
 * latencies apart — with one delay for both, a clock that was never restarted
 * reports a plausible number as well.
 *
 * No `Q_OBJECT`: it declares no signal of its own and emits its base class's,
 * which are public. That keeps it a header the test sets simply include.
 */
class AiProviderMock : public AiProvider
{
public:
    using AiProvider::AiProvider;

    /** What chat() answers with, unless `chatError` is set. */
    QString chatAnswer = QStringLiteral("pong");
    /**
     * One answer per call, taken from the front; once they are used up
     * `chatAnswer` stands in again.
     *
     * A run over several notes needs a different answer per note, or a check
     * that every note gets **its** answer could not come out red: with one
     * answer for all of them, an implementation that writes the first answer
     * onto every note passes (CLAUDE.md, finding 34).
     */
    QStringList chatAnswers;
    /** What embed() answers with, unless `embedError` is set. */
    QList<double> embedVector = {0.5, -0.25};
    /**
     * One vector per call, taken from the front; once they are used up
     * `embedVector` stands in again.
     *
     * The same reason `chatAnswers` stands above it: with one vector for every
     * note, a run that writes the first answer onto all of them passes
     * (CLAUDE.md, finding 34).
     */
    QList<QList<double>> embedVectors;
    /** Non-empty turns the call into a failure carrying exactly this reason. */
    QString chatError;
    QString embedError;
    /**
     * Which kind of failure `embedError` is — a backend that never answered, or
     * one that answered and refused the text (AiFailure).
     *
     * It has no default that fits both: the embedding run treats the two
     * differently on purpose (SPEC 7.2), so a check that sets `embedError`
     * says which of them it is standing in for. `Refused` stands here because
     * it is the one that costs the note an attempt — a check that forgot the
     * line comes out on the counting side and is noticed.
     */
    AiFailure embedFailure = AiFailure::Refused;
    /**
     * The one text embed() refuses instead of answering — the note a backend
     * chokes on while it answers every other one.
     *
     * A whole corpus that fails is `embedError`; this is the case that tells a
     * run which ends on any failure from one which counts the refusal and goes
     * on, and no global error can stand in for it.
     */
    QString refusedText;
    /** What the refusal of `refusedText` says. */
    QString refusalReason = QStringLiteral("Ollama refused the request: input is not embeddable");
    /** How long chat() takes to answer. */
    std::chrono::milliseconds chatDelay{0};
    /** How long embed() takes to answer. */
    std::chrono::milliseconds embedDelay{0};

    /** Every prompt chat() was handed, in the order it was handed them. */
    QStringList prompts;
    /** Every text embed() was handed. */
    QStringList texts;

    int chat(const QString &prompt) override
    {
        prompts.append(prompt);
        const QString answer = chatAnswers.isEmpty() ? chatAnswer : chatAnswers.takeFirst();
        const int id = nextRequestId();
        QTimer::singleShot(chatDelay, this, [this, id, answer] {
            Q_EMIT chatFinished(id, chatError.isEmpty() ? answer : QString(), chatError);
        });
        return id;
    }

    int embed(const QString &text) override
    {
        texts.append(text);
        const bool refused = !refusedText.isEmpty() && text == refusedText;
        const QString error = refused ? refusalReason : embedError;
        const AiFailure failure = refused ? AiFailure::Refused : embedFailure;
        // A call that is refused takes no vector out of the list: what it was
        // going to answer belongs to the next note that is answered at all.
        const QList<double> vector = (error.isEmpty() && !embedVectors.isEmpty()) ? embedVectors.takeFirst() : embedVector;
        const int id = nextRequestId();
        QTimer::singleShot(embedDelay, this, [this, id, vector, error, failure] {
            Q_EMIT embedFinished(id,
                                 error.isEmpty() ? vector : QList<double>(),
                                 error,
                                 error.isEmpty() ? AiFailure::None : failure);
        });
        return id;
    }
};
