#include "shell/trayicon.h"

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QAction>
#include <QApplication>
#include <QDialog>
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
    // Status and subtitle of the untroubled state, and they are set from the
    // one place that also takes them back: written out a second time here they
    // would be the copy that goes stale when the wording changes (issue #24).
    setTranscriptionError({});
    m_item->setStandardActionsEnabled(false);
    m_item->setContextMenu(buildMenu());
    // The left click is to open the same menu as the right one (issue #44,
    // user decision of 01.08.2026, deliberately unlike the KDE default).
    // ItemIsMenu has no change signal in the SNI protocol — the host reads it
    // when the item registers and never asks again, so it belongs here among
    // the other properties and not to some later moment.
    //
    // It stays true although issue #60 asked for two menus. False would split
    // the clicks, and the menu of the left one would then be ours to draw — the
    // measurement of 02.08.2026 shows that it cannot be drawn where it belongs:
    // as a popup it closes two milliseconds after opening, as a window Wayland
    // discards the position and KWin puts it in the middle of the screen
    // (Messung zu #33, Sprint 4).
    m_item->setIsMenu(true);
}

const KStatusNotifierItem *TrayIcon::item() const
{
    return m_item;
}

void TrayIcon::setTranscriptionError(const QString &reason)
{
    m_item->setStatus(reason.isEmpty() ? KStatusNotifierItem::Active
                                       : KStatusNotifierItem::NeedsAttention);
    // This is the quiet channel of SPEC 14 — tray state and tooltip — and it
    // is all this class serves. The loud one stands beside these two lines and
    // not in here: a transcription that has finally failed also sends one
    // KNotification, wired in main.cpp where the error path of the queue
    // reaches the user (SPEC 10, issue #115). The two carry different things.
    // The notification is for the moment it happens and goes out once; this
    // state stands as long as the queue holds a job that was given up on, a
    // restart included.
    m_item->setToolTipSubTitle(reason.isEmpty() ? i18n("Capture thoughts quickly")
                                                : i18n("Transcription failed: %1", reason));
}

QMenu *TrayIcon::buildMenu()
{
    auto *menu = new QMenu();

    QAction *captureAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18n("Capture note"));
    // A hint, not a second binding: the sequence is drawn beside the entry, and
    // the context keeps it from ever answering. A menu shortcut reaches the
    // window of its menu, and this menu has none — plasmashell draws it. The
    // binding that works lives with kglobalacceld (SPEC 2.4).
    captureAction->setShortcut(QKeySequence(Qt::META | Qt::Key_N));
    captureAction->setShortcutContext(Qt::WidgetShortcut);
    connect(captureAction, &QAction::triggered, this, &TrayIcon::captureRequested);

    addStub(menu, i18n("Record voice note"), QStringLiteral("audio-input-microphone"));

    // Separates capturing from looking at and working on — the only grouping
    // among the working paths (wireframe 5a).
    menu->addSeparator();

    const QAction *libraryAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("view-list-text")), i18n("Open library"));
    connect(libraryAction, &QAction::triggered, this, &TrayIcon::libraryRequested);

    addStub(menu, i18n("Analyze now"), QStringLiteral("system-run"));
    addStub(menu, i18n("Suggestions"), QStringLiteral("tools-wizard"));

    menu->addSeparator();

    // The wording names the application, unlike the window title of the dialog
    // it opens: this entry stands among entries of other programs, and the KDE
    // habit for it is "Configure %1…" (dolphin.mo: „%1 einrichten …"). The
    // window on the other side names no application, because the decoration
    // appends it (issue #16, UX decision of 29.08.2026).
    const QAction *configureAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure Denkzettel…"));
    connect(configureAction, &QAction::triggered, this, &TrayIcon::configureRequested);

    // Where the version becomes visible in the running application (issue #87,
    // SPEC 15.1). It stands in the last group, which is the administrative one,
    // and above "Quit", which stays the last entry — the same order a KDE
    // application menu has. The tray menu and not the library window: the
    // library is one route among several of equal rank (SPEC 2.1), and a
    // statement about the whole application must not hang off one of them.
    const QAction *aboutAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("help-about")), i18n("About Denkzettel"));
    connect(aboutAction, &QAction::triggered, this, [this] {
        if (!m_about) {
            // KAboutData::applicationData() is what registerApplicationIdentity()
            // filled — name, version, description and licence live there and in
            // no second place (SPEC 15.1).
            auto *dialog = new KAboutApplicationDialog(KAboutData::applicationData());
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            m_about = dialog;
        } else if (m_about->isVisible()) {
            // THE ONE LINE THAT BRINGS THE STANDING DIALOG BACK TO THE FRONT,
            // and it is a measurement, not a preference. Measured 28.08.2026 in
            // a nested kwin_wayland with WAYLAND_DEBUG=1: between two title
            // markers, show() on a visible widget together with raise() sent
            // NOT ONE request over the wire — show() returns at once on a
            // visible widget, and xdg-shell has no restacking request a client
            // could use, so raise() has nothing to send (activateWindow() is
            // the same dead end, CLAUDE.md "Runs that prove nothing", 3). The
            // hide() below is what makes the following show() build the surface
            // anew, and only that carries an xdg-activation token to the
            // compositor.
            //
            // The price is visible and the user decided to pay it (28.08.2026):
            // surface and decoration are destroyed and built again, so the
            // dialog blinks. Only when it really stands — on the first click
            // there is nothing to hide.
            m_about->hide();
        }
        m_about->show();
    });

    const QAction *quitAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), i18n("Quit"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    return menu;
}
