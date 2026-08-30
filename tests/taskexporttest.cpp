#include "proposals/taskexport.h"

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#include <memory>
#include <utility>

/**
 * The Taskwarrior execution of SPEC 8.2 (issue #33).
 *
 * Everything here breaks without a sound. A field that never reaches the
 * command line, a note text that Taskwarrior reads as fields instead of as
 * text, an error the caller takes for a success — all three end with return
 * value 0 and a task that looks right, and the user finds out weeks later that
 * a due date or an annotation was never there. There is nothing to look at:
 * what the process was handed is not on any screen.
 *
 * **No `task` is started here, and none may be.** SPEC 15 makes Taskwarrior an
 * optional program, and a check that needs it would turn the CI red on a
 * machine that does exactly what the specification allows. What stands in its
 * place is a stand-in, the way SPEC 12 puts one in whisper-cli's place — and
 * every case reads back the argument array it really received, out of a file
 * the stand-in writes. That the argument array works against the **real**
 * Taskwarrior is a measurement of its own, against a TASKDATA directory of its
 * own (SPEC 16, manual column); what it produced stands in the header comments
 * of taskexport.h with its date.
 */
class TaskExportTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void theAddCommandCarriesEveryPopulatedField();
    void anUnsetFieldGetsNoArgument();
    void aHostileTextCannotBecomeFields();
    void aTagTaskwarriorCannotCarryIsLeftOut();

    void aMissingProgramLeavesTheSuggestionOpen();
    void aRefusedAddLeavesTheSuggestionOpen();
    void theNoteTextReachesTaskwarriorAsAnAnnotation();
    void aNoteThatSaysNoMoreThanTheDescriptionGetsNoAnnotation();
    void anAddThatNamesNoUuidIsReported();

private:
    /**
     * Writes an executable stand-in for `task` under `name` and returns its
     * path. `body` is the shell that runs after the arguments have been noted.
     *
     * Every run appends a file `<name>.args.<n>` holding its arguments
     * NUL-separated — NUL and not newline, because the texts this story carries
     * contain newlines and a line-based log could not tell one argument from
     * two.
     */
    QString writeStub(const QString &name, const QByteArray &body);

    /** The arguments of the `n`-th run of the stand-in `name`, in order. */
    QStringList argumentsOfRun(const QString &name, int n) const;

    /** How often the stand-in `name` was started. */
    int runCount(const QString &name) const;

    std::unique_ptr<QTemporaryDir> m_dir;
};

void TaskExportTest::initTestCase()
{
    // The messages that go on the card go through i18n(); without the domain
    // every one of them warns before it hands the msgid back — and the CI reads
    // that warning out of the run and fails on it (issue #53).
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());
}

QString TaskExportTest::writeStub(const QString &name, const QByteArray &body)
{
    // NOLINTNEXTLINE(misc-const-correctness) - returned below, see rule 1 in .clang-tidy
    QString path = m_dir->filePath(name);
    QFile stub(path);
    if (!stub.open(QIODevice::WriteOnly)) {
        return {};
    }
    stub.write("#!/bin/sh\n"
               "n=0\n"
               "while [ -e \"$0.args.$n\" ]; do n=$((n+1)); done\n"
               "printf '%s\\0' \"$@\" > \"$0.args.$n\"\n");
    stub.write(body);
    stub.close();
    if (!stub.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)) {
        return {};
    }
    return path;
}

QStringList TaskExportTest::argumentsOfRun(const QString &name, int n) const
{
    QFile log(m_dir->filePath(QStringLiteral("%1.args.%2").arg(name).arg(n)));
    if (!log.open(QIODevice::ReadOnly)) {
        return {};
    }
    // The trailing NUL of the last argument leaves an empty tail behind.
    QList<QByteArray> parts = log.readAll().split('\0');
    if (!parts.isEmpty()) {
        parts.removeLast();
    }
    QStringList arguments;
    for (const QByteArray &part : std::as_const(parts)) {
        arguments.append(QString::fromUtf8(part));
    }
    return arguments;
}

