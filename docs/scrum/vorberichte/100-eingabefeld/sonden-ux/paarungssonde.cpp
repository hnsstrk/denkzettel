/*
 * UX-Entscheidung zur Textfarbe (#100), Messung 2: die vier Paarungen von
 * Vordergrund und Grund, über alle installierten Farbschemata.
 *
 * Die Messung der Vorprüfung (m3-schriftgrund-18-schemata.txt) druckt drei der
 * vier Spalten aus; die vierte — Fensterrolle auf der Ansichtsfläche — steht
 * dort nur als Zusammenzug. Für die Entscheidung wird sie je Schema gebraucht,
 * weil sie einer der beiden Fälle ist, in denen die gewählte Rolle nicht zum
 * gezeichneten Grund gehört.
 *
 * Die Trennung ist der Punkt der Messung:
 *
 *   WindowText auf Window  — ein Paar aus einem Farbsatz
 *   Text       auf View    — ein Paar aus einem Farbsatz
 *   Text       auf Window  — gemischt
 *   WindowText auf View    — gemischt
 *
 * KColorScheme sagt zu den Farbsätzen: „Colors from different sets should not
 * be combined." (`/usr/include/KF6/KColorScheme/kcolorscheme.h`, Zeile 65).
 * Ob das für die hier installierten Schemata eine messbare Folge hat, sagt die
 * Ausgabe.
 *
 * Aufruf: paarungssonde <Pfad zur .colors> …
 */

#include <KColorScheme>
#include <KSharedConfig>

#include <QApplication>
#include <QFileInfo>
#include <QTextStream>

#include <cmath>

namespace
{
double channel(int c)
{
    const double v = c / 255.0;
    return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
}

double luminance(const QColor &c)
{
    return 0.2126 * channel(c.red()) + 0.7152 * channel(c.green()) + 0.0722 * channel(c.blue());
}

double contrast(const QColor &a, const QColor &b)
{
    const double la = luminance(a);
    const double lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    out << QStringLiteral("%1 %2 %3 %4 %5\n")
               .arg(QStringLiteral("Schema"), -24)
               .arg(QStringLiteral("WT:Window"), 10)
               .arg(QStringLiteral("Text:View"), 10)
               .arg(QStringLiteral("Text:Window"), 12)
               .arg(QStringLiteral("WT:View"), 10);

    QList<double> reinWindow;
    QList<double> reinView;
    QList<double> mischTextWindow;
    QList<double> mischWtView;

    for (int i = 1; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        KSharedConfigPtr scheme = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
        const QPalette p = KColorScheme::createApplicationPalette(scheme);

        const QColor window = p.color(QPalette::Active, QPalette::Window);
        const QColor windowText = p.color(QPalette::Active, QPalette::WindowText);
        const QColor view = p.color(QPalette::Active, QPalette::Base);
        const QColor text = p.color(QPalette::Active, QPalette::Text);

        const double a = contrast(windowText, window);
        const double b = contrast(text, view);
        const double c = contrast(text, window);
        const double d = contrast(windowText, view);

        reinWindow << a;
        reinView << b;
        mischTextWindow << c;
        mischWtView << d;

        out << QStringLiteral("%1 %2 %3 %4 %5\n")
                   .arg(QFileInfo(path).baseName(), -24)
                   .arg(a, 10, 'f', 2)
                   .arg(b, 10, 'f', 2)
                   .arg(c, 12, 'f', 2)
                   .arg(d, 10, 'f', 2);
    }

    auto zusammenzug = [&out](const QString &name, const QList<double> &v) {
        double schlechtest = v.first();
        int unter45 = 0;
        for (const double x : v) {
            schlechtest = std::min(schlechtest, x);
            if (x < 4.5) {
                ++unter45;
            }
        }
        out << QStringLiteral("%1 schlechtester Fall %2 : 1   unter 4,5 : 1 -> %3 von %4\n")
                   .arg(name, -28)
                   .arg(schlechtest, 6, 'f', 2)
                   .arg(unter45)
                   .arg(v.size());
    };

    out << "\n";
    zusammenzug(QStringLiteral("WindowText auf Window (rein)"), reinWindow);
    zusammenzug(QStringLiteral("Text auf View (rein)"), reinView);
    zusammenzug(QStringLiteral("Text auf Window (gemischt)"), mischTextWindow);
    zusammenzug(QStringLiteral("WindowText auf View (gemischt)"), mischWtView);

    return 0;
}
