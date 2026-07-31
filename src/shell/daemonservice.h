#pragma once

#include <QObject>

/**
 * The `org.denkzettel.Daemon` interface on the session bus (SPEC 2.3).
 *
 * Only ShowCapture() exists so far — the remaining methods arrive with the
 * stories that implement them. The method name follows the D-Bus interface,
 * not the C++ naming style.
 */
class DaemonService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.denkzettel.Daemon")

public:
    explicit DaemonService(QObject *parent = nullptr);

    /**
     * Exports the interface at `/Daemon`. KDBusService owns the service name
     * and the object path derived from it, hence the separate path.
     */
    bool registerOnSessionBus();

public Q_SLOTS:
    Q_SCRIPTABLE void ShowCapture();

Q_SIGNALS:
    void captureRequested();
};
