#include "settings/settings.h"
#include "settings/settingsdialog.h"

#include <KConfig>
#include <KConfigGroup>

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
#include <QStandardPaths>
#include <QStyle>
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
 */
class SettingsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void settingsSurviveARestart();
    void theWindowSizeSurvivesEveryWayOut();
    void helpIsHiddenAndNotReplaced();
    void everyPageCarriesAnIcon();

private:
    static SettingsDialog *openDialog();
    static void closeDialog(SettingsDialog *dialog);
};

void SettingsTest::initTestCase()
{
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
    SettingsDialog::showSettings();
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
    QCOMPARE(pages->rowCount(), 2);

    const QStringList expected{QStringLiteral("preferences-system-network-server"),
                               QStringLiteral("preferences-system-time")};
    for (int row = 0; row < expected.size(); ++row) {
        const QIcon icon = pages->data(pages->index(row, 0), Qt::DecorationRole).value<QIcon>();
        QCOMPARE(icon.name(), expected.at(row));
    }

    closeDialog(dialog);
}

QTEST_MAIN(SettingsTest)

#include "settingstest.moc"
