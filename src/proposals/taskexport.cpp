#include "proposals/taskexport.h"

#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

#include <algorithm>

namespace
{

/**
 * How long one Taskwarrior call may take before it is killed.
 *
 * `task add` answers in milliseconds; what this bound exists for is a user
 * hook, which Taskwarrior runs on every add and which is a program of the
 * user's own. A hook that waits for something would otherwise hold the calling
 * thread for good — and the review of SPEC 9 calls this from the interface.
 */
constexpr int callTimeoutMs = 10000;

/**
 * Taskwarrior is to write the UUID of the new task and nothing else.
 *
 * Its default verbosity prints `Created task 3.` — the **id**, which SPEC 8.2
 * cannot use: the annotation goes to the UUID. `new-uuid` is what turns the
 * same line into `Created task <uuid>.`, measured on 3.5.0 on 30.08.2026, and
 * the control came out different in the same run: with a made-up verbosity
 * token the same command printed the id again, with return value 0 and no
 * complaint. So this string is load-bearing and a typo in it is silent — which
 * is why uuidFrom() below reports an output it cannot read rather than going
 * on without an annotation.
 */
const QLatin1StringView verboseNewUuid("rc.verbose=new-uuid");

/**
 * Belt and braces against a question nobody could answer — and measured to
 * change nothing on either of the two calls this file makes.
 *
 * The reasoning is sound and the effect is not: a prompt would go to a process
 * whose standard input nobody writes to, but neither `add` nor an `annotate`
 * naming exactly one UUID ever asks. Measured on 3.5.0 on 30.08.2026 in
 * production form: the same single-task annotate with and without the setting
 * came back rc=0 with empty standard error, twice alike. And the confirmation
 * that could occur at all — the bulk change — is governed by `rc.bulk`
 * (default 3), not by `rc.confirmation` (default 1), so this is not the lever
 * for it either.
 *
 * It stays because it costs one argument and closes the case where a later
 * caller passes more than one task; what it does not do is carry the guarantee
 * on its own. On the command line rather than in the user's `taskrc`, which
 * this program has no business writing to.
 */
const QLatin1StringView noConfirmation("rc.confirmation=no");

/** Everything after this is text, never a field — see taskAddArguments(). */
const QLatin1StringView endOfOptions("--");

/** Whether Taskwarrior can carry `tag` as a tag at all. */
bool isUsableTag(const QString &tag)
{
    return !tag.isEmpty()
        && std::none_of(tag.cbegin(), tag.cend(), [](QChar character) {
               return character.isSpace();
           });
}

/**
 * The UUID out of what `task add` wrote, empty when there is none to be had.
 *
 * The shape is matched, not the sentence around it. `Created task %1.` is a
 * message Taskwarrior renders for the user, and a check for those two words
 * would be a check on the wording of a foreign program — the UUID is the
 * statement (CLAUDE.md, finding 59). With `verboseNewUuid` set, nothing else
 * stands in that output.
 */
QString uuidFrom(const QString &output)
{
    static const QRegularExpression pattern(
        QStringLiteral("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}"));
    return pattern.match(output).captured();
}

/**
 * Runs `program` with `arguments` and hands its standard output back in
 * `output`.
 *
 * The return value is what belongs on the card, and empty means the call went
 * through. The two error cases SPEC 8.2 names are told apart here and nowhere
 * else: a program that cannot be started is a machine without Taskwarrior,
 * a return value other than 0 is Taskwarrior refusing — and the second one
 * knows why, so its own message is passed on rather than replaced. Errors go
 * to standard error, the created UUID to standard output (measured on 3.5.0,
 * 30.08.2026), so the two never mix.
 */
QString runTask(const QString &program, const QStringList &arguments, QString *output)
{
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(callTimeoutMs)) {
        return i18n("%1 is not available.", program);
    }
    if (!process.waitForFinished(callTimeoutMs)) {
        process.kill();
        process.waitForFinished();
        return i18n("%1 did not answer within %2 seconds.", program, callTimeoutMs / 1000);
    }

