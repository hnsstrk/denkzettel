#include "aiprovidermock.h"

#include "analysis/ollamaprovider.h"

#include <KLocalizedString>

#include <QSignalSpy>
#include <QTest>

#include <chrono>

/**
 * The AI provider interface, its Ollama reply mapping and the connection test
 * (SPEC 7.1, issue #13).
 *
 * **No server is talked to here and none may be.** The automated run has no
 * Ollama, and a check that reached for one would be green on this machine and
 * red on every other. What is checked is what breaks silently: the road from a
 * finished reply to either a value or a sentence the user can act on, and that
 * the connection test measures two calls rather than reporting one number
 * twice. That a real Ollama answers at all is measured by hand on the
 * development machine, and its numbers stand in the commit message.
 *
 * The mapping is checked as a pure function on made-up replies, because that
 * is the only way to hand it the cases that matter: a timeout, an HTTP status,
 * a body that is not JSON. A running server produces none of them on demand.
 */
class AiTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void chatAnswerIsReadOutOfTheBody();
    void embeddingIsReadOutOfTheBody();
    void timeoutIsNamedAsOne();
    void ollamasOwnRefusalBeatsTheStatusCode();
    void unreachableServerNamesTheTransport();
    void httpStatusIsReported();
    void unreadableBodyIsReported();
    void missingFieldIsNotSuccess();

    void defaultsAreTheModelsOfTheSpec();

    void connectionTestMeasuresBothCallsSeparately();
    void connectionTestReportsTheFirstError();
    void connectionTestSkipsTheEmbeddingAfterAFailedChat();
};

void AiTest::initTestCase()
{
    // The error sentences below are compared in the source language. Without
    // the domain an installed German catalogue would reach the run through
    // XDG_DATA_DIRS; the LANGUAGE pin in tests/CMakeLists.txt is the other half.
    KLocalizedString::setApplicationDomain("denkzettel");
}

void AiTest::chatAnswerIsReadOutOfTheBody()
{
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Chat,
                                                QNetworkReply::NoError,
                                                QString(),
                                                200,
                                                R"({"message":{"role":"assistant","content":"Hallo, Grüße"}})");
    QCOMPARE(answer.error, QString());
    QCOMPARE(answer.text, QStringLiteral("Hallo, Grüße"));
}

void AiTest::embeddingIsReadOutOfTheBody()
{
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Embed,
                                                QNetworkReply::NoError,
                                                QString(),
                                                200,
                                                R"({"embeddings":[[0.5,-0.25,0.125]]})");
    QCOMPARE(answer.error, QString());
    QCOMPARE(answer.vector, QList<double>({0.5, -0.25, 0.125}));
}

void AiTest::timeoutIsNamedAsOne()
{
    // What setTransferTimeout really produces, measured on 2026-08-29 against
    // a real Ollama with the limit at 5 ms. Qt's errorString for it says
    // "Zeitüberschreitung" in a German session, which the "could not be
    // reached" case below would have wrapped in the wrong sentence.
    const OllamaAnswer timedOut = readOllamaReply(OllamaCall::Chat,
                                                  QNetworkReply::TimeoutError,
                                                  QStringLiteral("Connection timed out"),
                                                  0,
                                                  QByteArray());
    QCOMPARE(timedOut.error, QStringLiteral("Ollama did not answer within the time limit."));
    QCOMPARE(timedOut.text, QString());

    // And an abort, which is what Qt's documentation of setTransferTimeout
    // names. Same statement to the user.
    const OllamaAnswer aborted = readOllamaReply(OllamaCall::Chat,
                                                 QNetworkReply::OperationCanceledError,
                                                 QStringLiteral("Operation canceled"),
                                                 0,
                                                 QByteArray());
    QCOMPARE(aborted.error, QStringLiteral("Ollama did not answer within the time limit."));
}

void AiTest::ollamasOwnRefusalBeatsTheStatusCode()
{
    // A model that is not pulled: HTTP 404 with the reason in the body. Reported
    // as "HTTP 404" the user would go looking for a network fault.
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Chat,
                                                QNetworkReply::ContentNotFoundError,
                                                QStringLiteral("Not Found"),
                                                404,
                                                R"({"error":"model \"qwen3:8b\" not found"})");
    QCOMPARE(answer.error, QStringLiteral("Ollama refused the request: model \"qwen3:8b\" not found"));
}

void AiTest::unreachableServerNamesTheTransport()
{
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Embed,
                                                QNetworkReply::ConnectionRefusedError,
                                                QStringLiteral("Connection refused"),
                                                0,
                                                QByteArray());
    QCOMPARE(answer.error, QStringLiteral("Ollama could not be reached: Connection refused"));
}

void AiTest::httpStatusIsReported()
{
    // A status with nothing readable beside it — what a proxy in front of the
    // address answers with.
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Chat,
                                                QNetworkReply::InternalServerError,
                                                QStringLiteral("Internal Server Error"),
                                                503,
                                                QByteArray("<html>Service Unavailable</html>"));
    QCOMPARE(answer.error, QStringLiteral("Ollama answered with HTTP status 503."));
}

