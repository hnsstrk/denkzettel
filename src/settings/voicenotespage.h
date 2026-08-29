#pragma once

#include <QString>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;

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
    explicit VoiceNotesPage(QWidget *parent = nullptr);

private:
    void browseForProgram();
    /** Takes the path over into the stored field if it is an executable. */
    void takeProgram(const QString &path);
    /** Says where the chosen model is expected, as long as it is not there. */
    void showModelState();
    /** Drops the `ModelPath` this page has been reporting, after a save. */
    void forgetTheEarlierPath();

    /** The unmigratable `ModelPath`, empty once there is none to report. */
    QString m_earlierPath;
    QComboBox *m_size;
    QLineEdit *m_program;
    QLineEdit *m_stored;
    QLabel *m_modelState;
    QLabel *m_programState;
};
