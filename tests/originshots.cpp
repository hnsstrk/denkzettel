#include "settings/capturepage.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QListView>
#include <QPixmap>
#include <QPushButton>
#include <QSplitter>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>

/**
 * The pictures of issue #47: the origin of a note in the reading pane, and the
 * privacy switch that decides whether there is one.
 *
 * Not a test and out of `add_test()`, for the reason `readmeshots` is out of
 * it: a broken picture writer must not turn the suite red. It is built with
 * the suite all the same, because a runner nobody rebuilds ages unnoticed and
 * then writes plausible pictures of an **old** state with a fresh timestamp
 * (CLAUDE.md, rule 4).
 *
 * Beside every picture it prints what it drew with and what it measured — the
 * style it resolved, the two colours, the width the origin line really got and
 * where the two buttons stand. Those numbers are the reason the design of this
 * story was decided twice: behind the timestamp the line had 57 to 74 logical
 * pixels of the 256 the drawing reckoned with, so a browser title came out as
 * „· Firefox — …". It stands under the head row since, and gets 416.
 *
 * Every note and every window title here is invented. The repository is public
 * and a window title is personal data, so no run of this may ever take its
 * material out of the session somebody is working in.
 *
 * Usage — the environment is not optional, see rule 2 and finding 28:
 *
 *   cmake --build build --target originshots
 *   env -u LANGUAGE LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/originshots docs/images/reviews
 */
namespace
{
// The four strings are the columns of one note, and a type of their own for
// four literals used once would be the abstraction nobody asked for.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void add(Store &store, const QString &text, const QString &iso, const QString &origin,
         const QString &app)
{
    Note note;
    note.createdAt = QDateTime::fromString(iso, Qt::ISODate);
    note.type = Note::Type::Text;
    note.content = text;
    note.origin = origin;
    note.originApp = app;
    if (!store.addNote(note).has_value()) {
        qFatal("addNote: %s", qUtf8Printable(store.lastError()));
    }
}

void shoot(QWidget &window, const QString &file)
{
    const QPixmap picture = window.grab();
    if (!picture.save(file)) {
        qFatal("could not write %s", qUtf8Printable(file));
    }
    qWarning("%s  %d x %d", qUtf8Printable(file), picture.width(), picture.height());
}

/** The origin line is the one widget that gives its width wish up. */
const QLabel *originLine(const QWidget &window)
{
    const QList<QLabel *> labels = window.findChildren<QLabel *>();
    for (const QLabel *label : labels) {
        if (label->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) {
            return label;
        }
    }
    return nullptr;
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        qFatal("usage: originshots <target directory>");
    }

    // A configuration directory of its own, and a real colour scheme in it
    // **before** QApplication: without a kdeglobals the platform theme and
    // KColorScheme read two different sources, and the picture then shows a
    // fault of the runner (CLAUDE.md, finding 38).
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

    // Read back what the run really drew with, rather than trusting that the
    // variables were set (findings 28 and 38).
    qWarning("style: %s", qUtf8Printable(app.style()->objectName()));
    qWarning("palette Base %s Text %s PlaceholderText %s",
             qUtf8Printable(app.palette().color(QPalette::Base).name()),
             qUtf8Printable(app.palette().color(QPalette::Text).name()),
             qUtf8Printable(app.palette().color(QPalette::PlaceholderText).name()));

    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    const QTemporaryDir data;
    Store store(data.path() + QStringLiteral("/denkzettel.db"));
    if (!store.open()) {
        qFatal("store: %s", qUtf8Printable(store.lastError()));
    }

    // Three notes, newest first: one without an origin, one whose title fits,
    // and one whose title is that title twice over and has to be cut.
    add(store, QStringLiteral("Keep listening to the podcast about sleep phases, from minute 40"),
        QStringLiteral("2026-07-31T15:04:00"), QString(), QString());
    add(store, QStringLiteral("Idea: have the summer photos printed as a small photo book."),
        QStringLiteral("2026-07-31T09:12:00"),
        QStringLiteral("Firefox — Human Interface Guidelines · Developer"),
        QStringLiteral("org.mozilla.firefox"));
    add(store, QStringLiteral("Never pack book boxes heavier than 15 kg."),
        QStringLiteral("2026-07-30T21:38:00"),
        QStringLiteral("Firefox — Human Interface Guidelines · Developer — Human Interface "
                       "Guidelines · Developer"),
        QStringLiteral("org.mozilla.firefox"));

    LibraryWindow window(&store);
    window.resize(900, 700);
    window.showLibrary();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("the library never reached the screen");
    }
    // The width the design was argued about, and the splitter as it comes.
    window.resize(900, 700);
    QTest::qWait(200);

    auto *list = window.findChild<QListView *>();
    const auto *splitter = window.findChild<QSplitter *>();
    auto *model = qobject_cast<NoteListModel *>(list->model());

    const QStringList names{QStringLiteral("47-ohne-herkunft.png"),
                            QStringLiteral("47-eigene-zeile.png"),
                            QStringLiteral("47-eigene-zeile-gekuerzt.png")};

    for (int note = 0; note < model->noteCount() && note < names.size(); ++note) {
        list->setCurrentIndex(model->index(model->rowOfNote(note)));
        QTest::qWait(150);

        // The buttons must stand in the same place in every picture, and the
        // line must not be the reason they move (the check point of the UX
        // decision). Printed rather than asserted: this is a picture runner.
        const QList<QPushButton *> buttons = window.findChildren<QPushButton *>();
        for (const QPushButton *button : buttons) {
            if (button->isVisible() && !button->text().isEmpty()) {
                qWarning("%s  button %s at x=%d w=%d", qUtf8Printable(names.at(note)),
                         qUtf8Printable(button->text()), button->mapTo(&window, QPoint(0, 0)).x(),
                         button->width());
            }
        }
        const QLabel *line = originLine(window);
        qWarning("%s  origin line visible=%d w=%d text=%s", qUtf8Printable(names.at(note)),
                 line ? int(line->isVisible()) : -1, line ? line->width() : -1,
                 line ? qUtf8Printable(line->text()) : "");
        qWarning("%s  splitter %d/%d/%d", qUtf8Printable(names.at(note)), splitter->sizes().at(0),
                 splitter->sizes().at(1), splitter->sizes().at(2));

        shoot(window, directory + QLatin1Char('/') + names.at(note));
    }

    // The page of SPEC 13 on its own, as modelshots does it with "Voice notes":
    // what fills a `kcfg_` widget is KConfigDialogManager and there is none
    // here, so the box stands unchecked — which is the default anyway.
    CapturePage page;
    page.resize(460, 160);
    page.show();
    if (!QTest::qWaitForWindowExposed(&page)) {
        qFatal("the page never reached the screen");
    }
    // Breeze animates the check mark, so a grab in the same turn draws the
    // state the animation starts from (finding 43).
    QTest::qWait(400);
    shoot(page, directory + QStringLiteral("/47-einstellungen-erfassung.png"));

    return 0;
}
