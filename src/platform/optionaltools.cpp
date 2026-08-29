#include "platform/optionaltools.h"

#include <QFileInfo>
#include <QStandardPaths>

bool tools::isRunnable(const QString &program)
{
    // A bare name is the only thing PATH is asked about; anything carrying a
    // separator is asked of the file system, because that is what QProcess
    // does with it. `QStandardPaths::findExecutable()` takes the shortcut only
    // for an **absolute** path and searches PATH for `./ffmpeg`, which no PATH
    // holds — measured 2026-08-29: empty for a file that lies there and is
    // executable and that QProcess starts without a murmur. A wrong "not
    // available" is worse than none: it sends the user looking for a program
    // that is there.
    //
    // The two branches answer the same three cases alike — the path as it
    // stands, the search along PATH, and a file without an execute bit — and
    // the two nobody asked for as well: `isFile()` is false for a directory
    // and false for a symbolic link that points nowhere.
    if (program.contains(QLatin1Char('/'))) {
        const QFileInfo file(program);
        return file.isFile() && file.isExecutable();
    }
    return !QStandardPaths::findExecutable(program).isEmpty();
}

QStringList tools::missing(const QStringList &programs)
{
    QStringList names;
    for (const QString &program : programs) {
        // A setting that names no program at all is left out rather than
        // reported under an empty name: there is nothing to go looking for,
        // and the page the setting belongs to is what says it is unset
        // (issue #27, "No program is set for the transcription").
        const QString name = QFileInfo(program).fileName();
        if (!name.isEmpty() && !isRunnable(program)) {
            names.append(name);
        }
    }
    return names;
}
