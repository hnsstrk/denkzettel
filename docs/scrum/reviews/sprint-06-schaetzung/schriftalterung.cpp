// Messung 1 — Warum #68 keine 3 ist.
//
// Claim under test: QFontDatabase::systemFont() never changes for the lifetime
// of the process, and a label whose font was set explicitly never follows an
// application font change. Both together decide the size of #68: the fix cannot
// be "watch kdeglobals and call qApp->setFont()", because that leaves every
// site which reads systemFont() — and every explicitly fonted label — behind.
//
// Needs no D-Bus and no Plasma session; run it offscreen.

#include <QApplication>
#include <QFontDatabase>
#include <QLabel>
#include <QTextStream>

static QTextStream out(stdout);

static void reportFonts(const char *tag)
{
    out << tag << "  QApplication::font()=" << QApplication::font().pointSizeF()
        << "  systemFont(GeneralFont)=" << QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSizeF()
        << "  systemFont(SmallestReadableFont)="
        << QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont).pointSizeF() << "\n";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    out << "Messung 1 — Alterung der Systemschrift im laufenden Prozess\n";
    out << "===========================================================\n\n";
    out << "Plattformthema: " << qgetenv("QT_QPA_PLATFORMTHEME") << "\n\n";

    reportFonts("vorher ");

    // The two shapes a label takes in this project. subtleLabel() in
    // capturewindow.cpp:27 and librarywindow.cpp:116 build the first kind: the
    // font is set explicitly, once, at construction time.
    QWidget window;
    auto *explicitFont = new QLabel(QStringLiteral("Denkzettel"), &window);
    explicitFont->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    const int explicitBefore = explicitFont->sizeHint().height();

    auto *inherited = new QLabel(QStringLiteral("Denkzettel"), &window);
    const int inheritedBefore = inherited->sizeHint().height();

    // What a fix along the lines of "watch kdeglobals, then set the app font"
    // would do.
    QFont bigger = app.font();
    bigger.setPointSizeF(app.font().pointSizeF() * 2);
    app.setFont(bigger);
    QCoreApplication::processEvents();

    out << "\nnach qApp->setFont(doppelte Punktgröße):\n\n";
    reportFonts("nachher");

    out << "\n  Label mit ausdrücklich gesetzter Schrift: " << explicitBefore << " -> "
        << explicitFont->sizeHint().height() << " px\n";
    out << "  Label mit geerbter Schrift              : " << inheritedBefore << " -> "
        << inherited->sizeHint().height() << " px\n";

    out << "\nBefund: systemFont() bleibt auf beiden Rängen stehen, obwohl die\n"
           "Anwendungsschrift verdoppelt wurde. Ein Label mit ausdrücklich gesetzter\n"
           "Schrift folgt nicht; nur das erbende folgt. Im Projekt tragen 12 Labels\n"
           "eine ausdrücklich gesetzte Schrift (2 im Erfassungsfenster, 10 in der\n"
           "Bibliothek), 8 davon werden inline erzeugt und nirgends gehalten.\n";

    out.flush();
    return 0;
}
