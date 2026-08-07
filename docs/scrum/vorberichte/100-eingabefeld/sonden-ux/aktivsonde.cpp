/*
 * Fokuszustand des Textfeldes (#100), Messung 5: ob der Zustand „Fenster offen,
 * aber nicht aktiv" offscreen überhaupt prüfbar ist.
 *
 * SPEC 3 legt fest: „Fokusverlust: Fenster bleibt (kein Datenverlust durch
 * versehentlichen Klick daneben)." Der Zustand existiert also und ist gewollt.
 * Eine Fokusschicht, die allein am Widget-Fokus hängt, zeichnete darin einen
 * Fokusrahmen an einem Fenster, das der Compositor gerade nicht bedient —
 * Plasmas eigener Bau hängt sie an `activeFocus`, was in Qt Widgets
 * `hasFocus() && isActiveWindow()` entspricht.
 *
 * Gemessen wird mit gewöhnlichen Qt-Mitteln, ohne die Projektbibliothek: Was
 * melden `hasFocus()` und `isActiveWindow()` offscreen, und trifft ein
 * `ActivationChange` ein, wenn ein zweites Fenster aufgeht?
 *
 * Aufruf: aktivsonde
 */
#include <QApplication>
#include <QEvent>
#include <QPlainTextEdit>
#include <QTextStream>
#include <QWidget>

namespace
{
class Wache : public QWidget
{
public:
    int aktivierungen = 0;

protected:
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::ActivationChange) {
            ++aktivierungen;
        }
        QWidget::changeEvent(event);
    }
};
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    Wache fenster;
    auto *text = new QPlainTextEdit(&fenster);
    fenster.setFocusProxy(text);
    fenster.resize(600, 174);
    fenster.show();
    text->setFocus();
    qApp->processEvents();

    out << "Nach show() und setFocus():\n";
    out << "  text->hasFocus()          " << (text->hasFocus() ? "true" : "false") << "\n";
    out << "  fenster.isActiveWindow()  " << (fenster.isActiveWindow() ? "true" : "false") << "\n";
    out << "  ActivationChange bisher   " << fenster.aktivierungen << "\n\n";

    QWidget zweites;
    zweites.resize(400, 200);
    zweites.show();
    zweites.activateWindow();
    qApp->processEvents();

    out << "Nachdem ein zweites Fenster aufgegangen und aktiviert ist:\n";
    out << "  text->hasFocus()          " << (text->hasFocus() ? "true" : "false") << "\n";
    out << "  fenster.isActiveWindow()  " << (fenster.isActiveWindow() ? "true" : "false") << "\n";
    out << "  ActivationChange gesamt   " << fenster.aktivierungen << "\n";
    out << "  QApplication::activeWindow() ist das erste Fenster: "
        << (QApplication::activeWindow() == &fenster ? "ja" : "nein") << "\n";

    return 0;
}
