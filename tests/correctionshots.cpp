#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPixmap>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>

/**
 * The pictures and the numbers of issue #69: the line above the list that says
 * what the tolerant search of SPEC 6 really looked for.
 *
 * Not a test and out of `add_test()`, for the reason `readmeshots` is out of
 * it: a broken picture writer must not turn the suite red. It is built with
 * the suite all the same, because a runner nobody rebuilds ages unnoticed and
 * then writes plausible pictures of an **old** state with a fresh timestamp
 * (CLAUDE.md, rule 4).
 *
 * **The count beside the pictures is the point.** The UX decision of
 * 30.08.2026 rests on one sentence that was read out of SPEC 6 and not
 * measured: that the line cannot flicker while the user types, because every
 * prefix of a correct word has hits under the trigram tokenizer and the second
 * pass therefore never runs on the way to a finished word. This runner types
 * „prüfen" one character at a time into the real search field and prints, for
 * every prefix, how many notes the list holds and whether the line stands.
 * Expected is **zero** appearances; anything else sends the decision back.
 *
 * The second number is the same walk over „prüfem", the typo — there the line
 * has to appear exactly once, on the last character.
 *
 * Every note here is invented. The repository is public and a note is personal
 * data, so no run of this may ever take its material out of the session
 * somebody is working in.
 *
 * Usage — the environment is not optional, see rule 2 and findings 28 and 38.
 * The catalogue has to be findable at runtime and the **build** is what
 * compiles it (finding 57), so the order is build, install, run:
 *
 *   cmake --build build --target correctionshots
 *
 *   conf=$(mktemp -d)
 *   printf '[Theme]\nname=breeze-dark\n' > "$conf/plasmarc"
 *
 *   # German — this is what stands in docs/images/reviews/.
 *   cmake --build build
 *   dest=$(mktemp -d)
 *   DESTDIR="$dest" cmake --install build
 *
 *   env LANGUAGE=de LANG=de_DE.UTF-8 LC_ALL=de_DE.UTF-8 \
 *       XDG_DATA_DIRS="$dest/usr/share:/usr/share" XDG_CONFIG_DIRS="$conf:/etc/xdg" \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/correctionshots docs/images/reviews
 *
 * The check that turns the run into evidence is the run **without**
 * `XDG_DATA_DIRS`: the line then reads „Results for …" instead of „Ergebnisse
 * für …", so the difference is exactly the catalogue this run staged.
 */
