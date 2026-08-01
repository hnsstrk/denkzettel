#include "ui/elidedlines.h"

#include <QFont>
#include <QFontMetrics>
#include <QStringList>
#include <QTextLayout>
#include <QTextLine>

namespace
{
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
}

library::EntryText library::subjectAndPreview(const QString &text, const QFont &font, int width)
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
    entry.preview = metrics.elidedText(rest.join(separator), Qt::ElideRight, width);

    return entry;
}
