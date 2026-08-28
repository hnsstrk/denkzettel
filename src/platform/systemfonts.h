#pragma once

#include <QFont>
#include <QLatin1StringView>

class QObject;

namespace platform
{

/**
 * The property a widget whose font was set by hand carries, so that a font
 * change can find it again (issue #68).
 *
 * A widget that was never given a font of its own follows the application font
 * by itself; one that was does not. The library window marks its small labels
 * with this and collects them again out of `findChildren()`; the audio player
 * builds one of those labels and lives in another file, which is why the name
 * stands here rather than in `librarywindow.cpp`. The capture window solves the
 * same problem with a list of its own (`m_subtleLabels`) and does not read this.
 */
constexpr QLatin1StringView FontSetByHand("denkzettel_smallFont");

/**
 * The general interface font as `kdeglobals` names it **now**.
 *
 * `QFontDatabase::systemFont()` answers out of the platform theme, and that
 * one reads the file once: measured 24.08.2026 with the KDE platform theme
 * loaded, a font changed in `kdeglobals` from 10 pt to 16 pt did not reach a
 * running application at all (issue #68). So the file is read here, and the
 * platform theme's answer stays as the fallback for everything this cannot
 * decide — a missing key, a value that does not parse, a session that is no
 * Plasma session.
 *
 * Cheap to call in a loop **once `followSystemFonts()` has run**, and only
 * then: the answer is held from one change of the file to the next, and that
 * watch is what lets go of it. A process that does not call it reads the file
 * on every call (issue #110).
 */
QFont generalFont();

/** The smallest readable font as `kdeglobals` names it now, same rules. */
QFont smallestReadableFont();

/**
 * Keeps the application font on what `kdeglobals` says, for as long as `owner`
 * lives.
 *
 * The watch hangs on the **file** and not on `KConfigWatcher`, and that is
 * measured, in this project and for this reason: a writer that omits
 * `KConfig::Notify` never reaches `KConfigWatcher`, while `KDirWatch` sees both
 * kinds. The same choice carries the theme switch in the capture window.
 *
 * Widgets that were never given a font of their own follow the application font
 * by themselves. Whoever did set one — the small labels, the note list's
 * delegate — has to ask again, and gets a `QEvent::ApplicationFontChange` for
 * it.
 *
 * This is also what turns the two functions above into cheap ones: what they
 * hold is thrown away here, so nothing may be held before this call
 * (issue #110). **`owner` has to live as long as the process does** — the two
 * connections die with it, and what the two functions hold would then be held
 * for good. Every caller hands over `qApp`.
 */
void followSystemFonts(QObject *owner);

}
