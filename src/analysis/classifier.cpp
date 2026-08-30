#include "analysis/classifier.h"

#include "analysis/aiprovider.h"
#include "analysis/modelanswer.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace
{
/** SPEC 7.2: 1 to 4 tags. */
constexpr qsizetype tagLimit = 4;

/**
 * What the model wrote in a field, whatever JSON type it used.
 *
 * A reason that names the value is worth nothing if the value falls out of it:
 * `{"category": 3}` said `the category ""` before, and that sentence stands in
 * `analysis_last_error`, in the tray tooltip and in the log (SPEC 14).
 */
QString rawText(const QJsonValue &value)
{
    if (value.isObject() || value.isArray()) {
        return QString::fromUtf8(QJsonDocument::fromVariant(value.toVariant()).toJson(QJsonDocument::Compact)).trimmed();
    }
    return value.toVariant().toString();
}

/** The tags of an answer, lower case, without repetitions, at most four. */
QStringList readTags(const QJsonArray &values)
{
    QStringList tags;
    for (const auto &value : values) {
        const QString tag = value.toString().trimmed().toLower();
        if (tag.isEmpty() || tags.contains(tag)) {
            continue;
        }
        tags.append(tag);
        if (tags.size() == tagLimit) {
            break;
        }
    }
    return tags;
}

/**
 * How far after the note's own day a due date may lie before it is taken for
 * invented (SPEC 7.2, issue #117).
 *
 * A year. The lower bound below — nothing before the day the note was written —
 * catches what was actually measured, a date out of the training data lying
 * years in the past; this is the other direction, where a model that guesses
 * forward would land. A year is the horizon a note jotted down in a hurry still
 * plausibly names, and the two mistakes do not cost the same: a wrong date that
 * is kept becomes a due date in a real task list (SPEC 8.2) that nobody can see
 * is invented, while a right date that is dropped costs one null field on a
 * suggestion the user confirms by hand anyway (SPEC 7.4, nothing is carried out
 * without confirmation). So the bound is drawn where the cheaper mistake is
 * made.
 */
constexpr int dueDaysAhead = 365;

/**
 * The task fields of SPEC 7.2, built from what the model wrote and from
 * nothing else.
 *
 * `description` is the one that has to be there; the four beside it are copied
 * only where they carry what SPEC 7.2 allows, and left out otherwise rather
 * than refusing the whole answer — both are allowed to be null, and a model
 * that over-reads the note gets them wrong without getting the note wrong.
 *
 * **`due` is held against the day the note was written**, and that is the
 * second half of issue #117: the prompt now says what day the note is from, but
 * a prompt is a request and not a guarantee (CLAUDE.md, finding 50). Measured
 * on 2026-08-29: qwen3:8b turned "Morgen" into `"due": "2023-10-26"`, a
 * well-formed date out of its training that passes every format check. A due
 * date before the note, or more than dueDaysAhead after it, is dropped and the
 * field stays null.
 */
QJsonObject readTask(const QJsonObject &task, const QString &description, QDate writtenOn)
{
    QJsonObject kept{{QLatin1String("description"), description}};

    const QString project = task.value(QLatin1String("project")).toString().trimmed().toLower();
    if (!project.isEmpty()) {
        kept.insert(QLatin1String("project"), project);
    }

    const QStringList tags = readTags(task.value(QLatin1String("tags")).toArray());
    if (!tags.isEmpty()) {
        kept.insert(QLatin1String("tags"), QJsonArray::fromStringList(tags));
    }

    const QString due = task.value(QLatin1String("due")).toString();
    const QDate day = QDate::fromString(due, Qt::ISODate);
    if (day.isValid() && day >= writtenOn && day <= writtenOn.addDays(dueDaysAhead)) {
        kept.insert(QLatin1String("due"), due);
    }

    const QString priority = task.value(QLatin1String("priority")).toString().toUpper();
    if (priority == QLatin1String("H") || priority == QLatin1String("M") || priority == QLatin1String("L")) {
        kept.insert(QLatin1String("priority"), priority);
    }

    return kept;
}
}

QStringList analysisCategories()
{
    return {
        QStringLiteral("todos"),
        QStringLiteral("ideen"),
        QStringLiteral("cli"),
        QStringLiteral("persoenlich"),
        QStringLiteral("software"),
    };
}

QString classificationPrompt(const QString &noteText, QDate writtenOn)
{
    return QStringLiteral(
               "You sort short personal notes, most of them written in German. "
               "Answer with one JSON object and nothing else.\n"
               "\n"
               "{\"category\": one of %1,\n"
               " \"tags\": 1 to 4 lower-case keywords taken from the note,\n"
               " \"is_todo\": true when the note asks for something to be done,\n"
               " \"task\": null, or when \"is_todo\" is true\n"
               "         {\"description\": what is to be done,\n"
               "          \"project\": one lower-case word or null,\n"
               "          \"tags\": lower-case keywords,\n"
               "          \"due\": \"YYYY-MM-DD\" or null,\n"
               "          \"priority\": \"H\", \"M\", \"L\" or null}}\n"
               "\n"
               "Write the tags and the task in the language of the note. "
               "Set \"due\" and \"priority\" only where the note says so, otherwise null.\n"
               "The note was written on %2, and every relative date in it is read "
               "from that day. Where the note names no day at all, \"due\" stays null; "
               "never fill it with a guessed date.\n"
               "\n"
               "Note:\n"
               "%3")
        .arg(QStringLiteral("\"%1\"").arg(analysisCategories().join(QLatin1String("\", \""))),
             writtenOn.toString(Qt::ISODate),
             noteText);
}

