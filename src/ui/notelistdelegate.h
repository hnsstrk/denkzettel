#pragma once

#include <QFont>
#include <QStyledItemDelegate>

/**
 * Draws the rows of the library list (wireframe 3a): a group head, or a note
 * as a small dimmed timestamp above its subject and preview.
 *
 * Two hairlines of one colour separate them, and where they lie carries the
 * ranking (issue #104): between two notes of one group an inset one on the
 * text edge of the row boundary, beside the label of every head one that runs
 * out to the right text edge. The head line is no separator at an edge — it
 * belongs to the heading, whose bold type in the size of the application is the
 * second feature of a group boundary.
 *
 * No entry line at an edge of the selected row — a separator there would
 * compete with the selection mark, and so would a background or a selectable
 * head, which is why a head has neither.
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

    /**
     * The type a group head is set in — the text size of the application, bold,
     * in plain text colour (issue #104).
     *
     * It was the smallest type of the list until 11.08.2026, the size of the
     * timestamp and therefore smaller than the note text it stands over: a
     * heading without rank. The rank is one of the two features that tell a
     * group boundary from a note boundary now.
     *
     * Public for the same reason as textLeft(): it is a decision about rank,
     * and where a delegate puts its ink is invisible to a test that does not
     * count pixels. Asked here, it can be measured against the sizes the
     * application itself uses.
     */
    static QFont groupHeadFont();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    /** Room the text of `row` has, once both margins are taken off. */
    static int textWidth(const QRect &row);

    /**
     * Draws one line of text, elided, `top` pixels below the upper edge of
     * `row`, and hands back how wide the text came out.
     *
     * Head, timestamp, subject and preview all go through here, so the left
     * edge of the text exists exactly once in this class: no second place can
     * drift away from it, and the alignment the wireframe asks for cannot be
     * broken by writing the number down twice. The width is what the head line
     * of issue #104 begins after.
     */
    static int drawLine(QPainter *painter,
                        const QRect &row,
                        int top,
                        const QFont &font,
                        const QColor &color,
                        const QString &text);
};
