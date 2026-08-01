#pragma once

#include <QString>

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
 */
EntryText subjectAndPreview(const QString &text, const QFont &font, int width);
}
