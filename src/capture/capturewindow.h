#pragma once

#include <QWidget>

class Store;
class QPlainTextEdit;

/**
 * Frameless window for capturing a text note (SPEC 3).
 *
 * It lives hidden inside the daemon so that it can appear without a process
 * start. Placement is left to KWin (PO decision after the T1 spike); a Wayland
 * client cannot position itself anyway.
 */
class CaptureWindow : public QWidget
{
    Q_OBJECT

public:
    /** `store` outlives the window and is not owned by it. */
    explicit CaptureWindow(Store *store, QWidget *parent = nullptr);

public Q_SLOTS:
    /** Brings the window up with the keyboard focus, empty and ready. */
    void showCapture();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void present();
    void save();
    void discard();
    void adjustHeight();

    Store *m_store;
    QPlainTextEdit *m_text;
};
