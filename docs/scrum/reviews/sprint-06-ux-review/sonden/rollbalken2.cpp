// Genau der Weg des Bildlaeufers: neues Fenster, Text, show(), grab().
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

static void zeig(QTextStream &out, const QString &wann, QPlainTextEdit *t, QWidget *w)
{
    out << "   " << wann << ": waagerecht sichtbar=" << t->horizontalScrollBar()->isVisible()
        << " max=" << t->horizontalScrollBar()->maximum()
        << " | senkrecht sichtbar=" << t->verticalScrollBar()->isVisible()
        << " | Feld " << t->height() << " Sichtfenster " << t->viewport()->height()
        << " | Fenster " << w->height() << "\n";
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

    for (int state = 0; state < 3; ++state) {
        out << "Zustand " << state << " (0 leer, 1 getippt, 2 zwoelf Zeilen)\n";
        CaptureWindow w(&store);
        w.reloadDesktopTheme(pair->first);
        auto *t = w.findChild<QPlainTextEdit *>();
        if (state == 1) {
            t->setPlainText(QStringLiteral("Denkzettel soll die Hülle des Desktop-Themes tragen"));
        } else if (state == 2) {
            t->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht\nneun\nzehn\nelf\nzwölf"));
        }
        w.show();
        zeig(out, "direkt nach show()  ", t, &w);
        QTest::qWait(150);
        zeig(out, "150 ms nach show()  ", t, &w);
    }
    return 0;
}
