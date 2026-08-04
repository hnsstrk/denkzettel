// #56 in beide Richtungen: Schrift groesser UND wieder kleiner, am gezeigten
// und am verdeckten Fenster.
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

static void lauf(QTextStream &out, bool zeigen, Store &store, const QString &theme)
{
    CaptureWindow w(&store);
    w.reloadDesktopTheme(theme);
    if (zeigen) {
        w.show();
        QTest::qWait(100);
    }
    auto *t = w.findChild<QPlainTextEdit *>();
    out << (zeigen ? "  gezeigt : " : "  verdeckt: ");
    for (int pt : {9, 24, 9}) {
        QFont f = t->font();
        f.setPointSize(pt);
        t->setFont(f);
        QTest::qWait(20);
        const int chrome = 2 * qRound(t->document()->documentMargin()) + 2 * t->frameWidth();
        out << pt << "pt -> Feld " << t->height() << " (" 
            << qreal(t->height() - chrome) / t->fontMetrics().lineSpacing() << " Zeilen), Fenster "
            << w.height() << "   ";
    }
    out << "\n";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    store.open();
    themes::addBundledThemesToDataPath();
    const auto pair = themes::installedThemePair();
    out << "Schriftfolge 9pt -> 24pt -> 9pt\n";
    lauf(out, false, store, pair->first);
    lauf(out, true, store, pair->first);
    return 0;
}
