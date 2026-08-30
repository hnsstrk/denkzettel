#include "settings/aiproviderpage.h"
#include "settings/settings.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QRadioButton>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>

/**
 * The pictures of issue #127: the page "AI provider" once openrouter.ai and
 * OpenAI are no longer selectable, and the sentence that says why.
 *
 * Not a test and out of `add_test()`, for the reason `readmeshots` is out of
 * it: a broken picture writer must not turn the suite red. It is built with
 * the suite all the same, because a runner nobody rebuilds ages unnoticed and
 * then writes plausible pictures of an **old** state with a fresh timestamp
 * (CLAUDE.md, rule 4).
 *
 * **One page per stored value, freshly built.** That is not tidiness, it is
 * the only way the picture shows what the dialog shows: the API key row hangs
 * on the Ollama button's `toggled` signal, so a page that is switched from
 * Ollama to another provider grows the row, while a page that comes up with
 * another provider already checked never sees the signal and stays without it
 * — and the second is what a freshly opened settings dialog does. Both are
 * printed below, the second as the control which says the row can come out
 * the other way at all.
 *
 * Beside every picture the run prints what it drew with and what it measured:
 * the style it resolved, the two colours, the resolved wording of the sentence
 * (which says whether the catalogue was found), and for all three buttons
 * `isEnabled()` and `isChecked()` next to the API key row's state. The checked
 * state is read **after** `QTest::qWait`, because Breeze animates the dot and
 * a `grab()` in the same turn draws the state the animation starts from
 * (CLAUDE.md, finding 43).
 *
 * The page stands on its own here, without the dialog that otherwise holds it,
 * so nothing fills the form from denkzettelrc: what fills it is
 * KConfigDialogManager, and there is none. That the stored value really
 * arrives on the greyed button, and goes back into the file unchanged, is the
 * business of `settingstest`, which owns the dialog —
 * `aStoredProviderSurvivesItsDisabledButton()`.
 *
 * **The committed pictures under `docs/images/reviews/` are the German ones**,
 * and the call below is the one that reproduces them byte for byte. The run
 * writes one language set per call under **the same three file names**, so an
 * English run pointed at that directory overwrites them with English pictures
 * of the same state — it belongs in a throwaway directory. Read it back: run
 * the line, then `git status`, and nothing may have changed.
 *
 * Usage — the environment is not optional, see rule 2 and finding 28. The
 * catalogue has to be findable at runtime, the way the README section
 * "Screenshots" describes it; nothing is written outside the staging root:
 *
 *   cmake --build build --target providershots
 *   conf=$(mktemp -d); printf '[Theme]\nname=breeze-dark\n' > "$conf/plasmarc"
 *   dest=$(mktemp -d); DESTDIR="$dest" cmake --install build
 *   env LANGUAGE=de LANG=de_DE.UTF-8 LC_ALL=de_DE.UTF-8 \
 *       XDG_DATA_DIRS="$dest/usr/share:/usr/share" XDG_CONFIG_DIRS="$conf:/etc/xdg" \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/providershots docs/images/reviews
 *
 * The same call without `XDG_DATA_DIRS` and with `-u LANGUAGE
 * LANG=en_US.UTF-8` writes the English set — **into a directory of its own**,
 * `build/providershots-en` or a `mktemp -d`, never into `docs/images/reviews`.
 * It is the control that says the catalogue is what makes the difference: the
 * sentence and the OpenAI label have to come out English then.
 *
 * Without `QT_FORCE_STDERR_LOGGING=1` every line this run reports goes into
 * the journal instead of the terminal (CLAUDE.md, finding 25).
 */
namespace
{
/** What the page carries, read back beside every picture. */
void report(const QString &what, QWidget &page)
{
    const auto *box = page.findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
    const auto *key = page.findChild<QLineEdit *>(QStringLiteral("apiKey"));
    const QList<QRadioButton *> buttons = box->findChildren<QRadioButton *>();
    for (const QRadioButton *button : buttons) {
        qWarning("%s  button \"%s\" enabled=%d checked=%d",
                 qUtf8Printable(what),
                 qUtf8Printable(button->text()),
                 int(button->isEnabled()),
                 int(button->isChecked()));
    }
    // `isHidden()` and not `isVisible()`: a row hidden by the page and a row on
    // a widget that has not been shown look the same to `isVisible()`.
    qWarning("%s  API key row shown=%d", qUtf8Printable(what), int(!key->isHidden()));
}

void shoot(QWidget &page, const QString &directory, const QString &name)
{
    const QPixmap picture = page.grab();
    if (!picture.save(directory + QLatin1Char('/') + name)) {
        qFatal("Picture %s could not be written", qUtf8Printable(name));
    }
    qWarning("written: %s (%dx%d px)", qUtf8Printable(name), picture.width(), picture.height());
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        qFatal("usage: providershots <target directory>");
    }

