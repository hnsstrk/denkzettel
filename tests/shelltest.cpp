#include "shell/daemonservice.h"
#include "shell/shortcutconflict.h"
#include "shell/shortcutregistration.h"
#include "shell/trayicon.h"
#include "store/store.h"

#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QAction>
#include <QFile>
#include <QIcon>
#include <QMenu>
#include <QSet>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

/**
 * Unit tests of the D-Bus entry points, of the shortcut conflict rule, of the
 * reading of what the shortcut daemon answers (SPEC 2.3, 2.4) and of what the
 * tray item announces (SPEC 10). AddNote is called as a plain method: the bus
 * adds nothing to what the method does, and the test stays independent of a
 * session bus. The D-Bus conversation with kglobalacceld itself needs a running
 * daemon and stays manual (SPEC 16) — what it answers is decided here.
 */
class ShellTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void addsNoteAndReturnsItsId();
    void announcesTheNoteItWrites();
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
    void readsAnUndeclaredDesktopActionAsAFailure();
    void readsTheActionsOfADesktopFile();
    void hasAMessageForEveryFailureAndNoneForSuccess();

    void hintsTheShortcutWithoutBindingItASecondTime();
    void asksForTheSettingsDialog();
    void showsAFailedTranscriptionAndTakesItBack();

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    std::unique_ptr<DaemonService> m_service;
};

void ShellTest::initTestCase()
{
    // As in main.cpp: without the domain every i18n() call in tray icon and
    // shortcut messages warns that translation will not work. The source
    // strings the checks below compare stay in place because
    // tests/CMakeLists.txt pins LANGUAGE to the source language, for which no
    // catalogue exists.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // The menu entries name their icons; whether a name comes back out of them
    // depends on an icon theme being resolvable at all. Without one,
    // QIcon::fromTheme() yields a null icon with an empty name — the icon
    // search paths then hold nothing but the Qt resource (SPEC 10). The test
    // environment brings the KDE platform theme for the paths; the theme itself
    // is pinned here so the run does not depend on which one the user has set.
    QIcon::setThemeName(QStringLiteral("breeze"));
    QVERIFY2(QIcon::hasThemeIcon(QStringLiteral("document-edit")),
             "Breeze is missing — SPEC 10 takes the menu icons from the Breeze set.");
}

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

void ShellTest::announcesTheNoteItWrites()
{
    // The other half of the road of issue #105: the library hangs on
    // Store::noteAdded, and a note written from outside over the bus has to
    // ring the same bell as one written in the capture window. It does because
    // it goes through the same door — the announcement is the store's, not the
    // window's, and this is what says so for the D-Bus entry point.
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy added(m_store.get(), &Store::noteAdded);

    const qlonglong id = m_service->AddNote(QStringLiteral("Gedanke aus einem Skript"));

    QCOMPARE(added.size(), 1);
    QCOMPARE(added.first().first().toLongLong(), id);

    // Blank text is no note and announces nothing.
    QCOMPARE(m_service->AddNote(QStringLiteral("   \n  ")), 0);
    QCOMPARE(added.size(), 1);
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
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy requested(m_service.get(), &DaemonService::libraryRequested);

    m_service->ShowLibrary();

    QCOMPARE(requested.size(), 1);
}

void ShellTest::findsNoConflictWithoutOwners()
{
    QVERIFY(foreignShortcutOwners({}, QStringLiteral("io.github.hnsstrk.denkzettel.desktop")).isEmpty());
}

void ShellTest::ignoresOurOwnRegistration()
{
    // From the second start on kglobalacceld reports our own registration back
    // to us; treating that as a conflict would warn at every session start.
    const QList<ShortcutOwner> owners = {
        {QStringLiteral("io.github.hnsstrk.denkzettel.desktop"), QStringLiteral("Denkzettel")},
    };

    QVERIFY(foreignShortcutOwners(owners, QStringLiteral("io.github.hnsstrk.denkzettel.desktop")).isEmpty());
}

void ShellTest::reportsForeignOwner()
{
    const QList<ShortcutOwner> owners = {
        {QStringLiteral("kwin"), QStringLiteral("KWin")},
    };

    const QList<ShortcutOwner> conflicts =
        foreignShortcutOwners(owners, QStringLiteral("io.github.hnsstrk.denkzettel.desktop"));
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
        {QStringLiteral("io.github.hnsstrk.denkzettel.desktop"), QStringLiteral("Denkzettel")},
        {QStringLiteral("org.kde.spectacle.desktop"), QStringLiteral("Spectacle")},
    };

    const QList<ShortcutOwner> conflicts =
        foreignShortcutOwners(owners, QStringLiteral("io.github.hnsstrk.denkzettel.desktop"));
    QCOMPARE(conflicts.size(), 2);
    QCOMPARE(conflicts.at(0).component, QStringLiteral("kwin"));
    QCOMPARE(conflicts.at(1).component, QStringLiteral("org.kde.spectacle.desktop"));
}

