#include "ui/notelistdelegate.h"

#include "ui/elidedlines.h"
#include "ui/notelistmodel.h"

#include <KColorScheme>

#include <QAbstractItemView>
#include <QApplication>
#include <QFontDatabase>
#include <QItemSelectionModel>
#include <QPaintDevice>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace
{
constexpr int HorizontalPadding = 12;
constexpr int VerticalPadding = 9;

/** Distance between the timestamp and the note text. */
constexpr int Gap = 3;

/** Subject and preview — the two lines of note text an entry shows. */
constexpr int TextLines = 2;

/**
 * Room above a group head. The first head of the list sits close under the
 * upper edge, every following one keeps the larger distance that separates it
 * from the group above (wireframe 3a).
 */
constexpr int FirstHeadTopPadding = 6;
constexpr int HeadTopPadding = 14;
constexpr int HeadBottomPadding = 6;

QFont timestampFont()
{
    return QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
}

/** Small like the timestamp, half-bold, in plain text colour (wireframe 3a). */
QFont groupHeadFont()
{
    QFont font = timestampFont();
    font.setWeight(QFont::DemiBold);
    return font;
}

int headTopPadding(int row)
{
    return row == 0 ? FirstHeadTopPadding : HeadTopPadding;
}

bool isGroupHead(const QModelIndex &index)
{
    return index.data(NoteListModel::GroupHeaderRole).toBool();
}

/**
 * Colour of both separator lines: list ground and text colour mixed in the
 * ratio KColorScheme::frameContrast() — the way Kirigami colours its own
 * separators (wireframe 3a, issue #101).
 *
 * Deliberately not a palette role. Over the eighteen colour schemes measured on
 * 06.08.2026 the roles that sit near the ground — AlternateBase above all —
 * stay between 1.00 : 1 and 1.21 : 1 against it, and at a group boundary
 * AlternateBase lands at 1.00 : 1 every single time, because the head is a row
 * of the model and eats a stripe of its own. This mixture lies between
 * 1.24 : 1 and 1.93 : 1.
 *
 * The ratio comes out of the `[KDE]` group of the application configuration and
 * not out of the colour scheme: no installed scheme carries the key, so it
 * reads 0.20 everywhere until somebody sets it (measured 07.08.2026). What
 * pulls the contrasts apart are the colours of the schemes.
 *
 * Read from the Normal colour group, because a group head is handed to the
 * delegate disabled — a line greyed out above one row and not below the next
 * would be a line of two colours.
 */
QColor separatorColor(const QPalette &palette)
{
    const QColor ground = palette.color(QPalette::Normal, QPalette::Base);
    const QColor text = palette.color(QPalette::Normal, QPalette::Text);
    // In float, because that is what QColor works in: the channels come back as
    // float and fromRgbF takes float, so a double ratio would narrow three
    // times over.
    const auto share = static_cast<float>(KColorScheme::frameContrast());

    return QColor::fromRgbF(ground.redF() * (1 - share) + text.redF() * share,
                            ground.greenF() * (1 - share) + text.greenF() * share,
                            ground.blueF() * (1 - share) + text.blueF() * share);
}

/**
 * The rectangle a hairline of one logical point occupies, laid on the device
 * pixel grid (issue #101, UI review of Sprint 9, L9).
 *
 * `top` is the logical row the line sits in. At the customer's ratio of 1.6 a
 * logical point covers 1.6 device pixel rows, and which whole rows those become
 * depends on where `top` falls in the grid: measured on his scaling, the same
 * line came out one device pixel thick above one entry and two above the next,
 * and a group line could end up thinner than the entry lines beneath it. Then
 * the weight says the opposite of the length — while the rule is that the
 * ranking comes from the length of the stroke and not from its weight
 * (customer decision of 06.08.2026).
 *
 * Hence a whole number of device pixel rows, never fewer than one. That single
 * term carries the whole assurance: two edges that are an integer number of
 * device pixels apart round to values that are the same integer apart, wherever
 * they fall. Measured over 280 positions — seven ratios, twenty rows, two
 * painter origins — the height is the same in every one of them.
 *
 * Rounded rather than truncated: truncation would leave 0.625 logical points at
 * 1.6, less than a device pixel row of the unscaled list. Rounding never goes
 * below one device pixel row, and one device pixel row is exactly what the line
 * is at ratio 1 — the appearance the customer approved. Below 1.5 that means
 * fewer than one logical point (1.25 → 1 row → 0.8 points); it is the same ink
 * on the same screen as at ratio 1, so the line never gets thinner than what
 * was accepted.
 *
 * At ratio 1 the term comes out exactly as before, which is why the pixel
 * checks of AK 1 to AK 3 are untouched by this.
 *
 * **No attempt is made to put the upper edge on a device pixel boundary.** One
 * stood here until 08.08.2026 (`std::round(top * ratio) / ratio`) and was
 * removed as measured: it never reached a boundary, because the painter origin
 * is off the grid itself — the list viewport starts at logical 48, which is
 * 76.8 device pixels at 1.6, and this function only ever sees widget-local
 * coordinates. What the term did instead was shift the line by one device pixel
 * in 8 of those 280 positions, without changing a single height. An arbitrary
 * shift under the name of an alignment (karpathy follow-up of Sprint 9, N1).
 */
QRectF hairline(const QPaintDevice *device, int left, int top, int width)
{
    const qreal ratio = device ? device->devicePixelRatioF() : 1.0;
    const qreal rows = std::max(1.0, std::round(ratio));

    return QRectF(left, top, width, rows / ratio);
}

/**
 * True when `row` is selected in the view the delegate is painting for.
 *
 * Asked of the view rather than read out of `option.state`, because the entry
 * line has to know it of the row *below* as well — and `option.state` only ever
 * speaks of the row being painted. The cast was measured: `option.widget`
 * carries the QListView in every paint, and its selection model answers
 * (issue #101). Where there is no view — nobody builds this list without one —
 * the answer is „not selected“, and the line stays.
 */
bool isSelectedIn(const QStyleOptionViewItem &option, const QModelIndex &row)
{
    const auto *view = qobject_cast<const QAbstractItemView *>(option.widget);
    if (!view || !view->selectionModel()) {
        return false;
    }

    return view->selectionModel()->isSelected(row);
}
}

