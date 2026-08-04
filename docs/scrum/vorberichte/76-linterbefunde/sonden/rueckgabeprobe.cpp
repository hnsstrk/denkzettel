// Probe for the vorprüfung of #76: what does the return value of
// KGlobalAccel::setGlobalShortcut() report, and does the read-back in
// GlobalShortcuts::registerCaptureShortcut() cover it?
//
// Deliberately harmless: the action carries no objectName, which kglobalaccel
// refuses before it sends anything over D-Bus. Nothing is registered, and the
// component name is never used, so the daemon's stored state is not touched.
#include <KGlobalAccel>

#include <QAction>
#include <QGuiApplication>
#include <QKeySequence>
#include <QTextStream>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);

    QAction nameless(nullptr);
    nameless.setProperty("componentName", QStringLiteral("denkzettel-vorpruefung-76.desktop"));
    const bool refused = KGlobalAccel::setGlobalShortcut(&nameless, QKeySequence(Qt::META | Qt::Key_F12));
    out << "A  Aktion ohne objectName -> setGlobalShortcut() = " << (refused ? "true" : "false") << "\n";

    // What the daemon holds for an action that was never registered.
    const QList<QKeySequence> stored =
        KGlobalAccel::self()->globalShortcut(QStringLiteral("denkzettel-vorpruefung-76.desktop"),
                                             QStringLiteral("nie-registriert"));
    out << "B  Rücklesen einer nie registrierten Aktion -> " << stored.size() << " Sequenz(en)\n";

    // And what it holds for the real registration of the running daemon.
    const QList<QKeySequence> ours =
        KGlobalAccel::self()->globalShortcut(QStringLiteral("org.denkzettel.Denkzettel.desktop"),
                                             QStringLiteral("show-capture"));
    out << "C  Rücklesen von org.denkzettel.Denkzettel.desktop/show-capture -> " << ours.size() << " Sequenz(en)";
    for (const QKeySequence &s : ours) {
        out << " [" << s.toString() << "]";
    }
    out << "\n";
    return 0;
}
