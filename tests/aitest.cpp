#include "aiprovidermock.h"

#include "analysis/analysisscheduler.h"
#include "analysis/classifier.h"
#include "analysis/clustering.h"
#include "analysis/embedder.h"
#include "analysis/ollamaprovider.h"
#include "analysis/suggester.h"
#include "store/store.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QtMath>

#include <chrono>
#include <cmath>
#include <memory>
#include <utility>

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
    void streamedChatAnswerIsPutBackTogether();
    void streamCutOffAtALineBoundaryIsNoAnswer();
    void aCallOverTheTotalLimitSaysSo();
    void embeddingIsReadOutOfTheBody();
    void timeoutIsNamedAsOne();
    void ollamasOwnRefusalBeatsTheStatusCode();
    void unreachableServerNamesTheTransport();
    void httpStatusIsReported();
    void unreadableBodyIsReported();
    void missingFieldIsNotSuccess();

    void defaultsAreTheModelsOfTheSpec();

    void nothingIsAskedTwiceWhenNobodyAnswers();
    void aServerThatAnsweredIsNotAskedAgain();
    void theTotalLimitEndsACallThatKeepsTrickling();
    void readsReachabilityWithoutLoadingAModel();
    void aChangedAddressAndModelReachTheRunningProvider();

    void connectionTestMeasuresBothCallsSeparately();
    void connectionTestReportsTheFirstError();
    void connectionTestSkipsTheEmbeddingAfterAFailedChat();

    void readsTheAnswerBesideAThinkingBlock();
    void unclosedThinkingBlockCarriesNoAnswer();
    void braceInTheProseIsNotTheAnswer();
    void categoryOutsideTheListIsRefused();
    void categoryCaseIsFolded();
    void tagsAreLoweredDedupedAndCutToFour();
    void answerWithoutATagIsRefused();
    void categoryThatIsNoTextIsNamedAnyway();
    void markerInsideATagDoesNotCut();
    void todoWithoutADescriptionKeepsItsCategory();
    void taskSurvivesAnIsTodoThatIsNoBool();
    void taskKeepsOnlyWhatTheNoteSaid();
    void theNotesOwnDayStandsInThePrompt();
    void aDueDateOutsideTheNotesReachIsDropped();

    void everyNoteGetsItsOwnAnswer();
    void noteThatFailedTwiceIsSkippedAndReported();
    void theErrorCountSurvivesARestart();
    void successClearsTheErrorCount();

    void theBudgetStopsAtFiftyAndTheRestFollows();
    void theTriggerFollowsTheSetting();
    void aNoteWrittenDuringARunIsNotLost();

    void cosineSimilarityIsTheAngleBetweenTwoVectors();
    void chainedNotesLandInOneCluster();
    void similarityAtTheThresholdStillCounts();
    void distantNotesFormTheirOwnCluster();
    void nothingToClusterYieldsNoCluster();
    void bundleThresholdIsClampedToWhatTheDialogAllows();

    void everyNoteGetsItsOwnVector();
    void aVectorIsStoredUnderTheModelItWasAskedOf();
    void aRefusedNoteDoesNotBlockTheOthers();
    void anUnreachableBackendCostsNoAttempt();

    void aClusterBecomesAnOpenBundleSuggestion();
    void theModelMayDropAnOutlier();
    void aNoteWithTaskFieldsBecomesATaskSuggestion();
    void aDeferredBundleIsClusteredAgainAndReplaced();
};

void AiTest::initTestCase()
{
    // The error sentences below are compared in the source language. Without
    // the domain an installed German catalogue would reach the run through
    // XDG_DATA_DIRS; the LANGUAGE pin in tests/CMakeLists.txt is the other half.
    KLocalizedString::setApplicationDomain("denkzettel");

    // theTriggerFollowsTheSetting() writes a trigger into KSharedConfig, and
    // KSharedConfig writes what it holds to disk when the process ends. A case
    // that dies before its deleteGroup() therefore leaves a real value behind,
    // and the NEXT run would read it as a setting somebody made — and read it
    // before the case that sets one has run, which is the half that would look
    // like a fault of the scheduler.
    //
    // Before the first KSharedConfig::openConfig() of the run, or the stale
    // value would already be in memory and deleting the file would change
    // nothing. The name is built rather than asked for that reason.
    //
    // The guard is not decoration: without XDG_CONFIG_HOME pointing into the
    // build directory (tests/CMakeLists.txt) this line would delete a
    // configuration file of whoever runs the check.
    const QByteArray configHome = qgetenv("XDG_CONFIG_HOME");
    QVERIFY2(!configHome.isEmpty(), "XDG_CONFIG_HOME has to point into the build directory");
    const QString file = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QLatin1Char('/') + QCoreApplication::applicationName() + QStringLiteral("rc");
    QVERIFY(file.startsWith(QString::fromLocal8Bit(configHome)));
    QFile::remove(file);
}

void AiTest::chatAnswerIsReadOutOfTheBody()
{
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Chat,
                                                QNetworkReply::NoError,
                                                QString(),
                                                200,
                                                R"({"message":{"role":"assistant","content":"Hallo, Grüße"},"done":true})");
    QCOMPARE(answer.error, QString());
    QCOMPARE(answer.text, QStringLiteral("Hallo, Grüße"));
}

namespace
{
/**
 * Lines out of one recorded stream, so that every case below reads what the
 * server really sends (issue #121).
 *
 * Recorded on 2026-08-30 from Ollama 0.32.15 with `qwen3:8b` over `/api/chat`
 * with `stream: true` and **nothing else set** — which is the call
 * `OllamaProvider::chat()` makes. The recording has 218 lines; these are its
 * lines 1, 2, 215, 216, 217 and 218, and the 212 reasoning lines between the
 * second and the third are left out. Nothing inside a line is changed.
 *
 * Two things about it are the server's decision and not a reader's guess, and
 * both are why the body is recorded rather than written: „Grüße" arrives in
 * three pieces with the umlaut a piece of its own, and the reasoning arrives
 * in a `thinking` field of its own beside an empty `content` (CLAUDE.md,
 * finding 45). A body assembled by hand would have put the cut where somebody
 * thought it likely and carried no reasoning at all.
 */
constexpr const char *thinkingLine1 =
    R"({"model":"qwen3:8b","created_at":"2026-08-30T07:15:08.901473844Z","message":{"role":"assistant","content":"","thinking":"Okay"},"done":false})";
constexpr const char *thinkingLine2 =
    R"({"model":"qwen3:8b","created_at":"2026-08-30T07:15:08.912065757Z","message":{"role":"assistant","content":"","thinking":","},"done":false})";
constexpr const char *textLine1 =
    R"({"model":"qwen3:8b","created_at":"2026-08-30T07:15:11.198964902Z","message":{"role":"assistant","content":"Gr"},"done":false})";
constexpr const char *textLine2 =
    R"({"model":"qwen3:8b","created_at":"2026-08-30T07:15:11.209637558Z","message":{"role":"assistant","content":"ü"},"done":false})";
constexpr const char *textLine3 =
    R"({"model":"qwen3:8b","created_at":"2026-08-30T07:15:11.220228942Z","message":{"role":"assistant","content":"ße"},"done":false})";
constexpr const char *doneLine =
    R"({"model":"qwen3:8b","created_at":"2026-08-30T07:15:11.231002909Z","message":{"role":"assistant","content":""},"done":true,)"
    R"("done_reason":"stop","total_duration":36067140899,"load_duration":1027214,"prompt_eval_count":21,)"
    R"("prompt_eval_duration":25430000,"eval_count":222,"eval_duration":2355064000})";

QByteArray streamOf(const QList<QByteArray> &lines)
{
    QByteArray body;
    for (const QByteArray &line : lines) {
        body += line + '\n';
    }
    return body;
}
}

void AiTest::streamedChatAnswerIsPutBackTogether()
{
    // Two things break here without a sound, and neither reaches the user as
    // itself: a reader that takes only the first line or only the last gets a
    // piece of the answer that is still valid text, and a reader that takes
    // `thinking` along puts the model's reasoning into the JSON the classifier
    // parses. Both come out as "the model answered nonsense" and cost the note
    // an attempt (SPEC 7.2).
    const OllamaAnswer answer = readOllamaReply(
        OllamaCall::Chat,
        QNetworkReply::NoError,
        QString(),
        200,
        streamOf({thinkingLine1, thinkingLine2, textLine1, textLine2, textLine3, doneLine}));

    QCOMPARE(answer.error, QString());
    // The reasoning of the two lines above is not in it — `Okay` and `,` would
    // stand in front of the word if it were.
    QCOMPARE(answer.text, QStringLiteral("Grüße"));
}

void AiTest::streamCutOffAtALineBoundaryIsNoAnswer()
{
    // **The case the streaming created** (found in review, 30.08.2026). A
    // connection that goes away between two lines leaves a body in which every
    // line parses, the status is 200, and half the answer is missing — and
    // nothing but the absent `done` says so. Unstreamed it could not happen: a
    // cut-off document has no closing brace.
    //
    // What it cost before the reader asked for `done`: this body came back as
    // `text = "Grüße"` with no error at all, the classifier read a fragment of
    // JSON, and the note paid one of the two attempts of SPEC 7.2 — the very
    // currency issue #121 is about — with an error naming the model.
    const OllamaAnswer answer = readOllamaReply(
        OllamaCall::Chat,
        QNetworkReply::RemoteHostClosedError,
        QStringLiteral("Connection closed"),
        // 200, because the headers did arrive before the connection went away.
        200,
        streamOf({thinkingLine1, thinkingLine2, textLine1, textLine2, textLine3}));

    QCOMPARE(answer.error, QStringLiteral("Ollama's answer broke off."));
    QCOMPARE(answer.text, QString());
    // Unreachable and not Refused: the server refused nothing, the transfer
    // stopped — and the embedding run of SPEC 7.2 decides on this value.
    QCOMPARE(answer.failure, AiFailure::Unreachable);
}

