#include "capture/recordingwindow.h"

#include "capture/audiorecorder.h"
#include "platform/systemfonts.h"
#include "store/note.h"
#include "store/store.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <KNotification>

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

namespace
{
constexpr int WindowWidth = 600;

/** Time the compositor gets to notice the surface going away, in ms. */
constexpr int RemapDelayMs = 50;

/** Inner spacing of wireframe 4b — counted on top of the theme's own margin. */
constexpr QMargins ContentMargins = QMargins(12, 10, 12, 8);

/**
 * The grouping of the capture window, taken over unchanged: the footer gets
 * more air than the application name, and there is no parting line above it
 * (wireframe 4b, and the note on 1f of 29.08.2026).
 */
constexpr int SpacingBelowAppName = 8;
constexpr int SpacingAboveFooter = 12;

/** The gap between the dot, the meter and the running time (wireframe 1f). */
constexpr int RowSpacing = 12;

/**
 * How long before the upper bound the window says what is coming — one minute,
 * which is SPEC 4's "from minute 14" at the bound of fifteen.
 *
 * Derived from the bound rather than written out as fourteen minutes: the two
 * would otherwise drift apart, and a check that lowers the bound to reach it
 * within a run would never see the hint at all.
 */
constexpr qint64 HintBeforeBoundMs = 60LL * 1000;

/** The running time, in the form wireframe 1f writes it: `0:23`, `14:07`. */
QString asClock(qint64 milliseconds)
{
    return QTime(0, 0).addMSecs(static_cast<int>(milliseconds)).toString(QStringLiteral("m:ss"));
}

/** Small label in the smallest readable font — the heading and the footer. */
QLabel *smallLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setFont(platform::smallestReadableFont());

    return label;
}

/**
 * The running time's font: fixed pitch, at the size of the general interface
 * font.
 *
 * Fixed pitch so the digits do not shift sideways once a second — the number
 * is the one thing in this window that changes on its own. And bigger than the
 * two labels around it, because it is what the user looks at; the size comes
 * from the general font rather than from a factor of ours, so it follows
 * `kdeglobals` like everything else (issue #68).
 */
QFont clockFont()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSizeF(platform::generalFont().pointSizeF());
    return font;
}
}

namespace capture
{

/**
 * The red dot of SPEC 4, with the soft halo wireframe 1f draws around it.
 *
 * It rests. In a window where the running time and the level are already
 * moving, a blinking dot is the third movement in something that stands open
 * for minutes (UX decision of 29.08.2026).
 *
 * The colour is the scheme's negative text and not a red of our own: it is the
 * same source the running time turns to at minute 14, so the window has one
 * red and not two.
 */
class RecordingDot : public QWidget
{
public:
    explicit RecordingDot(QWidget *parent)
        : QWidget(parent)
    {
        setFixedSize(Diameter + 2 * Halo, Diameter + 2 * Halo);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const QColor red =
            KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::NegativeText).color();

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        QColor halo = red;
        halo.setAlphaF(0.25);
        painter.setBrush(halo);
        painter.drawEllipse(rect());

        painter.setBrush(red);
        painter.drawEllipse(rect().adjusted(Halo, Halo, -Halo, -Halo));
    }

private:
    static constexpr int Diameter = 12;
    static constexpr int Halo = 4;
};

/**
 * The level meter of SPEC 4: seven bars that light up from the left.
 *
 * An amplitude meter and not a history — what the user wants of it during a
 * recording is whether the microphone hears them, and that is in the current
 * value (UX decision of 29.08.2026). It therefore needs one number per buffer
 * and no memory.
 *
 * The two colours are the ones the window has already resolved for its writing:
 * a lit bar in the note text's colour, an unlit one in the dimmed one. So the
 * meter follows the desktop theme and the colour scheme without a colour of
 * its own (issue #85).
 */
class LevelMeter : public QWidget
{
public:
    explicit LevelMeter(QWidget *parent)
        : QWidget(parent)
    {
        setFixedSize(Bars * BarWidth + (Bars - 1) * BarGap, Height);
    }

