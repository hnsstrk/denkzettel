#include "shell/trayicon.h"

#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>

namespace
{
/** Menu entry for a feature that is not implemented yet. */
void addStub(QMenu *menu, const QString &text)
{
    QAction *action = menu->addAction(text);
    action->setEnabled(false);
}
}

TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
    , m_item(new KStatusNotifierItem(QStringLiteral("denkzettel"), this))
{
    m_item->setCategory(KStatusNotifierItem::ApplicationStatus);
    m_item->setStatus(KStatusNotifierItem::Active);
    // The theme name lets Plasma recolor the monochrome icon to the panel
    // (issue #43). A name would travel over D-Bus and fail silently when the
    // icon is not installed, so fall back to sending pixmaps in that case.
    const QString trayIconName = QStringLiteral("denkzettel-tray");
    if (QIcon::hasThemeIcon(trayIconName)) {
        m_item->setIconByName(trayIconName);
        m_item->setToolTipIconByName(trayIconName);
    } else {
        const QIcon bundled(QStringLiteral(":/icons/denkzettel-tray.svg"));
        m_item->setIconByPixmap(bundled);
        m_item->setToolTipIconByPixmap(bundled);
    }
    m_item->setTitle(i18n("Denkzettel"));
    m_item->setToolTipTitle(i18n("Denkzettel"));
    m_item->setToolTipSubTitle(i18n("Gedanken schnell festhalten"));
    m_item->setStandardActionsEnabled(false);
    m_item->setContextMenu(buildMenu());
    // The left click is to open the same menu as the right one (issue #44,
    // customer decision of 01.08.2026, deliberately unlike the KDE default).
    // ItemIsMenu has no change signal in the SNI protocol — the host reads it
    // when the item registers and never asks again, so it belongs here among
    // the other properties and not to some later moment.
    m_item->setIsMenu(true);
}

const KStatusNotifierItem *TrayIcon::item() const
{
    return m_item;
}

QMenu *TrayIcon::buildMenu()
{
    auto *menu = new QMenu();

    QAction *captureAction = menu->addAction(i18n("Capture öffnen"));
    connect(captureAction, &QAction::triggered, this, &TrayIcon::captureRequested);

    addStub(menu, i18n("Sprachnotiz aufnehmen"));

    QAction *libraryAction = menu->addAction(i18n("Bibliothek"));
    connect(libraryAction, &QAction::triggered, this, &TrayIcon::libraryRequested);

    addStub(menu, i18n("Analyse jetzt"));
    addStub(menu, i18n("Vorschläge"));
    addStub(menu, i18n("Einstellungen"));

    menu->addSeparator();

    QAction *quitAction = menu->addAction(i18n("Beenden"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    return menu;
}