int TaskExportTest::runCount(const QString &name) const
{
    return static_cast<int>(QDir(m_dir->path()).entryList({name + QStringLiteral(".args.*")}, QDir::Files).size());
}

/** The payload of SPEC 7.2 with all five fields filled. */
void TaskExportTest::theAddCommandCarriesEveryPopulatedField()
{
    const QString payload = QStringLiteral(
        R"({"description":"Buy milk","project":"home","tags":["errands","shopping"],)"
        R"("due":"2026-09-05","priority":"H"})");

    // Held against a literal and not against a loop over the same JSON: the
    // point of this case is that every one of the five fields arrives, and a
    // comparison built out of the input could not notice one of them going
    // missing on both sides at once (CLAUDE.md, finding 10).
    const QStringList expected{QStringLiteral("rc.verbose=new-uuid"),
                               QStringLiteral("rc.confirmation=no"),
                               QStringLiteral("add"),
                               QStringLiteral("project:home"),
                               QStringLiteral("+errands"),
                               QStringLiteral("+shopping"),
                               QStringLiteral("due:2026-09-05"),
                               QStringLiteral("priority:H"),
                               QStringLiteral("--"),
                               QStringLiteral("Buy milk")};
    QCOMPARE(taskAddArguments(payload), expected);
}

void TaskExportTest::anUnsetFieldGetsNoArgument()
{
    // SPEC 8.2: "only populated fields". A `project:` with nothing behind it
    // would be a project named "" that the user never asked for.
    const QStringList bare = taskAddArguments(QStringLiteral(
        R"({"description":"Call back","project":"","tags":[],"due":"","priority":""})"));
    const QStringList expected{QStringLiteral("rc.verbose=new-uuid"),
                               QStringLiteral("rc.confirmation=no"),
                               QStringLiteral("add"),
                               QStringLiteral("--"),
                               QStringLiteral("Call back")};
    QCOMPARE(bare, expected);

    // Without a description there is no task to be had (SPEC 5.1), and
    // Taskwarrior refuses the add anyway — with return value 2 and a message
    // the card would then show over a suggestion nobody could have carried out.
    QVERIFY(taskAddArguments(QStringLiteral(R"({"description":"   "})")).isEmpty());
    QVERIFY(taskAddArguments(QStringLiteral("not json at all")).isEmpty());
}

/**
 * The security case, and it is about Taskwarrior's parser, not about a shell.
 *
 * The argument array keeps `sh` out; Taskwarrior reads `project:`, `+tag` and
 * `due:` out of its own arguments wherever they stand. Measured against 3.5.0
 * on 30.08.2026: without the separator this description was eaten field by
 * field and the add came back "A task must have a description." with return
 * value 2, and the same text as an annotation silently overwrote the task's
 * project instead of being attached.
 */
void TaskExportTest::aHostileTextCannotBecomeFields()
{
    const QString hostile = QStringLiteral("project:secret +evil due:tomorrow \"quoted\" ; $(id)\nsecond line");
    const QJsonObject payload{{QStringLiteral("description"), hostile}, {QStringLiteral("project"), QStringLiteral("home")}};
    const QStringList arguments =
        taskAddArguments(QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)));

    // The text arrives whole, and it arrives as ONE argument: split anywhere,
    // its second half would be a field.
    QCOMPARE(arguments.constLast(), hostile);
    QCOMPARE(arguments.at(arguments.size() - 2), QStringLiteral("--"));

    // And nothing the text carries stands in front of the separator, where it
    // would be read as a field. The project the payload really named does.
    const QStringList head = arguments.first(arguments.size() - 2);
    QVERIFY(head.contains(QStringLiteral("project:home")));
    QVERIFY(!head.contains(QStringLiteral("project:secret")));
    QVERIFY(!head.contains(QStringLiteral("+evil")));
    QVERIFY(!head.contains(QStringLiteral("due:tomorrow")));

    // The annotation takes the same road, and there it is worth more: the
    // measured run lost the note text entirely and rewrote a field of the
    // user's task, with return value 0.
    const QStringList annotate = taskAnnotateArguments(QStringLiteral("d601cec6-b2f3-4d48-bfe5-a2d571d279f7"), hostile);
    const QStringList expected{QStringLiteral("rc.verbose=new-uuid"),
                               QStringLiteral("rc.confirmation=no"),
                               QStringLiteral("d601cec6-b2f3-4d48-bfe5-a2d571d279f7"),
                               QStringLiteral("annotate"),
                               QStringLiteral("--"),
                               hostile};
    QCOMPARE(annotate, expected);
}

