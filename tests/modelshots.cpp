#include "settings/voicenotespage.h"
#include "transcribe/modeldownload.h"
#include "transcribe/transcriber.h"

#include <KColorScheme>
#include <KLocalizedString>

#include <QApplication>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

/**
 * The pictures of issue #23: the page "Voice notes" while a model is really
 * being fetched, the question that is asked before it, and what a cancelled
 * download leaves standing.
 *
 * Not a test and out of `add_test()` for a harder reason than readmeshots has:
 * it talks to the internet, and a build server neither can nor should.
 *
 * **It fetches the smallest model there is (`tiny`, 74 MB) into a throwaway
 * data directory** — twice, because the first run is cancelled on purpose —
 * and takes the directory down with it. Nothing here touches the models, the
 * database or the configuration of whoever runs it.
 *
 * The page stands on its own here, without the dialog that otherwise holds it,
 * so the field "whisper-cli program" is empty in every picture: what fills it
 * is KConfigDialogManager, and there is none. Everything the pictures are
 * about — the list, the line under it and the button beside it — is the page's
 * own.
 *
 * Usage: QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *            QT_FORCE_STDERR_LOGGING=1 modelshots <target directory>
 *
 * For German pictures the message catalogue has to be findable, the same way
 * the README section "Screenshots" describes it for readmeshots:
 * `DESTDIR=<root> cmake --install build` and then `LANGUAGE=de
 * LANG=de_DE.UTF-8 XDG_DATA_DIRS=<root>/usr/share:/usr/share`. Without
 * `QT_FORCE_STDERR_LOGGING=1` every line this run reports goes into the
 * journal instead of the terminal (CLAUDE.md, finding 25).
 */
namespace
{
void shoot(QWidget &widget, const QString &directory, const QString &name)
{
    const QPixmap picture = widget.grab();
    if (!picture.save(directory + QLatin1Char('/') + name)) {
        qFatal("Picture %s could not be written", qUtf8Printable(name));
    }
    qInfo("written: %s (%dx%d px)", qUtf8Printable(name), picture.width(), picture.height());
}

/** What the page has in its line, read beside every picture (finding 43). */
QString stateLine(QWidget &page)
{
    auto *line = page.findChild<QLabel *>(QStringLiteral("modelState"));
    return line == nullptr ? QStringLiteral("<no line>") : line->text();
}

/** Answers the question the page asks before it fetches anything. */
void takeTheQuestion(const QString &directory, bool picture)
{
    QWidget *question = QApplication::activeModalWidget();
    if (question == nullptr) {
        qFatal("no question was asked before the download");
    }
    if (picture) {
        shoot(*question, directory, QStringLiteral("23-frage.png"));
    }
    const QList<QPushButton *> buttons = question->findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        if (button->text().contains(QStringLiteral("erunterladen"))
            || button->text().contains(QStringLiteral("ownload"))) {
            button->click();
            return;
        }
    }
    qFatal("the question has no button that starts the download");
}
}

