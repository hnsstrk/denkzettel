#include "shell/trayicon.h"

#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>

namespace
{
/**
 * Menu entry for a feature that is not implemented yet.
 *
 * The icon comes from the theme like every other one: only a themed icon
 * carries a name, and only the name travels to Plasma over the tray protocol
 * (wireframe 5a).
 */
void addStub(QMenu *menu, const QString &text, const QString &iconName)
{
    QAction *action = menu->addAction(QIcon::fromTheme(iconName), text);
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
    //
    // It stays true although issue #60 asked for two menus. False would split
    // the clicks, and the menu of the left one would then be ours to draw — the
    // measurement of 02.08.2026 shows that it cannot be drawn where it belongs:
    // as a popup it closes two milliseconds after opening, as a window Wayland
    // discards the position and KWin puts it in the middle of the screen
    // (docs/scrum/reviews/sprint-04-s33-traymenues/messung.md).
    m_item->setIsMenu(true);
}

const KStatusNotifierItem *TrayIcon::item() const
{
    return m_item;
}

QMenu *TrayIcon::buildMenu()
{
    auto *menu = new QMenu();

    QAction *captureAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18n("Notiz erfassen"));
    // A hint, not a second binding: the sequence is drawn beside the entry, and
    // the context keeps it from ever answering. A menu shortcut reaches the
    // window of its menu, and this menu has none — plasmashell draws it. The
    // binding that works lives with kglobalacceld (SPEC 2.4).
    captureAction->setShortcut(QKeySequence(Qt::META | Qt::Key_N));
    captureAction->setShortcutContext(Qt::WidgetShortcut);
    connect(captureAction, &QAction::triggered, this, &TrayIcon::captureRequested);

    addStub(menu, i18n("Sprachnotiz aufnehmen"), QStringLiteral("audio-input-microphone"));

    // Separates capturing from looking at and working on — the only grouping
    // among the working paths (wireframe 5a).
    menu->addSeparator();

    const QAction *libraryAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("view-list-text")), i18n("Bibliothek öffnen"));
    connect(libraryAction, &QAction::triggered, this, &TrayIcon::libraryRequested);

    addStub(menu, i18n("Jetzt analysieren"), QStringLiteral("system-run"));
    addStub(menu, i18n("Vorschläge"), QStringLiteral("tools-wizard"));

    // "Denkzettel einrichten …" joins the group below with the settings dialog
    // (#16). Until then there is no entry for it: a permanently greyed one does
    // not tell the user why it is greyed (KDE HIG, wireframe 5a).
    menu->addSeparator();

    const QAction *quitAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), i18n("Beenden"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    return menu;
}