void AiTest::aCallOverTheTotalLimitSaysSo()
{
    // The value an abort() on a running reply really produces, and it is in
    // this case because it was measured rather than read (CLAUDE.md, finding
    // 40): on 2026-08-30 against a real Ollama a streamed call aborted after
    // 3 s came back `OperationCanceledError` at 2,849 ms, while the same
    // program with its timer set past the end of the answer ran 38,537 ms to
    // `NoError`.
    //
    // Until that day the mapping put this value in with the silence limit and
    // a comment said nothing in the program could reach it. The total limit of
    // SPEC 7.1 reaches it, and it needs a sentence of its own: this server did
    // answer, it only took too long, and "did not answer within the time
    // limit" would send the user looking for a server that is running.
    const OllamaAnswer answer = readOllamaReply(OllamaCall::Chat,
                                                QNetworkReply::OperationCanceledError,
                                                QStringLiteral("Operation canceled"),
                                                200,
                                                QByteArray());

    QCOMPARE(answer.error, QStringLiteral("Ollama took longer over this call than it is allowed."));
    QCOMPARE(answer.failure, AiFailure::Unreachable);
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

    // OperationCanceledError, which Qt's documentation names for the same
    // limit, is deliberately NOT asserted here. Nothing in this program can
    // produce it — it takes an abort() or a close() on a running reply, and
    // there is none in the tree — so a case handing it in would be finding 40
    // written into the very set that records it: a guarantee over a value the
    // code never sees. The mapping keeps the value; the comment there says
    // what would reach it.
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

void AiTest::nothingIsAskedTwiceWhenNobodyAnswers()
{
    // The one guarantee of this class that would fail without a sound: a
    // second HTTP request nobody sees. Until 2026-08-29 the provider repeated
    // a request whose limit had been hit, which turned the 30 s of SPEC 7.1
    // into a minute per call and put a second generation — and a second bill —
    // on every provider that charges (SPEC 7.1 names openrouter and OpenAI
    // beside Ollama).
    //
    // A QTcpServer in this process, on the loopback interface: no name is
    // resolved, no route is used and nothing leaves the machine. **It does
    // need loopback to be up**, which a bare `unshare -rn` does not give — the
    // interface exists there and is DOWN (measured 2026-08-29). Whoever wants
    // to repeat that demonstration brings it up first:
    // `unshare -rn sh -c 'ip link set lo up; ./build/bin/aitest'`.
    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));

    int connections = 0;
    // Accepted and then held for ever. A socket that is merely accepted and
    // never answered is what a server behind a hung model looks like, and it
    // is the only thing that makes the limit the thing under test.
    connect(&server, &QTcpServer::newConnection, this, [&server, &connections] {
        ++connections;
        server.nextPendingConnection();
    });

    OllamaProvider provider;
    provider.setUrl(QUrl(QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())));
    provider.setTimeout(std::chrono::milliseconds(300));

    QSignalSpy finished(&provider, &AiProvider::chatFinished);
    QElapsedTimer wall;
    wall.start();
    provider.chat(QStringLiteral("ping"));

    QVERIFY(finished.wait(std::chrono::seconds(10)));
    QCOMPARE(finished.constFirst().at(2).toString(),
             QStringLiteral("Ollama did not answer within the time limit."));

    // The count is the finding, and the clock is the second reading of it: a
    // repeat cannot hide inside a limit that is served twice.
    QCOMPARE(connections, 1);
    QVERIFY2(wall.elapsed() < 600, qPrintable(QString::number(wall.elapsed())));
}

void AiTest::aServerThatAnsweredIsNotAskedAgain()
{
    // The other half of the same guarantee, and the guard on the ceiling the
    // old comment named: whoever adds an HTTP 5xx to a repeat later finds this
    // case red. A server that answered has had the request; asking again is a
    // second job, not a second chance.
    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));

    int connections = 0;
    connect(&server, &QTcpServer::newConnection, this, [&server, &connections] {
        ++connections;
        QTcpSocket *socket = server.nextPendingConnection();
        // Answered once the headers are complete, not on the first byte that
        // arrives: a POST body may follow in a segment of its own.
        auto request = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, socket, [socket, request] {
            request->append(socket->readAll());
            if (!request->contains("\r\n\r\n")) {
                return;
            }
            socket->write("HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
            socket->disconnectFromHost();
        });
    });

    OllamaProvider provider;
    provider.setUrl(QUrl(QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())));
    provider.setTimeout(std::chrono::seconds(5));

    QSignalSpy finished(&provider, &AiProvider::embedFinished);
    provider.embed(QStringLiteral("ping"));

    QVERIFY(finished.wait(std::chrono::seconds(10)));
    QCOMPARE(finished.constFirst().at(2).toString(), QStringLiteral("Ollama answered with HTTP status 503."));
    QCOMPARE(connections, 1);
}

void AiTest::theTotalLimitEndsACallThatKeepsTrickling()
{
    // **The limit the streaming made necessary** (SPEC 7.1, decision
    // 30.08.2026). Since the chat call streams, setTimeout() bounds a stretch
    // of silence and no longer bounds the call: this stand-in sends its headers
    // and then one byte every 20 ms and never finishes, so the silence limit
    // never bites and, without the total limit, the analysis run would wait for
    // ever — nothing else in the tree ends a call.
    //
    // Chosen so the two limits cannot be confused: the silence limit is set to
    // ten seconds and the total limit to 300 ms. Were the trickle to stop, the
    // silence limit would fire and the sentence would be the other one.
    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));

    QList<QTcpSocket *> sockets;
    connect(&server, &QTcpServer::newConnection, this, [&server, &sockets] {
        QTcpSocket *socket = server.nextPendingConnection();
        sockets.append(socket);
        auto request = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, socket, [socket, request] {
            request->append(socket->readAll());
            if (!request->contains("\r\n\r\n")) {
                return;
            }
            // Chunked, so nothing announces an end the client could wait for.
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/x-ndjson\r\n"
                          "Transfer-Encoding: chunked\r\n\r\n");
            auto *trickle = new QTimer(socket);
            connect(trickle, &QTimer::timeout, socket, [socket] {
                socket->write("1\r\n \r\n");
                socket->flush();
            });
            trickle->start(std::chrono::milliseconds(20));
        });
    });

    const auto closeTheSockets = qScopeGuard([&sockets] {
        for (QTcpSocket *socket : std::as_const(sockets)) {
            socket->abort();
        }
    });

    OllamaProvider provider;
    provider.setUrl(QUrl(QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())));
    provider.setTimeout(std::chrono::seconds(10));

    // **The counter-run first, and it is what makes the rest evidence**: with
    // the total limit out of reach the same stand-in holds the same call open,
    // and nothing answers. Asserted before the limit is lowered, or "the limit
    // ended it" would be green over a call that ended by itself (CLAUDE.md,
    // finding 27).
    provider.setCallLimit(std::chrono::seconds(30));
    QSignalSpy trickling(&provider, &AiProvider::chatFinished);
    provider.chat(QStringLiteral("ping"));
    QVERIFY2(!trickling.wait(std::chrono::milliseconds(600)), "the trickling call ended without any limit");

    provider.setCallLimit(std::chrono::milliseconds(300));
    QSignalSpy limited(&provider, &AiProvider::chatFinished);
    provider.chat(QStringLiteral("ping"));

    QVERIFY(limited.wait(std::chrono::seconds(5)));
    QCOMPARE(limited.constFirst().at(2).toString(),
             QStringLiteral("Ollama took longer over this call than it is allowed."));
}

void AiTest::readsReachabilityWithoutLoadingAModel()
{
    // What the tray tooltip of SPEC 2.5 asks at every start (issue #17), and
    // what it must NOT do while asking: testConnection() next door makes a
    // real chat and a real embed call, which measured 3.08 s and 1.64 s
    // against a running Ollama on 2026-08-29 and pins two models for one line
    // of tooltip. So this is checked on two things — that it answers at all,
    // and that what went over the wire was `/api/tags` and nothing naming a
    // model.
    QTcpServer server;
    QVERIFY2(server.listen(QHostAddress::LocalHost), qPrintable(server.errorString()));

    // Shared and not captured by reference: the lambda outlives this frame as
    // far as anything but a reading of the code can tell, and the CI fails on
    // the clazy warning that says so.
    auto asked = std::make_shared<QString>();
    connect(&server, &QTcpServer::newConnection, this, [&server, asked] {
        QTcpSocket *socket = server.nextPendingConnection();
        auto request = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, socket, [socket, request, asked] {
            request->append(socket->readAll());
            if (!request->contains("\r\n\r\n")) {
                return;
            }
            *asked = QString::fromLatin1(request->left(request->indexOf("\r\n")));
            socket->write("HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\n{\"models\":[]}");
            socket->disconnectFromHost();
        });
    });

    OllamaProvider provider;
    provider.setUrl(QUrl(QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())));
    provider.setTimeout(std::chrono::seconds(5));

    QSignalSpy answered(&provider, &OllamaProvider::reachabilityTested);
    provider.testReachability();
    QVERIFY(answered.wait(std::chrono::seconds(10)));
    QVERIFY2(answered.constFirst().at(0).toBool(), qPrintable(*asked));
    // A GET on `/api/tags`: the endpoint that lists what is pulled and starts
    // nothing. Either of the two that load a model would be a POST.
    QCOMPARE(*asked, QStringLiteral("GET /api/tags HTTP/1.1"));

    // And the case the whole thing is built for, on the same object so that
    // the answer has to come out **different** once: a port nobody listens on.
    // It is the port of the server just closed, so no guess of ours can
    // collide with something this machine happens to be running.
    const quint16 abandoned = server.serverPort();
    server.close();
    provider.setUrl(QUrl(QStringLiteral("http://127.0.0.1:%1").arg(abandoned)));
    provider.testReachability();
    QVERIFY(answered.wait(std::chrono::seconds(10)));
    QVERIFY(!answered.constLast().at(0).toBool());
}

namespace
{
/**
 * A stand-in Ollama that keeps the body of every request it was sent and
 * answers each of them.
 *
 * The one answer serves a chat and an embedding alike, so that a case using
 * this can vary the address and the model and nothing else. It waits for the
 * announced length before it reads: `QNetworkAccessManager` sends the headers
 * and the JSON body in segments of their own, and a recorder that stops at the
 * empty line would write down half a request.
 */
void recordAndAnswer(QTcpServer *server, const std::shared_ptr<QStringList> &asked)
{
    QObject::connect(server, &QTcpServer::newConnection, server, [server, asked] {
        QTcpSocket *socket = server->nextPendingConnection();
        auto request = std::make_shared<QByteArray>();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, request, asked] {
            request->append(socket->readAll());
            const qsizetype headersEnd = request->indexOf("\r\n\r\n");
            if (headersEnd < 0) {
                return;
            }
            const QByteArray announced = QByteArray("Content-Length: ");
            const qsizetype at = request->indexOf(announced);
            const qsizetype length = at < 0
                ? 0
                : request->mid(at + announced.size(), request->indexOf("\r\n", at) - at - announced.size()).toLongLong();
            const QByteArray body = request->mid(headersEnd + 4);
            if (body.size() < length) {
                return;
            }

            asked->append(QString::fromUtf8(body));
            // `done` because every real chat answer carries it and the reader
            // now asks for it (issue #121): without it this stand-in would
            // answer every chat with a stream that broke off.
            const QByteArray answer = R"({"message":{"content":"ok"},"embeddings":[[0.5]],"done":true})";
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
                          + QByteArray::number(answer.size()) + "\r\nConnection: close\r\n\r\n" + answer);
            socket->disconnectFromHost();
        });
    });
}
}