    void setColours(const QColor &lit, const QColor &unlit)
    {
        m_lit = lit;
        m_unlit = unlit;
        update();
    }

    /** `level` from 0 (silence) to 1 (full scale); anything outside is clamped. */
    void setLevel(qreal level)
    {
        // ponytail: the peak straight onto the bars, no curve and no falloff.
        // Ceiling: a linear scale spends its upper half on the loudest tenth
        // of what a voice does, so a quiet speaker lights two bars where a
        // loud one lights five. The way up is a decibel scale — 20*log10 over
        // a floor of about -50 dB — and it is worth building only once
        // somebody has looked at the meter and called it dead.
        const int lit = qBound(0, qRound(qBound(qreal(0), level, qreal(1)) * Bars), Bars);
        if (lit == m_litBars) {
            return;
        }
        m_litBars = lit;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        for (int bar = 0; bar < Bars; ++bar) {
            painter.setBrush(bar < m_litBars ? m_lit : m_unlit);
            painter.drawRect(bar * (BarWidth + BarGap), 0, BarWidth, Height);
        }
    }

private:
    static constexpr int Bars = 7;
    static constexpr int BarWidth = 8;
    static constexpr int BarGap = 3;
    static constexpr int Height = 20;

    QColor m_lit;
    QColor m_unlit;
    int m_litBars = 0;
};

}

