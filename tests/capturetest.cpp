#include "capture/audiorecorder.h"
#include "capture/capturewindow.h"
#include "capture/recordingwindow.h"
#include "capture/textareaheight.h"
#include "capture/textcontrast.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QApplication>
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QLayout>
#include <QPlainTextEdit>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

#include <cmath>
#include <memory>
#include <numbers>

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

    void ranksTheTwoWritingsOnThePoorerGround();

    void savesTextOnControlReturn();
    void keepsBlankTextOutOfTheStore();
    void discardsTextOnEscape();

    void windowShrinksBackOnAShownWindow();

    void staysUsableWithoutADesktopTheme();
    void survivesAnUnresolvableDesktopTheme();

    void writesOpusIntoOgg();
    void stopsAtTheLimit();
    void stopsAtTheLimitOnTheClockAlone();
    void keepsTheTailOfTheRecording();
    void cancellingDeletesTheFile();
    void dropsWhatItCannotSaveOnTheWayOut();
    void dropsARecordingStoppedButNotYetClosed();
    void cancellingOvertakesAStopInFlight();
    void stopsAtTheLimitWithNoBufferAtAll();
    void announcesAFailedRecording();
    void theLevelFollowsWhatTheSignalDoes();

    void makesTheNoteWhenTheRecordingIsFinished();
    void discardsTheRecordingOnEscape();
    void keepsTheRecordingWhenTheNoteCannotBeStored();
    void armsNothingWhenTheRecordingBreaksOffAtTheStart();

private:
    QPlainTextEdit *textArea() const;
    /**
     * Hands the recorder `milliseconds` of a tone of the test's own, in buffers
     * of 100 ms, and returns early once the recorder has stopped itself.
     *
     * Never the microphone (the user's instruction of 28.08.2026): this machine
     * is the one they work at, this repository is public, and a recording that
     * has been made cannot be unmade. The measurement brings its own signal.
     *
     * **No case below calls AudioRecorder::start()**, only startEncoder() and
     * encode() — that is what keeps the device shut, and it is a property to
     * keep, not an accident. The counter-probe: with `PULSE_SERVER`,
     * `PIPEWIRE_REMOTE` and `XDG_RUNTIME_DIR` pointed at paths that do not
     * exist, all seven cases stay green (measured 2026-08-28). A run that
     * opened a device would go out differently.
     */
    static void feedTone(AudioRecorder &recorder, int milliseconds);
    /** One buffer of `frames` samples of that tone, continuing at `phase`. */
    static QAudioBuffer tone(qint64 &phase, int frames);
    /** One buffer of `frames` samples of nothing at all — the counter-run. */
    static QAudioBuffer silence(int frames);
    static QImage shot(QWidget &window);
    /**
     * The window's own timer, the one that writes the running time.
     *
     * Direct children only: findChild() descends by default, and the recorder
     * below the window parents timers of its own to itself — the wakeup that
     * ends a recording at the bound is one of them.
     */
    static const QTimer *theClock(const RecordingWindow &window);
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

