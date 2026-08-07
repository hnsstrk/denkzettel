// Wo das Textfeld im Fenster sitzt — und ob der Mittelpunkt des Fensterbildes,
// den zwei bestehende Prüfsätze abtasten, darin liegt.
//
// Diese Sonde braucht das Fenster selbst, steht also nicht im CMakeLists.txt
// daneben: sie wird gegen einen eigenen Projektbau gelinkt. Vom
// Projektwurzelverzeichnis aus:
//
//   cmake -B docs/scrum/vorberichte/100-eingabefeld/build-projekt -S . \
//         -DCMAKE_BUILD_TYPE=Debug
//   cmake --build docs/scrum/vorberichte/100-eingabefeld/build-projekt -j8
//   cd docs/scrum/vorberichte/100-eingabefeld
//   g++ -std=c++20 -fPIC -o build-b/geometriesonde sonden-b/geometriesonde.cpp \
//       -I../../../../src -I../../../../tests \
//       -DDENKZETTEL_TEST_THEMES='"<Projektpfad>/tests/themes"' \
//       $(pkg-config --cflags Qt6Widgets Qt6Sql Qt6DBus) \
//       -I/usr/include/KF6 -I/usr/include/KF6/KSvg -I/usr/include/KF6/KI18n \
//       -I/usr/include/KF6/KConfigCore -I/usr/include/KF6/KConfig \
//       -I/usr/include/KF6/KCoreAddons -I/usr/include/KF6/KWindowSystem \
//       build-projekt/lib/libdenkzettelcapture.a build-projekt/lib/libdenkzettelstore.a \
//       $(pkg-config --libs Qt6Widgets Qt6Sql Qt6DBus) \
//       -lKF6Svg -lKF6I18n -lKF6ConfigCore -lKF6CoreAddons -lKF6WindowSystem
//   QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde ./build-b/geometriesonde
//
// `-fPIC` ist nicht Zierrat: ohne die Angabe bricht der Binder an einer
// Kopie-Umlagerung gegen `QPlainTextEdit::staticMetaObject` ab.
// `-DCMAKE_INSTALL_PREFIX` bleibt ungesetzt — hier wird nichts installiert.
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"

#include <KLocalizedString>
#include <QApplication>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));
    QTextStream out(stdout);

    themes::addBundledThemesToDataPath();

    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store fehlgeschlagen\n";
        return 1;
    }

    for (const QString &theme : {QStringLiteral("default"),
                                 QStringLiteral("denkzettel-test-schmal"),
                                 QStringLiteral("CachyOS-Nord-round")}) {
        CaptureWindow window(&store);
        window.reloadDesktopTheme(theme);
        window.show();
        QCoreApplication::processEvents();

        auto *text = window.findChild<QPlainTextEdit *>();
        const QRect r = text->geometry();
        const QPoint centre(window.width() / 2, window.height() / 2);

        out << theme << ": Fenster " << window.width() << "x" << window.height()
            << "  Textbereich " << r.x() << "," << r.y() << " " << r.width() << "x" << r.height()
            << "  Mitte " << centre.x() << "," << centre.y()
            << "  Mitte im Textbereich: " << (r.contains(centre) ? "JA" : "nein")
            << "  documentMargin " << text->document()->documentMargin()
            << "  frameWidth " << text->frameWidth() << "\n";

        // Und bei acht Zeilen, der zweiten Größe aus SPEC 3.
        text->setPlainText(QStringLiteral("eins\nzwei\ndrei\nvier\nfünf\nsechs\nsieben\nacht"));
        QCoreApplication::processEvents();
        const QRect r8 = text->geometry();
        const QPoint c8(window.width() / 2, window.height() / 2);
        out << theme << " (acht Zeilen): Fenster " << window.width() << "x" << window.height()
            << "  Textbereich " << r8.x() << "," << r8.y() << " " << r8.width() << "x" << r8.height()
            << "  Mitte " << c8.x() << "," << c8.y()
            << "  Mitte im Textbereich: " << (r8.contains(c8) ? "JA" : "nein") << "\n";
    }
    return 0;
}