RecordingWindow::RecordingWindow(Store *store, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , m_store(store)
    , m_recorder(new AudioRecorder(store->audioDirectory(), this))
    , m_appName(smallLabel(i18n("Denkzettel"), this))
    , m_dot(new capture::RecordingDot(this))
    , m_meter(new capture::LevelMeter(this))
    , m_elapsed(new QLabel(this))
    , m_hint(smallLabel(i18n("Esc discards · Ctrl+Enter saves"), this))
    , m_clock(new QTimer(this))
    , m_hull(new capture::WindowHull(this))
{
    setWindowTitle(i18n("Denkzettel"));

    // The hull has rounded corners, so the corners of the window have to be
    // able to disappear. The theme's own graphic decides how much of the rest
    // stays see-through; without a theme paintEvent() fills every pixel.
    setAttribute(Qt::WA_TranslucentBackground);

    // Nothing in this window takes the keyboard by itself — there is no text
    // field — so the window is what the two keys of SPEC 4 have to reach.
    setFocusPolicy(Qt::StrongFocus);

    // Not dimmed, like the capture window's heading and for the same reason
    // (issue #84): a window that shows nothing but dimmed text looks foreign
    // to the scheme under it even where every colour is right.
    m_appName->setForegroundRole(QPalette::WindowText);
    m_hint->setForegroundRole(QPalette::PlaceholderText);
    m_hint->setAlignment(Qt::AlignCenter);
    m_elapsed->setFont(clockFont());

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(m_appName);
    layout->addSpacing(SpacingBelowAppName);

    auto *row = new QHBoxLayout;
    row->setSpacing(RowSpacing);
    row->addWidget(m_dot);
    row->addWidget(m_meter);
    row->addStretch();
    row->addWidget(m_elapsed);
    layout->addLayout(row);

    layout->addSpacing(SpacingAboveFooter);
    layout->addWidget(m_hint);

    // The display reads AudioRecorder::duration(), so it moves when the file
    // does. Four times a second: a second is the smallest step it shows, and
    // reading it at exactly that rate would let the shown second lag by up to
    // one whole step.
    m_clock->setInterval(250);
    connect(m_clock, &QTimer::timeout, this, &RecordingWindow::showElapsed);

    connect(m_recorder, &AudioRecorder::levelChanged, m_meter, &capture::LevelMeter::setLevel);
    connect(m_recorder, &AudioRecorder::finished, this, &RecordingWindow::storeRecording);
    connect(m_recorder, &AudioRecorder::cancelled, this, [this] {
        m_awaitingAnswer = false;
    });
    // A recording that breaks off says so, and it has to: the window closes on
    // Ctrl+Enter before the file is finished, so a failure after that point
    // would otherwise be a voice note the user believes they have made
    // (SPEC 14, the loud channel). The file is already gone — the recorder
    // deletes what it cannot answer for — so there is no path to name here.
    connect(m_recorder, &AudioRecorder::failed, this, [this](const QString &message) {
        m_awaitingAnswer = false;
        // Like the two answers beside it, and it was the one that forgot: a
        // recording that broke off left the display ticking four times a second
        // on a hidden window for the rest of the service's life (finding of the
        // review, 29.08.2026 — QTimer::isActive() read 1 after failed() where
        // Esc read 0).
        m_clock->stop();
        hide();
        KNotification::event(KNotification::Error,
                             i18n("Recording failed"),
                             i18n("The voice note could not be recorded: %1", message));
    });

    // The hull tells this window when the desktop theme, or the compositor's
    // blur, has moved under it — it is what watches `plasmarc` and `kwinrc`.
    connect(m_hull, &capture::WindowHull::changed, this, &RecordingWindow::applyHull);

    m_hull->reload();
    showElapsed();
    resize(WindowWidth, sizeHint().height());
}

RecordingWindow::~RecordingWindow() = default;

AudioRecorder *RecordingWindow::recorder() const
{
    return m_recorder;
}

bool RecordingWindow::startWithoutADevice()
{
    show();
    return beginRecording(false);
}

QString capture::recordingNotSavedMessage(const QString &path, bool rescued)
{
    if (rescued) {
        return i18n("The note could not be created. The recording was moved to %1 "
                    "and will not be deleted.",
                    path);
    }

    return i18n("The note could not be created. The recording is at %1, and moving it "
                "to safety failed — the next start of Denkzettel will delete it.",
                path);
}

bool RecordingWindow::beginRecording(bool withADevice)
{
    if (m_awaitingAnswer) {
        return true;
    }

    // The moment is taken once: it names the note and the file at the same
    // time, and asked twice it would name two.
    m_createdAt = QDateTime::currentDateTime();
    if (!(withADevice ? m_recorder->start(m_createdAt) : m_recorder->startEncoder(m_createdAt))) {
        return false;
    }

    // **The recorder can answer inside the call above.** Measured 29.08.2026:
    // with a directory the muxer cannot write into, QMediaRecorder emits
    // errorOccurred **synchronously** out of record(), so failed() has already
    // been handled — window hidden, user told — by the time startEncoder()
    // returns true. Arming below would then start a display for a recording
    // that is over, and leave m_awaitingAnswer standing: the timer would tick
    // four times a second on a hidden window for the rest of the service's
    // life, and the next Meta+Shift+N would show a window and record nothing.
    //
    // True and not false, because there is nothing left for the caller to
    // report: the answer went out through the signal, and present() would
    // otherwise put a second notification on top of it for the same failure.
    if (!m_recorder->isRecording()) {
        return true;
    }

    m_awaitingAnswer = true;
    m_meter->setLevel(0);
    showElapsed();
    m_clock->start();
    return true;
}

void RecordingWindow::reloadDesktopTheme(const QString &name)
{
    m_hull->reload(name);
}

void RecordingWindow::applyHull()
{
    applyTextColours();

    // The inner spacing counts on top of the strip the theme claims for
    // itself, exactly as in the capture window.
    layout()->setContentsMargins(ContentMargins + m_hull->margins());
    layout()->invalidate();
    layout()->activate();
    resize(width(), sizeHint().height());

    m_hull->resizeToWindow();
    m_hull->bindToWindow();
    update();
}

void RecordingWindow::applyTextColours()
{
    // The same order of precedence as the capture window's, and for the same
    // reason (issue #85): where the desktop theme brings a hand of its own the
    // writing comes from it, otherwise from the colour scheme.
    const capture::ThemeTextColours themeText = m_hull->themeTextColours();
    const QColor noteColour = themeText.normal.isValid()
        ? m_hull->textColour()
        : palette().color(QPalette::WindowText);
    const QColor subtleColour = themeText.inactive.isValid()
        ? themeText.inactive
        : palette().color(QPalette::PlaceholderText);

    for (QLabel *label : {m_appName, m_hint}) {
        QPalette labelPalette = label->palette();
        labelPalette.setColor(label->foregroundRole(),
                              label == m_appName ? noteColour : subtleColour);
        if (labelPalette != label->palette()) {
            label->setPalette(labelPalette);
        }
    }

    m_meter->setColours(noteColour, subtleColour);
    // The running time carries the note text's colour until minute 14, and
    // showElapsed() is the one place that decides which of the two it is.
    showElapsed();
}

void RecordingWindow::showElapsed()
{
    const qint64 elapsed = m_recorder->duration();
    const qint64 bound = m_recorder->maximumDuration();
    const bool nearTheBound = bound > 0 && elapsed >= bound - HintBeforeBoundMs;

    m_elapsed->setText(asClock(elapsed));

    const capture::ThemeTextColours themeText = m_hull->themeTextColours();
    const QColor noteColour = themeText.normal.isValid()
        ? m_hull->textColour()
        : palette().color(QPalette::WindowText);
    // The colour scheme's own negative text, the same source the dot takes its
    // red from. Colour alone does not say how much time is left, which is why
    // the footer says it too — and the footer is a line that is there anyway,
    // so the hint costs no space (UX decision of 29.08.2026).
    const QColor warning =
        KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::NegativeText).color();

    QPalette elapsedPalette = m_elapsed->palette();
    elapsedPalette.setColor(m_elapsed->foregroundRole(), nearTheBound ? warning : noteColour);
    if (elapsedPalette != m_elapsed->palette()) {
        m_elapsed->setPalette(elapsedPalette);
    }

    // It says what will happen and not what will be lost: at the bound the
    // recording is saved, the way Ctrl+Enter saves it (SPEC 4, decision of
    // 29.08.2026).
    m_hint->setText(nearTheBound
                        ? i18n("%1 left — the recording ends at %2.",
                               asClock(bound - elapsed),
                               asClock(bound))
                        : i18n("Esc discards · Ctrl+Enter saves"));
}

