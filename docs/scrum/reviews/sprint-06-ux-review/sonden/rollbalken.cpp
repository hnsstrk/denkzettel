// Was ist der graue Balken am unteren Rand des Textfelds im Zustand "getippt"?
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"
#include <QApplication>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    store.open();
    themes::addBundledThemesToDataPath();
    const auto pair = themes::installedThemePair();

    CaptureWindow w(&store);
    w.reloadDesktopTheme(pair->first);
    auto *t = w.findChild<QPlainTextEdit *>();
    w.show();

    const QStringList states{QString(),
                             QStringLiteral("Denkzettel soll die Hülle des Desktop-Themes tragen"),
                             QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht\nneun\nzehn\nelf\nzwölf")};
    for (const QString &s : states) {
        t->setPlainText(s);
        QCoreApplication::processEvents();
        out << "Text \"" << s.left(18) << "\"\n";
        out << "   Zeilenumbruchmodus       : " << int(t->lineWrapMode()) << " (WidgetWidth=1)\n";
        out << "   waagerechte Politik      : " << int(t->horizontalScrollBarPolicy())
            << " (AsNeeded=0, AlwaysOff=1)\n";
        out << "   waagerechter Rollbalken  : sichtbar=" << t->horizontalScrollBar()->isVisible()
            << "  Bereich " << t->horizontalScrollBar()->minimum() << ".."
            << t->horizontalScrollBar()->maximum()
            << "  Hoehe " << t->horizontalScrollBar()->height() << "\n";
        out << "   senkrechter Rollbalken   : sichtbar=" << t->verticalScrollBar()->isVisible()
            << "  Bereich " << t->verticalScrollBar()->minimum() << ".."
            << t->verticalScrollBar()->maximum() << "\n";
        out << "   Feldhoehe " << t->height() << "  Sichtfensterhoehe " << t->viewport()->height()
            << "  Dokumenthoehe(Zeilen) " << t->document()->size().height() << "\n";
    }
    return 0;
}
