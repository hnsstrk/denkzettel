/*
 * Fokuszustand des Textfeldes (#100), Messung 3: die Hinweis-Elemente, an denen
 * Plasmas eigener Bau die Fallunterscheidungen festmacht.
 *
 * Quelle der Fragen ist Plasmas `TextField.qml` (Zeile 223) und
 * `private/TextFieldFocus.qml` (Zeile 18–33): Der Fokusrahmen nimmt den Vorsatz
 * `focusframe`, wenn `focusframe-center` vorhanden ist, sonst `focus`; und er
 * liegt über der Grundfläche, wenn `hint-focus-over-base` vorhanden ist, sonst
 * darunter. Beide Fragen entscheiden mit, was eine solche Schicht im Bau kostet.
 *
 * Dazu die Ränder, die die drei Vorsätze für sich beanspruchen — Plasma zieht
 * den Fokusrahmen mit negativen Rändern nach aussen (`leftMargin: -margins.left`).
 *
 * Aufruf: hinweissonde
 */
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KSvg/Svg>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    const QString themePath = QStringLiteral("plasma/desktoptheme");
    QStringList names;
    for (const QString &root : QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, themePath, QStandardPaths::LocateDirectory)) {
        for (const QString &n : QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
            if (!names.contains(n)) names << n;
        }
    }
    out << QStringLiteral("%1 %2 %3 %4 %5 %6\n")
               .arg(QStringLiteral("Theme"), -24)
               .arg(QStringLiteral("focusframe-center"), 18)
               .arg(QStringLiteral("hint-focus-over-base"), 21)
               .arg(QStringLiteral("Rand base"), 10)
               .arg(QStringLiteral("Rand focus"), 11)
               .arg(QStringLiteral("Rand focusframe"), 16);
    for (const QString &theme : names) {
        KSvg::ImageSet set(theme, themePath);
        KSvg::Svg svg;
        svg.setImageSet(&set);
        svg.setImagePath(QStringLiteral("widgets/lineedit"));
        auto rand = [&set](const QString &prefix) {
            KSvg::FrameSvg f;
            f.setImageSet(&set);
            f.setImagePath(QStringLiteral("widgets/lineedit"));
            f.setElementPrefix(prefix);
            f.setEnabledBorders(KSvg::FrameSvg::AllBorders);
            f.resizeFrame(QSizeF(544, 90));
            if (!f.hasElementPrefix(prefix)) {
                return QStringLiteral("—");
            }
            return QStringLiteral("%1/%2/%3/%4")
                .arg(f.marginSize(KSvg::FrameSvg::LeftMargin), 0, 'f', 0)
                .arg(f.marginSize(KSvg::FrameSvg::TopMargin), 0, 'f', 0)
                .arg(f.marginSize(KSvg::FrameSvg::RightMargin), 0, 'f', 0)
                .arg(f.marginSize(KSvg::FrameSvg::BottomMargin), 0, 'f', 0);
        };
        out << QStringLiteral("%1 %2 %3 %4 %5 %6\n")
                   .arg(theme, -24)
                   .arg(svg.hasElement(QStringLiteral("focusframe-center")) ? QStringLiteral("ja") : QStringLiteral("nein"), 18)
                   .arg(svg.hasElement(QStringLiteral("hint-focus-over-base")) ? QStringLiteral("ja") : QStringLiteral("nein"), 21)
                   .arg(rand(QStringLiteral("base")), 10)
                   .arg(rand(QStringLiteral("focus")), 11)
                   .arg(rand(QStringLiteral("focusframe")), 16);
    }
    return 0;
}
