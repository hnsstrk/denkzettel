#include "capture/capturewindow.h"

#include "capture/textareaheight.h"
#include "store/note.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QAbstractTextDocumentLayout>
#include <QDateTime>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>

namespace
{
constexpr int WindowWidth = 600;

/** Time the compositor gets to notice the surface going away, in ms. */
constexpr int RemapDelayMs = 50;

/** Small, dimmed label — used for the application name and the key hint. */
QLabel *subtleLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

    // The role, not the colour: the daemon keeps the window for its whole life
    // (SPEC 2.1), and a colour taken from the palette once would stay put when
    // the user changes the colour scheme (issue #54). A role is resolved anew
    // on every palette change.
    label->setForegroundRole(QPalette::PlaceholderText);

    return label;
}
}

CaptureWindow::CaptureWindow(Store *store, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , m_store(store)
    , m_text(new QPlainTextEdit(this))
{
    setWindowTitle(i18n("Denkzettel"));

    m_text->setFrameShape(QFrame::NoFrame);
    m_text->setPlaceholderText(i18n("Gedanke festhalten …"));
    m_text->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_text->installEventFilter(this);

    // Activating the window puts the keyboard focus straight into the text.
    setFocusProxy(m_text);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 8);
    layout->setSpacing(8);
    layout->addWidget(subtleLabel(i18n("Denkzettel"), this));
    layout->addWidget(m_text);

    QLabel *hint = subtleLabel(i18n("Esc verwirft · Strg+Enter speichert"), this);
    hint->setAlignment(Qt::AlignCenter);
    layout->addWidget(hint);

    connect(m_text->document()->documentLayout(),
            &QAbstractTextDocumentLayout::documentSizeChanged,
            this,
            &CaptureWindow::adjustHeight);

    adjustHeight();
    resize(WindowWidth, sizeHint().height());
}

void CaptureWindow::showCapture()
{
    // A mapped window cannot take the keyboard focus back on Wayland (T1,
    // issue #1): hide() destroys the surface, so the following show() is a
    // fresh mapping, and a fresh toplevel is focused by the compositor on its
    // own. The delay gives the compositor time to see the surface go away.
    if (isVisible()) {
        hide();
        QTimer::singleShot(RemapDelayMs, this, &CaptureWindow::present);
        return;
    }

    present();
}

bool CaptureWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_text && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        const bool isReturn = keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter;

        if (isReturn && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            save();
            return true;
        }

        if (keyEvent->key() == Qt::Key_Escape) {
            discard();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CaptureWindow::present()
{
    show();
    raise();
    activateWindow();
}

void CaptureWindow::save()
{
    const QString content = m_text->toPlainText().trimmed();
    if (content.isEmpty()) {
        discard();
        return;
    }

    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.type = Note::Type::Text;
    note.content = content;

    if (!m_store->addNote(note)) {
        // Keep window and text: a lost thought is worse than a window that
        // stays open.
        qWarning("Storing the note failed: %s", qPrintable(m_store->lastError()));
        return;
    }

    m_text->clear();
    hide();
}

void CaptureWindow::discard()
{
    m_text->clear();
    hide();
}

void CaptureWindow::adjustHeight()
{
    // QPlainTextDocumentLayout reports its height as a line count, not in
    // pixels — wrapped lines included.
    const int documentLines = qCeil(m_text->document()->size().height());
    const int chrome = 2 * qRound(m_text->document()->documentMargin()) + 2 * m_text->frameWidth();

    m_text->setFixedHeight(capture::textAreaHeight(documentLines, m_text->fontMetrics().lineSpacing(), chrome));
    resize(width(), sizeHint().height());
}