void CaptureTest::ranksTheTwoWritingsOnThePoorerGround()
{
    // Nobody checks a colour formula by looking. Which of the two writings the
    // window picks depends on the desktop theme and the colour scheme together,
    // and a user runs one of those combinations — a wrong pick under the others
    // would never be seen (issue #97).
    //
    // The anchors come from outside the formula: WCAG 2.1 fixes black against
    // white at 21 : 1 and any colour against itself at 1 : 1. Comparing the
    // formula only against itself would move both sides of every comparison
    // together.
    QCOMPARE(qRound(capture::contrastRatio(Qt::black, Qt::white)), 21);
    QCOMPARE(qRound(capture::contrastRatio(Qt::white, Qt::white)), 1);

    // The measured case of the issue, `cachyos-emerald-color` under a dark
    // scheme (24.08.2026): the note's green stands well on the dark ground and
    // almost vanishes on the light one, while the placeholder's grey is
    // middling on both. Judged on the poorer ground the note is the quieter of
    // the two, and the window has to say so.
    const QColor note(QStringLiteral("#00c790"));
    const QColor placeholder(QStringLiteral("#666a73"));
    const QColor lightGround(QStringLiteral("#efeff0"));
    const QColor darkGround(QStringLiteral("#080808"));
    QVERIFY(capture::contrastRatio(note, darkGround) > capture::contrastRatio(placeholder, darkGround));
    QVERIFY(capture::noteIsTheQuieterWriting({.note = note,
                                              .placeholder = placeholder,
                                              .groundOverWhite = lightGround,
                                              .groundOverBlack = darkGround}));

    // And an opaque field, where both grounds are the same one: `breeze-dark`
    // measured on the same day. Nothing to lift there, and a rule that lifted
    // anyway would break the ordinary case to heal the exception.
    const QColor brightNote(QStringLiteral("#fcfcfc"));
    const QColor dimmed(QStringLiteral("#a1a9b1"));
    const QColor opaqueGround(QStringLiteral("#141618"));
    QVERIFY(!capture::noteIsTheQuieterWriting({.note = brightNote,
                                               .placeholder = dimmed,
                                               .groundOverWhite = opaqueGround,
                                               .groundOverBlack = opaqueGround}));
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

QAudioBuffer CaptureTest::tone(qint64 &phase, int frames)
{
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    format.setSampleFormat(QAudioFormat::Int16);

    QByteArray samples(static_cast<qsizetype>(frames) * 2, Qt::Uninitialized);
    auto *value = reinterpret_cast<qint16 *>(samples.data());
    for (int i = 0; i < frames; ++i, ++phase) {
        const double t = static_cast<double>(phase) / 48000.0;
        value[i] = static_cast<qint16>(12000.0 * std::sin(2.0 * std::numbers::pi * 440.0 * t));
    }
    return {samples, format};
}

const QTimer *CaptureTest::theClock(const RecordingWindow &window)
{
    const QList<QTimer *> clocks =
        window.findChildren<QTimer *>(QString(), Qt::FindDirectChildrenOnly);
    return clocks.size() == 1 ? clocks.constFirst() : nullptr;
}

QAudioBuffer CaptureTest::silence(int frames)
{
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    format.setSampleFormat(QAudioFormat::Int16);

    return {QByteArray(static_cast<qsizetype>(frames) * 2, '\0'), format};
}

void CaptureTest::feedTone(AudioRecorder &recorder, int milliseconds)
{
    qint64 phase = 0;
    for (int buffers = milliseconds / 100; buffers > 0 && recorder.isRecording(); --buffers) {
        recorder.encode(tone(phase, 4800)); // 100 ms at 48 kHz
        // The encoder takes one buffer at a time. Without the pause between two
        // of them it refuses the second, and the recording would be short of it
        // — which is what the duration comparisons in the cases below measure.
        QTest::qWait(10);
    }
}

void CaptureTest::writesOpusIntoOgg()
{
    const QString directory = m_dir->filePath(QStringLiteral("audio"));
    AudioRecorder recorder(directory);
    const QDateTime createdAt =
        QDateTime::fromString(QStringLiteral("2026-08-28T21:07:03.250"), Qt::ISODateWithMs);
    QSignalSpy finished(&recorder, &AudioRecorder::finished);

    QVERIFY2(recorder.startEncoder(createdAt), qPrintable(recorder.lastError()));
    feedTone(recorder, 1000);
    QCOMPARE(recorder.duration(), 1000);
    recorder.stop();
    QTRY_COMPARE(finished.count(), 1);

    // The name is the timestamp of the note in the form the store writes into
    // `created_at`, with the colons of the hour replaced (SPEC 4 and 5.1
    // together). A second rendering of the same moment would look right and
    // point somewhere else; a colon would fail on the FAT stick the full
    // export of SPEC 8.3 writes to.
    QCOMPARE(finished.first().at(0).toString(), QStringLiteral("2026-08-28T21-07-03.250.ogg"));
    QCOMPARE(finished.first().at(1).toInt(), 1);

    QFile file(directory + QStringLiteral("/2026-08-28T21-07-03.250.ogg"));
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.fileName()));
    const QByteArray head = file.read(64);
    // What the file is, read out of the file. "The recorder wrote something"
    // is the same output for a WAV, for a Vorbis stream and for the 127-byte
    // header an encoder leaves behind that never took a sample.
    QVERIFY(head.startsWith(QByteArrayLiteral("OggS")));
    QVERIFY(head.contains(QByteArrayLiteral("OpusHead")));
    QVERIFY(file.size() > 1000);
}

