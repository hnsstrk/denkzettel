/**
 * Vorprüfung #83, Bearbeiter B — welche bestehenden Zusicherungen der native
 * Weg umwirft.
 *
 * Nachgebaut wird nicht das Fenster, sondern die **Messstelle** der drei
 * Zusicherungen aus `tests/capturetest.cpp`, die auf die Hülle greifen:
 *
 *   Z1  hullIsCompleteAtFiveAndEightLines() — fünfmal `qAlpha(...) == 255`
 *   Z2  paintsOneSurfaceInThePaletteColours() — `QCOMPARE(pixel, Window)`
 *   Z3  cornerRun() — Zahl der von x=0 an vollständig durchsichtigen Pixel
 *       der obersten Zeile; getragen von hullFollowsTheDesktopTheme(),
 *       hullFollowsAnInstalledDesktopTheme() und staysUsableWithoutADesktopTheme()
 *
 * Der Aufbau ist der von `CaptureTest::shot()`: WA_TranslucentBackground,
 * show(), processEvents(), grab().toImage(). Gemalt wird einmal auf dem
 * heutigen Weg (alphaMask + tinted) und einmal nativ (framePixmap in einem
 * Stück), damit die Zahlen nebeneinander stehen.
 *
 * Aufruf: testfolgen [Themename]   (offscreen, wie die Tests laufen)
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QTextStream>
#include <QWidget>

namespace {

constexpr int OutlineWidth = 1;
constexpr qreal FrameContrast = 0.20;

QColor mixed(const QColor &from, const QColor &to, qreal amount)
{
    return QColor::fromRgbF(from.redF() + (to.redF() - from.redF()) * amount,
                            from.greenF() + (to.greenF() - from.greenF()) * amount,
                            from.blueF() + (to.blueF() - from.blueF()) * amount);
}

QPixmap tinted(const QPixmap &shape, const QColor &colour)
{
    QPixmap result(shape.size());
    result.setDevicePixelRatio(shape.devicePixelRatio());
    result.fill(colour);
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, shape);
    return result;
}

/** Wie `CaptureTest::cornerRun()`. */
int cornerRun(const QImage &bild)
{
    int x = 0;
    while (x < bild.width() && qAlpha(bild.pixel(x, 0)) == 0) {
        ++x;
    }
    return x;
}

/** Das Fenster der Tests, auf die beiden Zeichenwege umschaltbar. */
class Probe : public QWidget
{
public:
    Probe(const QString &theme, bool nativ)
        : m_nativ(nativ)
        , m_set(theme, QStringLiteral("plasma/desktoptheme"))
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        for (KSvg::FrameSvg *f : {&m_hull, &m_inner}) {
            f->setImageSet(&m_set);
            f->setImagePath(QStringLiteral("dialogs/background"));
            f->setEnabledBorders(KSvg::FrameSvg::AllBorders);
        }
        resize(600, 174);
    }

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        m_hull.resizeFrame(size());
        m_inner.resizeFrame(size() - QSize(2 * OutlineWidth, 2 * OutlineWidth));
        QWidget::resizeEvent(e);
    }

    void paintEvent(QPaintEvent *e) override
    {
        QPainter p(this);
        const QColor surface = palette().color(QPalette::Window);
        if (m_nativ) {
            // Der native Weg: ein Stück, keine Einfärbung, keine Kontur.
            p.drawPixmap(0, 0, m_hull.framePixmap());
        } else {
            p.drawPixmap(0, 0, tinted(m_hull.alphaMask(), mixed(surface, palette().color(QPalette::WindowText), FrameContrast)));
            p.drawPixmap(OutlineWidth, OutlineWidth, tinted(m_inner.alphaMask(), surface));
        }
        QWidget::paintEvent(e);
    }

private:
    bool m_nativ;
    KSvg::ImageSet m_set;
    KSvg::FrameSvg m_hull;
    KSvg::FrameSvg m_inner;
};

}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);
    const QString theme = argc > 1 ? QString::fromUtf8(argv[1]) : QStringLiteral("default");

    out << "=== #83: was der native Weg an bestehenden Zusicherungen umwirft ===\n";
    out << "Plattform : " << QGuiApplication::platformName() << "\n";
    out << "Theme     : " << theme << "\n";
    out << "Palette   : Window " << qApp->palette().color(QPalette::Window).name() << "\n\n";

    for (const bool nativ : {false, true}) {
        Probe probe(theme, nativ);
        probe.show();
        QCoreApplication::processEvents();
        const QImage bild = probe.grab().toImage();

        const QColor surface = probe.palette().color(QPalette::Window);
        const int rand = qAlpha(bild.pixel(bild.width() / 2, 0));
        const int mitte = qAlpha(bild.pixel(bild.width() / 2, bild.height() / 2));
        const QColor mittePixel = bild.pixelColor(bild.width() / 2, bild.height() / 2);

        out << (nativ ? "########## nativ — framePixmap() in einem Stück\n"
                      : "########## heute — alphaMask() + tinted(), Ring aus zwei Rahmen\n");
        out << "  Z1 Alpha Randmitte oben : " << rand << (rand == 255 ? "   (== 255, Z1 hält)" : "   (!= 255, Z1 FÄLLT)") << "\n";
        out << "  Z1 Alpha Fenstermitte   : " << mitte << (mitte == 255 ? "   (== 255, Z1 hält)" : "   (!= 255, Z1 FÄLLT)") << "\n";
        out << "  Z2 Pixel Fenstermitte   : " << mittePixel.name(QColor::HexArgb)
            << "   Window " << surface.name(QColor::HexArgb)
            << (mittePixel == surface ? "   (gleich, Z2 hält)" : "   (verschieden, Z2 FÄLLT)") << "\n";
        out << "  Z3 cornerRun            : " << cornerRun(bild) << "\n\n";
    }

    out << "Lesart: Z1/Z2/Z3 sind die Zusicherungen aus tests/capturetest.cpp.\n"
           "Ein „FÄLLT\" heißt: Der Test wird durch #83 rot und muss neu gefasst werden —\n"
           "Umfang der Story, nicht Fehler des nativen Weges.\n";
    return 0;
}
