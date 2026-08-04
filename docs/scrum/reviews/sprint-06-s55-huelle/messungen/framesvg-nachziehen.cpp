// Messung 3 — Was bringt ein FrameSvg dazu, einem Theme-Wechsel zu folgen?
//
// Diese Sonde ist aus einem Fehlschlag entstanden, nicht aus einer Vermutung.
// Der Bildläufer schrieb unter zwei verschiedenen Desktop-Themes **byteweise
// identische** Bilder. Der naheliegende Bau — ein ImageSet anlegen und ihm bei
// jedem Wechsel den neuen Namen geben — sieht richtig aus, wirkt aber nicht:
// Der FrameSvg behält die Grafik, die er einmal aufgelöst hat.
//
// Vier Wege werden gegeneinander gestellt. Drei davon sehen aus, als müssten
// sie greifen; nur der vierte greift. Ohne diese Messung fiele der Unterschied
// nicht auf, denn die Hülle wird in allen vier Fällen gezeichnet — nur eben
// die falsche.

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QGuiApplication>
#include <QTextStream>

namespace
{
QTextStream out(stdout);

const QString Path = QStringLiteral("dialogs/background");
const QString BasePath = QStringLiteral("plasma/desktoptheme");

/** Hin und zurück: Ein Weg, der nur einmal wirkt, fiele sonst nicht auf. */
const QStringList Round{QStringLiteral("breeze-dark"),
                        QStringLiteral("CachyOS-Nord-round"),
                        QStringLiteral("breeze-dark")};

void report(const QString &theme, KSvg::FrameSvg &frame)
{
    frame.resizeFrame(QSizeF(600, 200));
    out << "   " << theme.leftJustified(22) << " gültig=" << frame.isValid()
        << "  Rand links=" << frame.marginSize(KSvg::FrameSvg::LeftMargin) << "\n";
    out.flush();
}
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    out << "Messung 3 — Wie ein FrameSvg einem Theme-Wechsel folgt (#55, AK 5)\n";
    out << "==================================================================\n\n";
    out << "Erwartet: breeze-dark Rand 4, CachyOS-Nord-round Rand 8, dann wieder 4.\n\n";

    out << "A) ImageSet umbenennen (setImageSetName)\n";
    {
        KSvg::ImageSet set;
        set.setBasePath(BasePath);
        KSvg::FrameSvg frame;
        frame.setImageSet(&set);
        frame.setImagePath(Path);
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        for (const QString &theme : Round) {
            set.setImageSetName(theme);
            report(theme, frame);
        }
    }

    out << "\nB) Umbenennen und den Bildpfad erneut setzen\n";
    {
        KSvg::ImageSet set;
        set.setBasePath(BasePath);
        KSvg::FrameSvg frame;
        frame.setImageSet(&set);
        frame.setImagePath(Path);
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        for (const QString &theme : Round) {
            set.setImageSetName(theme);
            frame.setImagePath(Path);
            report(theme, frame);
        }
    }

    out << "\nC) Umbenennen und dasselbe ImageSet erneut zuweisen\n";
    {
        KSvg::ImageSet set;
        set.setBasePath(BasePath);
        KSvg::FrameSvg frame;
        frame.setImageSet(&set);
        frame.setImagePath(Path);
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        for (const QString &theme : Round) {
            set.setImageSetName(theme);
            frame.setImageSet(&set);
            report(theme, frame);
        }
    }

    out << "\nD) Je Wechsel ein FRISCHES ImageSet zuweisen\n";
    {
        KSvg::FrameSvg frame;
        frame.setImagePath(Path);
        frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        std::unique_ptr<KSvg::ImageSet> set;
        for (const QString &theme : Round) {
            auto fresh = std::make_unique<KSvg::ImageSet>(theme, BasePath);
            frame.setImageSet(fresh.get());
            set = std::move(fresh);
            report(theme, frame);
        }
    }

    out << "\nBefund: Nur D folgt. Das Erfassungsfenster baut deshalb bei jedem\n"
           "Theme-Wechsel ein neues ImageSet und wirft das alte erst weg, wenn alle\n"
           "drei FrameSvg auf das neue zeigen. A bis C zeichnen eine Hülle, die\n"
           "richtig aussieht und zum falschen Theme gehört.\n";
    out.flush();

    return 0;
}
