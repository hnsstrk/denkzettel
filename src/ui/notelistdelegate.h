#pragma once

#include <QStringList>
#include <QStyledItemDelegate>

/**
 * Draws the rows of the library list (wireframe 3a): a group head, or a note
 * as a small dimmed timestamp above its subject and preview. A voice note adds
 * the play symbol and its length opposite the timestamp, a text note nothing.
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
     * The terms of the running search — what a hit is marked by (issue #77).
     *
     * `SearchQuery::terms`, not the raw text of the search field: `tag:` and
     * the other operators pick the notes but stand in none of them, and a mark
     * on their value would point at a word the user never searched for.
     *
     * Empty while nothing is being searched for, and then the list looks as it
     * always did.
     */
    void setSearchTerms(const QStringList &terms);

    /**
     * Left edge of every text of the list: the timestamp, subject and preview
     * of a note and the text of a group head all start here (wireframe 3a,
     * 12 px from the left edge of the list).
     *
     * Both painting paths ask this function rather than each writing the
     * number down — which is what makes the alignment testable at all.
     */
    static int textLeft(const QRect &row);

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
     *
     * `terms` are marked wherever they stand in the text that is actually
     * drawn — after the eliding, because a mark outside the line would be a
     * mark nobody sees (issue #77). The timestamp and the group head hand none
     * over: they are not what was searched in.
     */
    static int drawLine(QPainter *painter,
                        const QRect &row,
                        int top,
                        const QFont &font,
                        const QColor &color,
                        const QString &text,
                        const QStringList &terms = {});

    QStringList m_terms;
};
