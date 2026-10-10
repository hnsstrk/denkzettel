#include "platform/logfile.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>

#include <cstdio>

namespace
{

/** SPEC 14: at 1 MB the file is renamed and one predecessor is kept. */
constexpr qint64 RotateAt = 1024LL * 1024;

/**
 * The handler that was in place when startLogFile() ran — Qt's own where
 * nothing had installed one, and never a null pointer (see handleMessage()).
 * It is called for every line, so nothing this file takes goes missing from
 * the channel that carried it before (SPEC 14).
 */
QtMessageHandler g_previousHandler = nullptr;

/**
 * Whether the first failed write has been reported. Without it a directory that
 * has gone would produce one complaint per line, and the complaint is on stderr
 * — the very channel the user would then be reading.
 */
bool g_complained = false;

QMutex &logMutex()
{
    static QMutex mutex;
    return mutex;
}

/**
 * The three letters of SPEC 14. `qFatal` shares the C of a critical: it is the
 * last line the program writes, and a fourth letter for it would say nothing
 * the line beside it does not.
 */
char severity(QtMsgType type)
{
    switch (type) {
    case QtInfoMsg:
        return 'I';
    case QtWarningMsg:
        return 'W';
    case QtCriticalMsg:
    case QtFatalMsg:
        return 'C';
    case QtDebugMsg:
        break;
    }
    return 'D';
}

/** Never through qWarning: that would call the handler below again. */
void complainOnce(const char *what, const QString &path)
{
    if (g_complained) {
        return;
    }
    g_complained = true;
    std::fprintf(stderr, "denkzettel: %s: %s\n", what, qUtf8Printable(path));
}

void appendToLogFile(QtMsgType type, const QString &message)
{
    const QString path = platform::logFilePath();
    const QMutexLocker locker(&logMutex());

    // Before the write and out of the file's own size: the rename has to happen
    // while the file is closed, so opening it comes after.
    if (QFileInfo(path).size() >= RotateAt) {
        const QString predecessor = path + QStringLiteral(".1");
        QFile::remove(predecessor);
        if (!QFile::rename(path, predecessor)) {
            complainOnce("could not rotate the log file", predecessor);
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        complainOnce("could not open the log file", path);
        return;
    }

    const QString line = QStringLiteral("%1 %2 %3\n")
                             .arg(QDateTime::currentDateTime().toString(Qt::ISODate),
                                  QString(QChar::fromLatin1(severity(type))),
                                  message);
    if (file.write(line.toUtf8()) < 0) {
        complainOnce("could not write to the log file", path);
    }
}

void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    // First, and unconditionally: whatever this function does afterwards, the
    // line has already gone where it went before (SPEC 14) — for the daemon
    // that is Qt's own road, journald included, and the journal carries the
    // line unchanged (CLAUDE.md, finding 25).
    //
    // **There is no second road for a missing pointer, because there is no
    // missing pointer.** `qInstallMessageHandler()` never answers with
    // `nullptr`: where nothing is installed it hands back Qt's own default
    // handler. Measured 10.10.2026 against Qt 6.12.0, three states of one
    // probe — before any application object, after a bare `QCoreApplication`,
    // and after installing `nullptr`, which is what resets Qt to its default
    // — the same non-null address all three times. An earlier wording here
    // rebuilt the default handler out of `qFormatLogMessage()` for a case it
    // claimed to have measured; that branch could never be reached.
    g_previousHandler(type, context, message);
    if (type != QtDebugMsg) {
        appendToLogFile(type, message);
    }
}

}

QString platform::logFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/denkzettel.log");
}

void platform::startLogFile()
{
    // The data directory is the store's to create, and this runs before it —
    // the first line worth keeping is the one saying the database would not
    // open.
    QDir().mkpath(QFileInfo(logFilePath()).absolutePath());

    // Installed once, and the pointer is what says so — it starts out null and
    // what `qInstallMessageHandler()` answers never is (see handleMessage()).
    // A flag of its own stood here while that was believed to be the other way
    // round. A second call would hand back this very handler, and
    // `g_previousHandler` pointing at handleMessage() is a call that never
    // returns.
    if (g_previousHandler == nullptr) {
        g_previousHandler = qInstallMessageHandler(handleMessage);
    }
}
