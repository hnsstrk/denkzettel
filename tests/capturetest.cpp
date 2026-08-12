#include "capture/capturewindow.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPlainTextEdit>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

/**
 * Unit tests of the growth logic and of the saving path of the capture window
 * (SPEC 3). The window's behaviour under the compositor — focus, placement,
 * appearance — belongs to the manual checklist (SPEC 16).
 */
class CaptureTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void savesTextOnControlReturn();
    void keepsBlankTextOutOfTheStore();
    void discardsTextOnEscape();

    void staysUsableWithoutADesktopTheme();

private:
    QPlainTextEdit *textArea() const;
    static QImage shot(QWidget &window);
    /** How far into the top row of the picture the hull is still transparent. */
    static int cornerRun(const QImage &picture);

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    std::unique_ptr<CaptureWindow> m_window;
};

namespace
{
/** The middle of the text area: the field's own surface, clear of its border. */
QPoint fieldSurface(const QPlainTextEdit *text)
{
    return text->geometry().center();
}

/** The outermost pixel of the field's left border, where the graphic draws its edge. */
QPoint fieldEdge(const QPlainTextEdit *text)
{
    return QPoint(text->x(), text->y() + text->height() / 2);
}

/** A logical point of the window in the pixels of a grabbed picture. */
QPoint inPicture(const QPoint &logical)
{
    return QPoint(qRound(logical.x() * qApp->devicePixelRatio()),
                  qRound(logical.y() * qApp->devicePixelRatio()));
}
}

void CaptureTest::initTestCase()
{
    // Without the domain every i18n() call in the window warns.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // The window reads its desktop theme out of `plasmarc` and hangs a KDirWatch
    // on that file. Test mode points QStandardPaths at a directory of the test's
    // own, so neither road reaches the developer's own configuration; the
    // directory has to exist before the watch is set.
    QStandardPaths::setTestModeEnabled(true);
    const QString configDirectory =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QDir().mkpath(configDirectory);

    // And then the file goes, before the first window reads it. This run writes
    // no `plasmarc` — measured — so whatever is found there was left by an
    // earlier one, and until 11.08.2026 a test did leave a theme name behind.
    // A run whose outcome depends on what an earlier run left lying around
    // measures the earlier run and not the code.
    //
    // Cleared rather than filled with a theme of our own, and the difference
    // matters: a run that writes itself a known theme measures a state it built
    // for itself, while a cleared one measures the fallback the customer gets —
    // his own `plasmarc` carries no `[Theme] name` either.
    //
    // This is no guard against the fault behind the crash of 12.08.2026, and it
    // is not meant as one. It keeps left-over state from deciding the run; that
    // a theme name which no longer resolves takes the process down is a fault
    // of its own and has an issue of its own.
    QFile::remove(configDirectory + QStringLiteral("/plasmarc"));
}

void CaptureTest::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    m_store = std::make_unique<Store>(m_dir->filePath(QStringLiteral("denkzettel.db")));
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    m_window = std::make_unique<CaptureWindow>(m_store.get());
}

void CaptureTest::cleanup()
{
    m_window.reset();
    m_store.reset();
    m_dir.reset();
}

QPlainTextEdit *CaptureTest::textArea() const
{
    return m_window->findChild<QPlainTextEdit *>();
}

void CaptureTest::savesTextOnControlReturn()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    m_window->show();
    QVERIFY(m_window->isVisible());

    text->setPlainText(QStringLiteral("  Bücher über Straßenbahnen ansehen  "));
    QTest::keyClick(text, Qt::Key_Return, Qt::ControlModifier);

    // The first note of a fresh database gets id 1.
    const std::optional<Note> stored = m_store->note(1);
    QVERIFY2(stored.has_value(), qPrintable(m_store->lastError()));
    QCOMPARE(stored->content, QStringLiteral("Bücher über Straßenbahnen ansehen"));
    QCOMPARE(stored->type, Note::Type::Text);
    QCOMPARE(stored->state, Note::State::New);
    QVERIFY(stored->createdAt.isValid());

    QVERIFY(text->toPlainText().isEmpty());
    QVERIFY(!m_window->isVisible());
}

void CaptureTest::keepsBlankTextOutOfTheStore()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    text->setPlainText(QStringLiteral("   \n  "));
    QTest::keyClick(text, Qt::Key_Return, Qt::ControlModifier);

    QVERIFY(!m_store->note(1).has_value());
    QVERIFY(text->toPlainText().isEmpty());
}

