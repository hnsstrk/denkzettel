/**
 * What the search mark of the note list covers, in numbers (issue #122).
 *
 * The mark is drawn between two `QTextLine::cursorToX()` values, and the
 * question this runner was written for is whether that reaches past the matched
 * word: a cursor sits at the pen position, which is the last glyph's origin plus
 * its **advance**, while the letter's ink ends at its bounding box. The two
 * differ by the side bearings, and by whatever the shaper kerns into the pair
 * straddling the mark's end — which is why the following character is the
 * variable here and every case names one, right down to the line end, where
 * there is none.
 *
 * Four numbers per case, and the first two are the ones the answer is read off:
 *
 * 1. the mark as `notelistdelegate.cpp` computes it,
 * 2. the columns the matched letters really inked — the matched stretch drawn a
 *    second time on its own, at the very x the full line places it at. **This
 *    is what „past the word" is measured against**, because it is what an eye
 *    can see.
 * 3. the design box of those glyphs, per glyph out of `QRawFont`. It is the
 *    wider of the two by nearly a device pixel and printed only so the
 *    difference is on the record — a clearance read off it flips sign against
 *    the drawn pixels for „Backup".
 * 4. what Qt puts under a text selection of the same range
 *    (`QTextLayout::draw()` with a `FormatRange`), read out of a rendered
 *    image.
 *
 * **Number 4 is not an independent measurement and must not be reported as
 * one** (CLAUDE.md, finding 10, which it does *not* satisfy): Qt paints the
 * very same `cursorToX()` rectangle, expanded to whole logical pixels —
 * `round(floor(left / ratio) * ratio) .. round(ceil(right / ratio) * ratio)`
 * predicts all five geometries this runner produces exactly. What it answers is
 * the question of convention, and that is the one that decides here: the mark
 * takes the rectangle every other highlight in this toolkit takes, so „unusual"
 * is not a fault. It cannot answer whether that rectangle is right, because it
 * is computed the same way.
 *
 * No `add_test()`, for the reason `levelshots` has none: it produces numbers a
 * design decision was taken on, and a runner nobody can build again leaves them
 * unrepeatable.
 *
 * ```sh
 * cmake --build build --target markshots
 * env QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_SCALE_FACTOR=1.5 \
 *     QT_FORCE_STDERR_LOGGING=1 build/bin/markshots docs/images/reviews
 * ```
 *
 * `QT_QPA_PLATFORMTHEME=kde` decides the style and with it every metric
 * (finding 28), `QT_SCALE_FACTOR` is the user's scaling and the unit every
 * number below is given in, and `QT_FORCE_STDERR_LOGGING=1` is what makes the
 * run speak at all once its output is redirected (finding 25). The style, the
 * font and the scaling are printed back, because setting a variable is not the
 * same as it having taken.
 */

#include "platform/systemfonts.h"
#include "store/note.h"
#include "ui/notelistdelegate.h"
#include "ui/notelistmodel.h"
#include "ui/searchmarks.h"

#include <KColorScheme>
#include <KLocalizedString>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontMetricsF>
#include <QGlyphRun>
#include <QImage>
#include <QListView>
#include <QPainter>
#include <QRawFont>
#include <QStyle>
#include <QTemporaryDir>
#include <QTest>
#include <QTextLayout>
#include <QTextLine>
#include <QWidget>

namespace
{
/** Room around the drawn line, so nothing the measurement wants falls off. */
constexpr int Margin = 20;
constexpr int BoxWidth = 600;

struct Case {
    const char *name;
    QString text;
    QString term;
};

/** One text laid out in one line, the way `drawMarked()` lays it out. */
struct Laid {
    QTextLayout layout;
    QTextLine line;

