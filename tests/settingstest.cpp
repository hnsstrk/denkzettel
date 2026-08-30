#include "analysis/analysisscheduler.h"
#include "analysis/classifier.h"
#include "analysis/embedder.h"
#include "analysis/ollamaprovider.h"
#include "analysis/suggester.h"
#include "settings/settings.h"
#include "settings/settingsdialog.h"
#include "settings/settingswiring.h"
#include "shell/globalshortcuts.h"
#include "shell/originwatcher.h"
#include "store/store.h"
#include "transcribe/modeldownload.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QAbstractItemModel>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QGroupBox>
#include <QIcon>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QRadioButton>
#include <QScopeGuard>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStyle>
#include <QTemporaryDir>
#include "transcribe/transcriber.h"
#include <KSharedConfig>
#include <QFileInfo>
#include <QLabel>
#include <QSignalSpy>
#include <QTest>

/**
 * What the settings dialog does where nobody can see it (SPEC 13, issue #16).
 *
 * Three things break without a sound, and only those are asserted here — how
 * the dialog looks is looked at, not measured:
 *
 * 1. **Whether Apply writes.** KConfigDialog wires its buttons up in its own
 *    constructor; whoever replaces the button set afterwards gets an Apply
 *    button that looks right and writes nothing. Read back through a KConfig
 *    of its own rather than through the skeleton the dialog just filled — a
 *    readback from the object that did the writing would only measure its
 *    memory.
 * 2. **Whether Help is only hidden.** The one line that hides it is what an
 *    unwitting hand replaces with `setStandardButtons()`, which is the fault
 *    above.
 * 3. **The icon names of the page list.** `KPageDialog::List` keeps the height
 *    of an icon free whether one is there or not, so a page without one stands
 *    among holes. What is binding is the name, so the name is what is read.
 * 4. **Whether a refused vault folder stays out of the file** (issue #75). The
 *    red line under the field is looked at; that the value behind it did not
 *    reach denkzettelrc is not visible anywhere and is asserted here.
 * 5. **Whether anybody is listening to what was saved** (issue #123). The
 *    announcement is only half the road: without the connection at the other
 *    end the dialog writes, the page confirms, and the running daemon goes on
 *    with the old value. The connections live in
 *    `settings/settingswiring.cpp`, which this set links through
 *    `denkzettelsettings` — that is what makes this a guard rather than an
 *    assertion (CLAUDE.md, finding 48).
 * 6. **Whether a stored provider survives its disabled button** (issue #127).
 *    That two of the three buttons are grey is looked at; that the choice
 *    already standing in denkzettelrc comes back out of it unchanged is not
 *    visible anywhere — a value quietly rewritten to Ollama would show up as a
 *    different button checked at some later opening and nowhere else.
 */
class SettingsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void settingsSurviveARestart();
    void aStoredProviderSurvivesItsDisabledButton();
    void theWindowSizeSurvivesEveryWayOut();
    void aRefusedVaultFolderIsNotStored();
    void theDefaultsComeBack();
    void afterTheDefaultsARefusalGoesBackToNothing();
    void aRejectedProgramPathIsNotStored();
    void savingAnnouncesItself();
    void everySettingReachesItsRunningObject();
    void reportsAModelPathItCouldNotTakeOver();
    void restoringTheDefaultsResetsThePathField();
    void helpIsHiddenAndNotReplaced();
    void everyPageCarriesAnIcon();

private:
    static SettingsDialog *openDialog();
    static void closeDialog(SettingsDialog *dialog);
    static void editFolder(QLineEdit *field, const QString &folder);
};

