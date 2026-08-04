// Prüft, welche Farbe QPalette::PlaceholderText unter dem KDE-Plattformthema
// annimmt, und hält sie gegen ForegroundInactive der [Colors:Window]-Gruppe
// des Farbschemas. Hintergrund: AK 10 von #83 nennt ForegroundInactive,
// der Code (capturewindow.cpp:132) setzt PlaceholderText.
#include <QApplication>
#include <QLabel>
#include <QPalette>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    QLabel label(QStringLiteral("Denkzettel"));
    label.setForegroundRole(QPalette::PlaceholderText);
    const QPalette p = label.palette();
    auto zeig = [&out](const char *name, const QColor &c) {
        out << QString::asprintf("%-22s %3d,%3d,%3d\n", name, c.red(), c.green(), c.blue());
    };
    out << "Farbschema: " << qgetenv("PRUEF_SCHEMA") << "\n\n";
    zeig("PlaceholderText", p.color(QPalette::PlaceholderText));
    zeig("WindowText", p.color(QPalette::WindowText));
    zeig("Window", p.color(QPalette::Window));
    zeig("Text (disabled)", p.color(QPalette::Disabled, QPalette::Text));
    zeig("WindowText(disabled)", p.color(QPalette::Disabled, QPalette::WindowText));
    return 0;
}