namespace
{
// The two are the text of one note and its timestamp, and a type of their own
// for two values used six times would be the abstraction nobody asked for.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void add(Store &store, const QString &text, const QString &iso)
{
    Note note;
    note.createdAt = QDateTime::fromString(iso, Qt::ISODate);
    note.type = Note::Type::Text;
    note.content = text;
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

/**
 * Types `word` into the field one character at a time and counts how often the
 * line comes up.
 *
 * The count is of **appearances**, not of characters it stands for: what the
 * decision claims is that the line does not come and go while a correct word is
 * being typed, and a line that stood from the second character to the last
 * would be one appearance and still be the flicker the decision denies. So the
 * state of every prefix is printed beside it.
 *
 * The characters go in through `setText()` and not through
 * `QTest::keyClicks()`, and that is a limit of QTest rather than a choice:
 * `keyClicks` maps every character to a key code through `asciiToKey()`, which
 * asserts on anything outside ASCII — a walk over „prüfen" dies on the third
 * character (`qasciikey.cpp:192`, measured 30.08.2026). What the window reacts
 * to is `QLineEdit::textChanged`, and both roads emit it exactly once per
 * character.
 */
int typeAndCount(const QString &word, QLineEdit *field, const QLabel *line, const QListView *list)
{
    field->clear();
    QTest::qWait(50);
    int appearances = 0;
    bool wasVisible = false;
    for (qsizetype length = 1; length <= word.size(); ++length) {
        field->setText(word.left(length));
        QTest::qWait(50);
        const bool visible = line->isVisible();
        if (visible && !wasVisible) {
            ++appearances;
        }
        wasVisible = visible;
        const auto *model = qobject_cast<const NoteListModel *>(list->model());
        qWarning("  \"%s\"%*s notes=%-3d line=%d text=\"%s\"", qUtf8Printable(field->text()),
                 int(10 - field->text().size()), "", model ? model->noteCount() : -1,
                 int(visible), qUtf8Printable(line->text()));
    }
    return appearances;
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        qFatal("usage: correctionshots <target directory>");
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
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Read back what the run really drew with, rather than trusting that the
    // variables were set (findings 28 and 38).
    qWarning("style: %s  scale: %g", qUtf8Printable(app.style()->objectName()),
             qreal(app.devicePixelRatio()));

    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    const QTemporaryDir data;
    Store store(data.path() + QStringLiteral("/denkzettel.db"));
    if (!store.open()) {
        qFatal("store: %s", qUtf8Printable(store.lastError()));
    }
    // The registration read back at the database, not at the return value of
    // the call (SPEC 5.1). Without it every picture below would show a search
    // that never corrects, and nothing in the picture would say so.
    qWarning("corrections ready: %d", int(store.correctionsReady()));

    // Six notes, and the one that carries „prüfen" is neither the first nor
    // the last of them: a second pass that simply handed back the whole
    // library, or the newest note, would look right in a picture otherwise
    // (finding 34).
    add(store, QStringLiteral("Fotos vom Sommer als Fotobuch drucken lassen"),
        QStringLiteral("2026-07-31T15:04:00"));
    add(store, QStringLiteral("Straßenbahnen an der Endhaltestelle fotografieren"),
        QStringLiteral("2026-07-31T09:12:00"));
    add(store, QStringLiteral("Backup der Bücher-Datenbank prüfen"),
        QStringLiteral("2026-07-30T21:38:00"));
    add(store, QStringLiteral("Vor dem Monatsende nach dem Liefertermin fragen"),
        QStringLiteral("2026-07-30T08:15:00"));
    add(store, QStringLiteral("journalctl -u whisperd --since today"),
        QStringLiteral("2026-07-29T19:20:00"));
    add(store, QStringLiteral("Bücherkisten nie schwerer packen als 15 kg"),
        QStringLiteral("2026-07-29T08:05:00"));

    LibraryWindow window(&store);
    window.resize(900, 700);
    window.showLibrary();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("the library never reached the screen");
    }
    window.resize(900, 700);
    QTest::qWait(200);

    auto *search = window.findChild<QLineEdit *>();
    const auto *line = window.findChild<QLabel *>(QStringLiteral("correctionLine"));
    const auto *list = window.findChild<QListView *>();
    if (search == nullptr || line == nullptr || list == nullptr) {
        qFatal("the library window is not built the way this runner expects");
    }

    // The measurement the UX decision is owed: „prüfen" typed character by
    // character. Every prefix of a word that is really in the corpus has hits
    // under the trigram tokenizer, so the second pass never runs and the line
    // never comes up.
    qWarning("typing \"prüfen\" one character at a time:");
    const int correctWord = typeAndCount(QStringLiteral("prüfen"), search, line, list);
    qWarning("appearances while typing a correct word: %d (expected 0)", correctWord);

    // The counter-run, without which the number above says nothing: the same
    // walk over the typo has to come out **different**.
    qWarning("typing \"prüfem\" one character at a time:");
    const int typo = typeAndCount(QStringLiteral("prüfem"), search, line, list);
    qWarning("appearances while typing a typo: %d (expected 1)", typo);

    // Three states, and they have to come out different from one another
    // (finding 10): a spelling variant, a typo, and a word that is nowhere.
    const QStringList terms{QStringLiteral("pruefen"), QStringLiteral("prüfem"),
                            QStringLiteral("Kaltluftschleuse")};
    const QStringList names{QStringLiteral("69-schreibvariante-still.png"),
                            QStringLiteral("69-tippfehler-mit-zeile.png"),
                            QStringLiteral("69-kein-treffer.png")};
    for (int index = 0; index < terms.size(); ++index) {
        search->setText(terms.at(index));
        QTest::qWait(200);
        const auto *model = qobject_cast<const NoteListModel *>(list->model());
        qWarning("%s  typed=\"%s\" notes=%d line=%d text=\"%s\"", qUtf8Printable(names.at(index)),
                 qUtf8Printable(terms.at(index)), model ? model->noteCount() : -1,
                 int(line->isVisible()), qUtf8Printable(line->text()));
        // The statement about space the picture cannot carry: whether the
        // sentence begins where the list's own text begins. `HorizontalPadding`
        // is 12 in notelistdelegate.cpp and file-local there, so this is the
        // one place the two numbers can be held against each other.
        qWarning("%s  lineTextX=%d listTextX=%d", qUtf8Printable(names.at(index)),
                 line->mapTo(&window, QPoint(0, 0)).x() + line->contentsMargins().left(),
                 list->viewport()->mapTo(&window, QPoint(0, 0)).x() + 12);
        shoot(window, directory + QLatin1Char('/') + names.at(index));
    }

    return 0;
}