void ShellTest::takesAnyAnswerOfTheDaemonAsRegistered()
{
    const QList<QKeySequence> stored = {QKeySequence(Qt::META | Qt::Key_N)};

    QCOMPARE(shortcutRegistration(stored, true, true), ShortcutRegistration::Reached);

    // Which sequence the daemon holds is none of our business — the user may
    // have changed it in the Plasma settings, and that is a registration that
    // arrived.
    const QList<QKeySequence> changed = {QKeySequence(Qt::META | Qt::Key_F10)};
    QCOMPARE(shortcutRegistration(changed, true, true), ShortcutRegistration::Reached);

    // An installation we cannot see is none we should judge: the daemon has
    // answered, so it knows us, and the desktop action is beyond our sight.
    QCOMPARE(shortcutRegistration(stored, false, false), ShortcutRegistration::Reached);
}

void ShellTest::readsAnEmptyAnswerWithoutDesktopFileAsAMissingInstallation()
{
    // The user finding of 01.08.2026: kglobalacceld resolves the component
    // through the desktop file and creates none without it, so the answer stays
    // empty however often we register.
    QCOMPARE(shortcutRegistration({}, false, false), ShortcutRegistration::ApplicationNotInstalled);
}

void ShellTest::readsAnEmptyAnswerWithDesktopFileAsADaemonThatKeptNothing()
{
    // Installed and still nothing stored — then the daemon is the suspect, and
    // a message blaming the installation would send the user the wrong way.
    QCOMPARE(shortcutRegistration({}, true, true), ShortcutRegistration::DaemonKeptNothing);
}

void ShellTest::readsAnUndeclaredDesktopActionAsAFailure()
{
    // The second user finding of 01.08.2026: registration and read-back are
    // both fine, and the key press still goes nowhere, because with an
    // installed desktop file kglobalacceld starts the desktop action of that
    // name instead of signalling us — and finds none.
    const QList<QKeySequence> stored = {QKeySequence(Qt::META | Qt::Key_N)};

    QCOMPARE(shortcutRegistration(stored, true, false), ShortcutRegistration::DesktopActionMissing);
}

void ShellTest::readsTheActionsOfADesktopFile()
{
    const QString path = m_dir->filePath(QStringLiteral("io.github.hnsstrk.denkzettel.desktop"));
    const QString entry = QStringLiteral("[Desktop Entry]\nType=Application\nName=Denkzettel\n"
                                         "Exec=denkzetteld\n");

    auto write = [&path](const QString &contents) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(file.write(contents.toUtf8()), contents.toUtf8().size());
    };

    // Both halves have to be there — kglobalacceld reads the list, then looks
    // for the group with the Exec line it is to start.
    write(entry + QStringLiteral("Actions=show-capture;\n\n[Desktop Action show-capture]\n"
                                 "Name=Capture note\nExec=denkzetteld\n"));
    QVERIFY(desktopFileDeclaresAction(path, QStringLiteral("show-capture")));

    write(entry + QStringLiteral("Actions=show-capture;\n"));
    QVERIFY(!desktopFileDeclaresAction(path, QStringLiteral("show-capture")));

    write(entry + QStringLiteral("\n[Desktop Action show-capture]\nName=Capture note\n"
                                 "Exec=denkzetteld\n"));
    QVERIFY(!desktopFileDeclaresAction(path, QStringLiteral("show-capture")));

    // The state before this sprint: a file without any action at all.
    write(entry);
    QVERIFY(!desktopFileDeclaresAction(path, QStringLiteral("show-capture")));

    // What cannot be read cannot be vouched for.
    QVERIFY(!desktopFileDeclaresAction(m_dir->filePath(QStringLiteral("gibtsnicht.desktop")),
                                       QStringLiteral("show-capture")));
    QVERIFY(!desktopFileDeclaresAction(QString(), QStringLiteral("show-capture")));
}