void SettingsTest::initTestCase()
{
    // Every visible string of the two pages goes through i18n(), and without
    // the domain each of them warns before handing the msgid back — 102 of
    // them, which is what the CI's string check counts. The product sets it in
    // main.cpp; a test binary is its own application and has to set it too.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Without a resolvable platform theme QIcon::fromTheme() hands back icons
    // whose name is empty, and the third case would then pass on nothing. The
    // variable is set in tests/CMakeLists.txt; what proves it arrived is the
    // style read back here, not the variable being set.
    QCOMPARE(QApplication::style()->objectName(), QStringLiteral("breeze"));

    // The run starts from nothing written. A denkzettelrc left over from the
    // previous run would turn the change in settingsSurviveARestart() into a
    // change of nothing: Apply stays grey and the case fails for a reason that
    // has nothing to do with the code — measured on 29.08.2026, where the very
    // same binary was green on the first run and red on the second.
    //
    // The guard below is not decoration. Without XDG_CONFIG_HOME pointing into
    // the build directory (tests/CMakeLists.txt) this line would delete the
    // denkzettelrc of whoever runs the check.
    // The name of the daemon, and the file follows from it: with no file name
    // of its own KCoreConfigSkeleton takes KSharedConfig::openConfig(), which
    // builds its name out of QCoreApplication::applicationName() + "rc". QTest
    // fills that in from the binary, so without this line the dialog writes a
    // settingstestrc while every readback below looks at a denkzettelrc nobody
    // ever wrote — measured on 29.08.2026, where the case failed on a program
    // that was doing exactly the right thing.
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    const QByteArray configHome = qgetenv("XDG_CONFIG_HOME");
    QVERIFY2(!configHome.isEmpty(), "XDG_CONFIG_HOME has to point into the build directory");
    const QString file = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QStringLiteral("/denkzettelrc");
    QVERIFY(file.startsWith(QString::fromLocal8Bit(configHome)));
    QFile::remove(file);
}

SettingsDialog *SettingsTest::openDialog()
{
    // The two actions the page "Shortcuts" writes through. They are built and
    // never registered here: registering would talk to the shortcut service of
    // whoever runs the check, and nothing below touches a key sequence field,
    // so nothing is written either.
    static GlobalShortcuts shortcuts;
    // The daemon's model download, which the page "Voice notes" shows the
    // progress of (issue #23). Nothing here starts one, so nothing here
    // reaches the network.
    static ModelDownload download;

    SettingsDialog::showSettings(&shortcuts, &download);
    return qobject_cast<SettingsDialog *>(KConfigDialog::exists(QStringLiteral("settings")));
}

