#pragma once

#include <QStringList>

class QFont;

namespace library
{
/**
 * Breaks `text` into at most `maxLines` lines of `width` pixels, eliding the
 * last one (wireframe 2b: two lines of note text, then an ellipsis).
 *
 * Line breaks inside the note collapse into single spaces — a list entry is a
 * preview, not a rendering of the note.
 */
QStringList elidedLines(const QString &text, const QFont &font, int width, int maxLines);
}
