/**
 * Vorprüfung #83, Sonde 2 — das Fenster: Plattformgleichheit, Bindung, Blur.
 *
 * Ein Fenster nach dem nativen Weg: durchsichtiger Grund, `framePixmap()` in
 * einem Stück, kein `tinted()`, kein Ring, keine eigene Kontur. Gemessen wird,
 * was die Akzeptanzkriterien von #83 an diesem Fenster prüfen wollen:
 *
 *   A  grab(): Format, Alphakanal, Alpha in der Mitte und an den Kanten —
 *      derselbe Binärcode offscreen und unter Wayland (AK 4)
 *   B  die Zusicherungen, die `tests/capturetest.cpp` heute führt, gegen dieses
 *      Fenster gehalten: welche fallen?
 *   C  Fensterbindung über hide()/show(): bleibt QWindow dasselbe Objekt,
 *      bleibt die Fensterkennung? (Frage zur Weichzeichner-Anmeldung)
 *   D  KWindowEffects: isEffectAvailable(BlurBehind), enableBlurBehind vor und
 *      nach show()
 *   E  zuletzt und mit Absicht am Ende: enableBlurBehind(nullptr) — stürzt es?
 *
 * Aufruf: fensterlauf <Zielverzeichnis> [Themename]
 */

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowEffects>

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QEventLoop>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <QWindow>

namespace
{
constexpr int WindowWidth = 600;
constexpr int WindowHeight = 174;

/** Das Fenster, wie #83 es haben will: eine Hülle, in einem Stück. */
class NativeHull : public QWidget
{
public:
    explicit NativeHull(const QString &theme)
        : QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint)
        , m_imageSet(theme, QStringLiteral("plasma/desktoptheme"))
    {
        setAttribute(Qt::WA_TranslucentBackground);
        m_frame.setImageSet(&m_imageSet);
        m_frame.setImagePath(QStringLiteral("dialogs/background"));
        m_frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
        resize(WindowWidth, WindowHeight);
    }

    QRegion hullMask() const
    {
        return m_frame.mask();
    }

    /** Was die Heilung täte, die AK 2 verlangt: das Verhältnis nachziehen. */
    void catchUpRatio()
    {
        m_frame.setDevicePixelRatio(devicePixelRatioF());
        m_frame.resizeFrame(size());
        update();
    }

    /** Welches Verhältnis die Hülle zuletzt bekommen hat — nicht das des Fensters. */
    qreal hullRatio() const
    {
        return m_frame.devicePixelRatio();
    }

    /** Mitschrift der Ereignisse, die nach dem ersten Zeigen noch eintreffen. */
    QString eventLog() const
    {
        return m_log.join(QStringLiteral(", "));
    }

    bool event(QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::Resize:
            m_log << QStringLiteral("Resize");
            break;
        case QEvent::DevicePixelRatioChange:
            m_log << QStringLiteral("DevicePixelRatioChange");
            break;
        case QEvent::ScreenChangeInternal:
            m_log << QStringLiteral("ScreenChangeInternal");
            break;
        default:
            break;
        }
        return QWidget::event(event);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.drawPixmap(0, 0, m_frame.framePixmap());
    }

    void resizeEvent(QResizeEvent *) override
    {
        m_frame.setDevicePixelRatio(devicePixelRatioF());
        m_frame.resizeFrame(size());
    }

private:
    KSvg::ImageSet m_imageSet;
    mutable KSvg::FrameSvg m_frame;
    QStringList m_log;
};

QString rgba(const QColor &c)
{
    return QStringLiteral("%1,%2,%3 / Alpha %4")
        .arg(c.red(), 3)
        .arg(c.green(), 3)
        .arg(c.blue(), 3)
        .arg(c.alpha(), 3);
}

