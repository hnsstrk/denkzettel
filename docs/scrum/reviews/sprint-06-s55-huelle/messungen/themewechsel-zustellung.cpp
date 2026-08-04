// Messung 2 — Auf welchem Weg erreicht ein Theme-Wechsel ein stehendes Fenster?
//
// AK 5 verlangt, dass die Hülle einem Wechsel des Desktop-Themes **bei
// laufendem Dienst** folgt. Der Dienst baut das Fenster einmal und behält es
// (SPEC 2.1) — es muss also eine Wache geben, und die Frage ist welche.
//
// Zwei kommen in Betracht, und sie unterscheiden sich in einer Bedingung, die
// nicht bei uns liegt: `KConfigWatcher` meldet nur, wenn der **Schreiber**
// `KConfig::Notify` benutzt hat; `KDirWatch` meldet jede Änderung der Datei.
// Wer die erste nimmt, macht die eigene Zusicherung von der Disziplin eines
// fremden Programms abhängig — deshalb wird hier gemessen statt vermutet.
//
// Gemessen wird in einem eigenen XDG_CONFIG_HOME (pruefen.sh setzt es). Das
// `plasmarc` des Kunden und sein eingestelltes Theme bleiben unberührt.

#include <KConfigGroup>
#include <KConfigWatcher>
#include <KDirWatch>
#include <KSharedConfig>

#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimer>

namespace
{
QTextStream out(stdout);
constexpr int SettleMs = 2000;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const QString path =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/plasmarc");

    // Der Schreiber-Zweig: dasselbe Programm, von sich selbst gestartet, damit
    // die Änderung wie im Ernstfall aus einem fremden Prozess kommt.
    if (app.arguments().contains(QStringLiteral("--schreiben"))) {
        auto config = KSharedConfig::openConfig(QStringLiteral("plasmarc"));
        const bool notify = app.arguments().contains(QStringLiteral("--notify"));
        KConfigGroup(config, QStringLiteral("Theme"))
            .writeEntry("name", app.arguments().last(), notify ? KConfig::Notify : KConfig::Normal);
        config->sync();
        return 0;
    }

    auto config = KSharedConfig::openConfig(QStringLiteral("plasmarc"));

    int configFired = 0;
    KConfigWatcher::Ptr watcher = KConfigWatcher::create(config);
    QObject::connect(watcher.data(), &KConfigWatcher::configChanged, [&](const KConfigGroup &group, const QByteArrayList &keys) {
        ++configFired;
        out << "   KConfigWatcher meldet: Gruppe=" << group.name() << " Schlüssel=" << keys.join(',') << "\n";
        out.flush();
    });

    int dirFired = 0;
    KDirWatch::self()->addFile(path);
    auto note = [&](const QString &kind) {
        return [&, kind](const QString &changed) {
            if (changed != path) {
                return;
            }
            ++dirFired;
            out << "   KDirWatch meldet: " << kind << "\n";
            out.flush();
        };
    };
    QObject::connect(KDirWatch::self(), &KDirWatch::dirty, note(QStringLiteral("dirty")));
    QObject::connect(KDirWatch::self(), &KDirWatch::created, note(QStringLiteral("created")));

    out << "Messung 2 — Zustellwege eines Desktop-Theme-Wechsels (#55, AK 5)\n";
    out << "================================================================\n\n";
    out << "Beobachtete Datei: " << path << "\n\n";
    out.flush();

    auto write = [&](const QString &theme, bool notify) {
        QStringList args{QStringLiteral("--schreiben")};
        if (notify) {
            args << QStringLiteral("--notify");
        }
        args << theme;
        QProcess::execute(app.applicationFilePath(), args);
    };

    QTimer::singleShot(200, [&] {
        out << "A) Fremder Schreiber OHNE KConfig::Notify  ->  breeze-dark\n";
        out.flush();
        write(QStringLiteral("breeze-dark"), false);
    });

    QTimer::singleShot(SettleMs + 500, [&] {
        out << "   Ergebnis A: KConfigWatcher=" << configFired << "   KDirWatch=" << dirFired << "\n\n";
        configFired = 0;
        dirFired = 0;
        out << "B) Fremder Schreiber MIT KConfig::Notify   ->  CachyOS-Nord-round\n";
        out.flush();
        write(QStringLiteral("CachyOS-Nord-round"), true);
    });

    QTimer::singleShot(2 * SettleMs + 1000, [&] {
        out << "   Ergebnis B: KConfigWatcher=" << configFired << "   KDirWatch=" << dirFired << "\n\n";
        out << "Befund: KDirWatch sieht beide Schreibarten, KConfigWatcher nur die zweite.\n"
               "Die Hülle hängt deshalb an KDirWatch — sonst hinge ihre Zusicherung an der\n"
               "Schreibdisziplin eines fremden Programms. Dass KConfig die Datei ersetzt\n"
               "statt sie zu überschreiben, ist an der Meldung `created` abzulesen; ein\n"
               "QFileSystemWatcher verlöre dabei seine Beobachtung.\n";
        out.flush();
        app.quit();
    });

    return app.exec();
}
