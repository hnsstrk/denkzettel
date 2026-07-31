#pragma once

#include <QStyledItemDelegate>

/**
 * Draws a note as the library lists it (wireframe 2b): a small dimmed
 * timestamp above at most two lines of note text, elided at the end.
 */
class NoteListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit NoteListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
