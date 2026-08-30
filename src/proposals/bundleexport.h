#pragma once

#include <QDate>
#include <QString>

struct Proposal;
class Store;

/**
 * What one run of exportBundle() left behind.
 */
struct BundleExportResult {
    /**
     * The file that was written; empty when nothing was written.
     *
     * Read back off the open handle rather than assembled a second time: the
     * name is chosen by trying to create it (see exportBundle()), so the name
     * this run *wanted* and the name it *got* are two different things
     * whenever a file of that name already stood there.
     */
    QString file;
    /** Why nothing was exported; empty when the run went through. */
    QString error;

    bool ok() const
    {
        return error.isEmpty();
    }
};

/**
 * Writes the collective note of one bundle suggestion into
 * `<vaultPath>/_INBOX/` and then deletes what it exported (SPEC 8.1, issue
 * #32).
 *
 * **Two steps in this order, and the order is the guarantee.** The file is
 * written first and the corpus is emptied only once that has succeeded, so no
 * road through this function loses a note without the note standing in a file:
 * a vault path that is missing, unwritable or full ends the run before
 * `Store::removeExportedBundle()` is reached. The other direction is the
 * cheaper failure — a deletion that fails after the file was written leaves
 * the notes standing beside a collective note that holds them, which is a
 * duplicate and not a loss.
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
 * with hyphens, `created` as an ISO day. The file name is the one SPEC 8.1
 * fixes, `Denkzettel <topic> <YYYY-MM-DD>.md`; `date` is passed in so a check
 * can hold it against a literal instead of against the same clock the code
 * read.
 *
 * Nothing here touches `INDEX.md` and nothing links to the file. `_INBOX/` is
 * staging and nobody points at it (user decision of 06.08.2026); a note earns
 * its links when the user files it out of staging, and not before.
 */
BundleExportResult exportBundle(Store &store,
                                const Proposal &proposal,
                                const QString &vaultPath,
                                const QDate &date = QDate::currentDate());
