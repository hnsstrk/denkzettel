#include "analysis/suggester.h"

#include "analysis/aiprovider.h"
#include "analysis/clustering.h"
#include "analysis/modelanswer.h"
#include "store/overflowthreshold.h"
#include "store/store.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

#include <algorithm>
#include <optional>
#include <utility>

namespace
{
/**
 * The notes of one cluster, **oldest first**.
 *
 * Not in the order the clustering hands them over, and that is the whole point
 * of the sort: clusterNotes() answers in the order the chain was walked
 * (clustering.h), and a chain runs A → C → B whenever A and B are only related
 * through C — which is the case single linkage exists for. Measured on
 * 29.08.2026 with three notes at 0°, 100° and 50° on the days 08-01, 08-01 and
 * 08-02: the cluster comes back as 1, 3, 2, and the collective note built from
 * it carried `## 2026-08-01`, `## 2026-08-02` and `## 2026-08-01` — the same
 * day heading twice, because bundleMarkdown() opens a section wherever the day
 * changes. SPEC 8.1 asks for chronological, so the order is put right here,
 * once: the numbering the model is handed, the references written into
 * `proposal_notes` and the Markdown all come out of this list.
 *
 * The id breaks a tie, so two notes of the same millisecond keep one fixed
 * order and a suggestion does not change its notes between two runs.
 *
 * A note the clustering named and the database no longer holds is left out
 * rather than ending the cluster: the vector goes with the note (ON DELETE
 * CASCADE), so this is the narrow window between the two reads of one run.
 */
QList<Note> clusterNotesFromStore(Store *store, const QList<qint64> &noteIds)
{
    QList<Note> notes;
    for (const qint64 noteId : noteIds) {
        const std::optional<Note> note = store->note(noteId);
        if (note.has_value()) {
            notes.append(*note);
        }
    }
    std::sort(notes.begin(), notes.end(), [](const Note &left, const Note &right) {
        return left.createdAt == right.createdAt ? left.id < right.id : left.createdAt < right.createdAt;
    });
    return notes;
}
}

QString bundlePrompt(const QList<Note> &notes)
{
    QString numbered;
    for (qsizetype index = 0; index < notes.size(); ++index) {
        numbered += QStringLiteral("%1. %2\n\n").arg(index + 1).arg(notes.at(index).content);
    }

    return QStringLiteral(
               "These short personal notes, most of them written in German, came out of "
               "one cluster: their texts are similar to each other. "
               "Name the topic they share, and say which of them really belong to it.\n"
               "\n"
               "Answer with one JSON object and nothing else.\n"
               "\n"
               "{\"title\": a short name for the topic, in the language of the notes,\n"
               " \"notes\": the numbers of the notes that belong to the topic}\n"
               "\n"
               "Leave a note out of \"notes\" when it does not fit the others. "
               "Do not rewrite the notes and do not summarise them.\n"
               "\n"
               "Notes:\n"
               "%1")
        .arg(numbered);
}

BundleNaming readBundleNaming(const QString &answer, const QList<Note> &notes)
{
    const QJsonObject object = modelAnswerObject(answer, QLatin1String("title"));
    if (object.isEmpty()) {
        return {{}, {}, i18n("The model's answer carried no JSON object.")};
    }

    const QString title = object.value(QLatin1String("title")).toString().trimmed();
    if (title.isEmpty()) {
        return {{}, {}, i18n("The model's answer carried no title for the bundle.")};
    }

    // The numbers are collected first and the notes taken in the order they
    // were laid before the model: the bundle is chronological (SPEC 8.1), and
    // an answer that names them in another order must not change that.
    QSet<qsizetype> kept;
    const QJsonArray numbers = object.value(QLatin1String("notes")).toArray();
    for (const auto &number : numbers) {
        // Through QVariant, because QJsonValue::toInt() answers 0 for a number
        // a model wrote as the string "3" — and 0 is out of range, so the note
        // would be dropped from its own bundle without a word.
        const qsizetype position = qsizetype(number.toVariant().toLongLong());
        if (position >= 1 && position <= notes.size()) {
            kept.insert(position - 1);
        }
    }
    if (kept.isEmpty()) {
        return {{}, {}, i18n("The model's answer named none of the notes of the bundle.")};
    }

    BundleNaming naming;
    naming.title = title;
    for (qsizetype index = 0; index < notes.size(); ++index) {
        if (kept.contains(index)) {
            naming.noteIds.append(notes.at(index).id);
        }
    }
    return naming;
}

QString bundleMarkdown(const QString &title, const QList<Note> &notes)
{
    QString markdown = QStringLiteral("# %1\n").arg(title);

    QDate day;
    for (const Note &note : notes) {
        const QDate noteDay = note.createdAt.date();
        if (noteDay != day) {
            day = noteDay;
            markdown += QStringLiteral("\n## %1\n").arg(day.toString(Qt::ISODate));
        }
        // The text as it stands, and a blank line around it: two notes of one
        // day are two paragraphs, and a note that carries its own line breaks
        // keeps them (SPEC 8.1, "the notes as paragraphs").
        markdown += QStringLiteral("\n%1\n").arg(note.content.trimmed());
    }

    return markdown;
}