void AiTest::aChangedAddressAndModelReachTheRunningProvider()
{
    // The daemon holds one provider for the whole session, and until issue #119
    // it read the address and the two models once at construction: what the
    // settings dialog wrote reached the next start of the daemon and nothing
    // before it. Silently, and worse than silently — "Test connection" on that
    // same page works with the value out of the form, so it reported the new
    // address as reachable while every analysis run went on talking to the old
    // server.
    //
    // **The old address is asked first and that is asserted before anything
    // changes** (CLAUDE.md, finding 27): "it takes the new value" is green over
    // a provider that never had the old one — the first server has to be shown
    // to have been the one answering.
    QTcpServer chosen;
    QTcpServer replacement;
    QVERIFY2(chosen.listen(QHostAddress::LocalHost), qPrintable(chosen.errorString()));
    QVERIFY2(replacement.listen(QHostAddress::LocalHost), qPrintable(replacement.errorString()));

    auto askedChosen = std::make_shared<QStringList>();
    auto askedReplacement = std::make_shared<QStringList>();
    recordAndAnswer(&chosen, askedChosen);
    recordAndAnswer(&replacement, askedReplacement);

    // Written into the same KSharedConfig instance the provider reads from, so
    // what the file is called plays no part here (CLAUDE.md, finding 42). The
    // model names are made up: what is asserted is which of them arrived, and a
    // real one could arrive because something else put it there.
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    // Taken away again whatever happens below, and that is not tidiness: a case
    // that dies on an assertion would otherwise leave a made-up model in the
    // group everyNoteGetsItsOwnVector() reads its default out of, and the next
    // case would go red on this one's leftovers (CLAUDE.md, finding 47's
    // family — a red that says nothing about the code under it).
    const auto forgetTheGroup = qScopeGuard([&group] {
        group.deleteGroup();
    });

    group.writeEntry("OllamaUrl", QStringLiteral("http://127.0.0.1:%1").arg(chosen.serverPort()));
    group.writeEntry("ChatModel", QStringLiteral("first-language-model"));
    group.writeEntry("EmbeddingModel", QStringLiteral("first-embedding-model"));

    OllamaProvider provider;
    provider.setTimeout(std::chrono::seconds(5));

    QSignalSpy chats(&provider, &AiProvider::chatFinished);
    provider.chat(QStringLiteral("ping"));
    QVERIFY(chats.wait(std::chrono::seconds(10)));
    QCOMPARE(askedChosen->size(), 1);
    QVERIFY2(askedChosen->constFirst().contains(QStringLiteral("first-language-model")),
             qPrintable(askedChosen->constFirst()));

    // Now the dialog writes, and the daemon goes on running.
    group.writeEntry("OllamaUrl", QStringLiteral("http://127.0.0.1:%1").arg(replacement.serverPort()));
    group.writeEntry("ChatModel", QStringLiteral("second-language-model"));
    group.writeEntry("EmbeddingModel", QStringLiteral("second-embedding-model"));
    provider.reloadSettings();

    provider.chat(QStringLiteral("ping"));
    QVERIFY(chats.wait(std::chrono::seconds(10)));
    QCOMPARE(askedReplacement->size(), 1);
    QVERIFY2(askedReplacement->constFirst().contains(QStringLiteral("second-language-model")),
             qPrintable(askedReplacement->constFirst()));

    // The embedding model of SPEC 7.1 takes the same road, and it is a second
    // key: a reload that only re-read the address would stand green above.
    QSignalSpy embeddings(&provider, &AiProvider::embedFinished);
    provider.embed(QStringLiteral("ping"));
    QVERIFY(embeddings.wait(std::chrono::seconds(10)));
    QCOMPARE(askedReplacement->size(), 2);
    QVERIFY2(askedReplacement->constLast().contains(QStringLiteral("second-embedding-model")),
             qPrintable(askedReplacement->constLast()));

    // And the server that used to be asked was not asked again — the one
    // reading that tells a changed address from a second address answering
    // beside the first.
    QCOMPARE(askedChosen->size(), 1);
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

namespace
{
/**
 * A database of its own on a temporary file (SPEC 16).
 *
 * **The user's library is never touched and no note of theirs is ever handed
 * to a model.** Every note below is invented for the occasion, and the model
 * in these checks is the stand-in.
 */
std::unique_ptr<Store> openStore(const QTemporaryDir &directory)
{
    auto store = std::make_unique<Store>(directory.path() + QStringLiteral("/denkzettel.db"));
    return store->open() ? std::move(store) : nullptr;
}

qint64 addNote(Store &store, const QString &content, const QDateTime &createdAt)
{
    Note note;
    note.createdAt = createdAt;
    note.content = content;
    const std::optional<qint64> id = store.addNote(note);
    return id.value_or(-1);
}

/**
 * The day the made-up notes of these checks were written on.
 *
 * Every classification is read against the day of its own note (SPEC 7.2,
 * issue #117), and a case handing today's date in would ask a different
 * question tomorrow.
 */
QDate noteDay()
{
    return QDate(2026, 8, 1);
}

/** A well-formed answer, so that a check varies only what it is about. */
QString answerFor(const QString &category, const QString &tags)
{
    return QStringLiteral(R"({"category": "%1", "tags": [%2], "is_todo": false})").arg(category, tags);
}

/** A note that has been through the classification, so the clustering sees it. */
qint64 addAnalysedNote(Store &store, const QString &content, const QDateTime &createdAt)
{
    Note note;
    note.createdAt = createdAt;
    note.content = content;
    note.state = Note::State::Analysed;
    note.category = QStringLiteral("ideen");
    const std::optional<qint64> id = store.addNote(note);
    return id.value_or(-1);
}

/**
 * The embedding model these checks write beside their vectors and hand to the
 * suggester.
 *
 * Named here rather than read out of Embedder::model(), because the point of
 * that parameter is that the two sides say the same thing: a suggester asking
 * for another model finds an empty corpus and reports nothing at all.
 */
QString testEmbeddingModel()
{
    return QStringLiteral("bge-m3");
}

/**
 * A note that has been through both steps of the run, its vector pointing at
 * `degrees` in the plane — what step 3 finds in the database.
 *
 * The angle rather than made-up numbers, for the reason corpusAt() below uses
 * one: the threshold of 0.60 is 53.1°, so two notes 20° apart belong together
 * and two 110° apart do not.
 */
qint64 addEmbeddedNote(Store &store, const QString &content, const QDateTime &createdAt, double degrees,
                       const QString &task = {})
{
    Note note;
    note.createdAt = createdAt;
    note.content = content;
    note.state = Note::State::Analysed;
    note.category = QStringLiteral("ideen");
    note.task = task;
    const std::optional<qint64> id = store.addNote(note);
    if (!id.has_value()) {
        return -1;
    }
    const double radians = qDegreesToRadians(degrees);
    const QList<float> vector = {float(std::cos(radians)), float(std::sin(radians))};
    return store.setEmbedding(*id, testEmbeddingModel(), vector) ? *id : -1;
}

/** A suggestion already standing when a run begins. */
qint64 addStandingProposal(Store &store, Proposal::Kind kind, Proposal::Status status, const QList<qint64> &noteIds)
{
    Proposal proposal;
    proposal.kind = kind;
    proposal.status = status;
    // Older than anything the run writes, so the order proposals() answers in
    // is the order these checks read them in.
    proposal.createdAt = QDateTime::fromString(QStringLiteral("2026-07-01T08:00:00.000"), Qt::ISODateWithMs);
    proposal.payload = QStringLiteral(R"({"title": "Aus einem frueheren Lauf"})");
    proposal.noteIds = noteIds;
    return store.addProposal(proposal).value_or(-1);
}

/** The `title` and `markdown` of a bundle payload, as the review reads them. */
QJsonObject payloadOf(const Proposal &proposal)
{
    return QJsonDocument::fromJson(proposal.payload.toUtf8()).object();
}

/**
 * A corpus of unit vectors in the plane, one per angle, numbered from 1.
 *
 * Two dimensions and an angle rather than made-up numbers, because that is
 * what makes the cases readable: the threshold of 0.60 is an angle of 53.1°,
 * so two notes 50° apart belong together and two 100° apart do not — and
 * whoever reads the case can work out the answer without a calculator.
 */
QList<NoteEmbedding> corpusAt(const QList<double> &degrees)
{
    QList<NoteEmbedding> corpus;
    for (const double angle : degrees) {
        const double radians = qDegreesToRadians(angle);
        corpus.append({corpus.size() + 1, {float(std::cos(radians)), float(std::sin(radians))}});
    }
    return corpus;
}
}

void AiTest::readsTheAnswerBesideAThinkingBlock()
{
    // The trap is not the block, it is what stands in it: qwen3 weighs the
    // categories out loud and writes drafts while doing so. Whoever takes the
    // first JSON object of the text takes the draft — so the draft here is a
    // **different** valid answer, and the case tells the two apart (CLAUDE.md,
    // finding 34).
    const Classification classification = readClassification(QStringLiteral(
        "<think>Okay, the note mentions rsync. First I thought "
        R"({"category": "ideen", "tags": ["rsync"], "is_todo": false})"
        " but that is wrong, it is a command line.</think>\n"
        "```json\n"
        R"({"category": "cli", "tags": ["rsync", "spiegeln"], "is_todo": false})"
        "\n```"), noteDay());

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.category, QStringLiteral("cli"));
    QCOMPARE(classification.tags, QStringList({QStringLiteral("rsync"), QStringLiteral("spiegeln")}));
}

void AiTest::unclosedThinkingBlockCarriesNoAnswer()
{
    // The answer broke off inside the reasoning. What stands there is a draft
    // by definition, and reading it would put a category on the note that the
    // model never settled on.
    // The draft is a **complete** answer, so that only the cut makes it an
    // error: with an incomplete one the case would be red over a parser that
    // reads the block as well, and green for the wrong reason.
    const Classification classification = readClassification(QStringLiteral(
        "<think>The note asks for something to be done, so probably "
        R"({"category": "todos", "tags": ["backup"], "is_todo": false})"
        " — although"), noteDay());

    QVERIFY2(!classification.error.isEmpty(), qPrintable(classification.category));
    QCOMPARE(classification.category, QString());
}

void AiTest::braceInTheProseIsNotTheAnswer()
{
    // Two things before the answer that a scan from the front would stop at: a
    // brace in a sentence, and a complete JSON object that is not the answer.
    // What tells them from the answer is the `category` key — so an object
    // that carries one is taken as the answer whatever stands in it, and a
    // wrong value is refused by name rather than looked past (see
    // categoryOutsideTheListIsRefused()).
    const Classification classification = readClassification(QStringLiteral(
        "Here is the object { as requested. I used "
        R"({"format": "json", "temperature": 0})"
        ", and the answer is:\n"
        R"({"category": "software", "tags": ["qt", "wayland"], "is_todo": false})"), noteDay());

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.category, QStringLiteral("software"));
}

void AiTest::categoryOutsideTheListIsRefused()
{
    // A sixth category would be a note that no `kat:` search and no sidebar
    // entry ever reaches again (SPEC 6) — so it is not written, and the value
    // stands in the reason.
    const Classification classification = readClassification(answerFor(QStringLiteral("haushalt"), QStringLiteral(R"("kaffee")")), noteDay());

    QVERIFY(classification.error.contains(QStringLiteral("haushalt")));
    QCOMPARE(classification.category, QString());
    QVERIFY(classification.tags.isEmpty());
}

void AiTest::categoryCaseIsFolded()
{
    // Upper case is how the answer is written, not what it says: `kat:` is a
    // literal comparison against the short form (SPEC 6), so it is folded here
    // rather than refused.
    const Classification classification = readClassification(answerFor(QStringLiteral("Persoenlich"), QStringLiteral(R"("geburtstag")")), noteDay());

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.category, QStringLiteral("persoenlich"));
}

void AiTest::tagsAreLoweredDedupedAndCutToFour()
{
    // Six tags come in, one of them twice and in another case. Four go out, in
    // the order they were written — the repetition is not one of them, or the
    // note would carry three tags where the answer offered six different ones.
    const Classification classification =
        readClassification(answerFor(QStringLiteral("ideen"),
                                     QStringLiteral(R"("Zeitleiste", "spuren", "ZEITLEISTE", "  notizen  ", "", "woche", "farbe")")), noteDay());

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.tags,
             QStringList({QStringLiteral("zeitleiste"),
                          QStringLiteral("spuren"),
                          QStringLiteral("notizen"),
                          QStringLiteral("woche")}));
}

