#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <KMessageWidget>

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QToolButton>

/**
 * Writes the picture series of the edit view for the handover of S8 (#11)
 * and walks its main path once (wireframe 2a).
 *
 * Not a test — a picture maker, and the self-check of the story in one. It is
 * built and run by hand and stays out of `add_test()`, like `libraryshots`: a
 * picture nobody looks at proves nothing, and a failing screenshot writer must
 * not turn the suite red.
 *
 * Run it with QT_QPA_PLATFORMTHEME=kde. Without it Qt falls back to a
 * substitute font whose sizes are not the ones the running application uses.
 *
 * Category and tags are filled in here by hand: in M2 no analysis run exists,
 * and without them state B could not show the two rows it is judged on. That
 * belongs into the test bench, not into the product (issue #11, K3) — the
 * database is a temporary one, never the user's.
 *
 * Usage: QT_QPA_PLATFORMTHEME=kde editshots <target directory>
 */
namespace
{
/** The Friday the pictures are taken on, as in `libraryshots`. */
QDateTime friday()
{
    return QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate);
}

QDateTime at(const QString &isoDateTime)
{
    return QDateTime::fromString(isoDateTime, Qt::ISODate);
}

// Healing this means changing the signature or introducing a type of its own,
// which is design rather than tidying up (issue #76). The one case a mix-up
// would be visible in - placeholderPage() in the empty library - gets a test
// assurance instead, as issue #88.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
qint64 addNote(Store &store, const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = at(isoDateTime);

    const std::optional<qint64> id = store.addNote(note);
    if (!id.has_value()) {
        qFatal("Notiz ließ sich nicht speichern");
    }
    return *id;
}

/** Writes what the analysis run of M3 would write (K3). */
void analyse(Store &store, qint64 id, const QString &category, const QStringList &tags)
{
    const std::optional<Note> stored = store.note(id);
    if (!stored.has_value()) {
        qFatal("Notiz %lld nicht gefunden", static_cast<long long>(id));
    }

    Note note = *stored;
    note.category = category;
    note.state = Note::State::Analysed;
    if (!store.updateNote(note) || !store.setTags(id, tags)) {
        qFatal("Prüfaufbau ließ sich nicht bestücken");
    }
}

QListView *listOf(QWidget &window)
{
    auto *list = window.findChild<QListView *>();
    Q_ASSERT(list);
    return list;
}

QPlainTextEdit *editorOf(QWidget &window)
{
    auto *editor = window.findChild<QPlainTextEdit *>();
    Q_ASSERT(editor);
    return editor;
}

QPushButton *buttonNamed(QWidget &window, const QString &text)
{
    const QList<QPushButton *> buttons = window.findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        if (button->text() == text) {
            return button;
        }
    }
    qFatal("Schaltfläche „%s“ nicht gefunden", qUtf8Printable(text));
}

/**
 * The answers of a message dialog.
 *
 * Out of its QDialogButtonBox: a KMessageDialog also carries a hidden „do not
 * ask again“ checkbox, and that is a QAbstractButton as well.
 */
QList<QAbstractButton *> answersOf(QDialog *dialog)
{
    auto *box = dialog->findChild<QDialogButtonBox *>();
    return box ? box->buttons() : QList<QAbstractButton *>();
}

/** The answer Return would give, or nullptr. */
QAbstractButton *defaultAnswer(QDialog *dialog)
{
    const QList<QAbstractButton *> answers = answersOf(dialog);
    for (QAbstractButton *answer : answers) {
        auto *push = qobject_cast<QPushButton *>(answer);
        if (push && push->isDefault()) {
            return push;
        }
    }
    return nullptr;
}

void open(LibraryWindow &window)
{
    window.resize(900, 600);
    window.showLibrary();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("Fenster kam nicht auf den Schirm");
    }
}

void save(const QPixmap &picture, const QString &directory, const QString &name)
{
    if (!picture.save(directory + QLatin1Char('/') + name)) {
        qFatal("Bild %s ließ sich nicht schreiben", qUtf8Printable(name));
    }
    qInfo("geschrieben: %s", qUtf8Printable(name));
}

void shoot(QWidget &window, const QString &directory, const QString &name)
{
    QTest::qWait(200);
    save(window.grab(), directory, name);
}

/** The transcript of wireframe 2a, with the word Whisper misheard. */
QString wrongTranscript()
{
    return QStringLiteral("Transkript: Idee für Denkzettel — den Bündel-Export erst vorschlagen, "
                          "wenn mindestens fünf Notizen zum selben Thema da sind, sonst wird der "
                          "Fold zugemüllt. Schwelle einstellbar machen.");
}

/**
 * Replaces the misheard word the way a user does: pick it, type over it.
 *
 * Not through setPlainText() — that would put the cursor back to the first
 * character, and the picture would then show a cursor position the running
 * application never produces.
 */
