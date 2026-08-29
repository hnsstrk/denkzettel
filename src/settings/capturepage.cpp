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
    // **Two things are stored and one of them is shown**, and the sentence says
    // so since the review of 29.08.2026: the application id feeds the
    // classification of SPEC 7 and stands in no window (SPEC 5.1 says why). The
    // earlier wording — „the stamp stands in the detail view" — read as if both
    // were on show, and a privacy sentence that promises more visibility than
    // there is, is the wrong one to be loose in.
    auto *explains = new QLabel(i18n("Stored are the name of the application and the window title at "
                                     "the moment of capture. The title stands in the detail view and "
                                     "can be deleted there; the name of the application is not shown "
                                     "and goes with it."),
                                this);
    explains->setWordWrap(true);
    // Under the box and indented to its label, not beside it: the sentence
    // belongs to the switch and reads as its second line.
    explains->setIndent(store->style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, store)
                        + store->style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, nullptr, store));
    layout->addWidget(explains);

    layout->addStretch();
}
