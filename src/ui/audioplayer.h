#pragma once

#include <QWidget>

class QLabel;
class QMediaPlayer;
class QProgressBar;
class QToolButton;

/**
 * The audio player of the reading pane (SPEC 9, wireframe 1b): a play/pause
 * button, a progress display and the running time as "0:14 / 0:41", in that
 * order and with nothing else in the row.
 *
 * **Three parts, because three are drawn.** The wireframe carries this player
 * four times over (1b, 2a, 2b, 3a) and it looks the same every time. A volume
 * control, a handle on the bar, a playback speed and a waveform appear in none
 * of them and are therefore not built; the level meter of wireframe 1f belongs
 * to the recording window and only there. The bar is drawn as a display and
 * not as a control, so it does not seek — making it seek is a question for the
 * customer, not a decision taken here.
 *
 * The player sits above the transcript and stands on its own: a voice note may
 * exist without a transcript (SPEC 12, error path), and then the row below it
 * is empty while this row is unchanged.
 *
 * **Without the box the wireframe draws around the three.** It was built as a
 * `QFrame::StyledPanel` first; measured 2026-08-28 under `Breeze::Style`,
 * `frameWidth()` comes back 0 and the picture shows no outline — Breeze draws
 * no frame around a plain QFrame, and drawing one by hand would be a
 * stylesheet or a paintEvent for a decoration nothing depends on. What the box
 * says — that the three belong together — the row says by standing on one line
 * between the head row and the transcript. If the customer wants the outline,
 * it is one paintEvent.
 */
class AudioPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit AudioPlayer(QWidget *parent = nullptr);

    /**
     * Puts `file` under the button, rewound to the start; an empty path leaves
     * the player without a source.
     *
     * `durationSeconds` is what the note carries (`Note::audioDurationS`) and
     * what the total reads until the player has opened the file and knows
     * better. Without it the row would show "0:00 / 0:00" until the first
     * press, and the length of a voice note is what one reads before deciding
     * to listen at all.
     */
    void setSource(const QString &file, int durationSeconds);

    /** Stops what is playing and rewinds to the start. */
    void stop();

private:
    /** Writes position and total into the bar and the label. */
    void updateProgress();

    /** Sets symbol, tooltip and accessible name of the one button. */
    void updateButton();

    QMediaPlayer *m_player;
    QToolButton *m_playPause;
    QProgressBar *m_progress;
    QLabel *m_time;

    /** What the note says its file is long, in milliseconds. */
    qint64 m_noteDurationMs = 0;
};
