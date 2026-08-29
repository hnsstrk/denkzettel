#pragma once

#include "store/note.h"

#include <QList>

/**
 * The similarity from which on two notes belong to the same topic (SPEC 7.3).
 *
 * **An internal constant and no setting**, and SPEC 7.3 says so: the number is
 * calibrated against one embedding model, and a slider would let the user turn
 * a corpus into one cluster or into none without any way of telling which of
 * the two happened. It is calibratable in the sense that it is written down
 * once and changed here when a measurement against real notes says so.
 *
 * **Where the value was measured, and what moving it costs.** Against the real
 * bge-m3 on 2026-08-29, with invented notes — the user's own library is what
 * the calibration of issue #28 still owes, and it is the only thing that can
 * settle the number for their writing:
 *
 * - Nine notes in three unmistakable topics (photo backup, bicycle, garden).
 *   Similarity **within** a topic ran from 0.549 to 0.673, **across** two
 *   topics up to 0.561 — **the two bands overlap**, so no threshold separates
 *   them cleanly and every value is a trade.
 * - At 0.60 two of the three topics came out as bundles and the garden fell
 *   apart. At 0.65 nothing at all was left. At 0.55 the bicycle and the garden
 *   **melted into one bundle of six**, and at 0.50 all nine notes became a
 *   single bundle.
 * - Ten unrelated notes, 45 pairs: the closest of them — a dentist's
 *   appointment and a tax return — stood at 0.614, and **not one bundle** came
 *   out at 0.60 (a pair is no bundle, see bundle::MinimumNotes). At 0.55 five
 *   of the ten chained into one, at 0.50 seven did.
 *
 * So the two directions cost different things: too strict misses bundles the
 * user would have wanted, and they stay in the corpus where nothing is lost.
 * Too loose throws unrelated notes together and hands the user a suggestion
 * they have to read and refuse. 0.60 is the value at which the second did not
 * happen once in these runs.
 */
inline constexpr double clusterSimilarity = 0.60;

/**
 * The cosine of the angle between two vectors: 1 for the same direction, 0 for
 * a right angle, -1 for the opposite one.
 *
 * Two vectors of **different length** cannot be compared and answer 0 rather
 * than reading past the end of the shorter one — that is what a BLOB read with
 * the wrong element size looks like, and 0 keeps such a note out of every
 * cluster instead of putting it into a wrong one. A vector of nothing but
 * zeros has no direction and answers 0 for the same reason.
 *
 * Summed in `double` although the components are `float`: 1024 of them (bge-m3)
 * add up, and the threshold above is decided in the third decimal place.
 */
double cosineSimilarity(const QList<float> &left, const QList<float> &right);

/**
 * The single-linkage clusters of SPEC 7.3, each with at least `minimumNotes`
 * notes.
 *
 * **Single linkage means the chain counts, not the pair.** A and B above the
 * threshold, B and C above it, A and C below it: that is **one** cluster of
 * three, not two of two. Whoever asks for every pair of a cluster to be
 * similar gets a different answer for the same corpus, and with two notes per
 * cluster the two answers are indistinguishable — the check for this builds
 * exactly that chain (CLAUDE.md, finding 34).
 *
 * Notes without a cluster simply stay out of the answer (SPEC 7.3); what
 * becomes of them is the overflow guard's business (#34), not this function's.
 * The order is the order of the input, and inside a cluster the order the chain
 * was walked in — so the same corpus always yields the same clusters, and a
 * suggestion does not change its notes between two runs.
 *
 * ponytail: every pair is compared, O(n²). SPEC 7.3 does the sum itself — at
 * the 200 notes of the overflow threshold that is 20,000 comparisons, and no
 * vector database. The way up, if the corpus ever gets bigger, is an index over
 * the vectors.
 */
QList<QList<qint64>> clusterNotes(const QList<NoteEmbedding> &embeddings, int minimumNotes);

/**
 * What `[Export] BundleNotes` may say (SPEC 7.3), and the one place those
 * values stand.
 *
 * Two parties need the same ones: bundleThreshold() below, which reads
 * `denkzettelrc` at runtime, and the settings skeleton, which offers the same
 * default and the same bounds on the form — `settings.{h,cpp}` is built from
 * the numbers here, so there is nothing to keep in step.
 *
 * **The floor is two and it is not a matter of taste**: at one, every note is a
 * cluster of its own and the whole corpus becomes a bundle. It is the floor of
 * the export page's other two thresholds as well, for a reason of its own —
 * a spin box whose suffix is a fixed plural can never show a singular.
 *
 * Here and not in the settings, because the dependency only runs one way:
 * `denkzettelsettings` links `denkzettelanalysis`, not the other way round.
 * That is where `ollama::` and `analysis::` stand and for the same reason.
 */
namespace bundle
{
inline constexpr int DefaultNotes = 3;
inline constexpr int MinimumNotes = 2;
inline constexpr int MaximumNotes = 100;
}

/**
 * How many notes a cluster needs before it is laid before the model
 * (SPEC 7.3): `[Export] BundleNotes` out of `denkzettelrc`, default 3.
 *
 * Read here rather than taken from the settings skeleton, the way
 * OllamaProvider reads its address and its models. The key is the one the
 * export page writes (#75) — a second spelling would be a second setting that
 * nobody can find.
 *
 * **Clamped to the bounds above, and that is not belt and braces**: what is
 * read here is a `denkzettelrc` that nobody promised went through the dialog,
 * and `BundleNotes=1` in a hand-written file turns every note into its own
 * bundle while a 0 or a negative number does the same without even looking
 * odd. The item's own bounds only reach what the dialog writes.
 */
int bundleThreshold();
