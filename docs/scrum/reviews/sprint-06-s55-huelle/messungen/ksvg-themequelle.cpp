// Messung 1 — Woher kennt KSvg das Desktop-Theme?
//
// Die Annahme, gegen die diese Sonde läuft, ist die naheliegende: `KSvg` sei
// die KDE-Bibliothek für Desktop-Theme-Grafik, also finde sie das eingestellte
// Theme selbst. Wäre es so, brauchte das Erfassungsfenster weder KConfigCore
// noch eine Wache auf `plasmarc`.
//
// Die Zweideutigkeit, an der die Frage hängt: Ein frisch gebautes `ImageSet`
// meldet den Namen `default` — und `default` ist zugleich der Wert, der auf
// dieser Maschine in `plasmarc` steht. Aus dem einen Lauf ist nicht zu sehen,
// ob KSvg gelesen oder geraten hat. Entschieden wird es mit einem eigenen
// XDG_CONFIG_HOME, in dem ein anderes Theme steht: Folgt KSvg der Datei, muss
// der Name mitgehen. Die Konfiguration des Kunden wird dabei nicht angefasst.

#include <KConfigGroup>
#include <KSharedConfig>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QTextStream>

namespace
{
QTextStream out(stdout);

void report(const QString &tag)
{
    KConfigGroup group(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Theme"));

    KSvg::ImageSet set;
    set.setBasePath(QStringLiteral("plasma/desktoptheme"));

    out << tag << "\n";
    out << "   plasmarc [Theme] name = " << group.readEntry("name", QStringLiteral("<nicht gesetzt>")) << "\n";
    out << "   ImageSet::imageSetName() = " << set.imageSetName() << "\n";
    out << "   aufgelöst auf = " << set.imagePath(QStringLiteral("dialogs/background")) << "\n\n";
    out.flush();
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    out << "Messung 1 — Woher kennt KSvg das Desktop-Theme? (#55)\n";
    out << "====================================================\n\n";

    report(app.arguments().size() > 1 ? app.arguments().at(1) : QStringLiteral("Lauf"));

    // Ein zweites Bild derselben Frage: Ein ImageSet, dem der Name ausdrücklich
    // mitgegeben wird, löst auf — es liegt also nicht an fehlenden Dateien.
    KSvg::ImageSet named(QStringLiteral("CachyOS-Nord-round"), QStringLiteral("plasma/desktoptheme"));
    KSvg::FrameSvg frame;
    frame.setImageSet(&named);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(600, 200));

    out << "   Gegenprobe, Name ausdrücklich mitgegeben:\n";
    out << "   ImageSet(\"CachyOS-Nord-round\") -> gültig=" << frame.isValid()
        << "  Rand links=" << frame.marginSize(KSvg::FrameSvg::LeftMargin) << "\n";

    out.flush();
    return 0;
}