    Laid(const QString &text, const QFont &font, QPaintDevice *device)
        : layout(text, font, device)
    {
        layout.beginLayout();
        line = layout.createLine();
        line.setLineWidth(BoxWidth);
        layout.endLayout();
    }
};

/**
 * The ink of the glyphs behind `from`..`from + length`, in layout coordinates.
 *
 * Per glyph out of `QRawFont::boundingRect()` rather than out of
 * `QGlyphRun::boundingRect()`, which is free to answer with the font's box
 * instead of the letter's — and the difference between advance and ink is
 * exactly what is being measured.
 */
QRectF inkOf(QTextLayout &layout, qsizetype from, qsizetype length)
{
    QRectF ink;
    bool started = false;

    const QList<QGlyphRun> runs = layout.glyphRuns(static_cast<int>(from), static_cast<int>(length));
    for (const QGlyphRun &run : runs) {
        const QRawFont font = run.rawFont();
        const QList<quint32> glyphs = run.glyphIndexes();
        const QList<QPointF> places = run.positions();

        for (qsizetype glyph = 0; glyph < glyphs.size(); ++glyph) {
            QRectF box = font.boundingRect(glyphs.at(glyph));
            box.translate(places.at(glyph));

            ink = started ? ink.united(box) : box;
            started = true;
        }
    }

    return ink;
}

/** A white image of the size one line needs, at the scaling handed in. */
QImage sheet(qreal ratio, int height)
{
    QImage image(qRound((BoxWidth + 2 * Margin) * ratio), qRound((height + 2 * Margin) * ratio),
                 QImage::Format_RGB32);
    image.setDevicePixelRatio(ratio);
    image.fill(Qt::white);

    return image;
}

/** The device pixel columns of `image` on which `hit` answers for any row. */
template<typename Hit>
QPair<int, int> columns(const QImage &image, Hit hit)
{
    int left = -1;
    int right = -1;

    for (int x = 0; x < image.width(); ++x) {
        for (int y = 0; y < image.height(); ++y) {
            if (!hit(image.pixelColor(x, y), x, y)) {
                continue;
            }

            if (left < 0) {
                left = x;
            }
            right = x;
            break;
        }
    }

    return {left, right};
}

void measure(const Case &example, const QFont &font, qreal ratio)
{
    const QList<library::SearchMark> marks = library::searchMarks(example.text, {example.term});
    if (marks.size() != 1) {
        qFatal("%s: the term has to match exactly once, it matched %lld times", example.name,
               static_cast<long long>(marks.size()));
    }
    const library::SearchMark mark = marks.first();
    const int height = qCeil(QFontMetricsF(font).height());

    // A — the line as the list draws it, and the two numbers the delegate takes.
    QImage plain = sheet(ratio, height);
    QPainter first(&plain);
    Laid laid(example.text, font, first.device());

    const qreal from = laid.line.cursorToX(static_cast<int>(mark.start));
    const qreal to = laid.line.cursorToX(static_cast<int>(mark.start + mark.length));
    const QRectF ink = inkOf(laid.layout, mark.start, mark.length);

    first.setPen(Qt::black);
    laid.layout.draw(&first, QPointF(Margin, Margin));
    first.end();

    // B — the same line with Qt's own selection ground under the same range.
    QImage selected = sheet(ratio, height);
    QPainter second(&selected);
    const Laid again(example.text, font, second.device());

    QTextCharFormat ground;
    ground.setBackground(QColor(255, 0, 0));
    QTextLayout::FormatRange range;
    range.start = static_cast<int>(mark.start);
    range.length = static_cast<int>(mark.length);
    range.format = ground;

    second.setPen(Qt::black);
    again.layout.draw(&second, QPointF(Margin, Margin), {range});
    second.end();

    // C — the matched letters alone, drawn at the very x the full line puts
    // them at, so their inked columns can be held against the analytic ink.
    QImage alone = sheet(ratio, height);
    QPainter third(&alone);
    const Laid only(example.text.mid(mark.start, mark.length), font, third.device());
    third.setPen(Qt::black);
    only.layout.draw(&third, QPointF(Margin + from, Margin));
    third.end();

    // What Qt's selection covers, bounded from both sides: the outer bound is
    // every column that differs from the unselected line at all, the inner one
    // every column carrying untouched red. The true edge lies between them —
    // a fractional edge blends into its column and neither bound alone is it.
    const QPair<int, int> outer = columns(selected, [&plain](const QColor &pixel, int x, int y) {
        return pixel != plain.pixelColor(x, y);
    });
    const QPair<int, int> inner = columns(selected, [](const QColor &pixel, int, int) {
        return pixel.red() == 255 && pixel.green() == 0 && pixel.blue() == 0;
    });
    const QPair<int, int> drawn = columns(alone, [](const QColor &pixel, int, int) {
        return pixel.value() < 250;
    });

    if (outer.first < 0 || inner.first < 0 || drawn.first < 0) {
        qFatal("%s: selection or type inked nothing at all", example.name);
    }

    // Everything in device pixels, the unit the acceptance criterion asks for.
    // The box starts at `Margin`, so the mark runs from `Margin + from` to
    // `Margin + to`.
    const qreal markLeft = (Margin + from) * ratio;
    const qreal markRight = (Margin + to) * ratio;

    // Two answers to „where does the word end", and they are **not** the same:
    //
    // - the design box, `QRawFont::boundingRect()` per glyph — what the type
    //   designer reserved for the letter,
    // - the columns the rasteriser really inked, which is what an eye can see.
    //
    // The box is the wider of the two, and by nearly a whole device pixel: for
    // „Backup" it ends at 136.92 while the last inked column ends at 136.00, so
    // a clearance read off the box says the mark stops 0.02 px short of the
    // letter while the drawn pixels say it reaches 0.90 px past it. **The sign
    // flips.** The drawn value is therefore the one this runner leads with, and
    // the divergence is printed beside it rather than left for a reader to work
    // out — printing both without comparing them is how the first reading of
    // this measurement came out wrong (review of 30.08.2026).
    //
    // Positive means the mark reaches past the letter, negative that it stops
    // inside it.
    const qreal drawnLeft = drawn.first;
    const qreal drawnRight = drawn.second + 1;
    const qreal boxLeft = (Margin + ink.left()) * ratio;
    const qreal boxRight = (Margin + ink.right()) * ratio;

    const bool atEnd = mark.start + mark.length >= example.text.size();
    const QChar next = atEnd ? QChar(u'—') : example.text.at(mark.start + mark.length);

    qInfo().noquote()
        << QStringLiteral("%1").arg(QString::fromUtf8(example.name), -16)
            + QStringLiteral("next %1  mark %2..%3  drawn ink %4..%5  over L %6 R %7")
                  .arg(next)
                  .arg(markLeft, 7, 'f', 2)
                  .arg(markRight, 7, 'f', 2)
                  .arg(drawnLeft, 7, 'f', 2)
                  .arg(drawnRight, 7, 'f', 2)
                  .arg(drawnLeft - markLeft, 5, 'f', 2)
                  .arg(markRight - drawnRight, 5, 'f', 2);

    qInfo().noquote()
        << QStringLiteral("%1").arg(QString(), -16)
            + QStringLiteral("        design box %1..%2 (wider than the drawn ink by L %3 R %4)  "
                             "Qt selection %5..%6 (inner %7..%8)")
                  .arg(boxLeft, 7, 'f', 2)
                  .arg(boxRight, 7, 'f', 2)
                  .arg(drawnLeft - boxLeft, 5, 'f', 2)
                  .arg(boxRight - drawnRight, 5, 'f', 2)
                  .arg(outer.first)
                  .arg(outer.second + 1)
                  .arg(inner.first)
                  .arg(inner.second + 1);
}

/**
 * Whether the following character can move the mark's right edge at all.
 *
 * Without this the run above proves nothing by coming out equal for every
 * following character: a font that kerns nowhere answers the same for `i` and
 * for `W` whatever `cursorToX()` does, and the case the acceptance criterion is
 * built on could not occur (CLAUDE.md, verification stance). Two readings
 * therefore, and both are needed — the pairs the cases above really use, `t` and
 * `p` before each follower, which is their direct control; and the same for the
 * capitals `T`, `A` and `V`, which say the mechanism is live at this size and
 * that `cursorToX()` carries it.
 */
void kerning(const QFont &font, qreal ratio)
{
    QImage sheet(10, 10, QImage::Format_RGB32);
    sheet.setDevicePixelRatio(ratio);
    QPainter painter(&sheet);

    const auto advance = [&font, &painter, ratio](const QString &pair) {
        const Laid laid(pair, font, painter.device());
        return laid.line.cursorToX(1) * ratio;
    };

    for (const QString &first : {QStringLiteral("t"), QStringLiteral("p"), QStringLiteral("T"),
                                 QStringLiteral("A"), QStringLiteral("V")}) {
        QString reading;
        for (const QString &next : {QStringLiteral("i"), QStringLiteral("l"), QStringLiteral(","),
                                    QStringLiteral("m"), QStringLiteral("W"), QStringLiteral(" "),
                                    QStringLiteral("a"), QStringLiteral("x")}) {
            reading += QStringLiteral("%1 %2  ").arg(next).arg(advance(first + next), 0, 'f', 2);
        }
        qInfo().noquote() << QStringLiteral("advance of '%1' before  ").arg(first) + reading;
    }
}

/**
 * The mark as the list really paints it — the delegate, in a `QListView`, at
 * the user's scaling.
 *
 * The numbers above are taken off a `QTextLayout` of the runner's own, and the
 * delegate writes its type with `QPainter::drawText()` beside that layout
 * (issue #77, finding 39). Whether the two place the glyphs alike is not
 * something either of them can answer about itself, so the answer is read off
 * the painted row: where the mark's ground stands, and where the type on it
 * stands.
 *
 * Two words, and the second is not decoration: the mark is the whole line box,
 * so how much coloured ground stands above and below the type depends on
 * whether the hit carries a descender. „Backup" has one and „Gedanke" — the
 * word of the pictures the issue was written from — has not, and the excess
 * comes out 7 px against 12 px of the same 27 px band. One word alone reports
 * the favourable half of that range as if it were the number.
 */
void row(const Case &example, const QString &directory, bool picture)
{
    NoteListModel model;
    Note note;
    note.id = 1;
    note.createdAt = QDateTime(QDate(2026, 8, 30), QTime(9, 15));
    // One match in the note, not two: the reading below takes the outermost
    // marked columns, and a second mark on the preview line would come back as
    // one very wide mark.
    note.content = example.text;
    model.setNotes({note}, note.createdAt);

    NoteListDelegate delegate;
    delegate.setSearchTerms({example.term});

    QListView view;
    view.setModel(&model);
    view.setItemDelegate(&delegate);
    view.setFrameShape(QFrame::NoFrame);
    view.resize(380, 200);
    view.show();
    if (!QTest::qWaitForWindowExposed(&view)) {
        qFatal("the list never reached the screen");
    }

    const QImage painted = view.grab().toImage();
    const QColor ground = KColorScheme(QPalette::Normal, KColorScheme::View)
                              .background(KColorScheme::NeutralBackground)
                              .color();
    const QColor base = QApplication::palette().color(QPalette::Normal, QPalette::Base);

    // The mark is the only warm area in the row: every column that is neither
    // the list's own ground nor a shade of grey belongs to it. Read by hue, so
    // that the deepening of issue #116 does not have to be recomputed here —
    // and the note carries exactly one match, or two marks at two different x
    // would come back as one very wide one.
    const auto marked = [](const QColor &pixel) {
        return pixel.red() > pixel.blue() + 6 && pixel.red() > 200;
    };

    const QPair<int, int> markColumns = columns(painted, [&marked](const QColor &pixel, int, int) {
        return marked(pixel);
    });
    if (markColumns.first < 0) {
        qFatal("no mark was painted at all");
    }

    // The rows the mark occupies, so the type is only looked for on it.
    int top = -1;
    int bottom = -1;
    for (int y = 0; y < painted.height(); ++y) {
        if (marked(painted.pixelColor(markColumns.first, y))) {
            if (top < 0) {
                top = y;
            }
            bottom = y;
        }
    }

    // The type **on the mark**, so the rest of the line cannot answer for it:
    // "Das " stands before the match and "medium ist alt" after it, both on the
    // same rows and both dark. Bounded from two sides like the selection above,
    // because the outermost column of a letter is a pale antialiased fringe and
    // a single threshold decides by itself how much of it counts as type.
    const auto type = [&](int threshold) {
        return columns(painted, [&markColumns, top, bottom, threshold](const QColor &pixel, int x,
                                                                       int y) {
            return x >= markColumns.first && x <= markColumns.second && y >= top && y <= bottom
                && pixel.value() < threshold;
        });
    };
    const QPair<int, int> solid = type(140);
    const QPair<int, int> faint = type(250);

    // The colour is read off the painted row, not off `KColorScheme`: what the
    // mark ends up in is the deepening of issue #116 or the plain neutral,
    // whichever passed the contrast threshold, and only the pixel says which.
    const QColor painted_ground =
        painted.pixelColor((markColumns.first + markColumns.second) / 2, top);

    // The mark's height beside its width. The width is what the issue asks
    // about; the height is what the picture shows, because the mark is the whole
    // line box and the letters are not.
    int inkTop = -1;
    int inkBottom = -1;
    for (int y = top; y <= bottom; ++y) {
        for (int x = markColumns.first; x <= markColumns.second; ++x) {
            if (painted.pixelColor(x, y).value() < 140) {
                if (inkTop < 0) {
                    inkTop = y;
                }
                inkBottom = y;
                break;
            }
        }
    }

    qInfo().noquote() << QStringLiteral(
                             "painted row: mark %1..%2 (y %3..%4, %5 on %6, neutral %7)  type on it "
                             "%8..%9 solid, %10..%11 with the fringe  clear L %12..%13 R %14..%15")
                             .arg(markColumns.first)
                             .arg(markColumns.second + 1)
                             .arg(top)
                             .arg(bottom + 1)
                             .arg(painted_ground.name(), base.name(), ground.name())
                             .arg(solid.first)
                             .arg(solid.second + 1)
                             .arg(faint.first)
                             .arg(faint.second + 1)
                             .arg(faint.first - markColumns.first)
                             .arg(solid.first - markColumns.first)
                             .arg(markColumns.second - solid.second)
                             .arg(markColumns.second - faint.second);

    qInfo().noquote() << QStringLiteral(
                             "             '%1': mark %2 px wide and %3 px tall; the type on it "
                             "inks %4 px of that height (y %5..%6), leaving %7 px of coloured "
                             "ground above and below it")
                             .arg(example.term)
                             .arg(markColumns.second - markColumns.first + 1)
                             .arg(bottom - top + 1)
                             .arg(inkBottom - inkTop + 1)
                             .arg(inkTop)
                             .arg(inkBottom + 1)
                             .arg((bottom - top + 1) - (inkBottom - inkTop + 1));

    if (!picture) {
        return;
    }

    // The picture, magnified so the edge under discussion is a thing the eye
    // can hold against the numbers. Nearest neighbour: an interpolated
    // enlargement invents an edge where the measurement says there is none.
    const int pad = 12;
    const QImage crop = painted.copy(markColumns.first - pad, top - pad,
                                     markColumns.second - markColumns.first + 1 + 2 * pad,
                                     bottom - top + 1 + 2 * pad);
    const QString file = directory + QStringLiteral("/122-suchmarke-kanten.png");
    if (!crop.scaled(crop.width() * 6, crop.height() * 6, Qt::IgnoreAspectRatio, Qt::FastTransformation)
             .save(file)) {
        qFatal("the picture could not be written to %s", qUtf8Printable(file));
    }
    qInfo("written: %s (%dx%d px, magnified 6x)", qUtf8Printable(file), crop.width(), crop.height());
}
}

