#include "proposals/bundleexport.h"

#include "store/note.h"
#include "store/proposal.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

#include <optional>

namespace
{

/**
 * How many files of one name `_INBOX/` may hold.
 *
 * A ceiling on the counting-up loop in writeNewFile() rather than a rule about
 * vaults: the loop goes on while the name it tried is taken, and a directory
 * that answers "exists" for a name it will not open would otherwise spin. A
 * hundred files of one name is a state to report, not to serve.
 */
constexpr int NameAttempts = 100;

/**
 * One tag in the form the vault keeps them: lower case, words joined by
 * hyphens.
 *
 * German special characters survive — `ü` stays `ü` and never becomes `ue`;
 * the vault checks that rule over its whole tree, and a tag is the one piece
 * of the frontmatter that carries the user's own language. What is dropped is
 * only what YAML or Obsidian would read as something other than a tag, which
 * is why the result needs no quoting in the block list below.
 */
QString vaultTag(const QString &tag)
{
    // The lower-case copy into a named const, not iterated as a temporary: a
    // range loop over a non-const Qt container detaches it
    // (-Wclazy-range-loop-detach), and the CI fails on every linter finding.
    const QString lowered = tag.toLower();
    QString result;
    result.reserve(lowered.size());
    for (const QChar character : lowered) {
        if (character.isLetterOrNumber() || character == QLatin1Char('-') || character == QLatin1Char('_')
            || character == QLatin1Char('/')) {
            result.append(character);
        } else if (!result.endsWith(QLatin1Char('-'))) {
            // Spaces and punctuation become the separator, and two of them in
            // a row stay one — `KI, Pipeline` is `ki-pipeline`, not
            // `ki--pipeline`.
            result.append(QLatin1Char('-'));
        }
    }
    while (result.endsWith(QLatin1Char('-'))) {
        result.chop(1);
    }
    while (result.startsWith(QLatin1Char('-'))) {
        result.remove(0, 1);
    }
    return result;
}

/**
 * The frontmatter of one exported file, against the vault's conventions.
 *
 * `tags` carries `denkzettel` as the fixed mark of where the file came from,
 * and beneath it the tags the caller hands in. Those come out of the AI
 * classification (SPEC 7.2) and are **not** in the vault's
 * `_META/taxonomy.md`, so some of them will read as unknown there. That is
 * right rather than a reason to leave them out: they are the information the
 * export is made for, this is the one step where the original note is deleted,
 * and `_INBOX/` is explicitly the staging area where unsorted material belongs
 * until the user files it.
 *
 * `created` is the day the contents belong to, which is why it is a parameter:
 * for the collective note it is the day of the **export**, because what stands
 * in it spans several days and each of them has its own `## <YYYY-MM-DD>`
 * section in the body; for a note's own file it is the day of that note, which
 * is the one day the file is about.
 */
QString frontmatter(const QStringList &tags, const QDate &date)
{
    QString text = QStringLiteral("---\ntype: note\ntags:\n  - denkzettel\n");
    for (const QString &tag : tags) {
        text += QStringLiteral("  - %1\n").arg(tag);
    }
    text += QStringLiteral("created: %1\n---\n\n").arg(date.toString(Qt::ISODate));
    return text;
}

/** The tags of every note handed in, in the vault's form, sorted and unique. */
QStringList collectedTags(const Store &store, const QList<qint64> &noteIds)
{
    QSet<QString> found;
    for (const qint64 id : noteIds) {
        const QStringList tags = store.tags(id);
        for (const QString &tag : tags) {
            const QString normalised = vaultTag(tag);
            // A tag that is nothing but punctuation comes out empty, and an
            // empty list entry is not a tag.
            if (!normalised.isEmpty() && normalised != QLatin1String("denkzettel")) {
                found.insert(normalised);
            }
        }
    }
    QStringList sorted(found.cbegin(), found.cend());
    sorted.sort();
    return sorted;
}

/**
 * The `tags` list of one file: the topic of the bundle, then the note tags.
 *
 * **The topic tag is what holds the N+1 files of one export together, and it
 * is there because nothing links them** (UX recommendation 03.09.2026, PO
 * decision 10.10.2026). A tag groups the files in Obsidian's tag view,
 * survives the filing out of `_INBOX/` and points nowhere, which a link does
 * not: the vault updates links on move, so a link between two staged files
 * turns into a link into `_INBOX/` as soon as one of them is filed.
 *
 * The topic is dropped out of the note tags rather than listed twice — a
 * bundle named "Backup" over notes tagged `backup` is the ordinary case, not a
 * curiosity.
 */
QStringList fileTags(const QString &topicTag, const QStringList &noteTags)
{
    QStringList tags;
    tags.reserve(noteTags.size() + 1);
    if (!topicTag.isEmpty()) {
        tags.append(topicTag);
    }
    for (const QString &tag : noteTags) {
        if (tag != topicTag) {
            tags.append(tag);
        }
    }
    return tags;
}

/**
 * The topic as a file name may carry it.
 *
 * The title is what a language model wrote, so it carries whatever the
 * language brings. Only what a path cannot hold is taken out — the separators
 * and the control characters; umlauts stay, because the name is read by a
 * person and the vault forbids replacing them. A leading dot would hide the
 * file from Obsidian, which is the one further case worth a line.
 */
QString fileNameTopic(const QString &title)
{
    QString result;
    result.reserve(title.size());
    for (const QChar character : title) {
        if (character == QLatin1Char('/') || character == QLatin1Char('\\') || character.category() == QChar::Other_Control) {
            result.append(QLatin1Char(' '));
        } else {
            result.append(character);
        }
    }
    result = result.simplified();
    while (result.startsWith(QLatin1Char('.'))) {
        result.remove(0, 1);
    }
    return result.trimmed();
}

/**
 * Writes `content` into `<pathStem>.md`, counting the name up while it is
 * taken, and appends the name the file really got to `written`.
 *
 * The name is taken by **creating** the file, not by asking whether it exists
 * and writing afterwards: between those two steps lies a window in which
 * somebody else writes the same name, and what would follow is an overwrite —
 * the one outcome that really loses data here, because the notes of an earlier
 * export are already deleted by then. `NewOnly` is O_EXCL, so a name that is
 * taken fails to open and the counter goes up. A file of that name standing
 * there can only be the leftover of an earlier failure that was not swept, or
 * a copy of the user's own; neither may be written over.
 *
 * The name goes into `written` straight after the `open()` succeeded and not
 * after the write — a short write leaves a torso behind, and the caller's
 * clean-up is the one road that takes it away again.
 *
 * The content arrives as bytes and the caller encodes it: UTF-8 and nothing
 * else, because the umlauts have to come out of the file the way they went
 * into the note, and a local 8-bit codec would turn every one of them into a
 * question mark without a word.
 */
bool writeNewFile(const QString &pathStem, const QByteArray &content, QStringList &written, QString &error)
{
    QFile file;
    for (int attempt = 1;; ++attempt) {
        file.setFileName(attempt == 1 ? pathStem + QStringLiteral(".md")
                                      : pathStem + QStringLiteral(" %1.md").arg(attempt));
        if (file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
            break;
        }
        // Told apart, because the two need different answers: a name that is
        // taken is counted up, everything else — a folder with no write
        // permission, a read-only file system, a full disk — ends the run.
        if (!QFileInfo::exists(file.fileName())) {
            error = i18n("The file %1 could not be written: %2", file.fileName(), file.errorString());
            return false;
        }
        if (attempt >= NameAttempts) {
            const QFileInfo wanted(pathStem);
            error = i18n("The folder %1 already holds %2 files named %3.",
                         wanted.path(),
                         NameAttempts,
                         wanted.fileName());
            return false;
        }
    }

    // The name off the handle, not the one assembled above: what the caller
    // reports and what it may have to remove again is the file that exists.
    written.append(QFileInfo(file).absoluteFilePath());

    if (file.write(content) != content.size() || !file.flush()) {
        error = i18n("The file %1 could not be written: %2", file.fileName(), file.errorString());
        return false;
    }
    file.close();
    return true;
}

}

