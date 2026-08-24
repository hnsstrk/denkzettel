#include "capture/capturewindow.h"
#include "capture/textareaheight.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QLayout>
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

    void windowShrinksBackOnAShownWindow();

    void staysUsableWithoutADesktopTheme();
    void survivesAnUnresolvableDesktopTheme();

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
    // for itself, while a cleared one measures the fallback the user gets —
    // his own `plasmarc` carries no `[Theme] name` either.
    //
    // This is no guard against the crash of 12.08.2026, and it is not meant as
    // one. It keeps left-over state from deciding the run; a theme name that no
    // longer resolves is measured by survivesAnUnresolvableDesktopTheme() below,
    // in a process with a `plasmarc` and a HOME of its own (issue #107).
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

void CaptureTest::windowShrinksBackOnAShownWindow()
{
    // SPEC 3 lets the window grow from five lines to eight — so the way back
    // belongs to it as well. On a **shown** window it did not happen (issue
    // #79): when the layout is activated it writes its total minimum onto the
    // window, `resize()` is clamped by that minimum, and the minimum of the
    // eight-line state stood until the next activation. From the first longer
    // note onwards the window then opened at the height of the longest note of
    // the session for the rest of it.
    //
    // The window has to be shown for this, and that is the whole point of the
    // criterion: on a hidden one no minimum has been applied yet, the fault
    // cannot occur, and a check built that way is green over the unfixed bug —
    // which is what the deleted `windowFollowsTheTextHeight()` was.
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    const QString eightLines =
        QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht");

    // The control, and it is named as one: hidden, the way back has always
    // worked. It stands here so that a red run says which of the two roads
    // broke, not merely that something did.
    const int hiddenResting = m_window->height();
    text->setPlainText(eightLines);
    QVERIFY(m_window->height() > hiddenResting);
    text->clear();
    QCOMPARE(m_window->height(), hiddenResting);

    // And now the road the user takes. `showCapture()` and not `show()`:
    // everything the window binds after the mapping hangs on it.
    m_window->showCapture();
    QVERIFY(QTest::qWaitForWindowExposed(m_window.get()));

    const int resting = m_window->height();
    // The anchor from outside, so the two comparisons below cannot both be
    // wrong in the same direction: at rest the text area is exactly the five
    // lines SPEC 3 promises — the guarantee the document margin must not move
    // either.
    const int chrome = 2 * qRound(text->document()->documentMargin()) + 2 * text->frameWidth();
    QCOMPARE(text->height(), capture::MinTextLines * text->fontMetrics().lineSpacing() + chrome);

    text->setPlainText(eightLines);
    QCoreApplication::processEvents();
    const int grown = m_window->height();
    QVERIFY2(grown > resting,
             qPrintable(QStringLiteral("grown %1 <= resting %2").arg(grown).arg(resting)));

    text->clear();
    QCoreApplication::processEvents();
    QCOMPARE(m_window->height(), resting);
    QCOMPARE(text->height(), capture::MinTextLines * text->fontMetrics().lineSpacing() + chrome);

    // Second face of the finding: after Esc and a second opening the window
    // stands at the resting height and not at the height of the longest note
    // ever typed. Esc through the key and not through discard(), because that
    // is the road the user takes.
    text->setPlainText(eightLines);
    QCoreApplication::processEvents();
    QTest::keyClick(text, Qt::Key_Escape);
    QVERIFY(!m_window->isVisible());
    m_window->showCapture();
    QVERIFY(QTest::qWaitForWindowExposed(m_window.get()));
    QCOMPARE(m_window->height(), resting);

    // Third face: the window follows a font made **smaller** as well, not only
    // the text area. The text area was right all along — issue #56 AK 1 speaks
    // of it and is literally fulfilled — while the window kept the old height
    // and left a 223 px field standing in a 299 px window.
    const int systemPointSize = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
    QFont font = text->font();
    font.setPointSize(24);
    text->setFont(font);
    QCoreApplication::processEvents();
    const int large = m_window->height();
    QVERIFY2(large > resting,
             qPrintable(QStringLiteral("large %1 <= resting %2").arg(large).arg(resting)));

    font.setPointSize(systemPointSize);
    text->setFont(font);
    QCoreApplication::processEvents();
    QCOMPARE(m_window->height(), resting);
}

