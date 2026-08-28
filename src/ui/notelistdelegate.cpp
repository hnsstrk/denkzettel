#include "ui/notelistdelegate.h"

#include "platform/systemfonts.h"

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

/** Clear ground between the head label and the line beside it (issue #104). */
constexpr int HeadLineGap = 8;

QFont timestampFont()
{
    return platform::smallestReadableFont();
}

/**
 * The type a group head is set in — the text size of the application, bold
 * (issue #104).
 *
 * It was the smallest type of the list until 11.08.2026, the size of the
 * timestamp and therefore smaller than the note text it stands over: a heading
 * without rank. The rank is one of the two features that tell a group boundary
 * from a note boundary now, the place of the line being the other.
 */
QFont groupHeadFont()
{
    QFont font = platform::generalFont();
    font.setWeight(QFont::Bold);
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
 * Deliberately not a palette role: measured over eighteen colour schemes, the
 * roles near the ground stay under 1.21 : 1 against it and AlternateBase lands
 * at 1.00 : 1 at every group boundary, while this mixture reaches 1.93 : 1.
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
 * A whole number of device pixel rows, never fewer than one. Filled as a
 * logical rectangle, the same line came out one device pixel thick above one
 * entry and two above the next.
 *
 * **Rounded, not truncated:** one device pixel row is the floor, and that is
 * exactly what the line is at ratio 1 — the appearance the user approved.
 *
 * **No attempt to put the upper edge on a device pixel boundary.** One stood
 * here and was removed as measured: it changed no height and only moved the
 * line. The boundary it rounded to was the widget's, not the screen's — this
 * function sees only widget-local coordinates (Sprint 9, N1; B8).
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
 * speaks of the row being painted (issue #101). Where there is no view, the
 * answer is „not selected“ and the line stays.
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
 * the width the line has, and hands back how wide the text came out.
 *
 * Every text of the list goes through here — head, timestamp, subject and
 * preview alike. That is what keeps them on one left edge: there is a single
 * place where it is worked out, so a second one cannot drift away from it.
 *
 * The width is handed back for the head line of issue #104, which begins where
 * the label ends — worked out here so the eliding cannot be forgotten at the
 * call site.
 */
int NoteListDelegate::drawLine(QPainter *painter,
                               const QRect &row,
                               int top,
                               const QFont &font,
                               const QColor &color,
                               const QString &text)
{
    const QFontMetrics metrics(font);
    const int width = textWidth(row);
    const QString drawn = metrics.elidedText(text, Qt::ElideRight, width);

    painter->setFont(font);
    painter->setPen(color);
    painter->drawText(QRect(textLeft(row), row.y() + top, width, metrics.height()),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      drawn);

    return metrics.horizontalAdvance(drawn);
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
        const QFont font = groupHeadFont();
        const int top = headTopPadding(index.row());

        // A head is not selectable, so the view hands it over disabled. Asking
        // the palette for its Normal colour keeps it from being greyed out —
        // it is a heading, not an unavailable entry. No background and no
        // selection: both would weaken the selection mark below it
        // (wireframe 3a).
        const int label = drawLine(painter,
                                   entry.rect,
                                   top,
                                   font,
                                   entry.palette.color(QPalette::Normal, QPalette::Text),
                                   index.data(Qt::DisplayRole).toString());

        // The group line: beside the label, on half the height of the head
        // type, 8 px clear of it and out to the same right text edge the entry
        // line stops at — the form of Kirigami.ListSectionHeader (issue #104).
        //
        // It used to run over the whole width in the topmost pixel row of the
        // head, and then the only thing telling a group boundary from a note
        // boundary was the length of a stroke — the user did not find the
        // boundary (finding of 11.08.2026). Two features carry it now, and
        // neither is a degree of the other: the line is somewhere else, and the
        // head has a rank of type that the note text has not.
        //
        // Every head carries it, the first one included: this line is part of
        // the heading itself, not a boundary, and a first head without it would
        // read as a fault rather than as an exception.
        //
        // Where the label fills the line, nothing is drawn: the 8 px of clear
        // ground are what keeps the line from reading as an underscore of the
        // last letter.
        const int left = textLeft(entry.rect) + label + HeadLineGap;
        const int right = textLeft(entry.rect) + width;
        if (left < right) {
            painter->fillRect(hairline(painter->device(),
                                       left,
                                       entry.rect.y() + top + QFontMetrics(font).height() / 2,
                                       right - left),
                              separatorColor(entry.palette));
        }

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
    // row below is that group's head: a line of the note kind at a group
    // boundary would say „note boundary" in the one place where the list has
    // something else to say. And at either edge of the selected row, because a
    // second separator there would compete with the selection mark.
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

    // A voice note writes "▶ 0:41" opposite its timestamp, on the same line and
    // out to the same right text edge (wireframe 2b, 3a); a text note writes
    // nothing there. Not through drawLine(), which is the left edge of this
    // list and elides against it — this one is the right edge and is short
    // enough never to meet the timestamp.
    const QString spoken = index.data(NoteListModel::AudioRole).toString();
    if (!spoken.isEmpty()) {
        painter->setFont(timestampFont());
        painter->setPen(dimmedColor);
        painter->drawText(QRect(textLeft(entry.rect), entry.rect.y() + y, width, timestampHeight),
                          Qt::AlignRight | Qt::AlignVCenter,
                          spoken);
    }

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
