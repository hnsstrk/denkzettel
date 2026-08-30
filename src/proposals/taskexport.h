#pragma once

#include "platform/optionaltools.h"

#include <QString>
#include <QStringList>

/**
 * What one run of exportTaskProposal() left behind.
 *
 * The three fields are read in this order: `error` says whether the card has
 * something to report, `uuid` whether a task exists, `annotated` whether the
 * note text is attached to it.
 *
 * **`uuid` and `error` can both be filled**, and a caller that only asks
 * `ok()` gets that case wrong. It is the road on which the annotation fails
 * after the task has been created: the task is in Taskwarrior, the note text
 * is not, and a second attempt at the same suggestion would create the task a
 * second time. SPEC 8.2 keeps the suggestion open on every error, so it is the
 * user who decides — and they can only decide it if the card says the task
 * exists.
 *
 * **An empty `uuid` beside an error is therefore no promise that nothing was
 * created.** The time limit is the case: an `on-add` hook that hangs after
 * Taskwarrior has written the task leaves a finished task behind, and the call
 * is killed before its output can be read — the same result as an add that
 * never happened. There is no way to tell the two apart from here, which is
 * why the message names the time limit rather than claiming nothing was
 * written. The card cannot say more than that, and a second attempt after a
 * timeout may duplicate.
 */
struct TaskExportResult {
    /** The UUID Taskwarrior gave the task; empty when none was created. */
    QString uuid;
    /** Whether the note text was attached to it as an annotation. */
    bool annotated = false;
    /** What goes on the card; empty when the run went through. */
    QString error;

    bool ok() const
    {
        return error.isEmpty();
    }
};

/**
 * The arguments of `task add` for one task suggestion, program name excluded —
 * the command line SPEC 8.2 lays down, built out of the payload of SPEC 7.2.
 *
 * Empty when the payload carries no description: SPEC 5.1 makes the
 * description the statement that there is a task at all, and Taskwarrior
 * refuses an add without one anyway.
 *
 * **The `--` before the description is the whole security of this function**,
 * and it is a measurement, not a precaution (30.08.2026, Taskwarrior 3.5.0).
 * An argument array keeps the *shell* out, and that is the smaller half:
 * Taskwarrior parses its own arguments and reads `project:`, `+tag` and
 * `due:` out of them wherever they stand. A note saying
 * `project:secret +evil due:tomorrow really` came back as
 * `A task must have a description.` with return value 2 — every word of it had
 * been eaten as a field. Behind `--` the same text lands in `description`
 * character for character, and the fields written before it still take effect.
 *
 * Two things the payload can carry that Taskwarrior cannot, and only one of
 * them says so:
 * - a **tag with whitespace** is dropped here. `+two words` is no tag for
 *   Taskwarrior; it falls into the description, and the add goes through with
 *   return value 0 — the tag is gone and the description carries a word nobody
 *   wrote. Left out, the task is short of a tag and says what it is about.
 * - a **project with whitespace** is passed on. Measured in the same run:
 *   `project:home stuff` as one argument becomes the project `home stuff`,
 *   whole. There is nothing to protect here.
 *
 * `due` and `priority` are handed on unchecked. The classification of SPEC 7.2
 * already holds the one against the note's own day and the other against the
 * three letters Taskwarrior knows; a second check here would be the same rule
 * in a second place, and what a wrong value costs is a return value of 2 whose
 * message goes on the card.
 */
QStringList taskAddArguments(const QString &payload);

/**
 * The arguments of `task <uuid> annotate` for `text`, program name excluded.
 *
 * The `--` is worth more here than at the add, and that too is measured
 * (30.08.2026): with the same text and without the separator, Taskwarrior
 * attached **no** annotation and instead overwrote the task's `project` with
 * `stolen +hijack the real annotation` — return value 0, not a word. The note
 * text was gone and a field of the user's task had been rewritten. That is the
 * shape SPEC 8.2 must not have: nothing is lost.
 *
 * **The verbosity setting rides along although nothing here reads a UUID**,
 * and it is what keeps Taskwarrior's housekeeping off the user's card. Measured
 * on 3.5.0 on 30.08.2026 in production form, a failing annotate wrote
 * `Configuration override rc.confirmation=no` on the line **above** its real
 * message, and exportTaskProposal() passes standard error on whole. With the
 * setting it reads `No tasks specified.` and nothing else. `rc.verbose=nothing`
 * is the wrong lever for the same job: it empties standard error altogether,
 * and the card would fall back to the bare return value.
 */
QStringList taskAnnotateArguments(const QString &uuid, const QString &text);

/**
 * Carries one confirmed task suggestion into Taskwarrior (SPEC 8.2):
 * `task add` with the populated fields of `payload`, and where `noteText` says
 * more than the description does, `task <uuid> annotate` with the note's own
 * text.
 *
 * `payload` is the JSON object of SPEC 7.2 as `Proposal::payload` carries it;
 * `noteText` is the content of the note it was extracted from.
 *
 * **It touches neither the store nor the suggestion.** SPEC 8.2 has the note
 * deleted and the suggestion removed after a success, and that is the road the
 * review of SPEC 9 walks with the result of this function in its hand — on an
 * error it does not walk it, which is what leaves the suggestion open and the
 * note where it is.
 *
 * `program` is what gets started, by default the `task` of SPEC 2.5 along
 * PATH. It is a parameter so a check can put a stand-in in its place, the way
 * SPEC 12 does for whisper-cli — and because the run must never depend on the
 * checking machine having Taskwarrior installed.
 */
TaskExportResult exportTaskProposal(const QString &payload,
                                    const QString &noteText,
                                    const QString &program = tools::TaskProgram);