void CaptureTest::stopsAtTheLimit()
{
    AudioRecorder recorder(m_dir->filePath(QStringLiteral("audio")));
    // The bound of SPEC 4 stands as the default; the run reaches it in half a
    // second, because a check that sat out the real quarter of an hour would
    // not be run.
    QCOMPARE(recorder.maximumDuration(), 15 * 60 * 1000);
    recorder.setMaximumDuration(500);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&recorder, &AudioRecorder::finished);

    QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));
    feedTone(recorder, 2000);

    QCOMPARE(recorder.isRecording(), false);
    QCOMPARE(recorder.duration(), 500);
    QTRY_COMPARE(finished.count(), 1);
}

void CaptureTest::cancellingDeletesTheFile()
{
    const QString directory = m_dir->filePath(QStringLiteral("audio"));
    AudioRecorder recorder(directory);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy cancelled(&recorder, &AudioRecorder::cancelled);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&recorder, &AudioRecorder::finished);

    QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));
    feedTone(recorder, 300);
    const QString file = directory + QLatin1Char('/') + recorder.fileName();
    QVERIFY(QFile::exists(file));

    recorder.cancel();
    QTRY_COMPARE(cancelled.count(), 1);
    QCOMPARE(finished.count(), 0);
    QVERIFY(!QFile::exists(file));
}

void CaptureTest::stopsAtTheLimitOnTheClockAlone()
{
    AudioRecorder recorder(m_dir->filePath(QStringLiteral("audio")));
    recorder.setMaximumDuration(300);
    QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));

    // Buffers of 20 ms handed over every 60 ms: the clock passes the bound
    // long before that much audio has been recorded. That is the forgotten
    // recording of SPEC 4 in miniature, and the case a bound on the frame
    // count alone would never reach — an encoder that takes nothing lets it
    // run for ever.
    qint64 phase = 0;
    for (int buffers = 12; buffers > 0 && recorder.isRecording(); --buffers) {
        recorder.encode(tone(phase, 960));
        QTest::qWait(60);
    }

    QCOMPARE(recorder.isRecording(), false);
    QVERIFY2(recorder.duration() < 300, qPrintable(QString::number(recorder.duration())));
}

void CaptureTest::keepsTheTailOfTheRecording()
{
    AudioRecorder recorder(m_dir->filePath(QStringLiteral("audio")));
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&recorder, &AudioRecorder::finished);
    QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));

    // Five buffers with no pause at all: the encoder takes one at a time, so
    // four of them are still queued when the stop comes. They are the end of
    // the recording, not surplus — the last word spoken lies in there.
    qint64 phase = 0;
    for (int buffers = 5; buffers > 0; --buffers) {
        recorder.encode(tone(phase, 4800));
    }
    recorder.stop();

    QTRY_COMPARE(finished.count(), 1);
    QCOMPARE(recorder.duration(), 500);
    QCOMPARE(finished.first().at(1).toInt(), 0);
}

void CaptureTest::dropsWhatItCannotSaveOnTheWayOut()
{
    const QString directory = m_dir->filePath(QStringLiteral("audio"));
    QString file;
    {
        AudioRecorder recorder(directory);
        QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));
        feedTone(recorder, 300);
        file = directory + QLatin1Char('/') + recorder.fileName();
        QVERIFY(QFile::exists(file));
    }
    // Esc closes the window, and the window takes the recorder with it in the
    // same turn of the event loop — the deletion that waits for the muxer
    // would then never run. And a recording still going when the window goes
    // is one nobody will ever make a note for either.
    QVERIFY(!QFile::exists(file));
}

void CaptureTest::dropsARecordingStoppedButNotYetClosed()
{
    const QString directory = m_dir->filePath(QStringLiteral("audio"));
    QString file;
    {
        AudioRecorder recorder(directory);
        QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));
        // Five buffers with no pause: four of them are still queued, so stop()
        // returns with the muxer still open. Ctrl+Enter and a window that
        // closes in the same turn is exactly this.
        qint64 phase = 0;
        for (int buffers = 5; buffers > 0; --buffers) {
            recorder.encode(tone(phase, 4800));
        }
        file = directory + QLatin1Char('/') + recorder.fileName();
        recorder.stop();
        QVERIFY(QFile::exists(file));
    }
    // Nobody is left to hear finished(), so nobody will make a note out of
    // that file either — and a file no note points at is an orphan.
    QVERIFY(!QFile::exists(file));
}

