#pragma once

#include <QString>
#include <QStringList>

class QFont;

namespace library
{
/** The two lines of text a list entry shows (wireframe 3a). */
struct EntryText {
    /** The first line of the note, in text colour. */
    QString subject;
    /** What follows it, dimmed; empty for a note of a single line. */
    QString preview;
};

/**
 * Splits a note into the subject and the preview a list entry shows
 * (wireframe 3a/3b, SPEC 9).
 *
 * The subject is the text up to the first line break *or* up to where `width`
 * is used up — whichever comes first. It is elided and never wrapped, so it
 * always takes exactly one line.
 *
 * The preview is the text following it, on one line as well: what did not fit
 * into the subject, then the remaining lines of the note with their breaks
 * read as separators. It is elided at the end of the text.
 *
 * `terms` are the search terms of the moment (`SearchQuery::terms`), and they
 * decide **where** the preview begins: a match beyond the end of the line moves
 * the excerpt to itself instead of leaving the user with an entry that does not
 * say why it is a hit (issue #77). A leading „…" says that something stands in
 * front of it. Without terms the preview begins at the start of the text, as it
 * always did.
 */
EntryText subjectAndPreview(const QString &text, const QFont &font, int width, const QStringList &terms);
}
