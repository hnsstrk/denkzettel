/**
 * Messsonde zu Kundenbefund B3 — „Die Schriftfarben des Themes werden nicht
 * übernommen."
 *
 * Der Kunde hat Denkzettel neben Dolphin unter seinem Farbschema fotografiert:
 * Dolphins Text ist bläulich, Denkzettels grau. Die Sonde misst deshalb nicht
 * Eindrücke, sondern drei Zahlenpaare unter **demselben** Farbschema:
 *
 *   Fläche          gezeichnetes Pixel  gegen  QPalette::Window
 *   gedämpfte Texte gezeichnetes Pixel  gegen  QPalette::PlaceholderText
 *   Notiztext       gezeichnetes Pixel  gegen  QPalette::WindowText
 *
 * Das Farbschema kommt aus einem **eigenen** XDG_CONFIG_HOME, das `pruefen.sh`
 * anlegt; das Schema des Kunden wird gelesen, nie geschrieben.
 *
 * Aufruf: QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde \
 *         flaechenfarbe <Zielverzeichnis>
 */

#include "capture/capturewindow.h"
#include "store/store.h"

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTextStream>

namespace {

QString rgb(const QColor &c)
{
    return QStringLiteral("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue());
}

/**
 * Das hellste bzw. dunkelste Pixel eines Ausschnitts — der Kern eines
 * Buchstabens, unverfälscht von der Kantenglättung an seinem Rand.
 */
QColor extreme(const QImage &image, const QRect &area, bool brightest)
{
    QColor best;
    int bestValue = brightest ? -1 : 1 << 20;
    for (int y = area.top(); y <= area.bottom() && y < image.height(); ++y) {
        for (int x = area.left(); x <= area.right() && x < image.width(); ++x) {
            const QColor c = image.pixelColor(x, y);
            const int value = c.red() + c.green() + c.blue();
            if (brightest ? (value > bestValue) : (value < bestValue)) {
                bestValue = value;
                best = c;
            }
        }
    }
    return best;
}

void compare(QTextStream &out, const QString &what, const QColor &drawn, const QColor &expected)
{
    const bool same = drawn.rgb() == expected.rgb();
    out << QStringLiteral("  %1\n      gezeichnet %2   Palette %3   %4\n")
               .arg(what, -46)
               .arg(rgb(drawn), -12)
               .arg(rgb(expected), -12)
               .arg(same ? QStringLiteral("GLEICH") : QStringLiteral("ABWEICHUNG"));
}

} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QTextStream out(stdout);
    const QString directory = app.arguments().value(1, QStringLiteral("."));
    QDir().mkpath(directory);

    const QPalette palette = app.palette();
    out << "=== Flächenfarbe ===\n";
    out << "XDG_CONFIG_HOME  : " << qEnvironmentVariable("XDG_CONFIG_HOME", QStringLiteral("(nicht gesetzt)")) << "\n";
    out << "Plattform-Thema  : " << qEnvironmentVariable("QT_QPA_PLATFORMTHEME", QStringLiteral("(nicht gesetzt)")) << "\n\n";
    out << "Palette der Anwendung\n";
    out << "  Window          : " << rgb(palette.color(QPalette::Window)) << "\n";
    out << "  WindowText      : " << rgb(palette.color(QPalette::WindowText)) << "\n";
    out << "  PlaceholderText : " << rgb(palette.color(QPalette::PlaceholderText)) << "\n";
    out << "  Base            : " << rgb(palette.color(QPalette::Base)) << "\n";
    out << "  Text            : " << rgb(palette.color(QPalette::Text)) << "\n\n";

    const QTemporaryDir tmp;
    Store store(tmp.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen\n";
        return 1;
    }

    CaptureWindow window(&store);
    auto *text = window.findChild<QPlainTextEdit *>();
    text->setPlainText(QStringLiteral("Notiztext"));
    window.show();
    QCoreApplication::processEvents();

    const QImage shot = window.grab().toImage();
    const qreal dpr = shot.devicePixelRatio();
    const auto dev = [dpr](int x, int y, int w, int h) {
        return QRect(qRound(x * dpr), qRound(y * dpr), qRound(w * dpr), qRound(h * dpr));
    };

    // Ist das Farbschema hell oder dunkel? Danach richtet sich, ob der Kern
    // eines Buchstabens das dunkelste oder das hellste Pixel ist.
    const QColor window0 = palette.color(QPalette::Window);
    const bool dark = window0.red() + window0.green() + window0.blue() < 3 * 128;

    out << "Gemessen am gezeichneten Fenster (" << shot.width() << "x" << shot.height()
        << " Gerätepixel, DPR " << dpr << ")\n";

    // Fläche: ein Streifen rechts oben, in dem kein Text steht.
    const QColor surface = shot.pixelColor(qRound((window.width() - 40) * dpr), qRound(20 * dpr));
    compare(out, QStringLiteral("Fläche (rechts oben, textfrei)"), surface, palette.color(QPalette::Window));

    // Anwendungsname „Denkzettel" — Rolle PlaceholderText laut Zeichnung 4b.
    compare(out,
            QStringLiteral("Anwendungsname „Denkzettel\" (PlaceholderText)"),
            extreme(shot, dev(14, 8, 200, 26), dark),
            palette.color(QPalette::PlaceholderText));

    // Der eingetippte Notiztext — Rolle WindowText, siehe applyTextColours().
    //
    // Hier zählt nicht das hellste Pixel, sondern ob die Palettenfarbe **vorkommt**:
    // Im Textbereich steht auch die Schreibmarke, und die malt Qt als Umkehrung
    // des Grundes (255 minus Fläche), nicht in der Textfarbe. Ein Extremwert
    // fände sie und nicht den Text.
    const QRect textArea = dev(14, 36, 260, 30);
    const QColor windowText = palette.color(QPalette::WindowText);
    int hits = 0;
    for (int y = textArea.top(); y <= textArea.bottom() && y < shot.height(); ++y) {
        for (int x = textArea.left(); x <= textArea.right() && x < shot.width(); ++x) {
            if (shot.pixelColor(x, y).rgb() == windowText.rgb()) {
                ++hits;
            }
        }
    }
    out << QStringLiteral("  %1\n      Palette %2   im Textbereich %3-mal getroffen   %4\n")
               .arg(QStringLiteral("Notiztext (WindowText)"), -46)
               .arg(rgb(windowText), -12)
               .arg(hits)
               .arg(hits > 0 ? QStringLiteral("VORHANDEN") : QStringLiteral("FEHLT"));
    out << QStringLiteral("      Schreibmarke im selben Bereich: %1  (Umkehrung der Fläche %2)\n")
               .arg(rgb(extreme(shot, textArea, dark)), -12)
               .arg(rgb(QColor(255 - window0.red(), 255 - window0.green(), 255 - window0.blue())));

    // Fußzeile — ebenfalls PlaceholderText.
    compare(out,
            QStringLiteral("Fußzeile (PlaceholderText)"),
            extreme(shot, dev(120, window.height() - 32, 360, 26), dark),
            palette.color(QPalette::PlaceholderText));

    shot.save(QDir(directory).filePath(QStringLiteral("flaechenfarbe.png")));
    out << "\nBild: flaechenfarbe.png\n";
    return 0;
}
