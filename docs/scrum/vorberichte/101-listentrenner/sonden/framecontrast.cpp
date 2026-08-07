/*
 * Sonde zur Vorprüfung von #101, Messung B (07.08.2026).
 *
 * AK 4 verlangt die Linienfarbe als Mischung aus Listengrund und Textfarbe im
 * Verhältnis `frameContrast` des Schemas. `frameContrast` ist keine
 * Palettenrolle, sondern ein Konfigurationswert (`[General]` in kdeglobals) —
 * die Frage ist also, ob ein Test ihn setzen kann, ohne von den Schemadateien
 * der Maschine abzuhängen.
 *
 * Prüffrage 1: Was liest `KSharedConfig::openConfig()` im Testmodus, wenn dort
 *   keine kdeglobals liegt?
 * Prüffrage 2: Kommt ein **zweiter** Wert im selben Lauf an? AK 4 verlangt zwei
 *   Schemata, also genau diesen Fall.
 * Prüffrage 3: Ändert sich das, wenn zwischen den Schreibvorgängen eine
 *   Sekunde vergeht?
 *
 * Der Testmodus ist derselbe, den `librarytest.cpp:364` bereits setzt.
 */

#include <KConfigGroup>
#include <KSharedConfig>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>

namespace
{
QString g_globals;

double readFrameContrast()
{
    return KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("General"))
        .readEntry("frameContrast", 0.20);
}

void write(const QString &value)
{
    QFile file(g_globals);
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream stream(&file);
    stream << "[General]\nframeContrast=" << value << "\n";
    stream.flush();
    file.close();
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
    QTextStream out(stdout);

    QStandardPaths::setTestModeEnabled(true);
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QDir().mkpath(configDir);
    g_globals = configDir + QStringLiteral("/kdeglobals");
    QFile::remove(g_globals);
    KSharedConfig::openConfig()->reparseConfiguration();

    out << "## Prüffrage 1 — Testmodus ohne kdeglobals\n";
    out << QStringLiteral("kdeglobals vorhanden: %1\n")
               .arg(QFile::exists(g_globals) ? QStringLiteral("ja") : QStringLiteral("nein"));
    out << QStringLiteral("gelesener frameContrast: %1 (Vorgabewert 0.20)\n\n").arg(readFrameContrast());

    out << "## Prüffrage 2 — vier Werte hintereinander, ohne Pause\n";
    for (const QString &value : {QStringLiteral("0.85"), QStringLiteral("0.05"), QStringLiteral("0.5"),
                                 QStringLiteral("0.33")}) {
        write(value);
        KSharedConfig::openConfig()->reparseConfiguration();
        const double read = readFrameContrast();
        out << QStringLiteral("geschrieben %1 → gelesen %2   %3\n")
                   .arg(value)
                   .arg(read)
                   .arg(qFuzzyCompare(read, value.toDouble()) ? QStringLiteral("passt")
                                                              : QStringLiteral("WEICHT AB"));
    }

    out << "\n## Prüffrage 3 — dieselben vier Werte, je 1100 ms Pause vor dem Lesen\n";
    for (const QString &value : {QStringLiteral("0.85"), QStringLiteral("0.05"), QStringLiteral("0.5"),
                                 QStringLiteral("0.33")}) {
        write(value);
        QThread::msleep(1100);
        KSharedConfig::openConfig()->reparseConfiguration();
        const double read = readFrameContrast();
        out << QStringLiteral("geschrieben %1 → gelesen %2   %3\n")
                   .arg(value)
                   .arg(read)
                   .arg(qFuzzyCompare(read, value.toDouble()) ? QStringLiteral("passt")
                                                              : QStringLiteral("WEICHT AB"));
    }

    out << "\n## Prüffrage 4 — derselbe Wechsel über eine eigene Konfiguration auf den Pfad\n";
    for (const QString &value : {QStringLiteral("0.85"), QStringLiteral("0.05"), QStringLiteral("0.5")}) {
        write(value);
        KSharedConfigPtr fresh = KSharedConfig::openConfig(g_globals, KConfig::SimpleConfig);
        fresh->reparseConfiguration();
        const double read = KConfigGroup(fresh, QStringLiteral("General")).readEntry("frameContrast", 0.20);
        out << QStringLiteral("geschrieben %1 → gelesen %2   %3\n")
                   .arg(value)
                   .arg(read)
                   .arg(qFuzzyCompare(read, value.toDouble()) ? QStringLiteral("passt")
                                                              : QStringLiteral("WEICHT AB"));
    }

    QFile::remove(g_globals);
    out.flush();
    return 0;
}
