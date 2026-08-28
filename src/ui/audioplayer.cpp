#include "ui/audioplayer.h"

#include "platform/systemfonts.h"
#include "ui/timestampformat.h"

#include <KLocalizedString>

#include <QAudioOutput>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMediaPlayer>
#include <QProgressBar>
#include <QToolButton>
#include <QUrl>

#include <algorithm>

AudioPlayer::AudioPlayer(QWidget *parent)
    : QWidget(parent)
    , m_player(new QMediaPlayer(this))
    , m_playPause(new QToolButton(this))
    , m_progress(new QProgressBar(this))
    , m_time(new QLabel(this))
{
    // QMediaPlayer keeps a plain pointer to its output and neither owns it nor
    // creates one of its own: without this line the file plays silently.
    m_player->setAudioOutput(new QAudioOutput(this));

    // The one symbol-only button of the application, and the exception is the
    // wireframe's: a play button is drawn as a symbol in every media player
    // there is, and the KDE HIG allow the symbol alone where it is that
    // established. It still says what it is — tooltip and accessible name are
    // set in updateButton(), which the playback state calls again.
    m_playPause->setAutoRaise(true);
    m_playPause->setFocusPolicy(Qt::TabFocus);

    // Progress, not a control: the bar shows where the playback stands and
    // takes nothing (wireframe 1b). Hence no text of its own — the time stands
    // beside it in the form the wireframe writes it — and no focus.
    m_progress->setTextVisible(false);
    m_progress->setFocusPolicy(Qt::NoFocus);

    // Small and dimmed like every other secondary text of this window. The
    // property is what a change of the system font finds the label by
    // (issue #68); a font set by hand does not follow the application font.
    m_time->setFont(platform::smallestReadableFont());
    m_time->setProperty(platform::FontSetByHand.data(), true);
    m_time->setForegroundRole(QPalette::PlaceholderText);

    connect(m_playPause, &QToolButton::clicked, this, [this] {
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->pause();
        } else {
            m_player->play();
        }
    });

    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &AudioPlayer::updateButton);
    connect(m_player, &QMediaPlayer::positionChanged, this, &AudioPlayer::updateProgress);
    // The file's own length, which arrives once the player has opened it and
    // overrides what the note remembers.
    connect(m_player, &QMediaPlayer::durationChanged, this, &AudioPlayer::updateProgress);

    auto *row = new QHBoxLayout(this);
    row->addWidget(m_playPause);
    // The bar takes the surplus width, the button and the time keep theirs
    // (wireframe 1b). Said out loud rather than left to the size policies: the
    // label would grow with the row otherwise, and the time would drift away
    // from the right edge.
    row->addWidget(m_progress, 1);
    row->addWidget(m_time);

    updateButton();
    updateProgress();
}

void AudioPlayer::setSource(const QString &file, int durationSeconds)
{
    // Whatever was playing belongs to the note the user has just left.
    stop();

    m_noteDurationMs = qint64(durationSeconds) * 1000;
    m_player->setSource(file.isEmpty() ? QUrl() : QUrl::fromLocalFile(file));

    updateProgress();
}

void AudioPlayer::stop()
{
    m_player->stop();
    updateProgress();
}

void AudioPlayer::updateButton()
{
    const bool playing = m_player->playbackState() == QMediaPlayer::PlayingState;
    const QString activity = playing ? i18nc("@action:button", "Pause") : i18nc("@action:button", "Play");

    m_playPause->setIcon(QIcon::fromTheme(playing ? QStringLiteral("media-playback-pause")
                                                  : QStringLiteral("media-playback-start")));
    m_playPause->setToolTip(activity);
    m_playPause->setAccessibleName(activity);
}

void AudioPlayer::updateProgress()
{
    // The file beats the note: `duration()` is what the player read out of it,
    // and it is zero until the file is open.
    const qint64 total = m_player->duration() > 0 ? m_player->duration() : m_noteDurationMs;
    const qint64 position = std::clamp(m_player->position(), qint64(0), std::max(total, qint64(0)));

    // A QProgressBar whose minimum and maximum are both zero draws a busy
    // indicator that runs for ever — which is what a note without a length
    // would get.
    m_progress->setMaximum(total > 0 ? static_cast<int>(total) : 1);
    m_progress->setValue(static_cast<int>(position));

    m_time->setText(i18nc("@info elapsed and total playing time of a voice note",
                          "%1 / %2",
                          library::clockTime(position),
                          library::clockTime(total)));
}
