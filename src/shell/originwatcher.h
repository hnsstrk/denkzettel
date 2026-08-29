#pragma once

#include <QObject>
#include <QString>

/**
 * The context stamp of SPEC 5.1 and 13: what application the user was in when
 * the capture window came up (issue #47).
 *
 * **The switch stands before the determination, not beside it.** With
 * „[Capture] StoreOrigin" off nothing is loaded into KWin, so nothing is
 * determined and there is nothing to throw away afterwards — the acceptance
 * criterion says „nothing is determined and nothing is stored", and those are
 * two statements. Switching the setting takes hold at once, without a restart
 * of the daemon; what says so is isScriptLoaded() on KWin's side.
 *
 * The road is a KWin script loaded at runtime over the session bus, and it is
 * the only one there is under Plasma/Wayland — the spike of issue #50 measured
 * the other two and both fail. The script reports to the `Report` slot below,
 * which is why this class is a D-Bus object of its own at `/Origin`.
 *
 * **Three things about that road cost a run to find out and are not to be
 * rediscovered:**
 *
 * - `loadScript` hands back an id for a file KWin never runs, so the value
 *   proves nothing. `isScriptLoaded` is the readback (CLAUDE.md finding 1's
 *   family).
 * - `org.kde.kwin.Scripting` has **no signals**, only methods — introspected
 *   29.08.2026. There is nothing to subscribe to, and a subscription would
 *   report success and stay silent (finding 15).
 * - The script lives in KWin's process, so `kwin --replace` throws it away.
 *   The QDBusServiceWatcher below is what notices and loads it again.
 */
class OriginWatcher : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.hnsstrk.denkzettel.Origin")

public:
    explicit OriginWatcher(QObject *parent = nullptr);
    ~OriginWatcher() override;

    OriginWatcher(const OriginWatcher &) = delete;
    OriginWatcher &operator=(const OriginWatcher &) = delete;

    /**
     * Exports `/Origin` and follows the stored setting for the first time.
     *
     * Separate from the constructor because it talks to two services: it
     * belongs where main() has the bus, not where a member is built.
     */
    void start();

public Q_SLOTS:
    /**
     * What the KWin script calls, once per capture: the caption and the
     * application id of the window that was active before ours.
     *
     * Both strings empty means „there was nothing before us" — the script says
     * so rather than sending something plausible.
     *
     * **It asks the setting itself.** With the switch off the call changes
     * nothing and says nothing: barring the source keeps KWin from reporting,
     * and this keeps anybody else on the session bus from reporting in its
     * place.
     *
     * The name is the D-Bus method's, not the C++ naming style's, as in
     * DaemonService.
     */
    Q_SCRIPTABLE void Report(const QString &caption, const QString &appId);

    /**
     * Reads the setting again and loads or unloads the script accordingly.
     *
     * Connected to Settings::configChanged in main.cpp: the dialog's Apply has
     * to reach the running daemon, or the switch would look like a setting
     * that does nothing until the next login.
     */
    void reloadSettings();

Q_SIGNALS:
    /**
     * The value the next note is to carry, and the two windows that write
     * notes hold it.
     *
     * Also emitted with two empty strings when the setting goes off, and that
     * half is not cosmetic: without it a note saved after the switch was
     * turned off would still carry the title of the last capture.
     */
    void originChanged(const QString &caption, const QString &appId);

private:
    /** Loads the script into KWin and reads back whether it is really there. */
    void loadScript();
    void unloadScript();

    QString m_caption;
    QString m_appId;

    /**
     * Whether **this** object put the script into KWin.
     *
     * Only the destructor asks it, and only to stay out of a KWin it never
     * wrote to: a process ending must not take a script out of the compositor
     * that somebody else put there. reloadSettings() unloads without asking,
     * and that is deliberate — a daemon that was killed leaves its script
     * behind, and the next start with the switch off is the one chance to
     * sweep it.
     */
    bool m_loaded = false;
};
