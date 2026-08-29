#include "shell/originwatcher.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QFile>
#include <QGuiApplication>
#include <QStandardPaths>

namespace
{
/** The name KWin files the script under; every call below names it. */
QString scriptName()
{
    return QStringLiteral("denkzettel-origin");
}

QString kwinService()
{
    return QStringLiteral("org.kde.KWin");
}

QDBusInterface scripting()
{
    return {kwinService(), QStringLiteral("/Scripting"), QStringLiteral("org.kde.kwin.Scripting"),
            QDBusConnection::sessionBus()};
}

/** True while the user has said the origin may be determined (SPEC 13). */
bool settingIsOn()
{
    // Read out of denkzettelrc rather than off Settings::self(): the skeleton
    // lives in denkzettelsettings, which links this library and not the other
    // way round. The group and the key are the ones settings.cpp writes, and
    // the comment there says so too.
    return KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Capture"))
        .readEntry("StoreOrigin", false);
}

/** Where the installed script lies, empty if it is not installed. */
QString scriptPath()
{
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                  QStringLiteral("denkzettel/origin.js"));
}
}

OriginWatcher::OriginWatcher(QObject *parent)
    : QObject(parent)
{
}

OriginWatcher::~OriginWatcher()
{
    // The script outlives this process otherwise, and its callDBus would then
    // hit an unowned bus name — which the bus answers by *starting* the daemon
    // again from its service file (CLAUDE.md finding 37 names that road for the
    // other direction). So the last thing this object does is take it out —
    // but only what it put there itself, or a check that builds one of these
    // would reach into the compositor of whoever runs it.
    if (m_loaded) {
        unloadScript();
    }
}

void OriginWatcher::start()
{
    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/Origin"),
                                                      this,
                                                      QDBusConnection::ExportScriptableSlots)) {
        qWarning("Exporting io.github.hnsstrk.denkzettel.Origin failed; the origin of a note stays empty.");
        return;
    }

    // `kwin --replace` and a KWin crash both throw the script away, and there
    // is nothing to subscribe to inside KWin that would say so — the scripting
    // interface has no signals at all. What does say so is the bus name coming
    // back: a fresh KWin registers org.kde.KWin again, and that is the moment
    // to load the script a second time.
    auto *kwin = new QDBusServiceWatcher(kwinService(),
                                         QDBusConnection::sessionBus(),
                                         QDBusServiceWatcher::WatchForRegistration,
                                         this);
    connect(kwin, &QDBusServiceWatcher::serviceRegistered, this, [this] {
        if (settingIsOn()) {
            loadScript();
        }
    });

    reloadSettings();
}

void OriginWatcher::Report(const QString &caption, const QString &appId)
{
    // **The sink asks the setting too, and not only the source.** Barring the
    // source is what makes the assurance of SPEC 13 true — with the switch off
    // KWin never gets the script and nothing is determined — but this method
    // sits on the session bus and anybody there can call it. Without this line
    // the switch is a lock on one door of two, and a call from outside would be
    // stored and handed to the next note; on the receiving side the two states
    // would be indistinguishable (finding of the review, 29.08.2026).
    //
    // Read out of the configuration on every call rather than kept in a
    // member: what is asked here is the state at this moment, and a member
    // would be a second place for the same fact.
    if (!settingIsOn()) {
        return;
    }

    m_caption = caption;
    m_appId = appId;
    Q_EMIT originChanged(m_caption, m_appId);
}

void OriginWatcher::reloadSettings()
{
    if (settingIsOn()) {
        loadScript();
        return;
    }
    unloadScript();
}

void OriginWatcher::loadScript()
{
    const QString path = scriptPath();
    if (path.isEmpty()) {
        // A run out of the build directory is the ordinary case here: the file
        // is found through XDG_DATA_DIRS, so it has to be installed. Said out
        // loud, because the alternative is a feature that is switched on and
        // silently does nothing.
        qWarning("denkzettel/origin.js is not installed; the origin of a note stays empty.");
        return;
    }

    // The one thing about this script that breaks without a word: it filters on
    // the application id, and a renamed application leaves the filter matching
    // nothing while everything else reports success — loaded, isScriptLoaded
    // true, and not one report for the rest of the session. Issue #112 did
    // exactly that to the spike of #50, and nobody noticed until #47 was
    // estimated. So the file is held against the name this process registers
    // under, which is the same one Wayland hands KWin as the app_id.
    //
    // **The whole assignment and not the bare name**, measured 29.08.2026: the
    // id stands in the script's comment and in the bus name it calls back on as
    // well, so a search for the name alone passes over a script whose filter
    // carries the *old* id — the control run stood green on exactly the
    // breakage this check exists for.
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString source = QString::fromUtf8(file.readAll());
        const QString filter =
            QStringLiteral(R"(var OWN_CLASS = "%1";)").arg(QGuiApplication::desktopFileName());
        if (!source.contains(filter)) {
            qWarning("%s does not filter on %s; the origin of a note stays empty.",
                     qUtf8Printable(path),
                     qUtf8Printable(QGuiApplication::desktopFileName()));
            return;
        }
    }

    QDBusInterface kwin = scripting();
    if (const QDBusReply<bool> loaded = kwin.call(QStringLiteral("isScriptLoaded"), scriptName());
        loaded.isValid() && loaded.value()) {
        m_loaded = true;
        return;
    }

    // The return value of loadScript() is an id, and KWin hands one back for a
    // file it never runs as well — it says nothing (CLAUDE.md finding 1's
    // family). start() is what makes a loaded script run, and isScriptLoaded is
    // what answers afterwards.
    kwin.call(QStringLiteral("loadScript"), path, scriptName());
    kwin.call(QStringLiteral("start"));

    const QDBusReply<bool> running = kwin.call(QStringLiteral("isScriptLoaded"), scriptName());
    if (!running.isValid() || !running.value()) {
        qWarning("KWin did not take denkzettel/origin.js; the origin of a note stays empty.");
        return;
    }
    m_loaded = true;
}

void OriginWatcher::unloadScript()
{
    QDBusInterface kwin = scripting();
    kwin.call(QStringLiteral("unloadScript"), scriptName());

    // Read back, the way loadScript() does. The whole assurance hangs on this
    // call having taken, and the return value cannot say: `unloadScript`
    // answers `false` for a script that was never loaded as well. Only
    // isScriptLoaded answers.
    //
    // An **invalid** reply is the one case where nothing is wrong: it means
    // KWin is not on the bus, and the script lives in KWin's process — no KWin,
    // no script. Measured 29.08.2026: without KWin the call comes back
    // `ServiceUnknown`, and taking that for a failure would put a line in the
    // journal at every logout.
    const QDBusReply<bool> still = kwin.call(QStringLiteral("isScriptLoaded"), scriptName());
    if (still.isValid() && still.value()) {
        qWarning("KWin still runs denkzettel/origin.js; window titles go on being reported.");
        return;
    }
    m_loaded = false;

    // Whatever the last capture left standing goes with the script. Without
    // this a note written after the switch was turned off would carry the title
    // of a window from before it — the invisible collection this switch exists
    // against.
    if (!m_caption.isEmpty() || !m_appId.isEmpty()) {
        m_caption.clear();
        m_appId.clear();
        Q_EMIT originChanged(m_caption, m_appId);
    }
}
