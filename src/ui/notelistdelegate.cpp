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

/** Lines of note text an entry shows (wireframe 2b). */
constexpr int TextLines = 2;

QFont timestampFont()
{
    return QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
}
}

NoteListDelegate::NoteListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void NoteListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem entry = option;
    initStyleOption(&entry, index);
    // Selection, hover and focus come from the style, the text does not: the
    // style would draw the note as a single line.
    entry.text.clear();

    QStyle *style = entry.widget ? entry.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &entry, painter, entry.widget);

    const bool selected = entry.state.testFlag(QStyle::State_Selected);
    const QColor textColor = entry.palette.color(selected ? QPalette::HighlightedText : QPalette::Text);
    const QColor dimmedColor = selected ? textColor : entry.palette.color(QPalette::PlaceholderText);

    const QRect content =
        entry.rect.adjusted(HorizontalPadding, VerticalPadding, -HorizontalPadding, -VerticalPadding);

    painter->save();

    const QFontMetrics timestampMetrics(timestampFont());
    painter->setFont(timestampFont());
    painter->setPen(dimmedColor);
    painter->drawText(QRect(content.x(), content.y(), content.width(), timestampMetrics.height()),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      timestampMetrics.elidedText(index.data(NoteListModel::TimestampRole).toString(),
                                                  Qt::ElideRight,
                                                  content.width()));

    const QFontMetrics textMetrics(entry.font);
    painter->setFont(entry.font);
    painter->setPen(textColor);

    int y = content.y() + timestampMetrics.height() + Gap;
    const QStringList lines =
        library::elidedLines(index.data(Qt::DisplayRole).toString(), entry.font, content.width(), TextLines);
    for (const QString &line : lines) {
        painter->drawText(QRect(content.x(), y, content.width(), textMetrics.height()),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          line);
        y += textMetrics.height();
    }

    painter->restore();
}

QSize NoteListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
{
    // Every entry is as tall as a full one, however short its note: a list of
    // uniform rows is easier to scan than one that jumps.
    const int height = 2 * VerticalPadding + QFontMetrics(timestampFont()).height() + Gap
        + TextLines * QFontMetrics(option.font).height();

    return QSize(option.rect.width(), height);
}
