// Vorprüfung #72 — Sonde: Woher kommt ein Tooltip-Text mit Kürzel?
// Kein Produktivcode. Misst Qt-/KF6-Verhalten, nicht Denkzettel-Code.

#include <KLocalizedString>
#include <KStandardShortcut>
#include <KMessageWidget>
#include <KToolTipHelper>

#include <QAction>
#include <QApplication>
#include <QHelpEvent>
#include <QKeySequence>
#include <QLibraryInfo>
#include <QPushButton>
#include <QTextStream>
#include <QToolButton>
#include <QToolTip>
#include <QTranslator>

static QTextStream out(stdout);

static void line()
{
    out << "-------------------------------------------------------------\n";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    out << "Plattform: " << app.platformName() << "\n";
    line();

    // --- A: Was liefert QAction::toolTip() ohne eigenes setToolTip()? -----
    out << "A) QAction-Vorgabetooltip (ohne setToolTip)\n";
    QAction edit(QStringLiteral("Bearbeiten"), nullptr);
    edit.setShortcut(Qt::Key_F2);
    QAction del(QStringLiteral("Löschen"), nullptr);
    del.setShortcut(QKeySequence::Delete);
    QAction undo(QStringLiteral("Rückgängig"), nullptr);
    undo.setShortcuts(KStandardShortcut::undo());
    for (QAction *a : {&edit, &del, &undo}) {
        out << "  text=\"" << a->text() << "\"  toolTip()=\"" << a->toolTip()
            << "\"  shortcut()=\"" << a->shortcut().toString(QKeySequence::NativeText) << "\"\n";
    }
    out << "  undo hat " << undo.shortcuts().size() << " Kürzel:";
    for (const QKeySequence &s : undo.shortcuts()) {
        out << " [" << s.toString(QKeySequence::NativeText) << "]";
    }
    out << "\n";
    line();

    // --- B: Wie heißen die Kürzel als Text? Mit und ohne Qt-Übersetzung ---
    out << "B) QKeySequence::toString — Portable vs. NativeText, ohne Qt-Katalog\n";
    const QList<QKeySequence> keys{QKeySequence(Qt::Key_F2), QKeySequence(QKeySequence::Delete),
                                   KStandardShortcut::undo().value(0)};
    for (const QKeySequence &s : keys) {
        out << "  Portable=\"" << s.toString(QKeySequence::PortableText) << "\"  Native=\""
            << s.toString(QKeySequence::NativeText) << "\"\n";
    }
    out << "  Qt-Übersetzungspfad: " << QLibraryInfo::path(QLibraryInfo::TranslationsPath) << "\n";
    QTranslator qt;
    const bool loaded = qt.load(QStringLiteral("qtbase_de"), QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    out << "  qtbase_de geladen: " << (loaded ? "ja" : "NEIN") << "\n";
    if (loaded) {
        app.installTranslator(&qt);
        out << "  nach dem Laden:\n";
        for (const QKeySequence &s : keys) {
            out << "    Native=\"" << s.toString(QKeySequence::NativeText) << "\"\n";
        }
        app.removeTranslator(&qt);
    }
    line();

    // --- C: KMessageWidget::addAction — was für ein Knopf entsteht? -------
    out << "C) KMessageWidget::addAction — welcher Knopf, welcher Tooltip?\n";
    KMessageWidget message;
    message.setMessageType(KMessageWidget::Warning);
    message.setCloseButtonVisible(false);
    message.setWordWrap(false);
    undo.setToolTip(QStringLiteral("PROBE-TOOLTIP"));
    message.addAction(&undo);
    const auto tools = message.findChildren<QToolButton *>();
    out << "  QToolButton-Kinder: " << tools.size() << "\n";
    for (QToolButton *b : tools) {
        out << "    text=\"" << b->text() << "\"  defaultAction="
            << (b->defaultAction() ? "gesetzt" : "keine") << "  toolTip()=\"" << b->toolTip() << "\"\n";
    }
    undo.setToolTip(QString()); // zurücksetzen: Vorgabe wieder aus text()
    out << "  nach setToolTip(QString()):\n";
    for (QToolButton *b : tools) {
        out << "    toolTip()=\"" << b->toolTip() << "\"\n";
    }
    line();

    // --- D: QPushButton mit verbundener QAction (Denkzettel-Bauart) -------
    out << "D) QPushButton, per connect an eine QAction gehängt (Bauart in librarywindow.cpp)\n";
    QPushButton push(QStringLiteral("Bearbeiten"));
    QObject::connect(&push, &QPushButton::clicked, &edit, &QAction::trigger);
    out << "  toolTip()=\"" << push.toolTip() << "\" (leer = kein Tooltip)\n";
    out << "  actions().size()=" << push.actions().size() << "\n";
    QPushButton pushDefault;
    pushDefault.addAction(&edit);
    out << "  QPushButton::addAction -> actions().size()=" << pushDefault.actions().size()
        << ", toolTip()=\"" << pushDefault.toolTip() << "\"\n";
    line();

    // --- E: KToolTipHelper — ändert er toolTip() oder nur die Anzeige? ----
    out << "E) KToolTipHelper (KF6::XmlGui)\n";
    qApp->installEventFilter(KToolTipHelper::instance());
    QToolButton withAction;
    withAction.setDefaultAction(&edit);
    out << "  QToolButton mit defaultAction: toolTip()=\"" << withAction.toolTip() << "\"\n";
    out << "  QPushButton:                   toolTip()=\"" << push.toolTip() << "\"\n";

    // Anzeige-Ebene: ein QEvent::ToolTip zustellen und QToolTip::text() lesen.
    push.setToolTip(QStringLiteral("Notiz bearbeiten"));
    withAction.setToolTip(QStringLiteral("Notiz bearbeiten"));
    push.resize(120, 30);
    withAction.resize(120, 30);
    push.show();
    withAction.show();
    app.processEvents();
    for (QWidget *w : QList<QWidget *>{&push, &withAction}) {
        QHelpEvent help(QEvent::ToolTip, QPoint(10, 10), w->mapToGlobal(QPoint(10, 10)));
        QApplication::sendEvent(w, &help);
        app.processEvents();
        out << "  nach QEvent::ToolTip an " << w->metaObject()->className()
            << ": QToolTip::text()=\"" << QToolTip::text() << "\" sichtbar="
            << (QToolTip::isVisible() ? "ja" : "nein") << "\n";
        QToolTip::hideText();
    }
    line();

    out << "Fertig.\n";
    out.flush();
    return 0;
}
