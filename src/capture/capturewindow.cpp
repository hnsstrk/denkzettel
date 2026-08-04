#include "capture/capturewindow.h"

#include "capture/textareaheight.h"
#include "store/note.h"
#include "store/store.h"

#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>
#include <KWindowShadow>

#include <QAbstractTextDocumentLayout>
#include <QDateTime>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPlainTextEdit>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>
#include <QtMath>

namespace
{
constexpr int WindowWidth = 600;

/** Time the compositor gets to notice the surface going away, in ms. */
constexpr int RemapDelayMs = 50;

/** Inner spacing of wireframe 4b — counted on top of the theme's own margin. */
constexpr QMargins ContentMargins = QMargins(12, 10, 12, 8);

/**
 * The whole grouping of the window since the separator line was dropped: the
 * footer gets more air than the application name (wireframe 4b).
 */
constexpr int SpacingBelowAppName = 8;
constexpr int SpacingAboveFooter = 12;

/** Width of the single line of the hull, in logical pixels. */
constexpr int OutlineWidth = 1;

/**
 * How far the outline is mixed from the surface towards the text colour. In a
 * Widgets application `frameContrast` is this constant — there is no
 * KColorScheme behind it (issue #55). Measured over all 18 installed colour
 * schemes it lands between 1,24:1 and 1,91:1 against the surface: visible
 * everywhere, obtrusive nowhere (wireframe 4b).
 */
constexpr qreal FrameContrast = 0.20;

/**
 * Where the hull comes from, and what KSvg falls back to without an answer.
 *
 * Latin-1 views and not `QString`: a `QString` at namespace scope is a non-POD
 * static — it is built in an unspecified order relative to other translation
 * units and torn down again at exit. These four are pure ASCII, so the view
 * costs nothing and carries the same name.
 */
constexpr QLatin1StringView DesktopThemePath("plasma/desktoptheme");
constexpr QLatin1StringView HullImage("dialogs/background");
constexpr QLatin1StringView ShadowPrefix("shadow");
constexpr QLatin1StringView DefaultDesktopTheme("default");

QColor mixed(const QColor &from, const QColor &to, qreal amount)
{
    return QColor::fromRgbF(from.redF() + (to.redF() - from.redF()) * amount,
                            from.greenF() + (to.greenF() - from.greenF()) * amount,
                            from.blueF() + (to.blueF() - from.blueF()) * amount);
}

/**
 * The theme's shape, filled with a colour of ours.
 *
 * `shape` is the alpha channel the desktop theme renders — the rounding of its
 * corners with every intermediate step, not a radius we made up. What it gets
 * filled with comes from the palette (wireframe 4a).
 */
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

/**
 * One shadow tile of the desktop theme, ready for the compositor.
 *
 * Not `Svg::pixmap(element)`, and that is measured: it ignores the element and
 * hands back the **whole** image at the size of the SVG — every tile would then
 * be the entire shadow, corners included, and the compositor would take it
 * without complaint (measurement 6 of this story). `image()` with the element's
 * own size is the call that cuts.
 */
KWindowShadowTile::Ptr shadowTile(KSvg::FrameSvg *tiles, const QString &element)
{
    const QSize size = tiles->elementSize(element).toSize();
    if (size.isEmpty()) {
        return {};
    }

    const QImage image = tiles->image(size, element);
    if (image.isNull()) {
        return {};
    }

    auto tile = KWindowShadowTile::Ptr::create();
    tile->setImage(image);
    return tile;
}

/** Small, dimmed label — used for the application name and the key hint. */
QLabel *subtleLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

    // The role, not the colour: the daemon keeps the window for its whole life
    // (SPEC 2.1), and a colour taken from the palette once would stay put when
    // the user changes the colour scheme (issue #54). A role is resolved anew
    // on every palette change.
    label->setForegroundRole(QPalette::PlaceholderText);

    return label;
}
}