void SettingsTest::closeDialog(SettingsDialog *dialog)
{
    dialog->close();
    // Qt::WA_DeleteOnClose posts a deferred deletion, and KConfigDialog only
    // gives its name back in the destructor. Without this the next case would
    // be handed the closed dialog instead of a fresh one.
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

void SettingsTest::editFolder(QLineEdit *field, const QString &folder)
{
    // The user's order, and it is load-bearing: into the field first, then the
    // text, then away from it. The page remembers what stood in the field when
    // it was entered, so a text set before the focus would hand it the very
    // value under test and no refusal could send anything back.
    //
    // Leaving the field is what makes a QLineEdit report that its editing is
    // finished — not QTest::keyClick(Qt::Key_Return): QLineEdit ignores that
    // key so the dialog's default button can have it, and the click would
    // close the whole dialog under the assertions below (measured 29.08.2026).
    field->setFocus();
    field->setText(folder);
    field->clearFocus();
}

void SettingsTest::settingsSurviveARestart()
{
    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    auto *chatModel = dialog->findChild<QComboBox *>(QStringLiteral("kcfg_ChatModel"));
    auto *address = dialog->findChild<QLineEdit *>(QStringLiteral("kcfg_OllamaUrl"));
    // The provider is a group box of auto-exclusive buttons, and what
    // KConfigDialogManager stores from it is the index of the checked one.
    const auto *provider = dialog->findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
    QVERIFY(chatModel);
    QVERIFY(address);
    QVERIFY(provider);

    QPushButton *apply = dialog->button(QDialogButtonBox::Apply);
    QVERIFY(apply);
    // Grey until something changes, and that is the control which gives the
    // click below its meaning: a button enabled from the start would not say
    // whether the manager ever noticed the change.
    QVERIFY(!apply->isEnabled());

    chatModel->setCurrentText(QStringLiteral("llama3:70b"));
    address->setText(QStringLiteral("http://127.0.0.1:11500"));
    provider->findChildren<QRadioButton *>().at(Settings::OpenAi)->setChecked(true);
    QVERIFY(apply->isEnabled());

    // Asked before the click so that the readback afterwards has something to
    // come out differently from.
    {
        KConfig before(QStringLiteral("denkzettelrc"));
        QVERIFY(before.group(QStringLiteral("AI")).readEntry("ChatModel", QString())
                != QStringLiteral("llama3:70b"));
    }

    apply->click();

    KConfig written(QStringLiteral("denkzettelrc"));
    const KConfigGroup ai = written.group(QStringLiteral("AI"));
    QCOMPARE(ai.readEntry("ChatModel", QString()), QStringLiteral("llama3:70b"));
    QCOMPARE(ai.readEntry("OllamaUrl", QString()), QStringLiteral("http://127.0.0.1:11500"));
    // The enum reaches the file as its name and not as the number 2 — that is
    // what keeps a hand-edited denkzettelrc readable, and what the list of
    // choices in Settings is for.
    QCOMPARE(ai.readEntry("Provider", QString()), QStringLiteral("OpenAI"));

    // And Apply goes grey again, because what stands on the form is now what
    // stands in the file.
    QVERIFY(!apply->isEnabled());

    closeDialog(dialog);
}

void SettingsTest::aStoredProviderSurvivesItsDisabledButton()
{
    // openrouter.ai and OpenAI are not selectable until #38 and #39 have built
    // their clients (issue #127). A configuration that already names one of
    // them — the customer's does, that is how the defect was found — has to
    // arrive on the page all the same and go back into the file unchanged.
    // **This is the half nobody can see**: were the greyed button dropped on
    // the way in, opening the settings once and pressing OK would quietly
    // write the choice back as Ollama, and the only sign of it would be a
    // different button checked at some later opening.
    //
    // Everything visible about the change — that two buttons are grey and that
    // a sentence stands under them — is looked at in the picture, not asserted
    // here (CLAUDE.md, "What gets verified").
    const QByteArray configHome = qgetenv("XDG_CONFIG_HOME");
    QVERIFY2(!configHome.isEmpty(), "XDG_CONFIG_HOME has to point into the build directory");
    QVERIFY(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                .startsWith(QString::fromLocal8Bit(configHome)));

    // The key goes away whichever way this case ends. It leaks into every case
    // below otherwise, and a QCOMPARE that fails skips every tidy-up line
    // written after it (CLAUDE.md, finding 42).
    const auto tidy = qScopeGuard([] {
        KConfig back(QStringLiteral("denkzettelrc"));
        back.group(QStringLiteral("AI")).deleteEntry(QStringLiteral("Provider"));
        back.sync();
        Settings::self()->load();
    });

    struct Stored {
        int provider;
        const char *name;
    };
    // All three, because "the value comes back" only says something where it
    // comes back **differently** at least once (finding 10) — and Ollama is
    // the one the whole page falls back to if the greyed buttons are dropped.
    const QList<Stored> cases{{Settings::Ollama, "Ollama"},
                              {Settings::OpenRouter, "OpenRouter"},
                              {Settings::OpenAi, "OpenAI"}};

    for (const Stored &stored : cases) {
        {
            KConfig prefilled(QStringLiteral("denkzettelrc"));
            prefilled.group(QStringLiteral("AI"))
                .writeEntry("Provider", QString::fromLatin1(stored.name));
            prefilled.sync();
        }
        // The skeleton is a singleton and read the file once, at the first
        // opening of the dialog. A daemon started with this configuration
        // would read it here; without this line the page would be handed
        // whatever the previous case left in memory.
        Settings::self()->load();

        SettingsDialog *dialog = openDialog();
        QVERIFY(dialog);
        QVERIFY(QTest::qWaitForWindowExposed(dialog));

        const auto *provider = dialog->findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
        QVERIFY(provider);
        const QList<QRadioButton *> buttons = provider->findChildren<QRadioButton *>();
        QCOMPARE(buttons.size(), 3);

        // Breeze animates the dot (finding 43). The checked state itself does
        // not wait for the animation, but the picture does, and this case and
        // the picture runner read the same moment on purpose.
        QTest::qWait(400);

        for (int index = 0; index < buttons.size(); ++index) {
            QCOMPARE(buttons.at(index)->isChecked(), index == stored.provider);
            QCOMPARE(buttons.at(index)->isEnabled(), index == Settings::Ollama);
        }

        // **The wording, because the index cannot see itself.** The stored
        // value is the position of the checked button, so both sides of the
        // round trip above are that same position and a reordering of the
        // three shifts them together — finding 10's case exactly. Measured
        // 30.08.2026 in review: with `openrouter` and `openai` created the
        // other way round all fourteen sets stayed green while a stored
        // OpenRouter checked the button reading "OpenAI (API key)". The label
        // is the one value here that is set from outside, and it is what makes
        // the reordering visible. This set runs with LANGUAGE=en_US
        // (tests/CMakeLists.txt), so the English wording is the stable one.
        QCOMPARE(buttons.at(Settings::Ollama)->text(), QStringLiteral("Ollama"));
        QCOMPARE(buttons.at(Settings::OpenRouter)->text(), QStringLiteral("openrouter.ai"));
        QCOMPARE(buttons.at(Settings::OpenAi)->text(), QStringLiteral("OpenAI (API key)"));

        // **The API key row is away for all three stored values**, measured
        // 30.08.2026 while writing this case, and the second reason for it was
        // a surprise: the row hangs on the Ollama button's `toggled` signal,
        // and a freshly opened dialog whose stored value is *not* Ollama never
        // emits it — no button is checked before the manager reads the
        // setting, so checking button 1 or 2 toggles nothing. The row
        // therefore never came up on an opening, only on a click, and with the
        // two buttons disabled there are no more clicks. Nobody can type a key
        // into the field that loses it (the UX decision of 30.08.2026), and
        // for a reason one line stronger than that decision assumed.
        //
        // `isHidden()` and not `isVisible()`: the AI page is not the dialog's
        // current one, so every widget on it is invisible whatever the row
        // does — that readback would answer the same in every state and carry
        // nothing. The control that makes this one carry stands after the
        // loop.
        const auto *apiKey = dialog->findChild<QLineEdit *>(QStringLiteral("apiKey"));
        QVERIFY(apiKey);
        QVERIFY(apiKey->isHidden());

        // OK and not Apply: Apply is grey, because the form holds exactly what
        // the file holds — and OK writes the same way (the reasoning of
        // aRefusedVaultFolderIsNotStored, one page further on).
        dialog->button(QDialogButtonBox::Ok)->click();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        // Read with "Ollama" standing in for a missing key: a skeleton deletes
        // an entry whose value equals the default instead of writing it, so
        // the Ollama case legitimately leaves no line behind. What the two
        // others must not come back as is precisely that fallback.
        KConfig written(QStringLiteral("denkzettelrc"));
        QCOMPARE(written.group(QStringLiteral("AI")).readEntry("Provider", QStringLiteral("Ollama")),
                 QString::fromLatin1(stored.name));
    }

    // The control: hidden three times over says nothing unless the same
    // readback can come out the other way on the same build. Checked by hand —
    // the road a click would take if the button still took clicks — the row
    // does appear, so the assertions above are about the state and not about a
    // row that was never built. Closed and not OK'd: this flip must not reach
    // the file.
    //
    // It starts from Ollama, and that is the whole reason the flip is visible
    // at all: the row hangs on **this** button's signal, so it only moves when
    // this button's state moves.
    {
        KConfig prefilled(QStringLiteral("denkzettelrc"));
        prefilled.group(QStringLiteral("AI")).writeEntry("Provider", QStringLiteral("Ollama"));
        prefilled.sync();
    }
    Settings::self()->load();

    SettingsDialog *control = openDialog();
    QVERIFY(control);
    QVERIFY(QTest::qWaitForWindowExposed(control));
    const auto *box = control->findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
    QVERIFY(box);
    const auto *field = control->findChild<QLineEdit *>(QStringLiteral("apiKey"));
    QVERIFY(field);
    QVERIFY(field->isHidden());
    box->findChildren<QRadioButton *>().at(Settings::OpenRouter)->setChecked(true);
    QVERIFY(!field->isHidden());
    closeDialog(control);
}

void SettingsTest::theWindowSizeSurvivesEveryWayOut()
{
    // Both ways out in one run, with different sizes, because they take
    // different roads: OK ends in QDialog::done() and sends no QCloseEvent at
    // all, while the window manager's close button does. A dialog that writes
    // its size in closeEvent() therefore forgets it on OK — and nothing says
    // so; the next opening simply stands at the built-in size again.
    SettingsDialog *viaOk = openDialog();
    QVERIFY(viaOk);
    QVERIFY(QTest::qWaitForWindowExposed(viaOk));
    viaOk->resize(900, 700);
    viaOk->button(QDialogButtonBox::Ok)->click();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    SettingsDialog *afterOk = openDialog();
    QVERIFY(QTest::qWaitForWindowExposed(afterOk));
    QCOMPARE(afterOk->size(), QSize(900, 700));

    // And the other road, with a size the first one cannot have left behind.
    afterOk->resize(800, 600);
    closeDialog(afterOk);

    SettingsDialog *afterClose = openDialog();
    QVERIFY(QTest::qWaitForWindowExposed(afterClose));
    QCOMPARE(afterClose->size(), QSize(800, 600));

    closeDialog(afterClose);
}

void SettingsTest::aRefusedVaultFolderIsNotStored()
{
    // The one thing on the export page that breaks without a sound (issue
    // #75): a folder that cannot be written to is reported when it is SET, and
    // the refused value must not push the stored one out of denkzettelrc. What
    // the display does is looked at, what the file holds is asserted here.
    const QTemporaryDir vault;
    QVERIFY(vault.isValid());

    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    auto *path = dialog->findChild<QLineEdit *>(QStringLiteral("kcfg_VaultPath"));
    QVERIFY(path);
    QPushButton *apply = dialog->button(QDialogButtonBox::Apply);
    QVERIFY(apply);

    // A folder that is there gets through — the half of the run that has to
    // come out differently, without which "nothing was stored" would say
    // nothing about the check.
    editFolder(path, vault.path());
    QCOMPARE(path->text(), vault.path());
    QVERIFY(apply->isEnabled());
    apply->click();

    {
        KConfig written(QStringLiteral("denkzettelrc"));
        QCOMPARE(written.group(QStringLiteral("Export")).readEntry("VaultPath", QString()), vault.path());
    }

    // And one that is not there does not. Not a folder with the write bit
    // taken away: isWritable() asks the effective user, and for root — which
    // is what the CI container runs as — everything is writable, so that case
    // would stand green over a program that stores the refused folder.
    const QString missing = vault.path() + QStringLiteral("/nirgends");
    editFolder(path, missing);
    QCOMPARE(path->text(), vault.path());
    // Nothing left to apply, because the form holds what the file holds.
    QVERIFY(!apply->isEnabled());

    // OK and not Apply: Apply is grey now, so a click on it would prove
    // nothing about what a write does — OK is never grey and writes just the
    // same, which gives the refused folder a real road into the file.
    dialog->button(QDialogButtonBox::Ok)->click();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    KConfig written(QStringLiteral("denkzettelrc"));
    QCOMPARE(written.group(QStringLiteral("Export")).readEntry("VaultPath", QString()), vault.path());
}

void SettingsTest::theDefaultsComeBack()
{
    // KConfigDialog brings the button, but whether it reaches a page's fields
    // depends on the `kcfg_` names being right — and a wrong name is exactly
    // what the previous case cannot see for the three spin boxes.
    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    auto *notes = dialog->findChild<QSpinBox *>(QStringLiteral("kcfg_OverflowNotes"));
    auto *days = dialog->findChild<QSpinBox *>(QStringLiteral("kcfg_OverflowDays"));
    auto *bundle = dialog->findChild<QSpinBox *>(QStringLiteral("kcfg_BundleNotes"));
    QVERIFY(notes);
    QVERIFY(days);
    QVERIFY(bundle);

    notes->setValue(17);
    days->setValue(5);
    bundle->setValue(9);

    dialog->button(QDialogButtonBox::RestoreDefaults)->click();

    // SPEC 11 names the first two, SPEC 7.3 the third.
    QCOMPARE(notes->value(), 200);
    QCOMPARE(days->value(), 30);
    QCOMPARE(bundle->value(), 3);

    closeDialog(dialog);
}

void SettingsTest::afterTheDefaultsARefusalGoesBackToNothing()
{
    // "Defaults" empties the folder field, and a folder refused afterwards must
    // go back to that emptiness — not to the folder from before the reset,
    // which the user has just thrown away (review of issue #75).
    const QTemporaryDir vault;
    QVERIFY(vault.isValid());

    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    auto *path = dialog->findChild<QLineEdit *>(QStringLiteral("kcfg_VaultPath"));
    QVERIFY(path);

    editFolder(path, vault.path());
    QCOMPARE(path->text(), vault.path());

    dialog->button(QDialogButtonBox::RestoreDefaults)->click();
    QCOMPARE(path->text(), QString());

    editFolder(path, vault.path() + QStringLiteral("/nirgends"));
    QCOMPARE(path->text(), QString());

    closeDialog(dialog);
}

void SettingsTest::aRejectedProgramPathIsNotStored()
{
    // Files of this run's own, and never a lookup on PATH: `whisper-cli` is
    // installed on the machine this was written on and is not on the automated
    // run, so a case built on it would be measuring the machine (finding 33 of
    // CLAUDE.md in its other shape).
    const QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString executable = dir.filePath(QStringLiteral("whisper-cli"));
    QFile program(executable);
    QVERIFY(program.open(QIODevice::WriteOnly));
    program.write("#!/bin/sh\nexit 0\n");
    program.close();
    QVERIFY(program.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));

    // There, a file, readable — and not executable. That is the one case
    // QFile::exists() waves through, and the whole of this check.
    const QString rejected = dir.filePath(QStringLiteral("whisper-cli.txt"));
    QFile plain(rejected);
    QVERIFY(plain.open(QIODevice::WriteOnly));
    plain.write("no program\n");
    plain.close();
    QVERIFY(plain.setPermissions(QFile::ReadOwner | QFile::WriteOwner));
    // And it stays rejected as uid 0, which is what lets this case stand in a
    // set the CI runs as root: on Linux X_OK is granted to root only when at
    // least one execute bit is set, so a file at 0644 is not executable for
    // anybody. Measured 29.08.2026 — the whole set run under `unshare -r` as
    // uid 0 came out 8 passed, 0 failed, while a file at 0000 *is* writable
    // there. That is the difference to finding 46 of CLAUDE.md, which is about
    // isWritable(). The line stays as the readback that says so.
    QVERIFY(!QFileInfo(rejected).isExecutable());

    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    auto *editor = dialog->findChild<QLineEdit *>(QStringLiteral("whisperProgram"));
    const auto *stored = dialog->findChild<QLineEdit *>(QStringLiteral("kcfg_WhisperProgram"));
    const auto *message = dialog->findChild<QLabel *>(QStringLiteral("whisperProgramState"));
    QVERIFY(editor);
    QVERIFY(stored);
    QVERIFY(message);
    QPushButton *apply = dialog->button(QDialogButtonBox::Apply);
    QVERIFY(apply);

    // The accepted state first, or the rejection below has nothing to come out
    // differently from (finding 27): a path that IS a program travels into the
    // stored field, wakes the Apply button and reaches the file.
    editor->setText(executable);
    QCOMPARE(stored->text(), executable);
    QVERIFY(message->text().isEmpty());
    QVERIFY(apply->isEnabled());
    apply->click();
    {
        KConfig written(QStringLiteral("denkzettelrc"));
        QCOMPARE(written.group(QStringLiteral("Transcription")).readEntry("WhisperProgram", QString()),
                 executable);
    }

    // And the rejected one. It stands in the editor, it is reported, and it
    // reaches neither the stored field nor the file — not even over a click on
    // Apply, which has nothing to write and stays grey.
    editor->setText(rejected);
    QCOMPARE(editor->text(), rejected);
    QCOMPARE(stored->text(), executable);
    QVERIFY(!message->text().isEmpty());
    QVERIFY(!apply->isEnabled());
    apply->click();

    KConfig after(QStringLiteral("denkzettelrc"));
    QCOMPARE(after.group(QStringLiteral("Transcription")).readEntry("WhisperProgram", QString()),
             executable);

    closeDialog(dialog);
}

