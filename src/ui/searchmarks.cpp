#include "ui/searchmarks.h"

#include <algorithm>
#include <utility>

namespace
{
bool isCombiningMark(QChar character)
{
    const QChar::Category category = character.category();
    return category == QChar::Mark_NonSpacing || category == QChar::Mark_SpacingCombining
        || category == QChar::Mark_Enclosing;
}

/**
 * `text` folded the way the search index folds it, and beside it, for every
 * code unit of the result, the index of the character it came out of.
 *
 * That second list is the whole point of doing this by hand: a fold changes the
 * length of the text — „ä" written as a with a combining diaeresis is two
 * characters and folds to one — and without the mapping the mark would be drawn
 * where the folded text has it rather than where the user sees it. Wrong by one
 * character looks like a mark, not like a fault.
 *
 * Character by character, not string at a time, because that is what keeps the
 * mapping honest: a decomposition of the whole string could reorder combining
 * marks across characters, and there would be no way back.
 *
 * ponytail: rebuilt rather than bought from SQLite's `highlight()`. Ceiling: the
 * folding is measured to agree with the index today (see the header), and where
 * it ever drifted apart a hit would simply stand unmarked. One drift exists by
 * design — a term under three characters is not in the trigram index and the
 * store compares it with `LIKE '%…%'`, which folds ASCII case and nothing else.
 * That is narrower than it sounds: „ku" typed on its own does not fetch „Küche"
 * at all, because `LIKE '%ku%'` does not match it. A mark the store would not
 * have matched on needs two things at once — the note is in the result through
 * **another** condition (a second term, `tag:`, `kat:`, a date) **and** carries
 * an ASCII „ku" of its own somewhere, while the umlaut stands elsewhere in the
 * same note. „Kuchen für die Küche" under `kat:haushalt ku` is the case, and it
 * marks both. Upgrade path: `highlight()` over an FTS join, which then still
 * needs a second road for exactly those short terms (see `Store::search()`).
 */
QString foldForSearch(const QString &text, QList<qsizetype> *positions)
{
    QString folded;
    for (qsizetype index = 0; index < text.size();) {
        // A surrogate pair is one character and has to stay together — split,
        // its halves would fold to nothing recognisable.
        const qsizetype length = (text.at(index).isHighSurrogate() && index + 1 < text.size()) ? 2 : 1;

        QString stripped;
        const QString decomposed = text.mid(index, length).normalized(QString::NormalizationForm_D);
        for (const QChar character : decomposed) {
            if (!isCombiningMark(character)) {
                stripped.append(character);
            }
        }

        const QString piece = stripped.toCaseFolded();
        if (positions) {
            for (qsizetype unit = 0; unit < piece.size(); ++unit) {
                positions->append(index);
            }
        }
        folded += piece;

        index += length;
    }

    return folded;
}
}

QList<library::SearchMark> library::searchMarks(const QString &text, const QStringList &terms)
{
    if (text.isEmpty() || terms.isEmpty()) {
        return {};
    }

    QList<qsizetype> positions;
    const QString folded = foldForSearch(text, &positions);
    // One past the end, so that a match reaching the last character has an end
    // to be measured against.
    positions.append(text.size());

    QList<SearchMark> found;
    for (const QString &term : terms) {
        const QString needle = foldForSearch(term, nullptr);
        // A term of nothing but combining marks folds away entirely, and an
        // empty needle stands at every position.
        if (needle.isEmpty()) {
            continue;
        }

        for (qsizetype at = folded.indexOf(needle); at >= 0; at = folded.indexOf(needle, at + needle.size())) {
            const qsizetype start = positions.at(at);
            const qsizetype end = positions.at(at + needle.size());
            // Both ends name the character a folded unit came out of, so both
            // fall back to a character boundary — and both to the same side,
            // which is **not** outwards on each end: a character whose fold is
            // matched only in part is taken in at the front and left out at the
            // back. What that buys is the thing worth having, that no character
            // is ever marked by halves; where the whole partial match lies
            // inside one character, `end == start` and nothing is marked.
            //
            // It takes a fold that grows to reach this case at all, and an
            // umlaut shrinks. A Hangul syllable grows: „각" is one character and
            // three units decomposed, no combining mark among them. Measured on
            // this function, 29.08.2026 — a term matching its last jamo marks
            // the whole syllable, one matching its first two marks nothing, and
            // one running out of it into the next syllable marks the first
            // alone.
            if (end > start) {
                found.append(SearchMark{start, end - start});
            }
        }
    }

    std::sort(found.begin(), found.end(), [](const SearchMark &left, const SearchMark &right) {
        return left.start < right.start;
    });

    // Two terms that meet in the same word become one mark. Drawn separately
    // they would put two boxes over the same letters, and the overlap would
    // show as a seam.
    QList<SearchMark> merged;
    for (const SearchMark &mark : std::as_const(found)) {
        if (!merged.isEmpty() && mark.start <= merged.last().start + merged.last().length) {
            SearchMark &open = merged.last();
            open.length = std::max(open.length, mark.start + mark.length - open.start);
            continue;
        }
        merged.append(mark);
    }

    return merged;
}