void AiTest::answerWithoutATagIsRefused()
{
    const Classification classification = readClassification(answerFor(QStringLiteral("ideen"), QString()), noteDay());

    QVERIFY2(!classification.error.isEmpty(), qPrintable(classification.category));
    QCOMPARE(classification.category, QString());
}

void AiTest::categoryThatIsNoTextIsNamedAnyway()
{
    // The reason goes into `analysis_last_error` and from there into the tray
    // tooltip and the log (SPEC 14). Read out as a string, a number falls out
    // of it and the sentence names an empty pair of quotation marks.
    const Classification classification =
        readClassification(QStringLiteral(R"({"category": 3, "tags": ["backup"], "is_todo": false})"), noteDay());

    QVERIFY2(classification.error.contains(QStringLiteral("3")), qPrintable(classification.error));
}

void AiTest::markerInsideATagDoesNotCut()
{
    // The marker stands inside a JSON string, so it is text of the answer and
    // no end of any reasoning. Cutting there costs the whole answer and an
    // attempt with it — after two of those the note keeps no category at all.
    const Classification classification = readClassification(
        QStringLiteral(R"({"category": "ideen", "tags": ["das </think> steht im text"], "is_todo": false})"), noteDay());

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.category, QStringLiteral("ideen"));
}

void AiTest::todoWithoutADescriptionKeepsItsCategory()
{
    // A flag without fields names no task (SPEC 5.1: `task IS NOT NULL` is the
    // statement). What it must not cost is the classification: the category and
    // the tags of this answer are sound, and refusing them would spend an
    // attempt on an answer that carried what the call was for.
    const Classification classification = readClassification(
        QStringLiteral(R"({"category": "todos", "tags": ["backup"], "is_todo": true, "task": null})"), noteDay());

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.category, QStringLiteral("todos"));
    QCOMPARE(classification.tags, QStringList({QStringLiteral("backup")}));
    QCOMPARE(classification.task, QString());
}

void AiTest::taskSurvivesAnIsTodoThatIsNoBool()
{
    // The other direction, and the silent one: QJsonValue::toBool() answers
    // false for the string "true", so a model that confuses the type used to
    // lose exactly the fields the call went out for — stored as a sound
    // classification, with nothing to say that anything went missing.
    const Classification classification = readClassification(
        QStringLiteral(R"({"category": "todos", "tags": ["backup"], "is_todo": "true",)"
                       R"( "task": {"description": "Platte anstecken", "project": "vault"}})"), noteDay());

    QCOMPARE(classification.error, QString());
    const QJsonObject task = QJsonDocument::fromJson(classification.task.toUtf8()).object();
    QCOMPARE(task.value(QStringLiteral("description")).toString(), QStringLiteral("Platte anstecken"));
    QCOMPARE(task.value(QStringLiteral("project")).toString(), QStringLiteral("vault"));
}

void AiTest::taskKeepsOnlyWhatTheNoteSaid()
{
    // Measured against qwen3:8b on 2026-08-29: for a note reading „Morgen" the
    // model wrote `"due": "2023-10-26"`, a date out of its training. A `due`
    // that is no date and a `priority` outside H/M/L are left out instead of
    // refusing the whole answer — SPEC 7.2 lets both be null — and what the
    // note did say stays.
    const Classification classification = readClassification(QStringLiteral(
        R"({"category": "todos", "tags": ["filter"], "is_todo": true,)"
        R"( "task": {"description": "Wasserfilter tauschen", "project": "Kueche", "tags": ["FILTER"],)"
        R"( "due": "irgendwann", "priority": "dringend", "erfunden": "weg"}})"), noteDay());

    QCOMPARE(classification.error, QString());

    const QJsonObject task = QJsonDocument::fromJson(classification.task.toUtf8()).object();
    QCOMPARE(task.value(QStringLiteral("description")).toString(), QStringLiteral("Wasserfilter tauschen"));
    QCOMPARE(task.value(QStringLiteral("project")).toString(), QStringLiteral("kueche"));
    QCOMPARE(task.value(QStringLiteral("tags")).toArray().first().toString(), QStringLiteral("filter"));
    QVERIFY(!task.contains(QStringLiteral("due")));
    QVERIFY(!task.contains(QStringLiteral("priority")));
    QVERIFY(!task.contains(QStringLiteral("erfunden")));
}

void AiTest::theNotesOwnDayStandsInThePrompt()
{
    // The first half of issue #117. Without a day in the prompt the model is
    // asked to read "Morgen" knowing nothing about when the note was written,
    // and it answers out of its training: measured against qwen3:8b on
    // 2026-08-29 as `"due": "2023-10-26"` for a note of 2026.
    //
    // **The sentence, not the date on its own.** A date can stand anywhere in
    // the prompt without saying whose day it is: with the sentence deleted and
    // the same value left in as a meaningless "Request %2", an assertion on the
    // string "2026-08-01" alone stays green over a prompt that tells the model
    // nothing at all — measured 2026-08-29, whole set green.
    const QString prompt = classificationPrompt(QStringLiteral("Morgen den Wasserfilter tauschen"), noteDay());
    QVERIFY2(prompt.contains(QStringLiteral("The note was written on 2026-08-01, and every relative date in it is read from that day.")),
             qPrintable(prompt));

    // The day of the **note**, not of the run — so the same text asked about a
    // note of another day has to come out different, or the case would be green
    // over a prompt carrying a date from anywhere at all.
    const QString older = classificationPrompt(QStringLiteral("Morgen den Wasserfilter tauschen"), QDate(2025, 3, 4));
    QVERIFY2(older.contains(QStringLiteral("The note was written on 2025-03-04,")), qPrintable(older));
    QVERIFY2(!older.contains(QStringLiteral("2026-08-01")), qPrintable(older));
}

void AiTest::aDueDateOutsideTheNotesReachIsDropped()
{
    // The second half, and the one that assures something about the **answer**:
    // the day in the prompt is a request to a model and no guarantee (CLAUDE.md,
    // finding 50). A date it invents is well-formed and passes every format
    // check — and #29 and #33 carry this field into a real task list, where
    // nothing about the value says it was guessed.
    const auto dueOf = [](const QString &due) {
        const Classification classification = readClassification(
            QStringLiteral(R"({"category": "todos", "tags": ["filter"], "is_todo": true,)"
                           R"( "task": {"description": "Wasserfilter tauschen", "due": "%1"}})")
                .arg(due),
            noteDay());
        return QJsonDocument::fromJson(classification.task.toUtf8())
            .object()
            .value(QStringLiteral("due"))
            .toString();
    };

    // What is kept comes first: a guard that drops everything would satisfy the
    // two refusals below and take the feature with it.
    QCOMPARE(dueOf(QStringLiteral("2026-08-02")), QStringLiteral("2026-08-02"));
    // Both ends of what is allowed, so that a `>` where a `>=` belongs and a
    // year counted one day short come out red rather than unnoticed: the day of
    // the note itself, and the last day of the year after it.
    QCOMPARE(dueOf(QStringLiteral("2026-08-01")), QStringLiteral("2026-08-01"));
    QCOMPARE(dueOf(QStringLiteral("2027-08-01")), QStringLiteral("2027-08-01"));

    // The measured case: a date out of the training data, lying before the note
    // that is supposed to ask for it.
    QCOMPARE(dueOf(QStringLiteral("2023-10-03")), QString());
    // And the other direction, one day past the bound.
    QCOMPARE(dueOf(QStringLiteral("2027-08-02")), QString());
}

void AiTest::everyNoteGetsItsOwnAnswer()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // Four invented notes, oldest first, and four answers that differ in every
    // field. A run that wrote the first answer onto all of them, or that took
    // the notes newest first, comes out red here — with one answer for four
    // notes it could not (CLAUDE.md, finding 34).
    const QDateTime first = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    const qint64 filter = addNote(*store, QStringLiteral("Wasserfilter der Kaffeemaschine tauschen, die Anzeige blinkt."), first);
    const qint64 timeline = addNote(*store, QStringLiteral("Wochennotizen als Zeitleiste zeichnen, eine Spur je Projekt."), first.addSecs(60));
    const qint64 journal = addNote(*store, QStringLiteral("journalctl --user -u denkzetteld -f zeigt den laufenden Dienst."), first.addSecs(120));
    const qint64 flagged = addNote(*store, QStringLiteral("Der Zettel vom Amt liegt noch auf dem Schreibtisch."), first.addSecs(180));
    QVERIFY(filter > 0 && timeline > 0 && journal > 0 && flagged > 0);

    AiProviderMock provider;
    provider.chatAnswers = QStringList({
        QStringLiteral(R"({"category": "todos", "tags": ["filter"], "is_todo": true,)"
                       R"( "task": {"description": "Wasserfilter tauschen", "due": "2026-08-05"}})"),
        answerFor(QStringLiteral("ideen"), QStringLiteral(R"("zeitleiste", "notizen")")),
        answerFor(QStringLiteral("cli"), QStringLiteral(R"("journalctl")")),
        // The flag set and no fields behind it. This is the data consequence
        // the check is here for: refused, the note would keep its attempt
        // counted and stand at `neu` without a category after the second run.
        QStringLiteral(R"({"category": "persoenlich", "tags": ["amt"], "is_todo": true, "task": null})"),
    });

    Classifier classifier(store.get(), &provider);
    QSignalSpy done(&classifier, &Classifier::finished);
    classifier.start();
    QVERIFY(done.wait(std::chrono::seconds(5)));

    QCOMPARE(provider.prompts.size(), 4);
    // The prompt carries the note and binds the model to the five categories
    // of SPEC 6 — the note text without them would be a free-hand answer.
    QVERIFY(provider.prompts.constFirst().contains(QStringLiteral("Wasserfilter")));
    QVERIFY(provider.prompts.constFirst().contains(QStringLiteral("persoenlich")));

    const std::optional<Note> classified = store->note(filter);
    QVERIFY(classified.has_value());
    QCOMPARE(classified->category, QStringLiteral("todos"));
    QCOMPARE(classified->state, Note::State::Analysed);
    QCOMPARE(store->tags(filter), QStringList({QStringLiteral("filter")}));
    QCOMPARE(QJsonDocument::fromJson(classified->task.toUtf8()).object().value(QStringLiteral("due")).toString(),
             QStringLiteral("2026-08-05"));

    QCOMPARE(store->note(timeline)->category, QStringLiteral("ideen"));
    // Store::tags() hands them back sorted, not in the order the model wrote.
    QCOMPARE(store->tags(timeline), QStringList({QStringLiteral("notizen"), QStringLiteral("zeitleiste")}));
    // A note that is no todo carries no task — the flag has no column of its
    // own, so an empty text is the only place that can say so.
    QCOMPARE(store->note(timeline)->task, QString());
    QCOMPARE(store->note(journal)->category, QStringLiteral("cli"));

    // The note whose answer set the flag without filling the fields is
    // classified all the same, and carries no task.
    const std::optional<Note> withoutTask = store->note(flagged);
    QVERIFY(withoutTask.has_value());
    QCOMPARE(withoutTask->state, Note::State::Analysed);
    QCOMPARE(withoutTask->category, QStringLiteral("persoenlich"));
    QCOMPARE(store->tags(flagged), QStringList({QStringLiteral("amt")}));
    QCOMPARE(withoutTask->task, QString());
    QCOMPARE(withoutTask->analysisAttempts, 0);

    // And a second run has nothing left to do.
    provider.prompts.clear();
    const QSignalSpy again(&classifier, &Classifier::finished);
    classifier.start();
    QCOMPARE(again.count(), 1);
    QVERIFY(provider.prompts.isEmpty());
}

