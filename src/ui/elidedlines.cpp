#include "ui/elidedlines.h"

#include "ui/searchmarks.h"

#include <QFont>
#include <QFontMetrics>
#include <QStringList>
#include <QTextLayout>
#include <QTextLine>

#include <algorithm>

namespace
{
/**
 * How far in front of a match a word boundary is looked for, and where the cut
 * falls when there is none.
 *
 * In prose this number changes nothing: a space stands directly in front of the
 * match, the cut lands there, and the excerpt opens with the found word itself.
 * It decides the other case — a match sitting inside a token longer than this,
 * where the cut is made hard, twelve characters in front of the match, so that
 * the excerpt does not open with the tail of a single word.
 *
 * Measured 29.08.2026 on `subjectAndPreview()`: a line of shopping gives
 * „…Kuchen holen.", a match buried in a run of As gives
 * „…AAAAAAAAAAAAKuchenBBBBBBBB" — twelve of them. With the floor lifted the
 * second one falls back to the last space before the token and reads
 * „…AAAA…" with the match out of sight again, which is what the excerpt was
 * moved for. Prose is unchanged either way, so this is the only case the
 * number carries.
 */
constexpr qsizetype LeadIn = 12;

/**
 * The first line `text` breaks into at `width`, and what is left over.
 *
 * Line breaks are the caller's business — `text` is expected to hold none.
 */
QString firstLine(const QString &text, const QFont &font, int width, QString *rest)
{
    QTextLayout layout(text, font);
    layout.beginLayout();
    QTextLine line = layout.createLine();
    if (!line.isValid()) {
        layout.endLayout();
        return {};
    }
    line.setLineWidth(width);
    layout.endLayout();

    const int placed = line.textStart() + line.textLength();
    *rest = text.mid(placed).trimmed();

    return text.left(placed).trimmed();
}

/**
 * `preview` moved so that the first match stands in the line (issue #77).
 *
 * The line is cut in pixels and the match sits at a character, so where to cut
 * is asked of the same eliding that draws it: what `elidedText()` keeps is what
 * the user sees, and only a match beyond that moves anything. Everything in
 * front of the new beginning is replaced by „…" — the same character the
 * eliding uses at the other end, and for the same reason.
 */
QString excerptAtMatch(const QString &preview, const QStringList &terms, const QFontMetrics &metrics, int width)
{
    // ponytail: the whole remainder of the note is folded, once per drawn
    // preview line and per repaint, although only the first match is wanted.
    // Ceiling, measured 29.08.2026 (-O2, offscreen, 50 runs per length): 556 µs
    // at 10 kB and 2.79 ms at 50 kB, against 868 µs and 4.33 ms for the
    // `elidedText()` that stood at this place anyway — 64 % on top of what the
    // line already cost. At the 200 characters a note usually carries it is
    // 12.6 µs per line and invisible; ten visible lines of 10 kB each come to
    // roughly 5.6 ms per repaint, and that last number is an estimate off the
    // single calls, not a scroll anybody held a clock to. It neither adds to
    // nor hides inside the 120 ms a broad query costs at 20,000 notes: that is
    // the store, this is the painting, and this one grows with the **length**
    // of a note rather than with their number, because the list draws only the
    // lines it shows. Ways up: fold no further than the first match instead of
    // to the end, or keep the folded copy per line.
    const QList<library::SearchMark> marks = library::searchMarks(preview, terms);
    if (marks.isEmpty()) {
        return preview;
    }

    const QString shown = metrics.elidedText(preview, Qt::ElideRight, width);
    if (shown == preview) {
        return preview;
    }

    // What `elidedText()` kept is its answer minus the one ellipsis character
    // it appends.
    const qsizetype visible = shown.size() - 1;
    const library::SearchMark first = marks.constFirst();
    if (first.start + first.length <= visible) {
        return preview;
    }

    const qsizetype earliest = std::max<qsizetype>(0, first.start - LeadIn);
    const qsizetype space = preview.lastIndexOf(QLatin1Char(' '), first.start);
    // Whole words in front of the match where one begins close enough, a hard
    // cut where none does — a long word would otherwise push the match back out
    // of the line it was moved into.
    const qsizetype from = (space >= earliest && space < first.start) ? space + 1 : earliest;
    if (from <= 0) {
        return preview;
    }

    return QStringLiteral("…") + preview.mid(from);
}
}

library::EntryText library::subjectAndPreview(const QString &text, const QFont &font, int width,
                                              const QStringList &terms)
{
    if (text.trimmed().isEmpty() || width <= 0) {
        return {};
    }

    // The first line break ends the subject; everything after it is preview,
    // however many lines it holds.
    const qsizetype breakAt = text.indexOf(QLatin1Char('\n'));
    const QString head = (breakAt < 0 ? text : text.left(breakAt)).simplified();
    const QString tail = breakAt < 0 ? QString() : text.mid(breakAt + 1);

    QString overflow;
    const QString subject = firstLine(head, font, width, &overflow);

    QStringList rest;
    if (!overflow.isEmpty()) {
        rest.append(overflow);
    }
    const QStringList tailLines = tail.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : tailLines) {
        const QString simplified = line.simplified();
        if (!simplified.isEmpty()) {
            rest.append(simplified);
        }
    }

    const QFontMetrics metrics(font);

    // The breaks of a multi-line remainder are read as separators
    // (wireframe 3b).
    const QString separator = QStringLiteral(" · ");

    EntryText entry;
    // Eliding the subject changes nothing while it fits — it is the answer to
    // the single word that is wider than the whole list.
    entry.subject = metrics.elidedText(subject, Qt::ElideRight, width);
    // Eliding the joined remainder rather than each line puts the ellipsis at
    // the end of the text instead of at a word break.
    entry.preview = metrics.elidedText(excerptAtMatch(rest.join(separator), terms, metrics, width),
                                       Qt::ElideRight,
                                       width);

    return entry;
}
