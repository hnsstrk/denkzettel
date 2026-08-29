#include "settings/capturepage.h"

#include <KLocalizedString>

#include <QCheckBox>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

CapturePage::CapturePage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    // The object name is what KConfigDialogManager binds to the item
    // `StoreOrigin` of the group "Capture"; nothing else connects the two.
    auto *store = new QCheckBox(i18n("Store the note's origin"), this);
    store->setObjectName(QStringLiteral("kcfg_StoreOrigin"));
    layout->addWidget(store);

    // The sentence SPEC 13 asks for: it says what is kept, and it says where it
    // can be taken back. A checkbox that only names the feature would leave the
    // user to guess what a window title is, and this switch exists because the
    // answer must not have to be guessed.
    //
    // **Both of them are shown**, and the sentence says so since the customer's
    // report of 29.08.2026. The wording before it promised the opposite — „the
    // name of the application is not shown and goes with it" — because the id
    // was held to be there for the classification of SPEC 7 alone. It is on the
    // origin line since, in front of the title, and a privacy sentence that
    // names less than the window shows is as wrong as one that names more.
    auto *explains = new QLabel(i18n("Stored are the name of the application and the window title at "
                                     "the moment of capture. Both stand in the detail view and are "
                                     "deleted there together."),
                                this);
    explains->setWordWrap(true);
    // Under the box and indented to its label, not beside it: the sentence
    // belongs to the switch and reads as its second line.
    explains->setIndent(store->style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, store)
                        + store->style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, nullptr, store));
    layout->addWidget(explains);

    layout->addStretch();
}