void AiTest::noteThatFailedTwiceIsSkippedAndReported()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    const QDateTime when = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);

    // The used-up note is the **older** one, so that a run which simply takes
    // the first note it finds classifies the wrong one. Read the other way
    // round the case would be green over an implementation that skips nothing
    // but the last note (CLAUDE.md, finding 34).
    Note used;
    used.createdAt = when;
    used.content = QStringLiteral("Diese Notiz hat zweimal nichts geliefert.");
    used.analysisAttempts = 2;
    used.analysisLastError = QStringLiteral("Ollama did not answer within the time limit.");
    const std::optional<qint64> skipped = store->addNote(used);
    QVERIFY(skipped.has_value());

    const qint64 fresh = addNote(*store, QStringLiteral("Diese Notiz ist noch dran."), when.addSecs(60));
    QVERIFY(fresh > 0);

    AiProviderMock provider;
    provider.chatAnswers = QStringList({answerFor(QStringLiteral("ideen"), QStringLiteral(R"("notiz")"))});

    Classifier classifier(store.get(), &provider);
    const QSignalSpy reported(&classifier, &Classifier::paused);
    QSignalSpy done(&classifier, &Classifier::finished);
    classifier.start();
    QVERIFY(done.wait(std::chrono::seconds(5)));

    // Exactly one call went out, and it was about the other note.
    QCOMPARE(provider.prompts.size(), 1);
    QVERIFY(provider.prompts.constFirst().contains(QStringLiteral("noch dran")));

    // Skipped, and reported with the reason the database kept (SPEC 14).
    QCOMPARE(reported.size(), 1);
    QCOMPARE(reported.constFirst().at(0).toLongLong(), *skipped);
    QCOMPARE(reported.constFirst().at(1).toString(), used.analysisLastError);

    QCOMPARE(store->note(*skipped)->state, Note::State::New);
    QCOMPARE(store->note(*skipped)->category, QString());
    QCOMPARE(store->note(fresh)->category, QStringLiteral("ideen"));
}

void AiTest::theErrorCountSurvivesARestart()
{
    const QTemporaryDir directory;
    const QDateTime when = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    qint64 noteId = -1;

    {
        const std::unique_ptr<Store> store = openStore(directory);
        QVERIFY(store);
        noteId = addNote(*store, QStringLiteral("Eine Notiz, an der jeder Versuch scheitert."), when);
        QVERIFY(noteId > 0);
    }

    // The stand-in fails **every** call and not only the first: one that
    // recovered by itself could not tell a counter that survives from one that
    // starts afresh (CLAUDE.md, finding 41).
    const QString reason = QStringLiteral("Ollama could not be reached: Connection refused");

    // Three runs, each on a store of its own — that is what a restart of the
    // daemon leaves behind, and the counter comes out different in all three.
    for (int run = 1; run <= 3; ++run) {
        const std::unique_ptr<Store> store = openStore(directory);
        QVERIFY(store);

        AiProviderMock provider;
        provider.chatError = reason;

        Classifier classifier(store.get(), &provider);
        const QSignalSpy attempted(&classifier, &Classifier::failed);
        const QSignalSpy givenUp(&classifier, &Classifier::paused);
        QSignalSpy done(&classifier, &Classifier::finished);
        classifier.start();
        if (done.isEmpty()) {
            QVERIFY(done.wait(std::chrono::seconds(5)));
        }

        const std::optional<Note> note = store->note(noteId);
        QVERIFY(note.has_value());

        if (run == 1) {
            // First failure: counted, reported as an attempt, and the note stays.
            QCOMPARE(provider.prompts.size(), 1);
            QCOMPARE(note->analysisAttempts, 1);
            QCOMPARE(note->analysisLastError, reason);
            QCOMPARE(attempted.size(), 1);
            QVERIFY(givenUp.isEmpty());
        } else if (run == 2) {
            // Second failure: the limit of SPEC 7.2 is reached, and this is
            // where the note is reported as given up on.
            QCOMPARE(provider.prompts.size(), 1);
            QCOMPARE(note->analysisAttempts, 2);
            QVERIFY(attempted.isEmpty());
            QCOMPARE(givenUp.size(), 1);
        } else {
            // Third run: nothing is asked any more, and the count stands where
            // the second run left it — a counter held in memory would be 0 here.
            QVERIFY(provider.prompts.isEmpty());
            QCOMPARE(note->analysisAttempts, 2);
            QCOMPARE(givenUp.size(), 1);
        }
    }
}

void AiTest::successClearsTheErrorCount()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // One attempt behind it and still worth another (SPEC 7.2), so the reset is
    // visible: from 0 nothing could be seen to change.
    Note tried;
    tried.createdAt = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    tried.content = QStringLiteral("Beim ersten Mal kam nichts zurück.");
    tried.analysisAttempts = 1;
    tried.analysisLastError = QStringLiteral("Ollama sent an unreadable answer");
    const std::optional<qint64> noteId = store->addNote(tried);
    QVERIFY(noteId.has_value());

    AiProviderMock provider;
    provider.chatAnswers = QStringList({answerFor(QStringLiteral("software"), QStringLiteral(R"("qt")"))});

    Classifier classifier(store.get(), &provider);
    QSignalSpy done(&classifier, &Classifier::finished);
    classifier.start();
    QVERIFY(done.wait(std::chrono::seconds(5)));

    const std::optional<Note> analysed = store->note(*noteId);
    QVERIFY(analysed.has_value());
    QCOMPARE(analysed->category, QStringLiteral("software"));
    QCOMPARE(analysed->analysisAttempts, 0);
    QCOMPARE(analysed->analysisLastError, QString());
}

void AiTest::theBudgetStopsAtFiftyAndTheRestFollows()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // Ten more than the budget, one minute apart: the timestamps decide the
    // order (Store::unanalysedNotes), so which fifty a run takes can be read
    // off afterwards. Written at the same moment they would fall back on the
    // ids and the two halves could not be told apart.
    const QDateTime when = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    const int total = Classifier::notesPerRun + 10;
    QList<qint64> ids;
    for (int index = 0; index < total; ++index) {
        const qint64 id = addNote(*store,
                                  QStringLiteral("Notiz Nummer %1 aus einer vollen Bibliothek.").arg(index),
                                  when.addSecs(static_cast<qint64>(index) * 60));
        QVERIFY(id > 0);
        ids.append(id);
    }

    AiProviderMock provider;
    // One answer for all of them, unlike everyNoteGetsItsOwnAnswer() above:
    // what is measured here is how many notes a run hands to the model and
    // which, not what it writes onto them.
    provider.chatAnswer = answerFor(QStringLiteral("ideen"), QStringLiteral(R"("notiz")"));

    Classifier classifier(store.get(), &provider);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy done(&classifier, &Classifier::finished);

    // **Waited for is the number of ends the run can still produce**, not one
    // signal (CLAUDE.md, finding 32). `finished` comes synchronously out of
    // start() wherever the queue is empty, and a wait() posted afterwards then
    // sits until it times out over a run that was long through — the failure
    // reads "something hangs" and says nothing about the budget.
    classifier.start();
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 1, 10000);

    // The budget of SPEC 14 bit, and it bit at the front of the list.
    QCOMPARE(provider.prompts.size(), qsizetype(Classifier::notesPerRun));
    QCOMPARE(store->unanalysedNotes().size(), qsizetype(total - Classifier::notesPerRun));
    QCOMPARE(store->note(ids.at(Classifier::notesPerRun - 1))->state, Note::State::Analysed);
    QCOMPARE(store->note(ids.at(Classifier::notesPerRun))->state, Note::State::New);

    // **And the second half, which is the one a check stopping after the first
    // would let through.** A budget that took the surplus away for good — a run
    // that never becomes startable again, a note marked off it never saw —
    // comes out green above and red here.
    classifier.start();
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 2, 10000);

    QCOMPARE(provider.prompts.size(), qsizetype(total));
    QVERIFY(store->unanalysedNotes().isEmpty());
    QCOMPARE(store->note(ids.constLast())->state, Note::State::Analysed);
}

void AiTest::theTriggerFollowsTheSetting()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    const QDateTime when = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);

    AiProviderMock provider;
    provider.chatAnswer = answerFor(QStringLiteral("ideen"), QStringLiteral(R"("notiz")"));

    Classifier classifier(store.get(), &provider);

    // Written into the same KSharedConfig instance the scheduler reads from, so
    // what the file is called plays no part here (CLAUDE.md, finding 42).
    //
    // **What this checks is the scheduler's side and no more**: that it reads
    // the trigger at all and arms what the value names. It compares against the
    // constants of its own library, and it could not do otherwise — the choice
    // list of the settings skeleton is built from those same constants, so
    // there are no two values to hold against each other. The group and the two
    // key names are the duplication that is left, and they carry a comment on
    // both sides rather than an assertion (see analysisscheduler.h).
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Analysis"));

    // "on demand only": a note that is written sets nothing going, and no timer
    // stands either.
    group.writeEntry("Trigger", QString::fromLatin1(analysis::TriggerOnDemand));
    // The two steps behind the classification, because a run is all three
    // (SPEC 7.2). Nothing here reaches them: with fewer notes than the bundle
    // threshold no cluster comes out, so the prompts counted below are the
    // classification's alone.
    Embedder embedder(store.get(), &provider);
    Suggester suggester(store.get(), &provider, embedder.model());
    AnalysisScheduler scheduler(&classifier, &embedder, &suggester);
    QCOMPARE(scheduler.interval(), std::chrono::milliseconds(0));

    QVERIFY(addNote(*store, QStringLiteral("Eine Notiz auf Abruf."), when) > 0);
    scheduler.noteIsReady();
    // The stand-in records the prompt inside chat(), so a run that had started
    // would already stand here — there is nothing to wait for.
    QVERIFY(provider.prompts.isEmpty());

    // ... and the road the tray entry and AnalyzeNow() take runs it all the same.
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy done(&suggester, &Suggester::finished);
    scheduler.analyzeNow();
    // **The end of a run is the end of its LAST step**, not of its first: the
    // classification hands on to the embedding and that to the suggestions
    // (SPEC 7.2), and the scheduler declines a second run until all three are
    // through. Waited for on Classifier::finished, the noteIsReady() below
    // would find the run still going and be remembered instead of started —
    // measured here as 0 prompts against 1 (CLAUDE.md, finding 32).
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 1, 5000);
    QCOMPARE(provider.prompts.size(), 1);

    // "at once after saving": the same call now is a run.
    group.writeEntry("Trigger", QString::fromLatin1(analysis::TriggerAfterSaving));
    scheduler.applySettings();
    QCOMPARE(scheduler.interval(), std::chrono::milliseconds(0));

    const qint64 saved = addNote(*store, QStringLiteral("Eine Notiz sofort nach dem Speichern."), when.addSecs(60));
    QVERIFY(saved > 0);
    provider.prompts.clear();
    scheduler.noteIsReady();
    QCOMPARE(provider.prompts.size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 2, 5000);
    QCOMPARE(store->note(saved)->category, QStringLiteral("ideen"));

    // "periodically": the timer stands at what the setting says, and saving a
    // note is no longer a run.
    group.writeEntry("Trigger", QString::fromLatin1(analysis::TriggerPeriodically));
    group.writeEntry("IntervalMinutes", 45);
    scheduler.applySettings();
    QCOMPARE(scheduler.interval(), std::chrono::milliseconds(std::chrono::minutes(45)));

    provider.prompts.clear();
    QVERIFY(addNote(*store, QStringLiteral("Eine Notiz für den nächsten Durchlauf."), when.addSecs(120)) > 0);
    scheduler.noteIsReady();
    QVERIFY(provider.prompts.isEmpty());

    // A denkzettelrc written by hand never passes the spin box, and a zero
    // there would be a timer firing as fast as the event loop turns.
    group.writeEntry("IntervalMinutes", 0);
    scheduler.applySettings();
    QCOMPARE(scheduler.interval(),
             std::chrono::milliseconds(std::chrono::minutes(analysis::MinimumIntervalMinutes)));

    group.deleteGroup();
}

