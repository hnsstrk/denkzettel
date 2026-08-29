#include "ui/framecontrast.h"

#include <KColorScheme>

namespace library
{
QColor frameContrastMix(const QColor &ground, const QColor &text)
{
    // In float, because that is what QColor works in: the channels come back as
    // float and fromRgbF takes float, so a double ratio would narrow three
    // times over.
    const auto share = static_cast<float>(KColorScheme::frameContrast());

    return QColor::fromRgbF(ground.redF() * (1 - share) + text.redF() * share,
                            ground.greenF() * (1 - share) + text.greenF() * share,
                            ground.blueF() * (1 - share) + text.blueF() * share);
}
}