// Healing this means changing the signature or introducing a type of its own,
// which is design rather than tidying up (issue #76). The one case a mix-up
// would be visible in - placeholderPage() in the empty library - gets a test
// assurance instead, as issue #88.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void correct(QPlainTextEdit *editor, const QString &misheard, const QString &meant)
{
    const QTextCursor found = editor->document()->find(misheard);
    if (found.isNull()) {
        qFatal("Wort „%s“ steht nicht im Text", qUtf8Printable(misheard));
    }

    editor->setTextCursor(found);
    QTest::keyClicks(editor, meant);
}

/** Fills a store with the notes of wireframe 2a and returns the edited one. */
qint64 fill(Store &store)
{
    addNote(store, QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten"),
            QStringLiteral("2026-07-31T14:32:00"));
    const qint64 edited = addNote(store, wrongTranscript(), QStringLiteral("2026-07-31T11:05:00"));
    addNote(store, QStringLiteral("journalctl -u whisperd --since today"),
            QStringLiteral("2026-07-30T21:48:00"));

    analyse(store,
            edited,
            QStringLiteral("Software-Ideen"),
            QStringList({QStringLiteral("software-idee"), QStringLiteral("denkzettel"),
                         QStringLiteral("export")}));

    return edited;
}
}

int main(int argc, char **argv)
{
    // As in `libraryshots`: the pictures are to show the state as shipped, not
    // the window size and splitter position whoever runs this has stored.
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    // Note on the symbols of the guard dialog: this bench can show them again
    // since issue #66, and that is the point of the change.
    //
    // Until then it could not, and no setting here helped: under
    // QT_QPA_PLATFORMTHEME=kde the KDE platform integration answered a built
    // QMessageBox with a message box of its own, and that one carried none of
    // the icons set on our buttons afterwards — the buttons in the picture
    // were not the buttons the code had given symbols to. The guard is a
    // KMessageDialog now, a plain QDialog that stays ours (SPEC 9).
    if (argc < 2) {
        qFatal("Aufruf: editshots <Zielverzeichnis>");
    }
    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    // 1 and 2 — states A and B of wireframe 2a, and the main path of the story
    // in between: correct the transcript, save, find it again through the
    // search.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        const qint64 edited = fill(store);

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(2, 0));
        shoot(window, directory, QStringLiteral("01-lesen.png"));

        // Beside the picture, in words: the symbol name and what the symbol
        // costs in button width. The picture shows a drawing, the name shows
        // what was asked for, and only the two together say whether issue #67
        // is met (wireframe 2a, table „Symbole an den Schaltflächen“).
        for (const QString &label : {QStringLiteral("Bearbeiten"), QStringLiteral("Löschen")}) {
            const QPushButton *button = buttonNamed(window, label);
            qInfo("Detailkopf: „%s“ Symbol „%s“, %d px breit (natürlich %d px)",
                  qUtf8Printable(label),
                  qUtf8Printable(button->icon().name()),
                  button->width(),
                  button->sizeHint().width());
        }

        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        correct(editorOf(window), QStringLiteral("Fold"), QStringLiteral("Vault"));
        shoot(window, directory, QStringLiteral("02-bearbeiten.png"));

        for (const QString &label : {QStringLiteral("Speichern"), QStringLiteral("Abbrechen")}) {
            const QPushButton *button = buttonNamed(window, label);
            qInfo("Bearbeiten-Fußzeile: „%s“ Symbol „%s“, %d px breit (natürlich %d px)",
                  qUtf8Printable(label),
                  qUtf8Printable(button->icon().name()),
                  button->width(),
                  button->sizeHint().width());
        }

        qInfo("vor dem Speichern: „Fold“ %lld Treffer, „Vault“ %lld Treffer, needs_reembed=%d",
              static_cast<long long>(store.search(QStringLiteral("Fold")).size()),
              static_cast<long long>(store.search(QStringLiteral("Vault")).size()),
              store.note(edited)->needsReembed ? 1 : 0);

        buttonNamed(window, QStringLiteral("Speichern"))->click();

        const std::optional<Note> saved = store.note(edited);
        qInfo("nach dem Speichern: „Fold“ %lld Treffer, „Vault“ %lld Treffer, needs_reembed=%d, "
              "Kategorie „%s“, Tags „%s“, Zustand %d",
              static_cast<long long>(store.search(QStringLiteral("Fold")).size()),
              static_cast<long long>(store.search(QStringLiteral("Vault")).size()),
              saved->needsReembed ? 1 : 0,
              qUtf8Printable(saved->category),
              qUtf8Printable(store.tags(edited).join(QStringLiteral(" · "))),
              static_cast<int>(saved->state));
        qInfo("Text jetzt: %s", qUtf8Printable(saved->content));

        // Finding it again over the search — the last step of the main path.
        auto *search = window.findChild<QLineEdit *>();
        Q_ASSERT(search);
        search->setText(QStringLiteral("Vault"));
        QTest::qWait(100);
        qInfo("Suche „Vault“ in der Bibliothek: %d Zeile(n) in der Liste", list->model()->rowCount());
        shoot(window, directory, QStringLiteral("04-wiedergefunden.png"));
    }

    // 3 — state C: the guard dialog over unsaved changes. grab() only ever
    // catches one top-level window, so window and dialog are grabbed in the
    // same moment and drawn one over the other. Nothing is added to the
    // picture that the running application does not show — in particular the
    // dialog is not dimmed or shaded here.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        fill(store);

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(2, 0));
        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        correct(editorOf(window), QStringLiteral("Fold"), QStringLiteral("Vault"));

        QTimer::singleShot(0, qApp, [&window, &directory] {
            QDialog *dialog = nullptr;
            for (int attempt = 0; attempt < 200 && !dialog; ++attempt) {
                dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dialog) {
                    QTest::qWait(10);
                }
            }
            if (!dialog) {
                qFatal("Der Wächterdialog ist nicht erschienen");
            }
            if (!QTest::qWaitForWindowExposed(dialog)) {
                qFatal("Der Wächterdialog kam nicht auf den Schirm");
            }
            QTest::qWait(200);

            // What the picture is to be judged on, written out beside it: the
            // symbol names and the default answer are what issues #66 and #67
            // are about, and a picture alone cannot tell „no symbol asked for“
            // from „theme did not resolve it“.
            const QList<QAbstractButton *> answers = answersOf(dialog);
            for (const QAbstractButton *answer : answers) {
                qInfo("Wächterdialog: „%s“ Symbol „%s“%s",
                      qUtf8Printable(answer->text()),
                      qUtf8Printable(answer->icon().name()),
                      defaultAnswer(dialog) == answer ? " (Vorgabe)" : "");
            }

            // The warning symbol sits in a picture label and has no name to
            // ask for, so its size is what can be said about it. It stands in
            // the log as well as in the picture on purpose: a picture does not
            // carry its build state in its face — this bench is
            // EXCLUDE_FROM_ALL, and a plain `cmake --build build` leaves it as
            // it was (measured 02.08.2026, when a fresh picture showed an old
            // dialog).
            QSize warning;
            const QList<QLabel *> labels = dialog->findChildren<QLabel *>();
            for (const QLabel *label : labels) {
                if (label->isVisible() && !label->pixmap().isNull()) {
                    warning = label->pixmap().size();
                }
            }
            qInfo("Wächterdialog: Warnsymbol %dx%d", warning.width(), warning.height());

            QPixmap behind = window.grab();
            const QPixmap front = dialog->grab();
            QPainter painter(&behind);
            painter.drawPixmap((behind.width() - front.width()) / 2,
                               (behind.height() - front.height()) / 2,
                               front);
            painter.end();
            save(behind, directory, QStringLiteral("03-waechterdialog.png"));

            // Back into the edit state, so the run ends where it started —
            // by the label, because the order is the platform's.
            for (QAbstractButton *answer : answers) {
                if (answer->text() == QStringLiteral("Abbrechen")) {
                    answer->click();
                    return;
                }
            }
            qFatal("Der Wächterdialog bietet kein „Abbrechen“");
        });

        // The third of the three ways into the dialog (wireframe 2a, state C).
        window.close();
        qInfo("Fenster nach „Abbrechen“ noch offen: %s", window.isVisible() ? "ja" : "nein");
    }

    // 5 — the message row of the deletion, the fourth labelled control of the
    // library and the only one outside the detail pane (wireframe 2b, #67).
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        fill(store);

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(2, 0));
        buttonNamed(window, QStringLiteral("Löschen"))->click();

        // animatedShow() grows the band out of nothing; the picture is to show
        // it grown.
        QTest::qWait(600);

        auto *message = window.findChild<KMessageWidget *>();
        if (!message || !message->isVisible()) {
            qFatal("Die Meldungszeile ist nicht erschienen");
        }

        const QList<QToolButton *> buttons = message->findChildren<QToolButton *>();
        for (const QToolButton *button : buttons) {
            qInfo("Meldungszeile: „%s“ Symbol „%s“",
                  qUtf8Printable(button->text()),
                  qUtf8Printable(button->icon().name()));
        }

        shoot(window, directory, QStringLiteral("05-loeschmeldung.png"));

        // Taken back again, so the bench leaves the store as it found it.
        for (QToolButton *button : buttons) {
            if (button->text() == QStringLiteral("Rückgängig")) {
                button->click();
                break;
            }
        }
        QTest::qWait(100);
        qInfo("Notizen nach „Rückgängig“: %lld", static_cast<long long>(store.notes().size()));
    }

    return 0;
}