void AiTest::aNoteWrittenDuringARunIsNotLost()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    const QDateTime when = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(addNote(*store, QStringLiteral("Die erste Notiz des Laufs."), when) > 0);
    QVERIFY(addNote(*store, QStringLiteral("Die zweite Notiz des Laufs."), when.addSecs(60)) > 0);

    AiProviderMock provider;
    provider.chatAnswer = answerFor(QStringLiteral("ideen"), QStringLiteral(R"("notiz")"));
    // Long enough that the note below really arrives during the run: with the
    // answer coming back at once the run would be over before it was written,
    // and the case could not come out red.
    provider.chatDelay = std::chrono::milliseconds(50);
    // Three vectors pointing in three directions, so that step 3 of the run
    // finds no cluster and asks the model nothing: with the stand-in's one
    // default vector all three notes are identical, they chain into a bundle at
    // the threshold of 3, and the prompt count below counts a call this case is
    // not about — measured as 4 against 3, and only when the event loop got far
    // enough between the two, which is worse than wrong (SPEC 7.3).
    provider.embedVectors = {{1.0, 0.0}, {0.0, 1.0}, {-1.0, 0.0}};

    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Analysis"));
    group.writeEntry("Trigger", QString::fromLatin1(analysis::TriggerAfterSaving));

    Classifier classifier(store.get(), &provider);
    Embedder embedder(store.get(), &provider);
    Suggester suggester(store.get(), &provider, embedder.model());
    AnalysisScheduler scheduler(&classifier, &embedder, &suggester);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy done(&classifier, &Classifier::finished);

    scheduler.analyzeNow();
    QVERIFY(classifier.isBusy());

    const qint64 late = addNote(*store, QStringLiteral("Und diese kommt mitten hinein."), when.addSecs(120));
    QVERIFY(late > 0);
    scheduler.noteIsReady();

    // **Waited for is the number of runs this can still produce, not an idle
    // queue** (CLAUDE.md, finding 32): between two notes the classifier is not
    // busy either, and a wait on that would return in the gap and count a
    // half-done run as a finished one.
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 2, 5000);

    // Three calls for three notes: the late one was taken up, and neither of
    // the first two was handed out a second time.
    QCOMPARE(provider.prompts.size(), 3);
    QCOMPARE(store->note(late)->state, Note::State::Analysed);
    QVERIFY(store->unanalysedNotes().isEmpty());

    group.deleteGroup();
}

void AiTest::cosineSimilarityIsTheAngleBetweenTwoVectors()
{
    // (3,4) and (5,0): the dot product is 15, the lengths are 5 and 5, so the
    // cosine is 15/25 — exactly the threshold, and exactly representable, see
    // similarityAtTheThresholdStillCounts() below.
    QCOMPARE(cosineSimilarity({3.0F, 4.0F}, {5.0F, 0.0F}), 0.6);
    // The same direction at a different length is the same topic: the cosine
    // knows the angle and not the size. A dot product without the division
    // would answer 50 here and 15 above.
    QCOMPARE(cosineSimilarity({3.0F, 4.0F}, {6.0F, 8.0F}), 1.0);
    QCOMPARE(cosineSimilarity({3.0F, 4.0F}, {-3.0F, -4.0F}), -1.0);

    // The two that would otherwise divide by nothing or read past the end of a
    // vector — the second is what a BLOB read with the wrong element size
    // looks like (SPEC 5.1: float32).
    QCOMPARE(cosineSimilarity({0.0F, 0.0F}, {5.0F, 0.0F}), 0.0);
    QCOMPARE(cosineSimilarity({3.0F, 4.0F, 5.0F}, {5.0F, 0.0F}), 0.0);
    QCOMPARE(cosineSimilarity({}, {}), 0.0);
}

void AiTest::chainedNotesLandInOneCluster()
{
    // The case the whole method stands or falls by (SPEC 7.3, single linkage):
    // A to B is 50°, B to C is 50°, A to C is 100°. The first two are above the
    // threshold, the third is far below it — so this is **one** cluster of
    // three. Whoever asks every pair of a cluster to be similar gets two
    // clusters of two here and none at a minimum of three, and with only two
    // notes the two readings would be indistinguishable (CLAUDE.md, finding
    // 34).
    const QList<NoteEmbedding> corpus = corpusAt({0.0, 50.0, 100.0});

    QVERIFY(cosineSimilarity(corpus.at(0).vector, corpus.at(1).vector) >= clusterSimilarity);
    QVERIFY(cosineSimilarity(corpus.at(1).vector, corpus.at(2).vector) >= clusterSimilarity);
    QVERIFY(cosineSimilarity(corpus.at(0).vector, corpus.at(2).vector) < clusterSimilarity);

    const QList<QList<qint64>> clusters = clusterNotes(corpus, 3);
    QCOMPARE(clusters.size(), 1);
    QCOMPARE(clusters.constFirst(), QList<qint64>({1, 2, 3}));
}

void AiTest::similarityAtTheThresholdStillCounts()
{
    // SPEC 7.3 says "≥ 0.60", and the difference to "> 0.60" is one pair. The
    // vectors are chosen so the cosine is exactly 0.6 rather than nearly: 3-4-5
    // is exact in float32, 15/25 rounds to the same double the constant does.
    const QList<NoteEmbedding> exactly = {{1, {5.0F, 0.0F}}, {2, {3.0F, 4.0F}}};
    QCOMPARE(cosineSimilarity(exactly.at(0).vector, exactly.at(1).vector), clusterSimilarity);
    QCOMPARE(clusterNotes(exactly, 2), QList<QList<qint64>>({{1, 2}}));

    // And a hair below it is two notes that have nothing to do with each other.
    const QList<NoteEmbedding> below = {{1, {5.0F, 0.0F}}, {2, {2.9F, 4.1F}}};
    QVERIFY(cosineSimilarity(below.at(0).vector, below.at(1).vector) < clusterSimilarity);
    QVERIFY(clusterNotes(below, 2).isEmpty());
}

void AiTest::distantNotesFormTheirOwnCluster()
{
    // Two topics: three notes around 0° to 100°, two more around 180°. The
    // nearest pair across the gap is 80° apart and stays apart.
    const QList<NoteEmbedding> corpus = corpusAt({0.0, 50.0, 100.0, 180.0, 200.0});

    QCOMPARE(clusterNotes(corpus, 2), QList<QList<qint64>>({{1, 2, 3}, {4, 5}}));

    // The bundle threshold of SPEC 7.3 is what drops the smaller one: at three
    // notes the pair is no bundle, and its notes simply stay in the corpus.
    QCOMPARE(clusterNotes(corpus, 3), QList<QList<qint64>>({{1, 2, 3}}));
}

void AiTest::nothingToClusterYieldsNoCluster()
{
    QVERIFY(clusterNotes({}, 3).isEmpty());
    QVERIFY(clusterNotes(corpusAt({0.0}), 3).isEmpty());
    // A note whose vector is empty — a wrongly read BLOB — clusters with
    // nobody, itself included, rather than dragging the corpus together.
    QVERIFY(clusterNotes({{1, {}}, {2, {}}}, 2).isEmpty());
}

void AiTest::bundleThresholdIsClampedToWhatTheDialogAllows()
{
    // SPEC 7.3, default 3. The set runs with a configuration directory that
    // holds nothing (tests/CMakeLists.txt), so what comes back with no entry is
    // the default — and it is `[Export] BundleNotes` the settings page writes
    // (#75).
    QCOMPARE(bundleThreshold(), bundle::DefaultNotes);
    QCOMPARE(bundle::DefaultNotes, 3);

    // Written into the same KSharedConfig instance the reader opens, so what
    // the file is called plays no part (CLAUDE.md, finding 42).
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Export"));

    // A hand-written value the dialog would never have produced. At 1 every
    // note is a cluster of its own and the whole corpus turns into bundles;
    // the item's own bounds do not reach a file nobody edited through the
    // dialog, so the reader clamps.
    group.writeEntry("BundleNotes", 1);
    QCOMPARE(bundleThreshold(), bundle::MinimumNotes);
    group.writeEntry("BundleNotes", 0);
    QCOMPARE(bundleThreshold(), bundle::MinimumNotes);
    group.writeEntry("BundleNotes", -5);
    QCOMPARE(bundleThreshold(), bundle::MinimumNotes);
    group.writeEntry("BundleNotes", 500);
    QCOMPARE(bundleThreshold(), bundle::MaximumNotes);

    // And a value inside the bounds is handed through untouched — without this
    // line a reader that always answered the floor would pass.
    group.writeEntry("BundleNotes", 7);
    QCOMPARE(bundleThreshold(), 7);

    group.deleteGroup();
    QCOMPARE(bundleThreshold(), bundle::DefaultNotes);
}

