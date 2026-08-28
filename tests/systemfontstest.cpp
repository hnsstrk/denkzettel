#include "capture/capturewindow.h"
#include "platform/systemfonts.h"
#include "store/note.h"
#include "store/store.h"
#include "ui/librarywindow.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFont>
#include <QLabel>
#include <QListView>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <algorithm>

/**
 * That a change of the system font still reaches both windows while they are
 * running (issues #68 and #110).
 *
 * A set of its own, and the reason is the isolation: `kdeglobals` has to be
 * **rewritten under the running process**, and the only file the two windows
 * read is the one `QStandardPaths` names. Pointing that somewhere else has to
 * happen before `QApplication` is built — the KDE platform theme opens
 * `kdeglobals` while the application comes up, and `KSharedConfig` hands that
 * same instance out to everything afterwards. A `HOME` set in a test function
 * would be set too late, and the run would rewrite the user's own
 * configuration. Hence the `main()` at the end of this file instead of
 * `QTEST_MAIN`.
 *
 * Why this is a test and not an image: the fault it guards is the one issue
 * #110 could have introduced. The fonts are held from one change of the file
 * to the next now; a cache that is never let go of makes the list fast and the
 * guarantee of #68 silently untrue — the window would keep the old type until
 * the next login, and nobody looks at a font size they did not just change.
 */
class SystemFontsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void bothWindowsFollowAFontChangeUnderTheRunningProcess();
};

namespace
{
/** Point size the small type stands at before and after the change. */
constexpr int SmallBefore = 8;
constexpr int SmallAfter = 13;

/** And the general type, which the group heads of the list are measured from. */
constexpr int GeneralBefore = 10;
constexpr int GeneralAfter = 15;

QString kdeglobalsPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QStringLiteral("/kdeglobals");
}

/**
 * Writes the two font keys the way a foreign process writes them — a plain
 * file, not through the `KSharedConfig` this process holds open.
 *
 * Through that instance the new value would already stand in memory, and the
 * run would then pass over a unit that never re-read anything.
 *
 * `QSaveFile`, so the file is **replaced** and not overwritten in place — the
 * road KConfig itself takes when Plasma saves a font, and the one thing that
 * makes this check independent of the clock. KDirWatch reports a watched file
 * as dirty on a new inode, but on an overwrite in place it compares
 * `qMax(st_ctime, st_mtime)` in whole seconds: measured 28.08.2026, a rewrite
 * in place inside the same second was reported to nobody, and the check stood
 * red with the fix in place (CLAUDE.md, "Runs that prove nothing", 22).
 */
bool writeFonts(int generalPointSize, int smallPointSize)
{
    QSaveFile file(kdeglobalsPath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    // The family is spelled out rather than asked of the font database: what
    // is compared here is the point size, and QFont keeps the size of a family
    // that no system has installed just as well.
    const QString entry = QStringLiteral("Noto Sans,%1,-1,5,400,0,0,0,0,0,0,0,0,0,0,1");
    file.write(QStringLiteral("[General]\nfont=%1\nsmallestReadableFont=%2\n")
                   .arg(entry.arg(generalPointSize), entry.arg(smallPointSize))
                   .toUtf8());

    return file.commit();
}

/**
 * Every label of `window` that carries a font of its own at `pointSize`.
 *
 * `marker` is the property `LibraryWindow` writes on the labels it sets a font
 * on, and the one its own handler looks them up by — asked for here so that
 * this check keeps hold of the same labels the window does. A label that
 * merely happens to stand at the same size, having inherited the application
 * font, would follow a change by itself and does not belong in the comparison.
 * The capture window keeps no such mark: it has two labels and names both.
 */
QList<QLabel *> labelsAt(const QWidget &window, int pointSize, const char *marker = nullptr)
{
    QList<QLabel *> found;
    const QList<QLabel *> labels = window.findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (marker && !label->property(marker).toBool()) {
            continue;
        }
        if (label->font().pointSize() == pointSize) {
            found.append(label);
        }
    }

    return found;
}