Suggester::Suggester(Store *store, AiProvider *provider, QObject *parent)
    : QObject(parent)
    , m_store(store)
    , m_provider(provider)
    , m_embeddingModel(provider->embeddingModel())
    , m_embeddingService(provider->serviceId())
{
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) - the signature is AiProvider::chatFinished
    connect(m_provider, &AiProvider::chatFinished, this, [this](int id, const QString &answer, const QString &error) {
        // Every call answers under an id of its own (AiProvider), and the
        // provider is shared: the classification run and the connection test of
        // SPEC 7.1 ask over the same object.
        if (!m_busy || id != m_requestId) {
            return;
        }
        m_requestId = -1;

        if (!error.isEmpty()) {
            Q_EMIT failed(error);
            takeNextCluster();
            return;
        }

        const BundleNaming naming = readBundleNaming(answer, m_cluster);
        if (!naming.error.isEmpty()) {
            Q_EMIT failed(naming.error);
            takeNextCluster();
            return;
        }

        // What the sanity check left has to be a bundle in its own right: below
        // the threshold of SPEC 7.3 the notes are no bundle, and they stay in
        // the corpus for the next run rather than becoming a suggestion the
        // setting rules out.
        if (naming.noteIds.size() < bundleThreshold()) {
            Q_EMIT failed(i18n("The model kept %1 of %2 notes of the bundle, fewer than a bundle needs.",
                               naming.noteIds.size(),
                               m_cluster.size()));
            takeNextCluster();
            return;
        }

        QList<Note> kept;
        for (const Note &note : std::as_const(m_cluster)) {
            if (naming.noteIds.contains(note.id)) {
                kept.append(note);
            }
        }

        writeBundle(naming.title, kept);
        takeNextCluster();
    });
}

void Suggester::writeBundle(const QString &title, const QList<Note> &notes)
{
    QList<qint64> noteIds;
    noteIds.reserve(notes.size());
    for (const Note &note : notes) {
        noteIds.append(note.id);
    }

    Proposal proposal;
    proposal.kind = Proposal::Kind::Bundle;
    proposal.createdAt = QDateTime::currentDateTime();
    proposal.status = Proposal::Status::Open;
    proposal.payload = QString::fromUtf8(
        QJsonDocument(QJsonObject{{QLatin1String("title"), title},
                                  {QLatin1String("markdown"), bundleMarkdown(title, notes)}})
            .toJson(QJsonDocument::Compact));
    proposal.noteIds = noteIds;

    const std::optional<qint64> proposalId = m_store->addProposal(proposal);
    if (!proposalId.has_value()) {
        Q_EMIT failed(m_store->lastError());
        return;
    }

    // The deferred suggestion this one replaces goes **after** the new one
    // stands, not before: a call that failed would otherwise have taken the
    // old card away and put nothing in its place (UX decision of
    // 29.08.2026, see the class comment).
    const QList<Proposal> standing = m_store->proposals();
    bool tookADeferredOnesPlace = false;
    for (const Proposal &other : standing) {
        if (other.id == *proposalId || other.kind != Proposal::Kind::Bundle
            || other.status != Proposal::Status::Deferred) {
            continue;
        }
        const bool sharesANote = std::any_of(other.noteIds.cbegin(), other.noteIds.cend(),
                                             [&noteIds](qint64 noteId) {
                                                 return noteIds.contains(noteId);
                                             });
        if (!sharesANote) {
            continue;
        }
        if (m_store->removeProposal(other.id)) {
            tookADeferredOnesPlace = true;
        } else {
            Q_EMIT failed(m_store->lastError());
        }
    }

    // And only now is it decided whether this is news (see finished()).
    // The removal is what the question hangs on and not the finding of it:
    // where it failed, the old card is still standing beside the new one,
    // and then there really are two questions where there was one.
    //
    // **Here and not in the lambda above**, which is where it stood until the
    // miscellaneous bundle existed (#40, #35): counted there, a run whose only
    // new suggestion is that bundle reports 0 and the notification of SPEC 10
    // stays mute — the shape of finding 94, in the very code that was repaired
    // for it. One writer of a bundle suggestion, one place that counts it.
    if (!tookADeferredOnesPlace) {
        ++m_newSuggestions;
    }

    Q_EMIT suggested(*proposalId);
}