void AiTest::everyNoteGetsItsOwnVector()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    AiProviderMock provider;
    Embedder embedder(store.get(), &provider);
    // Every embedding comes from Ollama in v1 (SPEC 7.1), and the model stands
    // beside the vector — the clustering asks for the vectors of one model.
    QCOMPARE(embedder.model(), QString(ollama::DefaultEmbeddingModel));

    // Four notes, and only two of them have anything outstanding.
    const QDateTime first = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    const qint64 fresh = addAnalysedNote(*store, QStringLiteral("Regentonne an die Fallrohre hängen."), first);
    const qint64 done = addAnalysedNote(*store, QStringLiteral("Hochbeet im Frühjahr neu schichten."), first.addSecs(60));
    const qint64 edited = addAnalysedNote(*store, QStringLiteral("Der Kompost braucht mehr Braunmaterial."), first.addSecs(120));
    const qint64 unanalysed = addNote(*store, QStringLiteral("Noch nicht klassifiziert."), first.addSecs(180));
    QVERIFY(fresh > 0 && done > 0 && edited > 0 && unanalysed > 0);

    QVERIFY2(store->setEmbedding(done, embedder.model(), {0.125F, 0.25F}), qPrintable(store->lastError()));
    QVERIFY2(store->setEmbedding(edited, embedder.model(), {2.0F, 2.0F}), qPrintable(store->lastError()));
    // What SPEC 9 sets when the user saves an edited note: the text has moved
    // on, the vector has not.
    std::optional<Note> reedit = store->note(edited);
    QVERIFY(reedit.has_value());
    reedit->needsReembed = true;
    QVERIFY2(store->updateNote(*reedit), qPrintable(store->lastError()));

    // Two different answers, so that a run writing the first vector onto every
    // note comes out red (CLAUDE.md, finding 34).
    provider.embedVectors = {{0.5, -0.25, 0.75}, {-1.0, 0.25, 0.5}};

    QSignalSpy done_(&embedder, &Embedder::finished);
    embedder.start();
    QVERIFY(done_.wait(std::chrono::seconds(5)));

    // The note that has a current vector is not asked again, and the one that
    // is not analysed is not asked at all — an `embed` call is paid for.
    QCOMPARE(provider.texts, QStringList({QStringLiteral("Regentonne an die Fallrohre hängen."),
                                          QStringLiteral("Der Kompost braucht mehr Braunmaterial.")}));

    const QList<NoteEmbedding> stored = store->embeddings(embedder.model());
    QCOMPARE(stored.size(), 3);
    QCOMPARE(stored.at(0).noteId, fresh);
    QCOMPARE(stored.at(0).vector, QList<float>({0.5F, -0.25F, 0.75F}));
    // Untouched, and still the two components it was written with.
    QCOMPARE(stored.at(1).noteId, done);
    QCOMPARE(stored.at(1).vector, QList<float>({0.125F, 0.25F}));
    // Replaced rather than added beside the old one, and the flag is cleared —
    // left standing, the note would be embedded again in every run to come.
    QCOMPARE(stored.at(2).noteId, edited);
    QCOMPARE(stored.at(2).vector, QList<float>({-1.0F, 0.25F, 0.5F}));
    QVERIFY(!store->note(edited)->needsReembed);

    // And a second run has nothing left to do.
    provider.texts.clear();
    const QSignalSpy again(&embedder, &Embedder::finished);
    embedder.start();
    QCOMPARE(again.count(), 1);
    QVERIFY(provider.texts.isEmpty());
}

void AiTest::aVectorIsStoredUnderTheModelItWasAskedOf()
{
    // The settings dialog can write while an `embed` call is on its way, and
    // the answer arrives after the change. Read at that moment, the model would
    // be the NEW one while the vector was made by the OLD — which is exactly
    // the mixing this class re-reads the setting to avoid (issue #119), and
    // permanent: notesToEmbed(new) sees a note that already has a vector under
    // that name and never asks again, so nothing ever puts the row right.
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("AI"));
    const auto forgetTheGroup = qScopeGuard([&group] {
        group.deleteGroup();
    });
    group.writeEntry("EmbeddingModel", QStringLiteral("model-asked"));

    AiProviderMock provider;
    // Long enough for the change to fall between the request and the answer,
    // short enough not to slow the set down.
    provider.embedDelay = std::chrono::milliseconds(300);

    Embedder embedder(store.get(), &provider);
    QCOMPARE(embedder.model(), QStringLiteral("model-asked"));

    const QDateTime when = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    const qint64 note = addAnalysedNote(*store, QStringLiteral("Regentonne an die Fallrohre hängen."), when);
    QVERIFY(note > 0);

    QSignalSpy done(&embedder, &Embedder::finished);
    embedder.start();

    // Mid-call, the way the dialog reaches the running daemon.
    QTest::qWait(50);
    group.writeEntry("EmbeddingModel", QStringLiteral("model-set-meanwhile"));
    embedder.reloadSettings();
    QCOMPARE(embedder.model(), QStringLiteral("model-set-meanwhile"));

    QVERIFY(done.wait(std::chrono::seconds(5)));

    // The vector belongs to the model that made it. Both sides are asked, so
    // that a run storing under neither name cannot pass the first line alone.
    QCOMPARE(store->embeddings(QStringLiteral("model-asked")).size(), 1);
    QCOMPARE(store->embeddings(QStringLiteral("model-asked")).constFirst().noteId, note);
    QVERIFY(store->embeddings(QStringLiteral("model-set-meanwhile")).isEmpty());

    // The reload is not undone by this: from the next request on the new model
    // is what is asked and what is stored. And the note above is asked **again**
    // — it has no vector for the new name, so notesToEmbed() hands it over and
    // the corpus becomes whole under one model. That repair is what the wrong
    // name would have cost: written under `model-set-meanwhile`, the row would
    // have looked current and this run would have skipped it, leaving a vector
    // of one model in the corpus of another for good.
    const qint64 later = addAnalysedNote(*store, QStringLiteral("Hochbeet im Frühjahr neu schichten."), when.addSecs(60));
    QVERIFY(later > 0);
    embedder.start();
    QVERIFY(done.wait(std::chrono::seconds(5)));
    QCOMPARE(store->embeddings(QStringLiteral("model-set-meanwhile")).size(), 2);
    // A note carries exactly one vector (`note_id` is the primary key of
    // `embeddings`), so the repair replaces the old row rather than standing
    // beside it — which is why the name on it decides whether the note is ever
    // asked again at all.
    QVERIFY(store->embeddings(QStringLiteral("model-asked")).isEmpty());
}

void AiTest::aRefusedNoteDoesNotBlockTheOthers()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // The poison sits on the **oldest** note, which is the one the queue takes
    // first (Store::notesToEmbed). On the youngest, a run that gives up on the
    // first failure still writes two vectors and would look almost healthy.
    const QDateTime first = QDateTime::fromString(QStringLiteral("2026-08-03T08:00:00.000"), Qt::ISODateWithMs);
    const qint64 poison = addAnalysedNote(*store, QStringLiteral("Die Notiz, an der das Modell scheitert."), first);
    const qint64 second = addAnalysedNote(*store, QStringLiteral("Das Vogelhaus vor dem Winter abnehmen."), first.addSecs(60));
    const qint64 third = addAnalysedNote(*store, QStringLiteral("Die Hecke wächst über den Gehweg."), first.addSecs(120));
    QVERIFY(poison > 0 && second > 0 && third > 0);

    AiProviderMock provider;
    provider.refusedText = QStringLiteral("Die Notiz, an der das Modell scheitert.");
    provider.embedVectors = {{1.0, 0.0}, {0.0, 1.0}};

    Embedder embedder(store.get(), &provider);
    QSignalSpy failed(&embedder, &Embedder::failed);
    QSignalSpy paused(&embedder, &Embedder::paused);
    QSignalSpy finished(&embedder, &Embedder::finished);

    // First run: the refusal costs that note an attempt, and the two healthy
    // notes are embedded all the same. A run that ends on the first failure
    // asks once and writes nothing.
    embedder.start();
    QVERIFY(finished.wait(std::chrono::seconds(5)));
    QCOMPARE(provider.texts.size(), 3);
    QCOMPARE(store->embeddings(embedder.model()).size(), 2);
    QCOMPARE(failed.count(), 1);
    QCOMPARE(failed.constFirst().at(0).toLongLong(), poison);
    QCOMPARE(paused.count(), 0);
    QCOMPARE(store->note(poison)->analysisAttempts, 1);
    QCOMPARE(store->note(second)->analysisAttempts, 0);

    // Second run: only the refused note is outstanding, and its second refusal
    // is the last (SPEC 7.2). What SPEC 14 asks to be reported leaves as
    // paused() and carries the reason.
    provider.texts.clear();
    failed.clear();
    embedder.start();
    QVERIFY(finished.wait(std::chrono::seconds(5)));
    QCOMPARE(provider.texts, QStringList({provider.refusedText}));
    QCOMPARE(paused.count(), 1);
    QCOMPARE(paused.constFirst().at(0).toLongLong(), poison);
    QCOMPARE(paused.constFirst().at(1).toString(), provider.refusalReason);
    QCOMPARE(failed.count(), 0);

    // Third run: the note is not handed over again — that is the endless
    // retrying SPEC 7.2 rules out — and it is reported once more, because a
    // restart is the only place the tray could learn of it.
    provider.texts.clear();
    paused.clear();
    finished.clear();
    embedder.start();
    // Counted and not waited for: with nothing left to ask, the run ends inside
    // start() and the signal is gone before a wait() could arm itself — the
    // ten-second timeout #15's review ran into.
    QCOMPARE(finished.count(), 1);
    QVERIFY(provider.texts.isEmpty());
    QCOMPARE(paused.count(), 1);
    QCOMPARE(store->note(poison)->analysisAttempts, 2);
    QCOMPARE(store->note(poison)->analysisLastError, provider.refusalReason);
}

void AiTest::anUnreachableBackendCostsNoAttempt()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    const QDateTime first = QDateTime::fromString(QStringLiteral("2026-08-02T07:30:00.000"), Qt::ISODateWithMs);
    const qint64 one = addAnalysedNote(*store, QStringLiteral("Fahrradkette entfetten und neu ölen."), first);
    const qint64 two = addAnalysedNote(*store, QStringLiteral("Schaltzug vorne ist ausgefranst."), first.addSecs(60));
    QVERIFY(one > 0 && two > 0);

    AiProviderMock provider;
    // What an Ollama that is not running answers with, and it answers the same
    // for every note — so the run says it once instead of fifty times.
    provider.embedError = QStringLiteral("Ollama could not be reached: Connection refused");
    provider.embedFailure = AiFailure::Unreachable;

    Embedder embedder(store.get(), &provider);
    const QSignalSpy failed(&embedder, &Embedder::failed);
    const QSignalSpy paused(&embedder, &Embedder::paused);
    QSignalSpy finished(&embedder, &Embedder::finished);
    embedder.start();
    QVERIFY(finished.wait(std::chrono::seconds(5)));

    QCOMPARE(provider.texts.size(), 1);
    QCOMPARE(failed.count(), 1);
    QCOMPARE(failed.constFirst().at(0).toLongLong(), one);
    QCOMPARE(failed.constFirst().at(1).toString(), provider.embedError);
    QCOMPARE(paused.count(), 0);
    QVERIFY(!embedder.isBusy());

    // **No counter moved**, and that is the half the run stands or falls by: an
    // outage counted against the notes would leave the whole corpus given up on
    // after the second one, and nobody would ever see why.
    QCOMPARE(store->note(one)->analysisAttempts, 0);
    QCOMPARE(store->note(two)->analysisAttempts, 0);
    QCOMPARE(store->note(one)->analysisLastError, QString());

    // Nothing is written and nothing is given up on: both notes stand in the
    // next run, which is what SPEC 7.1 means by the bundles falling away while
    // Ollama is unreachable.
    QVERIFY(store->embeddings(embedder.model()).isEmpty());
    QCOMPARE(store->notesToEmbed(embedder.model()).size(), 2);
}

