#include "aiprovidermock.h"

#include "analysis/classifier.h"
#include "analysis/ollamaprovider.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>

#include <chrono>
#include <memory>

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

    void nothingIsAskedTwiceWhenNobodyAnswers();
    void aServerThatAnsweredIsNotAskedAgain();

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

    void everyNoteGetsItsOwnAnswer();
    void noteThatFailedTwiceIsSkippedAndReported();
    void theErrorCountSurvivesARestart();
    void successClearsTheErrorCount();
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

/** A well-formed answer, so that a check varies only what it is about. */
QString answerFor(const QString &category, const QString &tags)
{
    return QStringLiteral(R"({"category": "%1", "tags": [%2], "is_todo": false})").arg(category, tags);
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
        "\n```"));

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
        " — although"));

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
        R"({"category": "software", "tags": ["qt", "wayland"], "is_todo": false})"));

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.category, QStringLiteral("software"));
}

void AiTest::categoryOutsideTheListIsRefused()
{
    // A sixth category would be a note that no `kat:` search and no sidebar
    // entry ever reaches again (SPEC 6) — so it is not written, and the value
    // stands in the reason.
    const Classification classification = readClassification(answerFor(QStringLiteral("haushalt"), QStringLiteral(R"("kaffee")")));

    QVERIFY(classification.error.contains(QStringLiteral("haushalt")));
    QCOMPARE(classification.category, QString());
    QVERIFY(classification.tags.isEmpty());
}

void AiTest::categoryCaseIsFolded()
{
    // Upper case is how the answer is written, not what it says: `kat:` is a
    // literal comparison against the short form (SPEC 6), so it is folded here
    // rather than refused.
    const Classification classification = readClassification(answerFor(QStringLiteral("Persoenlich"), QStringLiteral(R"("geburtstag")")));

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
                                     QStringLiteral(R"("Zeitleiste", "spuren", "ZEITLEISTE", "  notizen  ", "", "woche", "farbe")")));

    QCOMPARE(classification.error, QString());
    QCOMPARE(classification.tags,
             QStringList({QStringLiteral("zeitleiste"),
                          QStringLiteral("spuren"),
                          QStringLiteral("notizen"),
                          QStringLiteral("woche")}));
}

void AiTest::answerWithoutATagIsRefused()
{
    const Classification classification = readClassification(answerFor(QStringLiteral("ideen"), QString()));

    QVERIFY2(!classification.error.isEmpty(), qPrintable(classification.category));
    QCOMPARE(classification.category, QString());
}

void AiTest::categoryThatIsNoTextIsNamedAnyway()
{
    // The reason goes into `analysis_last_error` and from there into the tray
    // tooltip and the log (SPEC 14). Read out as a string, a number falls out
    // of it and the sentence names an empty pair of quotation marks.
    const Classification classification =
        readClassification(QStringLiteral(R"({"category": 3, "tags": ["backup"], "is_todo": false})"));

    QVERIFY2(classification.error.contains(QStringLiteral("3")), qPrintable(classification.error));
}

void AiTest::markerInsideATagDoesNotCut()
{
    // The marker stands inside a JSON string, so it is text of the answer and
    // no end of any reasoning. Cutting there costs the whole answer and an
    // attempt with it — after two of those the note keeps no category at all.
    const Classification classification = readClassification(
        QStringLiteral(R"({"category": "ideen", "tags": ["das </think> steht im text"], "is_todo": false})"));

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
        QStringLiteral(R"({"category": "todos", "tags": ["backup"], "is_todo": true, "task": null})"));

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
                       R"( "task": {"description": "Platte anstecken", "project": "vault"}})"));

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
        R"( "due": "irgendwann", "priority": "dringend", "erfunden": "weg"}})"));

    QCOMPARE(classification.error, QString());

    const QJsonObject task = QJsonDocument::fromJson(classification.task.toUtf8()).object();
    QCOMPARE(task.value(QStringLiteral("description")).toString(), QStringLiteral("Wasserfilter tauschen"));
    QCOMPARE(task.value(QStringLiteral("project")).toString(), QStringLiteral("kueche"));
    QCOMPARE(task.value(QStringLiteral("tags")).toArray().first().toString(), QStringLiteral("filter"));
    QVERIFY(!task.contains(QStringLiteral("due")));
    QVERIFY(!task.contains(QStringLiteral("priority")));
    QVERIFY(!task.contains(QStringLiteral("erfunden")));
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

QTEST_GUILESS_MAIN(AiTest)

#include "aitest.moc"