void RecordingWindow::showRecorder()
{
    // A mapped window cannot take the keyboard focus back on Wayland (T1,
    // issue #1): hide() destroys the surface, so the following show() is a
    // fresh mapping, and a fresh toplevel is focused by the compositor on its
    // own. The delay gives the compositor time to see the surface go away.
    if (isVisible()) {
        hide();
        QTimer::singleShot(RemapDelayMs, this, &RecordingWindow::present);
        return;
    }

    present();
}

void RecordingWindow::present()
{
    show();
    raise();
    activateWindow();

    // After show(), and after every show(): each appearance destroys the
    // Wayland surface and maps a fresh one, and a shadow bound to the old one
    // is gone with it. The class comment of bindToWindow() carries the reason
    // the effects have to follow in the same breath.
    m_hull->bindToWindow();

    // The recording starts with the window and not with a button (SPEC 4).
    if (!beginRecording(true)) {
        hide();
        KNotification::event(KNotification::Error,
                             i18n("Recording failed"),
                             i18n("The voice note could not be recorded: %1", m_recorder->lastError()));
    }
}

void RecordingWindow::keyPressEvent(QKeyEvent *event)
{
    const bool isReturn = event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter;
    if (isReturn && event->modifiers().testFlag(Qt::ControlModifier)) {
        save();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        discard();
        return;
    }

    QWidget::keyPressEvent(event);
}

void RecordingWindow::save()
{
    // The window goes at once (SPEC 4) and the note follows: stop() returns
    // while the muxer is still writing the last page, and the note is made in
    // storeRecording(), when the file is closed and its length is known. Made
    // here it would enter the transcription queue ahead of its own file, and
    // both attempts of SPEC 12 would be spent in no time at all (issue #22).
    m_clock->stop();
    m_recorder->stop();
    hide();
}

void RecordingWindow::discard()
{
    m_clock->stop();
    m_recorder->cancel();
    hide();
}

