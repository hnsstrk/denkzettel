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

private:
    QMenu *buildMenu();

    KStatusNotifierItem *m_item;
};