    *output = QString::fromUtf8(process.readAllStandardOutput());
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        return {};
    }

    // NOLINTNEXTLINE(misc-const-correctness) - returned below, see rule 1 in .clang-tidy
    QString message = QString::fromUtf8(process.readAllStandardError()).trimmed();
    if (!message.isEmpty()) {
        return message;
    }
    // A refusal without a word. The return value is then all there is to say,
    // and saying nothing would leave the card blank over a suggestion that was
    // not carried out.
    return i18n("%1 ended with the return value %2.", program, process.exitCode());
}

}

QStringList taskAddArguments(const QString &payload)
{
    const QJsonObject fields = QJsonDocument::fromJson(payload.toUtf8()).object();

    const QString description = fields.value(QLatin1String("description")).toString().trimmed();
    if (description.isEmpty()) {
        return {};
    }

    QStringList arguments{verboseNewUuid, noConfirmation, QStringLiteral("add")};

    const QString project = fields.value(QLatin1String("project")).toString().trimmed();
    if (!project.isEmpty()) {
        arguments.append(QStringLiteral("project:") + project);
    }

    const QJsonArray tags = fields.value(QLatin1String("tags")).toArray();
    for (const auto &value : tags) {
        const QString tag = value.toString().trimmed();
        if (isUsableTag(tag)) {
            arguments.append(QLatin1Char('+') + tag);
        }
    }

    const QString due = fields.value(QLatin1String("due")).toString().trimmed();
    if (!due.isEmpty()) {
        arguments.append(QStringLiteral("due:") + due);
    }

    const QString priority = fields.value(QLatin1String("priority")).toString().trimmed();
    if (!priority.isEmpty()) {
        arguments.append(QStringLiteral("priority:") + priority);
    }

    arguments.append(endOfOptions);
    arguments.append(description);
    return arguments;
}

QStringList taskAnnotateArguments(const QString &uuid, const QString &text)
{
    return {verboseNewUuid, noConfirmation, uuid, QStringLiteral("annotate"), endOfOptions, text};
}

// payload and noteText are the two halves SPEC 8.2 hands this function, and the third is the
// program of SPEC 2.5 with a default; a wrapper type per argument would be an abstraction for
// one call site.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
TaskExportResult exportTaskProposal(const QString &payload, const QString &noteText, const QString &program)
{
    TaskExportResult result;

    const QStringList arguments = taskAddArguments(payload);
    if (arguments.isEmpty()) {
        result.error = i18n("The suggestion carries no description; nothing was created.");
        return result;
    }

    QString output;
    const QString addFailed = runTask(program, arguments, &output);
    if (!addFailed.isEmpty()) {
        result.error = addFailed;
        return result;
    }

    result.uuid = uuidFrom(output);
    if (result.uuid.isEmpty()) {
        // The add went through, so a task stands in Taskwarrior — it just
        // cannot be reached to hang the note text on. Reported rather than
        // passed over: an annotation that disappears without a word is the
        // half of SPEC 8.2 the user would never notice was missing.
        result.error = i18n("%1 created the task but did not name it; the note text was not attached.", program);
        return result;
    }

    // The description is the last argument by construction — taskAddArguments()
    // puts it behind the separator — so it is read off there rather than parsed
    // a second time out of the payload, where the two could drift apart.
    const QString &description = arguments.constLast();

    // SPEC 8.2 attaches the note "with a longer note text". The comparison is
    // against the description and not against a character count: a note the
    // classification took over word for word would otherwise carry its one
    // sentence twice, and any number here would be a threshold nobody set.
    const QString text = noteText.trimmed();
    if (text.isEmpty() || text == description) {
        return result;
    }

    const QString annotateFailed = runTask(program, taskAnnotateArguments(result.uuid, text), &output);
    if (!annotateFailed.isEmpty()) {
        result.error = annotateFailed;
        return result;
    }

    result.annotated = true;
    return result;
}
