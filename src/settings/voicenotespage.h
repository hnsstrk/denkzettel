#pragma once

#include <QString>
#include <QWidget>

class ModelDownload;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

/**
 * The page "Voice notes" of SPEC 13: which model whisper.cpp recognises with,
 * and where the program lies (SPEC 12).
 *
 * Two things here are not the plain `kcfg_` pattern of the other pages, and
 * both are the acceptance criteria of issue #27:
 *
 * - **The path field is two widgets.** The visible one is the editor, a hidden
 *   one carries the `kcfg_` name and is what the dialog's manager stores. A
 *   path that names no executable program never reaches the hidden one, so the
 *   value in `denkzettelrc` cannot be overwritten by a rejected one — and that
 *   needs no moment between the click on Apply and the manager writing, which
 *   there is none of.
 * - **Every model size is offered, and one that is not on disk is greyed.**
 *   Choosing it would put a file that is not there in front of the queue: two
 *   failed attempts and the tray in its error state (SPEC 12).
 *
 * **The download of issue #23 hangs on the same two widgets.** A size that is
 * not on disk is offered like any other; choosing it asks once, naming the
 * size and how big the file is, and then fetches it — the line under the list
 * carries the progress while it runs and the button beside the list is what
 * stops it (UX decision, 29.08.2026). There is no window of its own for it,
 * and the download does not belong to this page: it is the daemon's, handed
 * in, so that closing the dialog does not throw a file of gigabytes away.
 *
 * And it is where a `ModelPath` that could not be migrated is reported:
 * migrateModelPath() leaves such a key standing, this page says so with the
 * old path in the sentence, and the next Apply takes the key away. The key is
 * the whole of that state — there is no second mark anywhere saying a
 * migration happened (customer decision, 29.08.2026).
 */
class VoiceNotesPage : public QWidget
{
    Q_OBJECT

public:
    /** `download` outlives the dialog and is not owned by this page. */
    explicit VoiceNotesPage(ModelDownload *download, QWidget *parent = nullptr);

private:
    void browseForProgram();
    /** Asks about the model behind `index` and fetches it if it is wanted. */
    void chooseSize(int index);
    void showProgress(qint64 received, qint64 total);
    void downloadEnded(const QString &size, const QString &error);
    /** Takes the path over into the stored field if it is an executable. */
    void takeProgram(const QString &path);
    /** Says where the chosen model is expected, as long as it is not there. */
    void showModelState();
    /** Drops the `ModelPath` this page has been reporting, after a save. */
    void forgetTheEarlierPath();

    /** The unmigratable `ModelPath`, empty once there is none to report. */
    QString m_earlierPath;
    ModelDownload *m_download;
    /**
     * Which entry the list shows, and which it showed before that.
     *
     * **A click on a size that is not on disk must not change the setting.**
     * The list moves with the click before anything of ours runs, so the
     * choice is put straight back to `m_shownBefore` and only a download that
     * really finished moves it — a size that is on its way is not a size the
     * dialog may write. Otherwise Apply would store a model that is not
     * there: the queue takes that over at once (SPEC 13) and every note in the
     * window would wait on a file still arriving.
     */
    int m_shown = -1;
    int m_shownBefore = -1;
    /** The entry this page has fetched the model for itself, else -1. */
    int m_fetched = -1;
    /** Whether the download that is ending was stopped by the button here. */
    bool m_cancelling = false;
    QComboBox *m_size;
    QLineEdit *m_program;
    QLineEdit *m_stored;
    QPushButton *m_cancel;
    QLabel *m_modelState;
    QLabel *m_programState;
};
