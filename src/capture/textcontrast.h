#pragma once

#include <QColor>

namespace capture
{

/**
 * The contrast ratio of two colours after WCAG 2.1, between 1 and 21.
 *
 * Its own unit rather than a helper inside the window, because nobody checks a
 * colour formula by looking: the capture window picks between two writings with
 * it, and which one it picks depends on the desktop theme and the colour scheme
 * together. A user runs one of those combinations and would never see the
 * choice go wrong under the others (issue #97).
 */
double contrastRatio(const QColor &one, const QColor &other);

/**
 * The four colours the choice between the two writings is made from.
 *
 * A struct and not four parameters: all four are colours, and a swap of the
 * first two turns the answer into its opposite without any compiler noticing.
 * Issue #88 is what that costs when it happens — there a swap of two strings
 * put the hint where the heading belonged and no check saw it. Named fields at
 * the call site make the mistake visible where it would be made.
 */
struct WritingChoice
{
    /** The writing a note is set in once something is typed. */
    QColor note;
    /** The writing that stands in the empty field and asks for that note. */
    QColor placeholder;
    /** The ground under both, with a white screen behind the window ... */
    QColor groundOverWhite;
    /** ... and with a black one. */
    QColor groundOverBlack;
};

/**
 * Whether the note text is the quieter of the two writings that share the
 * field, judged on the poorer of the two grounds.
 *
 * The window lets the screen behind it through, so what lies under the writing
 * depends on what the user has there. Each colour is therefore taken at its
 * poorer contrast over the two grounds, and the note counts as the quieter one
 * where its poorer case is worse than the placeholder's.
 */
bool noteIsTheQuieterWriting(const WritingChoice &choice);

}
