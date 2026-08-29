#pragma once

#include <QObject>

class Store;

/**
 * The `io.github.hnsstrk.denkzettel.Daemon` interface on the session bus
 * (SPEC 2.3).
 *
 * The remaining methods arrive with the stories that implement them. Method
 * names follow the D-Bus interface, not the C++ naming style.
 */
class DaemonService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.hnsstrk.denkzettel.Daemon")

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
     * Shows the recording window; the recording starts with it (SPEC 2.3, 4).
     *
     * The other road to the same window is the desktop action of SPEC 2.4:
     * with Denkzettel installed, a key press does not signal the running
     * process but starts `denkzetteld` again, and the single-instance branch
     * turns that into this call.
     */
    Q_SCRIPTABLE void ShowRecorder();

    /**
     * Stores `text` as a new note and returns its id, 0 on failure — D-Bus has
     * no optional, and a caller can tell an id from a failure that way. Blank
     * text is no note, as in the capture window.
     */
    Q_SCRIPTABLE qlonglong AddNote(const QString &text);

    Q_SCRIPTABLE void ShowLibrary();

    /**
     * Kicks off an analysis run (SPEC 2.3, 7.2) — the on-demand road, beside
     * the tray entry of the same name.
     *
     * It returns at once and says nothing about what the run found: a run takes
     * one model call per note and the caller of a D-Bus method waits for its
     * reply. What the run did stands in the library and in the log.
     */
    Q_SCRIPTABLE void AnalyzeNow();

    Q_SCRIPTABLE void Quit();

Q_SIGNALS:
    void captureRequested();
    void recorderRequested();
    void libraryRequested();
    void analysisRequested();
    void quitRequested();

private:
    Store *m_store;
};
