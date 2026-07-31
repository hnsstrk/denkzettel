#pragma once

namespace capture
{

/** The text area rests at five lines and stops growing at eight (SPEC 3). */
inline constexpr int MinTextLines = 5;
inline constexpr int MaxTextLines = 8;

/**
 * Height in pixels of a text area showing a document of `documentLines` lines,
 * clamped to the range above: below the minimum the window would flicker on
 * every keystroke, above the maximum a scrollbar takes over.
 *
 * `lineHeight` is the height of a single line, `chrome` the vertical space the
 * widget needs beyond the text itself (document margins, frame).
 */
int textAreaHeight(int documentLines, int lineHeight, int chrome);

}
