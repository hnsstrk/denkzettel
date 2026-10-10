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

    // **Plasma sets a NeedsAttention item apart by itself, and that is not
    // enough** (CLAUDE.md, finding 95). Its system tray runs a
    // `PulseAnimation` over every item in that status — scale 1 → 1.2 → 1
    // over two times 15 % of a period of about two seconds, then 70 % at rest,
    // looping for ever. **Read** in `AbstractItem.qml` and
    // `PulseAnimation.qml` of the systemtray applet of plasma-workspace 6.7.5,
    // not run (finding 50). Switching animations off does not stop it: with
    // `AnimationDurationFactor=0`, `Kirigami.Units.longDuration` is **1** and
    // not 0, so the pulse collapses into a flicker rather than going away —
    // and that half was measured.
    //
    // An earlier wording here had the state invisible without this line and
    // offered a channel difference of 1005 over the icon as the proof. That is
    // one frame of an animated state and says nothing in either direction,
    // and neither does the 104253 read beside it. The sentence that replaced
    // it — "the state was never invisible" — went just as far past the
    // evidence in the other direction.
    //
    // **What carries is the weaker sentence, and it is the one the user
    // feels: without an attention icon of our own there is no difference that
    // holds independently of the moment.** Measured in a nested
    // `kwin_wayland` with a real panel, the same binary once with this
    // statement and once without it: still frames of the fault state come out
    // bit-identical with the normal state in 3 of 8 grabs with animations off
    // and 3 of 6 at the default, against an `Active` reference that is itself
    // perfectly still — the remaining frames differ by 152,078 summed channel
    // difference, which is what says the identity belongs to the product and
    // not to the measurement. Those counts are **not** a frequency
    // (finding 76); what they say is that such a frame exists. Without the
    // icon the only mark left is the size, 23×24 against 27×28 device pixels.
    //
    // `docs/images/reviews/40-ux-ohne-aufmerksamkeitssymbol.png` is that run,
    // four tiles of 90×50 device pixels out of the panel at three times the
    // size: without `AttentionIconName` set, `Active` and `NeedsAttention` at
    // rest are bit-identical (tiles 1 and 2) and only the pulse phase differs
    // (3); tile 4 is the built state with `dialog-warning`.
    //
    // A state indicator whose only mark can be absent at the moment you look
    // is not one. So this line is an accessibility matter and not a design
    // preference — the KDE HIG asks that an animated element either
    // transition instantly or show a static image once animations are off, and
    // Plasma's attention hint does neither.
    //
    // Out of the Breeze stock and not drawn here (SPEC 10: "no graphics work of
    // our own"), and neither a badge nor an overlay — the two the customer
    // refused on 29.08.2026: this is the icon the tray protocol has for exactly
    // this state, and it stands in for the normal one for as long as the state
    // does.
    //
    // Outside the branch above, unlike the two icons in it: a name is what the
    // protocol carries either way, and set inside the branch this would be
    // missing on precisely the machines that have no Denkzettel icon installed
    // — a check for it would then be green here and red in the automated run.
    m_item->setAttentionIconByName(QStringLiteral("dialog-warning"));
    m_item->setTitle(i18n("Denkzettel"));
    m_item->setToolTipTitle(i18n("Denkzettel"));
    // Status and subtitle of the untroubled state, and they are set from the
    // one place that also takes them back: written out a second time here they
    // would be the copy that goes stale when the wording changes (issue #24).
    showToolTip();
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

void TrayIcon::setNotesWithoutTranscript(int count)
{
    // This is the quiet channel of SPEC 14 — tray state and tooltip — and it
    // is all this class serves. The loud one stands beside this line and not
    // in here: a transcription that has finally failed also sends one
    // KNotification, wired in main.cpp where the error path of the queue
    // reaches the user (SPEC 10, issue #115). The two carry different things.
    // The notification is for the moment it happens and goes out once; this
    // state stands as long as the queue holds a job that was given up on, a
    // restart included.
    //
    // The state itself is raised in showToolTip() and not here, where it used
    // to be: with a second count beside this one, whichever setter ran last
    // would take the other one's state back (issue #118).
    m_notesWithoutTranscript = count;
    showToolTip();
}

void TrayIcon::setNotesWithoutCategory(int count)
{
    m_notesWithoutCategory = count;
    showToolTip();
}

void TrayIcon::setUnavailableTools(const QStringList &names)
{
    m_unavailableTools = names;
    showToolTip();
}

void TrayIcon::setMissingModel(const QString &report)
{
    m_missingModel = report;
    showToolTip();
}

void TrayIcon::setOverflow(const QString &report)
{
    m_overflow = report;
    showToolTip();
}

void TrayIcon::setBundlesDropped(const QString &report)
{
    m_bundlesDropped = report;
    showToolTip();
}

void TrayIcon::setWaitingSuggestions(int count)
{
    m_waitingSuggestions = count;
    showWaitingSuggestions();
}

