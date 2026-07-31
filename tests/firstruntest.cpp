#include "shell/firstrun.h"
#include "store/store.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

/**
 * Tests the first start (SPEC 2.5) against a fresh XDG_CONFIG_HOME and
 * XDG_DATA_HOME, so the paths under test are the ones a new user gets.
 *
 * The other two acceptance criteria of the story — the installed autostart
 * entry starting the daemon with the session, and switching it off in the
 * Plasma autostart module — need a real installation and a session change;
 * they belong to the manual M1 checklist (SPEC 16, sprint-02 3.3). What the
 * installation places where is covered by installtest.
 */
class FirstRunTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void createsDataDirectoryDatabaseAndConfiguration();
    void reportsTheFirstStartOnlyOnce();

private:
    QTemporaryDir m_home;
};

void FirstRunTest::initTestCase()
{
    QVERIFY(m_home.isValid());

    // A first start can only be observed on paths nobody has written to yet.
    // Neither directory is created here: whether the first start creates them
    // is precisely what is under test.
    qputenv("XDG_CONFIG_HOME", QFile::encodeName(m_home.filePath(QStringLiteral("config"))));
    qputenv("XDG_DATA_HOME", QFile::encodeName(m_home.filePath(QStringLiteral("data"))));

    // Database path and configuration file name both derive from the
    // application name, which the test main sets to the test binary's name.
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
}

void FirstRunTest::createsDataDirectoryDatabaseAndConfiguration()
{
    const QString dataDirectory = m_home.filePath(QStringLiteral("data/denkzettel"));
    const QString databasePath = dataDirectory + QStringLiteral("/denkzettel.db");
    const QString configurationPath = m_home.filePath(QStringLiteral("config/denkzettelrc"));

    QCOMPARE(Store::defaultDatabasePath(), databasePath);
    QVERIFY(!QFileInfo::exists(dataDirectory));

    Store store(Store::defaultDatabasePath());
    QVERIFY2(store.open(), qPrintable(store.lastError()));

    QVERIFY2(QFileInfo(dataDirectory).isDir(), qPrintable(dataDirectory));
    QVERIFY2(QFile::exists(databasePath), qPrintable(databasePath));
    // Version 1 is the current schema (SPEC 5.1); storetest holds the detail.
    QCOMPARE(store.schemaVersion(), 1);

    // The file name follows from the application name, so the daemon has to be
    // back to "denkzettel" before this call — otherwise the configuration lands
    // in Daemonrc, the trap S4 documented. That order is kept by main.cpp and
    // cannot be seen from here.
    KConfigGroup general(KSharedConfig::openConfig(), QStringLiteral("General"));
    QVERIFY(runFirstStart(general));

    QVERIFY2(QFile::exists(configurationPath), qPrintable(configurationPath));
}

void FirstRunTest::reportsTheFirstStartOnlyOnce()
{
    // Its own configuration file: the test above already consumed the marker in
    // denkzettelrc, and the rule under test does not depend on the file name.
    KConfigGroup general(KSharedConfig::openConfig(m_home.filePath(QStringLiteral("laterstartsrc")),
                                                  KConfig::SimpleConfig),
                         QStringLiteral("General"));

    QVERIFY(runFirstStart(general));

    // SPEC 2.4 hangs the shortcut conflict warning on the first start: a marker
    // that is taken twice turns that warning into a recurring one.
    QVERIFY(!runFirstStart(general));
    QVERIFY(general.readEntry("FirstRunDone", false));
}

QTEST_GUILESS_MAIN(FirstRunTest)

#include "firstruntest.moc"
