#include "analysis/suggester.h"
#include "store/proposal.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/proposalwindow.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>

/**
 * The pictures of issue #30: the bundle card of the suggestion review, with
 * every note in the bundle and with one taken out of it.
 *
 * Not a test and out of `add_test()`, for the reason `readmeshots` is out of
 * it: a broken picture writer must not turn the suite red. It is built with
 * the suite all the same, because a runner nobody rebuilds ages unnoticed and
 * then writes plausible pictures of an **old** state with a fresh timestamp
 * (CLAUDE.md, rule 4).
 *
 * **What the numbers beside the pictures are for.** The acceptance criterion
 * is that deselecting works on the preview **live**, and a preview that was
 * never written looks exactly like one that was just written correctly
 * (finding 27) — so the run prints the preview of both states and how they
 * differ, and the two pictures have to come out different in the same place.
 * The note rows carry the second trap: an item view hands back the value it is
 * not showing (finding 51), so every row is printed with its check state, its
 * text and its rectangle in device pixels, beside the width the view really
 * gave it.
 *
 * Every note here is invented. The repository is public and a note is personal
 * data, so no run of this may ever take its material out of the session
 * somebody is working in.
 *
 * Usage — the environment is not optional, see rule 2 and findings 28 and 38.
 *
 * **The pictures under `docs/images/reviews/` are the German ones**, because
 * the wording of the three answers is what the customer reads. So the second
 * call below is the one that reproduces them; the first writes the same window
 * in the source language and belongs somewhere else.
 *
 * The catalogue has to be findable at runtime, and the **build** is what
 * compiles it (finding 57), so the order is build, install, run — never
 * install before build, or the readback names the previous wording:
 *
 *   cmake --build build --target proposalshots
 *
 *   conf=$(mktemp -d)
 *   printf '[Theme]\nname=breeze-dark\n' > "$conf/plasmarc"
 *
 *   # English — the source language, no catalogue involved. Writes into a
 *   # directory of its own; it is not what is committed.
 *   env -u LANGUAGE LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
 *       XDG_CONFIG_DIRS="$conf:/etc/xdg" \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/proposalshots /tmp/proposalshots-en
 *
 *   # German — this is what stands in docs/images/reviews/. Installed into a
 *   # throwaway root with DESTDIR, nothing is written outside it.
 *   cmake --build build
 *   dest=$(mktemp -d)
 *   DESTDIR="$dest" cmake --install build
 *
 *   env LANGUAGE=de LANG=de_DE.UTF-8 LC_ALL=de_DE.UTF-8 \
 *       XDG_DATA_DIRS="$dest/usr/share:/usr/share" XDG_CONFIG_DIRS="$conf:/etc/xdg" \
 *       QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *       QT_FORCE_STDERR_LOGGING=1 \
 *       build/bin/proposalshots docs/images/reviews
 *
 * The check that turns the second call into evidence is the same call
 * **without** `XDG_DATA_DIRS`: the three buttons have to read
 * "Accept · Later · Discard" then, and the head rows "Bundle: …" and
 * "Task: …".
 *
 * The third picture is the task card of issue #31 beside the bundle card, the
 * way wireframe 1c draws the two. Its numbers answer the question the picture
 * cannot: whether the five fields carry the suggestion's own values, and
 * whether what the annotation preview offers is the note and not the
 * description a second time.
 */
