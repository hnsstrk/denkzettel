#pragma once

#include <QObject>

class KStatusNotifierItem;
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
};
