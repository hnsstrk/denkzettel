#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBrowser>
#include <QTimer>

#include <functional>

/**
 * Bildläufer des UI-Reviews zu S8 (#11), unabhängig von `tests/editshots.cpp`
 * geschrieben: die Szenen kommen aus Wireframe 2a/2b, nicht aus der Bildstrecke
 * des Entwicklers.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde
 *         QT_LOGGING_TO_CONSOLE=1 uxshots <Zielverzeichnis>
 */
namespace
{
QString gDirectory;

QDateTime friday()
{
    return QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate);
}

qint64 addNote(Store &store, const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = QDateTime::fromString(isoDateTime, Qt::ISODate);

    const std::optional<qint64> id = store.addNote(note);
    if (!id.has_value()) {
        qFatal("Notiz ließ sich nicht speichern");
    }
    return *id;
}

/** Schreibt, was der Analyse-Lauf ab M3 schriebe (Prüfmittel-Vermerk K3). */
void analyse(Store &store, qint64 id, const QString &category, const QStringList &tags)
{
    Note note = *store.note(id);
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

QLineEdit *searchOf(QWidget &window)
{
    auto *search = window.findChild<QLineEdit *>();
    Q_ASSERT(search);
    return search;
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

/** Der Beschriftungstext rechts neben einer Marke der Merkmalszeile. */
QString valueAfter(QWidget &window, const QString &caption)
{
    const QList<QLabel *> labels = window.findChildren<QLabel *>();
    for (int i = 0; i < labels.size(); ++i) {
        if (labels.at(i)->text() == caption && i + 1 < labels.size()) {
            return labels.at(i + 1)->text();
        }
    }
    return QStringLiteral("<nicht gefunden>");
}

void open(LibraryWindow &window, int width = 900, int height = 600)
{
    window.resize(width, height);
    window.showLibrary();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("Fenster kam nicht auf den Schirm");
    }
    window.resize(width, height);
    QTest::qWait(100);
}

void save(const QPixmap &picture, const QString &name)
{
    if (!picture.save(gDirectory + QLatin1Char('/') + name)) {
        qFatal("Bild %s ließ sich nicht schreiben", qUtf8Printable(name));
    }
    qInfo("geschrieben: %s (%dx%d)", qUtf8Printable(name), picture.width(), picture.height());
}

void shoot(QWidget &window, const QString &name)
{
    QTest::qWait(150);
    save(window.grab(), name);
}

/** Ausschnitt eines Kindfensters, in Fensterkoordinaten. */
void shootPart(QWidget &window, QWidget *part, const QString &name)
{
    QTest::qWait(150);
    const QRect area(part->mapTo(&window, QPoint(0, 0)), part->size());
    save(window.grab(area), name);
}

/**
 * Führt `trigger` aus und nimmt den dabei aufgehenden modalen Dialog auf.
 *
 * grab() erwischt immer nur ein Fenster; Bibliothek und Dialog werden im selben
 * Augenblick aufgenommen und übereinandergelegt. Hinzugefügt wird nichts —
 * insbesondere ist das Fenster hinter dem Dialog nicht abgedunkelt.
 */
void withDialog(QWidget &window, const std::function<void()> &trigger, const QString &name,
                QMessageBox::ButtonRole answer)
{
    QTimer::singleShot(0, qApp, [&window, name, answer] {
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

        qInfo("Dialog „%s“: Titel „%s“, Text „%s“, Zusatz „%s“",
              qUtf8Printable(name),
              qUtf8Printable(dialog->windowTitle()),
              qUtf8Printable(dialog->text()),
              qUtf8Printable(dialog->informativeText()));

        const QList<QAbstractButton *> buttons = dialog->buttons();
        QStringList order;
        for (QAbstractButton *button : buttons) {
            order << QStringLiteral("%1 (x=%2, Rolle %3%4)")
                         .arg(button->text())
                         .arg(button->x())
                         .arg(static_cast<int>(dialog->buttonRole(button)))
                         .arg(button == dialog->defaultButton() ? QStringLiteral(", Vorgabe")
                                                                : QString());
        }
        qInfo("  Knöpfe in Fensterreihenfolge: %s", qUtf8Printable(order.join(QStringLiteral(" · "))));
        qInfo("  Vorgabeknopf „%s“ · Escape-Knopf „%s“ · Fokus auf „%s“",
              dialog->defaultButton() ? qUtf8Printable(dialog->defaultButton()->text()) : "keiner",
              dialog->escapeButton() ? qUtf8Printable(dialog->escapeButton()->text()) : "keiner",
              qobject_cast<QAbstractButton *>(dialog->focusWidget())
                  ? qUtf8Printable(qobject_cast<QAbstractButton *>(dialog->focusWidget())->text())
                  : "kein Knopf");

        QPixmap behind = window.grab();
        const QPixmap front = dialog->grab();
        QPainter painter(&behind);
        painter.drawPixmap((behind.width() - front.width()) / 2, (behind.height() - front.height()) / 2, front);
        painter.end();
        save(behind, name);

        for (QAbstractButton *button : buttons) {
            if (dialog->buttonRole(button) == answer) {
                button->click();
                return;
            }
        }
        qFatal("Antwortknopf nicht gefunden");
    });

    trigger();
}

QString wrongTranscript()
{
    return QStringLiteral("Transkript: Idee für Denkzettel — den Bündel-Export erst vorschlagen, "
                          "wenn mindestens fünf Notizen zum selben Thema da sind, sonst wird der "
                          "Fold zugemüllt. Schwelle einstellbar machen.");
}

/** Ersetzt ein Wort so, wie ein Mensch es ersetzt: auswählen, darüber tippen. */
void correct(QPlainTextEdit *editor, const QString &misheard, const QString &meant)
{
    const QTextCursor found = editor->document()->find(misheard);
    if (found.isNull()) {
        qFatal("Wort „%s“ steht nicht im Text", qUtf8Printable(misheard));
    }
    editor->setTextCursor(found);
    QTest::keyClicks(editor, meant);
}

/** Die Notizen aus Wireframe 2a/2b; gibt die Sprachnotiz zurück. */
qint64 fill(Store &store)
{
    addNote(store, QStringLiteral("restic-Backup: prune-Policy prüfen, monatliche Snapshots behalten"),
            QStringLiteral("2026-07-31T14:32:00"));
    const qint64 edited = addNote(store, wrongTranscript(), QStringLiteral("2026-07-31T11:05:00"));
    addNote(store, QStringLiteral("journalctl -u whisperd --since today"),
            QStringLiteral("2026-07-30T21:48:00"));

    analyse(store, edited, QStringLiteral("Software-Ideen"),
            QStringList({QStringLiteral("software-idee"), QStringLiteral("denkzettel"),
                         QStringLiteral("export")}));
    return edited;
}

/** Die Zeile mit dem Ergebnis, damit die Zahlen im Bericht belegt sind. */
void measureLayout(QWidget &window, const QString &state)
{
    QWidget *header = searchOf(window)->parentWidget();
    auto *editor = editorOf(window);
    auto *reader = window.findChild<QTextBrowser *>();
    auto *list = listOf(window);

    qInfo("Raumaufteilung (%s): Fenster %dx%d · Kopfzeile h=%d · Liste b=%d h=%d · Leser sichtbar %d "
          "(y=%d h=%d) · Editor sichtbar %d (y=%d h=%d)",
          qUtf8Printable(state), window.width(), window.height(), header->height(),
          list->width(), list->height(), reader->isVisible() ? 1 : 0,
          reader->mapTo(&window, QPoint(0, 0)).y(), reader->height(), editor->isVisible() ? 1 : 0,
          editor->mapTo(&window, QPoint(0, 0)).y(), editor->height());
}
}

