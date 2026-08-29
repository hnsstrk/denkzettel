#pragma once

#include "capture/windowhull.h"

#include <QDateTime>
#include <QList>
#include <QWidget>

class AudioRecorder;
class Store;
class QLabel;
class QTimer;

namespace capture
{
class LevelMeter;
class RecordingDot;

/**
 * What the user is told when a finished recording could not be stored, with
 * `path` naming where the recording now lies (SPEC 2.5, 14).
 *
 * Two sentences, and `rescued` picks between them, because only one of the two
 * outcomes may carry a promise. **Naming the file was not enough** (finding of
 * the review of 29.08.2026): the message said the recording "will not be
 * deleted" while `Store::sweepOrphanedAudio()` removed it at the very next
 * service start, because nothing pointed at it — measured, the file was gone
 * after one sweep. The promise is true only once the file stands outside the
 * directory the sweep reads, so it is made only then; where the move itself
 * failed, the message says what will happen instead of promising.
 *
 * A function of its own because this sentence is what issue #25 hangs on, and
 * because a KNotification cannot be read back on a bus that has no
 * notification server (CLAUDE.md, finding 37).
 */
QString recordingNotSavedMessage(const QString &path, bool rescued);
}

/**
 * Frameless window for a voice note (SPEC 4).
 *
 * Of the same make as the capture window and wearing the same hull — the two
 * share `capture::WindowHull` and nothing else. The recording **runs from the
 * moment the window opens**: there is no start button, and the window that
 * stands there is the recording.
 *
 * What it shows is wireframe 1f, at the five points the drawing lost against
 * the built capture window (decision of 29.08.2026, noted in the drawing
 * itself): 600 px wide like the capture window, the heading in the note text's
 * colour, no parting line above the footer, seven bars for the level, a
 * resting dot — and from minute 14 the running time turns to the colour scheme's
 * negative text while the footer says when the recording will end.
 *
 * **The note is made when the recording is finished, not when the key is
 * pressed** (the lesson of issue #22). A note created earlier would be in the
 * transcription queue before its file was closed, and both attempts of SPEC 12
 * would be spent in no time at all on a file that is not there yet.
 *
 * **And a recording that cannot be stored is reported with its path**
 * (SPEC 2.5, addition of 29.08.2026). The file lies on the disk before the row
 * does; if `addNote()` fails, what stays behind is a file with no row — which
 * the cleanup check of SPEC 2.5 cannot tell from a harmless orphan and would
 * delete at the next start of the service. Naming the file is what keeps that
 * from being the second half of a data loss.
 */
class RecordingWindow : public QWidget
{
    Q_OBJECT

public:
    /** `store` outlives the window and is not owned by it. */
    explicit RecordingWindow(Store *store, QWidget *parent = nullptr);
    ~RecordingWindow() override;

    /**
     * The recording this window shows — for a check, which feeds it a signal
     * of its own instead of the room the user sits in.
     */
    AudioRecorder *recorder() const;

    /**
     * Starts a recording whose samples the caller hands in through recorder(),
     * and brings the window up for it. Returns what the recorder said.
     *
     * The road without a microphone, and it is public for the reason
     * `AudioRecorder::encode()` is: **no check in this repository opens the
     * user's microphone** (their instruction of 28.08.2026), and what this
     * window does with a finished recording — the note, and the failure that
     * must not lose one — cannot be reached without starting one.
     */
    bool startWithoutADevice();

public Q_SLOTS:
    /** Brings the window up with the keyboard focus and starts recording. */
    void showRecorder();

    /** Re-reads the desktop theme and rebuilds the hull on a standing window. */
    void reloadDesktopTheme(const QString &name = {});

protected:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void present();
    /** Takes the note's moment and starts the recorder, with or without a device. */
    bool beginRecording(bool withADevice);
    void save();
    void discard();
    void applyHull();
    void applyTextColours();
    /** Writes the running time and, from minute 14, what the footer then says. */
    void showElapsed();
    void storeRecording(const QString &fileName, int durationSeconds);

    Store *m_store;
    AudioRecorder *m_recorder;
    /**
     * The moment the recording started, and therefore the note's `created_at`
     * and the stem of the file name (SPEC 4, 5.1). Taken once at the start:
     * asked again when the note is written it would name a different file
     * from the one on the disk.
     */
    QDateTime m_createdAt;
    /**
     * Whether a recording is still to be answered for — set at the start and
     * cleared by whichever of finished(), cancelled() and failed() arrives.
     *
     * ponytail: a second Meta+Shift+N during that gap only brings the window
     * back up and starts nothing, because the recorder refuses a second
     * recording anyway. The gap is the muxer closing the file, under two
     * seconds. Ceiling: whoever hits the shortcut inside it sees a window that
     * hides itself again when the earlier recording is stored. The way up is
     * a recorder per recording rather than one per window — which then has to
     * be kept alive past its own signal, because its destructor deletes the
     * file of a recording it never answered for.
     */
    bool m_awaitingAnswer = false;

    QLabel *m_appName;
    capture::RecordingDot *m_dot;
    capture::LevelMeter *m_meter;
    QLabel *m_elapsed;
    QLabel *m_hint;
    /**
     * Writes the running time. It reads AudioRecorder::duration(), which
     * counts the frames the encoder took — the same clock the upper bound of
     * SPEC 4 is measured on, so the display and the bound cannot disagree.
     */
    QTimer *m_clock;

    /** The same hull the capture window wears (SPEC 4: "same make as capture"). */
    capture::WindowHull *m_hull;
};
