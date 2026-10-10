#pragma once

#include <QDate>
#include <QString>
#include <QStringList>

struct Proposal;
class Store;

/**
 * What one run of exportBundle() left behind.
 */
struct BundleExportResult {
    /**
     * The files that were written — the collective note first, then one per
     * exported note; empty when nothing was written and empty after a failure,
     * because a run that fails after writing removes what it wrote.
     *
     * Read back off the open handles rather than assembled a second time: a
     * name is chosen by trying to create it (see exportBundle()), so the name
     * this run *wanted* and the name it *got* are two different things
     * whenever a file of that name already stood there. That is also what
     * makes the clean-up safe — it removes these names and no others.
     */
    QStringList files;
    /** Why nothing was exported; empty when the run went through. */
    QString error;

    bool ok() const
    {
        return error.isEmpty();
    }
};

/**
 * Writes one bundle suggestion into `<vaultPath>/_INBOX/` and then deletes
 * what it exported (SPEC 8.1, issue #32).
 *
 * **N+1 files, not one** (customer decision 10.10.2026): the collective note
 * of SPEC 8.1 and beside it a file per note. The collective note keeps the
 * bundle as an artefact of its own; the single files are what the user files
 * out of staging one by one. A note's file is named after its own stem — the
 * same one its audio file and its file in the full export of 8.3 carry
 * (`noteFileStem()`, SPEC 4) — and carries the note's text as its body. No
 * title is made out of that text: `_INBOX/` is exempt from the vault's title
 * rule, and the title is given when the user files the note (PO decision
 * 10.10.2026).
 *
 * **Nothing links the files of one bundle; the topic tag holds them
 * together.** `_INBOX/` is staging and nobody points at it (user decision
 * 06.08.2026), and a link between two staged files becomes a link into
 * `_INBOX/` the moment one of them is filed, because the vault updates links
 * on move (measured 03.09.2026). The tag survives the filing and points
 * nowhere. Nothing here touches `INDEX.md` either.
 *
 * **The files first, the deletion afterwards, and the order is the
 * guarantee.** No road through this function loses a note without the note
 * standing in a file: a vault path that is missing, unwritable or full ends
 * the run before `Store::removeExportedBundle()` is reached. And a run that
 * fails **after** it has written — a later file, or the deletion — removes
 * exactly the files it wrote, under the names it read back from its own
 * handles, checks on the file system that they are gone and says what it could
 * not remove (customer decision 30.08.2026). At that moment the corpus is
 * unchanged, the deletion runs in one transaction and rolls back, so removing
 * the files restores the state before the attempt. Without it every retry
 * would leave another set of orphans in `_INBOX/`.
 *
 * **What is exported is what the suggestion carries.** `proposal.payload`
 * holds the title and the Markdown that Suggester wrote (SPEC 7.3) and that
 * the review of SPEC 9 showed the user; it is written out rather than built
 * again here, so what lands in the vault is what was on the card. The review
 * of #30 lets notes be deselected, and it hands in a `Proposal` whose
 * `noteIds` and Markdown both reflect that selection — deselected notes are in
 * neither, and are therefore neither exported nor deleted.
 *
 * The frontmatter follows the conventions of the user's vault, researched for
 * this story on 30.08.2026: `type: note`, `tags` as a block list in lower case
 * with hyphens, `created` as an ISO day. `date` is passed in so a check can
 * hold the collective note's name and its `created` against a literal instead
 * of against the same clock the code read; a note's file dates itself from the
 * note.
 */
BundleExportResult exportBundle(Store &store,
                                const Proposal &proposal,
                                const QString &vaultPath,
                                const QDate &date = QDate::currentDate());