namespace
{
/** The width ProposalWindow gives itself; the third picture keeps it. */
constexpr int WindowWidth = 760;

/** Room under the lower card, the margin the card layout already keeps. */
constexpr int CardMargin = 12;

/** One invented note, written with the timestamp the bundle is ordered by. */
// Text and timestamp are the two columns of one note, and a type of their own
// for two values used five times would be the abstraction nobody asked for —
// the same reasoning categoryshots.cpp carries at its own add().
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
qint64 add(Store &store, const QString &text, const QString &iso)
{
    Note note;
    note.createdAt = QDateTime::fromString(iso, Qt::ISODate);
    note.type = Note::Type::Text;
    note.content = text;
    note.category = QStringLiteral("software");
    note.state = Note::State::Analysed;
    const std::optional<qint64> id = store.addNote(note);
    if (!id.has_value()) {
        qFatal("addNote: %s", qUtf8Printable(store.lastError()));
    }
    const std::optional<Note> stored = store.note(*id);
    if (!stored.has_value()) {
        qFatal("note: %s", qUtf8Printable(store.lastError()));
    }
    Note written = *stored;
    written.category = QStringLiteral("software");
    written.state = Note::State::Analysed;
    if (!store.updateNote(written)) {
        qFatal("updateNote: %s", qUtf8Printable(store.lastError()));
    }
    return *id;
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
 * Prints what one card shows: every note row with its check state and its
 * rectangle, and the preview beside it.
 *
 * The rectangle is what tells a row that is set from a row that is seen
 * (finding 51), and the preview is printed whole because the whole of it is
 * what the acceptance criterion is about.
 */
void report(const QString &name, const QWidget &window, qint64 id)
{
    const auto *notes = window.findChild<QListWidget *>(QStringLiteral("notes-%1").arg(id));
    const auto *preview = window.findChild<QTextBrowser *>(QStringLiteral("preview-%1").arg(id));
    const auto *accept = window.findChild<QPushButton *>(QStringLiteral("accept-%1").arg(id));
    if (!notes || !preview || !accept) {
        qFatal("the card of suggestion %lld is not built", static_cast<long long>(id));
    }

    const qreal ratio = window.devicePixelRatioF();
    for (int row = 0; row < notes->count(); ++row) {
        const QListWidgetItem *item = notes->item(row);
        const QRect rect = notes->visualItemRect(item);
        qWarning("%s  row %d checked=%d device x=%d..%d y=%d..%d text=%s",
                 qUtf8Printable(name), row, int(item->checkState() == Qt::Checked),
                 int(rect.left() * ratio), int(rect.right() * ratio),
                 int(rect.top() * ratio), int(rect.bottom() * ratio),
                 qUtf8Printable(item->text()));
    }
    qWarning("%s  noteList=%d viewport=%d preview=%d acceptEnabled=%d",
             qUtf8Printable(name), notes->width(), notes->viewport()->width(),
             preview->width(), int(accept->isEnabled()));

    // Where the two halves of the card body stand in the window, in device
    // pixels — the rectangle a difference between the two pictures is held
    // against. Taken from the built window and not from the picture: a
    // difference box that begins inside a widget looks exactly like a widget
    // that has moved (CLAUDE.md, finding 64).
    const QRect listRect(notes->mapTo(&window, QPoint(0, 0)), notes->size());
    const QRect previewRect(preview->mapTo(&window, QPoint(0, 0)), preview->size());
    qWarning("%s  body device x=%d..%d y=%d..%d (list x=%d..%d, preview x=%d..%d)",
             qUtf8Printable(name),
             int(listRect.left() * ratio), int(previewRect.right() * ratio),
             int(listRect.top() * ratio), int(listRect.bottom() * ratio),
             int(listRect.left() * ratio), int(listRect.right() * ratio),
             int(previewRect.left() * ratio), int(previewRect.right() * ratio));
    // The wording of the three answers, read back inside the run: it is what
    // says which catalogue this picture was drawn with (#138).
    QStringList labels;
    labels << accept->text();
    labels << window.findChild<QPushButton *>(QStringLiteral("later-%1").arg(id))->text();
    labels << window.findChild<QPushButton *>(QStringLiteral("discard-%1").arg(id))->text();
    const QString buttons = labels.join(QStringLiteral(" · "));
    qWarning("%s  buttons: %s", qUtf8Printable(name), qUtf8Printable(buttons));
    qWarning("%s  preview [%s]", qUtf8Printable(name), qUtf8Printable(preview->toPlainText()));
}

/**
 * Prints what one task card shows: the five fields with their values and the
 * annotation preview (issue #31).
 *
 * The values are read out of the widgets and not out of the payload that was
 * written: a field that never received its value looks exactly like one that
 * did, and only the readback tells the two apart (finding 27). The preview is
 * printed whole for the same reason — it says either the note text or the
 * sentence that nothing is annotated, and a picture cropping a long note
 * cannot show which.
 */
void reportTask(const QString &name, const QWidget &window, qint64 id)
{
    const auto *title = window.findChild<QLabel *>(QStringLiteral("title-%1").arg(id));
    const auto *preview = window.findChild<QTextBrowser *>(QStringLiteral("preview-%1").arg(id));
    const auto *accept = window.findChild<QPushButton *>(QStringLiteral("accept-%1").arg(id));
    const auto *frame = window.findChild<QWidget *>(QStringLiteral("card-%1").arg(id));
    if (!title || !preview || !accept || !frame) {
        qFatal("the task card of suggestion %lld is not built", static_cast<long long>(id));
    }

    const QStringList roles{QStringLiteral("description"),
                            QStringLiteral("project"),
                            QStringLiteral("tags"),
                            QStringLiteral("due"),
                            QStringLiteral("priority")};
    for (const QString &role : roles) {
        const auto *field = window.findChild<QLineEdit *>(QStringLiteral("%1-%2").arg(role).arg(id));
        if (!field) {
            qFatal("the task card carries no field %s", qUtf8Printable(role));
        }
        // The width beside the value: a field is a value that is **set**, the
        // width is what says it can be seen (finding 51).
        qWarning("%s  field %s width=%d text=[%s]",
                 qUtf8Printable(name), qUtf8Printable(role), field->width(),
                 qUtf8Printable(field->text()));
    }

    const qreal ratio = window.devicePixelRatioF();
    const QRect frameRect(frame->mapTo(&window, QPoint(0, 0)), frame->size());
    qWarning("%s  card device x=%d..%d y=%d..%d  acceptEnabled=%d",
             qUtf8Printable(name),
             int(frameRect.left() * ratio), int(frameRect.right() * ratio),
             int(frameRect.top() * ratio), int(frameRect.bottom() * ratio),
             int(accept->isEnabled()));
    qWarning("%s  title [%s]", qUtf8Printable(name), qUtf8Printable(title->text()));
    qWarning("%s  annotation [%s]", qUtf8Printable(name), qUtf8Printable(preview->toPlainText()));

    QStringList labels;
    labels << accept->text();
    labels << window.findChild<QPushButton *>(QStringLiteral("later-%1").arg(id))->text();
    labels << window.findChild<QPushButton *>(QStringLiteral("discard-%1").arg(id))->text();
    qWarning("%s  buttons: %s", qUtf8Printable(name), qUtf8Printable(labels.join(QStringLiteral(" · "))));
}
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        qFatal("usage: proposalshots <target directory>");
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
    // Without the domain every i18n() call falls back to the source language,
    // and a German run then writes an English picture without saying so (#138).
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // Read back what the run really drew with, rather than trusting that the
    // variables were set (findings 28 and 38).
    qWarning("style: %s  scale: %g", qUtf8Printable(app.style()->objectName()),
             qreal(app.devicePixelRatio()));
    qWarning("palette Window %s Base %s Text %s PlaceholderText %s",
             qUtf8Printable(app.palette().color(QPalette::Window).name()),
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

    // Five notes over two days, so the collective note of SPEC 8.1 shows both
    // of its `## <day>` sections and the deselection below can take a whole
    // section out of the preview — a change the picture cannot miss.
    const QList<qint64> noteIds{
        add(store, QStringLiteral("Bündel-Export erst ab fünf Notizen anbieten"),
            QStringLiteral("2026-07-29T09:14:00")),
        add(store, QStringLiteral("Whisper-Warteschlange bei Suspend anhalten"),
            QStringLiteral("2026-07-29T11:02:00")),
        add(store, QStringLiteral("Tray-Symbol im dunklen Thema prüfen"),
            QStringLiteral("2026-07-29T16:40:00")),
        add(store, QStringLiteral("Kategorien-Prompt: Beispiele mitgeben"),
            QStringLiteral("2026-07-30T08:20:00")),
        add(store, QStringLiteral("Fehlertext der Einbettung im Tray zeigen"),
            QStringLiteral("2026-07-30T10:05:00"))};

    QList<Note> notes;
    for (const qint64 id : noteIds) {
        notes.append(*store.note(id));
    }

    const QString title = QStringLiteral("Denkzettel-Entwicklung");
    Proposal proposal;
    proposal.kind = Proposal::Kind::Bundle;
    proposal.createdAt = QDateTime::fromString(QStringLiteral("2026-07-30T12:00:00"), Qt::ISODate);
    proposal.status = Proposal::Status::Open;
    // The Markdown as step 3 writes it (SPEC 7.3) — the same function the card
    // writes its preview with, so the picture shows the run's own text.
    proposal.payload = QString::fromUtf8(
        QJsonDocument(QJsonObject{{QLatin1String("title"), title},
                                  {QLatin1String("markdown"), bundleMarkdown(title, notes)}})
            .toJson(QJsonDocument::Compact));
    proposal.noteIds = noteIds;
    const std::optional<qint64> id = store.addProposal(proposal);
    if (!id.has_value()) {
        qFatal("addProposal: %s", qUtf8Printable(store.lastError()));
    }

    ProposalWindow window(&store);
    window.showProposals();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("the review never reached the screen");
    }
    QTest::qWait(200);

    report(QStringLiteral("30-karte-alle-notizen.png"), window, *id);
    shoot(window, directory + QStringLiteral("/30-karte-alle-notizen.png"));

    // And the same card with one note taken out. The **first** row, and that is
    // the whole reason it is that one: the preview is four lines tall and shows
    // the beginning of the collective note, so a note taken out further down
    // changes the text below the fold and the two pictures come out looking
    // identical — measured on the first pair of this story, where row 2 was
    // unticked, the readback said the preview had changed and the picture could
    // not show it (findings 27 and 51). Unticked at the top, the paragraph
    // leaves the visible part and the heading of its day stays, which is what
    // says the Markdown was written again rather than cut about.
    auto *notesView = window.findChild<QListWidget *>(QStringLiteral("notes-%1").arg(*id));
    notesView->item(0)->setCheckState(Qt::Unchecked);
    // Breeze animates a check mark, and a grab in the same turn draws the state
    // the animation starts from (CLAUDE.md, finding 43).
    QTest::qWait(400);

    report(QStringLiteral("30-karte-ohne-erste-notiz.png"), window, *id);
    shoot(window, directory + QStringLiteral("/30-karte-ohne-erste-notiz.png"));

    // The task card of issue #31, beside the bundle card the way wireframe 1c
    // draws the two. The first row is ticked again, so this picture differs
    // from the one above by the card that was added and by nothing else.
    notesView->item(0)->setCheckState(Qt::Checked);

    const qint64 callId = add(store, QStringLiteral("Mara wegen Wochenende anrufen\n"
                                                    "Kuchen nicht vergessen, sie mag den mit Mohn"),
                              QStringLiteral("2026-07-30T18:30:00"));
    Proposal task;
    task.kind = Proposal::Kind::Task;
    // Later than the bundle, so the bundle card keeps the top of the window and
    // the two pictures above stay the pictures they were.
    task.createdAt = QDateTime::fromString(QStringLiteral("2026-07-30T18:31:00"), Qt::ISODate);
    task.status = Proposal::Status::Open;
    // The `task` object of SPEC 7.2, handed on unchanged — that is what
    // Suggester writes into a task suggestion (SPEC 7.4).
    task.payload = QStringLiteral(R"({"description":"Mara wegen Wochenende anrufen",)"
                                  R"("project":"privat","tags":["anruf","familie"],)"
                                  R"("due":"2026-08-01","priority":"M"})");
    task.noteIds = {callId};
    const std::optional<qint64> taskId = store.addProposal(task);
    if (!taskId.has_value()) {
        qFatal("addProposal: %s", qUtf8Printable(store.lastError()));
    }

    // Both cards at once: the review is a place to compare questions in, and a
    // card that only exists below the fold says nothing about how the two kinds
    // sit together.
    //
    // **The height comes from the cards, and it is read while they are still
    // taller than the window** (CLAUDE.md, finding 84). In a window with room
    // to spare the box layout hands the surplus out and the reading comes back
    // 200 rows too large — measured on the first pair of this picture, where a
    // window resized to 1200 first answered 812 and the same two cards then
    // stood 611 tall. At the window's own height the cards overflow, the scroll
    // area gives its content the size it asks for, and that is the size they
    // really are.
    window.showProposals();
    QTest::qWait(400);

    const auto *taskFrame = window.findChild<QWidget *>(QStringLiteral("card-%1").arg(*taskId));
    if (!taskFrame) {
        qFatal("the task card is not built");
    }
    const int filled = taskFrame->mapTo(&window, QPoint(0, 0)).y() + taskFrame->height() + CardMargin;
    // The overflow is read back, not assumed: this measurement is only the size
    // the cards need while they are **taller** than the window — in a window
    // with room to spare the box layout has already handed the surplus to them
    // (CLAUDE.md, finding 90). A shorter pair of cards would quietly measure
    // the window height instead, and nothing in the picture would say so.
    qWarning("31-zwei-karten.png  the two cards fill %d logical rows, window was %d, overflowing=%d",
             filled, window.height(), int(filled > window.height()));
    if (filled <= window.height()) {
        qFatal("the cards fit the window, so their height was not measured but taken from it");
    }
    window.resize(WindowWidth, filled);
    QTest::qWait(200);

    report(QStringLiteral("31-zwei-karten.png"), window, *id);
    reportTask(QStringLiteral("31-zwei-karten.png"), window, *taskId);
    shoot(window, directory + QStringLiteral("/31-zwei-karten.png"));

    // The badge of SPEC 9 on the library header, over the same two open
    // suggestions the cards above stand for — so the number in the picture and
    // the number of cards beside it are one statement and not two.
    //
    // The picture is what says whether the counter fits the header row; that it
    // follows the store is a movement and belongs in a check, not in a still
    // image (CLAUDE.md, rule 2). `librarytest` holds that one.
    LibraryWindow library(&store);
    library.setReferenceTime(QDateTime::fromString(QStringLiteral("2026-07-31T15:30:00"), Qt::ISODate));
    library.showLibrary();
    if (!QTest::qWaitForWindowExposed(&library)) {
        qFatal("the library never reached the screen");
    }
    QTest::qWait(200);

    const auto *badge = library.findChild<QPushButton *>(QStringLiteral("proposals"));
    if (!badge) {
        qFatal("the library header carries no button for the suggestions");
    }
    // Read back beside the picture: a button that never received its number
    // looks exactly like one that has none to show (finding 27).
    qWarning("31-vorschlaege-plakette.png  badge [%s] width=%d visible=%d",
             qUtf8Printable(badge->text()), badge->width(), int(badge->isVisible()));
    shoot(library, directory + QStringLiteral("/31-vorschlaege-plakette.png"));

    return 0;
}
