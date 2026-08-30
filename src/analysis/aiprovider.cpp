#include "analysis/aiprovider.h"

namespace
{
/**
 * What the connection test asks with — short on purpose.
 *
 * SPEC 7.1 calls for a "mini chat call": what is measured is whether the
 * backend answers and how long it takes to start, not what it can say. A long
 * prompt would put the model's generation time into the latency and tell the
 * user nothing about their configuration.
 */
QString probe()
{
    return QStringLiteral("ping");
}
}

AiProvider::AiProvider(QObject *parent)
    : QObject(parent)
{
    // The connection test is written as a listener on the two ordinary answer
    // signals rather than as a road of its own: a backend that answers chat()
    // and embed() is tested by exactly those two calls, and an implementation
    // cannot forget to report to the test.
    connect(this, &AiProvider::chatFinished, this, [this](int id, const QString &, const QString &error) {
        if (id != m_testChatId) {
            return;
        }
        m_testChatId = -1;
        if (!error.isEmpty()) {
            Q_EMIT connectionTested(-1, -1, error);
            return;
        }
        m_testChatMilliseconds = m_testClock.elapsed();

        // A backend that does not embed is not asked for a vector, and the
        // test is over with the one call it could make (SPEC 7.1, issue #38).
        // -1 for the second latency is what the page reads as "not measured
        // here"; the alternative, asking anyway, would report openrouter as
        // broken for doing exactly what it says it does.
        if (!canEmbed()) {
            Q_EMIT connectionTested(m_testChatMilliseconds, -1, QString());
            return;
        }

        m_testClock.restart();
        m_testEmbedId = embed(probe());
    });

    connect(this, &AiProvider::embedFinished, this, [this](int id, const QList<double> &, const QString &error) {
        if (id != m_testEmbedId) {
            return;
        }
        m_testEmbedId = -1;
        if (!error.isEmpty()) {
            Q_EMIT connectionTested(-1, -1, error);
            return;
        }
        Q_EMIT connectionTested(m_testChatMilliseconds, m_testClock.elapsed(), QString());
    });
}

AiProvider::~AiProvider() = default;

bool AiProvider::canEmbed() const
{
    return true;
}

QString AiProvider::unmetPrecondition() const
{
    return {};
}

void AiProvider::testConnection()
{
    m_testEmbedId = -1;
    m_testChatMilliseconds = -1;
    m_testClock.start();
    m_testChatId = chat(probe());
}

int AiProvider::nextRequestId()
{
    return ++m_lastRequestId;
}