void SettingsTest::savingAnnouncesItself()
{
    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    auto *size = dialog->findChild<QComboBox *>(QStringLiteral("kcfg_ModelSize"));
    QVERIFY(size);
    // All five of SPEC 12, whether their model lies on disk or not (UX,
    // 29.08.2026) — what is missing is greyed, not left out.
    QCOMPARE(size->count(), static_cast<int>(whisper::Sizes.size()));

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy announced(Settings::self(), &Settings::configChanged);
    QPushButton *apply = dialog->button(QDialogButtonBox::Apply);
    QVERIFY(apply);
    QVERIFY(!apply->isEnabled());

    // `medium`: neither the first entry, nor the last, nor the default. The
    // first index is what a broken store lands on by itself — measured
    // 29.08.2026, with the widget property left at the USER default the
    // manager writes the entry TEXT into an int item, "tiny — not
    // downloaded".toInt() is 0, and a case built on index 0 stands green over
    // it (finding 34).
    size->setCurrentIndex(3);
    QVERIFY(apply->isEnabled());
    QCOMPARE(announced.count(), 0);
    apply->click();

    // The announcement the running transcriber of main.cpp hangs on. The
    // transition and not a count: what the dialog runs through on one Apply is
    // its own business, and asking for a number would make this case red over
    // a KConfigDialog that saves twice.
    QVERIFY(announced.count() > 0);

    KConfig written(QStringLiteral("denkzettelrc"));
    QCOMPARE(written.group(QStringLiteral("Transcription")).readEntry("ModelSize", QString()),
             QStringLiteral("medium"));

    closeDialog(dialog);
}

