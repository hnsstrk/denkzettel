// Vorprüfung #72 — Sonde 2: Woraus setzt KToolTipHelper den Text zusammen,
// und was passiert an einer abgeschalteten Schaltfläche?

#include <KStandardShortcut>
#include <KToolTipHelper>

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QHelpEvent>
#include <QPushButton>
#include <QTextStream>
#include <QToolButton>
#include <QToolTip>
#include <QWidget>

static QTextStream out(stdout);

static QString shown(QWidget *w)
{
    QHelpEvent help(QEvent::ToolTip, QPoint(5, 5), w->mapToGlobal(QPoint(5, 5)));
    QApplication::sendEvent(w, &help);
    qApp->processEvents();
    const QString t = QToolTip::text();
    QToolTip::hideText();
    return t;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    qApp->installEventFilter(KToolTipHelper::instance());
    out << "Plattform: " << app.platformName() << "  Locale: " << QLocale().name() << "\n";
    out << "LANG=" << qEnvironmentVariable("LANG") << " LC_ALL=" << qEnvironmentVariable("LC_ALL") << "\n";
    out << "Kürzeltext hier: Entf=\"" << QKeySequence(QKeySequence::Delete).toString(QKeySequence::NativeText)
        << "\" Undo=\"" << KStandardShortcut::undo().value(0).toString(QKeySequence::NativeText) << "\"\n";
    out << "-------------------------------------------------------------\n";

    QAction act(QStringLiteral("Bearbeiten"), nullptr);
    act.setShortcut(Qt::Key_F2);

    out << "A) QToolButton + setDefaultAction — woher kommt der angezeigte Text?\n";
    QToolButton tb;
    tb.setDefaultAction(&act);
    tb.resize(120, 30);
    tb.show();
    qApp->processEvents();
    out << "  Vorgabe (Aktion ohne eigenen Tooltip): \"" << shown(&tb) << "\"\n";
    act.setToolTip(QStringLiteral("Notiz zum Bearbeiten öffnen"));
    out << "  Aktion mit eigenem Tooltip:            \"" << shown(&tb) << "\"\n";
    tb.setToolTip(QStringLiteral("KNOPF-EIGENER-TEXT"));
    out << "  danach Tooltip am Knopf gesetzt:       \"" << shown(&tb) << "\"\n";
    act.setToolTip(QString());
    tb.setToolTip(QStringLiteral("KNOPF-EIGENER-TEXT"));
    out << "  Knopftext, Aktion ohne Tooltip:        \"" << shown(&tb) << "\"\n";
    out << "-------------------------------------------------------------\n";

    out << "B) QPushButton — greift der Helfer?\n";
    QPushButton pb(QStringLiteral("Bearbeiten"));
    pb.setToolTip(QStringLiteral("Notiz bearbeiten"));
    pb.resize(120, 30);
    pb.show();
    qApp->processEvents();
    out << "  QPushButton, eigener Tooltip:          \"" << shown(&pb) << "\"\n";
    pb.addAction(&act);
    out << "  QPushButton nach addAction(F2):        \"" << shown(&pb) << "\"\n";
    out << "-------------------------------------------------------------\n";

    out << "C) Abgeschaltete Schaltfläche — wird sie überhaupt getroffen?\n";
    QWidget box;
    auto *row = new QHBoxLayout(&box);
    auto *off = new QPushButton(QStringLiteral("Bearbeiten"), &box);
    off->setEnabled(false);
    off->setToolTip(QStringLiteral("Bearbeiten (F2)"));
    row->addWidget(off);
    box.resize(200, 60);
    box.show();
    qApp->processEvents();
    const QPoint global = off->mapToGlobal(QPoint(5, 5));
    QWidget *hit = QApplication::widgetAt(global);
    out << "  widgetAt über dem abgeschalteten Knopf: "
        << (hit ? hit->metaObject()->className() : "nullptr")
        << (hit == off ? "  (= der Knopf selbst)" : "  (NICHT der Knopf)") << "\n";
    out << "  Tooltip bei direkter Zustellung:       \"" << shown(off) << "\"\n";
    out << "-------------------------------------------------------------\n";
    out.flush();
    return 0;
}
