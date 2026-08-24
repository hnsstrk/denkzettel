#pragma once

#include <QFont>

class QObject;

namespace platform
{

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
 */
void followSystemFonts(QObject *owner);

}