int main(int argc, char **argv)
{
    const QTemporaryDir home;
    // Configuration, data and cache of the run's own — the download goes in
    // here and nowhere near ~/.local/share/denkzettel/models.
    qputenv("XDG_CONFIG_HOME", (home.path() + QStringLiteral("/config")).toLocal8Bit());
    qputenv("XDG_DATA_HOME", (home.path() + QStringLiteral("/data")).toLocal8Bit());
    qputenv("XDG_CACHE_HOME", (home.path() + QStringLiteral("/cache")).toLocal8Bit());
    QDir().mkpath(home.path() + QStringLiteral("/config"));

    // A colour scheme of the run's own, **before** QApplication: KColorScheme
    // never reads the application palette, so a palette set in code and the
    // roles this page asks for would be two different sources (CLAUDE.md,
    // finding 38). With this file the platform theme and KColorScheme read the
    // same one.
    QFile::copy(QStringLiteral("/usr/share/color-schemes/BreezeDark.colors"),
                home.path() + QStringLiteral("/config/kdeglobals"));

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    // The name the daemon registers, or the skeleton writes a modelshotsrc
    // (CLAUDE.md, finding 42).
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    if (argc < 2) {
        qFatal("Usage: modelshots <target directory>");
    }
    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    // Read back inside the run rather than trusted to the environment
    // (findings 28 and 38).
    const KColorScheme scheme(QPalette::Normal, KColorScheme::View);
    qInfo("style: %s · view %s on %s · neutral %s",
          qUtf8Printable(QApplication::style()->objectName()),
          qUtf8Printable(scheme.foreground(KColorScheme::NormalText).color().name()),
          qUtf8Printable(scheme.background().color().name()),
          qUtf8Printable(scheme.foreground(KColorScheme::NeutralText).color().name()));

    ModelDownload download;
    VoiceNotesPage page(&download);
    page.resize(620, 260);
    page.show();
    if (!QTest::qWaitForWindowExposed(&page)) {
        qFatal("The page never reached the screen");
    }
    QTest::qWait(400);
    qInfo("before: %s", qUtf8Printable(stateLine(page)));
    shoot(page, directory, QStringLiteral("23-sprachnotizen-vorher.png"));

    auto *sizes = page.findChild<QComboBox *>(QStringLiteral("kcfg_ModelSize"));
    if (sizes == nullptr) {
        qFatal("no model list on the page");
    }

    // The road a hand takes: the entry for `tiny` is pickable although the
    // model is not there, and picking it is what asks the question. The answer
    // has to come out of a timer — the question is modal.
    const auto choose = [sizes, &directory](bool picture) {
        QTimer::singleShot(600, sizes, [&directory, picture] {
            takeTheQuestion(directory, picture);
        });
        sizes->setCurrentIndex(0);
        QMetaObject::invokeMethod(sizes, "activated", Q_ARG(int, 0));
    };

    // Run one: the picture of a download under way, and then the button.
    QTimer::singleShot(3000, &page, [&page, &directory] {
        qInfo("during: %s", qUtf8Printable(stateLine(page)));
        auto *cancel = page.findChild<QPushButton *>(QStringLiteral("cancelDownload"));
        if (cancel == nullptr || !cancel->isVisible()) {
            qFatal("no cancel button while a download runs");
        }
        shoot(page, directory, QStringLiteral("23-sprachnotizen-laedt.png"));
        qInfo("%lld bytes of the model directory before the cancel",
              static_cast<long long>(
                  QFileInfo(Transcriber::modelPath(QStringLiteral("tiny"))).size()));
        cancel->click();
    });

    bool afterCancel = false;
    QObject::connect(&download,
                     &ModelDownload::finished,
                     &page,
                     [&page, &directory, &afterCancel, choose](const QString &size,
                                                               const QString &error) {
                         if (!afterCancel) {
                             if (error.isEmpty()) {
                                 qFatal("the download was not cancelled at all");
                             }
                             afterCancel = true;
                             QTest::qWait(400);
                             qInfo("after the cancel: %s", qUtf8Printable(stateLine(page)));
                             // Nothing of it is left, and that is the
                             // acceptance criterion of the story.
                             qInfo("model on disk: %s",
                                   QFile::exists(Transcriber::modelPath(size)) ? "yes" : "no");
                             shoot(page, directory, QStringLiteral("23-abgebrochen.png"));
                             // Run two, out of a timer of its own so that the
                             // question does not open inside this handler.
                             QTimer::singleShot(0, &page, [choose] {
                                 choose(false);
                             });
                             return;
                         }

                         if (!error.isEmpty()) {
                             qFatal("the download failed: %s", qUtf8Printable(error));
                         }
                         const QString path = Transcriber::modelPath(size);
                         QFile file(path);
                         if (!file.open(QIODevice::ReadOnly)) {
                             qFatal("no model at %s", qUtf8Printable(path));
                         }
                         QCryptographicHash hash(QCryptographicHash::Sha1);
                         hash.addData(&file);
                         // The two numbers that say the file really is the
                         // model: its length, and the SHA-1 the upstream
                         // README names for it (SPEC 12).
                         qInfo("%s: %lld bytes, sha1 %s",
                               qUtf8Printable(size),
                               static_cast<long long>(file.size()),
                               hash.result().toHex().constData());
                         qInfo("expected sha1 %s",
                               qUtf8Printable(ModelDownload::checksumFor(size)));
                         QTest::qWait(400);
                         qInfo("after: %s", qUtf8Printable(stateLine(page)));
                         shoot(page, directory, QStringLiteral("23-sprachnotizen-fertig.png"));
                         QCoreApplication::quit();
                     });

    choose(true);

    return QCoreApplication::exec();
}
