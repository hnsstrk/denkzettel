// UX-Prüfsonde Sprint 6 — misst die Maße aus Wireframe 4b am gebauten Fenster.
// Liest nur; ändert nichts am Produkt. Läuft offscreen.
#include "capture/capturewindow.h"
#include "desktopthemes.h"
#include "store/store.h"

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTextStream>

static void report(QTextStream &out, const QString &theme, Store &store)
{
    CaptureWindow window(&store);
    window.reloadDesktopTheme(theme);
    window.show();

    auto labels = window.findChildren<QLabel *>();
    QLabel *appName = labels.value(0);
    QLabel *footer = labels.value(1);
    auto *text = window.findChild<QPlainTextEdit *>();

    const QMargins m = window.layout()->contentsMargins();

    out << "--- Theme: " << theme << "\n";
    out << "Fenster           : " << window.width() << " x " << window.height() << "\n";
    out << "Layout-Innenrand  : links " << m.left() << " oben " << m.top()
        << " rechts " << m.right() << " unten " << m.bottom() << "\n";
    out << "App-Name          : x " << appName->x() << " y " << appName->y()
        << "  " << appName->width() << "x" << appName->height()
        << "  Text=\"" << appName->text() << "\"  Rolle=" << int(appName->foregroundRole()) << "\n";
    out << "Textfeld          : x " << text->x() << " y " << text->y()
        << "  " << text->width() << "x" << text->height()
        << "  Dokumentrand=" << text->document()->documentMargin()
        << "  Rahmen=" << text->frameWidth() << "\n";
    out << "Fusszeile         : x " << footer->x() << " y " << footer->y()
        << "  " << footer->width() << "x" << footer->height()
        << "  Ausrichtung=" << int(footer->alignment())
        << "  Text=\"" << footer->text() << "\"\n";
    out << "Abstand App->Feld : " << text->y() - (appName->y() + appName->height()) << "\n";
    out << "Abstand Feld->Fuss: " << footer->y() - (text->y() + text->height()) << "\n";
    out << "Fuss->Unterkante  : " << window.height() - (footer->y() + footer->height()) << "\n";
    out << "Zeilenabstand     : " << text->fontMetrics().lineSpacing()
        << "  Feldhoehe/Zeilen = "
        << qreal(text->height() - 2 * qRound(text->document()->documentMargin()) - 2 * text->frameWidth())
            / text->fontMetrics().lineSpacing()
        << "\n";
    out << "Schriften         : Notiz " << text->font().pointSizeF() << "pt"
        << "  App-Name " << appName->font().pointSizeF() << "pt"
        << "  Fuss " << footer->font().pointSizeF() << "pt\n";
    out << "Notiztextfarbe    : " << text->palette().color(QPalette::Text).name()
        << "  Fensterrolle WindowText " << window.palette().color(QPalette::WindowText).name()
        << "  Feld-Base " << text->palette().color(QPalette::Base).name()
        << " (alpha " << text->palette().color(QPalette::Base).alpha() << ")\n";
    out << "Kleintextrolle    : App " << int(appName->foregroundRole())
        << " Fuss " << int(footer->foregroundRole())
        << "  (PlaceholderText=" << int(QPalette::PlaceholderText) << ")\n";
    out << "Viewport autofill : " << text->viewport()->autoFillBackground() << "\n";

    // Acht Zeilen: waechst das Fenster, kommt der Scrollbalken?
    text->setPlainText(QStringLiteral("1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12"));
    out << "bei 12 Zeilen Text: Fenster " << window.width() << "x" << window.height()
        << "  Feld " << text->height()
        << "  Zeilen = " << qreal(text->height() - 2 * qRound(text->document()->documentMargin())
                                  - 2 * text->frameWidth()) / text->fontMetrics().lineSpacing()
        << "  Scrollbalken sichtbar=" << text->verticalScrollBar()->isVisible() << "\n";
    text->clear();
    out << "nach clear        : Fenster " << window.width() << "x" << window.height() << "\n";
    out << "\n";
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    const QTemporaryDir dir;
    Store store(dir.filePath(QStringLiteral("denkzettel.db")));
    if (!store.open()) {
        out << "Store fehlgeschlagen\n";
        return 1;
    }

    themes::addBundledThemesToDataPath();
    const auto pair = themes::installedThemePair();
    if (!pair) {
        out << "kein installiertes Themepaar\n";
        return 1;
    }
    out << "Themepaar: schmal=" << pair->first << " breit=" << pair->second << "\n\n";

    report(out, pair->first, store);
    report(out, pair->second, store);

    // Schriftwechsel (#56) am stehenden Fenster
    CaptureWindow window(&store);
    window.reloadDesktopTheme(pair->first);
    window.show();
    auto *text = window.findChild<QPlainTextEdit *>();
    for (int pt : {9, 16, 24}) {
        QFont f = text->font();
        f.setPointSize(pt);
        text->setFont(f);
        const int chrome = 2 * qRound(text->document()->documentMargin()) + 2 * text->frameWidth();
        out << "Schrift " << pt << "pt: Zeilenabstand " << text->fontMetrics().lineSpacing()
            << "  Feldhoehe " << text->height()
            << "  Zeilen = " << qreal(text->height() - chrome) / text->fontMetrics().lineSpacing()
            << "  Fenster " << window.width() << "x" << window.height() << "\n";
        text->setPlainText(QStringLiteral("1\n2\n3\n4\n5\n6\n7\n8"));
        out << "        acht Zeilen: Feldhoehe " << text->height()
            << "  Zeilen = " << qreal(text->height() - chrome) / text->fontMetrics().lineSpacing() << "\n";
        text->clear();
    }

    return 0;
}
