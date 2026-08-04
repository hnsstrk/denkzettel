// Der Zustand "getippt" einmal so gegriffen wie der Bildlaeufer (sofort nach
// show()) und einmal, nachdem die Ereignisschleife einmal durchgelaufen ist.
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"
#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

namespace
{
void shoot(QWidget &window, const QString &directory, const QString &name)
{
    constexpr int Frame = 24;
    constexpr int HatchStep = 8;
    const QPixmap grabbed = window.grab();
    QImage picture(grabbed.size() + QSize(2 * Frame, 2 * Frame), QImage::Format_ARGB32);
    QPainter painter(&picture);
    picture.fill(QColor(0xf2, 0xf0, 0xeb));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0xe9, 0xe7, 0xe2));
    for (int offset = -picture.height(); offset < picture.width(); offset += 2 * HatchStep) {
        painter.drawPolygon(QPolygon({QPoint(offset, 0), QPoint(offset + HatchStep, 0),
                                      QPoint(offset + HatchStep + picture.height(), picture.height()),
                                      QPoint(offset + picture.height(), picture.height())}));
    }
    painter.drawPixmap(Frame, Frame, grabbed);
    picture.save(QDir(directory).filePath(name));
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QString directory = app.arguments().value(1, QStringLiteral("."));
    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    store.open();
    themes::addBundledThemesToDataPath();
    const auto pair = themes::installedThemePair();

    QPalette p;
    p.setColor(QPalette::Window, QColor(0xef, 0xf0, 0xf1));
    p.setColor(QPalette::WindowText, QColor(0x23, 0x26, 0x29));
    p.setColor(QPalette::Base, QColor(0xff, 0xff, 0xff));
    p.setColor(QPalette::Text, QColor(0x23, 0x26, 0x29));
    p.setColor(QPalette::PlaceholderText, QColor(0x70, 0x7d, 0x8a));
    app.setPalette(p);

    CaptureWindow w(&store);
    w.reloadDesktopTheme(pair->first);
    auto *t = w.findChild<QPlainTextEdit *>();
    t->setPlainText(QStringLiteral("Denkzettel soll die Hülle des Desktop-Themes tragen"));
    w.show();
    QTest::qWait(150);
    shoot(w, directory, QStringLiteral("20-getippt-nach-ereignisschleife.png"));
    QTextStream(stdout) << "20 geschrieben (nach 150 ms Ereignisschleife)\n";
    return 0;
}
