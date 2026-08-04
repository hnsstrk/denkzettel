// Sichtprüfung zu Issue #59 am gebauten Stand, unter dem echten Compositor.
//
// Der Unit-Test misst denselben Weg unter QT_QPA_PLATFORM=offscreen. Diese
// Probe beantwortet die Frage, die offscreen offen lässt: Kommt der
// ActivationChange unter Wayland genauso an — und bleibt der Rollwert dann
// stehen? Sie läuft nicht über D-Bus und nicht auf der Datenbank des Kunden,
// sondern legt sich eine eigene an.
//
// Übersetzen und laufen lassen: siehe bericht.md, Abschnitt „Sichtprüfung".

#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QListView>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTextStream>

namespace
{

void pump(int ms)
{
    QElapsedTimer clock;
    clock.start();
    while (clock.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}

bool waitFor(const std::function<bool()> &done, int timeoutMs = 5000)
{
    QElapsedTimer clock;
    clock.start();
    while (!done() && clock.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    return done();
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("probe59.sqlite")));
    if (!store.open()) {
        out << "Datenbank ließ sich nicht öffnen: " << store.lastError() << "\n";
        return 1;
    }

    const auto add = [&store](const QString &text, const QString &iso) {
        Note note;
        note.createdAt = QDateTime::fromString(iso, Qt::ISODate);
        note.content = text;
        store.addNote(note);
    };
    for (int hour = 8; hour < 16; ++hour) {
        add(QStringLiteral("von heute, %1 Uhr").arg(hour),
            QStringLiteral("2026-07-31T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }
    for (int hour = 8; hour < 16; ++hour) {
        add(QStringLiteral("von gestern, %1 Uhr").arg(hour),
            QStringLiteral("2026-07-30T%1:00:00").arg(hour, 2, 10, QLatin1Char('0')));
    }

    LibraryWindow window(&store);
    window.setReferenceTime(QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate));
    window.resize(900, 600);
    window.showLibrary();
    if (!waitFor([&window] { return window.isVisible() && window.windowHandle(); })) {
        out << "Fenster kam nicht auf den Schirm\n";
        return 1;
    }
    pump(600);

    auto *list = window.findChild<QListView *>();
    auto *model = qobject_cast<NoteListModel *>(list->model());
    const QModelIndex selected = model->index(model->rowOfNote(12));
    list->setCurrentIndex(selected);
    pump(200);

    // Der Nutzer rollt zurück nach oben, weg von seiner Auswahl.
    list->verticalScrollBar()->setValue(0);
    pump(200);
    const int before = list->verticalScrollBar()->value();
    out << "Auswahl in Zeile " << selected.row() << ", Rollwert nach dem Zurückrollen: "
        << before << "\n";
    out << "Auswahl im Bild: "
        << (list->viewport()->rect().intersects(list->visualRect(selected)) ? "ja" : "nein")
        << "\n";

    // Fenster verlassen und wiederkommen — der Alt-Tab-Weg der Story.
    QWidget elsewhere;
    elsewhere.resize(300, 200);
    elsewhere.setWindowTitle(QStringLiteral("Probe 59 — anderes Fenster"));
    elsewhere.show();
    elsewhere.raise();
    elsewhere.activateWindow();
    const bool leftTheWindow = waitFor([&window] { return !window.isActiveWindow(); });
    pump(400);

    // Zurück geht es über das Schließen des anderen Fensters, nicht über
    // activateWindow(): unter Wayland reicht ein Prozess sich den Fokus nicht
    // selbst zu, dazu bräuchte er ein xdg-activation-Token. Der Compositor gibt
    // ihn von sich aus zurück, wenn das obenauf liegende Fenster verschwindet —
    // und genau das ist der Weg, den ein Alt-Tab auch nimmt.
    elsewhere.close();
    window.raise();
    window.activateWindow();
    const bool cameBack = waitFor([&window] { return window.isActiveWindow(); });
    pump(400);

    out << "Fenster verlassen: " << (leftTheWindow ? "ja" : "nein")
        << ", zurückgekommen: " << (cameBack ? "ja" : "nein") << "\n";
    out << "Rollwert nach der Rückkehr: " << list->verticalScrollBar()->value() << "\n";
    out << "Auswahl unverändert: "
        << (list->currentIndex() == selected ? "ja" : "nein") << "\n";
    out << (list->verticalScrollBar()->value() == before && leftTheWindow && cameBack
                ? "ERGEBNIS: Stelle gehalten\n"
                : "ERGEBNIS: Stelle verloren oder Aktivierung nicht angekommen\n");

    return 0;
}