void SettingsTest::everySettingReachesItsRunningObject()
{
    // The other half of the case above (issue #123). That one shows the
    // announcement leaving the skeleton; this one shows that somebody is
    // listening to it — and until now nobody could check that, because the
    // connections stood in main.cpp, which no library links and therefore no
    // test set reaches. Measured on 29.08.2026: the three connections of issue
    // #119 deleted, that fault fully back, `ctest` **14/14 green** — the same
    // output as the unchanged state.
    //
    // **What is read back is the connection, not what the slot does with the
    // value.** QObject::disconnect() looks a connection up by sender, signal,
    // receiver and the exact member function, and hands back whether there was
    // one — so a line deleted from settingswiring.cpp comes out below by name.
    // What each slot then makes of `denkzettelrc` is asserted where that slot
    // lives; three of these seven have nothing here to read an answer off at
    // all — Transcriber::start does nothing without a job in the queue,
    // OriginWatcher wants a running KWin, and Suggester keeps its model to
    // itself — and this is the one assertion that reaches all seven alike.
    const QTemporaryDir data;
    QVERIFY(data.isValid());
    Store store(data.filePath(QStringLiteral("notes.db")));
    QVERIFY(store.open());

    Transcriber transcriber(&store);
    OriginWatcher origins;
    OllamaProvider provider;
    Classifier classifier(&store, &provider);
    Embedder embedder(&store, &provider);
    Suggester suggester(&store, &provider, embedder.model());
    AnalysisScheduler analysis(&classifier, &embedder, &suggester);

    const Settings *settings = Settings::self();
    // Nothing is attached before the call — the control without which "all
    // seven are there" would not say who put them there. What it rules out is
    // a receiver that connects itself in its own constructor; none of the six
    // does today, so the control holds for all seven. (An earlier case of this
    // run cannot leave anything behind here: every receiver below is built in
    // this function and is gone when it ends.)
    QVERIFY(!QObject::disconnect(settings, &Settings::configChanged, &transcriber, &Transcriber::reloadSettings));

    connectSettingsToRunningObjects(&transcriber, &origins, &provider, &embedder, &suggester, &analysis);

    // Collected instead of asserted one by one: QCOMPARE ends the test function
    // at the first failure, so a check per line would name the first missing
    // connection and say nothing about the six behind it (CLAUDE.md, finding
    // 35). Gathered this way the message names every one of them.
    QStringList missing;
    const auto reaches = [&missing](const char *name, bool connected) {
        if (!connected) {
            missing.append(QString::fromLatin1(name));
        }
    };
    reaches("Transcriber::reloadSettings",
            QObject::disconnect(settings, &Settings::configChanged, &transcriber, &Transcriber::reloadSettings));
    reaches("Transcriber::start",
            QObject::disconnect(settings, &Settings::configChanged, &transcriber, &Transcriber::start));
    reaches("OriginWatcher::reloadSettings",
            QObject::disconnect(settings, &Settings::configChanged, &origins, &OriginWatcher::reloadSettings));
    reaches("AnalysisScheduler::applySettings",
            QObject::disconnect(settings, &Settings::configChanged, &analysis, &AnalysisScheduler::applySettings));
    reaches("OllamaProvider::reloadSettings",
            QObject::disconnect(settings, &Settings::configChanged, &provider, &OllamaProvider::reloadSettings));
    reaches("Embedder::reloadSettings",
            QObject::disconnect(settings, &Settings::configChanged, &embedder, &Embedder::reloadSettings));
    reaches("Suggester::reloadSettings",
            QObject::disconnect(settings, &Settings::configChanged, &suggester, &Suggester::reloadSettings));

    QCOMPARE(missing.join(QStringLiteral(", ")), QString());
}

