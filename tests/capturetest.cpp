#include "capture/capturewindow.h"
#include "capture/textareaheight.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QApplication>
#include <QFont>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>

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
    void heightFollowsAFontChange();

    void savesTextOnControlReturn();
    void keepsBlankTextOutOfTheStore();
    void discardsTextOnEscape();

    void textsFollowAColourSchemeChange();

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

void CaptureTest::heightFollowsAFontChange()
{
    QPlainTextEdit *text = textArea();
    QVERIFY(text);

    // The font is set on the widget itself, as issue #56 prescribes: Plasma
    // does not hand a font change to a standing Qt Widgets application at all
    // (B6 of the theme report), so waiting for that road would mean waiting
    // for a road that does not exist. The widget sees the change on every
    // road, this one included.
    //
    // Two clearly different sizes, and the assertion is relative — the height
    // is read against the line spacing of the font in force, not against a
    // pixel count.
    for (const int pointSize : {9, 24}) {
        QFont font = text->font();
        font.setPointSize(pointSize);
        text->setFont(font);

        const int lineSpacing = text->fontMetrics().lineSpacing();
        // What the widget needs beyond the text itself; the same two sources
        // adjustHeight() reads.
        const int chrome = 2 * qRound(text->document()->documentMargin()) + 2 * text->frameWidth();

        QCOMPARE(text->height() - chrome, capture::MinTextLines * lineSpacing);

        // And it still grows with the text under the new font, up to eight.
        text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
        QCOMPARE(text->height() - chrome, capture::MaxTextLines * lineSpacing);

        text->clear();
    }
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

void CaptureTest::textsFollowAColourSchemeChange()
{
    // The daemon builds the window once and keeps it (SPEC 2.1), so a colour
    // scheme change reaches a window that is already standing. Every text has
    // to follow it — the application name and the key hint included (issue #54).
    const QPalette startPalette = qApp->palette();

    QPalette switched = startPalette;
    switched.setColor(QPalette::WindowText, QColor(0x23, 0x26, 0x29));
    switched.setColor(QPalette::PlaceholderText, QColor(0x70, 0x7d, 0x8a));
    qApp->setPalette(switched);

    // Qt hands the new palette to the widgets through a posted event; without a
    // running event loop the test has to let it through itself.
    QCoreApplication::processEvents();

    const QList<QLabel *> labels = m_window->findChildren<QLabel *>();
    QCOMPARE(labels.size(), 2);

    for (QLabel *label : labels) {
        // What the label paints with: its own palette, read through its role.
        QCOMPARE(label->palette().color(label->foregroundRole()), QColor(0x70, 0x7d, 0x8a));
    }

    qApp->setPalette(startPalette);
}

QTEST_MAIN(CaptureTest)

#include "capturetest.moc"