int main(int argc, char **argv)
{
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());

    const QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    if (argc < 2) {
        qFatal("Aufruf: uxshots <Zielverzeichnis>");
    }
    gDirectory = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(gDirectory);

    // 1 — Zustand A und B von Wireframe 2a, der Zugang über F2 und die
    // Rückkehr in die Leseansicht nach dem Speichern.
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
        QTest::qWait(100);
        shoot(window, QStringLiteral("01-lesen.png"));
        measureLayout(window, QStringLiteral("Lesen"));
        shootPart(window, searchOf(window)->parentWidget(), QStringLiteral("02-kopfzeile-lesen.png"));

        qInfo("Leseansicht: Bearbeiten sichtbar %d, Löschen sichtbar %d, Kennzeichen sichtbar %d, "
              "Merkmalszeile sichtbar %d, Fußzeile sichtbar %d",
              buttonNamed(window, QStringLiteral("Bearbeiten"))->isVisible() ? 1 : 0,
              buttonNamed(window, QStringLiteral("Löschen"))->isVisible() ? 1 : 0,
              window.findChildren<QLabel *>().isEmpty() ? -1
                  : [&window] {
                        const QList<QLabel *> labels = window.findChildren<QLabel *>();
                        for (QLabel *label : labels) {
                            if (label->text() == QStringLiteral("wird bearbeitet")) {
                                return label->isVisible() ? 1 : 0;
                            }
                        }
                        return -1;
                    }(),
              buttonNamed(window, QStringLiteral("Speichern"))->parentWidget()->isVisible() ? 1 : 0,
              buttonNamed(window, QStringLiteral("Speichern"))->isVisible() ? 1 : 0);

        // Zugang über F2 — die Tastatur, nicht der Knopf.
        list->setFocus();
        QTest::keyClick(&window, Qt::Key_F2);
        QTest::qWait(150);

        QPlainTextEdit *editor = editorOf(window);
        qInfo("Nach F2: Editor sichtbar %d, Fokus auf Editor %d, Cursor bei %d von %lld, Auswahl %d",
              editor->isVisible() ? 1 : 0, window.focusWidget() == editor ? 1 : 0,
              editor->textCursor().position(), static_cast<long long>(editor->toPlainText().size()),
              editor->textCursor().hasSelection() ? 1 : 0);
        qInfo("Merkmalszeile: Kategorie „%s“, Tags „%s“",
              qUtf8Printable(valueAfter(window, QStringLiteral("Kategorie"))),
              qUtf8Printable(valueAfter(window, QStringLiteral("Tags"))));
        qInfo("Suchfeld im Bearbeiten-Zustand: freigeschaltet %d, Kurzhilfe „%s“, Platzhalter „%s“",
              searchOf(window)->isEnabled() ? 1 : 0, qUtf8Printable(searchOf(window)->toolTip()),
              qUtf8Printable(searchOf(window)->placeholderText()));

        shoot(window, QStringLiteral("03-bearbeiten-frisch.png"));
        measureLayout(window, QStringLiteral("Bearbeiten"));
        shootPart(window, searchOf(window)->parentWidget(), QStringLiteral("04-kopfzeile-bearbeiten.png"));

        // Reihenfolge der beiden Knöpfe der Fußzeile, wie sie im Fenster stehen.
        auto *box = window.findChild<QDialogButtonBox *>();
        Q_ASSERT(box);
        const QList<QAbstractButton *> footerButtons = box->buttons();
        QStringList order;
        for (QAbstractButton *button : footerButtons) {
            order << QStringLiteral("%1 (x=%2)").arg(button->text()).arg(button->mapTo(&window, QPoint(0, 0)).x());
        }
        qInfo("Fußzeile, Knöpfe: %s", qUtf8Printable(order.join(QStringLiteral(" · "))));

        correct(editor, QStringLiteral("Fold"), QStringLiteral("Vault"));
        shoot(window, QStringLiteral("05-bearbeiten-geaendert.png"));

        // Leeres Feld — „Speichern“ darf nicht auslösbar sein (AK 9).
        const QString written = editor->toPlainText();
        editor->clear();
        QTest::qWait(100);
        qInfo("Leeres Feld: Speichern freigeschaltet %d",
              buttonNamed(window, QStringLiteral("Speichern"))->isEnabled() ? 1 : 0);
        shoot(window, QStringLiteral("06-leeres-feld.png"));
        editor->setPlainText(written);
        QTest::qWait(100);

        buttonNamed(window, QStringLiteral("Speichern"))->click();
        QTest::qWait(200);
        qInfo("Nach dem Speichern: Editor sichtbar %d, Fokus auf %s, Text im Speicher „%s…“",
              editorOf(window)->isVisible() ? 1 : 0,
              window.focusWidget() ? window.focusWidget()->metaObject()->className() : "nichts",
              qUtf8Printable(store.note(edited)->content.left(60)));
        qInfo("Nach dem Speichern: needs_reembed=%d, Kategorie „%s“, Tags „%s“, Zustand %d",
              store.note(edited)->needsReembed ? 1 : 0, qUtf8Printable(store.note(edited)->category),
              qUtf8Printable(store.tags(edited).join(QStringLiteral(" · "))),
              static_cast<int>(store.note(edited)->state));
        shoot(window, QStringLiteral("07-lesen-nach-speichern.png"));
    }

    // 2 — Bearbeiten einer Notiz, die kein Analyse-Lauf berührt hat: Kategorie
    // und Tags zeigen „—“ (PO-Festlegung 3).
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        fill(store);

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(1, 0));
        QTest::qWait(100);
        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        QTest::qWait(150);
        qInfo("Notiz ohne Analyse: Kategorie „%s“, Tags „%s“",
              qUtf8Printable(valueAfter(window, QStringLiteral("Kategorie"))),
              qUtf8Printable(valueAfter(window, QStringLiteral("Tags"))));
        shoot(window, QStringLiteral("08-ohne-analyse.png"));
    }

    // 3 — Wächterdialog über den Abbrechen-Knopf (PO-Festlegung 1).
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
        QTest::qWait(100);
        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        correct(editorOf(window), QStringLiteral("Fold"), QStringLiteral("Vault"));

        withDialog(
            window, [&window] { buttonNamed(window, QStringLiteral("Abbrechen"))->click(); },
            QStringLiteral("09-waechter-abbrechen.png"), QMessageBox::RejectRole);
        QTest::qWait(150);
        qInfo("Nach „Abbrechen“ im Dialog: Editor noch offen %d, Text noch geändert %d",
              editorOf(window)->isVisible() ? 1 : 0,
              editorOf(window)->toPlainText().contains(QStringLiteral("Vault")) ? 1 : 0);

        // Und derselbe Dialog beim Auswahlwechsel.
        withDialog(
            window, [list] { list->setCurrentIndex(list->model()->index(1, 0)); },
            QStringLiteral("10-waechter-auswahlwechsel.png"), QMessageBox::RejectRole);
        QTest::qWait(150);
        qInfo("Nach „Abbrechen“ beim Auswahlwechsel: Auswahl auf Zeile %d, Editor offen %d",
              list->currentIndex().row(), editorOf(window)->isVisible() ? 1 : 0);
    }

    // 4 — K2: die gespeicherte Notiz fällt aus der laufenden Trefferliste.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        const qint64 edited = fill(store);

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QLineEdit *search = searchOf(window);
        search->setText(QStringLiteral("Fold"));
        QTest::qWait(200);

        QListView *list = listOf(window);
        qInfo("Suche „Fold“: %d Zeile(n)", list->model()->rowCount());
        // Die einzige Notiz der Trefferliste steht unter ihrem Gruppenkopf.
        list->setCurrentIndex(list->model()->index(1, 0));
        QTest::qWait(150);
        shoot(window, QStringLiteral("11-k2-treffer-vorher.png"));

        const int scrollBefore = list->verticalScrollBar()->value();
        const QRect rowBefore = list->visualRect(list->currentIndex());

        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        correct(editorOf(window), QStringLiteral("Fold"), QStringLiteral("Vault"));
        buttonNamed(window, QStringLiteral("Speichern"))->click();
        QTest::qWait(250);

        const int scrollAfter = list->verticalScrollBar()->value();
        const QRect rowAfter = list->visualRect(list->currentIndex());
        qInfo("K2 nach dem Speichern: %d Zeile(n), Auswahl auf Zeile %d, Rollstand %d → %d, "
              "Auswahlzeile y %d → %d, Suchfeld „%s“, Suchfeld frei %d",
              list->model()->rowCount(), list->currentIndex().row(), scrollBefore, scrollAfter,
              rowBefore.y(), rowAfter.y(), qUtf8Printable(search->text()),
              search->isEnabled() ? 1 : 0);
        qInfo("K2: Treffer im Speicher für „Fold“ %lld, für „Vault“ %lld",
              static_cast<long long>(store.search(QStringLiteral("Fold")).size()),
              static_cast<long long>(store.search(QStringLiteral("Vault")).size()));
        qInfo("K2: Listentext der Notiz jetzt „%s…“", qUtf8Printable(store.note(edited)->content.left(50)));
        shoot(window, QStringLiteral("12-k2-treffer-nachher.png"));

        // Und was die nächste Änderung des Suchbegriffs tut.
        search->setText(QStringLiteral("Fol"));
        QTest::qWait(250);
        qInfo("K2 nach Änderung des Suchbegriffs auf „Fol“: %d Zeile(n)", list->model()->rowCount());
        shoot(window, QStringLiteral("13-k2-nach-suchwechsel.png"));
    }

    // 5 — Raumaufteilung im größeren Fenster.
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        fill(store);

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window, 1400, 900);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(2, 0));
        QTest::qWait(100);
        measureLayout(window, QStringLiteral("Lesen 1400x900"));
        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        QTest::qWait(150);
        measureLayout(window, QStringLiteral("Bearbeiten 1400x900"));
        shoot(window, QStringLiteral("14-bearbeiten-1400x900.png"));
    }

    // 6 — kurze Textnotiz im Bearbeiten-Zustand (AK 10).
    {
        const QTemporaryDir dir;
        Store store(dir.filePath(QStringLiteral("denkzettel.db")));
        store.open();
        addNote(store, QStringLiteral("Mara wegen Wochenende anrufen, Kuchen nicht vergessen"),
                QStringLiteral("2026-07-31T09:12:00"));

        LibraryWindow window(&store);
        window.setReferenceTime(friday());
        open(window);

        QListView *list = listOf(window);
        list->setCurrentIndex(list->model()->index(1, 0));
        QTest::qWait(100);
        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        QTest::qWait(150);
        measureLayout(window, QStringLiteral("kurze Textnotiz"));
        shoot(window, QStringLiteral("15-kurze-textnotiz.png"));
    }

    // 7 — springt der Textbereich beim Zustandswechsel? Gemessen am selben
    // Fenster, am Stapel selbst, nicht an seinen beiden Seiten.
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
        QTest::qWait(150);

        QWidget *stack = editorOf(window)->parentWidget();
        QPushButton *edit = buttonNamed(window, QStringLiteral("Bearbeiten"));
        const int stackTopReading = stack->mapTo(&window, QPoint(0, 0)).y();
        const int stackHeightReading = stack->height();
        const int buttonHeight = edit->height();

        edit->click();
        QTest::qWait(150);

        QLabel *badge = nullptr;
        const QList<QLabel *> labels = window.findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("wird bearbeitet")) {
                badge = label;
            }
        }
        Q_ASSERT(badge);

        const int stackTopEditing = stack->mapTo(&window, QPoint(0, 0)).y();
        qInfo("Zustandswechsel: Textbereich oben %d → %d (Sprung %d px), Höhe %d → %d · "
              "Knopfhöhe %d, Kennzeichenhöhe %d",
              stackTopReading, stackTopEditing, stackTopEditing - stackTopReading,
              stackHeightReading, stack->height(), buttonHeight, badge->height());
        shoot(window, QStringLiteral("16-sprung-bearbeiten.png"));

        buttonNamed(window, QStringLiteral("Abbrechen"))->click();
        QTest::qWait(150);
        qInfo("Zurück im Lesezustand: Textbereich oben %d, Höhe %d",
              stack->mapTo(&window, QPoint(0, 0)).y(), stack->height());
        shoot(window, QStringLiteral("17-sprung-lesen.png"));
    }

    // 8 — Esc über dem Wächterdialog: die harmlose Antwort? Und der dritte
    // Ausweg, das Fensterschließen.
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
        QTest::qWait(100);
        buttonNamed(window, QStringLiteral("Bearbeiten"))->click();
        correct(editorOf(window), QStringLiteral("Fold"), QStringLiteral("Vault"));

        QTimer::singleShot(0, qApp, [] {
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
            QTest::qWaitForWindowExposed(dialog);
            QTest::qWait(150);
            QTest::keyClick(dialog, Qt::Key_Escape);
        });

        window.close();
        QTest::qWait(200);
        qInfo("Esc über dem Dialog (ausgelöst durchs Fensterschließen): Fenster offen %d, "
              "Editor offen %d, Änderung noch da %d, Text im Speicher unverändert %d",
              window.isVisible() ? 1 : 0, editorOf(window)->isVisible() ? 1 : 0,
              editorOf(window)->toPlainText().contains(QStringLiteral("Vault")) ? 1 : 0,
              store.note(edited)->content.contains(QStringLiteral("Fold")) ? 1 : 0);
    }

    return 0;
}
