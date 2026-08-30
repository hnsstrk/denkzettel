#include "proposals/bundleexport.h"

#include "store/proposal.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace
{

/**
 * How many collective notes of one topic and day `_INBOX/` may hold.
 *
 * A ceiling on the counting-up loop in exportBundle() rather than a rule about
 * vaults: the loop goes on while the name it tried is taken, and a directory
 * that answers "exists" for a name it will not open would otherwise spin. A
 * hundred exports of one topic on one day is a state to report, not to serve.
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
 * The frontmatter of the collective note, against the vault's conventions.
 *
 * `tags` carries `denkzettel` as the fixed mark of where the note came from,
 * and beneath it the tags of the exported notes as a union. Those come out of
 * the AI classification (SPEC 7.2) and are **not** in the vault's
 * `_META/taxonomy.md`, so some of them will read as unknown there. That is
 * right rather than a reason to leave them out: they are the information the
 * export is made for, this is the one step where the original note is deleted,
 * and `_INBOX/` is explicitly the staging area where unsorted material belongs
 * until the user files it.
 *
 * `created` is the day of the **export**, because that is when this file came
 * into being. When the notes in it were written stands in the body, at each
 * `## <YYYY-MM-DD>` section — putting it up here would make the field claim
 * something about the file that belongs to its contents.
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

/** The tags of every exported note, in the vault's form, sorted and unique. */
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

    const QString content = frontmatter(collectedTags(store, proposal.noteIds), date) + markdown;

    // The name is taken by **creating** the file, not by asking whether it
    // exists and writing afterwards: between those two steps lies a window in
    // which somebody else writes the same name, and what would follow is an
    // overwrite — the one outcome that really loses data here, because the
    // notes of the earlier export are already deleted by then. `NewOnly` is
    // O_EXCL, so a name that is taken fails to open and the counter goes up.
    const QString base = QStringLiteral("Denkzettel %1 %2").arg(topic, date.toString(Qt::ISODate));
    QFile file;
    for (int attempt = 1;; ++attempt) {
        const QString name = attempt == 1 ? base : base + QStringLiteral(" %1").arg(attempt);
        file.setFileName(inbox + QLatin1Char('/') + name + QStringLiteral(".md"));
        if (file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
            break;
        }
        // Told apart, because the two need different answers: a name that is
        // taken is counted up, everything else — a folder with no write
        // permission, a read-only file system, a full disk — ends the run with
        // the corpus untouched.
        if (!QFileInfo::exists(file.fileName())) {
            result.error = i18n("The file %1 could not be written: %2", file.fileName(), file.errorString());
            return result;
        }
        if (attempt >= NameAttempts) {
            result.error = i18n("The folder %1 already holds %2 collective notes of this topic and day.",
                                inbox,
                                NameAttempts);
            return result;
        }
    }

    // UTF-8 and nothing else: the umlauts have to come out of the file the way
    // they went into the note, and a local 8-bit codec would turn every one of
    // them into a question mark without a word.
    const QByteArray encoded = content.toUtf8();
    if (file.write(encoded) != encoded.size() || !file.flush()) {
        // A half-written file must not survive: the deletion below is skipped,
        // so the notes stay — but the torso would stand in the vault under the
        // name of a complete export, and the next run would count past it.
        const QString message = file.errorString();
        file.remove();
        result.error = i18n("The file %1 could not be written: %2", file.fileName(), message);
        return result;
    }
    file.close();

    // The name off the handle, not the one assembled above: what the user is
    // told is the file that exists.
    result.file = QFileInfo(file).absoluteFilePath();

    // Only now, and only in one transaction (SPEC 8.1). A failure here leaves
    // the notes where they are, beside a collective note that already holds
    // them — a duplicate the user can see and act on, which is the harmless
    // half of the two ways this can end.
    if (!store.removeExportedBundle(proposal.noteIds, proposal.id)) {
        result.error = i18n("The collective note %1 was written, but the exported notes could not be deleted: %2",
                            result.file,
                            store.lastError());
    }
    return result;
}
