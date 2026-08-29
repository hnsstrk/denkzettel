#include "settings/analysispage.h"

#include "settings/settings.h"

#include <KLocalizedString>

#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

AnalysisPage::AnalysisPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    layout->addLayout(form);

    // Flat, titleless group box with auto-exclusive children: what
    // KConfigDialogManager stores from it is the INDEX of the checked button,
    // so the order below is the order of Settings::AnalysisTrigger and must
    // stay that way. The reasoning stands in full in aiproviderpage.cpp.
    auto *trigger = new QGroupBox(this);
    trigger->setObjectName(QStringLiteral("kcfg_Trigger"));
    trigger->setFlat(true);
    auto *choices = new QVBoxLayout(trigger);
    choices->setContentsMargins(0, 0, 0, 0);
    // On the box as well as on its layout — see aiproviderpage.cpp for the
    // pixels this line is worth.
    trigger->setContentsMargins(0, 0, 0, 0);
    choices->addWidget(new QRadioButton(i18n("at once after saving"), trigger));
    auto *periodic = new QRadioButton(i18n("periodically"), trigger);
    choices->addWidget(periodic);
    choices->addWidget(new QRadioButton(i18n("on demand only"), trigger));
    form->addRow(i18n("Start analysis:"), trigger);

    auto *interval = new QSpinBox(this);
    interval->setObjectName(QStringLiteral("kcfg_IntervalMinutes"));
    interval->setRange(Settings::MinimumAnalysisInterval, Settings::MaximumAnalysisInterval);
    interval->setSingleStep(Settings::MinimumAnalysisInterval);
    // A fixed suffix, and the floor of five minutes is what allows it: the
    // singular "1 Minute" can never come up, so the word does not have to be
    // declined and KPluralHandlingSpinBox out of KTextWidgets stays out of the
    // dependencies (see Settings, item IntervalMinutes).
    interval->setSuffix(i18nc("@item:valuesuffix the unit of the analysis interval", " minutes"));
    form->addRow(i18n("Interval:"), interval);

    // The interval only means anything under "periodically", and greyed out is
    // what says so without a sentence. No button is checked until the dialog's
    // manager reads the setting, so the first state arrives through this signal.
    connect(periodic, &QRadioButton::toggled, interval, &QSpinBox::setEnabled);
    interval->setEnabled(false);

    layout->addStretch();
}
