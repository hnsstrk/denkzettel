#pragma once

#include <QObject>
#include <QPointer>

class KStatusNotifierItem;
class QDialog;
class QMenu;

/**
 * Permanent tray presence of the daemon (SPEC 10).
 */
class TrayIcon : public QObject
{
    Q_OBJECT

public:
    explicit TrayIcon(QObject *parent = nullptr);

    /**
     * The tray item, for reading only: what the item announces to the host is
     * only knowable by asking the item itself (issue #44). Const because
     * everything the item is told is decided in the constructor.
     */
    const KStatusNotifierItem *item() const;

Q_SIGNALS:
    void captureRequested();
    void libraryRequested();

private:
    QMenu *buildMenu();

    KStatusNotifierItem *m_item;
    /**
     * The open about dialog, or nothing.
     *
     * It deletes itself on close, so the pointer has to notice that by itself;
     * a raw one would be dangling the second time the entry is used.
     */
    QPointer<QDialog> m_about;
};
