#pragma once

#include <QObject>

class Store;

/**
 * The `org.denkzettel.Daemon` interface on the session bus (SPEC 2.3).
 *
 * The remaining methods arrive with the stories that implement them. Method
 * names follow the D-Bus interface, not the C++ naming style.
 */
class DaemonService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.denkzettel.Daemon")

public:
    /** `store` outlives the service and is not owned by it. */
    explicit DaemonService(Store *store, QObject *parent = nullptr);

    /**
     * Exports the interface at `/Daemon`. KDBusService owns the service name
     * and the object path derived from it, hence the separate path.
     */
    bool registerOnSessionBus();

public Q_SLOTS:
    Q_SCRIPTABLE void ShowCapture();

    /**
     * Stores `text` as a new note and returns its id, 0 on failure — D-Bus has
     * no optional, and a caller can tell an id from a failure that way. Blank
     * text is no note, as in the capture window.
     */
    Q_SCRIPTABLE qlonglong AddNote(const QString &text);

    Q_SCRIPTABLE void Quit();

Q_SIGNALS:
    void captureRequested();
    void quitRequested();

private:
    Store *m_store;
};
