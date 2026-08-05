#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>

// What denkzetteld does with its arguments (issue #61), measured on the built
// binary rather than on a copy of its code — the version has to travel from
// CMake through the compiler into a line on stdout, and no in-process assertion
// covers that road.
//
// ctest starts this test through dbus-run-session, so the daemon of the last
// case gets a session bus of its own and never touches the one the customer is
// logged into. The three cases before it point DBUS_SESSION_BUS_ADDRESS at a
// socket that does not exist, which is what makes them proof rather than
// coincidence: KDBusService ends the process with 1 when it cannot reach a bus
// (measured 05.08.2026), so a version line can only appear if the option is
// answered before that line runs.
class CommandLineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void writesTheVersionOfTheBuildAndEndsWithoutASessionBus();
    void refusesASwitchNobodyDeclared();
    void refusesTheSwitchThatWouldRenameTheApplication();
    void startsTheDaemonWithoutAnArgument();

private:
    QProcessEnvironment isolatedEnvironment(const QTemporaryDir &home, bool withSessionBus) const;
    void expectRefusal(const QStringList &arguments);
};

QProcessEnvironment CommandLineTest::isolatedEnvironment(const QTemporaryDir &home, bool withSessionBus) const
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    // Configuration, notes and cache of every run below stay in a directory of
    // their own; nothing here writes into the customer's.
    environment.insert(QStringLiteral("XDG_DATA_HOME"), home.filePath(QStringLiteral("data")));
    environment.insert(QStringLiteral("XDG_CONFIG_HOME"), home.filePath(QStringLiteral("config")));
    environment.insert(QStringLiteral("XDG_CACHE_HOME"), home.filePath(QStringLiteral("cache")));
    environment.insert(QStringLiteral("XDG_STATE_HOME"), home.filePath(QStringLiteral("state")));
    environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));

    if (!withSessionBus) {
        // Not unset but pointed at nothing: with the variable removed, D-Bus may
        // autolaunch a bus of its own, and a run that finds one measures the
        // opposite of what it claims to.
        environment.insert(QStringLiteral("DBUS_SESSION_BUS_ADDRESS"),
                           QStringLiteral("unix:path=") + home.filePath(QStringLiteral("kein-bus")));
    }

    return environment;
}

void CommandLineTest::writesTheVersionOfTheBuildAndEndsWithoutASessionBus()
{
    QTemporaryDir home;
    QVERIFY(home.isValid());

    QProcess daemon;
    daemon.setProcessEnvironment(isolatedEnvironment(home, false));
    daemon.start(QStringLiteral(DENKZETTEL_DAEMON_BINARY), {QStringLiteral("--version")});

    QVERIFY(daemon.waitForFinished(10000));
    QCOMPARE(daemon.exitStatus(), QProcess::NormalExit);
    QCOMPARE(daemon.exitCode(), 0);

    // The name, not the bus name: QCommandLineParser composes the line from
    // applicationName and applicationVersion, and src/main.cpp turns the name
    // into "Daemon" for the registration further down. "Daemon 0.1.0" here
    // would mean the option is answered inside that window.
    QCOMPARE(QString::fromLocal8Bit(daemon.readAllStandardOutput()).trimmed(),
             QStringLiteral("denkzettel " DENKZETTEL_VERSION));
}

// A refused switch has to be told apart from a daemon that merely died, and the
// obvious setup cannot do it: without a reachable bus KDBusService ends the
// process with 1 in silence, so an *accepted* switch would leave the same exit
// code as a refused one and the assertion would hold whatever the code did.
//
// Hence the session bus of the test: refused, the process ends by itself within
// a moment and with a code of its own; accepted, it becomes the daemon and does
// not end at all. The two outcomes cannot be confused.
void CommandLineTest::expectRefusal(const QStringList &arguments)
{
    QTemporaryDir home;
    QVERIFY(home.isValid());

    QProcess daemon;
    daemon.setProcessEnvironment(isolatedEnvironment(home, true));
    daemon.start(QStringLiteral(DENKZETTEL_DAEMON_BINARY), arguments);
    QVERIFY(daemon.waitForStarted(10000));

    if (!daemon.waitForFinished(10000)) {
        daemon.kill();
        daemon.waitForFinished(10000);
        QFAIL(qPrintable(QStringLiteral("%1 wurde angenommen: der Dienst lief damit weiter.")
                             .arg(arguments.join(QLatin1Char(' ')))));
    }

    QCOMPARE(daemon.exitStatus(), QProcess::NormalExit);
    QVERIFY2(daemon.exitCode() != 0,
             qPrintable(QStringLiteral("%1 endete mit 0 statt mit einer Zurückweisung.")
                            .arg(arguments.join(QLatin1Char(' ')))));
}

void CommandLineTest::refusesASwitchNobodyDeclared()
{
    expectRefusal({QStringLiteral("--kennt-keiner")});
}

void CommandLineTest::refusesTheSwitchThatWouldRenameTheApplication()
{
    // KAboutData::setupCommandLine() would declare --desktopfile beside the
    // help and version options and hand the application id of SPEC 2.4 to
    // whoever starts the process. It is not declared, so it falls under the
    // case above.
    expectRefusal({QStringLiteral("--desktopfile"), QStringLiteral("org.example.Fremd")});
}

void CommandLineTest::startsTheDaemonWithoutAnArgument()
{
    QVERIFY2(QDBusConnection::sessionBus().isConnected(),
             "Kein Sitzungsbus — der Test gehört unter dbus-run-session gestartet.");
    QVERIFY2(!QDBusConnection::sessionBus().interface()->isServiceRegistered(
                 QStringLiteral("org.denkzettel.Daemon")),
             "Auf diesem Bus liegt der Name schon; der Test misst dann einen fremden Dienst.");

    QTemporaryDir home;
    QVERIFY(home.isValid());

    // The way the autostart entry takes: both Exec= lines of the desktop file
    // start denkzetteld without an argument. Refusing unknown switches must not
    // touch it.
    QProcess daemon;
    daemon.setProcessEnvironment(isolatedEnvironment(home, true));
    daemon.start(QStringLiteral(DENKZETTEL_DAEMON_BINARY), {});
    QVERIFY(daemon.waitForStarted(10000));

    bool registered = false;
    for (int attempt = 0; attempt < 100 && !registered; ++attempt) {
        QTest::qWait(100);
        registered = QDBusConnection::sessionBus().interface()->isServiceRegistered(
            QStringLiteral("org.denkzettel.Daemon"));
    }

    // SPEC 2.3 fixes the name. It is built from the organisation domain, which
    // KAboutData writes along with the version — the reason this assertion
    // exists at all (issue #61).
    QVERIFY2(registered,
             qPrintable(QStringLiteral("org.denkzettel.Daemon meldete sich nicht an. Ausgabe: %1")
                            .arg(QString::fromLocal8Bit(daemon.readAllStandardError()))));
    QCOMPARE(daemon.state(), QProcess::Running);

    daemon.terminate();
    if (!daemon.waitForFinished(10000)) {
        daemon.kill();
        daemon.waitForFinished(10000);
    }
}

QTEST_MAIN(CommandLineTest)

#include "commandlinetest.moc"
