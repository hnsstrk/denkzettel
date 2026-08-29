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
