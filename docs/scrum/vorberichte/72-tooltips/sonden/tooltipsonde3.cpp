// Vorprüfung #72 — Sonde 3: Lässt sich ein Tooltip offscreen als Bild belegen?

#include <QApplication>
#include <QPushButton>
#include <QTextStream>
#include <QToolTip>
#include <QWidget>

static QTextStream out(stdout);

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QPushButton pb(QStringLiteral("Bearbeiten"));
    pb.setToolTip(QStringLiteral("Bearbeiten (F2)"));
    pb.resize(160, 32);
    pb.show();
    app.processEvents();

    out << "Plattform: " << app.platformName() << "\n";
    out << "A) Zeigt QWidget::grab() des Fensters den Tooltip?\n";
    QToolTip::showText(pb.mapToGlobal(QPoint(20, 20)), pb.toolTip(), &pb);
    app.processEvents();
    const QImage fenster = pb.grab().toImage();
    out << "   Fensterbild " << fenster.width() << "x" << fenster.height()
        << " — der Tooltip ist ein eigenes Fenster, er kann darin nicht liegen.\n";

    out << "B) Gibt es das Tooltip-Fenster als QWidget?\n";
    QWidget *tip = nullptr;
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (QLatin1String(w->metaObject()->className()) == QLatin1String("QTipLabel")) {
            tip = w;
        }
    }
    out << "   QTipLabel gefunden: " << (tip ? "ja" : "NEIN") << "\n";
    if (tip) {
        const QImage bild = tip->grab().toImage();
        bild.save(QStringLiteral("../messungen/sonde5-tooltip-offscreen.png"));
        out << "   Bild " << bild.width() << "x" << bild.height()
            << " geschrieben: messungen/sonde5-tooltip-offscreen.png\n";
    }
    out << "C) QToolTip::text() = \"" << QToolTip::text() << "\", sichtbar="
        << (QToolTip::isVisible() ? "ja" : "nein") << "\n";
    out.flush();
    return 0;
}