void SettingsTest::reportsAModelPathItCouldNotTakeOver()
{
    // What migrateModelPath() leaves standing: a path that names no size of
    // ours. Written before the dialog is opened, because the page reads the
    // key while it is built — there is nowhere else to read that state from.
    const QString earlier = QStringLiteral("/opt/whisper/one-of-my-own.bin");
    {
        KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
        group.writeEntry("ModelPath", earlier);
        group.sync();
    }

    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    const auto *message = dialog->findChild<QLabel *>(QStringLiteral("modelState"));
    QVERIFY(message);
    // The old path is in the sentence: it is what the user set, and what they
    // need if they want it back.
    QVERIFY2(message->text().contains(earlier), qPrintable(message->text()));

    QPushButton *apply = dialog->button(QDialogButtonBox::Apply);
    auto *size = dialog->findChild<QComboBox *>(QStringLiteral("kcfg_ModelSize"));
    QVERIFY(size);
    size->setCurrentIndex(1);
    QVERIFY(apply->isEnabled());
    apply->click();

    // Gone from the file, and gone from the page with it. After this click
    // `ModelSize` stands in the file in plain sight, so the old path could
    // never take effect again whatever happened to it.
    const KConfigGroup after(KSharedConfig::openConfig(), QStringLiteral("Transcription"));
    QVERIFY(!after.hasKey("ModelPath"));
    QVERIFY(!message->text().contains(earlier));

    closeDialog(dialog);
}

