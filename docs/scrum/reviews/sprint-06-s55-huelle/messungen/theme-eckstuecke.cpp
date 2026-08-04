// Messung 4 — Eckstücke, Ränder und Schattenkacheln aller installierten Themes.
//
// Sie schließt K6 des Sprint-6-Plannings. Die AK-1-Richtigstellung an #55 vom
// 02.08.2026 sagt: *alle acht installierten Themes bringen Eckstücke und
// Schattenkacheln mit* — und beruft sich auf `achse3-huellen.txt`. Diese Datei
// führt aber nur Ränder, Schattenteile und Füllfarbe; **eine Spalte für
// Eckstücke hat sie nicht.** Der Scrum Master hat die Aussage im Planning selbst
// nachgemessen und bestätigt; sein Beleg stand jedoch nur in einem Absatz des
// Protokolls, und ein Absatz ist keine Messdatei.
//
// Gemessen wird durch `KSvg::FrameSvg` — also so, wie der Produktivcode das
// Theme sieht, nicht durch Auszählen von Element-IDs in der SVG-Datei. Dazu die
// **Eckform**: wie weit die Hülle in der obersten Bildzeile noch durchsichtig
// ist. Sie trägt den Prüfsatz von AK 1, und sie ist der Grund, warum dort
// „nicht aus dem Randmaß ableiten" steht — zwei Themes können bei gleichem Rand
// verschieden gekrümmt sein.

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QDir>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QTextStream>

namespace
{
QTextStream out(stdout);

/** Wie weit die oberste Bildzeile links noch vollständig durchsichtig ist. */
int cornerRun(const QImage &alpha)
{
    int x = 0;
    while (x < alpha.width() && qAlpha(alpha.pixel(x, 0)) == 0) {
        ++x;
    }
    return x;
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    out << "Messung 4 — Eckstücke, Ränder und Schattenkacheln (#55, AK 1 · K6)\n";
    out << "==================================================================\n\n";

    QStringList themes;
    const QStringList roots = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("plasma/desktoptheme"),
                                                        QStandardPaths::LocateDirectory);
    for (const QString &root : roots) {
        for (const QString &name : QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            if (!themes.contains(name)) {
                themes << name;
            }
        }
    }

    out << "Gefunden: " << themes.size() << " Desktop-Themes in " << roots.join(QStringLiteral(", ")) << "\n\n";
    out << "Theme                   gültig  Rand links/oben/rechts/unten      topleft  shadow-Präfix  Eckform  Maske\n";
    out << "--------------------------------------------------------------------------------------------------------\n";

    for (const QString &name : themes) {
        KSvg::ImageSet set(name, QStringLiteral("plasma/desktoptheme"));

        KSvg::FrameSvg frame;
        frame.setImageSet(&set);
        frame.setImagePath(QStringLiteral("dialogs/background"));
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        frame.resizeFrame(QSizeF(600, 200));

        KSvg::FrameSvg shadow;
        shadow.setImageSet(&set);
        shadow.setImagePath(QStringLiteral("dialogs/background"));
        shadow.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        shadow.setElementPrefix(QStringLiteral("shadow"));
        shadow.resizeFrame(QSizeF(600, 200));

        qreal left = 0;
        qreal top = 0;
        qreal right = 0;
        qreal bottom = 0;
        frame.getMargins(left, top, right, bottom);

        out << name.leftJustified(24) << QString::number(frame.isValid()).leftJustified(8)
            << QStringLiteral("%1/%2/%3/%4")
                   .arg(left, 0, 'g', 6)
                   .arg(top, 0, 'g', 6)
                   .arg(right, 0, 'g', 6)
                   .arg(bottom, 0, 'g', 6)
                   .leftJustified(34)
            << QString::number(frame.hasElement(QStringLiteral("topleft"))).leftJustified(9)
            << QString::number(shadow.hasElementPrefix(QStringLiteral("shadow"))).leftJustified(15)
            << QString::number(cornerRun(frame.alphaMask().toImage())).leftJustified(9)
            << (frame.mask().isEmpty() ? QStringLiteral("leer") : QStringLiteral("belegt")) << "\n";
        out.flush();
    }

    out << "\nZu lesen:\n"
           "  * `Rand` ist der Streifen, den die Hülle für sich beansprucht — nicht die\n"
           "    Strichstärke der Kontur. Er kommt als Fließkomma und ist nicht glatt:\n"
           "    ein 8-px-Theme liefert 7,99998. Eine absolute Zusicherung fiele daran.\n"
           "  * `Eckform` ist die Zahl, die den Prüfsatz von AK 1 trägt: wie weit die\n"
           "    oberste Zeile noch durchsichtig ist. Sie ist **nicht** aus dem Randmaß\n"
           "    abzuleiten — dass zwei Themes mit gleichem Rand verschieden gekrümmt\n"
           "    sein können, ist genau der Grund für diese Spalte.\n"
           "  * Themes ohne eigenes `dialogs/background` erben das von `default`; sie\n"
           "    stehen deshalb mit dessen Werten in der Tabelle und sind trotzdem echt\n"
           "    aufgelöst (Spalte `gültig`).\n";
    out.flush();

    return 0;
}