void RecordingWindow::storeRecording(const QString &fileName, int durationSeconds)
{
    m_awaitingAnswer = false;
    m_clock->stop();
    // The window is already gone when the user pressed Ctrl+Enter; it is still
    // standing when the upper bound of SPEC 4 ended the recording, and then it
    // has to go here — minute 15 ends the recording the way Ctrl+Enter does.
    hide();

    Note note;
    note.createdAt = m_createdAt;
    note.type = Note::Type::Audio;
    note.audioPath = fileName;
    note.audioDurationS = durationSeconds;
    // No content: the transcription queue takes an audio note whose text is
    // still empty, and it hears of this one over Store::noteAdded (SPEC 12).
    if (m_store->addNote(note)) {
        return;
    }

    // The one case the cleanup check of SPEC 2.5 must not be left to decide.
    // The recording is on the disk and no row points at it; in the data that is
    // a harmless orphan, and the sweep at the next service start deletes it.
    //
    // **So the file is moved, not talked about.** Telling the user where it
    // lies was the first answer and it was not one: the sentence promised the
    // recording would not be deleted while the next start of the service
    // removed it, and it was true only for a user who acted before restarting
    // (finding of the review, 29.08.2026). `rescued/` lies beside `audio/`, and
    // the sweep reads the files of `audio/` and no subdirectory — so this is a
    // place it does not look, which is what "barred at its source" has to mean
    // here. Nothing in Denkzettel deletes anything under there; what happens to
    // it is the user's decision.
    const QString file = m_store->audioDirectory() + QLatin1Char('/') + fileName;
    const QString rescued = m_store->rescuedDirectory() + QLatin1Char('/') + fileName;
    // Both halves have to hold before the promise may be made: a directory that
    // cannot be created and a rename that fails leave the recording where it
    // stood, and the message then says so rather than assuring something nobody
    // checked.
    const bool moved = QDir().mkpath(m_store->rescuedDirectory()) && QFile::rename(file, rescued);
    qWarning("Storing the voice note failed: %s", qPrintable(m_store->lastError()));
    if (!moved) {
        qWarning("Moving the recording to %s failed; the next sweep will delete it",
                 qPrintable(rescued));
    }
    KNotification::event(KNotification::Error,
                         i18n("Recording not saved"),
                         capture::recordingNotSavedMessage(moved ? rescued : file, moved));
}

void RecordingWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (!m_hull->isValid()) {
        // Outside a Plasma session `dialogs/background` is simply not there
        // (SPEC 3.2 point 4). The window then wears no hull — and that is the
        // whole difference: it stays opaque, and it stays usable.
        painter.fillRect(rect(), palette().color(QPalette::Window));
        QWidget::paintEvent(event);
        return;
    }

    m_hull->paint(painter);
    QWidget::paintEvent(event);
}

void RecordingWindow::resizeEvent(QResizeEvent *event)
{
    m_hull->resizeToWindow();
    QWidget::resizeEvent(event);
}

bool RecordingWindow::event(QEvent *event)
{
    // The fonts of the three labels were set on them by hand, so they do not
    // follow the application font by themselves (issue #68).
    if (event->type() == QEvent::ApplicationFontChange) {
        const QFont small = platform::smallestReadableFont();
        m_appName->setFont(small);
        m_hint->setFont(small);
        m_elapsed->setFont(clockFont());
        resize(width(), sizeHint().height());
    }

    // The pixel ratio of the window is not settled when show() returns: under
    // Wayland Qt reports 2 first and 1,6 about a second later, and it delivers
    // that as a DevicePixelRatioChange **without** a Resize beside it. A hull
    // that is only redrawn out of resizeEvent() would keep drawing at the
    // first number for good.
    if (event->type() == QEvent::DevicePixelRatioChange) {
        m_hull->resizeToWindow();
        update();
    }

    // Both text classes may be drawn in a colour of the scheme, so they have to
    // be written over again whenever the scheme moves (issue #54: a colour
    // taken once and kept stays put).
    if (event->type() == QEvent::PaletteChange) {
        applyTextColours();
    }

    return QWidget::event(event);
}