void CaptureTest::cancellingOvertakesAStopInFlight()
{
    const QString directory = m_dir->filePath(QStringLiteral("audio"));
    AudioRecorder recorder(directory);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy cancelled(&recorder, &AudioRecorder::cancelled);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&recorder, &AudioRecorder::finished);
    QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));

    qint64 phase = 0;
    for (int buffers = 5; buffers > 0; --buffers) {
        recorder.encode(tone(phase, 4800));
    }
    const QString file = directory + QLatin1Char('/') + recorder.fileName();

    // Esc right after Ctrl+Enter. stop() returns while the muxer is still
    // closing, so the discard lands in a state the recorder used to ignore —
    // and the recording the user threw away was saved instead.
    recorder.stop();
    recorder.cancel();

    QTRY_COMPARE(cancelled.count(), 1);
    QCOMPARE(finished.count(), 0);
    QVERIFY(!QFile::exists(file));
}

void CaptureTest::stopsAtTheLimitWithNoBufferAtAll()
{
    AudioRecorder recorder(m_dir->filePath(QStringLiteral("audio")));
    recorder.setMaximumDuration(200);
    QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));

    // Not one buffer, the whole way. A device that goes silent without an
    // error delivers none — and the bound that is only read when a buffer
    // arrives would never be read again. The forgotten recording of SPEC 4 is
    // that case and no other.
    QTest::qWait(600);
    QCOMPARE(recorder.isRecording(), false);
}

void CaptureTest::announcesAFailedRecording()
{
    const QDateTime createdAt = QDateTime::currentDateTime();

    // `/proc` exists, so the recorder gets past creating its directory, and
    // nothing can be written into it — not even by root, which is how the CI
    // runs. A write-protected directory of our own would let root straight
    // through, and a directory in place of the file name does not stop the
    // muxer either: it makes up a name inside it (measured 2026-08-28).
    AudioRecorder recorder(QStringLiteral("/proc"));
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(&recorder, &AudioRecorder::failed);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(&recorder, &AudioRecorder::finished);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy cancelled(&recorder, &AudioRecorder::cancelled);

    QVERIFY2(recorder.startEncoder(createdAt), qPrintable(recorder.lastError()));
    feedTone(recorder, 300);

    // Exactly one of the three signals follows a started recording. A recorder
    // that only wrote the reason into lastError() would leave the window of
    // S13b waiting on Ctrl+Enter and on Esc alike, with nothing on either.
    QTRY_COMPARE(failed.count(), 1);
    QVERIFY(!failed.first().at(0).toString().isEmpty());
    QCOMPARE(recorder.isRecording(), false);

    recorder.stop();
    recorder.cancel();
    QTest::qWait(200);
    QCOMPARE(failed.count(), 1);
    QCOMPARE(finished.count(), 0);
    QCOMPARE(cancelled.count(), 0);
}

void CaptureTest::theLevelFollowsWhatTheSignalDoes()
{
    // Acceptance criterion 2 of issue #21, measured where SPEC 4 can be
    // measured: on the buffers that are on their way into the file, never on
    // the user's room. A meter that always read zero would look exactly like a
    // quiet room, and a meter that reported the buffer's existence rather than
    // its level would look exactly like a working one — so both ends are
    // asserted, and they have to come out different.
    AudioRecorder recorder(m_dir->filePath(QStringLiteral("audio")));
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy levels(&recorder, &AudioRecorder::levelChanged);

    QVERIFY2(recorder.startEncoder(QDateTime::currentDateTime()), qPrintable(recorder.lastError()));

    feedTone(recorder, 300);
    QVERIFY(!levels.isEmpty());
    // The tone of feedTone() swings to 12000 of a full scale of 32768, so its
    // peak is 0.366. The bounds are tight enough that a reading off by a
    // factor — a value taken per frame instead of per sample, a scale of
    // 32767 read as 255 — falls outside them.
    const qreal loud = levels.constLast().constFirst().toReal();
    QVERIFY2(loud > 0.35 && loud < 0.38, qPrintable(QString::number(loud)));

    levels.clear();
    recorder.encode(silence(4800));
    QCOMPARE(levels.count(), 1);
    QCOMPARE(levels.constFirst().constFirst().toReal(), 0.0);

    recorder.cancel();
}

