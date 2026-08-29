#include "analysis/suggester.h"

#include "analysis/aiprovider.h"
#include "analysis/clustering.h"
#include "analysis/modelanswer.h"
#include "store/store.h"

#include <KLocalizedString>

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
 * The notes of one cluster, in the order the clustering put them in.
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

Suggester::Suggester(Store *store, AiProvider *provider, const QString &embeddingModel, QObject *parent)
    : QObject(parent)
    , m_store(store)
    , m_provider(provider)
    , m_embeddingModel(embeddingModel)
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

        Proposal proposal;
        proposal.kind = Proposal::Kind::Bundle;
        proposal.createdAt = QDateTime::currentDateTime();
        proposal.status = Proposal::Status::Open;
        proposal.payload = QString::fromUtf8(
            QJsonDocument(QJsonObject{{QLatin1String("title"), naming.title},
                                      {QLatin1String("markdown"), bundleMarkdown(naming.title, kept)}})
                .toJson(QJsonDocument::Compact));
        proposal.noteIds = naming.noteIds;

        const std::optional<qint64> proposalId = m_store->addProposal(proposal);
        if (!proposalId.has_value()) {
            Q_EMIT failed(m_store->lastError());
            takeNextCluster();
            return;
        }

        // The deferred suggestion this one replaces goes **after** the new one
        // stands, not before: a call that failed would otherwise have taken the
        // old card away and put nothing in its place (UX decision of
        // 29.08.2026, see the class comment).
        const QList<Proposal> standing = m_store->proposals();
        for (const Proposal &other : standing) {
            if (other.id == *proposalId || other.kind != Proposal::Kind::Bundle
                || other.status != Proposal::Status::Deferred) {
                continue;
            }
            const bool sharesANote = std::any_of(other.noteIds.cbegin(), other.noteIds.cend(),
                                                 [&naming](qint64 noteId) {
                                                     return naming.noteIds.contains(noteId);
                                                 });
            if (sharesANote && !m_store->removeProposal(other.id)) {
                Q_EMIT failed(m_store->lastError());
            }
        }

        Q_EMIT suggested(*proposalId);
        takeNextCluster();
    });
}

void Suggester::start()
{
    if (m_busy) {
        return;
    }

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
    const QList<NoteEmbedding> stored = m_store->embeddings(m_embeddingModel);
    for (const NoteEmbedding &embedding : stored) {
        if (!spokenFor.contains(embedding.noteId)) {
            corpus.append(embedding);
        }
    }

    m_clusters.clear();
    const QList<QList<qint64>> clusters = clusterNotes(corpus, bundleThreshold());
    for (const QList<qint64> &cluster : clusters) {
        const QList<Note> notes = clusterNotesFromStore(m_store, cluster);
        if (notes.size() >= bundleThreshold()) {
            m_clusters.append(notes);
        }
    }

    m_busy = true;
    takeNextCluster();
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
        Q_EMIT finished();
        return;
    }

    m_cluster = m_clusters.takeFirst();
    m_requestId = m_provider->chat(bundlePrompt(m_cluster));
}
