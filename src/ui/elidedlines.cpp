#include "ui/elidedlines.h"

#include <QFont>
#include <QFontMetrics>
#include <QTextLayout>
#include <QTextLine>

QStringList library::elidedLines(const QString &text, const QFont &font, int width, int maxLines)
{
    const QString source = text.simplified();
    if (source.isEmpty() || width <= 0 || maxLines <= 0) {
        return {};
    }

    QStringList lines;

    QTextLayout layout(source, font);
    layout.beginLayout();
    for (int index = 0; index < maxLines; ++index) {
        QTextLine line = layout.createLine();
        if (!line.isValid()) {
            break;
        }
        line.setLineWidth(width);

        if (index == maxLines - 1) {
            // Everything still unplaced has to fit into this line or be cut:
            // eliding the remainder rather than the line itself is what puts
            // the ellipsis at the end of the text instead of at a word break.
            lines.append(QFontMetrics(font).elidedText(source.mid(line.textStart()), Qt::ElideRight, width));
        } else {
            lines.append(source.mid(line.textStart(), line.textLength()).trimmed());
        }
    }
    layout.endLayout();

    return lines;
}
