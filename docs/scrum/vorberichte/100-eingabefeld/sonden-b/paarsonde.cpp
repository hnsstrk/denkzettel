// Welches Theme-Paar der Bildläufer nimmt — und ob es die Belegform von F1
// trifft (eines mit eigener lineedit-Grafik, eines ohne).
#include "desktopthemes.h"
#include <QApplication>
#include <QTextStream>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    themes::addBundledThemesToDataPath();
    if (const auto pair = themes::installedThemePair()) {
        out << "installedThemePair(): schmal=" << pair->first << "  breit=" << pair->second << "\n";
    } else {
        out << "installedThemePair(): keines\n";
    }
    if (const auto any = themes::anyInstalledTheme()) {
        out << "anyInstalledTheme(): " << *any << "\n";
    }
    out << "installedThemes(): " << themes::installedThemes().join(QStringLiteral(", ")) << "\n";
    return 0;
}
