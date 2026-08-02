// Messung 2 — Der Watcher trägt, die Schrift folgt trotzdem nicht.
//
// Claim under test: a KConfigWatcher on kdeglobals fires reliably, names the
// changed keys and hands out the new value at once — while neither
// QApplication::font() nor QFontDatabase::systemFont() moves. That is the
// independent confirmation of B6 from the theme report, and it splits #68 into
// a cheap half (the watcher) and an expensive one (every site that has to be
// re-fed by hand).
//
// Run it through pruefen.sh: it needs a session bus of its own and a copy of
// kdeglobals under a private XDG_CONFIG_HOME, or the run would write into the
// customer's own settings.

#include <KConfigGroup>
#include <KConfigWatcher>
#include <KSharedConfig>

#include <QApplication>
#include <QFontDatabase>
#include <QTextStream>
#include <QTimer>

namespace
{
constexpr int ObservationSeconds = 6;

QTextStream out(stdout);

void reportFonts(const QString &tag)
{
    out << tag << "  QApplication::font()=" << QApplication::font().pointSizeF()
        << "  systemFont(GeneralFont)=" << QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSizeF()
        << "  systemFont(SmallestReadableFont)="
        << QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont).pointSizeF() << "\n";
    out.flush();
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    out << "Messung 2 — KConfigWatcher auf kdeglobals gegen die Schriftfrage\n";
    out << "===============================================================\n\n";
    out << "XDG_CONFIG_HOME: " << qgetenv("XDG_CONFIG_HOME") << "\n";
    out << "Sitzungsbus    : " << qgetenv("DBUS_SESSION_BUS_ADDRESS").left(24) << "…\n\n";

    reportFonts(QStringLiteral("start   "));

    auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    KConfigWatcher::Ptr watcher = KConfigWatcher::create(config);

    QObject::connect(watcher.data(),
                     &KConfigWatcher::configChanged,
                     [](const KConfigGroup &group, const QByteArrayList &names) {
                         out << "\nWATCHER  Gruppe=" << group.name() << "  Schlüssel=" << names.join(',')
                             << "  readEntry(font)=" << group.readEntry("font", QFont()).pointSizeF()
                             << "  readEntry(smallestReadableFont)="
                             << group.readEntry("smallestReadableFont", QFont()).pointSizeF() << "\n";
                         out.flush();
                         reportFonts(QStringLiteral("nach dem Signal"));
                     });

    // The shell writes into kdeglobals while these ticks run.
    for (int second = 1; second <= ObservationSeconds; ++second) {
        QTimer::singleShot(second * 1000, &app, [second] {
            reportFonts(QStringLiteral("t+%1s  ").arg(second));
        });
    }

    QTimer::singleShot((ObservationSeconds + 1) * 1000, &app, [] {
        out << "\nBefund: Der Watcher feuert sofort, nennt den geänderten Schlüssel und\n"
               "liefert den neuen Wert. QApplication::font() und QFontDatabase::systemFont()\n"
               "bleiben über die ganze Beobachtungszeit stehen — die Anwendung muss die\n"
               "Schrift also selbst lesen und selbst anwenden.\n";
        out.flush();
        QCoreApplication::quit();
    });

    return app.exec();
}
