#include "ui/notelistdelegate.h"

#include "ui/elidedlines.h"
#include "ui/notelistmodel.h"

#include <QApplication>
#include <QFontDatabase>
#include <QPainter>

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
        // A head is not selectable, so the view hands it over disabled. Asking
        // the palette for its Normal colour keeps it from being greyed out —
        // it is a heading, not an unavailable entry. No line, no background:
        // both would weaken the selection mark below it (wireframe 3a).
        drawLine(painter,
                 entry.rect,
                 headTopPadding(index.row()),
                 groupHeadFont(),
                 entry.palette.color(QPalette::Normal, QPalette::Text),
                 index.data(Qt::DisplayRole).toString());

        painter->restore();
        return;
    }

    QStyle *style = entry.widget ? entry.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &entry, painter, entry.widget);

    const bool selected = entry.state.testFlag(QStyle::State_Selected);
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