void AiTest::unreadableBodyIsReported()
{
    // A success status over something that is not JSON: a wrong port that does
    // answer, and the case a look at the status alone would call an answer.
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Chat,
                                                QNetworkReply::NoError,
                                                QString(),
                                                200,
                                                QByteArray("<html><body>It works!</body></html>"));
    QVERIFY2(answer.error.startsWith(QStringLiteral("Ollama sent an unreadable answer: ")),
             qPrintable(answer.error));
    QCOMPARE(answer.text, QString());
}

void AiTest::missingFieldIsNotSuccess()
{
    // HTTP 200 and valid JSON, and still no answer in it. This is the case a
    // return value would have hidden (CLAUDE.md, findings 24 and 26).
    const OllamaAnswer chat = readOllamaReply(OllamaCall::Chat,
                                              QNetworkReply::NoError,
                                              QString(),
                                              200,
                                              R"({"model":"qwen3:8b","done":true})");
    QCOMPARE(chat.error, QStringLiteral("Ollama's answer carried no text."));

    const OllamaAnswer embed = readOllamaReply(OllamaCall::Embed,
                                               QNetworkReply::NoError,
                                               QString(),
                                               200,
                                               R"({"model":"bge-m3","embeddings":[]})");
    QCOMPARE(embed.error, QStringLiteral("Ollama's answer carried no embedding."));
    QVERIFY(embed.vector.isEmpty());
}

void AiTest::defaultsAreTheModelsOfTheSpec()
{
    // Read back out of a fresh provider, not out of the source: the road from
    // SPEC 7.1 through denkzettelrc into the object is what the acceptance
    // criterion is about. XDG_CONFIG_HOME points at an empty directory (see
    // tests/CMakeLists.txt), so what arrives here is the default and not what
    // the machine running the check happens to have configured.
    const OllamaProvider provider;
    QCOMPARE(provider.chatModel(), QStringLiteral("qwen3:8b"));
    QCOMPARE(provider.embeddingModel(), QStringLiteral("bge-m3"));
}

void AiTest::connectionTestMeasuresBothCallsSeparately()
{
    AiProviderMock provider;
    provider.chatDelay = std::chrono::milliseconds(200);
    provider.embedDelay = std::chrono::milliseconds(20);

    QSignalSpy tested(&provider, &AiProvider::connectionTested);
    provider.testConnection();

    // Both calls have to be waited for, and the chat one goes out first.
    QVERIFY(tested.wait(std::chrono::seconds(5)));
    QCOMPARE(tested.count(), 1);

    const qint64 chatMilliseconds = tested.constFirst().at(0).toLongLong();
    const qint64 embedMilliseconds = tested.constFirst().at(1).toLongLong();
    QCOMPARE(tested.constFirst().at(2).toString(), QString());

    // Held against numbers set from outside the measurement (CLAUDE.md,
    // finding 10): the mock waits 200 ms over the chat and 20 ms over the
    // embedding, and the two are told apart on purpose. A clock that is never
    // restarted reports the embedding as everything since testConnection() —
    // upwards of 220 ms, which "greater than zero" would have called correct.
    QVERIFY2(chatMilliseconds >= 190, qPrintable(QString::number(chatMilliseconds)));
    QVERIFY2(embedMilliseconds >= 15, qPrintable(QString::number(embedMilliseconds)));
    QVERIFY2(embedMilliseconds < 150,
             qPrintable(QStringLiteral("chat %1, embed %2").arg(chatMilliseconds).arg(embedMilliseconds)));

    QCOMPARE(provider.prompts.count(), 1);
    QCOMPARE(provider.texts.count(), 1);
}

void AiTest::connectionTestReportsTheFirstError()
{
    AiProviderMock provider;
    provider.embedError = QStringLiteral("Ollama refused the request: model \"bge-m3\" not found");

    QSignalSpy tested(&provider, &AiProvider::connectionTested);
    provider.testConnection();

    QVERIFY(tested.wait(std::chrono::seconds(5)));
    QCOMPARE(tested.constFirst().at(2).toString(), provider.embedError);
    // No half-answer: a failed test reports no latency at all.
    QCOMPARE(tested.constFirst().at(0).toLongLong(), -1);
    QCOMPARE(tested.constFirst().at(1).toLongLong(), -1);
}

void AiTest::connectionTestSkipsTheEmbeddingAfterAFailedChat()
{
    AiProviderMock provider;
    provider.chatError = QStringLiteral("Ollama could not be reached: Connection refused");

    QSignalSpy tested(&provider, &AiProvider::connectionTested);
    provider.testConnection();

    QVERIFY(tested.wait(std::chrono::seconds(5)));
    QCOMPARE(tested.constFirst().at(2).toString(), provider.chatError);
    QVERIFY(provider.texts.isEmpty());
}

QTEST_GUILESS_MAIN(AiTest)

#include "aitest.moc"
