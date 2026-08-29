#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace library
{
/** A stretch of text a search term matched, in indices of that very text. */
struct SearchMark {
    qsizetype start = 0;
    qsizetype length = 0;
};

/**
 * Where the terms of the search field stand in `text` (issue #77, SPEC 6).
 *
 * The comparison is the one the search itself makes, rebuilt here: both sides
 * are decomposed, their combining marks dropped and case folded, so „kuche"
 * marks „Kuchen" and „muhe" marks „Mühe" — the index finds those notes just the
 * same, and the mark only says so.
 *
 * Measured against SQLite as the outside value on 29.08.2026: over 32 samples,
 * all 1,024 pairs of „does the trigram index with `remove_diacritics 1` call
 * these two equal" came out identical to this folding — ä Ä å é ç ñ İ š and Σ ς
 * folded on both sides, ß ø æ and the ligature ﬁ folded on neither.
 *
 * The marks come back sorted by their start and never overlap: two terms
 * meeting in the same word are one mark, not two boxes drawn on top of one
 * another.
 */
QList<SearchMark> searchMarks(const QString &text, const QStringList &terms);
}
