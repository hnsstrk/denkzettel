#include "shell/daemonservice.h"

#include "store/store.h"

#include <QDBusConnection>
#include <QDateTime>

DaemonService::DaemonService(Store *store, QObject *parent)
    : QObject(parent)
    , m_store(store)
{
}

bool DaemonService::registerOnSessionBus()
{
    return QDBusConnection::sessionBus().registerObject(QStringLiteral("/Daemon"),
                                                        this,
                                                        QDBusConnection::ExportScriptableSlots);
}

void DaemonService::ShowCapture()
{
    Q_EMIT captureRequested();
}

void DaemonService::ShowRecorder()
{
    Q_EMIT recorderRequested();
}

qlonglong DaemonService::AddNote(const QString &text)
{
    const QString content = text.trimmed();
    if (content.isEmpty()) {
        return 0;
    }

    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.type = Note::Type::Text;
    note.content = content;
    // **No origin, and that is not an omission** (issue #47). The origin is
    // what the KWin script reports when a window of ours takes the focus, and
    // this road opens no window at all: what OriginWatcher holds here is the
    // title of whatever stood before the *last* capture, minutes or days ago.
    // Written onto this note it would be a window title from another moment —
    // "no history: only the state at capture time" is exactly what it would
    // break. The two windows that do take the activation carry it (main.cpp).

    const std::optional<qint64> id = m_store->addNote(note);
    if (!id) {
        qWarning("AddNote failed: %s", qPrintable(m_store->lastError()));
        return 0;
    }

    return *id;
}

void DaemonService::ShowLibrary()
{
    Q_EMIT libraryRequested();
}

void DaemonService::AnalyzeNow()
{
    Q_EMIT analysisRequested();
}

void DaemonService::Quit()
{
    Q_EMIT quitRequested();
}
