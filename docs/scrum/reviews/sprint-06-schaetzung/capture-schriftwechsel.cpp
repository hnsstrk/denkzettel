// Messung 3 — Der Befund von #56 am echten Fenster, und wo seine Heilung hingehört.
//
// Two claims under test, both about CaptureWindow as it stands:
//
//  A) the text field keeps the height it was built with while the line spacing
//     grows underneath it — the five lines of SPEC 3 fall to three and below;
//
//  B) of the two places the fix could sit, only one sees every font change:
//     the eventFilter already installed on m_text (capturewindow.cpp:52) is
//     reached on all three roads, an overridden changeEvent on the window
//     misses the one that AK 2 prescribes for the test.
//
// Claim B is the one that saves the next strand an attempt against a red test
// that says nothing about the matter.

#include "capture/capturewindow.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QApplication>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTextStream>

namespace
{
QTextStream out(stdout);

/** Counts FontChange at the two places the fix of #56 could be hooked to. */
class FontChangeCounter : public QObject
{
public:
    QWidget *window = nullptr;
    QWidget *field = nullptr;
    int atWindow = 0;
    int atField = 0;

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::FontChange) {
            atWindow += (watched == window) ? 1 : 0;
            atField += (watched == field) ? 1 : 0;
        }

        return QObject::eventFilter(watched, event);
    }
};

void reportHeight(const QString &tag, QWidget *window, QPlainTextEdit *field)
{
    const int lineSpacing = field->fontMetrics().lineSpacing();
    out << tag << "  Zeilenabstand=" << lineSpacing << " px"
        << "  Feldhöhe=" << field->height() << " px"
        << "  entspricht " << QString::number(double(field->height()) / lineSpacing, 'f', 2) << " Zeilen"
        << "  Fensterhöhe=" << window->height() << " px\n";
    out.flush();
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));

    QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store ließ sich nicht öffnen: " << store.lastError() << "\n";
        return 1;
    }

    out << "Messung 3 — Feldhöhe und Zustellwege einer Schriftänderung (#56)\n";
    out << "================================================================\n\n";
    out << "A) Die Feldhöhe folgt der Schrift nicht\n\n";

    CaptureWindow window(&store);
    auto *field = window.findChild<QPlainTextEdit *>();

    FontChangeCounter counter;
    counter.window = &window;
    counter.field = field;
    window.installEventFilter(&counter);
    field->installEventFilter(&counter);

    auto step = [&](const QString &tag) {
        reportHeight(tag, &window, field);
        out << "        FontChange an CaptureWindow=" << counter.atWindow
            << "  an QPlainTextEdit=" << counter.atField << "\n";
        counter.atWindow = 0;
        counter.atField = 0;
        out.flush();
    };

    reportHeight(QStringLiteral("Start                 "), &window, field);

    // Road A — the way #68 would deliver a changed system font.
    QFont viaApplication = app.font();
    viaApplication.setPointSizeF(16);
    app.setFont(viaApplication);
    QCoreApplication::processEvents();
    step(QStringLiteral("qApp->setFont(16pt)   "));

    // Road B — the font set on the window itself.
    QFont viaWindow = window.font();
    viaWindow.setPointSizeF(24);
    window.setFont(viaWindow);
    QCoreApplication::processEvents();
    step(QStringLiteral("window.setFont(24pt)  "));

    // Road C — the font set on the text area alone. This is what AK 2 of #56
    // prescribes: "Der Test setzt die Schrift des Widgets direkt".
    QFont viaField = field->font();
    viaField.setPointSizeF(32);
    field->setFont(viaField);
    QCoreApplication::processEvents();
    step(QStringLiteral("field.setFont(32pt)   "));

    // What a window built now — at the application font of 16 pt — would have.
    CaptureWindow fresh(&store);
    reportHeight(QStringLiteral("frisch gebaut @16pt   "), &fresh, fresh.findChild<QPlainTextEdit *>());

    out << "\nB) Befund zu den Zustellwegen\n\n"
           "Die Schrift kann auf drei Wegen ankommen. Der eventFilter, der ohnehin auf\n"
           "m_text liegt (capturewindow.cpp:52), sieht alle drei; ein changeEvent am\n"
           "Fenster sieht Weg C nicht — und Weg C ist der, den AK 2 dem Test vorschreibt.\n"
           "Wer die Heilung ins changeEvent des Fensters legt und den Test nach AK 2\n"
           "schreibt, bekommt einen roten Test, der nichts über die Sache sagt.\n";

    out.flush();
    return 0;
}