CaptureWindow::CaptureWindow(Store *store, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , m_store(store)
    , m_text(new QPlainTextEdit(this))
    , m_plasmaConfig(KSharedConfig::openConfig(QStringLiteral("plasmarc")))
    , m_hull(new KSvg::FrameSvg(this))
    , m_hullInner(new KSvg::FrameSvg(this))
    , m_shadowTiles(new KSvg::FrameSvg(this))
{
    setWindowTitle(i18n("Denkzettel"));

    // The hull has rounded corners, so the corners of the window have to be
    // able to disappear. Nothing is left transparent by accident: paintEvent()
    // fills every pixel it keeps, and without a theme it fills all of them.
    setAttribute(Qt::WA_TranslucentBackground);

    for (KSvg::FrameSvg *frame : {m_hull, m_hullInner, m_shadowTiles}) {
        frame->setImagePath(HullImage);
        frame->setEnabledBorders(KSvg::FrameSvg::AllBorders);
        connect(frame, &KSvg::Svg::repaintNeeded, this, qOverload<>(&QWidget::update));
    }
    m_shadowTiles->setElementPrefix(ShadowPrefix);

    m_text->setFrameShape(QFrame::NoFrame);
    m_text->setPlaceholderText(i18n("Gedanke festhalten …"));
    m_text->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_text->installEventFilter(this);
    // One continuous surface, not a box inside a box (wireframe 4b): the text
    // area draws no ground of its own.
    m_text->viewport()->setAutoFillBackground(false);
    applyTextColours();

    // Activating the window puts the keyboard focus straight into the text.
    setFocusProxy(m_text);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(subtleLabel(i18n("Denkzettel"), this));
    layout->addSpacing(SpacingBelowAppName);
    layout->addWidget(m_text);
    layout->addSpacing(SpacingAboveFooter);

    QLabel *hint = subtleLabel(i18n("Esc verwirft · Strg+Enter speichert"), this);
    hint->setAlignment(Qt::AlignCenter);
    layout->addWidget(hint);

    connect(m_text->document()->documentLayout(),
            &QAbstractTextDocumentLayout::documentSizeChanged,
            this,
            &CaptureWindow::adjustHeight);

    // A desktop theme change has to reach a window that is already standing —
    // the daemon builds this one once and keeps it (SPEC 2.1). The watch sits
    // on the file and not on KConfigWatcher, and that is measured: a writer
    // that omits `KConfig::Notify` reaches KConfigWatcher not at all, while
    // KDirWatch sees both kinds (measurement 2 of this story). KConfig replaces
    // the file rather than rewriting it, which is why `created` counts too.
    const QString plasmarc = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QStringLiteral("/plasmarc");
    KDirWatch::self()->addFile(plasmarc);
    // Both connections spelled out rather than looped over the two signals:
    // clazy cannot see through a loop variable that a pointer-to-member is a
    // signal and reports the pair as a non-signal connect. Two lines are
    // shorter than the loop plus a suppression, and the next reader gets them
    // for free.
    auto onPlasmarcChanged = [this, plasmarc](const QString &path) {
        if (path == plasmarc) {
            reloadDesktopTheme();
        }
    };
    connect(KDirWatch::self(), &KDirWatch::dirty, this, onPlasmarcChanged);
    connect(KDirWatch::self(), &KDirWatch::created, this, onPlasmarcChanged);

    reloadDesktopTheme();
    adjustHeight();
    resize(WindowWidth, sizeHint().height());
}

CaptureWindow::~CaptureWindow() = default;

const KWindowShadow *CaptureWindow::shadow() const
{
    return m_shadow.get();
}

void CaptureWindow::reloadDesktopTheme(const QString &name)
{
    QString theme = name;
    if (theme.isEmpty()) {
        m_plasmaConfig->reparseConfiguration();
        theme = KConfigGroup(m_plasmaConfig, QStringLiteral("Theme"))
                    .readEntry("name", QString(DefaultDesktopTheme));
    }

    // Two measured properties of KSvg in one line (measurements 1 and 3 of this
    // story). First: KSvg does not read `plasmarc` itself — pointed at the
    // desktop theme path alone it stays on `default`, whatever the file says,
    // so the name has to be handed over. Second: a FrameSvg keeps the image it
    // once resolved. Renaming its image set does not move it, re-setting the
    // path does not, re-assigning the same set does not — only a **fresh** set
    // does. Hence a new one here rather than a rename, and the old one only
    // goes once all three frames point at the new one.
    auto imageSet = std::make_unique<KSvg::ImageSet>(theme, DesktopThemePath);
    for (KSvg::FrameSvg *frame : {m_hull, m_hullInner, m_shadowTiles}) {
        frame->setImageSet(imageSet.get());
    }
    m_imageSet = std::move(imageSet);

    applyHullMargins();
    // The margins are part of the height: a wider theme border makes a taller
    // window at the same five lines.
    adjustHeight();
    resizeHull();
    bindShadow();
    update();
}

void CaptureWindow::showCapture()
{
    // A mapped window cannot take the keyboard focus back on Wayland (T1,
    // issue #1): hide() destroys the surface, so the following show() is a
    // fresh mapping, and a fresh toplevel is focused by the compositor on its
    // own. The delay gives the compositor time to see the surface go away.
    if (isVisible()) {
        hide();
        QTimer::singleShot(RemapDelayMs, this, &CaptureWindow::present);
        return;
    }

    present();
}

bool CaptureWindow::eventFilter(QObject *watched, QEvent *event)
{
    // The height rests on the line spacing, and that changes with the font —
    // but the document does not, so documentSizeChanged stays silent and the
    // field keeps the height it was built with (issue #56). The filter is the
    // place for this and an overridden changeEvent() of the window is not: a
    // font set on the text area alone never reaches the window, and that is
    // the road the test takes (measurement 3 of the sprint 6 estimate).
    if (watched == m_text && event->type() == QEvent::FontChange) {
        adjustHeight();
        return QWidget::eventFilter(watched, event);
    }

    // The note text is drawn in the window's text colour, so it has to be
    // copied over again whenever that colour changes (issue #54: a colour
    // taken once and kept stays put when the scheme moves).
    if (watched == m_text && event->type() == QEvent::PaletteChange) {
        applyTextColours();
        return QWidget::eventFilter(watched, event);
    }

    if (watched == m_text && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        const bool isReturn = keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter;

        if (isReturn && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            save();
            return true;
        }

        if (keyEvent->key() == Qt::Key_Escape) {
            discard();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CaptureWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    const QColor surface = palette().color(QPalette::Window);

    if (!m_hull->isValid()) {
        // Outside a Plasma session `dialogs/background` is simply not there
        // (AK 9). The window then wears no hull — and that is the whole
        // difference: it stays opaque, and it stays usable.
        painter.fillRect(rect(), surface);
        QWidget::paintEvent(event);
        return;
    }

    // Two shapes of the same theme, one outline width apart. The ring between
    // them is the single line of the window, and it follows the theme's
    // rounding because it is that rounding, drawn twice.
    painter.drawPixmap(0, 0, tinted(m_hull->alphaMask(), mixed(surface, palette().color(QPalette::WindowText), FrameContrast)));
    painter.drawPixmap(OutlineWidth, OutlineWidth, tinted(m_hullInner->alphaMask(), surface));

    QWidget::paintEvent(event);
}

void CaptureWindow::resizeEvent(QResizeEvent *event)
{
    resizeHull();
    QWidget::resizeEvent(event);
}

void CaptureWindow::resizeHull()
{
    m_hull->resizeFrame(size());
    m_hullInner->resizeFrame(size() - QSize(2 * OutlineWidth, 2 * OutlineWidth));
    m_shadowTiles->resizeFrame(size());
}

void CaptureWindow::applyHullMargins()
{
    qreal left = 0;
    qreal top = 0;
    qreal right = 0;
    qreal bottom = 0;
    if (m_hull->isValid()) {
        m_hull->getMargins(left, top, right, bottom);
    }

    // The inner spacing of 4b counts on top of the strip the theme claims for
    // itself: the text starts 16 px from the window edge under Breeze and
    // 20 px under an 8 px theme.
    layout()->setContentsMargins(ContentMargins.left() + qRound(left),
                                 ContentMargins.top() + qRound(top),
                                 ContentMargins.right() + qRound(right),
                                 ContentMargins.bottom() + qRound(bottom));
}

void CaptureWindow::applyTextColours()
{
    QPalette palette = m_text->palette();
    // On the continuous surface the note text carries `WindowText` and not the
    // role for entry fields: measured over all 18 colour schemes the former
    // holds 4,74:1 at worst, the latter 4,22:1 and thus below the minimum of
    // 4,5:1 (wireframe 4b). Copied on every palette change rather than set
    // once — a colour taken from the palette and kept would freeze (issue #54).
    palette.setColor(QPalette::Text, this->palette().color(QPalette::WindowText));
    palette.setColor(QPalette::Base, Qt::transparent);

    if (palette != m_text->palette()) {
        m_text->setPalette(palette);
    }
}

void CaptureWindow::bindShadow()
{
    m_shadow.reset();

    // The tiles come from the same image as the hull, under its `shadow`
    // prefix. All eight desktop themes on the customer's machine bring them
    // (measured 01.08.2026); a theme without them simply gets no shadow.
    // Before the first show() there is no native window to hang it on; present()
    // comes back here once there is.
    if (!windowHandle() || !m_shadowTiles->hasElementPrefix(ShadowPrefix)) {
        return;
    }

    auto shadow = std::make_unique<KWindowShadow>();
    shadow->setTopLeftTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-topleft")));
    shadow->setTopTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-top")));
    shadow->setTopRightTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-topright")));
    shadow->setRightTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-right")));
    shadow->setBottomRightTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-bottomright")));
    shadow->setBottomTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-bottom")));
    shadow->setBottomLeftTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-bottomleft")));
    shadow->setLeftTile(shadowTile(m_shadowTiles, QStringLiteral("shadow-left")));

    qreal left = 0;
    qreal top = 0;
    qreal right = 0;
    qreal bottom = 0;
    m_shadowTiles->getMargins(left, top, right, bottom);
    shadow->setPadding(QMargins(qRound(left), qRound(top), qRound(right), qRound(bottom)));

    shadow->setWindow(windowHandle());
    // Fails without a compositor, and that is not an error of ours: offscreen
    // there is nobody to hand the tiles to (measured, sprint 6 planning 3.3).
    // The tiles above are the part that can be shown either way (AK 7).
    shadow->create();

    m_shadow = std::move(shadow);
}

void CaptureWindow::present()
{
    show();
    raise();
    activateWindow();

    // After show(), and after every show(): each appearance destroys the
    // Wayland surface and maps a fresh one (SPEC 3), and a shadow bound to the
    // old one is gone with it. This is the line no test of this project would
    // notice missing — it shows only on the second opening.
    bindShadow();
}

void CaptureWindow::save()
{
    const QString content = m_text->toPlainText().trimmed();
    if (content.isEmpty()) {
        discard();
        return;
    }

    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.type = Note::Type::Text;
    note.content = content;

    if (!m_store->addNote(note)) {
        // Keep window and text: a lost thought is worse than a window that
        // stays open.
        qWarning("Storing the note failed: %s", qPrintable(m_store->lastError()));
        return;
    }

    m_text->clear();
    hide();
}

void CaptureWindow::discard()
{
    m_text->clear();
    hide();
}

void CaptureWindow::adjustHeight()
{
    // QPlainTextDocumentLayout reports its height as a line count, not in
    // pixels — wrapped lines included.
    const int documentLines = qCeil(m_text->document()->size().height());
    const int chrome = 2 * qRound(m_text->document()->documentMargin()) + 2 * m_text->frameWidth();

    m_text->setFixedHeight(capture::textAreaHeight(documentLines, m_text->fontMetrics().lineSpacing(), chrome));
    resize(width(), sizeHint().height());
}