void AiTest::aClusterBecomesAnOpenBundleSuggestion()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // Three invented notes of one topic, written on two days, and **the angles
    // are chosen so that the chain does not run in the order of the clock**:
    // 0° and 100° stand at a similarity of -0.174 and reach each other only
    // through the 50° in between, so clusterNotes() answers mirror, offsite,
    // rsync while the days read 08-01, 08-02, 08-01.
    //
    // That is what the case is for. Written 0°/20°/40° the chain runs in the
    // order of the clock by itself, and the two orders agree — the collective
    // note then comes out right whether anything puts it in order or not, and
    // the case is green over both (CLAUDE.md, finding 34). Measured: with the
    // sort in clusterNotesFromStore() taken out, the stored Markdown carries
    // `## 2026-08-01`, `## 2026-08-02` and `## 2026-08-01`, the same day twice.
    //
    // The second day is what makes the sections visible at all; a single day
    // would hide a builder that writes one heading for everything.
    //
    // What this case does **not** rest on is the list of note ids: proposals()
    // sorts those by the note's own timestamp, so they come back in the right
    // order either way. The Markdown is the only place the fault shows.
    const QDateTime firstDay = QDateTime::fromString(QStringLiteral("2026-08-01T09:00:00.000"), Qt::ISODateWithMs);
    const QDateTime secondDay = QDateTime::fromString(QStringLiteral("2026-08-02T18:30:00.000"), Qt::ISODateWithMs);
    const qint64 mirror =
        addEmbeddedNote(*store, QStringLiteral("Die Fotos vom Sommer auf die zweite Platte spiegeln."), firstDay, 0.0);
    const qint64 rsync = addEmbeddedNote(*store,
                                         QStringLiteral("rsync mit --delete probieren, damit die Kopie nicht wächst."),
                                         firstDay.addSecs(3600),
                                         100.0);
    const qint64 offsite = addEmbeddedNote(*store,
                                           QStringLiteral("Eine Platte auswärts lagern, sonst nützt das Backup nichts."),
                                           secondDay,
                                           50.0);
    QVERIFY(mirror > 0 && rsync > 0 && offsite > 0);

    AiProviderMock provider;
    // A reasoning block with a draft object of its own inside it, and prose
    // around the real answer. **Ollama delivers none of that** — it hands the
    // reasoning over in a field of its own — so this case exists for what
    // SPEC 7.1 puts beside it, openrouter and OpenAI, and it is the only place
    // the shape can be produced at all (CLAUDE.md, finding 45).
    provider.chatAnswer = QStringLiteral(
        "<think>Alle drei drehen sich ums Sichern. {\"title\": \"Entwurf\", \"notes\": [1]}</think>\n"
        "Here is the JSON you asked for:\n"
        "{\"title\": \"Backup der Fotos\", \"notes\": [1, 2, 3]}");

    Suggester suggester(store.get(), &provider, testEmbeddingModel());
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy done(&suggester, &Suggester::finished);
    suggester.start();
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 1, 5000);

    const QList<Proposal> proposals = store->proposals();
    QCOMPARE(proposals.size(), qsizetype(1));
    QCOMPARE(proposals.constFirst().kind, Proposal::Kind::Bundle);
    QCOMPARE(proposals.constFirst().status, Proposal::Status::Open);
    QCOMPARE(proposals.constFirst().noteIds, QList<qint64>({mirror, rsync, offsite}));

    const QJsonObject payload = payloadOf(proposals.constFirst());
    QCOMPARE(payload.value(QLatin1String("title")).toString(), QStringLiteral("Backup der Fotos"));

    // The whole Markdown and not a substring of it: the form of SPEC 8.1 is
    // the topic as the heading, a section per day in order, and **the note
    // texts as they were typed**. That last part is what the model is not
    // asked for (SPEC 7.3 since 29.08.2026) — a check on the title alone would
    // be green over a note that came back paraphrased.
    QCOMPARE(payload.value(QLatin1String("markdown")).toString(),
             QStringLiteral("# Backup der Fotos\n"
                            "\n## 2026-08-01\n"
                            "\nDie Fotos vom Sommer auf die zweite Platte spiegeln.\n"
                            "\nrsync mit --delete probieren, damit die Kopie nicht wächst.\n"
                            "\n## 2026-08-02\n"
                            "\nEine Platte auswärts lagern, sonst nützt das Backup nichts.\n"));
}

void AiTest::theModelMayDropAnOutlier()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // Four notes chained into one cluster, and the model keeps the first, the
    // third and the fourth. **The dropped one is the second on purpose**: a
    // run that ignored the answer's list would keep four, and one that simply
    // took the first three would keep the wrong three — with the last one
    // dropped both mistakes would come out right (CLAUDE.md, finding 34).
    const QDateTime day = QDateTime::fromString(QStringLiteral("2026-08-03T10:00:00.000"), Qt::ISODateWithMs);
    const qint64 seeds = addEmbeddedNote(*store, QStringLiteral("Tomatensamen vorziehen, das Fenster nach Süden."), day, 0.0);
    const qint64 stranger =
        addEmbeddedNote(*store, QStringLiteral("Die Rechnung der Werkstatt liegt seit Freitag da."), day.addSecs(60), 20.0);
    const qint64 water = addEmbeddedNote(*store, QStringLiteral("Eine Regentonne an das Fallrohr hängen."), day.addSecs(120), 40.0);
    const qint64 soil = addEmbeddedNote(*store, QStringLiteral("Erde für die Hochbeete bestellen, vier Säcke."), day.addSecs(180), 60.0);
    QVERIFY(seeds > 0 && stranger > 0 && water > 0 && soil > 0);

    AiProviderMock provider;
    provider.chatAnswer = QStringLiteral(R"({"title": "Garten im Frühjahr", "notes": [1, 3, 4]})");

    Suggester suggester(store.get(), &provider, testEmbeddingModel());
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy done(&suggester, &Suggester::finished);
    suggester.start();
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 1, 5000);

    const QList<Proposal> proposals = store->proposals();
    QCOMPARE(proposals.size(), qsizetype(1));
    QCOMPARE(proposals.constFirst().noteIds, QList<qint64>({seeds, water, soil}));
    // And the text of the note that was dropped is in no part of the collective
    // note either: the references and the Markdown are two writes, and only one
    // of them was asked about above.
    QVERIFY(!payloadOf(proposals.constFirst()).value(QLatin1String("markdown")).toString().contains(
        QStringLiteral("Werkstatt")));
}

void AiTest::aNoteWithTaskFieldsBecomesATaskSuggestion()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // Only the fields the note carried, which is what the classification
    // already decided (SPEC 7.2): a description and a due date, no project, no
    // tags and no priority.
    const QString task = QStringLiteral(R"({"description":"Wasserfilter tauschen","due":"2026-08-05"})");
    const QDateTime day = QDateTime::fromString(QStringLiteral("2026-08-04T07:15:00.000"), Qt::ISODateWithMs);
    const qint64 filter = addEmbeddedNote(*store,
                                          QStringLiteral("Wasserfilter der Kaffeemaschine tauschen, die Anzeige blinkt."),
                                          day,
                                          0.0,
                                          task);
    QVERIFY(filter > 0);

    AiProviderMock provider;
    Suggester suggester(store.get(), &provider, testEmbeddingModel());
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy done(&suggester, &Suggester::finished);
    suggester.start();
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 1, 5000);

    const QList<Proposal> proposals = store->proposals();
    QCOMPARE(proposals.size(), qsizetype(1));
    QCOMPARE(proposals.constFirst().kind, Proposal::Kind::Task);
    QCOMPARE(proposals.constFirst().status, Proposal::Status::Open);
    QCOMPARE(proposals.constFirst().noteIds, QList<qint64>({filter}));
    // The text of `notes.task` handed on, character for character: assembled a
    // second time it would be a second place for the same fact, and a field
    // could go missing on the way without anything saying so.
    QCOMPARE(proposals.constFirst().payload, task);

    // No model was asked, because there is nothing to ask: the fields were
    // extracted by step 1 (SPEC 7.4).
    QVERIFY(provider.prompts.isEmpty());

    // And the next run does not offer the same task a second time. Without
    // that the review would fill up with one card per half hour (SPEC 7.2).
    suggester.start();
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 2, 5000);
    QCOMPARE(store->proposals().size(), qsizetype(1));
}

void AiTest::aDeferredBundleIsClusteredAgainAndReplaced()
{
    const QTemporaryDir directory;
    const std::unique_ptr<Store> store = openStore(directory);
    QVERIFY(store);

    // Two clusters far apart — 110° between the nearest members of the two, so
    // no note of one chains into the other. One of them was put aside for
    // later, the other one is a question still standing.
    const QDateTime day = QDateTime::fromString(QStringLiteral("2026-08-05T09:00:00.000"), Qt::ISODateWithMs);
    const qint64 chain = addEmbeddedNote(*store, QStringLiteral("Die Kette des Rades ölen, sie quietscht."), day, 0.0);
    const qint64 tyre = addEmbeddedNote(*store, QStringLiteral("Vorderreifen aufpumpen, drei Bar."), day.addSecs(60), 20.0);
    const qint64 light = addEmbeddedNote(*store, QStringLiteral("Das Rücklicht am Rad hält nicht mehr."), day.addSecs(120), 40.0);
    const qint64 pasta = addEmbeddedNote(*store, QStringLiteral("Nudeln und Passata sind alle."), day.addSecs(180), 150.0);
    const qint64 bread = addEmbeddedNote(*store, QStringLiteral("Brot beim Bäcker am Markt holen."), day.addSecs(240), 170.0);
    const qint64 coffee = addEmbeddedNote(*store, QStringLiteral("Kaffeebohnen für die nächste Woche."), day.addSecs(300), 190.0);
    QVERIFY(chain > 0 && tyre > 0 && light > 0 && pasta > 0 && bread > 0 && coffee > 0);

    const qint64 deferred = addStandingProposal(*store, Proposal::Kind::Bundle, Proposal::Status::Deferred,
                                                {chain, tyre, light});
    const qint64 open = addStandingProposal(*store, Proposal::Kind::Bundle, Proposal::Status::Open,
                                            {pasta, bread, coffee});
    QVERIFY(deferred > 0 && open > 0);

    AiProviderMock provider;
    provider.chatAnswer = QStringLiteral(R"({"title": "Das Rad", "notes": [1, 2, 3]})");

    Suggester suggester(store.get(), &provider, testEmbeddingModel());
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy done(&suggester, &Suggester::finished);
    suggester.start();
    QTRY_COMPARE_WITH_TIMEOUT(done.count(), 1, 5000);

    // **One call, and that is both halves of the criterion**: the deferred
    // notes went back into the corpus, and the notes of the open suggestion
    // did not — asked about again they would have made a second card over the
    // same three notes, every run. Held out both ways the number would be 0,
    // held out neither way it would be 2.
    QCOMPARE(provider.prompts.size(), qsizetype(1));
    QVERIFY(provider.prompts.constFirst().contains(QStringLiteral("Die Kette des Rades")));

    const QList<Proposal> proposals = store->proposals();
    QCOMPARE(proposals.size(), qsizetype(2));

    // The one that was standing keeps standing, unasked and untouched.
    QCOMPARE(proposals.constFirst().id, open);
    QCOMPARE(proposals.constFirst().noteIds, QList<qint64>({pasta, bread, coffee}));

    // The deferred one is gone and the new one has its place: two cards over
    // the same notes would be the same decision put twice (UX decision of
    // 29.08.2026).
    QCOMPARE(proposals.constLast().status, Proposal::Status::Open);
    QVERIFY(proposals.constLast().id != deferred);
    QCOMPARE(proposals.constLast().noteIds, QList<qint64>({chain, tyre, light}));
    QCOMPARE(payloadOf(proposals.constLast()).value(QLatin1String("title")).toString(), QStringLiteral("Das Rad"));
}

QTEST_GUILESS_MAIN(AiTest)

#include "aitest.moc"
