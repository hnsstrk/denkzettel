#include "proposals/fullexport.h"

#include "store/note.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace
{

/**
 * A text as a double-quoted YAML scalar.
 *
 * Category, tags and file name are user and AI material and carry whatever the
 * language brings — a colon, a hash, a quotation mark, a line break. Unquoted
 * those turn the frontmatter into something else or into nothing, and the
 * export would look complete while the receiving side reads a different note.
 * The double-quoted form is the one YAML flavour that has escapes at all.
 */
QString yamlString(const QString &value)
{
    QString quoted = value;
    // The backslash first, or the escapes written below get escaped a second
    // time.
    quoted.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    quoted.replace(QLatin1Char('"'), QLatin1String("\\\""));
    quoted.replace(QLatin1Char('\n'), QLatin1String("\\n"));
    quoted.replace(QLatin1Char('\r'), QLatin1String("\\r"));
    quoted.replace(QLatin1Char('\t'), QLatin1String("\\t"));
    return QLatin1Char('"') + quoted + QLatin1Char('"');
}

QString yamlList(const QStringList &values)
{
    QStringList quoted;
    quoted.reserve(values.size());
    for (const QString &value : values) {
        quoted.append(yamlString(value));
    }
    return QLatin1Char('[') + quoted.join(QLatin1String(", ")) + QLatin1Char(']');
}

/**
 * The Markdown of one note: the frontmatter SPEC 8.3 asks for, then the text.
 *
 * `created` stands in it although the file name carries the same moment: the
 * name has the colons of the hour replaced (SPEC 4), so it is not the
 * timestamp the database holds. `audio` names the copy in the subfolder, and
 * it is written whenever the database has one — also when the file itself is
 * missing, because what the export hands over is what the corpus says.
 */
QString markdownFor(const Note &note, const QStringList &tags)
{
    // The two words the store writes into `type` (SPEC 5.1).
    const QString type = note.type == Note::Type::Audio ? QStringLiteral("audio") : QStringLiteral("text");

    QString text = QStringLiteral("---\n");
    text += QStringLiteral("type: %1\n").arg(type);
    text += QStringLiteral("category: %1\n").arg(yamlString(note.category));
    text += QStringLiteral("tags: %1\n").arg(yamlList(tags));
    text += QStringLiteral("created: %1\n").arg(yamlString(note.createdAt.toString(Qt::ISODateWithMs)));
    if (!note.audioPath.isEmpty()) {
        text += QStringLiteral("audio: %1\n").arg(yamlString(QStringLiteral("audio/") + note.audioPath));
    }
    text += QStringLiteral("---\n\n");
    text += note.content;
    if (!text.endsWith(QLatin1Char('\n'))) {
        text += QLatin1Char('\n');
    }
    return text;
}

/** Writes `content`, reporting a refused open and a short write alike. */
bool writeFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(content) == content.size() && file.flush();
}

}

FullExportResult exportAllNotes(const Store &store, const QString &parentDirectory, const QDate &date)
{
    FullExportResult result;

    const QString path = parentDirectory + QStringLiteral("/denkzettel-export-")
        + date.toString(QStringLiteral("yyyy-MM-dd"));

    // An existing folder is refused rather than written into or dodged with a
    // suffix. Writing into it would mix two corpora and overwrite the files of
    // notes that have been deleted since, without saying so; a suffix would
    // give up the name SPEC 8.3 fixes. Refusing leaves both the old export and
    // the corpus whole, and the user picks another folder or clears this one.
    if (QFileInfo::exists(path)) {
        result.error = i18n("The folder %1 already exists.", path);
        return result;
    }

    const QString audioPath = path + QStringLiteral("/audio");
    // mkpath makes both at once and answers for the unwritable parent as well
    // — the other error path of a folder the user pointed at.
    if (!QDir().mkpath(audioPath)) {
        result.error = i18n("The folder %1 could not be created.", path);
        return result;
    }
    result.directory = path;

    const QString audioSourceDirectory = store.audioDirectory();

    const QList<Note> notes = store.notes();
    for (const Note &note : notes) {
        // The note's ISO timestamp, colons and all, is the name (SPEC 8.3
        // "ISO name") under the one departure SPEC 4 lays down for the audio
        // file — so a note and its recording carry the same stem.
        const QString stem = noteFileStem(note.createdAt);
        const QString file = path + QLatin1Char('/') + stem + QStringLiteral(".md");

        // Two notes of the same millisecond want the same name, and the second
        // of them does not get into the folder. It keeps the ISO name SPEC 8.3
        // fixes rather than taking a suffix of its own: the name is the note's
        // timestamp, and a `-2` beside it would be a second naming rule for a
        // case SPEC 4 already settles with the milliseconds. So the note is
        // reported as missing, which is what it is.
        if (QFileInfo::exists(file)) {
            result.missing.append(i18n("Two notes carry the timestamp %1; only the first is in the folder.", stem));
            continue;
        }

        // UTF-8 and nothing else: umlauts have to come out of the file the way
        // they went into the note, and a local 8-bit codec would turn every one
        // of them into a question mark without a word (issue #36).
        if (!writeFile(file, markdownFor(note, store.tags(note.id)).toUtf8())) {
            // A short write leaves a part of a note behind, and the count is
            // read off the folder — left standing, that half file would be
            // counted as an exported note. A stick that runs full mid-export is
            // the case this path exists for, not a theoretical one.
            QFile::remove(file);
            result.missing.append(i18n("The note %1 could not be written.", stem));
            continue;
        }

        if (note.audioPath.isEmpty()) {
            continue;
        }
        // QFile::copy refuses a missing source and an existing target alike,
        // and both are worth a line: a note whose recording is gone is exactly
        // what a rescue path must not pass over in silence. The note itself
        // stands in the folder, so this is the incomplete kind, not the missing
        // one.
        if (!QFile::copy(audioSourceDirectory + QLatin1Char('/') + note.audioPath,
                         audioPath + QLatin1Char('/') + note.audioPath)) {
            result.incomplete.append(i18n("The audio file %1 could not be copied.", note.audioPath));
        }
    }

    result.noteCount = static_cast<int>(QDir(path).entryList({QStringLiteral("*.md")}, QDir::Files).size());
    result.audioCount = static_cast<int>(QDir(audioPath).entryList(QDir::Files).size());
    return result;
}