int main(int argc, char **argv)
{
    // As in the other runners: no stored configuration is to reach the numbers.
    // A colour scheme of the run's own goes in before QApplication all the same,
    // or `KColorScheme` would answer with its built-in light defaults while the
    // palette says something else, and the mark would be measured on two
    // sources at once (CLAUDE.md, finding 38).
    const QTemporaryDir configuration;
    qputenv("XDG_CONFIG_HOME", configuration.path().toLocal8Bit());
    if (!QFile::copy(QStringLiteral("/usr/share/color-schemes/BreezeLight.colors"),
                     configuration.filePath(QStringLiteral("kdeglobals")))) {
        qFatal("the colour scheme could not be laid into the throwaway configuration");
    }

    const QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
    // Without it the group head of the list comes out in the source language
    // whatever LANGUAGE says — the fault `originshots` carries and `readmeshots`
    // does not.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    // The scaling is read back, not assumed: QT_SCALE_FACTOR can be set and the
    // run still measure at 1.0, the way the style name is read back in the other
    // runners (CLAUDE.md, finding 28).
    QWidget probe;
    probe.resize(10, 10);
    probe.show();
    const qreal ratio = probe.devicePixelRatioF();

    const QFont font = platform::generalFont();
    qInfo("style: %s · font: %s %g pt · scaling: %g",
          qUtf8Printable(QApplication::style()->objectName()),
          qUtf8Printable(font.family()),
          font.pointSizeF(),
          ratio);

    // Two matched words, so that neither the letter before the boundary nor the
    // one after it is held fixed across the whole run — and the following
    // character stands **against** the word, not a space away from it, or every
    // case would measure the same pair.
    const QList<Case> examples = {
        {"Test + i", QStringLiteral("Notiz Testinhalt offen"), QStringLiteral("Test")},
        {"Test + l", QStringLiteral("Notiz Testlauf offen"), QStringLiteral("Test")},
        {"Test + comma", QStringLiteral("Notiz Test, dann weiter"), QStringLiteral("Test")},
        {"Test + m", QStringLiteral("Notiz Testmappe offen"), QStringLiteral("Test")},
        {"Test + W", QStringLiteral("Notiz TestWerk offen"), QStringLiteral("Test")},
        {"Test + space", QStringLiteral("Notiz Test offen"), QStringLiteral("Test")},
        {"Test + end", QStringLiteral("Notiz Test"), QStringLiteral("Test")},
        {"Backup + i", QStringLiteral("Das Backupintervall ist lang"), QStringLiteral("Backup")},
        {"Backup + l", QStringLiteral("Das Backuplaufwerk ist alt"), QStringLiteral("Backup")},
        {"Backup + comma", QStringLiteral("Das Backup, dann Ruhe"), QStringLiteral("Backup")},
        {"Backup + m", QStringLiteral("Das Backupmedium ist alt"), QStringLiteral("Backup")},
        {"Backup + W", QStringLiteral("Das BackupWerk ist alt"), QStringLiteral("Backup")},
        {"Backup + space", QStringLiteral("Das Backup ist alt"), QStringLiteral("Backup")},
        {"Backup + end", QStringLiteral("Das Backup"), QStringLiteral("Backup")},
        // A third word, ending in the one letter of the three the font really
        // kerns after — see the control below. Without it the run would report
        // „the following character changes nothing" from cases in which it
        // could not have changed anything.
        {"PROJEKT + i", QStringLiteral("Notiz PROJEKTinfo offen"), QStringLiteral("PROJEKT")},
        {"PROJEKT + a", QStringLiteral("Notiz PROJEKTakte offen"), QStringLiteral("PROJEKT")},
        {"PROJEKT + comma", QStringLiteral("Notiz PROJEKT, dann Ruhe"), QStringLiteral("PROJEKT")},
        {"PROJEKT + space", QStringLiteral("Notiz PROJEKT offen"), QStringLiteral("PROJEKT")},
        {"PROJEKT + end", QStringLiteral("Notiz PROJEKT"), QStringLiteral("PROJEKT")},
        // The word of the pictures the issue was written from
        // (`docs/images/reviews/116-suchmarke-*.png`), so the numbers here and
        // the numbers read off those files answer for the same case.
        {"Gedanke + space", QStringLiteral("Ein Gedanke zum Morgen"), QStringLiteral("Gedanke")},
        {"Gedanke + comma", QStringLiteral("Ein Gedanke, der bleibt"), QStringLiteral("Gedanke")},
        {"Gedanke + end", QStringLiteral("Zweiter Gedanke"), QStringLiteral("Gedanke")},
    };

    for (const Case &example : examples) {
        measure(example, font, ratio);
    }

    kerning(font, ratio);

    const QString directory = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral(".");
    QDir().mkpath(directory);
    // With a descender in the hit and without one — the coloured ground above
    // and below the type is 7 px in the first case and 12 in the second, of the
    // same 27 px band. The picture is taken of the first; the second is the word
    // of `docs/images/reviews/116-suchmarke-*.png`, which is where the height
    // was read as „reaching past the word" in the first place.
    row({"with a descender", QStringLiteral("Das Backupmedium ist alt\nnoch nichts entschieden"),
         QStringLiteral("Backup")},
        directory, true);
    row({"without one", QStringLiteral("Ein Gedanke zum Morgen\nnoch nichts entschieden"),
         QStringLiteral("Gedanke")},
        directory, false);

    return 0;
}