void SettingsTest::restoringTheDefaultsResetsThePathField()
{
    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    auto *editor = dialog->findChild<QLineEdit *>(QStringLiteral("whisperProgram"));
    const auto *stored = dialog->findChild<QLineEdit *>(QStringLiteral("kcfg_WhisperProgram"));
    QPushButton *defaults = dialog->button(QDialogButtonBox::RestoreDefaults);
    QVERIFY(editor);
    QVERIFY(stored);
    QVERIFY(defaults);

    // Twice, and the second click is the whole case: after the first one the
    // stored field already holds the default, so the second sets it to what it
    // is — no change, no signal, and a field left to itself would keep the
    // rejected path below standing over it.
    defaults->click();
    const QString fallback = stored->text();
    QVERIFY(!fallback.isEmpty());

    editor->setText(QStringLiteral("/nowhere/at/all"));
    QCOMPARE(stored->text(), fallback);

    defaults->click();
    QCOMPARE(stored->text(), fallback);
    QCOMPARE(editor->text(), fallback);

    closeDialog(dialog);
}

void SettingsTest::helpIsHiddenAndNotReplaced()
{
    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    // The set KConfigDialog puts up by itself, all five of it. Whoever
    // exchanges it loses the wiring that makes Apply write, so the presence of
    // the other four is as much the object of this check as the hidden fifth.
    QVERIFY(dialog->button(QDialogButtonBox::Ok));
    QVERIFY(dialog->button(QDialogButtonBox::Apply));
    QVERIFY(dialog->button(QDialogButtonBox::Cancel));
    QVERIFY(dialog->button(QDialogButtonBox::RestoreDefaults));

    const QPushButton *help = dialog->button(QDialogButtonBox::Help);
    QVERIFY(help);
    QVERIFY(help->isHidden());

    closeDialog(dialog);
}

void SettingsTest::everyPageCarriesAnIcon()
{
    SettingsDialog *dialog = openDialog();
    QVERIFY(dialog);
    QVERIFY(QTest::qWaitForWindowExposed(dialog));

    const auto *list = dialog->findChild<QListView *>();
    QVERIFY(list);
    const QAbstractItemModel *pages = list->model();
    QVERIFY(pages);
    const QStringList expected{QStringLiteral("document-edit"),
                               QStringLiteral("preferences-system-network-server"),
                               QStringLiteral("preferences-system-time"),
                               QStringLiteral("document-export"),
                               QStringLiteral("audio-input-microphone"),
                               QStringLiteral("preferences-desktop-keyboard-shortcut")};
    QCOMPARE(pages->rowCount(), expected.size());

    for (int row = 0; row < expected.size(); ++row) {
        const QIcon icon = pages->data(pages->index(row, 0), Qt::DecorationRole).value<QIcon>();
        QCOMPARE(icon.name(), expected.at(row));
    }

    closeDialog(dialog);
}

QTEST_MAIN(SettingsTest)

#include "settingstest.moc"
