#include "shell/daemonservice.h"
#include "shell/shortcutconflict.h"
#include "shell/shortcutregistration.h"
#include "shell/trayicon.h"
#include "store/store.h"

#include <KStatusNotifierItem>

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

    void announcesItselfAsAMenuAndKeepsTheMenuToShow();
    void showsTheEntriesOfTheWireframeWithTheirIcons();
    void keepsQuitApartInTheLastGroup();
    void hintsTheShortcutWithoutBindingItASecondTime();

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    std::unique_ptr<DaemonService> m_service;
};

void ShellTest::initTestCase()
{
    // The menu entries name their icons; whether a name comes back out of them
    // depends on an icon theme being resolvable at all. Without one,
    // QIcon::fromTheme() yields a null icon with an empty name — the icon
    // search paths then hold nothing but the Qt resource (SPEC 10). The test
    // environment brings the KDE platform theme for the paths; the theme itself
    // is pinned here so the run does not depend on which one the user has set.
    QIcon::setThemeName(QStringLiteral("breeze"));
    QVERIFY2(QIcon::hasThemeIcon(QStringLiteral("document-edit")),
             "Breeze fehlt — SPEC 10 holt die Menü-Symbole aus dem Breeze-Bestand.");
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
    // The customer finding of 01.08.2026: kglobalacceld resolves the component
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
    // The second customer finding of 01.08.2026: registration and read-back are
    // both fine, and the key press still goes nowhere, because with an
    // installed desktop file kglobalacceld starts the desktop action of that
    // name instead of signalling us — and finds none.
    const QList<QKeySequence> stored = {QKeySequence(Qt::META | Qt::Key_N)};

    QCOMPARE(shortcutRegistration(stored, true, false), ShortcutRegistration::DesktopActionMissing);
}

void ShellTest::readsTheActionsOfADesktopFile()
{
    const QString path = m_dir->filePath(QStringLiteral("org.denkzettel.Denkzettel.desktop"));
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
                                 "Name=Notiz erfassen\nExec=denkzetteld\n"));
    QVERIFY(desktopFileDeclaresAction(path, QStringLiteral("show-capture")));

    write(entry + QStringLiteral("Actions=show-capture;\n"));
    QVERIFY(!desktopFileDeclaresAction(path, QStringLiteral("show-capture")));

    write(entry + QStringLiteral("\n[Desktop Action show-capture]\nName=Notiz erfassen\n"
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

void ShellTest::announcesItselfAsAMenuAndKeepsTheMenuToShow()
{
    TrayIcon icon;

    // ItemIsMenu has no change signal in the SNI protocol: the host reads the
    // property when the item registers and never asks again, so it has to stand
    // by the time the constructor is through (issue #44). Whether the panel then
    // opens the menu on a left click is the customer's check at the panel — no
    // agent can produce that click under Wayland.
    QVERIFY(icon.item()->isMenu());

    // An item that announces a menu and has none would swallow the click. The
    // right click has to keep finding the same menu it found before.
    QVERIFY(icon.item()->contextMenu() != nullptr);
    QVERIFY(!icon.item()->contextMenu()->actions().isEmpty());
}

void ShellTest::showsTheEntriesOfTheWireframeWithTheirIcons()
{
    // Wireframe 5a and issue #60 fix labels, icon names, order and state. The
    // icon name is the test subject, not the picture: only an icon taken from
    // the theme carries a name, and only a name travels to Plasma over the tray
    // protocol. An icon built from a resource would arrive nameless, and the
    // assurance would not be checkable at all.
    struct Entry {
        const char *text;
        const char *iconName;
        bool enabled;
    };
    const QList<Entry> expected = {
        {"Notiz erfassen", "document-edit", true},
        {"Sprachnotiz aufnehmen", "audio-input-microphone", false},
        {nullptr, nullptr, false}, // Trenner: Erfassen von Ansehen und Verarbeiten
        {"Bibliothek öffnen", "view-list-text", true},
        {"Jetzt analysieren", "system-run", false},
        {"Vorschläge", "tools-wizard", false},
        {nullptr, nullptr, false}, // Trenner: Arbeitswege von der Verwaltung
        {"Beenden", "application-exit", true},
    };

    TrayIcon icon;
    const QList<QAction *> actions = icon.item()->contextMenu()->actions();
    QCOMPARE(actions.size(), expected.size());

    for (qsizetype i = 0; i < expected.size(); ++i) {
        const Entry &entry = expected.at(i);
        QAction *action = actions.at(i);

        if (entry.text == nullptr) {
            QVERIFY2(action->isSeparator(), qPrintable(QStringLiteral("Eintrag %1 ist kein Trenner").arg(i)));
            continue;
        }

        QCOMPARE(action->text(), QString::fromUtf8(entry.text));
        QCOMPARE(action->icon().name(), QString::fromUtf8(entry.iconName));
        QCOMPARE(action->isEnabled(), entry.enabled);
    }
}

void ShellTest::keepsQuitApartInTheLastGroup()
{
    // Customer finding 1 of 02.08.2026 asked for "Beenden" to leave the left
    // click list. Two menus would have been the answer; the measurement of
    // 02.08.2026 (docs/scrum/reviews/sprint-04-s33-traymenues/messung.md) shows
    // they do not carry under Wayland. What is left of the finding is the
    // distance: the destructive action is last and behind a separator, never
    // next to the entry that is used most.
    TrayIcon icon;
    const QList<QAction *> actions = icon.item()->contextMenu()->actions();

    QVERIFY(!actions.isEmpty());
    QCOMPARE(actions.last()->text(), QStringLiteral("Beenden"));
    QVERIFY(actions.at(actions.size() - 2)->isSeparator());
    QVERIFY(!actions.first()->isSeparator());
    QVERIFY(actions.first()->text() != QStringLiteral("Beenden"));
}

void ShellTest::hintsTheShortcutWithoutBindingItASecondTime()
{
    // The hint Meta+N is a hint: it is drawn next to the entry and must not
    // become a second binding beside the one kglobalacceld holds (issue #60).
    // A shortcut on a menu action reaches only the window of that menu, and the
    // tray menu has none — it is drawn by plasmashell. So the entry may carry
    // the sequence for display, and no window of ours may answer to it.
    TrayIcon icon;
    QAction *capture = icon.item()->contextMenu()->actions().first();

    QCOMPARE(capture->shortcut(), QKeySequence(Qt::META | Qt::Key_N));
    QCOMPARE(capture->shortcutContext(), Qt::WidgetShortcut);
}

QTEST_MAIN(ShellTest)

#include "shelltest.moc"