BundleExportResult exportBundle(Store &store, const Proposal &proposal, const QString &vaultPath, const QDate &date)
{
    BundleExportResult result;

    // A task suggestion carries the fields of SPEC 7.4 and no Markdown; put
    // through here it would write a file of nothing and delete the note behind
    // it. Taskwarrior is the other road (SPEC 8.2).
    if (proposal.kind != Proposal::Kind::Bundle) {
        result.error = i18n("Only a bundle suggestion can be exported to Obsidian.");
        return result;
    }

    const QJsonObject payload = QJsonDocument::fromJson(proposal.payload.toUtf8()).object();
    const QString title = payload.value(QLatin1String("title")).toString().trimmed();
    const QString markdown = payload.value(QLatin1String("markdown")).toString();
    const QString topic = fileNameTopic(title);
    if (topic.isEmpty() || markdown.isEmpty()) {
        result.error = i18n("The suggestion carries no collective note that could be exported.");
        return result;
    }

    // The path is empty until the user sets it (issue #75), and that is the
    // ordinary state rather than a fault — so it gets a sentence of its own
    // instead of the one about a folder that is not there.
    if (vaultPath.isEmpty()) {
        result.error = i18n("No vault folder is set. The export writes into the Obsidian vault of the settings.");
        return result;
    }
    // Two sentences and not one, because the two are different mistakes: a
    // path that is not there was mistyped or has been moved, a path that is
    // there and is a file was pointed at the wrong thing.
    if (!QFileInfo::exists(vaultPath)) {
        result.error = i18n("The vault folder %1 does not exist.", vaultPath);
        return result;
    }
    if (!QFileInfo(vaultPath).isDir()) {
        result.error = i18n("%1 is not a folder.", vaultPath);
        return result;
    }

    const QString inbox = vaultPath + QStringLiteral("/_INBOX");
    // `_INBOX/` is flat and holds no subfolders, so this creates the one
    // directory SPEC 8.1 names and nothing beneath it. An existing one is left
    // as it is — mkpath answers true for that.
    if (!QDir().mkpath(inbox)) {
        result.error = i18n("The folder %1 could not be created.", inbox);
        return result;
    }

    // Every name this run read back off one of its own handles, in the order
    // they were written. The clean-up below works off this list and off nothing
    // else: a name rebuilt out of the stem and the folder would name the file
    // the run *wanted*, and that can be the leftover of an earlier failure or
    // a copy of the user's own rather than anything this run created.
    QStringList written;

    // Removes what this run wrote and hands back what is still there
    // afterwards. The file system and not the return value of remove() decides
    // that: this project has been bitten by a deletion that reported success
    // (CLAUDE.md, finding 26).
    const auto undoWrites = [&written] {
        QStringList left;
        for (const QString &path : written) {
            QFile::remove(path);
            if (QFileInfo::exists(path)) {
                left.append(path);
            }
        }
        return left;
    };

    // The message a failure after the first successful write reports: what went
    // wrong, and what the clean-up could not take away again. The corpus is
    // untouched at every point this is reached — the deletion is the last step
    // and rolls back as a whole — so with the files gone the vault stands as it
    // did before the attempt (customer decision 30.08.2026).
    const auto failed = [&undoWrites](const QString &message) {
        const QStringList left = undoWrites();
        if (left.isEmpty()) {
            return message;
        }
        return i18n("%1 What had already been written could not be removed: %2",
                    message,
                    left.join(QStringLiteral(", ")));
    };

    const QString topicTag = vaultTag(title);
    QString error;

    const QString collective = frontmatter(fileTags(topicTag, collectedTags(store, proposal.noteIds)), date) + markdown;
    if (!writeNewFile(inbox + QStringLiteral("/Denkzettel %1 %2").arg(topic, date.toString(Qt::ISODate)),
                      collective.toUtf8(),
                      written,
                      error)) {
        result.error = failed(error);
        return result;
    }

    // And beside the collective note one file per note (customer decision
    // 10.10.2026). Its body is the note's text and nothing else — for a voice
    // note the transcript, which is what the store holds there; the
    // `## <YYYY-MM-DD>` sections belong to the collective note.
    for (const qint64 id : proposal.noteIds) {
        const std::optional<Note> note = store.note(id);
        if (!note.has_value()) {
            // The library deleted the note while the suggestion stood.
            // `removeExportedBundle()` would refuse the whole call for it
            // below, so the run ends here — and it ends with the files it has
            // written removed, which leaves the state before the attempt.
            result.error = failed(i18n("The note %1 is no longer in the database.", id));
            return result;
        }

        QString body = note->content;
        if (!body.endsWith(QLatin1Char('\n'))) {
            body += QLatin1Char('\n');
        }
        const QString content =
            frontmatter(fileTags(topicTag, collectedTags(store, {id})), note->createdAt.date()) + body;
        if (!writeNewFile(inbox + QStringLiteral("/Denkzettel %1").arg(noteFileStem(note->createdAt)),
                          content.toUtf8(),
                          written,
                          error)) {
            result.error = failed(error);
            return result;
        }
    }

    // Only now, and only in one transaction (SPEC 8.1).
    if (!store.removeExportedBundle(proposal.noteIds, proposal.id)) {
        result.error = failed(i18n("The exported notes could not be deleted: %1", store.lastError()));
        return result;
    }

    result.files = written;
    return result;
}
