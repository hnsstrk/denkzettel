#include "capture/capturewindow.h"
#include "capture/textareaheight.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QPlainTextEdit>
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

    void restsAtFiveLines();
    void growsWithTheText();
    void stopsAtEightLines();
    void windowFollowsTheTextHeight();

    void savesTextOnControlReturn();
    void keepsBlankTextOutOfTheStore();
    void discardsTextOnEscape();

private:
    QPlainTextEdit *textArea() const;

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
    std::unique_ptr<CaptureWindow> m_window;
};

void CaptureTest::initTestCase()
{
    // Without the domain every i18n() call in the window warns.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));
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

void CaptureTest::restsAtFiveLines()
{
    // An empty document still reports one line, so the resting height has to
    // come from the lower bound.
    QCOMPARE(capture::textAreaHeight(0, 20, 4), 104);
    QCOMPARE(capture::textAreaHeight(1, 20, 4), 104);
    QCOMPARE(capture::textAreaHeight(5, 20, 4), 104);
}

void CaptureTest::growsWithTheText()
{
    QCOMPARE(capture::textAreaHeight(6, 20, 4), 124);
    QCOMPARE(capture::textAreaHeight(7, 20, 4), 144);
    QCOMPARE(capture::textAreaHeight(8, 20, 4), 164);

    // The chrome is added on top of the text, whatever the line count.
    QCOMPARE(capture::textAreaHeight(6, 20, 0), 120);
}

void CaptureTest::stopsAtEightLines()
{
    const int eightLines = capture::textAreaHeight(8, 20, 4);
    QCOMPARE(capture::textAreaHeight(9, 20, 4), eightLines);
    QCOMPARE(capture::textAreaHeight(400, 20, 4), eightLines);
}

void CaptureTest::windowFollowsTheTextHeight()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    const int resting = m_window->height();

    text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs"));
    const int sixLines = m_window->height();
    QVERIFY2(sixLines > resting, qPrintable(QStringLiteral("%1 <= %2").arg(sixLines).arg(resting)));

    text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
    const int eightLines = m_window->height();
    QVERIFY(eightLines > sixLines);

    // Beyond the maximum the scrollbar takes over and the window stands still.
    text->setPlainText(text->toPlainText() + QStringLiteral("\nneun\nzehn\nelf\nzwölf"));
    QCOMPARE(m_window->height(), eightLines);

    text->clear();
    QCOMPARE(m_window->height(), resting);
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

QTEST_MAIN(CaptureTest)

#include "capturetest.moc"
