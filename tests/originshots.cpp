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
#include <QStandardPaths>
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
 * style it resolved, the two colours, the rectangle the origin line really got
 * and where the two buttons stand. Those numbers are the reason the design of
 * this story was decided twice: behind the timestamp the line had 57 to 74
 * logical pixels of the 256 the drawing reckoned with, so a browser title came
 * out as „· Firefox — …". It stands under the head row since, and gets 416.
 *
 * The rectangle is printed in device pixels because a picture difference is
 * read against it (CLAUDE.md, finding 64): a difference box that begins inside
 * the line looks exactly like a line that has moved, and only a geometry taken
 * from outside the picture tells the two apart.
 *
 * **The application in front of the title is the second decision of this
 * story** (customer report 29.08.2026), and the three cases it can come out in
 * are three pictures here: an id with a desktop file, one without, and a note
 * that carries no id at all. The resolvable one is a desktop file this run
 * writes itself into an `XDG_DATA_HOME` of its own — a real application of the
 * machine would make the picture depend on what happens to be installed, and
 * `/usr/share/applications` stays reachable whatever a run stages
 * (finding 21). Both lookups are printed, so the run says that one of them
 * found a file and the other did not.
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

    // The desktop file behind the one origin whose id resolves. Invented, and
    // written here rather than taken off the machine: what is installed on the
    // machine running this is no part of the picture, and an id nobody ships
    // is the only one that is unresolvable everywhere.
    const QTemporaryDir share;
    qputenv("XDG_DATA_HOME", share.path().toLocal8Bit());
    QDir().mkpath(share.path() + QStringLiteral("/applications"));
    QFile entry(share.path() + QStringLiteral("/applications/com.example.terminal.desktop"));
    if (entry.open(QIODevice::WriteOnly)) {
        entry.write("[Desktop Entry]\nType=Application\nName=Sample Terminal\nExec=false\n");
        entry.close();
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

    // The control that has to come out different: one id has a desktop file
    // and the other has none. Without this line the two pictures would only
    // say what they show, not that the run really looked twice.
    for (const QString &id : {QStringLiteral("com.example.terminal"),
                              QStringLiteral("com.example.unpackaged")}) {
        qWarning("desktop file for %s: %s", qUtf8Printable(id),
                 qUtf8Printable(QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                                       id + QStringLiteral(".desktop"))));
    }

    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    const QTemporaryDir data;
    Store store(data.path() + QStringLiteral("/denkzettel.db"));
    if (!store.open()) {
        qFatal("store: %s", qUtf8Printable(store.lastError()));
    }

    // Four notes, newest first: one without an origin, one whose id resolves to
    // a name, one whose id resolves to nothing, and one whose title has to be
    // cut.
    //
    // The two middle titles are the customer's case of 29.08.2026: a terminal
    // writes the work into its title and its own name into nothing, so the
    // title alone leaves the line saying what was being done and not where.
    add(store, QStringLiteral("Keep listening to the podcast about sleep phases, from minute 40"),
        QStringLiteral("2026-07-31T15:04:00"), QString(), QString());
    add(store, QStringLiteral("Idea: have the summer photos printed as a small photo book."),
        QStringLiteral("2026-07-31T09:12:00"),
        QStringLiteral("Sprint triage: roadmap Q3-Q4"),
        QStringLiteral("com.example.terminal"));
    add(store, QStringLiteral("Never pack book boxes heavier than 15 kg."),
        QStringLiteral("2026-07-30T21:38:00"),
        QStringLiteral("Sprint triage: roadmap Q3-Q4"),
        QStringLiteral("com.example.unpackaged"));
    add(store, QStringLiteral("Ask again about the delivery date before the end of the month."),
        QStringLiteral("2026-07-30T08:15:00"),
        QStringLiteral("Human Interface Guidelines · Developer — Human Interface "
                       "Guidelines · Developer"),
        QStringLiteral("com.example.terminal"));

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
                            QStringLiteral("47-herkunft-ohne-desktop-datei.png"),
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
        // The rectangle in device pixels, which is what a picture difference is
        // held against (finding 64) — and the text beside it, which says only
        // that the value is set, never that it is visible (finding 51).
        const qreal ratio = window.devicePixelRatioF();
        const QPoint at = line ? line->mapTo(&window, QPoint(0, 0)) : QPoint(-1, -1);
        qWarning("%s  origin line visible=%d device x=%d..%d y=%d..%d text=%s",
                 qUtf8Printable(names.at(note)), line ? int(line->isVisible()) : -1,
                 int(at.x() * ratio), int((at.x() + (line ? line->width() : 0)) * ratio),
                 int(at.y() * ratio), int((at.y() + (line ? line->height() : 0)) * ratio),
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
