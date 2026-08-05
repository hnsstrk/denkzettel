/*
 * Sonde der Vorprüfung zu #85, Bearbeiter A (UX).
 *
 * Zwei Fragen, die eine Story mit acht Themes zuerst beantwortet haben muss:
 *
 *  A) **Färbt ein lebendes `KSvg::ImageSet` ein zweites daneben?** Für die
 *     Auswahlpfade ist das gemessen (Sprint 7, Fund 2). Für die **Farben** ist
 *     es offen — und genau die will #85 vergleichen.
 *  B) **Zieht `KSvg::Svg::color()` nach, wenn das Fenster sein Theme
 *     wechselt?** `reloadDesktopTheme()` setzt einen frischen Bildsatz; ob die
 *     Farbe daran hängt, entscheidet über den Prüfweg von AK 5.
 *
 * Aufruf: nebenlauf <theme des fensters> <fremdes theme>
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>

namespace
{
QTextStream out(stdout);

QString farbe(const QColor &c)
{
    return QStringLiteral("%1,%2,%3/a%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

struct Messung {
    QColor text;
    QColor flaeche;
};

/** Ein eigenständiger Bildsatz neben allem, was sonst lebt. */
Messung daneben(const QString &theme)
{
    auto satz = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::FrameSvg rahmen;
    rahmen.setImageSet(satz.get());
    rahmen.setImagePath(QStringLiteral("dialogs/background"));
    rahmen.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    rahmen.setColorSet(KSvg::Svg::Window);
    rahmen.resizeFrame(QSizeF(600, 174));

    QImage bild(600, 174, QImage::Format_ARGB32);
    bild.fill(Qt::transparent);
    if (rahmen.isValid()) {
        QPainter maler(&bild);
        maler.drawPixmap(0, 0, rahmen.framePixmap());
    }

    return {rahmen.color(KSvg::Svg::Text), bild.pixelColor(300, 87)};
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QStringList argumente = app.arguments().mid(1);
    if (argumente.size() < 2) {
        out << "Aufruf: nebenlauf <theme des fensters> <fremdes theme>\n";
        out.flush();
        return 2;
    }
    const QString eigenes = argumente.at(0);
    const QString fremdes = argumente.at(1);

    // Kein Testmodus: er schneidet das Farbschema des Kunden ab, und `default`
    // folgt genau diesem Schema — die halbe Messung stünde dann auf der
    // Qt-Ersatzpalette (gemessen, erster Lauf dieser Sonde). Das Schema kommt
    // aus einem eigenen XDG_CONFIG_HOME, in dem eine **Kopie** seiner
    // kdeglobals liegt; geschrieben wird dort nichts von uns.
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));

    out << "### A — Messung ohne Fenster (Bezugsgröße)\n";
    const Messung alleinEigenes = daneben(eigenes);
    const Messung alleinFremdes = daneben(fremdes);
    out << "  " << eigenes << " allein: Text " << farbe(alleinEigenes.text) << "  Fläche "
        << farbe(alleinEigenes.flaeche) << "\n";
    out << "  " << fremdes << " allein: Text " << farbe(alleinFremdes.text) << "  Fläche "
        << farbe(alleinFremdes.flaeche) << "\n\n";

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store nicht offen: " << store.lastError() << "\n";
        out.flush();
        return 1;
    }

    CaptureWindow fenster(&store);
    fenster.reloadDesktopTheme(eigenes);

    out << "### A — dieselbe Messung, während das Fenster " << eigenes << " trägt\n";
    const Messung nebenEigenes = daneben(eigenes);
    const Messung nebenFremdes = daneben(fremdes);
    out << "  " << eigenes << " daneben: Text " << farbe(nebenEigenes.text) << "  Fläche "
        << farbe(nebenEigenes.flaeche)
        << (nebenEigenes.flaeche == alleinEigenes.flaeche ? QStringLiteral("   Fläche gleich")
                                                          : QStringLiteral("   FLÄCHE VERSCHIEDEN"))
        << (nebenEigenes.text == alleinEigenes.text ? QStringLiteral("   Text gleich")
                                                    : QStringLiteral("   TEXT VERSCHIEDEN"))
        << "\n";
    out << "  " << fremdes << " daneben: Text " << farbe(nebenFremdes.text) << "  Fläche "
        << farbe(nebenFremdes.flaeche)
        << (nebenFremdes.flaeche == alleinFremdes.flaeche ? QStringLiteral("   Fläche gleich")
                                                          : QStringLiteral("   FLÄCHE VERSCHIEDEN"))
        << (nebenFremdes.text == alleinFremdes.text ? QStringLiteral("   Text gleich")
                                                    : QStringLiteral("   TEXT VERSCHIEDEN"))
        << "\n\n";

    out << "### B — zieht die Farbe des Fensters beim Theme-Wechsel nach?\n";
    out << "  Gemessen wird an einem eigenen FrameSvg, der denselben Weg geht wie die\n";
    out << "  Hülle: frischer Bildsatz je Wechsel, wie reloadDesktopTheme() ihn baut.\n";
    KSvg::FrameSvg huelle;
    huelle.setImagePath(QStringLiteral("dialogs/background"));
    huelle.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    huelle.setColorSet(KSvg::Svg::Window);

    std::unique_ptr<KSvg::ImageSet> satz;
    for (const QString &theme : {eigenes, fremdes, eigenes}) {
        auto neuer = std::make_unique<KSvg::ImageSet>(theme, QStringLiteral("plasma/desktoptheme"));
        huelle.setImageSet(neuer.get());
        satz = std::move(neuer);
        huelle.resizeFrame(QSizeF(600, 174));
        out << "  nach Wechsel auf " << theme << ": color(Text) " << farbe(huelle.color(KSvg::Svg::Text))
            << "\n";
    }

    out.flush();
    return 0;
}
