#include "shell/daemonservice.h"
#include "shell/shortcutconflict.h"
#include "shell/shortcutregistration.h"
#include "store/store.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

/**
 * Unit tests of the D-Bus entry points, of the shortcut conflict rule and of
 * the reading of what the shortcut daemon answers (SPEC 2.3, 2.4). AddNote is
 * called as a plain method: the bus adds nothing to what the method does, and
 * the test stays independent of a session bus. The D-Bus conversation with
 * kglobalacceld itself needs a running daemon and stays manual (SPEC 16) —
 * what it answers is decided here.
 */
class ShellTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void addsNoteAndReturnsItsId();
    void keepsBlankTextOutOfTheStore();
    void reportsFailedStorageAsZero();
    void asksForTheLibraryWindow();

    void findsNoConflictWithoutOwners();
    void ignoresOurOwnRegistration();
    void reportsForeignOwner();
    void reportsEveryForeignOwnerOfAMultipleAssignment();

    void takesAnyAnswerOfTheDaemonAsRegistered();
    void readsAnEmptyAnswerWithoutDesktopFileAsAMissingInstallation();
    void readsAnEmptyAnswerWithDesktopFileAsADaemonThatKeptNothing();
    void hasAMessageForEveryFailureAndNoneForSuccess();

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    std::unique_ptr<DaemonService> m_service;
};

void ShellTest::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    m_store = std::make_unique<Store>(m_dir->filePath(QStringLiteral("denkzettel.db")));
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    m_service = std::make_unique<DaemonService>(m_store.get());
}

void ShellTest::cleanup()
{
    m_service.reset();
    m_store.reset();
    m_dir.reset();
}

void ShellTest::addsNoteAndReturnsItsId()
{
    const qlonglong id = m_service->AddNote(QStringLiteral("Gedanke aus einem Skript"));
    QVERIFY(id > 0);

    const std::optional<Note> stored = m_store->note(id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->content, QStringLiteral("Gedanke aus einem Skript"));
    QCOMPARE(stored->type, Note::Type::Text);
    QCOMPARE(stored->state, Note::State::New);
    QVERIFY(stored->createdAt.isValid());
}

void ShellTest::keepsBlankTextOutOfTheStore()
{
    QCOMPARE(m_service->AddNote(QStringLiteral("   \n  ")), 0);
    QVERIFY(!m_store->note(1).has_value());
}

void ShellTest::reportsFailedStorageAsZero()
{
    // A store that was never opened has no table to write to.
    Store closed(m_dir->filePath(QStringLiteral("unopened.db")));
    DaemonService service(&closed);

    QCOMPARE(service.AddNote(QStringLiteral("Gedanke")), 0);
}

void ShellTest::asksForTheLibraryWindow()
{
    // SPEC 2.3: ShowLibrary() only passes the request on — whether the window
    // opens or comes to the front is the window's own decision.
    QSignalSpy requested(m_service.get(), &DaemonService::libraryRequested);

    m_service->ShowLibrary();

    QCOMPARE(requested.size(), 1);
}

void ShellTest::findsNoConflictWithoutOwners()
{
    QVERIFY(foreignShortcutOwners({}, QStringLiteral("org.denkzettel.Denkzettel.desktop")).isEmpty());
}

void ShellTest::ignoresOurOwnRegistration()
{
    // From the second start on kglobalacceld reports our own registration back
    // to us; treating that as a conflict would warn at every session start.
    const QList<ShortcutOwner> owners = {
        {QStringLiteral("org.denkzettel.Denkzettel.desktop"), QStringLiteral("Denkzettel")},
    };

    QVERIFY(foreignShortcutOwners(owners, QStringLiteral("org.denkzettel.Denkzettel.desktop")).isEmpty());
}

void ShellTest::reportsForeignOwner()
{
    const QList<ShortcutOwner> owners = {
        {QStringLiteral("kwin"), QStringLiteral("KWin")},
    };

    const QList<ShortcutOwner> conflicts =
        foreignShortcutOwners(owners, QStringLiteral("org.denkzettel.Denkzettel.desktop"));
    QCOMPARE(conflicts.size(), 1);
    QCOMPARE(conflicts.first().component, QStringLiteral("kwin"));
    QCOMPARE(conflicts.first().description, QStringLiteral("KWin"));
}

void ShellTest::reportsEveryForeignOwnerOfAMultipleAssignment()
{
    // SPEC 2.4 asks for multiple assignments explicitly: the T1 finding was a
    // sequence held by another component while ours looked registered.
    const QList<ShortcutOwner> owners = {
        {QStringLiteral("kwin"), QStringLiteral("KWin")},
        {QStringLiteral("org.denkzettel.Denkzettel.desktop"), QStringLiteral("Denkzettel")},
        {QStringLiteral("org.kde.spectacle.desktop"), QStringLiteral("Spectacle")},
    };

    const QList<ShortcutOwner> conflicts =
        foreignShortcutOwners(owners, QStringLiteral("org.denkzettel.Denkzettel.desktop"));
    QCOMPARE(conflicts.size(), 2);
    QCOMPARE(conflicts.at(0).component, QStringLiteral("kwin"));
    QCOMPARE(conflicts.at(1).component, QStringLiteral("org.kde.spectacle.desktop"));
}

void ShellTest::takesAnyAnswerOfTheDaemonAsRegistered()
{
    const QList<QKeySequence> stored = {QKeySequence(Qt::META | Qt::Key_N)};

    QCOMPARE(shortcutRegistration(stored, true), ShortcutRegistration::Reached);

    // Which sequence the daemon holds is none of our business — the user may
    // have changed it in the Plasma settings, and that is a registration that
    // arrived. Nor does our own view of the desktop file overrule the daemon:
    // it has answered, so it knows us.
    const QList<QKeySequence> changed = {QKeySequence(Qt::META | Qt::Key_F10)};
    QCOMPARE(shortcutRegistration(changed, false), ShortcutRegistration::Reached);
}

void ShellTest::readsAnEmptyAnswerWithoutDesktopFileAsAMissingInstallation()
{
    // The customer finding of 01.08.2026: kglobalacceld resolves the component
    // through the desktop file and creates none without it, so the answer stays
    // empty however often we register.
    QCOMPARE(shortcutRegistration({}, false), ShortcutRegistration::ApplicationNotInstalled);
}

void ShellTest::readsAnEmptyAnswerWithDesktopFileAsADaemonThatKeptNothing()
{
    // Installed and still nothing stored — then the daemon is the suspect, and
    // a message blaming the installation would send the user the wrong way.
    QCOMPARE(shortcutRegistration({}, true), ShortcutRegistration::DaemonKeptNothing);
}

void ShellTest::hasAMessageForEveryFailureAndNoneForSuccess()
{
    // A failure without a message is exactly the silent failure SPEC 2.4
    // forbids; a message on success would cry wolf at every start.
    QVERIFY(shortcutRegistrationFailure(ShortcutRegistration::Reached).isEmpty());
    QVERIFY(!shortcutRegistrationFailure(ShortcutRegistration::ApplicationNotInstalled).isEmpty());
    QVERIFY(!shortcutRegistrationFailure(ShortcutRegistration::DaemonKeptNothing).isEmpty());

    // The two failures are told apart for the user as well, not only in code.
    QVERIFY(shortcutRegistrationFailure(ShortcutRegistration::ApplicationNotInstalled)
            != shortcutRegistrationFailure(ShortcutRegistration::DaemonKeptNothing));
}

QTEST_GUILESS_MAIN(ShellTest)

#include "shelltest.moc"
