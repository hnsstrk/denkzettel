#include "shell/trayicon.h"

#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QAction>
#include <QApplication>
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
    m_item->setIconByName(QStringLiteral("knotes-symbolic"));
    m_item->setTitle(i18n("Denkzettel"));
    m_item->setToolTip(QStringLiteral("knotes-symbolic"), i18n("Denkzettel"), i18n("Gedanken schnell festhalten"));
    m_item->setStandardActionsEnabled(false);
    m_item->setContextMenu(buildMenu());
}

QMenu *TrayIcon::buildMenu()
{
    auto *menu = new QMenu();

    addStub(menu, i18n("Capture öffnen"));
    addStub(menu, i18n("Sprachnotiz aufnehmen"));
    addStub(menu, i18n("Bibliothek"));
    addStub(menu, i18n("Analyse jetzt"));
    addStub(menu, i18n("Vorschläge"));
    addStub(menu, i18n("Einstellungen"));

    menu->addSeparator();

    QAction *quitAction = menu->addAction(i18n("Beenden"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    return menu;
}