void CaptureTest::discardsTextOnEscape()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    m_window->show();
    QVERIFY(m_window->isVisible());

    text->setPlainText(QStringLiteral("doch nicht"));
    QTest::keyClick(text, Qt::Key_Escape);

    QVERIFY(!m_store->note(1).has_value());
    QVERIFY(text->toPlainText().isEmpty());
    QVERIFY(!m_window->isVisible());
}

QImage CaptureTest::shot(QWidget &window)
{
    // Shown, then let through: a window that was never shown has no laid-out
    // layout at all — every child sits at zero — and one that was only resized
    // gets that resize as a posted event, so the picture would be one step
    // behind the size: the hull of the previous height on the geometry of the
    // current one. Both were measured on this window.
    window.show();
    QCoreApplication::processEvents();

    return window.grab().toImage();
}

int CaptureTest::cornerRun(const QImage &picture)
{
    int x = 0;
    while (x < picture.width() && qAlpha(picture.pixel(x, 0)) == 0) {
        ++x;
    }
    return x;
}

void CaptureTest::staysUsableWithoutADesktopTheme()
{
    // Outside a Plasma session `dialogs/background` is not there (AK 8), and
    // the demand is the modest one the criterion makes: no crash, and a window
    // one can still see and type into. Not transparent — which is what a
    // hull-less window with a translucent background would otherwise be.
    //
    // The state cannot be produced inside this process, and that is measured:
    // an unknown theme name does **not** leave KSvg empty-handed, it falls back
    // to `default` and the hull renders as usual. A test that set one would be
    // a test in which the fault cannot occur. What produces it is an
    // environment with no theme files on the data path — hence a process of its
    // own, which the branch below runs.
    if (qEnvironmentVariableIsSet("DENKZETTEL_TEST_WITHOUT_DESKTOP_THEME")) {
        QPlainTextEdit *text = textArea();
        QVERIFY(text);

        // Every road the standing window takes, not only the one it was built
        // on: reading plasmarc, and being handed a name that nothing answers to.
        // The name below is invented and answers to nothing anywhere — which is
        // its whole job here, and on the empty data path of this process no name
        // would resolve anyway.
        m_window->reloadDesktopTheme();
        m_window->reloadDesktopTheme(QStringLiteral("denkzettel-kein-solches-theme"));
        m_window->showCapture();

        const QImage picture = shot(*m_window);
        QVERIFY(picture.width() > 0);
        // No hull, so no corner is cut away — and no pixel is left transparent.
        QCOMPARE(cornerRun(picture), 0);
        QCOMPARE(picture.pixelColor(0, 0), m_window->palette().color(QPalette::Window));
        QCOMPARE(picture.pixelColor(picture.width() / 2, picture.height() / 2),
                 m_window->palette().color(QPalette::Window));

        // And the window is the active one while that picture is taken, which
        // is what makes the two comparisons below say anything about the focus
        // layer as well (issue #102, AK 6). An inactive window draws no layer
        // whatever its `isValid()` says, so without this line the guard on the
        // fourth frame would go unmeasured and the run would read green.
        QVERIFY2(m_window->isActiveWindow() && m_window->hasFocus(),
                 "Das Fenster ist nicht aktiv — dann prüfen die beiden Vergleiche unten die "
                 "isValid()-Wache der Fokusschicht nicht mit.");

        // And no field either (issue #100, AK 8), and no focus layer over it
        // (issue #102, AK 6). `widgets/lineedit` is as absent here as
        // `dialogs/background` is, and a build that drew either all the same
        // would put a graphic on a window that has no theme — the middle of the
        // text area and its left edge are where the two would stand.
        QCOMPARE(picture.pixelColor(inPicture(fieldSurface(text))),
                 m_window->palette().color(QPalette::Window));
        QCOMPARE(picture.pixelColor(inPicture(fieldEdge(text))),
                 m_window->palette().color(QPalette::Window));

        text->setPlainText(QStringLiteral("geht trotzdem"));
        QCOMPARE(text->toPlainText(), QStringLiteral("geht trotzdem"));
        return;
    }

    const QTemporaryDir empty;
    QVERIFY(empty.isValid());

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("DENKZETTEL_TEST_WITHOUT_DESKTOP_THEME"), QStringLiteral("1"));
    environment.insert(QStringLiteral("XDG_DATA_DIRS"), empty.path());
    environment.insert(QStringLiteral("XDG_DATA_HOME"), empty.path());

    QProcess child;
    child.setProcessEnvironment(environment);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(QCoreApplication::applicationFilePath(),
                {QStringLiteral("staysUsableWithoutADesktopTheme")});

    QVERIFY(child.waitForFinished(60000));
    QVERIFY2(child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0,
             child.readAll().constData());
}

QTEST_MAIN(CaptureTest)

#include "capturetest.moc"
