#pragma once

#include <QLatin1StringView>
#include <QString>
#include <QStringList>

/**
 * The optional programs of SPEC 2.5 and 15, and whether this machine can run
 * them (issue #17).
 *
 * Optional means the application stays usable without them and only the
 * functions that need them fall away. So the question asked here is never
 * "which package is installed" — nothing in this program knows about packages,
 * and which one carries a program is a matter for the distribution — but "can
 * the program that is configured be started here".
 */
namespace tools
{
/**
 * Taskwarrior, the second transfer of SPEC 8.
 *
 * A bare name and no path, unlike the two programs of the transcription: those
 * are settings of SPEC 12 and carry their defaults in `whisper::`, while
 * nothing has ever offered a field for this one. Found along PATH, which is
 * the road SPEC 8.2 takes when it runs `task add`.
 */
inline constexpr QLatin1StringView TaskProgram("task");

/**
 * Whether `program` can be started here — a path as it stands, a bare name
 * looked up along PATH.
 *
 * Never `QFileInfo::exists()`, and that is the whole reason this is a function
 * rather than a line at each call site: a file that lies there without an
 * execute bit is the case exists() waves through, and it comes back at the
 * moment the user wanted the function as "could not be started" (SPEC 12).
 * Which of the two roads answers, and why a relative path may not go to PATH,
 * stands at the definition.
 */
bool isRunnable(const QString &program);

/**
 * The names of those `programs` that cannot be started, in the order given.
 *
 * The **name**, not the path and not a package: `/usr/bin/ffmpeg` comes back as
 * `ffmpeg`. That is what a user on another distribution can go looking for, and
 * it is all the tray tooltip has room for (UX decision of 29.08.2026).
 *
 * The name is taken off the setting rather than written down beside it, and
 * for a setting that points somewhere of its own that is the point: whoever
 * put `/opt/tools/ffmpeg-7` in `denkzettelrc` is told about `ffmpeg-7`, the
 * file they named. "ffmpeg" would send them looking at a program they never
 * configured. Which function falls away with it is said on the settings page,
 * where the sentence has room for it.
 */
QStringList missing(const QStringList &programs);
}