void CaptureTest::makesTheNoteWhenTheRecordingIsFinished()
{
    // No device anywhere in this case, and the window's own road says so:
    // startWithoutADevice() is the half of SPEC 4 a check can walk.
    RecordingWindow window(m_store.get());
    QVERIFY2(window.startWithoutADevice(), qPrintable(window.recorder()->lastError()));
    // The running time is written by a timer, and the assertion that it stops
    // is worth nothing without the one that it ran (CLAUDE.md, finding 27):
    // a window that never started anything looks exactly like one that stopped.
    const QTimer *clock = theClock(window);
    QVERIFY(clock->isActive());

    feedTone(*window.recorder(), 1000);

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(window.recorder(), &AudioRecorder::finished);
    QCOMPARE(m_store->notes().size(), 0);

    QTest::keyClick(&window, Qt::Key_Return, Qt::ControlModifier);
    // Still nothing, and this is the assertion the case exists for (issue #22):
    // stop() returns while the muxer is still writing, and a note made here
    // would be in the transcription queue ahead of its own file — both attempts
    // of SPEC 12 spent in no time at all.
    QCOMPARE(m_store->notes().size(), 0);
    QVERIFY(!window.isVisible());

    QTRY_COMPARE(finished.count(), 1);
    const QList<Note> notes = m_store->notes();
    QCOMPARE(notes.size(), 1);

    const Note &note = notes.constFirst();
    QCOMPARE(note.type, Note::Type::Audio);
    // Empty on purpose: that is what the transcription queue takes an audio
    // note by (SPEC 12, transcriber.cpp:101).
    QVERIFY(note.content.isEmpty());
    QCOMPARE(note.audioDurationS, std::optional<int>(1));
    // The row and the file have to name the same moment, and this holds the
    // one against the other rather than both against a value of the test's:
    // the name is built here out of the timestamp the **store** handed back.
    QCOMPARE(note.audioPath, AudioRecorder::fileNameFor(note.createdAt));
    QVERIFY(QFile::exists(m_store->audioDirectory() + QLatin1Char('/') + note.audioPath));

    // And the display is switched off. Left running it would tick four times a
    // second on a hidden window for the rest of the service's life, rebuilding
    // a KColorScheme each time — nothing about that is visible.
    QVERIFY(!clock->isActive());
}

void CaptureTest::discardsTheRecordingOnEscape()
{
    RecordingWindow window(m_store.get());
    QVERIFY2(window.startWithoutADevice(), qPrintable(window.recorder()->lastError()));
    feedTone(*window.recorder(), 300);

    const QString file =
        m_store->audioDirectory() + QLatin1Char('/') + window.recorder()->fileName();
    QVERIFY(QFile::exists(file));

    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy cancelled(window.recorder(), &AudioRecorder::cancelled);
    QTest::keyClick(&window, Qt::Key_Escape);
    QTRY_COMPARE(cancelled.count(), 1);

    // Esc discards the recording **together with its file** (SPEC 4). What
    // stayed would be an orphan, and the sweep of SPEC 2.5 is what it is for.
    QVERIFY2(!QFile::exists(file), qPrintable(file));
    QCOMPARE(m_store->notes().size(), 0);
    QVERIFY(!window.isVisible());
}

