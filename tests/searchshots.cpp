#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <QApplication>
#include <QDir>
#include <QLineEdit>
#include <QTemporaryDir>
#include <QTest>

/**
 * Writes the picture series of the search states for the handover (DoD 2).
 *
 * Not a test — a picture maker. It is built and run by hand, so it stays out
 * of `add_test()`: a picture nobody looks at proves nothing, and a failing
 * screenshot writer must not turn the suite red.
 *
 * Usage: searchshots <target directory>
 */
namespace
{
Note noteAt(const QString &content, const QString &isoDateTime)
{
    Note note;
    note.content = content;
    note.createdAt = QDateTime::fromString(isoDateTime, Qt::ISODate);
    note.type = Note::Type::Text;
    note.state = Note::State::New;
    return note;
}

void shoot(LibraryWindow &window, const QString &file)
{
    // Qt fades the clear button of a QLineEdit in and out over about 160 ms.
    // A shorter wait catches it half faded and puts a button into the picture
    // that the running window does not show.
    QTest::qWait(500);
    if (!window.grab().save(file)) {
        qFatal("Bild %s ließ sich nicht schreiben", qUtf8Printable(file));
    }
    qInfo("geschrieben: %s", qUtf8Printable(file));
}
}

int main(int argc, char **argv)
{
    const QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));

    if (argc < 2) {
        qFatal("Aufruf: searchshots <Zielverzeichnis>");
    }
    const QString target = QString::fromLocal8Bit(argv[1]);
    QDir().mkpath(target);

    const QTemporaryDir data;
    Store store(data.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        qFatal("Store: %s", qUtf8Printable(store.lastError()));
    }

    store.addNote(noteAt(QStringLiteral("Backup der Fotos prüfen\nDie externe Platte liegt im Schrank."),
                         QStringLiteral("2026-07-31T09:15:00")));
    store.addNote(noteAt(QStringLiteral("Milch kaufen"), QStringLiteral("2026-07-31T08:02:00")));
    store.addNote(noteAt(QStringLiteral("Bücher über Straßenbahnen ansehen\nBesonders die Baureihen der 60er."),
                         QStringLiteral("2026-07-30T19:40:00")));
    store.addNote(noteAt(QStringLiteral("Backup vom Vortag kontrollieren"),
                         QStringLiteral("2026-07-30T07:30:00")));
    store.addNote(noteAt(QStringLiteral("Besprechung mit Ada zur KI-Pipeline"),
                         QStringLiteral("2026-07-27T11:00:00")));

    LibraryWindow window(&store);
    window.setReferenceTime(QDateTime::fromString(QStringLiteral("2026-07-31T16:00:00"), Qt::ISODate));
    window.resize(900, 600);
    window.showLibrary();
    if (!QTest::qWaitForWindowExposed(&window)) {
        qFatal("Fenster wurde nicht sichtbar");
    }

    auto *search = window.findChild<QLineEdit *>();
    if (!search) {
        qFatal("Suchfeld nicht gefunden");
    }

    shoot(window, target + QStringLiteral("/1-volle-liste.png"));

    search->setText(QStringLiteral("Backup"));
    shoot(window, target + QStringLiteral("/2-trefferliste-mit-gruppen.png"));

    search->setText(QStringLiteral("bahn"));
    shoot(window, target + QStringLiteral("/3-treffer-wortteil.png"));

    search->setText(QStringLiteral("KI"));
    shoot(window, target + QStringLiteral("/4-treffer-kurzer-begriff.png"));

    search->setText(QStringLiteral("Fahrrad"));
    shoot(window, target + QStringLiteral("/5-keine-treffer.png"));

    search->clear();
    shoot(window, target + QStringLiteral("/6-feld-geleert-volle-liste.png"));

    return 0;
}
