// Messsonde zur Vorprüfung von Issue #101: Woher bekommt der Delegate den
// Wert `frameContrast`, und was kostet das?
//
// AK 4 verlangt die Linienfarbe als Mischung aus Listengrund und Textfarbe im
// Verhältnis `frameContrast` des Schemas. Gemessen wird:
//
//   1. Was `KColorScheme::frameContrast()` ohne Konfiguration liefert — also
//      das, was der Delegate im laufenden Programm bekäme.
//   2. Was dieselbe Funktion mit einer ausdrücklich geöffneten Schema-Datei
//      liefert, und ob das mit dem `KConfigGroup`-Weg der Messung vom
//      06.08.2026 übereinstimmt.
//   3. Ob `app.setPalette(KColorScheme::createApplicationPalette(schema))` —
//      der Weg, den ein Test naheliegenderweise nimmt — den Wert der
//      Konfigurations-Abfrage MITZIEHT. Zieht er ihn nicht mit, prüft ein
//      solcher Test die Farbe des falschen Schemas.
//
// Aufruf: farbe <Pfad zur .colors> …

#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <cmath>
#include <cstdio>

namespace
{
double luminance(const QColor &color)
{
    auto channel = [](double value) {
        return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * channel(color.redF()) + 0.7152 * channel(color.greenF()) + 0.0722 * channel(color.blueF());
}

double contrast(const QColor &a, const QColor &b)
{
    const double first = luminance(a);
    const double second = luminance(b);
    return (std::max(first, second) + 0.05) / (std::min(first, second) + 0.05);
}

QColor mixed(const QColor &ground, const QColor &text, double share)
{
    return QColor::fromRgbF(ground.redF() * (1 - share) + text.redF() * share,
                            ground.greenF() * (1 - share) + text.greenF() * share,
                            ground.blueF() * (1 - share) + text.blueF() * share);
}
}

int main(int argc, char **argv)
{
    QTemporaryDir sandbox;
    qputenv("XDG_DATA_HOME", sandbox.filePath(QStringLiteral("data")).toUtf8());
    qputenv("XDG_CONFIG_HOME", sandbox.filePath(QStringLiteral("config")).toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    QApplication app(argc, argv);

    printf("KColorScheme::frameContrast() ohne Konfiguration: %.4f\n",
           KColorScheme::frameContrast());
    printf("(Das ist der Wert, den der Delegate im laufenden Programm bekäme,\n"
           " hier allerdings in einem leeren XDG-Sandkasten.)\n\n");

    printf("%-24s %8s %8s %8s   %-9s %s\n", "Schema", "KConfig", "KColorSch", "nach set",
           "Linie", "Kontrast");
    printf("%s\n", QByteArray(84, '-').constData());

    int drift = 0;
    for (int i = 1; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);

        const double byConfig =
            KConfigGroup(scheme, QStringLiteral("General")).readEntry("frameContrast", 0.20);
        const double byApi = KColorScheme::frameContrast(scheme);

        const QPalette palette = KColorScheme::createApplicationPalette(scheme);
        app.setPalette(palette);
        // Nach dem Setzen der Palette: Was liefert die Abfrage OHNE Schema,
        // also die, die der Delegate im Programm führt?
        const double afterSet = KColorScheme::frameContrast();
        if (std::abs(afterSet - byConfig) > 0.0005) {
            ++drift;
        }

        const QColor base = palette.color(QPalette::Active, QPalette::Base);
        const QColor text = palette.color(QPalette::Active, QPalette::Text);
        const QColor line = mixed(base, text, byApi);

        printf("%-24s %8.4f %8.4f %8.4f   %-9s %5.2f:1\n", qPrintable(QFileInfo(path).baseName()),
               byConfig, byApi, afterSet, qPrintable(line.name()), contrast(line, base));
    }

    printf("%s\n", QByteArray(84, '-').constData());
    printf("Schemata, bei denen die Abfrage OHNE Schema dem gesetzten Schema NICHT folgt: %d von %d\n",
           drift, argc - 1);
    printf("ACHTUNG: Diese Zeile trägt nur, wenn die Schemata überhaupt verschiedene Werte\n"
           "tragen. Führen alle denselben, ist sie leer richtig — deshalb der Gegenversuch:\n\n");

    // Gegenversuch mit einem ausgedachten Wert. Nur er kann zeigen, ob die
    // Abfrage ohne Schema irgendetwas anderes liefert als die eingebaute 0,20.
    printf("== Gegenversuch: ein Schema mit abweichendem frameContrast ==\n");
    const QString made = sandbox.filePath(QStringLiteral("Erfunden.colors"));
    {
        KSharedConfigPtr fake = KSharedConfig::openConfig(made, KConfig::SimpleConfig);
        KConfigGroup(fake, QStringLiteral("General")).writeEntry("frameContrast", 0.45);
        KConfigGroup(fake, QStringLiteral("Colors:View")).writeEntry("BackgroundNormal", QColor(Qt::white));
        KConfigGroup(fake, QStringLiteral("Colors:View")).writeEntry("ForegroundNormal", QColor(Qt::black));
        fake->sync();
    }
    KSharedConfigPtr fake = KSharedConfig::openConfig(made, KConfig::SimpleConfig);
    printf("frameContrast(Schemadatei)          : %.4f  (geschrieben: 0,4500)\n",
           KColorScheme::frameContrast(fake));
    app.setPalette(KColorScheme::createApplicationPalette(fake));
    printf("frameContrast() nach setPalette     : %.4f\n", KColorScheme::frameContrast());
    printf("→ Eine gesetzte Palette zieht den Wert %s mit.\n",
           std::abs(KColorScheme::frameContrast() - 0.45) < 0.0005 ? "" : "NICHT");

    // Und der Weg, den Plasma geht: der Wert steht in kdeglobals.
    {
        KSharedConfigPtr globals = KSharedConfig::openConfig();
        KConfigGroup(globals, QStringLiteral("General")).writeEntry("frameContrast", 0.35);
        globals->sync();
        globals->reparseConfiguration();
    }
    printf("frameContrast() nach Eintrag in kdeglobals (0,3500): %.4f\n",
           KColorScheme::frameContrast());

    return 0;
}
