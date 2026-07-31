#include "shell/daemonservice.h"

#include <QDBusConnection>

DaemonService::DaemonService(QObject *parent)
    : QObject(parent)
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