Classification readClassification(const QString &answer, QDate writtenOn)
{
    const QJsonObject object = modelAnswerObject(answer, QLatin1String("category"));
    if (object.isEmpty()) {
        return {{}, {}, {}, i18n("The model's answer carried no JSON object.")};
    }

    Classification result;

    const QJsonValue category = object.value(QLatin1String("category"));
    result.category = category.toString().trimmed().toLower();
    if (!analysisCategories().contains(result.category)) {
        return {{}, {}, {}, i18n("The model answered with the category \"%1\", which is not one of the five.", rawText(category))};
    }

    result.tags = readTags(object.value(QLatin1String("tags")).toArray());
    if (result.tags.isEmpty()) {
        return {{}, {}, {}, i18n("The model's answer carried no tag.")};
    }

    // **The gate is the description, not the `is_todo` beside it.** SPEC 7.4
    // makes a suggestion out of the extracted fields, so a description is what
    // there is a task to be had from — and `task IS NOT NULL` is the statement
    // that the note is a task (SPEC 5.1), which makes the flag a second place
    // for the same fact rather than the deciding one.
    //
    // Read the other way round it failed twice over: `"is_todo": true` with a
    // `task` of null threw a sound category and its tags away and spent an
    // attempt on it, and `"is_todo": "true"` — the string, which
    // QJsonValue::toBool() answers false for — dropped a filled task without a
    // word. Both measured 2026-08-29.
    const QJsonObject task = object.value(QLatin1String("task")).toObject();
    const QString description = task.value(QLatin1String("description")).toString().trimmed();
    if (!description.isEmpty()) {
        result.task = QString::fromUtf8(QJsonDocument(readTask(task, description, writtenOn)).toJson(QJsonDocument::Compact));
    }

    return result;
}

Classifier::Classifier(Store *store, AiProvider *provider, QObject *parent)
    : QObject(parent)
    , m_store(store)
    , m_provider(provider)
{
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) - the signature is AiProvider::chatFinished
    connect(m_provider, &AiProvider::chatFinished, this, [this](int id, const QString &answer, const QString &error) {
        // Every call answers under an id of its own (AiProvider), and the
        // provider is shared: the connection test of SPEC 7.1 asks over the
        // same object, and its answer is none of this run's business.
        if (!m_busy || id != m_requestId) {
            return;
        }
        m_requestId = -1;

        if (!error.isEmpty()) {
            fail(error);
            return;
        }

        const Classification classification = readClassification(answer, m_noteWrittenOn);
        if (!classification.error.isEmpty()) {
            fail(classification.error);
            return;
        }

        if (!m_store->completeAnalysis(m_noteId, classification.category, classification.tags, classification.task)) {
            fail(m_store->lastError());
            return;
        }

        Q_EMIT classified(m_noteId);
        takeNextNote();
    });
}

void Classifier::start()
{
    if (m_busy) {
        return;
    }

    m_queue.clear();
    const QList<Note> notes = m_store->unanalysedNotes();
    for (const Note &note : notes) {
        // What the counter of SPEC 7.2 skips is reported rather than passed
        // over — including after a restart, which is the only place the tray
        // could learn of a note that was given up on in an earlier run.
        if (note.analysisAttempts >= Store::analysisAttemptLimit) {
            Q_EMIT paused(note.id, note.analysisLastError);
            continue;
        }
        // The budget of SPEC 14, and the loop runs on past it on purpose — see
        // notesPerRun: what it caps is the calls to the model, not the report
        // of the notes that were given up on.
        if (m_queue.size() < notesPerRun) {
            m_queue.append(note);
        }
    }

    // **An unusable backend is a precondition, not a failed attempt** (SPEC 12's
    // rule of issue #23, decided for this run on 30.08.2026, issue #38). Asked
    // after the report above and before the first note is handed to the model,
    // because handing one over is what spends its attempt: with openrouter
    // chosen and no model named, every note of every run would otherwise burn
    // both of its attempts on the same sentence — unattended, every 30 minutes,
    // and long before the user ever opened the settings page.
    //
    // The queue is emptied rather than the function left: takeNextNote() then
    // emits finished(), and the **embedding run still happens**. It talks to
    // Ollama and knows nothing of this backend, so stopping it here would take
    // the topic bundles away for a reason that has nothing to do with them.
    const QString missing = m_provider->unmetPrecondition();
    Q_EMIT notReady(missing);
    if (!missing.isEmpty()) {
        m_queue.clear();
    }

    m_busy = true;
    takeNextNote();
}

bool Classifier::isBusy() const
{
    return m_busy;
}

void Classifier::takeNextNote()
{
    if (m_queue.isEmpty()) {
        m_noteId = -1;
        m_busy = false;
        Q_EMIT finished();
        return;
    }

    const Note note = m_queue.takeFirst();
    m_noteId = note.id;
    // Kept for the answer: the day the note was written is what its relative
    // dates are read from, on the way out and on the way back (SPEC 7.2).
    m_noteWrittenOn = note.createdAt.date();
    m_requestId = m_provider->chat(classificationPrompt(note.content, m_noteWrittenOn));
}

void Classifier::fail(const QString &reason)
{
    const qint64 noteId = m_noteId;
    const std::optional<int> attempts = m_store->failAnalysis(noteId, reason);

    // The count comes out of the database and not out of the note this run read
    // at its start: it is what has to survive a restart (SPEC 7.2), and a run
    // that added up its own would decide on a number nothing wrote down.
    if (attempts.has_value() && *attempts >= Store::analysisAttemptLimit) {
        Q_EMIT paused(noteId, reason);
    } else {
        Q_EMIT failed(noteId, reason);
    }

    takeNextNote();
}
