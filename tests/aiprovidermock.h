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
    /** Non-empty turns the call into a failure carrying exactly this reason. */
    QString chatError;
    QString embedError;
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
        const int id = nextRequestId();
        QTimer::singleShot(embedDelay, this, [this, id] {
            Q_EMIT embedFinished(id, embedError.isEmpty() ? embedVector : QList<double>(), embedError);
        });
        return id;
    }
};
