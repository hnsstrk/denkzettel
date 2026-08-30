#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

class KStatusNotifierItem;
class QDialog;
class QMenu;

/**
 * Permanent tray presence of the daemon (SPEC 10).
 */
class TrayIcon : public QObject
{
    Q_OBJECT

public:
    explicit TrayIcon(QObject *parent = nullptr);

    /**
     * The tray item, for reading only: what the item announces to the host is
     * only knowable by asking the item itself (issue #44). Const because
     * everything the item is told is decided in the constructor.
     */
    const KStatusNotifierItem *item() const;

public Q_SLOTS:
    /**
     * How many notes the transcription queue has given up on (SPEC 12), and
     * with it the error state of SPEC 10; `0` takes both back (issues #24,
     * #118).
     *
     * **A count and no longer the reason it failed** (UX decision of
     * 29.08.2026): the reason of one job pushed the second kind of trouble
     * below out of the one subtitle line, and it is written where SPEC 14
     * sorts it — the log. What is left here is the number, which is what a
     * quiet channel can carry beside another one.
     *
     * `NeedsAttention` and no badge of our own: that is the state the tray
     * protocol has for exactly this, and Plasma is what decides how loudly the
     * icon is set apart (customer decision of 29.08.2026, against the quieter
     * overlay). It is the only state beside the normal one — a "transcribing
     * right now" does not exist and is not wanted (SPEC 14: the tray is quiet,
     * the log is detailed).
     */
    void setNotesWithoutTranscript(int count);

    /**
     * How many notes the analysis run has given up on (SPEC 7.2), and with it
     * the same error state as above; `0` takes it back (issue #118).
     *
     * The second source of the one subtitle line, and it carries **no
     * ranking** against the first: both stand for as long as their cause does,
     * so a ranking would keep the user from ever learning of the one that lost
     * (UX decision of 29.08.2026). What is counted is the row "Unclassified"
     * of the library's category column — the place such a note can be worked
     * off, without which this state would be a message with no way out.
     *
     * **No KNotification goes with it**, unlike the transcription of issue
     * #115: the analysis run is the routine run of SPEC 10, and a note left
     * without a category is readable in the library all the same — it is
     * missing its category and its tags, not its text.
     */
    void setNotesWithoutCategory(int count);

    /**
     * Names what of the optional tools of SPEC 2.5 this machine cannot offer —
     * `ffmpeg`, `whisper-cli`, `task`, and `Ollama` when the server does not
     * answer (issue #17). An empty list takes the statement back.
     *
     * **No error state comes with it**, unlike the two slots above, and that is
     * the difference between them: a tool that is not installed is not a fault
     * that happened, it is how this machine stands. A `NeedsAttention` raised
     * at every login that never falls again is the permanent finding nobody
     * reads any more — the reasoning of issue #118, which is where the
     * question of two sources in one line was settled.
     */
    void setUnavailableTools(const QStringList &names);

    /**
     * What the transcription queue is waiting for and where to get it — the
     * model of SPEC 12 that is not on disk (issue #23). Empty takes it back.
     *
     * The third part of the one subtitle line and, like the tools above, no
     * error state: a model that has not been fetched yet is a precondition
     * not yet met, and SPEC 12 says in as many words that it is not a failed
     * attempt. The queue is standing still, nothing has gone wrong, and the
     * note is untouched.
     */
    void setMissingModel(const QString &report);

Q_SIGNALS:
    void captureRequested();
    void recorderRequested();
    void libraryRequested();
    void analysisRequested();
    void configureRequested();

private:
    QMenu *buildMenu();
    /** Writes the one subtitle line out of whatever the two slots have said. */
    void showToolTip();

    int m_notesWithoutTranscript = 0;
    int m_notesWithoutCategory = 0;
    QString m_missingModel;
    QStringList m_unavailableTools;
    KStatusNotifierItem *m_item;
    /**
     * The open about dialog, or nothing.
     *
     * It deletes itself on close, so the pointer has to notice that by itself;
     * a raw one would be dangling the second time the entry is used.
     */
    QPointer<QDialog> m_about;
};