void Suggester::offerMiscellaneous(const QList<NoteEmbedding> &corpus, const QSet<qint64> &clustered)
{
    QList<qint64> loose;
    for (const NoteEmbedding &embedding : corpus) {
        if (!clustered.contains(embedding.noteId)) {
            loose.append(embedding.noteId);
        }
    }

    // The group `bundleThreshold()` reads its own setting out of, and the one
    // the export page writes (#75). The limit itself comes from
    // overflowDayLimit(), so the key, the default and the floor are the guard's
    // own (store/overflowthreshold.h).
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Export"));
    const int dayLimit = overflowDayLimit(group);
    const QDateTime now = QDateTime::currentDateTime();

    // The same comparison as `tooOld` in overflowReport(), so that the bundle
    // comes into being for exactly the notes the guard is reminding about: the
    // note that has waited **as long as** the threshold is in, the one a day
    // younger is not.
    QList<Note> old;
    const QList<Note> waiting = clusterNotesFromStore(m_store, loose);
    for (const Note &note : waiting) {
        if (note.createdAt.daysTo(now) >= dayLimit) {
            old.append(note);
        }
    }
    if (old.isEmpty()) {
        return;
    }

    // Oldest first out of clusterNotesFromStore(), so the two days are the ends
    // of the period and the Markdown is chronological (SPEC 8.1).
    //
    // The days in ISO form and not in the locale's: this title is the heading
    // of the collective note, whose day sections carry that very form, and the
    // export of SPEC 8.1 builds a file name out of it — where a short date
    // would put a slash of the locale's choosing into a path.
    const QDate first = old.constFirst().createdAt.date();
    const QDate last = old.constLast().createdAt.date();
    const QString title = first == last
        ? i18n("Miscellaneous from %1", first.toString(Qt::ISODate))
        : i18n("Miscellaneous from %1 to %2", first.toString(Qt::ISODate), last.toString(Qt::ISODate));

    writeBundle(title, old);
}

void Suggester::start()
{
    if (m_busy) {
        return;
    }

    // After the guard and before the first row: a second start() that finds a
    // run going changes nothing and must not reset what that run has counted.
    m_newSuggestions = 0;

    // The task suggestions of SPEC 7.4 first, and without a model: the fields
    // were extracted by the classification of SPEC 7.2 and are handed on as
    // they stand. Nothing is carried out — SPEC 7.4 rules out an automatic
    // `task add`.
    const QList<Note> tasks = m_store->notesForTaskProposals();
    for (const Note &note : tasks) {
        Proposal proposal;
        proposal.kind = Proposal::Kind::Task;
        proposal.createdAt = QDateTime::currentDateTime();
        proposal.status = Proposal::Status::Open;
        proposal.payload = note.task;
        proposal.noteIds = {note.id};

        const std::optional<qint64> proposalId = m_store->addProposal(proposal);
        if (!proposalId.has_value()) {
            Q_EMIT failed(m_store->lastError());
            continue;
        }
        // Always a new question: a note that already carries a task suggestion
        // gets no second one, whatever that one's status (see the class
        // comment), so nothing here can be a repeat.
        ++m_newSuggestions;
        Q_EMIT suggested(*proposalId);
    }

    // A note whose question is already standing is held out of the corpus, a
    // deferred one is not — that is the whole difference between the two
    // statuses (see the class comment).
    QSet<qint64> spokenFor;
    const QList<Proposal> standing = m_store->proposals();
    for (const Proposal &proposal : standing) {
        if (proposal.kind == Proposal::Kind::Bundle && proposal.status == Proposal::Status::Open) {
            for (const qint64 noteId : proposal.noteIds) {
                spokenFor.insert(noteId);
            }
        }
    }

    QList<NoteEmbedding> corpus;
    const QList<NoteEmbedding> stored = m_store->embeddings(m_embeddingModel, m_embeddingService);
    for (const NoteEmbedding &embedding : stored) {
        if (!spokenFor.contains(embedding.noteId)) {
            corpus.append(embedding);
        }
    }

    m_clusters.clear();
    QSet<qint64> clustered;
    const QList<QList<qint64>> clusters = clusterNotes(corpus, bundleThreshold());
    for (const QList<qint64> &cluster : clusters) {
        for (const qint64 noteId : cluster) {
            clustered.insert(noteId);
        }
        const QList<Note> notes = clusterNotesFromStore(m_store, cluster);
        if (notes.size() >= bundleThreshold()) {
            m_clusters.append(notes);
        }
    }

    // The old notes of this corpus that no cluster took, as one bundle
    // (SPEC 7.3 and 11). Here and not after the calls above, because what is
    // clustered is decided by clusterNotes() and not by the model: a note the
    // sanity check drops stays in the corpus for the next run, as it did
    // before this bundle existed.
    offerMiscellaneous(corpus, clustered);

    m_busy = true;
    takeNextCluster();
}

void Suggester::reloadSettings()
{
    m_embeddingModel = m_provider->embeddingModel();
    m_embeddingService = m_provider->serviceId();
}

bool Suggester::isBusy() const
{
    return m_busy;
}

void Suggester::takeNextCluster()
{
    if (m_clusters.isEmpty()) {
        m_cluster.clear();
        m_busy = false;
        Q_EMIT finished(m_newSuggestions);
        return;
    }

    m_cluster = m_clusters.takeFirst();
    m_requestId = m_provider->chat(bundlePrompt(m_cluster));
}
