#include "capture/textcontrast.h"

#include <QColor>

#include <algorithm>
#include <cmath>

namespace
{
/**
 * The relative luminance of a colour after WCAG 2.1, 0 for black and 1 for
 * white. Written out because Qt has no counterpart: `QColor::lightnessF()` is
 * the HSL lightness, which ranks a saturated blue and a saturated yellow alike
 * and would call the reversal of issue #97 an equal pair.
 */
double relativeLuminance(const QColor &colour)
{
    auto channel = [](double value) {
        return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };

    return 0.2126 * channel(colour.redF()) + 0.7152 * channel(colour.greenF())
        + 0.0722 * channel(colour.blueF());
}
}

double capture::contrastRatio(const QColor &one, const QColor &other)
{
    const double a = relativeLuminance(one);
    const double b = relativeLuminance(other);

    return (std::max(a, b) + 0.05) / (std::min(a, b) + 0.05);
}

bool capture::noteIsTheQuieterWriting(const WritingChoice &choice)
{
    auto poorest = [&choice](const QColor &writing) {
        return std::min(contrastRatio(writing, choice.groundOverWhite),
                        contrastRatio(writing, choice.groundOverBlack));
    };

    return poorest(choice.note) < poorest(choice.placeholder);
}
