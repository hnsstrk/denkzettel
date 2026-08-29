#include "analysis/analysisscheduler.h"
#include "analysis/classifier.h"
#include "analysis/ollamaprovider.h"
#include "capture/capturewindow.h"
#include "platform/optionaltools.h"
#include "platform/systemfonts.h"
#include "settings/settings.h"
#include "settings/settingsdialog.h"
#include "shell/appidentity.h"
#include "shell/daemonservice.h"
#include "shell/firstrun.h"
#include "shell/globalshortcuts.h"
#include "shell/trayicon.h"
#include "store/store.h"
#include "transcribe/modeldownload.h"
#include "transcribe/transcriber.h"
#include "ui/librarywindow.h"
#include "ui/timestampformat.h"

#include <KConfigGroup>
#include <KDBusService>
#include <KLocalizedString>
#include <KNotification>
#include <KSharedConfig>

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QStringList>

#include <optional>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Plasma does not pass a font change on to a running Qt Widgets
    // application — measured 24.08.2026: with the KDE platform theme loaded, a
    // font raised in kdeglobals from 10 pt to 16 pt never arrived. The daemon
    // keeps its two windows for the whole session (SPEC 2.1), so without this
    // they would carry the old font until the next login (issue #68).
    platform::followSystemFonts(&app);

    // Name, version, organisation domain and desktop file in one place —
    // shell/appidentity.cpp says why they may not be set anywhere else.
    registerApplicationIdentity();

    // Before KDBusService below, and that order carries the whole option
    // handling: with a running daemon the single-instance switch hands a second
    // start over to the first process, so --version would open a capture window
    // there instead of writing a line here — and return 0 while doing it
    // (measured, pre-check for #61, F2/F3). Without a
    // reachable session bus KDBusService ends the process with 1, so the
    // version would not appear in the automated run either.
    processCommandLineArguments(app);

    app.setQuitOnLastWindowClosed(false);
    // The bundled copy covers runs from the build directory, before the icon
    // is installed into any theme (issue #43).
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("denkzettel"),
                                       QIcon(QStringLiteral(":/icons/denkzettel.svg"))));

    // KDBusService builds the bus name from the reversed organization domain
    // plus the application name (see kdbusservice.h): hnsstrk.github.io and
    // denkzettel give io.github.hnsstrk.denkzettel, which SPEC 2.3 fixes as the
    // name. Both values come from registerApplicationIdentity() above, so
    // nothing has to be renamed around the registration any more (issue #112).
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    KDBusService service(KDBusService::Unique);

    Store store(Store::defaultDatabasePath());
    if (!store.open()) {
        qCritical("Opening the database failed: %s", qPrintable(store.lastError()));
        return 1;
    }

    // The cleanup check of SPEC 2.5, and here is the only place it may stand:
    // a recording writes its file before the note that points at it exists, so
    // the sweep has to be over before the first window can record. Nothing
    // else in this process touches that directory yet.
    store.sweepOrphanedAudio();

    // Before the transcriber below and before anything reads Settings::self():
    // it takes its model in its constructor, and the dialog's first Apply
    // writes every item of the skeleton at once. The reasoning and the
    // measurement stand at the function (issue #27).
    migrateModelPath();

    // Built before the first window and started at the end of this function:
    // it listens on Store::noteAdded for the audio notes to come, and its
    // start() picks up what an earlier run left in the queue (SPEC 12).
    Transcriber transcriber(&store);

    // The model download of SPEC 12 (issue #23). It belongs to the process and
    // not to the settings dialog: the dialog is built and destroyed per
    // opening, and a file of gigabytes must not go with it. The page shows what
    // it is doing and is the only thing that can stop it.
    ModelDownload modelDownload;

    // What the settings page "Voice notes" writes reaches the running queue at
    // once — model size and program path take hold without a restart (SPEC 13,
    // issue #27). The connection hangs on the skeleton and not on the dialog:
    // the dialog is built and destroyed per opening, this object lives as long
    // as the process. KCoreConfigSkeleton::save() emits configChanged() once
    // per save that changed something and not at all for one that did not
    // (measured 29.08.2026), so this runs when there is something new to read
    // and never on its own.
    QObject::connect(Settings::self(), &Settings::configChanged,
                     &transcriber, &Transcriber::reloadSettings);

    CaptureWindow capture(&store);

    // SPEC 2.3: a second process start must surface the capture window in the
    // running instance instead of just exiting.
    QObject::connect(&service, &KDBusService::activateRequested, &capture,
                     [&capture](const QStringList &, const QString &) {
                         capture.showCapture();
                     });

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    LibraryWindow library(&store);

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    TrayIcon tray;
    QObject::connect(&tray, &TrayIcon::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&tray, &TrayIcon::libraryRequested, &library, &LibraryWindow::showLibrary);
    // The error path of the transcription reaches the user here and nowhere
    // else (SPEC 10 and 12, issue #24). Both edges ask the same question of the
    // database rather than each carrying its own answer: **the state has to
    // stand where the queue stands.** A success that simply cleared the state
    // would make the icon quiet again while another note lies given up on in
    // the queue, and only the next start of the service would bring it back.
    //
    // Asked on both signals and not on failed(): a first attempt is followed
    // by a second one, so pausedTranscribeJob() still answers nothing there,
    // and a state raised for it would clear itself a moment later.
    const auto showWhereTheQueueStands = [&store, &tray] {
        const std::optional<TranscribeJob> givenUp = store.pausedTranscribeJob();
        tray.setTranscriptionError(givenUp.has_value() ? givenUp->lastError : QString());
    };
    QObject::connect(&transcriber, &Transcriber::paused, &tray, showWhereTheQueueStands);
    QObject::connect(&transcriber, &Transcriber::transcribed, &tray, showWhereTheQueueStands);

    // And the loud channel beside it (SPEC 10 and 14, issue #115): a voice note
    // whose transcription has finally failed is the case the user otherwise
    // notices nothing of for weeks, so it says so — once per note, because
    // paused() is emitted where the attempts are used up and failed() is the
    // one that fires per attempt.
    //
    // Armed only after start() below, and that is the whole of acceptance
    // criterion 3: start() emits paused() for a job that was already given up
    // on before this process began, so that the tray stands where the queue
    // stands after a restart. Announced, it would greet every login with last
    // week's failure. A note that is gone says nothing either — deleting it
    // takes its job row with it (ON DELETE CASCADE), and there is nothing left
    // to tell the user about.
    bool announceGivingUp = false;
    QObject::connect(&transcriber, &Transcriber::paused, &app,
                     [&store, &announceGivingUp](qint64 noteId, const QString &reason) {
                         if (!announceGivingUp) {
                             return;
                         }
                         const std::optional<Note> note = store.note(noteId);
                         if (!note.has_value()) {
                             return;
                         }
                         // The moment it was recorded is what names the note:
                         // that is the handle the library lists it under (SPEC
                         // 9), and one without a transcript has no other. It is
                         // written in the form the library writes it in, out of
                         // the same function — a second form of the same
                         // timestamp would be a second thing to look for.
                         KNotification::event(
                             KNotification::Warning,
                             i18n("Transcription failed"),
                             i18n("The voice note of %1 could not be transcribed: %2",
                                  library::entryTimestamp(note->createdAt, QLocale()),
                                  reasonWithoutDirectories(reason)));
                     });

    // The analysis run of SPEC 7.2 and what sets it going. Ollama without asking
    // `[AI] Provider`: it is the only one of the three backends of SPEC 7.1 that
    // is built, and a setting naming another would be answered by nobody.
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    OllamaProvider provider;
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    Classifier classifier(&store, &provider);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    AnalysisScheduler analysis(&classifier);

    // The two roads that mean "a note is ready" under the trigger "at once"
    // (SPEC 7.2): a note that is written, and a voice note whose transcript has
    // just arrived — the second is nothing the store announces, its text comes
    // long after the row did. The scheduler links neither side and is handed
    // both from here.
    QObject::connect(&store, &Store::noteAdded, &analysis, &AnalysisScheduler::noteIsReady);
    QObject::connect(&transcriber, &Transcriber::transcribed, &analysis, &AnalysisScheduler::noteIsReady);

    // What the settings dialog wrote reaches the running trigger here, and only
    // here: without it a switched mode would take effect at the next start of
    // the daemon and look like a setting that does nothing (SPEC 13, issue #16).
    QObject::connect(Settings::self(), &Settings::configChanged, &analysis, &AnalysisScheduler::applySettings);

    QObject::connect(&tray, &TrayIcon::analysisRequested, &analysis, &AnalysisScheduler::analyzeNow);

    // The tool detection of SPEC 2.5 (issue #17), and it runs at **every**
    // start and not only at the first: what it produces is a state and not
    // something that happened once (SPEC 14), so a program uninstalled after
    // the first login would otherwise never be noticed again. Which is also
    // why it is not in runFirstStart() further down — that function guards the
    // steps that must not repeat, and this one has to.
    //
    // The programs are asked for as the transcription queue holds them, not as
    // denkzettelrc spells them: the group and the defaults have one place, and
    // it is Transcriber::reloadSettings().
    const QStringList missingPrograms =
        tools::missing({transcriber.ffmpegProgram(), transcriber.whisperProgram(),
                        QString(tools::TaskProgram)});
    tray.setUnavailableTools(missingPrograms);

    // The fourth of them is a server and not a program, so its answer only
    // arrives through the event loop. Over the provider of issue #13 — no
    // second HTTP road of our own, SPEC 7.1 puts the timeout into that class
    // and a check written beside it would have none — but over
    // testReachability() and **not** testConnection(): the button of SPEC 7.1
    // makes a real chat and a real embed call and loads both models for it,
    // which measured 4.7 s here and would be paid at every single login. The
    // reasoning and the numbers stand at that method.
    QObject::connect(&provider, &OllamaProvider::reachabilityTested, &tray,
                     [&tray, missingPrograms](bool reachable) {
                         QStringList unavailable = missingPrograms;
                         if (!reachable) {
                             // The name of the server and not the reason it
                             // did not answer: the tooltip is the quiet
                             // channel (SPEC 14), and the reason is what the
                             // settings button shows. Untranslated because it
                             // is the program's name, like the three above it.
                             unavailable.append(QStringLiteral("Ollama"));
                         }
                         tray.setUnavailableTools(unavailable);
                     });
    provider.testReachability();

    // A note that has failed its classification twice is never handed to a model
    // again (SPEC 7.2), and this line is what SPEC 14 asks to be reported of it.
    //
    // ponytail: the log alone, although SPEC 14 names "tray tooltip + log" for
    // it. Since issue #17 the tooltip does take a part per source rather than
    // one writer's line (TrayIcon::showToolTip()), so covering the other
    // trouble up is no longer the obstacle — the state standing for ever is:
    // such a note is never taken up again, and there is no place yet where the
    // user could work one off. **The ceiling is the missing entry
    // "Unclassified" in the category column of issue #18**;
    // with it the tooltip becomes a count of both halves ("2 notes without a
    // transcript · 1 without a category") and this line stays as the detail
    // (UX decision of 29.08.2026).
    //
    // Where it is read: `journalctl --user -t denkzetteld`. Qt's default handler
    // writes to the journal wherever stderr is not a terminal, so in the running
    // service the sentence never reaches a pipe (CLAUDE.md, finding 25).
    QObject::connect(&classifier, &Classifier::paused, &app, [](qint64 noteId, const QString &reason) {
        qWarning("Note %lld is left without a category: %s", noteId, qUtf8Printable(reason));
    });

    DaemonService daemon(&store);
    QObject::connect(&daemon, &DaemonService::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&daemon, &DaemonService::libraryRequested, &library, &LibraryWindow::showLibrary);
    QObject::connect(&daemon, &DaemonService::analysisRequested, &analysis, &AnalysisScheduler::analyzeNow);
    QObject::connect(&daemon, &DaemonService::quitRequested, &app, &QApplication::quit);
    if (!daemon.registerOnSessionBus()) {
        qWarning("Exporting io.github.hnsstrk.denkzettel.Daemon failed; the D-Bus entry points are unavailable.");
    }

    // Store::open() above has created data directory and database; this
    // completes the first start of SPEC 2.5.
    KConfigGroup general(KSharedConfig::openConfig(), QStringLiteral("General"));
    const bool firstRun = runFirstStart(general);

    GlobalShortcuts shortcuts;
    QObject::connect(&shortcuts, &GlobalShortcuts::captureRequested, &capture, &CaptureWindow::showCapture);

    // The settings are free-standing and belong to no window, so no parent is
    // handed over — the dialog finds the standing one itself or builds a new
    // one (SPEC 13, issue #16). The shortcuts are: the page "Shortcuts" writes
    // through the same two actions that are registered here, and a second pair
    // of its own would take the key press away from these on the way in and
    // switch them off again on the way out (issue #74).
    QObject::connect(&tray, &TrayIcon::configureRequested, &app, [&shortcuts, &modelDownload] {
        SettingsDialog::showSettings(&shortcuts, &modelDownload);
    });

    const QList<ShortcutOwner> conflicts = shortcuts.registerCaptureShortcut();
    if (firstRun && !conflicts.isEmpty()) {
        // The sequence that is really registered, not the one that was asked
        // for: autoloading hands back what the user set, and since the settings
        // page that is no longer necessarily Meta+N.
        notifyShortcutConflict(GlobalShortcuts::assignedSequence(GlobalShortcuts::Shortcut::Capture), conflicts);
    }

    // Last, and after the first start above: the queue may hold a job from a
    // run that was killed, and working it off is the same road as a fresh one.
    transcriber.start();
    // Everything given up on from here on happened while the user was watching,
    // and only that is worth a notification (issue #115).
    announceGivingUp = true;

    return app.exec();
}
