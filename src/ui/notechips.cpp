#include "ui/notechips.h"

#include "platform/systemfonts.h"
#include "ui/framecontrast.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QRectF>

#include <utility>

namespace
{
/** Room left and right of the word inside a pill, as drawn in wireframe 2b. */
constexpr int HorizontalPadding = 8;
constexpr int VerticalPadding = 3;

/** Distance between two pills. */
constexpr int Spacing = 6;

/** Width a pill takes for `text` under `metrics`. */
int chipWidth(const QFontMetrics &metrics, const QString &text)
{
    return metrics.horizontalAdvance(text) + (2 * HorizontalPadding);
}
}

NoteChips::NoteChips(QWidget *parent)
    : QWidget(parent)
{
    hide();
}

void NoteChips::setChips(const QString &category, const QStringList &tags)
{
    m_category = category;
    m_tags = tags;

    // The UX decision of 29.08.2026: without both, the row is not there at all
    // — a hidden widget takes neither height nor the spacing of its layout.
    setVisible(!isEmpty());
    updateGeometry();
    update();
}

bool NoteChips::isEmpty() const
{
    return m_category.isEmpty() && m_tags.isEmpty();
}

QSize NoteChips::sizeHint() const
{
    const QFontMetrics metrics(platform::smallestReadableFont());
    const int height = metrics.height() + (2 * VerticalPadding);

    int width = 0;
    if (!m_category.isEmpty()) {
        width += chipWidth(metrics, m_category);
    }
    for (const QString &tag : std::as_const(m_tags)) {
        width += (width > 0 ? Spacing : 0) + chipWidth(metrics, tag);
    }

    return {width, height};
}

QSize NoteChips::minimumSizeHint() const
{
    // The height is what the row owes its layout; the width may be squeezed,
    // and what no longer fits is left undrawn (see paintEvent).
    return {0, sizeHint().height()};
}

void NoteChips::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const QFont font = platform::smallestReadableFont();
    const QFontMetrics metrics(font);
    const int chipHeight = metrics.height() + (2 * VerticalPadding);
    // A pill, so the radius is half the height — not the fixed 9 px of the
    // drawing, which was measured at one font size.
    const qreal radius = chipHeight / 2.0;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font);

    // One colour for both pills, and it is the one the list draws its separator
    // lines in: the category is filled with it, a tag outlined in it. Read at
    // every paint rather than once, because the window is built at daemon start
    // and has to follow a colour scheme changed underneath it (issue #54).
    //
    // **Measured, and it is not the colour the decision of 29.08.2026 named as
    // its approach.** `KColorScheme(View).background(AlternateBackground)`
    // stands at **1.05 : 1** against the ground of the reading pane under
    // BreezeDark and **1.07 : 1** under BreezeLight — a pill nobody can see;
    // every other ground role of the two schemes lands between 1.04 and
    // 1.19 : 1, which is the same ceiling issue #101 measured for the separator
    // lines. This mixture reaches **1.89 : 1** dark and **1.47 : 1** light,
    // with the type on it at 8.14 : 1 and 9.06 : 1.
    //
    // It is read off the palette rather than out of KColorScheme, and that
    // settles finding 38 of CLAUDE.md by construction: a runner that sets its
    // palette by hand gets a pill in the colours of that palette.
    const QColor ground = palette().color(QPalette::Normal, QPalette::Window);
    const QColor type = palette().color(QPalette::Normal, QPalette::WindowText);
    const QColor pill = library::frameContrastMix(ground, type);

    int x = 0;
    const auto drawChip = [&](const QString &text, bool filled) {
        const int chip = chipWidth(metrics, text);
        // ponytail: one row, and a pill that no longer fits is dropped rather
        // than wrapped. SPEC 7.2 caps a note at four tags of one word each; a
        // flow layout is the way up if that ever changes.
        if (x + chip > width()) {
            return;
        }

        const QRectF box(x, 0, chip, chipHeight);
        if (filled) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(pill);
            painter.drawRoundedRect(box, radius, radius);
        } else {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(pill);
            // Half a pen width inside, or the outline would be cut off at the
            // edges of the widget.
            painter.drawRoundedRect(box.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
        }
        painter.setPen(type);
        painter.drawText(box, Qt::AlignCenter, text);

        x += chip + Spacing;
    };

    // The category first, and it is the filled one (UX decision 2026-08-29).
    if (!m_category.isEmpty()) {
        drawChip(m_category, true);
    }
    for (const QString &tag : std::as_const(m_tags)) {
        drawChip(tag, false);
    }
}

void NoteChips::changeEvent(QEvent *event)
{
    // The pills are measured from the smallest readable font, which is not the
    // widget's own — so nothing tells this row that the font changed under it
    // (issue #68).
    if (event->type() == QEvent::ApplicationFontChange) {
        updateGeometry();
        update();
    }

    QWidget::changeEvent(event);
}
