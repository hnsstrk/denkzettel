#include "platform/optionaltools.h"
#include "shell/daemonservice.h"
#include "shell/shortcutconflict.h"
#include "shell/shortcutregistration.h"
#include "shell/trayicon.h"
#include "store/store.h"

#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QSet>
#include <QSignalSpy>
#include <QStringList>
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
    void asksForAnAnalysisRun();
    void asksForARecording();
    void showsAFailedTranscriptionAndTakesItBack();

    void findsAProgramByItsPathAndByItsName();
    void countsAFileWithoutAnExecuteBitAsMissing();
    void namesTheMissingToolsBesideTheFailedTranscription();

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
    // Since the settings page of SPEC 13 the sequence is the user's to choose,
    // so every message has to name the one that was really registered. A text
    // that dropped its argument would read like a message about Meta+N on a
    // machine where Meta+N is not set — which is what it used to say
    // literally (issue #74).
    const QKeySequence sequence(Qt::META | Qt::SHIFT | Qt::Key_K);
    const QString keys = sequence.toString(QKeySequence::NativeText);

    // A failure without a message is exactly the silent failure SPEC 2.4
    // forbids; a message on success would cry wolf at every start.
    QVERIFY(shortcutRegistrationFailure(ShortcutRegistration::Reached, sequence).isEmpty());

    const QStringList messages = {
        shortcutRegistrationFailure(ShortcutRegistration::ApplicationNotInstalled, sequence),
        shortcutRegistrationFailure(ShortcutRegistration::DaemonKeptNothing, sequence),
        shortcutRegistrationFailure(ShortcutRegistration::DesktopActionMissing, sequence),
    };
    for (const QString &message : messages) {
        QVERIFY(!message.isEmpty());
        QVERIFY2(message.contains(keys), qPrintable(message));
    }

    // The failures are told apart for the user as well, not only in code.
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

void ShellTest::asksForAnAnalysisRun()
{
    // Acceptance criterion 3 of issue #15. The entry stood in the menu before
    // this story as a greyed stub — so what breaks without a sound is not that
    // it is there but that it is **live**: an entry connected to nothing looks
    // exactly like one that works, and the run it starts is invisible anyway
    // (SPEC 14 keeps a routine run quiet).
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    TrayIcon icon;
    QAction *analyze = nullptr;
    const QList<QAction *> entries = icon.item()->contextMenu()->actions();
    for (QAction *entry : entries) {
        if (entry->text() == QStringLiteral("Analyze now")) {
            analyze = entry;
        }
    }
    QVERIFY2(analyze, "the tray menu carries no entry for an analysis run");
    QVERIFY(analyze->isEnabled());
    QCOMPARE(analyze->icon().name(), QStringLiteral("system-run"));

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy requested(&icon, &TrayIcon::analysisRequested);
    analyze->trigger();
    QCOMPARE(requested.count(), 1);
}

void ShellTest::asksForARecording()
{
    // Acceptance criterion 3 of issue #21 from the tray's side. The entry stood
    // in the menu before this story as a greyed stub, so what breaks without a
    // sound is not that it is there but that it is **live**: an entry connected
    // to nothing looks exactly like one that works, and the window it opens
    // starts recording by itself — nobody would look for the fault here.
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    TrayIcon icon;
    QAction *record = nullptr;
    const QList<QAction *> entries = icon.item()->contextMenu()->actions();
    for (QAction *entry : entries) {
        if (entry->text() == QStringLiteral("Record voice note")) {
            record = entry;
        }
    }
    QVERIFY2(record, "the tray menu carries no entry for a voice note");
    QVERIFY(record->isEnabled());
    QCOMPARE(record->icon().name(), QStringLiteral("audio-input-microphone"));
    // A hint and not a second binding, as beside the capture entry (issue #60).
    QCOMPARE(record->shortcut(), QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_N));
    QCOMPARE(record->shortcutContext(), Qt::WidgetShortcut);

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy requested(&icon, &TrayIcon::recorderRequested);
    record->trigger();
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

namespace
{
/** An empty file at `path` with exactly `mode` on it, and whether it worked. */
bool putProgram(const QString &path, QFile::Permissions mode)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.close();
    return file.setPermissions(mode);
}
}

