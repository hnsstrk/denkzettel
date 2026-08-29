#pragma once

#include <QColor>

namespace library
{
/**
 * A ground and a text colour mixed in the ratio `KColorScheme::frameContrast()`
 * — the way Kirigami colours its own separators (issue #101).
 *
 * It carries the two separator lines of the note list (wireframe 3a) and the
 * outline of a tag chip in the reading pane (issue #18). Deliberately not a
 * palette role: measured over eighteen colour schemes, the roles near the
 * ground stay under 1.21 : 1 against it and AlternateBase lands at 1.00 : 1 at
 * every group boundary, while this mixture reaches 1.93 : 1.
 *
 * The two colours are handed in rather than read off a role here, because the
 * ground differs by where the line lies: the list draws on `Base`, the reading
 * pane on `Window`.
 */
QColor frameContrastMix(const QColor &ground, const QColor &text);
}