void CaptureTest::keepsTheRecordingWhenTheNoteCannotBeStored()
{
    // The one case the cleanup check of SPEC 2.5 must not be left to decide
    // (addition of 29.08.2026): the recording is finished, addNote() fails, and
    // what lies on the disk is a file no row points at — which the sweep at the
    // next service start cannot tell from a harmless orphan.
    //
    // A store that was never opened is how the failure is produced here. Its
    // audio directory is the same one the case above records into, so the two
    // differ in exactly one thing: whether the database takes the note.
    Store closed(m_dir->filePath(QStringLiteral("never-opened.db")));

    RecordingWindow window(&closed);
    QVERIFY2(window.startWithoutADevice(), qPrintable(window.recorder()->lastError()));
    feedTone(*window.recorder(), 300);

    const QString name = window.recorder()->fileName();
    const QString inAudio = closed.audioDirectory() + QLatin1Char('/') + name;
    const QString rescued = closed.rescuedDirectory() + QLatin1Char('/') + name;
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy finished(window.recorder(), &AudioRecorder::finished);

    QTest::keyClick(&window, Qt::Key_Return, Qt::ControlModifier);
    QTRY_COMPARE(finished.count(), 1);

    // The recording is not where the sweep reads any more. Naming it and
    // leaving it lying was the first answer, and it was none — the sentence
    // promised it would not be deleted while the next start deleted it.
    QVERIFY2(!QFile::exists(inAudio), qPrintable(inAudio));
    QVERIFY2(QFile::exists(rescued), qPrintable(rescued));

    // And the sweep really runs, on a store that is open over the same data
    // directory and holds no note at all: every file under `audio/` is an
    // orphan to it. An assertion that only said "the file is somewhere else"
    // would be the same output for a directory nothing ever looks in.
    m_store->sweepOrphanedAudio();
    QVERIFY2(QFile::exists(rescued), qPrintable(rescued));

    // The counter-run, and it has to come out different: the same file put
    // back where it stood is gone after the same sweep. Without it the check
    // above would stand green over a sweep that deletes nothing at all.
    QVERIFY(QFile::copy(rescued, inAudio));
    m_store->sweepOrphanedAudio();
    QVERIFY2(!QFile::exists(inAudio), qPrintable(inAudio));
    QVERIFY2(QFile::exists(rescued), qPrintable(rescued));

    // What the user is told names the place the recording now lies. Read off
    // the function rather than off the notification: a KNotification on a bus
    // with no notification server sends nothing at all and says so nowhere
    // (CLAUDE.md, finding 37).
    const QString message = capture::recordingNotSavedMessage(rescued, true);
    QVERIFY2(message.contains(rescued), qPrintable(message));

    // And where the move itself failed there is no promise to make. The two
    // sentences have to be two: one text for both outcomes would be wrong in
    // one of them, and it was.
    const QString unmoved = capture::recordingNotSavedMessage(inAudio, false);
    QVERIFY2(unmoved.contains(inAudio), qPrintable(unmoved));
    QVERIFY(unmoved != message);
}

void CaptureTest::armsNothingWhenTheRecordingBreaksOffAtTheStart()
{
    // `audio/` as a symlink to `/proc`: the directory exists, so the recorder
    // gets past creating it, and nothing can be written into it — **not even
    // by root**, which is how the CI runs (CLAUDE.md, finding 46). A directory
    // of our own with the write bit taken away would let root straight through
    // and this case would stand green over a window that arms everything.
    QVERIFY(QFile::link(QStringLiteral("/proc"), m_dir->filePath(QStringLiteral("audio"))));

    RecordingWindow window(m_store.get());
    const QTimer *clock = theClock(window);
    // NOLINTNEXTLINE(misc-const-correctness) - changed through a Qt connection, see rule 2 in .clang-tidy
    QSignalSpy failed(window.recorder(), &AudioRecorder::failed);

    window.startWithoutADevice();

    // **The answer arrives inside the start.** Measured 29.08.2026: the muxer
    // emits its error synchronously out of record(), so the recording is over
    // before the call that began it has returned. The review of #21 read the
    // symptom off QTimer::isActive() and called for a stop in the failed
    // handler; the stop was there and ran — on a timer that had not been
    // started yet, because the window armed the display afterwards.
    QCOMPARE(failed.count(), 1);
    QVERIFY(!clock->isActive());
    QVERIFY(!window.isVisible());
    QCOMPARE(m_store->notes().size(), 0);

    // And nothing is left believing a recording is in flight. That is the
    // second half of the same fault and the worse one: a window that still
    // holds an answer outstanding shows itself on the next Meta+Shift+N and
    // records nothing at all, for the rest of the service's life. The readback
    // is a second attempt that has to reach the recorder — it fails the same
    // way, and that second failure is what says the first one was cleared.
    window.startWithoutADevice();
    QCOMPARE(failed.count(), 2);
    QVERIFY(!clock->isActive());
}

QTEST_MAIN(CaptureTest)

#include "capturetest.moc"