/**
 * A tag with whitespace is no tag for Taskwarrior — it falls into the
 * description, and the add goes through with return value 0. The tag is gone
 * and the description carries a word nobody wrote (measured 30.08.2026).
 */
void TaskExportTest::aTagTaskwarriorCannotCarryIsLeftOut()
{
    const QStringList arguments = taskAddArguments(
        QStringLiteral(R"({"description":"Tidy up","tags":["errands","two words","",  "shopping"]})"));
    QVERIFY(arguments.contains(QStringLiteral("+errands")));
    QVERIFY(arguments.contains(QStringLiteral("+shopping")));
    QVERIFY(!arguments.contains(QStringLiteral("+two words")));
    // And it is left out, not repaired: a hyphen in its place would be a tag
    // the user never wrote.
    QVERIFY(!arguments.contains(QStringLiteral("+two-words")));
}

/** SPEC 8.2, first error case: no Taskwarrior on this machine. */
void TaskExportTest::aMissingProgramLeavesTheSuggestionOpen()
{
    const QString absent = m_dir->filePath(QStringLiteral("no-task-here"));
    QVERIFY(!QFile::exists(absent));

    const TaskExportResult result =
        exportTaskProposal(QStringLiteral(R"({"description":"Buy milk"})"), QStringLiteral("Buy milk"), absent);

    QVERIFY(!result.ok());
    // The whole sentence, not `contains(absent)`. Every message this function
    // can produce names the program, so a containment check is green on all of
    // them — measured 30.08.2026 with the `waitForStarted` branch cut out: a
    // program that does not exist then fell through to the timeout and reported
    // "did not answer within 10 seconds", and the case stayed green over a
    // sentence that sends the user waiting instead of installing (CLAUDE.md,
    // finding 20 — a readback has to come out different at least once).
    QCOMPARE(result.error, QStringLiteral("%1 is not available.").arg(absent));
    // Nothing was created, so the caller has nothing to delete — which is what
    // leaves the note where it is (SPEC 8.2).
    QVERIFY(result.uuid.isEmpty());
    QVERIFY(!result.annotated);
}

/** SPEC 8.2, second error case: Taskwarrior ran and refused. */
void TaskExportTest::aRefusedAddLeavesTheSuggestionOpen()
{
    const QString name = QStringLiteral("refusing-task.sh");
    const QString stub = writeStub(name, "echo \"'notadate' is not a valid date in the 'Y-M-D' format.\" >&2\n"
                                         "exit 2\n");
    QVERIFY(!stub.isEmpty());

    const TaskExportResult result = exportTaskProposal(
        QStringLiteral(R"({"description":"Buy milk","due":"notadate"})"), QStringLiteral("Buy milk"), stub);

    QVERIFY(!result.ok());
    // Taskwarrior's own sentence, not one of ours: it knows which field it
    // refused and we do not.
    QCOMPARE(result.error, QStringLiteral("'notadate' is not a valid date in the 'Y-M-D' format."));
    QVERIFY(result.uuid.isEmpty());
    QVERIFY(!result.annotated);
    // And this is the other half of the same criterion: the annotation was
    // never attempted, so nothing half-finished stands in Taskwarrior.
    QCOMPARE(runCount(name), 1);
}