/** Wie weit die oberste Zeile noch ganz durchsichtig ist — cornerRun() der Tests. */
int cornerRun(const QImage &picture)
{
    int x = 0;
    while (x < picture.width() && picture.pixelColor(x, 0).alpha() == 0) {
        ++x;
    }
    return x;
}
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextStream out(stdout);

    const QString directory = app.arguments().value(1, QStringLiteral("."));
    const QString theme = app.arguments().value(2, QStringLiteral("default"));
    QDir().mkpath(directory);

    out << "=== Vorprüfung #83, Sonde 2: das Fenster auf dem nativen Weg ===\n";
    out << "Plattform      : " << app.platformName() << "\n";
    out << "Theme          : " << theme << "\n";
    out << "QT_SCALE_FACTOR: " << qEnvironmentVariable("QT_SCALE_FACTOR", QStringLiteral("(nicht gesetzt)"))
        << "\n";
    out << "qApp DPR       : " << app.devicePixelRatio() << "\n";

    NativeHull window(theme);
    window.show();
    QCoreApplication::processEvents();

    // Der Wert direkt nach show() — und der Wert, nachdem der Compositor dem
    // Fenster seine Fläche gegeben hat. Zwischen beiden liegt unter Wayland
    // ein Sprung, und **kein** Resize: das logische Maß bleibt 600x174.
    const qreal earlyRatio = window.devicePixelRatioF();
    const qreal earlyHull = window.hullRatio();
    QEventLoop settle;
    QTimer::singleShot(1200, &settle, &QEventLoop::quit);
    settle.exec();
    const qreal lateRatio = window.devicePixelRatioF();

    out << "Fenster-DPR direkt nach show()      : " << earlyRatio << "\n";
    out << "Fenster-DPR nach 1,2 s Ereignislauf : " << lateRatio << "\n";
    out << "Hülle bekam dabei zuletzt           : " << window.hullRatio()
        << " (direkt nach show(): " << earlyHull << ")\n";
    out << "Ereignisse nach dem ersten Zeigen   : " << window.eventLog() << "\n";
    out << "  -> Trägt die Hülle das Verhältnis des Fensters? "
        << (qFuzzyCompare(window.hullRatio(), lateRatio) ? "ja"
                                                        : "NEIN — resizeEvent() allein genügt nicht")
        << "\n";

    // ---------------------------------------------------------------- A
    out << "\n########## A — das Bild (AK 4: offscreen gleich Wayland) ##########\n";
    const QPixmap grabbed = window.grab();
    const QImage picture = grabbed.toImage();
    out << "  grab()      : " << grabbed.width() << "x" << grabbed.height() << " Bildpunkte, DPR "
        << grabbed.devicePixelRatio() << "\n";
    out << "  Bildformat  : " << static_cast<int>(picture.format()) << " (4 = RGB32 ohne Alpha, "
        << "6 = ARGB32_Premultiplied)\n";
    out << "  Alphakanal  : " << (picture.hasAlphaChannel() ? "ja" : "NEIN") << "\n";
    out << "  Ecke (0,0)          : " << rgba(picture.pixelColor(0, 0)) << "\n";
    out << "  Mitte               : "
        << rgba(picture.pixelColor(picture.width() / 2, picture.height() / 2)) << "\n";
    out << "  Kantenmitte oben    : " << rgba(picture.pixelColor(picture.width() / 2, 0)) << "\n";
    out << "  Kantenmitte links   : " << rgba(picture.pixelColor(0, picture.height() / 2)) << "\n";
    out << "  Kantenmitte unten   : "
        << rgba(picture.pixelColor(picture.width() / 2, picture.height() - 1)) << "\n";
    out << "  cornerRun()         : " << cornerRun(picture) << "\n";
    out << "  oberste Zeile, Alpha: ";
    for (int x = 0; x < 14 && x < picture.width(); ++x) {
        out << QStringLiteral("%1 ").arg(picture.pixelColor(x, 0).alpha(), 3);
    }
    out << "\n";

    picture.save(QDir(directory).filePath(
        QStringLiteral("fenster-%1-dpr-%2.png").arg(app.platformName()).arg(window.devicePixelRatioF())));

    // ---------------------------------------------------------------- A2
    // Dasselbe Bild, nachdem das Verhältnis nachgezogen wurde. **Das** ist der
    // Zustand, den AK 4 vergleicht: Hülle und Fenster auf demselben Wert. Der
    // Abschnitt darüber zeigt den Zustand, den ein Strang bekommt, der nur
    // resizeEvent() bedient.
    out << "\n########## A2 — nach dem Nachziehen des Verhältnisses ##########\n";
    window.catchUpRatio();
    QCoreApplication::processEvents();
    const QImage settled = window.grab().toImage();
    out << "  Hülle jetzt bei     : " << window.hullRatio() << ", Fenster bei "
        << window.devicePixelRatioF() << "\n";
    out << "  grab()              : " << settled.width() << "x" << settled.height()
        << " Bildpunkte\n";
    out << "  Mitte               : "
        << rgba(settled.pixelColor(settled.width() / 2, settled.height() / 2)) << "\n";
    out << "  cornerRun()         : " << cornerRun(settled) << "\n";
    out << "  oberste Zeile, Alpha: ";
    for (int x = 0; x < 14 && x < settled.width(); ++x) {
        out << QStringLiteral("%1 ").arg(settled.pixelColor(x, 0).alpha(), 3);
    }
    out << "\n";
    settled.save(QDir(directory).filePath(QStringLiteral("nachgezogen-%1-dpr-%2.png")
                                              .arg(app.platformName())
                                              .arg(window.devicePixelRatioF())));

    // ---------------------------------------------------------------- B
    out << "\n########## B — die heutigen Testzusicherungen an diesem Fenster ##########\n";
    out << "Die Zeilen stammen wörtlich aus tests/capturetest.cpp. Sie sind hier nicht\n"
           "ausgeführt, sondern nachgerechnet — ein Test darf in einer Vorprüfung nicht\n"
           "geändert werden.\n\n";
    const int centreAlpha = picture.pixelColor(picture.width() / 2, picture.height() / 2).alpha();
    const int topAlpha = picture.pixelColor(picture.width() / 2, 0).alpha();
    out << "  hullIsCompleteAtFiveAndEightLines(): QVERIFY(qAlpha(Mitte) == 255)      -> "
        << (centreAlpha == 255 ? "hält" : QStringLiteral("FÄLLT (Alpha %1)").arg(centreAlpha)) << "\n";
    out << "  hullIsCompleteAtFiveAndEightLines(): QVERIFY(qAlpha(Kantenmitte) == 255) -> "
        << (topAlpha == 255 ? "hält" : QStringLiteral("FÄLLT (Alpha %1)").arg(topAlpha)) << "\n";
    out << "  paintsOneSurfaceInThePaletteColours(): QCOMPARE(Pixel, palette Window)   -> "
        << "Pixel " << rgba(picture.pixelColor(picture.width() / 2, picture.height() / 2))
        << ", palette Window "
        << rgba(window.palette().color(QPalette::Window)) << "\n";
    out << "  hullFollowsTheDesktopTheme(): QVERIFY(cornerRun > 0)                     -> "
        << (cornerRun(picture) > 0 ? "hält" : "FÄLLT") << "\n";

    // ---------------------------------------------------------------- C
    out << "\n########## C — Fensterbindung über hide()/show() ##########\n";
    QWindow *before = window.windowHandle();
    const WId idBefore = window.winId();
    out << "  vor  hide(): QWindow " << (before ? "vorhanden" : "null") << ", winId "
        << QString::number(idBefore) << ", handle "
        << (before && before->handle() ? "vorhanden" : "null") << "\n";
    window.hide();
    QCoreApplication::processEvents();
    out << "  nach hide(): QWindow "
        << (window.windowHandle() ? "vorhanden" : "null") << ", handle "
        << (window.windowHandle() && window.windowHandle()->handle() ? "vorhanden" : "null")
        << "\n";
    window.show();
    QCoreApplication::processEvents();
    QWindow *after = window.windowHandle();
    out << "  nach show(): QWindow " << (after ? "vorhanden" : "null") << ", winId "
        << QString::number(window.winId()) << ", handle "
        << (after && after->handle() ? "vorhanden" : "null") << "\n";
    out << "  -> selbes QWindow-Objekt? " << (before == after ? "ja" : "NEIN") << "\n";
    out << "  -> selbe Fensterkennung? " << (idBefore == window.winId() ? "ja" : "NEIN") << "\n";
    out << "  Hinweis: dieselbe Kennung heißt unter Wayland nicht dieselbe Oberfläche.\n";

    // ---------------------------------------------------------------- D
    out << "\n########## D — KWindowEffects ##########\n";
    out << "  isEffectAvailable(BlurBehind)      : "
        << (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) ? "true" : "false") << "\n";
    out << "  isEffectAvailable(BackgroundContrast): "
        << (KWindowEffects::isEffectAvailable(KWindowEffects::BackgroundContrast) ? "true" : "false")
        << "\n";
    const QRegion mask = window.hullMask();
    out << "  Maskenregion: " << mask.rectCount() << " Rechteck(e), Hüllrechteck "
        << mask.boundingRect().width() << "x" << mask.boundingRect().height() << "\n";
    out << "  enableBlurBehind(windowHandle(), true, mask) …\n";
    KWindowEffects::enableBlurBehind(window.windowHandle(), true, mask);
    out << "  … zurückgekehrt. Rückgabewert: keiner — die Funktion ist void.\n";
    // Die Gegenprobe zur Frage, ob isEffectAvailable() als Zurücklesen taugt:
    // Vielleicht bindet das Wayland-Plugin seine Erweiterung erst beim ersten
    // Aufruf. Dann wäre der Wert danach ein anderer.
    QCoreApplication::processEvents();
    out << "  isEffectAvailable(BlurBehind) DANACH: "
        << (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) ? "true" : "false")
        << "\n";
    out << "  Erneuter Aufruf nach hide()/show():\n";
    window.hide();
    QCoreApplication::processEvents();
    window.show();
    QCoreApplication::processEvents();
    KWindowEffects::enableBlurBehind(window.windowHandle(), true, mask);
    out << "  … zurückgekehrt.\n";

    // ---------------------------------------------------------------- E
    out.flush();
    out << "\n########## E — enableBlurBehind(nullptr) ##########\n";
    out << "  Steht mit Absicht am Ende: stürzt der Aufruf, fehlt danach nur diese Zeile.\n";
    out.flush();
    KWindowEffects::enableBlurBehind(nullptr, true, mask);
    out << "  Kein Absturz — ein Fenster ohne QWindow ist verkraftet.\n";
    out.flush();

    return 0;
}