/**
 * Draws one line of text `top` pixels below the upper edge of `row`, elided at
 * the width the line has.
 *
 * Every text of the list goes through here — head, timestamp, subject and
 * preview alike. That is what keeps them on one left edge: there is a single
 * place where it is worked out, so a second one cannot drift away from it.
 */
void NoteListDelegate::drawLine(QPainter *painter,
                                const QRect &row,
                                int top,
                                const QFont &font,
                                const QColor &color,
                                const QString &text)
{
    const QFontMetrics metrics(font);
    const int width = textWidth(row);

    painter->setFont(font);
    painter->setPen(color);
    painter->drawText(QRect(textLeft(row), row.y() + top, width, metrics.height()),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      metrics.elidedText(text, Qt::ElideRight, width));
}

NoteListDelegate::NoteListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

int NoteListDelegate::textLeft(const QRect &row)
{
    return row.x() + HorizontalPadding;
}

int NoteListDelegate::textWidth(const QRect &row)
{
    return row.width() - 2 * HorizontalPadding;
}

void NoteListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem entry = option;
    initStyleOption(&entry, index);
    // Selection, hover and focus come from the style, the text does not: the
    // style would draw the note as a single line.
    entry.text.clear();

    const int width = textWidth(entry.rect);
    if (width <= 0) {
        return;
    }

    painter->save();

    if (isGroupHead(index)) {
        // The group line: the topmost pixel row of every head but the first,
        // over the full width of the row (wireframe 3a, issue #101). Its length
        // is what tells a group boundary from a note boundary — same colour,
        // same thickness, the inset line between two notes and the full one
        // here. It sits in the upper 14 px the head keeps free anyway, so no
        // row grows.
        //
        // It stays even when the note right above is the selected one: the
        // exception belongs to the entry line, and the wireframe draws this
        // very case at the head „Gestern“.
        //
        // Drawn on bare list ground — the head branch returns before
        // drawControl() and paints no background of its own.
        if (index.row() > 0) {
            painter->fillRect(
                hairline(painter->device(), entry.rect.x(), entry.rect.y(), entry.rect.width()),
                separatorColor(entry.palette));
        }

        // A head is not selectable, so the view hands it over disabled. Asking
        // the palette for its Normal colour keeps it from being greyed out —
        // it is a heading, not an unavailable entry. No background and no
        // selection: both would weaken the selection mark below it
        // (wireframe 3a).
        drawLine(painter,
                 entry.rect,
                 headTopPadding(index.row()),
                 groupHeadFont(),
                 entry.palette.color(QPalette::Normal, QPalette::Text),
                 index.data(Qt::DisplayRole).toString());

        painter->restore();
        return;
    }

    const QStyle *style = entry.widget ? entry.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &entry, painter, entry.widget);

    const bool selected = entry.state.testFlag(QStyle::State_Selected);

    // The entry line: the last pixel row of this note, inset to the text edge —
    // the same 12 px the timestamp starts at, asked of the same two functions
    // so that the number exists once (wireframe 3a, issue #101). It sits in the
    // lower of the 9 px the entry already keeps free, so no row grows.
    //
    // Drawn after drawControl(), which fills the whole rectangle and would
    // paint over it.
    //
    // Three cases carry no line. Under the last note of a group, because the
    // row below is that group's head and the head draws the full-width line
    // itself — the rectangles touch without a gap, so a line here would make a
    // double one of two pixel rows. And at either edge of the selected row,
    // because a second separator there would compete with the selection mark:
    // that is the one reason of the old „no lines at all“ rule that still
    // stands.
    const QModelIndex below = index.sibling(index.row() + 1, index.column());
    if (below.isValid() && !isGroupHead(below) && !selected && !isSelectedIn(entry, below)) {
        painter->fillRect(hairline(painter->device(), textLeft(entry.rect), entry.rect.bottom(), width),
                          separatorColor(entry.palette));
    }

    const QColor textColor = entry.palette.color(selected ? QPalette::HighlightedText : QPalette::Text);
    const QColor dimmedColor = selected ? textColor : entry.palette.color(QPalette::PlaceholderText);

    // Subject and preview differ in colour alone — bold would read as "unread"
    // in a list, and a bold subject cut off mid-sentence looks like a fault
    // (wireframe 3b).
    const library::EntryText text =
        library::subjectAndPreview(index.data(Qt::DisplayRole).toString(), entry.font, width);

    const int timestampHeight = QFontMetrics(timestampFont()).height();
    const int lineHeight = QFontMetrics(entry.font).height();

    int y = VerticalPadding;
    drawLine(painter, entry.rect, y, timestampFont(), dimmedColor,
             index.data(NoteListModel::TimestampRole).toString());

    y += timestampHeight + Gap;
    drawLine(painter, entry.rect, y, entry.font, textColor, text.subject);

    y += lineHeight;
    drawLine(painter, entry.rect, y, entry.font, dimmedColor, text.preview);

    painter->restore();
}

QSize NoteListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (isGroupHead(index)) {
        return QSize(option.rect.width(),
                     headTopPadding(index.row()) + QFontMetrics(groupHeadFont()).height() + HeadBottomPadding);
    }

    // Every entry is as tall as a full one, however short its note: a list of
    // uniform rows is easier to scan than one that jumps. The single-line note
    // keeps its empty preview row (wireframe 3b).
    const int height = 2 * VerticalPadding + QFontMetrics(timestampFont()).height() + Gap
        + TextLines * QFontMetrics(option.font).height();

    return QSize(option.rect.width(), height);
}
