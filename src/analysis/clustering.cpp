#include "analysis/clustering.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <algorithm>
#include <cmath>

double cosineSimilarity(const QList<float> &left, const QList<float> &right)
{
    if (left.isEmpty() || left.size() != right.size()) {
        return 0.0;
    }

    double dot = 0.0;
    double leftLength = 0.0;
    double rightLength = 0.0;
    for (qsizetype index = 0; index < left.size(); ++index) {
        const double one = left.at(index);
        const double other = right.at(index);
        dot += one * other;
        leftLength += one * one;
        rightLength += other * other;
    }

    if (leftLength <= 0.0 || rightLength <= 0.0) {
        return 0.0;
    }
    return dot / std::sqrt(leftLength * rightLength);
}

QList<QList<qint64>> clusterNotes(const QList<NoteEmbedding> &embeddings, int minimumNotes)
{
    QList<QList<qint64>> clusters;
    QList<bool> taken(embeddings.size(), false);

    for (qsizetype seed = 0; seed < embeddings.size(); ++seed) {
        if (taken.at(seed)) {
            continue;
        }

        // The chain: every note taken into the cluster is asked in turn which
        // of the remaining ones it reaches. That is what makes A–B–C one
        // cluster although A and C are not similar to each other.
        taken[seed] = true;
        QList<qsizetype> chain = {seed};
        QList<qint64> cluster;
        for (qsizetype at = 0; at < chain.size(); ++at) {
            const NoteEmbedding &member = embeddings.at(chain.at(at));
            cluster.append(member.noteId);
            for (qsizetype other = 0; other < embeddings.size(); ++other) {
                if (taken.at(other)) {
                    continue;
                }
                if (cosineSimilarity(member.vector, embeddings.at(other).vector) >= clusterSimilarity) {
                    taken[other] = true;
                    chain.append(other);
                }
            }
        }

        if (cluster.size() >= minimumNotes) {
            clusters.append(cluster);
        }
    }

    return clusters;
}

int bundleThreshold()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Export"));
    return std::clamp(group.readEntry("BundleNotes", bundle::DefaultNotes), bundle::MinimumNotes, bundle::MaximumNotes);
}