void TrayIcon::showWaitingSuggestions()
{
    // The label carries the number and the tooltip does not: the tooltip is the
    // channel for what has gone wrong (SPEC 14), and a waiting suggestion is
    // the program doing its work. Falling back to the bare wording at 0 is what
    // the wireframe asks for — the counter "tritt an die Beschriftung, sobald
    // es Vorschläge gibt" (5a) — and it is also what keeps an entry that says
    // "(0)" from looking like an offer with nothing behind it.
    m_proposalsAction->setText(
        m_waitingSuggestions > 0
            ? i18nc("@action:inmenu tray entry with the number of waiting suggestions",
                    "Suggestions (%1)", m_waitingSuggestions)
            : i18n("Suggestions"));
}

void TrayIcon::showToolTip()
{
    // `setToolTipSubTitle()` takes ONE string, so every source that has
    // something to say writes a **part** of it and none of them overwrites
    // another (issue #118). No ranking either: a ranking would keep the user
    // from ever learning of the second trouble while the first stands, and
    // both of these stand for as long as their cause does. The order below is
    // fixed only so that the line does not reshuffle itself under the pointer.
    //
    // The error state goes with the line and out of the same two counts: an
    // item set apart while its subtitle names nothing is a finding nobody can
    // act on, and one that stays quiet while the line names a trouble is
    // worse. The two counts are what raises it; the tools and the model above
    // deliberately do not (see the header).
    m_item->setStatus(m_notesWithoutTranscript > 0 || m_notesWithoutCategory > 0
                          ? KStatusNotifierItem::NeedsAttention
                          : KStatusNotifierItem::Active);

    QStringList parts;
    if (!m_unavailableTools.isEmpty()) {
        parts.append(i18n("Not available: %1", m_unavailableTools.join(QStringLiteral(", "))));
    }
    if (!m_missingModel.isEmpty()) {
        // Whole and not folded into the list above: it is a sentence and not a
        // name, because SPEC 12 asks it to say what is missing **and where to
        // get it**.
        parts.append(m_missingModel);
    }
    // The two troubles that stand for as long as their cause does, each with
    // its own number and each a whole statement: only with both of them up
    // does the line read "2 notes without a transcript · 1 note without a
    // category", and a half shortened for that one case would be a sentence
    // fragment on every day the other half is quiet.
    if (m_notesWithoutTranscript > 0) {
        parts.append(i18np("%1 note without a transcript", "%1 notes without a transcript",
                           m_notesWithoutTranscript));
    }
    if (m_notesWithoutCategory > 0) {
        parts.append(i18np("%1 note without a category", "%1 notes without a category",
                           m_notesWithoutCategory));
    }
    // Last of the five, and it raises no state — the header says why. It is a
    // whole sentence like the model's above, because SPEC 11 asks it to name
    // the count **and** the age: only both together read as news under either
    // of the two criteria.
    if (!m_overflow.isEmpty()) {
        parts.append(m_overflow);
    }
    // Sixth and last, appended for the reason the overflow is appended: a new
    // part at the end moves none of the five above, and the order is fixed only
    // so the line does not reshuffle itself under the pointer. It raises no
    // state — see the header.
    if (!m_bundlesDropped.isEmpty()) {
        parts.append(m_bundlesDropped);
    }
    m_item->setToolTipSubTitle(parts.isEmpty() ? i18n("Capture thoughts quickly")
                                               : parts.join(QStringLiteral(" · ")));
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

    QAction *recorderAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("audio-input-microphone")),
                        i18n("Record voice note"));
    // A hint like the one above it, and never a binding: same reasoning, same
    // context (SPEC 2.4, issue #60).
    recorderAction->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_N));
    recorderAction->setShortcutContext(Qt::WidgetShortcut);
    connect(recorderAction, &QAction::triggered, this, &TrayIcon::recorderRequested);

    // Separates capturing from looking at and working on — the only grouping
    // among the working paths (wireframe 5a).
    menu->addSeparator();

    const QAction *libraryAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("view-list-text")), i18n("Open library"));
    connect(libraryAction, &QAction::triggered, this, &TrayIcon::libraryRequested);

    // The on-demand road of SPEC 7.2, and it stays enabled under all three
    // settings: being asked for a run is what "on demand" means, and the two
    // other triggers have no reason to refuse one (issue #15).
    const QAction *analyzeAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("system-run")), i18n("Analyze now"));
    connect(analyzeAction, &QAction::triggered, this, &TrayIcon::analysisRequested);

    // The door into the review of SPEC 9 (issue #30). It stood here as a
    // disabled entry until the review existed; the wording and the symbol are
    // the ones it carried, because it is the same entry and not a new one.
    //
    // Kept as a member, unlike its neighbours: the counter of SPEC 10 is
    // written onto this label while the menu stands (issue #40).
    m_proposalsAction =
        menu->addAction(QIcon::fromTheme(QStringLiteral("tools-wizard")), i18n("Suggestions"));
    connect(m_proposalsAction, &QAction::triggered, this, &TrayIcon::proposalsRequested);

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