    // A configuration directory of its own, and a real colour scheme in it
    // **before** QApplication: without a kdeglobals the platform theme and
    // KColorScheme read two different sources, and the picture then shows a
    // fault of the runner (CLAUDE.md, finding 38). The desktop theme goes
    // beside it for finding 8.
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());
    QFile::copy(QStringLiteral("/usr/share/color-schemes/BreezeDark.colors"),
                configuration.path() + QStringLiteral("/kdeglobals"));
    QFile scheme(configuration.path() + QStringLiteral("/kdeglobals"));
    if (scheme.open(QIODevice::Append)) {
        scheme.write("\n[General]\nColorScheme=BreezeDark\n");
        scheme.close();
    }
    QFile plasma(configuration.path() + QStringLiteral("/plasmarc"));
    if (plasma.open(QIODevice::WriteOnly)) {
        plasma.write("[Theme]\nname=breeze-dark\n");
        plasma.close();
    }

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("denkzettel"));
    // Without the domain every i18n() falls back to English and the run writes
    // an English picture under a German name, whatever LANGUAGE says (#138).
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Read back what the run really drew with, rather than trusting that the
    // variables were set (findings 28 and 38).
    qWarning("style: %s", qUtf8Printable(app.style()->objectName()));
    qWarning("palette Window %s WindowText %s",
             qUtf8Printable(app.palette().color(QPalette::Window).name()),
             qUtf8Printable(app.palette().color(QPalette::WindowText).name()));

    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    const QList<QPair<int, QString>> states{{Settings::Ollama, QStringLiteral("127-anbieter-ollama.png")},
                                            {Settings::OpenRouter, QStringLiteral("127-anbieter-openrouter.png")},
                                            {Settings::OpenAi, QStringLiteral("127-anbieter-openai.png")}};

    for (const auto &state : states) {
        // A page of its own per state, so that the button is checked out of
        // the same nothing the dialog's manager checks it out of.
        AiProviderPage page;
        // The width the page has in the dialog at its built-in size: 640 less
        // the page list. Printed with every picture, so the numbers below can
        // be held against a rectangle set from outside (finding 64).
        page.resize(470, 260);
        page.show();
        if (!QTest::qWaitForWindowExposed(&page)) {
            qFatal("the page never reached the screen");
        }

        auto *box = page.findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
        box->findChildren<QRadioButton *>().at(state.first)->setChecked(true);

        // Breeze animates the dot, so both the readback and the grab wait for
        // the animation to finish (finding 43).
        QTest::qWait(400);

        // The sentence as it really resolved. That is the one line which says
        // whether the message catalogue was found — an English wording under a
        // German LANGUAGE is a runner without its domain or without its
        // catalogue, not a page without its sentence.
        //
        // Asked by object name and fatal when it is missing: a search over
        // every QLabel for a word out of the text prints nothing when it finds
        // nothing, and the run then ends with 0 and without the one line it is
        // here for (CLAUDE.md, findings 31 and 59).
        const auto *sentence = page.findChild<QLabel *>(QStringLiteral("unbuiltProviders"));
        if (sentence == nullptr) {
            qFatal("the page carries no sentence named unbuiltProviders");
        }
        qWarning("%s  sentence: %s", qUtf8Printable(state.second), qUtf8Printable(sentence->text()));
        qWarning("%s  page %dx%d logical, ratio %.1f",
                 qUtf8Printable(state.second),
                 page.width(),
                 page.height(),
                 page.devicePixelRatioF());
        report(state.second, page);
        // The row "Ollama address" is **empty in every picture**, and that is
        // this runner and not the product: the field is a `kcfg_` widget that
        // KConfigDialogManager fills, and the page stands here without the
        // dialog that owns one. In the built settings it carries the stored
        // address. Nothing this issue is about touches that row.
        shoot(page, directory, state.second);
    }

    // The control that has to come out different, and the only road left to
    // the API key row: switched **away** from Ollama, the row appears. Without
    // this line "the row is away" would be three readings of a row that might
    // as well never have been built (CLAUDE.md, verification stance).
    AiProviderPage control;
    control.resize(470, 260);
    control.show();
    if (!QTest::qWaitForWindowExposed(&control)) {
        qFatal("the control page never reached the screen");
    }
    auto *controlBox = control.findChild<QGroupBox *>(QStringLiteral("kcfg_Provider"));
    const QList<QRadioButton *> controlButtons = controlBox->findChildren<QRadioButton *>();
    controlButtons.at(Settings::Ollama)->setChecked(true);
    QTest::qWait(50);
    report(QStringLiteral("control, Ollama checked"), control);
    controlButtons.at(Settings::OpenRouter)->setChecked(true);
    QTest::qWait(50);
    report(QStringLiteral("control, switched away from Ollama by hand"), control);

    return 0;
}
