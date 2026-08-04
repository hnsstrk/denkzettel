// UX-Prüfsonde Sprint 6 — der Weg des Nutzers: öffnen, acht Zeilen tippen,
// Esc, erneut öffnen. Prüft die Zusage aus Wireframe 4b „5 Zeilen beim Öffnen".
// Liest nur; ändert nichts am Produkt.
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

namespace
{
QPalette breezeLight()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(0xef, 0xf0, 0xf1));
    p.setColor(QPalette::WindowText, QColor(0x23, 0x26, 0x29));
    p.setColor(QPalette::Base, QColor(0xff, 0xff, 0xff));
    p.setColor(QPalette::Text, QColor(0x23, 0x26, 0x29));
    p.setColor(QPalette::PlaceholderText, QColor(0x70, 0x7d, 0x8a));
    p.setColor(QPalette::Highlight, QColor(0x3d, 0xae, 0xe9));
    return p;
}

// Derselbe Hintergrund wie in captureshots, damit die Bilder vergleichbar sind.
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
        painter.drawPolygon(QPolygon({QPoint(offset, 0),
                                      QPoint(offset + HatchStep, 0),
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
    QTextStream out(stdout);
    const QString directory = app.arguments().value(1, QStringLiteral("."));

    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    store.open();

    themes::addBundledThemesToDataPath();
    const auto pair = themes::installedThemePair();
    app.setPalette(breezeLight());

    CaptureWindow window(&store);
    window.reloadDesktopTheme(pair ? pair->first : QString());
    auto *text = window.findChild<QPlainTextEdit *>();

    window.showCapture();
    QTest::qWait(120);
    out << "1. erstes Oeffnen              : Fenster " << window.height() << " px\n";
    shoot(window, directory, QStringLiteral("16-erstes-oeffnen.png"));

    text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfuenf\nsechs\nsieben\nacht"));
    QTest::qWait(50);
    out << "2. acht Zeilen getippt         : Fenster " << window.height() << " px\n";

    // Esc, wie der Nutzer ihn druckt — verwirft und versteckt.
    QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(text, &escape);
    QTest::qWait(120);
    out << "3. nach Esc (versteckt)        : Fenster " << window.height()
        << " px, sichtbar=" << window.isVisible() << ", Text leer=" << text->toPlainText().isEmpty() << "\n";

    window.showCapture();
    QTest::qWait(200);
    out << "4. zweites Oeffnen, Feld leer  : Fenster " << window.height() << " px\n";
    shoot(window, directory, QStringLiteral("17-zweites-oeffnen-nach-acht-zeilen.png"));

    return 0;
}