void TaskExportTest::theNoteTextReachesTaskwarriorAsAnAnnotation()
{
    const QString name = QStringLiteral("creating-task.sh");
    const QString stub = writeStub(name, "echo 'Created task 765f137a-541d-40d7-bfb0-09d5ed3b40c8.'\n"
                                         "exit 0\n");
    QVERIFY(!stub.isEmpty());

    const QString noteText = QStringLiteral("Buy milk\nand the oat one for Jörg — groß, süß, weiß.");
    const TaskExportResult result = exportTaskProposal(
        QStringLiteral(R"({"description":"Buy milk","project":"home"})"), noteText, stub);

    QVERIFY2(result.ok(), qPrintable(result.error));
    QCOMPARE(result.uuid, QStringLiteral("765f137a-541d-40d7-bfb0-09d5ed3b40c8"));
    QVERIFY(result.annotated);

    // Read back off the process rather than off the return value: that the
    // stand-in ran twice and what it was handed the second time is the only
    // statement that the annotation left this program at all.
    QCOMPARE(runCount(name), 2);
    QCOMPARE(argumentsOfRun(name, 0).constLast(), QStringLiteral("Buy milk"));

    const QStringList annotate = argumentsOfRun(name, 1);
    const QStringList expected{QStringLiteral("rc.verbose=new-uuid"),
                               QStringLiteral("rc.confirmation=no"),
                               QStringLiteral("765f137a-541d-40d7-bfb0-09d5ed3b40c8"),
                               QStringLiteral("annotate"),
                               QStringLiteral("--"),
                               noteText};
    // The newline and the umlauts come through the argument array untouched —
    // one argument, not two lines.
    QCOMPARE(annotate, expected);
}

/**
 * SPEC 8.2 annotates "with a longer note text". A note the classification took
 * over word for word carries nothing the description does not already say, and
 * a second copy of the same sentence under the task is noise.
 */
void TaskExportTest::aNoteThatSaysNoMoreThanTheDescriptionGetsNoAnnotation()
{
    const QString name = QStringLiteral("short-note-task.sh");
    const QString stub = writeStub(name, "echo 'Created task 074ad284-1943-4d32-a140-820c0833f8ab.'\n"
                                         "exit 0\n");
    QVERIFY(!stub.isEmpty());

    const TaskExportResult result =
        exportTaskProposal(QStringLiteral(R"({"description":"Buy milk"})"), QStringLiteral("  Buy milk\n"), stub);

    QVERIFY(result.ok());
    QVERIFY(!result.annotated);
    QCOMPARE(runCount(name), 1);
}

/**
 * The verbosity token `rc.verbose=new-uuid` is what turns `Created task 3.`
 * into the UUID SPEC 8.2 annotates against, and a wrong one is silent:
 * measured on 3.5.0, a made-up token printed the id again with return value 0.
 * The task then exists and cannot be reached — which is worth a sentence on the
 * card, because an annotation that disappears is the half of the story nobody
 * would notice was missing.
 */
void TaskExportTest::anAddThatNamesNoUuidIsReported()
{
    const QString name = QStringLiteral("id-only-task.sh");
    const QString stub = writeStub(name, "echo 'Created task 3.'\nexit 0\n");
    QVERIFY(!stub.isEmpty());

    const TaskExportResult result = exportTaskProposal(
        QStringLiteral(R"({"description":"Buy milk"})"), QStringLiteral("Buy milk, the oat one"), stub);

    // The whole sentence, for the reason aMissingProgramLeavesTheSuggestionOpen()
    // carries: `!ok()` is true for every message this function can produce, so
    // it cannot say which one the user reads. Measured in review on 30.08.2026
    // — with this message swapped for the time-limit one, which sends the user
    // waiting instead of telling them the task stands there without its note
    // text, the case stayed green.
    QCOMPARE(result.error,
             QStringLiteral("%1 created the task but did not name it; the note text was not attached.").arg(stub));
    QVERIFY(result.uuid.isEmpty());
    QVERIFY(!result.annotated);
    QCOMPARE(runCount(name), 1);
}

QTEST_GUILESS_MAIN(TaskExportTest)

#include "taskexporttest.moc"