bool allAt(const QList<QLabel *> &labels, int pointSize)
{
    return std::all_of(labels.cbegin(), labels.cend(), [pointSize](const QLabel *label) {
        return label->font().pointSize() == pointSize;
    });
}
}

void SystemFontsTest::bothWindowsFollowAFontChangeUnderTheRunningProcess()
{
    QVERIFY2(writeFonts(GeneralBefore, SmallBefore), qPrintable(kdeglobalsPath()));

    // The watch goes on before the windows are built, exactly as main.cpp does
    // it — and it is what allows the two read functions to hold anything at
    // all (issue #110).
    platform::followSystemFonts(qApp);

    QCOMPARE(platform::smallestReadableFont().pointSize(), SmallBefore);
    QCOMPARE(platform::generalFont().pointSize(), GeneralBefore);

    const QTemporaryDir data;
    QVERIFY(data.isValid());
    Store store(data.filePath(QStringLiteral("denkzettel.db")));
    QVERIFY2(store.open(), qPrintable(store.lastError()));

    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.content = QStringLiteral("Größe der Schrift");
    QVERIFY(store.addNote(note).has_value());

    CaptureWindow capture(&store);
    capture.showCapture();
    QVERIFY(QTest::qWaitForWindowExposed(&capture));

    LibraryWindow library(&store);
    library.showLibrary();
    QVERIFY(QTest::qWaitForWindowExposed(&library));

    // Shown, and not merely built: a row height comes out of the layout, and a
    // hidden list lays nothing out (CLAUDE.md, "Runs that prove nothing", 14).
    auto *list = library.findChild<QListView *>();
    QVERIFY(list);
    const QModelIndex first = list->model()->index(0, 0);
    QVERIFY(first.isValid());
    const int rowBefore = list->visualRect(first).height();
    QVERIFY(rowBefore > 0);

    // The labels whose font was set on them by hand: those are the ones that
    // do not follow the application font by themselves, and the ones #68 is
    // about. Read out of the standing windows rather than counted, so that a
    // window which stops building them fails here instead of passing on an
    // empty list.
    const QList<QLabel *> captureLabels = labelsAt(capture, SmallBefore);
    const QList<QLabel *> libraryLabels = labelsAt(library, SmallBefore, "denkzettel_smallFont");
    QVERIFY(!captureLabels.isEmpty());
    QVERIFY(!libraryLabels.isEmpty());

    QVERIFY2(writeFonts(GeneralAfter, SmallAfter), qPrintable(kdeglobalsPath()));

    // KDirWatch carries the change over the event loop, and where inotify is
    // not available it polls — hence the wait rather than a single comparison.
    QTRY_VERIFY(allAt(captureLabels, SmallAfter));
    QTRY_VERIFY(allAt(libraryLabels, SmallAfter));
    QTRY_COMPARE(platform::smallestReadableFont().pointSize(), SmallAfter);
    QTRY_COMPARE(platform::generalFont().pointSize(), GeneralAfter);

    // And the list measured its rows anew. The delegate asks for both fonts in
    // sizeHint(), so a row that stayed at its old height is the one thing that
    // a held font would leave behind while every label had already moved.
    QTRY_VERIFY(list->visualRect(first).height() > rowBefore);
}

int main(int argc, char *argv[])
{
    // A HOME of its own, in place before QApplication — see the class comment.
    // It lives to the end of main(), so the directory outlasts every read of
    // it.
    const QTemporaryDir home;
    if (!home.isValid()) {
        qFatal("no temporary home");
    }
    const QString config = home.path() + QStringLiteral("/.config");
    if (!QDir().mkpath(config)) {
        qFatal("no configuration directory");
    }
    qputenv("HOME", home.path().toLocal8Bit());
    // Explicitly, and not left to HOME: an XDG_CONFIG_HOME inherited from the
    // session would outrank it and point the run back at the user's own files.
    qputenv("XDG_CONFIG_HOME", config.toLocal8Bit());

    const QApplication app(argc, argv);

    // Without the domain every i18n() call in the two windows warns.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    SystemFontsTest test;

    return QTest::qExec(&test, argc, argv);
}

#include "systemfontstest.moc"
