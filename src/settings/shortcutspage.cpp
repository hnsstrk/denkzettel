#include "settings/shortcutspage.h"

#include <KColorScheme>
#include <KKeySequenceWidget>
#include <KLocalizedString>

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <algorithm>

ShortcutsPage::ShortcutsPage(GlobalShortcuts *shortcuts, QWidget *parent)
    : QWidget(parent)
    , m_shortcuts(shortcuts)
    , m_rows{Row{GlobalShortcuts::Shortcut::Capture, new KKeySequenceWidget(this)},
             Row{GlobalShortcuts::Shortcut::Recorder, new KKeySequenceWidget(this)}}
    , m_readback(new QLabel(this))
{
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    layout->addLayout(form);

    for (const Row &row : m_rows) {
        // The combination the widget's own header names for a global shortcut.
        // The local ones are left out on purpose: this page hands it no action
        // collection, and a check against an empty list is a check that can
        // never come out anything but green.
        row.editor->setCheckForConflictsAgainst(KKeySequenceWidget::StandardShortcuts
                                                | KKeySequenceWidget::GlobalShortcuts);
        // The documented way to build it — and its documented reason is wrong.
        // `kkeysequencewidget.h:296` says the name keeps our own registered
        // sequence from reporting a conflict with itself; measured 29.08.2026,
        // `isGlobalShortcutAvailable` answers 0 for our own name exactly as it
        // does for an empty and for a foreign one, because
        // `Component::isShortcutAvailable` resolves our name to
        // `shortcutContext("default")` and that context holds the key. The line
        // stays, the claim does not (CLAUDE.md, finding 50).
        row.editor->setComponentName(GlobalShortcuts::shortcutComponent());
        form->addRow(i18nc("@label:textbox the shortcut for an action", "%1:", GlobalShortcuts::label(row.which)),
                     row.editor);
        connect(row.editor, &KKeySequenceWidget::keySequenceChanged, this, &ShortcutsPage::changed);
    }

    // The result as a small coloured line right underneath, the same shape the
    // other pages use (wireframe 1d:172–173).
    const QFont bodyFont = font();
    if (bodyFont.pointSizeF() > 0) {
        QFont small = bodyFont;
        small.setPointSizeF(bodyFont.pointSizeF() * 0.9);
        m_readback->setFont(small);
    }
    m_readback->setWordWrap(true);
    // KColorScheme and not the palette: NegativeText does not exist in QPalette
    // at all. Set once — this line only ever reports a failure.
    const KColorScheme scheme(QPalette::Normal, KColorScheme::View);
    QPalette colours = m_readback->palette();
    colours.setColor(QPalette::WindowText, scheme.foreground(KColorScheme::NegativeText).color());
    m_readback->setPalette(colours);
    form->addRow(m_readback);

    // Nothing on this page wants to grow (UX decision of 29.08.2026).
    layout->addStretch();

    load();
}

void ShortcutsPage::load()
{
    for (const Row &row : m_rows) {
        // NoValidate: filling the field is not the user setting a shortcut, and
        // the conflict prompt belongs to the moment they do.
        row.editor->setKeySequence(GlobalShortcuts::assignedSequence(row.which), KKeySequenceWidget::NoValidate);
    }
    report({});
}

void ShortcutsPage::loadDefaults()
{
    for (const Row &row : m_rows) {
        row.editor->setKeySequence(GlobalShortcuts::defaultSequence(row.which), KKeySequenceWidget::NoValidate);
    }
    report({});
}

bool ShortcutsPage::hasChanged() const
{
    return std::any_of(m_rows.cbegin(), m_rows.cend(), [](const Row &row) {
        return row.editor->keySequence() != GlobalShortcuts::assignedSequence(row.which);
    });
}

bool ShortcutsPage::isDefault() const
{
    return std::all_of(m_rows.cbegin(), m_rows.cend(), [](const Row &row) {
        return row.editor->keySequence() == GlobalShortcuts::defaultSequence(row.which);
    });
}

bool ShortcutsPage::takeReadbackFailure()
{
    const bool failed = m_readbackFailed;
    m_readbackFailed = false;
    return failed;
}

void ShortcutsPage::save()
{
    QStringList failures;
    bool wroteSomething = false;

    for (const Row &row : m_rows) {
        const QKeySequence wanted = row.editor->keySequence();
        if (wanted == GlobalShortcuts::assignedSequence(row.which)) {
            // Nothing to write, and writing anyway would send the shortcut of
            // an untouched row through the service on every Apply.
            continue;
        }

        wroteSomething = true;
        const QKeySequence held = m_shortcuts->changeSequence(row.which, wanted);
        if (held == wanted) {
            continue;
        }

        // The display goes back to what the service really holds. Leaving the
        // wish standing would show a shortcut that no key press finds.
        row.editor->setKeySequence(held, KKeySequenceWidget::NoValidate);
        failures.append(held.isEmpty()
                            ? i18n("The shortcut service holds nothing for “%1”.",
                                   GlobalShortcuts::label(row.which))
                            : i18n("The shortcut service holds %2 for “%1”.",
                                   GlobalShortcuts::label(row.which),
                                   held.toString(QKeySequence::NativeText)));
    }

    // **"Nothing written" is not "everything is in order".** A failed save puts
    // every field back onto the value the service holds, so the save right
    // after it finds nothing left to write — and a report cleared there would
    // take the reason off the screen while the dialog is still refusing to
    // close. Only a save that actually wrote something says anything new.
    if (wroteSomething) {
        report(failures);
    }
}

void ShortcutsPage::report(const QStringList &failures)
{
    m_readbackFailed = !failures.isEmpty();
    m_readback->setText(failures.join(QLatin1Char(' ')));
}
