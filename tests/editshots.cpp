#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

/**
 * Writes the picture series of the edit view for the handover of S8 (#11)
 * and walks its main path once (DoD 2, wireframe 2a).
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

    const QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    // Note on the symbols of the guard dialog (finding 5 of the UI review):
    // this bench cannot show them. Measured on 02.08.2026, offscreen and on a
    // real Wayland session alike: the icon theme resolves by name to
    // „breeze-dark“, and `QIcon::fromTheme()` still comes back null under it —
    // so the dialog is drawn with bare buttons here. That the buttons ask for
    // symbols is held by `namesTheThreeAnswersOfTheGuardDialog` in
    // `librarytest`, which names an icon theme of its own. Neither a search
    // path nor a forced theme name changed anything here; forcing one would
    // only produce a picture that claims more than this machine shows.
    if (argc < 2) {
        qFatal("Aufruf: editshots <Zielverzeichnis>");
    }
    const QString directory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(directory);

    // 1 and 2 — states A and B of wireframe 2a, and the main path of the story
    // in between: correct the transcript, save, find it again through the
    // search (DoD 2).
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

        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        correct(editorOf(window), QStringLiteral("Fold"), QStringLiteral("Vault"));
        shoot(window, directory, QStringLiteral("02-bearbeiten.png"));

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
            QMessageBox *dialog = nullptr;
            for (int attempt = 0; attempt < 200 && !dialog; ++attempt) {
                dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
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

            QPixmap behind = window.grab();
            const QPixmap front = dialog->grab();
            QPainter painter(&behind);
            painter.drawPixmap((behind.width() - front.width()) / 2,
                               (behind.height() - front.height()) / 2,
                               front);
            painter.end();
            save(behind, directory, QStringLiteral("03-waechterdialog.png"));

            // Back into the edit state, so the run ends where it started —
            // told by the role, because the order is the platform's.
            const QList<QAbstractButton *> buttons = dialog->buttons();
            for (QAbstractButton *button : buttons) {
                if (dialog->buttonRole(button) == QMessageBox::RejectRole) {
                    button->click();
                    return;
                }
            }
        });

        // The third of the three ways into the dialog (wireframe 2a, state C).
        window.close();
        qInfo("Fenster nach „Abbrechen“ noch offen: %s", window.isVisible() ? "ja" : "nein");
    }

    return 0;
}
