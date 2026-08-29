#pragma once

#include <QSize>
#include <QString>
#include <QStringList>
#include <QWidget>

/**
 * The row of pills the reading pane carries under a note: its category first,
 * then its AI tags (SPEC 9, wireframes 2b and 3a, UX decision 2026-08-29 on
 * issue #18).
 *
 * **The two are told apart by their filling, not by their shape** — the
 * category is a filled pill without an outline, a tag an outlined one, and both
 * in the one colour the list draws its separator lines in. One value out of a
 * fixed list thereby looks different from several free ones without a second
 * shape and without a second colour coming in.
 *
 * **Without a category and without tags the row hides itself** — no
 * placeholder, no reserved height. Until the analysis run of M3 has been
 * through the library that is the normal case, and a „no category yet" under
 * every note would be the loudest text of the window.
 *
 * The edit state does not use this: there category and tags are fields in the
 * label row of wireframe 2a, and they become editable later. Two states, two
 * appearances — that is the difference between showing and entering.
 *
 * Drawn rather than built out of child widgets: a pill is a rounded rectangle
 * with a word in it, and the colours have to be read off the palette at every
 * paint, because the window is built once at daemon start and has to follow a
 * colour scheme changed underneath it (issue #54).
 */
class NoteChips : public QWidget
{
public:
    explicit NoteChips(QWidget *parent = nullptr);

    /**
     * Sets what the row shows and hides it when there is nothing.
     *
     * `category` is the readable label, not the short form the database keeps
     * (SPEC 7.2) — the mapping is the caller's.
     */
    void setChips(const QString &category, const QStringList &tags);

    /** True while the note has neither a category nor a tag. */
    bool isEmpty() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    QString m_category;
    QStringList m_tags;
};
