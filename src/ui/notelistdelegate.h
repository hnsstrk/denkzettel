#pragma once

#include <QStyledItemDelegate>

/**
 * Draws the rows of the library list (wireframe 3a): a group head, or a note
 * as a small dimmed timestamp above its subject and preview.
 *
 * A head carries no line, no background and no selection — every one of those
 * would weaken the selection mark of the notes below it.
 */
class NoteListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit NoteListDelegate(QObject *parent = nullptr);

    /**
     * Left edge of every text of the list: the timestamp, subject and preview
     * of a note and the text of a group head all start here (wireframe 3a,
     * 12 px from the left edge of the list).
     *
     * Both painting paths ask this function rather than each writing the
     * number down — which is what makes the alignment testable at all.
     */
    static int textLeft(const QRect &row);

    /** Room the text of `row` has, once both margins are taken off. */
    static int textWidth(const QRect &row);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    /**
     * Draws one line of text, elided, `top` pixels below the upper edge of
     * `row`.
     *
     * Head, timestamp, subject and preview all go through here, so the left
     * edge of the text exists exactly once in this class: no second place can
     * drift away from it, and the alignment the wireframe asks for cannot be
     * broken by writing the number down twice.
     */
    static void drawLine(QPainter *painter,
                         const QRect &row,
                         int top,
                         const QFont &font,
                         const QColor &color,
                         const QString &text);
};