void ShellTest::hasAMessageForEveryFailureAndNoneForSuccess()
{
    // A failure without a message is exactly the silent failure SPEC 2.4
    // forbids; a message on success would cry wolf at every start.
    QVERIFY(shortcutRegistrationFailure(ShortcutRegistration::Reached).isEmpty());
    QVERIFY(!shortcutRegistrationFailure(ShortcutRegistration::ApplicationNotInstalled).isEmpty());
    QVERIFY(!shortcutRegistrationFailure(ShortcutRegistration::DaemonKeptNothing).isEmpty());
    QVERIFY(!shortcutRegistrationFailure(ShortcutRegistration::DesktopActionMissing).isEmpty());

    // The failures are told apart for the user as well, not only in code.
    const QStringList messages = {
        shortcutRegistrationFailure(ShortcutRegistration::ApplicationNotInstalled),
        shortcutRegistrationFailure(ShortcutRegistration::DaemonKeptNothing),
        shortcutRegistrationFailure(ShortcutRegistration::DesktopActionMissing),
    };
    QCOMPARE(QSet<QString>(messages.begin(), messages.end()).size(), messages.size());
}

void ShellTest::hintsTheShortcutWithoutBindingItASecondTime()
{
    // The hint Meta+N is a hint: it is drawn next to the entry and must not
    // become a second binding beside the one kglobalacceld holds (issue #60).
    // A shortcut on a menu action reaches only the window of that menu, and the
    // tray menu has none — it is drawn by plasmashell. So the entry may carry
    // the sequence for display, and no window of ours may answer to it.
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    TrayIcon icon;
    const QList<QAction *> entries = icon.item()->contextMenu()->actions();
    const QAction *capture = entries.constFirst();

    QCOMPARE(capture->shortcut(), QKeySequence(Qt::META | Qt::Key_N));
    QCOMPARE(capture->shortcutContext(), Qt::WidgetShortcut);
}

void ShellTest::asksForTheSettingsDialog()
{
    // Acceptance criterion 3 of issue #16, and the half of it that breaks
    // without a sound: an entry that is there and connected to nothing looks
    // exactly like one that works. The other half — that main.cpp hangs the
    // dialog on this signal — is one line and is read.
    //
    // The entry names the application ("Configure Denkzettel…") because it
    // stands among entries of other programs; the window it opens does not,
    // because the decoration appends the name (UX decision of 29.08.2026).
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    TrayIcon icon;
    QAction *configure = nullptr;
    const QList<QAction *> entries = icon.item()->contextMenu()->actions();
    for (QAction *entry : entries) {
        if (entry->text() == QStringLiteral("Configure Denkzettel…")) {
            configure = entry;
        }
    }
    QVERIFY2(configure, "the tray menu carries no entry for the settings");
    // Enabled, unlike the stubs beside it: a permanently greyed entry does not
    // tell the user why it is greyed.
    QVERIFY(configure->isEnabled());
    QCOMPARE(configure->icon().name(), QStringLiteral("configure"));

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy requested(&icon, &TrayIcon::configureRequested);
    configure->trigger();
    QCOMPARE(requested.count(), 1);
}

void ShellTest::showsAFailedTranscriptionAndTakesItBack()
{
    // The error state of SPEC 10 is a **transition**, and a picture of the
    // resting state says nothing about it (CLAUDE.md, finding 27). What the
    // item announces goes over the tray protocol, where offscreen there is no
    // host to read it, so it is read back from the item itself (issue #44).
    TrayIcon icon;
    // Asserted before anything is set: without this line the check below would
    // stand green for an item that is in NeedsAttention from the start.
    QCOMPARE(icon.item()->status(), KStatusNotifierItem::Active);
    const QString quiet = icon.item()->toolTipSubTitle();
    QVERIFY(!quiet.isEmpty());

    const QString reason = QStringLiteral("/usr/bin/whisper-cli ended with code 1");
    icon.setTranscriptionError(reason);
    QCOMPARE(icon.item()->status(), KStatusNotifierItem::NeedsAttention);
    // The cause travels with it: an icon that is set apart and says nothing
    // leaves the user looking for the fault in their recording (SPEC 10).
    QVERIFY2(icon.item()->toolTipSubTitle().contains(reason),
             qPrintable(icon.item()->toolTipSubTitle()));

    // And back, because the next transcript that comes through does take it
    // back — the state must not stand for the rest of the session.
    icon.setTranscriptionError({});
    QCOMPARE(icon.item()->status(), KStatusNotifierItem::Active);
    QCOMPARE(icon.item()->toolTipSubTitle(), quiet);
}

QTEST_MAIN(ShellTest)

#include "shelltest.moc"
