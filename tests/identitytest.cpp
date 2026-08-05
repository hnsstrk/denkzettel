#include "shell/appidentity.h"
#include "store/store.h"

#include <QApplication>
#include <QTest>

// The three names the session hangs the daemon on survive the KAboutData
// registration (issue #61, SPEC 2.3 and 2.4).
//
// KAboutData::setApplicationData() writes applicationName, organizationDomain
// and desktopFileName along with the version, and the defaults it brings along
// are kde.org and org.kde.denkzettel. With them the daemon registers as
// org.kde.Daemon, the shortcut component is called org.kde.denkzettel.desktop
// and the notes move to another directory. None of the three reaches a return
// value, and before this test none of them reached a test either: the bus name
// showed up in the session and nowhere else.
//
// What this test cannot show is the name actually registered on a bus — that is
// commandlinetest, which starts the daemon in a session of its own.
class IdentityTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void keepsTheDomainTheBusNameIsBuiltFrom();
    void keepsTheDesktopFileTheShortcutComponentIsNamedAfter();
    void keepsTheNameTheConfigurationAndTheNotesHangOn();

private:
    QString m_databasePathBeforeRegistration;
};

void IdentityTest::initTestCase()
{
    // main() below has set the name src/main.cpp starts with, so this is the
    // path the notes lie under before anything registers. Comparing it against
    // itself after the registration is what makes the second half of the trap
    // visible: KAboutData decides the application name, and the data directory
    // derives from it.
    m_databasePathBeforeRegistration = Store::defaultDatabasePath();
    QVERIFY2(m_databasePathBeforeRegistration.contains(QLatin1String("/denkzettel/")),
             qPrintable(QStringLiteral("Der Ausgangspfad steht nicht unter denkzettel/: %1")
                            .arg(m_databasePathBeforeRegistration)));

    registerApplicationIdentity();
}

void IdentityTest::keepsTheDomainTheBusNameIsBuiltFrom()
{
    // KDBusService reverses this domain and appends the application name; SPEC
    // 2.3 fixes the result to org.denkzettel.Daemon.
    QCOMPARE(QCoreApplication::organizationDomain(), QStringLiteral("denkzettel.org"));
}

void IdentityTest::keepsTheDesktopFileTheShortcutComponentIsNamedAfter()
{
    // src/shell/globalshortcuts.cpp reads exactly this property and appends
    // ".desktop" to it (SPEC 2.4). Wayland uses it as the application id.
    QCOMPARE(QGuiApplication::desktopFileName(), QStringLiteral("org.denkzettel.Denkzettel"));
}

void IdentityTest::keepsTheNameTheConfigurationAndTheNotesHangOn()
{
    // denkzettelrc, not Daemonrc — and the notes stay where they were.
    QCOMPARE(QCoreApplication::applicationName(), QStringLiteral("denkzettel"));
    QCOMPARE(Store::defaultDatabasePath(), m_databasePathBeforeRegistration);
}

int main(int argc, char *argv[])
{
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);

    // The name src/main.cpp starts with, set before the registration runs, so
    // that the assertions above measure what the registration changed and not
    // what the test binary happens to be called.
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    IdentityTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "identitytest.moc"