void CaptureTest::staysUsableWithoutADesktopTheme()
{
    // Outside a Plasma session `dialogs/background` is not there (AK 8), and
    // the demand is the modest one the criterion makes: no crash, and a window
    // one can still see and type into. Not transparent — which is what a
    // hull-less window with a translucent background would otherwise be.
    //
    // The state cannot be produced inside this process: an unknown theme name
    // does not lead there either, it is turned into `default` before KSvg sees
    // it (issue #107) and the hull renders as usual. A test that set one would
    // be a test in which the fault cannot occur. What produces it is an
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
                 "The window is not active — the two comparisons below then do not "
                 "cover the isValid() guard of the focus layer.");

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

void CaptureTest::survivesAnUnresolvableDesktopTheme()
{
    // A `plasmarc` that names a desktop theme whose package is gone — the
    // configuration of everyone who sets a theme and later removes it. Until
    // issue #107 the window died on the way up, and the place of the crash was
    // not the place of the fault: KSvg keys its image set by the name it is
    // **given** and removes it again by the name it **resolved**, so a name it
    // cannot resolve leaves the table pointing at freed memory and the next
    // set of that name walks into it. Hence two windows below and not one —
    // with a single one the fault does not show at all.
    //
    // The name stands in the file before either window is built, and that is
    // the whole point of the setup (AK 3): handed to reloadDesktopTheme() as
    // an argument the fault measurably does **not** occur, so a test built
    // that way would stand green over an unfixed bug.
    //
    // A process of its own for two reasons: the fixture of this set has
    // already read `plasmarc` before any test function runs, and a crash here
    // would take the whole run with it instead of failing one function.
    if (qEnvironmentVariableIsSet("DENKZETTEL_TEST_UNRESOLVABLE_DESKTOP_THEME")) {
        const QString plasmarc =
            QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
            + QStringLiteral("/plasmarc");
        QFile file(plasmarc);
        QVERIFY2(file.open(QIODevice::WriteOnly | QIODevice::Truncate), qPrintable(plasmarc));
        file.write("[Theme]\nname=denkzettel-kein-solches-theme\n");
        file.close();

        for (int round = 0; round < 2; ++round) {
            CaptureWindow window(m_store.get());
            auto *text = window.findChild<QPlainTextEdit *>();
            QVERIFY(text);

            const QImage picture = shot(window);
            QVERIFY(window.isVisible());
            QVERIFY(picture.width() > 0);
            // Not a see-through ghost: the data path is full here, so the
            // fallback lands on a theme that draws, and the middle of the
            // window is covered whatever theme that turns out to be.
            QCOMPARE(qAlpha(picture.pixel(picture.width() / 2, picture.height() / 2)), 255);

            text->setPlainText(QStringLiteral("geht trotzdem"));
            QCOMPARE(text->toPlainText(), QStringLiteral("geht trotzdem"));

            // AK 2: the road the watch on `plasmarc` takes, on a window that is
            // already standing. Same door, same name, no argument.
            window.reloadDesktopTheme();
        }
        return;
    }

    // A HOME of its own, and not only out of tidiness: QStandardPaths in test
    // mode hangs its directories under it, so the child writes its `plasmarc`
    // there and nowhere near the developer's own — and a child that dies leaves
    // nothing behind that the next run would read.
    const QTemporaryDir home;
    QVERIFY(home.isValid());

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("DENKZETTEL_TEST_UNRESOLVABLE_DESKTOP_THEME"),
                       QStringLiteral("1"));
    environment.insert(QStringLiteral("HOME"), home.path());

    QProcess child;
    child.setProcessEnvironment(environment);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(QCoreApplication::applicationFilePath(),
                {QStringLiteral("survivesAnUnresolvableDesktopTheme")});

    QVERIFY(child.waitForFinished(60000));
    QVERIFY2(child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0,
             child.readAll().constData());
}

QTEST_MAIN(CaptureTest)

#include "capturetest.moc"