void ShellTest::findsAProgramByItsPathAndByItsName()
{
    // The two roads a setting takes: an absolute path, which SPEC 12 makes
    // configurable for both programs of the transcription, and a bare name,
    // which is what SPEC 8.2 runs `task` as. **Neither may go over the system
    // PATH of the machine this runs on**: here `ffmpeg`, `whisper-cli` and
    // `task` are all installed, so a run that is to show "missing" could never
    // come out red on it (CLAUDE.md, finding 33). Every case below stands in a
    // directory this function makes.
    const QTemporaryDir programs;
    QVERIFY(programs.isValid());
    const QString program = programs.filePath(QStringLiteral("denkzettel-probe"));
    QVERIFY(putProgram(program, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));

    QVERIFY(tools::isRunnable(program));
    QVERIFY(!tools::isRunnable(programs.filePath(QStringLiteral("denkzettel-absent"))));

    // Read out before the PATH is put back, so that a failing assertion does
    // not leave the rest of the run with a PATH holding one directory.
    const QByteArray inherited = qgetenv("PATH");
    qputenv("PATH", programs.path().toLocal8Bit());
    const bool byName = tools::isRunnable(QStringLiteral("denkzettel-probe"));
    const bool absentByName = tools::isRunnable(QStringLiteral("denkzettel-absent"));
    qputenv("PATH", inherited);
    QVERIFY(byName);
    QVERIFY(!absentByName);

    // And the third road, which is neither of the two: a **relative** path.
    // QProcess starts it off the working directory, and it must not be
    // searched along PATH, where it is never found — a wrong "not available"
    // sends the user looking for a program that is there. The directory is
    // changed and put back around the one call.
    const QString here = QDir::currentPath();
    QVERIFY(QDir::setCurrent(programs.path()));
    const bool relative = tools::isRunnable(QStringLiteral("./denkzettel-probe"));
    QVERIFY(QDir::setCurrent(here));
    QVERIFY(relative);
}

void ShellTest::countsAFileWithoutAnExecuteBitAsMissing()
{
    // The case an implementation written with QFile::exists() waves through in
    // silence, and the one the user meets only at the moment they wanted the
    // function — as "could not be started" (SPEC 12).
    //
    // It may stand in an automated set although finding 46 keeps permission
    // checks out of one: what root overrides is reading, writing and entering
    // a directory. X_OK is granted to uid 0 only where at least one execute
    // bit is set, so a file at 0644 is executable for nobody.
    const QTemporaryDir programs;
    QVERIFY(programs.isValid());
    const QString readable = programs.filePath(QStringLiteral("denkzettel-readable"));
    QVERIFY(putProgram(readable,
                       QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther));
    // Both halves, because only together do they say what the check is about:
    // the file is there, and it still cannot be started.
    QVERIFY(QFileInfo::exists(readable));
    QVERIFY(!tools::isRunnable(readable));

    // And what is handed on is the program's NAME — not the path it was found
    // under and not a package (UX decision of 29.08.2026).
    QCOMPARE(tools::missing({readable}), QStringList{QStringLiteral("denkzettel-readable")});
}

void ShellTest::namesTheMissingToolsBesideTheFailedTranscription()
{
    // `setToolTipSubTitle()` takes ONE string and two sources write into it, so
    // each writes a part and neither pushes the other out (issue #118). That is
    // what breaks without a sound: a user who reads one trouble takes it for
    // the whole of what is wrong and never learns of the second.
    TrayIcon icon;
    const QString quiet = icon.item()->toolTipSubTitle();

    icon.setUnavailableTools({QStringLiteral("task"), QStringLiteral("Ollama")});
    const QString named = icon.item()->toolTipSubTitle();
    QVERIFY2(named.contains(QStringLiteral("task")), qPrintable(named));
    QVERIFY2(named.contains(QStringLiteral("Ollama")), qPrintable(named));
    // And no error state with it: this one stands from the first login and
    // never falls by itself, and a permanent NeedsAttention is read by nobody.
    QCOMPARE(icon.item()->status(), KStatusNotifierItem::Active);

    const QString reason = QStringLiteral("/usr/bin/whisper-cli ended with code 1");
    icon.setTranscriptionError(reason);
    const QString both = icon.item()->toolTipSubTitle();
    QVERIFY2(both.contains(reason), qPrintable(both));
    QVERIFY2(both.contains(QStringLiteral("task")), qPrintable(both));
    QCOMPARE(icon.item()->status(), KStatusNotifierItem::NeedsAttention);

    // The other way round as well, because the writer that comes second is the
    // one that would overwrite: with the transcription standing, the tools are
    // set again.
    icon.setUnavailableTools({QStringLiteral("ffmpeg")});
    const QString again = icon.item()->toolTipSubTitle();
    QVERIFY2(again.contains(reason), qPrintable(again));
    QVERIFY2(again.contains(QStringLiteral("ffmpeg")), qPrintable(again));

    // Both taken back, and what stands is the resting line again — not an
    // empty one, and not one that kept a remnant.
    icon.setTranscriptionError({});
    icon.setUnavailableTools({});
    QCOMPARE(icon.item()->toolTipSubTitle(), quiet);
    QCOMPARE(icon.item()->status(), KStatusNotifierItem::Active);
}

QTEST_MAIN(ShellTest)

#include "shelltest.moc"
