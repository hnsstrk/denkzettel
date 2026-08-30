#include "analysis/analysisscheduler.h"
#include "analysis/classifier.h"
#include "analysis/embedder.h"
#include "analysis/ollamaprovider.h"
#include "analysis/suggester.h"
#include "capture/capturewindow.h"
#include "capture/recordingwindow.h"
#include "platform/optionaltools.h"
#include "platform/systemfonts.h"
#include "settings/settingsdialog.h"
#include "settings/settingswiring.h"
#include "shell/appidentity.h"
#include "shell/daemonservice.h"
#include "shell/firstrun.h"
#include "shell/globalshortcuts.h"
#include "shell/originwatcher.h"
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
#include <QKeySequence>
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

    // The queue is taken up again once a download has put the missing model of
    // SPEC 12 in place: otherwise the job it stopped for would lie there until
    // the next start of the daemon — still in the queue with its attempts
    // untouched, and nothing else ever asks again. start() does nothing when
    // there is no job and nothing when the model is still missing.
    //
    // The other road to the same thing is a size that is already on disk being
    // chosen in the settings, and that one is wired in
    // connectSettingsToRunningObjects() below with the rest of them (issue
    // #123).
    QObject::connect(&modelDownload, &ModelDownload::finished,
                     &transcriber, &Transcriber::start);

    CaptureWindow capture(&store);

    // The recording window of SPEC 4, built beside the capture window and kept
    // for the whole life of the daemon like it (SPEC 2.1). Nothing else has to
    // be wired for the transcription: the queue hangs on Store::noteAdded, and
    // the note this window writes when a recording is finished is what reaches
    // it (SPEC 12, issue #22).
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    RecordingWindow recorder(&store);

    // SPEC 2.3: a second process start must surface the capture window in the
    // running instance instead of just exiting.
    QObject::connect(&service, &KDBusService::activateRequested, &capture,
                     [&capture](const QStringList &, const QString &) {
                         capture.showCapture();
                     });

    // And this is where a **key press** arrives, which is not the same road
    // (SPEC 2.4, issue #125). kglobalacceld starts the desktop action of the
    // shortcut's name through an ApplicationLauncherJob; with DBusActivatable
    // set that job calls org.freedesktop.Application.ActivateAction(<name>) on
    // this process instead of running the Exec line, and KDBusService hands the
    // name on here.
    //
    // Without it both shortcuts came in through activateRequested above — the
    // same start line, no distinguishing mark — and every press showed the
    // capture window. Meta+N looked reliable because its target happened to be
    // what always happened, and Meta+Shift+N could not reach the recorder at
    // all; the customer found it on 29.08.2026, after every measurable thing
    // about the registration had come out right.
    //
    // The names come from GlobalShortcuts and are spelled nowhere else: the
    // desktop action group, the QAction's object name and this branch are three
    // readings of one string, and a mismatch is silent on all three roads
    // (CLAUDE.md, finding 48).
    QObject::connect(&service, &KDBusService::activateActionRequested, &app,
                     [&capture, &recorder](const QString &actionName, const QVariant &) {
                         if (actionName == GlobalShortcuts::actionId(GlobalShortcuts::Shortcut::Capture)) {
                             capture.showCapture();
                         } else if (actionName == GlobalShortcuts::actionId(GlobalShortcuts::Shortcut::Recorder)) {
                             recorder.showRecorder();
                         } else {
                             // Nothing shown, and said in the one place a silent
                             // fault of this kind can be read afterwards
                             // (CLAUDE.md, finding 25): showing *something* is
                             // how issue #125 stayed invisible for four weeks.
                             qWarning("A desktop action nobody knows was activated: %s",
                                      qUtf8Printable(actionName));
                         }
                     });

    // The context stamp of SPEC 5.1 and 13 (issue #47). It stands between the
    // two windows that write notes and KWin, and it is told what to do by the
    // setting alone: with „[Capture] StoreOrigin" off it loads nothing into
    // KWin, so there is nothing determined that could be thrown away.
    //
    // **Both windows are connected and not one.** The capture window writes the
    // text note, the recording window the voice note, and a wiring that reached
    // only one of them would leave one kind of note without an origin — the
    // fault every check that walks a single road falls through.
    //
    // The D-Bus method AddNote() is deliberately **not** among them: no window
    // of ours is activated on that road, so what the watcher holds is the
    // origin of an earlier capture. Written onto such a note it would be a
    // window title from another moment — the invisible collection this switch
    // exists against, and no longer „the state at capture time" (SPEC 13,
    // acceptance criterion 4).
    OriginWatcher origins;
    QObject::connect(&origins, &OriginWatcher::originChanged, &capture, &CaptureWindow::setOrigin);
    QObject::connect(&origins, &OriginWatcher::originChanged, &recorder, &RecordingWindow::setOrigin);

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    LibraryWindow library(&store);

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    TrayIcon tray;
    // What the queue is waiting for, in the tooltip and without an error state
    // (SPEC 12, issue #23): a model that has not been fetched yet is a
    // precondition not yet met, so it stands beside the optional tools of
    // SPEC 2.5 rather than beside a transcription that failed.
    QObject::connect(&transcriber, &Transcriber::modelMissing, &tray, &TrayIcon::setMissingModel);
    QObject::connect(&tray, &TrayIcon::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&tray, &TrayIcon::recorderRequested, &recorder, &RecordingWindow::showRecorder);
    QObject::connect(&tray, &TrayIcon::libraryRequested, &library, &LibraryWindow::showLibrary);
    // The error path of the transcription reaches the user here and nowhere
    // else (SPEC 10 and 12, issue #24). Both edges ask the same question of the
    // database rather than each carrying its own answer: **the state has to
    // stand where the queue stands.** A success that simply cleared the state
    // would make the icon quiet again while another note lies given up on in
    // the queue, and only the next start of the service would bring it back.
    //
    // Asked on both signals and not on failed(): a first attempt is followed
    // by a second one, so pausedTranscribeJobCount() still answers 0 there, and
    // a state raised for it would clear itself a moment later.
    //
    // What travels is the **number** of such jobs and no longer the reason the
    // last one failed (issue #118): the one subtitle line has a second source
    // since the analysis run reports into it, and a reason is a sentence that
    // pushes the other half out. The reason goes to the log, which is where
    // SPEC 14 sorts it, and to the notification below.
    const auto showWhereTheQueueStands = [&store, &tray] {
        tray.setNotesWithoutTranscript(store.pausedTranscribeJobCount());
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
    Embedder embedder(&store, &provider);
    // The model of the vectors comes from the run that writes them and is not
    // read out of denkzettelrc a second time: two spellings would be two models,
    // and the clustering would find an empty corpus (Embedder::model()).
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    Suggester suggester(&store, &provider, embedder.model());
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    AnalysisScheduler analysis(&classifier, &embedder, &suggester);

    // The two roads that mean "a note is ready" under the trigger "at once"
    // (SPEC 7.2): a note that is written, and a voice note whose transcript has
    // just arrived — the second is nothing the store announces, its text comes
    // long after the row did. The scheduler links neither side and is handed
    // both from here.
    QObject::connect(&store, &Store::noteAdded, &analysis, &AnalysisScheduler::noteIsReady);
    QObject::connect(&transcriber, &Transcriber::transcribed, &analysis, &AnalysisScheduler::noteIsReady);

    // And here every value the settings dialog writes reaches the object that
    // is already running with the old one (SPEC 13). All of them in one place
    // and none of them here, because a connection missing from this file is a
    // whole feature missing and nothing can see it: main.cpp is linked by no
    // library, so no test set reaches it — the three connections of issue #119
    // taken out left `ctest` at 14/14 green. The function called here is built
    // into `denkzettelsettings`, and `settingstest` links that (issue #123).
    connectSettingsToRunningObjects(&transcriber, &origins, &provider, &embedder, &suggester, &analysis);

    QObject::connect(&tray, &TrayIcon::analysisRequested, &analysis, &AnalysisScheduler::analyzeNow);

    // The third road of SPEC 7.2, from the library's application menu
    // (issue #132) — the same call as the tray's above and as the bus method's
    // further down, so the three cannot drift into three behaviours. The way
    // back is what the entry needs to say which of the two states it is in.
    //
    // ponytail: **no test set reaches these two lines**, and the reason is the
    // link graph, counted for issue #120 and unchanged: not one target links
    // denkzettelui and denkzettelshell together, and main.cpp, where they meet,
    // is linked by no library at all. What guards the halves is that they are
    // reachable on their own — `librarytest` links denkzettelui **and**
    // denkzettelanalysis, so it builds the pair below by hand and presses the
    // entry, which is where the read-back on the scheduler lives. What stays
    // unguarded is these two lines themselves. The ceiling is the usual one: a
    // wrong *name* is a compile error here, a *missing* line is silent.
    QObject::connect(&library, &LibraryWindow::analysisRequested, &analysis,
                     &AnalysisScheduler::analyzeNow);
    QObject::connect(&analysis, &AnalysisScheduler::busyChanged, &library,
                     &LibraryWindow::setAnalysisBusy);

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
    // again (SPEC 7.2), and this line is the detail half of what SPEC 14 asks
    // to be reported of it. The quiet half stands below.
    //
    // The wording no longer says "without a category", and that is a
    // correction rather than a rephrasing: since issue #133 a note whose
    // *re*analysis failed twice can carry a category and its tags all along —
    // `unanalysedNotes()` asks nothing about the category, so paused() fires
    // for it too, while the count below deliberately leaves it out.
    //
    // Where it is read: `journalctl --user -t denkzetteld`. Qt's default handler
    // writes to the journal wherever stderr is not a terminal, so in the running
    // service the sentence never reaches a pipe (CLAUDE.md, finding 25).
    QObject::connect(&classifier, &Classifier::paused, &app, [](qint64 noteId, const QString &reason) {
        qWarning("Note %lld is given up on by the analysis: %s", noteId, qUtf8Printable(reason));
    });

    // And the quiet half of SPEC 7.2 and 14, which issue #118 is: the tooltip
    // says how many notes the run has given up on, beside — never instead of —
    // the transcription's own count above.
    //
    // **The number is `Store::categoryCounts().unclassified` and no query of
    // its own.** That count is what the row "Unclassified" of the library is
    // written with, and the story is that the two agree; two transcriptions of
    // one condition agree until somebody edits one of them, and nothing would
    // say which of the two the user is reading (CLAUDE.md, finding 48). Which
    // also means the set is **narrower** than the sentence of SPEC 7.2 read on
    // its own: a note carrying a category is not uneingeordnet whatever its
    // attempt counter says, and it has a way into the window through the row
    // of its own category.
    //
    // **No KNotification beside it**, unlike the transcription: SPEC 10 rules
    // notifications out for routine runs, and the analysis run is the routine
    // run — every 30 minutes by default. A note left unclassified is readable
    // in the library all the same; it is missing its category and its tags,
    // not its text (UX decision of 29.08.2026).
    const auto showWhatTheAnalysisGaveUpOn = [&store, &tray] {
        tray.setNotesWithoutCategory(store.categoryCounts().unclassified);
    };
    // Both edges, and for the same reason as with the transcription above: the
    // state has to stand where the **library** stands and not where the last
    // note ended, so a run that classifies the last given-up note takes the
    // tooltip back and a run that gives one up raises it.
    QObject::connect(&classifier, &Classifier::paused, &tray, showWhatTheAnalysisGaveUpOn);
    QObject::connect(&classifier, &Classifier::classified, &tray, showWhatTheAnalysisGaveUpOn);
    // The third road out of that set, and the one no analysis run announces:
    // deleting such a note, editing it, or taking the deletion back. The
    // library recounts on every reload and hands the number on, so the tooltip
    // follows the column it points at.
    QObject::connect(&library, &LibraryWindow::unclassifiedCountChanged, &tray,
                     &TrayIcon::setNotesWithoutCategory);
    // Once at start, because nothing emits for what was already given up on
    // before this process began — the state stands as long as its cause does,
    // a restart included (SPEC 14). The transcription's half gets the same
    // through Transcriber::start(), which emits paused() for such a job.
    showWhatTheAnalysisGaveUpOn();

    // The same for step 2: a note the backend refuses twice gets no vector and
    // is in no bundle from then on (SPEC 7.2, Embedder::paused).
    QObject::connect(&embedder, &Embedder::paused, &app, [](qint64 noteId, const QString &reason) {
        qWarning("Note %lld is left without an embedding: %s", noteId, qUtf8Printable(reason));
    });

    // And for step 3, where nothing is counted against a note: a cluster the
    // model could not name is taken up again by the next run, so the log is the
    // only place it is visible at all (SPEC 14, Suggester).
    QObject::connect(&suggester, &Suggester::failed, &app, [](const QString &reason) {
        qWarning("A bundle suggestion came to nothing: %s", qUtf8Printable(reason));
    });

    DaemonService daemon(&store);
    QObject::connect(&daemon, &DaemonService::captureRequested, &capture, &CaptureWindow::showCapture);
    QObject::connect(&daemon, &DaemonService::recorderRequested, &recorder, &RecordingWindow::showRecorder);
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
    QObject::connect(&shortcuts, &GlobalShortcuts::recorderRequested, &recorder, &RecordingWindow::showRecorder);

    // The settings are free-standing and belong to no window, so no parent is
    // handed over — the dialog finds the standing one itself or builds a new
    // one (SPEC 13, issue #16). The shortcuts are: the page "Shortcuts" writes
    // through the same two actions that are registered here, and a second pair
    // of its own would take the key press away from these on the way in and
    // switch them off again on the way out (issue #74).
    QObject::connect(&tray, &TrayIcon::configureRequested, &app, [&shortcuts, &modelDownload] {
        SettingsDialog::showSettings(&shortcuts, &modelDownload);
    });

    // The second route, from the library's application menu (the user's
    // decision of 29.08.2026). Deliberately the same call and not a second
    // one: showSettings() hands back the standing dialog, so whichever route is
    // taken there is never more than one.
    QObject::connect(&library, &LibraryWindow::configureRequested, &app, [&shortcuts, &modelDownload] {
        SettingsDialog::showSettings(&shortcuts, &modelDownload);
    });

    // One registration and one read-back per shortcut, because SPEC 2.4 says so
    // per shortcut: Meta+Shift+N repeats Meta+N's failure of 01.08.2026 unless
    // it is asked about separately, and a pair reported together would not say
    // which of the two never arrived.
    for (const GlobalShortcuts::Shortcut which :
         {GlobalShortcuts::Shortcut::Capture, GlobalShortcuts::Shortcut::Recorder}) {
        const QList<ShortcutOwner> conflicts = shortcuts.registerShortcut(which);
        if (firstRun && !conflicts.isEmpty()) {
            // The sequence that is really registered, not the one that was
            // asked for: autoloading hands back what the user set, and since
            // the settings page that is no longer necessarily the default.
            notifyShortcutConflict(GlobalShortcuts::assignedSequence(which), conflicts);
        }
    }

    // The empty library names the key that captures a thought, and it can only
    // learn it here: denkzettelui links neither KF6::GlobalAccel nor the shell,
    // so the text comes in from outside the way store, transcriber and
    // scheduler do (issue #120). After the loop, because the read-back above is
    // what makes assignedSequence() answer for a registration that arrived —
    // and it is the service that is asked, not the default, so a sequence the
    // user set in an earlier session stands in the window at once.
    //
    // Not in connectSettingsToRunningObjects(): the sender there is
    // Settings::self(), and a global shortcut lives in the shortcut service and
    // not in denkzettelrc (issue #123, settingswiring.h).
    library.setCaptureShortcut(GlobalShortcuts::assignedSequence(GlobalShortcuts::Shortcut::Capture));
    // And the second road to the same text: the settings page of SPEC 13
    // writes through changeSequence(), which reports what the service kept.
    // Without this the hint would name the old key until the daemon is
    // restarted — the fault of issue #120 one step further on.
    //
    // ponytail: **no test set reaches this route**, and both of its ends are
    // unguarded, each measured on its own (CLAUDE.md, finding 62). The emit in
    // GlobalShortcuts::changeSequence() deleted: build clean, ctest 14/14. The
    // connect below deleted: build clean, ctest 14/14 — the same output as the
    // working state, which is the first rule of the verification stance. The
    // reason is the link graph and it was counted: **not one target links
    // denkzettelui and denkzettelshell together** — five link the one
    // (librarytest, readmeshots, originshots, systemfontstest, searchbench),
    // three the other (shelltest, firstruntest, identitytest), none both — and
    // main.cpp, where they meet, is linked by no library at all (issue #123,
    // settingswiring.h names the same ceiling for its own call site).
    //
    // What *is* guarded is the spelling, and only in two steps: renaming the
    // signal's declaration alone stops the compiler one file earlier, in
    // globalshortcuts.cpp, and main.cpp is then never translated. Renamed in
    // the header **and** at the emit, it stops here — `sequenceChanged is not
    // a member of GlobalShortcuts`, main.cpp:435 (finding 48).
    //
    // The ceiling is therefore: a wrong *name* is a compile error, a *missing*
    // line is silent. The way up is the other reading of issue #123 — a run
    // that starts the daemon, changes the shortcut and reads the arrival off
    // the window; short of that, a test set linking both libraries would at
    // least reach signal, connect and label together.
    QObject::connect(&shortcuts,
                     &GlobalShortcuts::sequenceChanged,
                     &library,
                     [&library](GlobalShortcuts::Shortcut which, const QKeySequence &held) {
                         if (which == GlobalShortcuts::Shortcut::Capture) {
                             library.setCaptureShortcut(held);
                         }
                     });

    // After the object is connected to both windows, and it needs the bus:
    // exporting /Origin and asking KWin for the script are the two things it
    // does here.
    origins.start();

    // Last, and after the first start above: the queue may hold a job from a
    // run that was killed, and working it off is the same road as a fresh one.
    transcriber.start();
    // Everything given up on from here on happened while the user was watching,
    // and only that is worth a notification (issue #115).
    announceGivingUp = true;

    return app.exec();
}
