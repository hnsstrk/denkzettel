#include "proposals/bundleexport.h"
#include "proposals/fullexport.h"
#include "store/note.h"
#include "store/proposal.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <QVariant>

#include <memory>

/**
 * The full export of SPEC 8.3 (issue #36).
 *
 * Four things about it cannot be looked at: whether the folder holds every
 * note, whether umlauts survive the way through file name and file, whether
 * the corpus really is untouched afterwards, and what happens when a folder or
 * an audio file is not where it should be. The rest — how the folder looks in
 * a file manager — is looked at.
 *
 * **And beside it the bundle export of SPEC 8.1 (issue #32)**, which is the
 * same subject with the stakes reversed: that one only reads, this one deletes
 * what it has written. So what the cases below watch is not the file but the
 * corpus around it — that everything an exported note owned is gone, that
 * everything a failed export owned is still there, and that no name is ever
 * written over. All of it against a **test vault** the run creates itself
 * (SPEC 16); nothing here goes near a real one.
 *
 * The counts are read through a connection of the check's own (see rows()),
 * not through the Store that did the deleting: a query that comes out of the
 * same object as the write agrees with it whether or not anything reached the
 * file.
 */
class ExportTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void exportsEveryNoteWithItsAudio();
    void keepsUmlautsInFileNameAndContent();
    void leavesTheCorpusUntouched();
    void countsTheFilesRatherThanTheNotes();
    void reportsAMissingAudioFile();
    void refusesAnExistingFolder();
    void refusesAFolderItCannotCreate();

    void writesTheCollectiveNoteIntoTheInbox();
    void keepsUmlautsInTheNameAndTheFrontmatter();
    void removesEverythingTheExportedNotesOwned();
    void countsTheNameUpRatherThanOverwriting();
    void keepsTheCorpusWhenTheVaultIsNotThere();
    void keepsTheCorpusWhenTheInboxCannotBeMade();
    void keepsEveryNoteWhenOneOfThemIsAlreadyGone();
    void leavesADeselectedNoteInTheCorpus();
    void refusesATaskSuggestion();

private:
    /** Adds a note and its tags, and returns it with the id the store gave it. */
    Note add(Note note, const QStringList &tags = {});
    /** Writes `content` as the note's audio file and returns the file name. */
    QString writeAudio(const QDateTime &createdAt, const QByteArray &content);
    QString target() const;
    /** Everything the corpus consists of, in one hash: database and audio files. */
    QByteArray corpusFingerprint() const;
    static QString read(const QString &path);

    /** Writes a bundle suggestion over `notes` and returns it with its id. */
    Proposal bundle(const QString &title, const QString &markdown, const QList<Note> &notes);
    /**
     * Rows the query counts, read through a connection of this check's own.
     *
     * The database file rather than the Store that wrote it: a readback out of
     * the same object cannot contradict it (CLAUDE.md, finding 10). The
     * connection is opened and closed per call, so nothing of it outlives the
     * case.
     */
    int rows(const QString &sql) const;
    /** The test vault of SPEC 16 — created by this run, never a real one. */
    QString vault() const;
    QString inboxFile(const QString &name) const;

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<QTemporaryDir> m_targetDir;
    std::unique_ptr<Store> m_store;

    static constexpr auto ExportDate = "2026-08-29";
};

void ExportTest::initTestCase()
{
    // The messages of the export go through i18n(); without the domain every
    // one of them warns before it hands the msgid back.
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("denkzettel"));
}

void ExportTest::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());
    m_targetDir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_targetDir->isValid());

    m_store = std::make_unique<Store>(m_dir->filePath(QStringLiteral("denkzettel.db")));
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));
}

void ExportTest::cleanup()
{
    m_store.reset();
    m_targetDir.reset();
    m_dir.reset();
}

Note ExportTest::add(Note note, const QStringList &tags)
{
    const std::optional<qint64> id = m_store->addNote(note);
    if (!id.has_value()) {
        qWarning("%s", qPrintable(m_store->lastError()));
        return note;
    }
    note.id = *id;
    if (!tags.isEmpty() && !m_store->setTags(*id, tags)) {
        qWarning("%s", qPrintable(m_store->lastError()));
    }
    return note;
}

QString ExportTest::writeAudio(const QDateTime &createdAt, const QByteArray &content)
{
    const QString name = noteFileStem(createdAt) + QStringLiteral(".ogg");
    QDir().mkpath(m_store->audioDirectory());
    QFile file(m_store->audioDirectory() + QLatin1Char('/') + name);
    if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size()) {
        qWarning("writing %s failed", qUtf8Printable(name));
    }
    return name;
}

QString ExportTest::target() const
{
    return m_targetDir->path() + QStringLiteral("/denkzettel-export-") + QLatin1String(ExportDate);
}

QByteArray ExportTest::corpusFingerprint() const
{
    QCryptographicHash hash(QCryptographicHash::Sha256);

    QFile database(m_dir->filePath(QStringLiteral("denkzettel.db")));
    if (database.open(QIODevice::ReadOnly)) {
        hash.addData(database.readAll());
    }

    const QDir audio(m_store->audioDirectory());
    const QStringList names = audio.entryList(QDir::Files, QDir::Name);
    for (const QString &name : names) {
        hash.addData(name.toUtf8());
        QFile file(audio.filePath(name));
        if (file.open(QIODevice::ReadOnly)) {
            hash.addData(file.readAll());
        }
    }
    return hash.result();
}

QString ExportTest::read(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

Proposal ExportTest::bundle(const QString &title, const QString &markdown, const QList<Note> &notes)
{
    Proposal proposal;
    proposal.kind = Proposal::Kind::Bundle;
    proposal.createdAt = QDateTime::fromString(QStringLiteral("2026-08-29T10:00:00.000"), Qt::ISODateWithMs);
    proposal.status = Proposal::Status::Open;
    proposal.payload = QString::fromUtf8(QJsonDocument(QJsonObject{{QLatin1String("title"), title},
                                                                   {QLatin1String("markdown"), markdown}})
                                             .toJson(QJsonDocument::Compact));
    for (const Note &note : notes) {
        proposal.noteIds.append(note.id);
    }
    const std::optional<qint64> id = m_store->addProposal(proposal);
    if (!id.has_value()) {
        qWarning("%s", qPrintable(m_store->lastError()));
        return proposal;
    }
    proposal.id = *id;
    return proposal;
}

int ExportTest::rows(const QString &sql) const
{
    const QString name = QStringLiteral("exporttest-readback");
    int count = -1;
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
        database.setDatabaseName(m_dir->filePath(QStringLiteral("denkzettel.db")));
        if (database.open()) {
            QSqlQuery query(database);
            if (query.exec(sql) && query.next()) {
                count = query.value(0).toInt();
            } else {
                qWarning("%s: %s", qPrintable(sql), qPrintable(query.lastError().text()));
            }
        }
    }
    QSqlDatabase::removeDatabase(name);
    return count;
}

QString ExportTest::vault() const
{
    // A vault this run makes, beside the export target and inside the
    // temporary directory that goes at the end of the case (SPEC 16). Nothing
    // in this file knows a path to a real one.
    return m_targetDir->filePath(QStringLiteral("test-vault"));
}

QString ExportTest::inboxFile(const QString &name) const
{
    return vault() + QStringLiteral("/_INBOX/") + name;
}

void ExportTest::exportsEveryNoteWithItsAudio()
{
    Note text;
    text.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    text.content = QStringLiteral("Erste Notiz");
    add(text);

    Note voice;
    voice.createdAt = QDateTime::fromString(QStringLiteral("2026-08-01T09:07:00.500"), Qt::ISODateWithMs);
    voice.type = Note::Type::Audio;
    voice.content = QStringLiteral("Transkript");
    voice.audioPath = writeAudio(voice.createdAt, QByteArray("OggS-nicht-wirklich"));
    add(voice);

    // A note whose audio_path is SQL NULL rather than an empty string: the
    // store writes NULL for an empty one, and every SQL comparison against
    // NULL is NULL again.
    Note second;
    second.createdAt = QDateTime::fromString(QStringLiteral("2026-08-02T11:00:00.000"), Qt::ISODateWithMs);
    second.content = QStringLiteral("Zweite Notiz");
    add(second);

    const FullExportResult result =
        exportAllNotes(*m_store, m_targetDir->path(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY2(result.ok(), qPrintable(result.error));
    QVERIFY2(result.missing.isEmpty(), qPrintable(result.missing.join(QLatin1Char('\n'))));
    QVERIFY2(result.incomplete.isEmpty(), qPrintable(result.incomplete.join(QLatin1Char('\n'))));
    // The name against a literal, not against the same clock the code read.
    QCOMPARE(result.directory, target());

    // Three notes in, three files out — and the three names, so a run that
    // wrote one file three times over could not pass.
    QCOMPARE(QDir(target()).entryList({QStringLiteral("*.md")}, QDir::Files, QDir::Name),
             QStringList({QStringLiteral("2026-07-31T14-05-23.123.md"),
                          QStringLiteral("2026-08-01T09-07-00.500.md"),
                          QStringLiteral("2026-08-02T11-00-00.000.md")}));
    QCOMPARE(result.noteCount, 3);

    QCOMPARE(QDir(target() + QStringLiteral("/audio")).entryList(QDir::Files, QDir::Name),
             QStringList({QStringLiteral("2026-08-01T09-07-00.500.ogg")}));
    QCOMPARE(result.audioCount, 1);

    // The copy is the original, byte for byte.
    QFile copied(target() + QStringLiteral("/audio/2026-08-01T09-07-00.500.ogg"));
    QVERIFY(copied.open(QIODevice::ReadOnly));
    QCOMPARE(copied.readAll(), QByteArray("OggS-nicht-wirklich"));

    // The voice note names its file, the text note carries no such key at all
    // — an empty one would read as "there is a recording and it is nameless".
    QVERIFY(read(target() + QStringLiteral("/2026-08-01T09-07-00.500.md"))
                .contains(QStringLiteral("\naudio: \"audio/2026-08-01T09-07-00.500.ogg\"\n")));
    QVERIFY(!read(target() + QStringLiteral("/2026-07-31T14-05-23.123.md")).contains(QStringLiteral("audio:")));
}

void ExportTest::keepsUmlautsInFileNameAndContent()
{
    Note note;
    note.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    note.category = QStringLiteral("Ideen & Ärger");
    note.content = QStringLiteral("Bücher über Straßenbahnen — Öl/Übergabe\nZweite Zeile: heikel");
    // A slash and a colon in the values, and a tag that carries both: unquoted
    // the colon ends the YAML key and the rest of the line is lost in silence.
    add(note, {QStringLiteral("Bücher"), QStringLiteral("a/b"), QStringLiteral("mit:Doppelpunkt")});

    const FullExportResult result =
        exportAllNotes(*m_store, m_targetDir->path(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));
    QVERIFY2(result.ok(), qPrintable(result.error));

    // The colons of the hour become hyphens (SPEC 4) and nothing else does —
    // the name is read out of the folder, not built the way the code built it.
    const QStringList names = QDir(target()).entryList({QStringLiteral("*.md")}, QDir::Files);
    QCOMPARE(names, QStringList({QStringLiteral("2026-07-31T14-05-23.123.md")}));

    QCOMPARE(read(target() + QLatin1Char('/') + names.first()),
             QStringLiteral("---\n"
                            "type: text\n"
                            "category: \"Ideen & Ärger\"\n"
                            "tags: [\"Bücher\", \"a/b\", \"mit:Doppelpunkt\"]\n"
                            "created: \"2026-07-31T14:05:23.123\"\n"
                            "---\n"
                            "\n"
                            "Bücher über Straßenbahnen — Öl/Übergabe\n"
                            "Zweite Zeile: heikel\n"));

    // And the same bytes on disk: a comparison of two QStrings would pass even
    // if the file held Latin-1, because reading it back would undo the fault.
    QFile file(target() + QLatin1Char('/') + names.first());
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY(file.readAll().contains(QByteArray("Stra\xC3\x9F""enbahnen")));
}

void ExportTest::leavesTheCorpusUntouched()
{
    Note voice;
    voice.createdAt = QDateTime::fromString(QStringLiteral("2026-08-01T09:07:00.500"), Qt::ISODateWithMs);
    voice.type = Note::Type::Audio;
    voice.content = QStringLiteral("Transkript");
    voice.audioPath = writeAudio(voice.createdAt, QByteArray("OggS-nicht-wirklich"));
    add(voice, {QStringLiteral("Bücher")});

    const QByteArray before = corpusFingerprint();

    const FullExportResult result =
        exportAllNotes(*m_store, m_targetDir->path(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));
    QVERIFY2(result.ok(), qPrintable(result.error));
    QCOMPARE(result.noteCount, 1);
    QCOMPARE(result.audioCount, 1);

    QCOMPARE(corpusFingerprint(), before);

    // Two controls, so the equality above is a finding and not the answer this
    // hash gives to everything (finding 10): a write into the database and a
    // write into the audio directory each have to come out different.
    Note added;
    added.createdAt = QDateTime::fromString(QStringLiteral("2026-08-03T08:00:00.000"), Qt::ISODateWithMs);
    added.content = QStringLiteral("Dazugekommen");
    add(added);
    const QByteArray afterWrite = corpusFingerprint();
    QVERIFY(afterWrite != before);

    writeAudio(added.createdAt, QByteArray("noch eine Datei"));
    QVERIFY(corpusFingerprint() != afterWrite);
}

void ExportTest::countsTheFilesRatherThanTheNotes()
{
    // Two notes of the same millisecond want the same file name. Only one of
    // them can stand in the folder, and the number the user is shown has to be
    // the number of files — a count kept by the loop would say two here and
    // nobody would ever look for the missing note.
    const QDateTime same = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    Note first;
    first.createdAt = same;
    first.content = QStringLiteral("Erste");
    add(first);
    Note second;
    second.createdAt = same;
    second.content = QStringLiteral("Zweite");
    add(second);

    const FullExportResult result =
        exportAllNotes(*m_store, m_targetDir->path(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY2(result.ok(), qPrintable(result.error));
    QCOMPARE(m_store->notes().size(), 2);
    QCOMPARE(QDir(target()).entryList({QStringLiteral("*.md")}, QDir::Files).size(), 1);
    QCOMPARE(result.noteCount, 1);
    // The second note is not in the folder at all, so it is reported as
    // missing and not as incomplete — the two ask for different next steps.
    QCOMPARE(result.missing.size(), 1);
    QVERIFY(result.incomplete.isEmpty());
}

void ExportTest::reportsAMissingAudioFile()
{
    Note voice;
    voice.createdAt = QDateTime::fromString(QStringLiteral("2026-08-01T09:07:00.500"), Qt::ISODateWithMs);
    voice.type = Note::Type::Audio;
    voice.content = QStringLiteral("Transkript ohne Datei");
    // The database points at a recording that is not on disk — the state a
    // deletion broken off half way leaves behind (SPEC 2.5).
    voice.audioPath = noteFileStem(voice.createdAt) + QStringLiteral(".ogg");
    add(voice);

    const FullExportResult result =
        exportAllNotes(*m_store, m_targetDir->path(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    // The run goes through — a rescue path saves what there is — and says what
    // is missing instead of passing over it.
    QVERIFY2(result.ok(), qPrintable(result.error));
    QCOMPARE(result.noteCount, 1);
    QCOMPARE(result.audioCount, 0);
    // The note itself stands in the folder, only its recording is gone — the
    // incomplete kind, not the missing one.
    QCOMPARE(result.incomplete.size(), 1);
    QVERIFY(result.missing.isEmpty());
    QVERIFY2(result.incomplete.first().contains(voice.audioPath), qPrintable(result.incomplete.first()));
    QVERIFY(read(target() + QStringLiteral("/2026-08-01T09-07-00.500.md")).contains(QStringLiteral("Transkript ohne Datei")));
}

void ExportTest::refusesAnExistingFolder()
{
    Note note;
    note.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    note.content = QStringLiteral("Erste Notiz");
    add(note);

    QVERIFY(QDir().mkpath(target()));
    QFile leftover(target() + QStringLiteral("/fremd.md"));
    QVERIFY(leftover.open(QIODevice::WriteOnly));
    leftover.close();

    const FullExportResult result =
        exportAllNotes(*m_store, m_targetDir->path(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY(!result.ok());
    QVERIFY2(result.error.contains(target()), qPrintable(result.error));
    // And it left the folder as it found it — no half-written second export
    // over an earlier one.
    QCOMPARE(QDir(target()).entryList(QDir::Files, QDir::Name), QStringList({QStringLiteral("fremd.md")}));
}

void ExportTest::refusesAFolderItCannotCreate()
{
    // A regular file where the parent folder would have to be: mkpath cannot
    // make a directory below it, which is the same answer a folder without
    // write permission gives — and it needs no permission bits, which a run as
    // root would ignore.
    QFile blocker(m_targetDir->filePath(QStringLiteral("kein-ordner")));
    QVERIFY(blocker.open(QIODevice::WriteOnly));
    blocker.close();

    const FullExportResult result = exportAllNotes(*m_store,
                                                   m_targetDir->filePath(QStringLiteral("kein-ordner")),
                                                   QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY(!result.ok());
    QVERIFY2(!result.error.isEmpty(), "the error path has to say something");
    QVERIFY(result.directory.isEmpty());
}

void ExportTest::writesTheCollectiveNoteIntoTheInbox()
{
    QVERIFY(QDir().mkpath(vault()));

    Note first;
    first.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    first.content = QStringLiteral("Erste Notiz");
    first = add(first, {QStringLiteral("backup"), QStringLiteral("CLI")});

    Note second;
    second.createdAt = QDateTime::fromString(QStringLiteral("2026-08-01T09:07:00.500"), Qt::ISODateWithMs);
    second.content = QStringLiteral("Zweite Notiz");
    second = add(second, {QStringLiteral("backup"), QStringLiteral("Shell Skript")});

    const QString markdown = QStringLiteral("# Backup-Strategie\n\n## 2026-07-31\n\nErste Notiz\n"
                                            "\n## 2026-08-01\n\nZweite Notiz\n");
    const Proposal proposal = bundle(QStringLiteral("Backup-Strategie"), markdown, {first, second});

    const BundleExportResult result =
        exportBundle(*m_store, proposal, vault(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY2(result.ok(), qPrintable(result.error));
    // The name against a literal, not against the same clock or the same title
    // the code read.
    QCOMPARE(result.file, inboxFile(QStringLiteral("Denkzettel Backup-Strategie 2026-08-29.md")));

    // The whole file against the whole expected text: the frontmatter is the
    // one part of this story that a foreign program reads, and a check that
    // only looked for `type: note` would pass over a broken list or a missing
    // separator.
    QCOMPARE(read(result.file),
             QStringLiteral("---\n"
                            "type: note\n"
                            "tags:\n"
                            "  - denkzettel\n"
                            "  - backup\n"
                            "  - cli\n"
                            "  - shell-skript\n"
                            "created: 2026-08-29\n"
                            "---\n"
                            "\n")
                 + markdown);

    // Flat in `_INBOX/`, and nothing beside it: the vault keeps no subfolders
    // there, and nothing of ours writes an INDEX.md.
    QCOMPARE(QDir(vault() + QStringLiteral("/_INBOX")).entryList(QDir::Files, QDir::Name),
             QStringList({QStringLiteral("Denkzettel Backup-Strategie 2026-08-29.md")}));
    QCOMPARE(QDir(vault()).entryList(QDir::Files, QDir::Name), QStringList());
}

void ExportTest::keepsUmlautsInTheNameAndTheFrontmatter()
{
    QVERIFY(QDir().mkpath(vault()));

    Note note;
    note.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    note.content = QStringLiteral("Über Straßenbahnen");
    note = add(note, {QStringLiteral("Grüße & Küsse")});

    const QString markdown =
        QStringLiteral("# Bücher über Straßenbahnen\n\n## 2026-07-31\n\nÜber Straßenbahnen\n");
    const Proposal proposal = bundle(QStringLiteral("Bücher über Straßenbahnen"), markdown, {note});

    const BundleExportResult result =
        exportBundle(*m_store, proposal, vault(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY2(result.ok(), qPrintable(result.error));

    // The name off the directory rather than off the result: what carries the
    // umlauts has to be the entry in the file system, not a string this run
    // built and handed back to itself.
    QCOMPARE(QDir(vault() + QStringLiteral("/_INBOX")).entryList(QDir::Files, QDir::Name),
             QStringList({QStringLiteral("Denkzettel Bücher über Straßenbahnen 2026-08-29.md")}));

    // ä, ö, ü and ß come out of the file as they went into the note — read
    // back as UTF-8, which is what a local 8-bit codec would have turned into
    // question marks without a word.
    const QString written = read(result.file);
    QVERIFY2(written.contains(QStringLiteral("  - grüße-küsse\n")), qPrintable(written));
    QVERIFY2(written.contains(QStringLiteral("# Bücher über Straßenbahnen")), qPrintable(written));
    QVERIFY2(written.contains(QStringLiteral("Über Straßenbahnen")), qPrintable(written));
    QVERIFY2(!written.contains(QLatin1Char('?')), "an umlaut came through as a question mark");
}

void ExportTest::removesEverythingTheExportedNotesOwned()
{
    QVERIFY(QDir().mkpath(vault()));

    Note text;
    text.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    text.content = QStringLiteral("Zettelkasten und Backup");
    text = add(text, {QStringLiteral("backup")});

    Note voice;
    voice.createdAt = QDateTime::fromString(QStringLiteral("2026-08-01T09:07:00.500"), Qt::ISODateWithMs);
    voice.type = Note::Type::Audio;
    voice.content = QStringLiteral("Transkript über Backups");
    voice.audioPath = writeAudio(voice.createdAt, QByteArray("OggS-nicht-wirklich"));
    voice = add(voice, {QStringLiteral("backup")});

    QVERIFY(m_store->setEmbedding(text.id, QStringLiteral("modell"), {1.0F, 0.0F}));
    QVERIFY(m_store->enqueueTranscription(voice.id));
    const Proposal proposal = bundle(QStringLiteral("Backup"),
                                     QStringLiteral("# Backup\n\n## 2026-07-31\n\nZettelkasten und Backup\n"),
                                     {text, voice});

    // Asserted **before** anything could take them away, and while the store
    // that owns them is alive: a check whose preconditions are only read at the
    // end cannot tell "deleted" from "never there" (CLAUDE.md, finding 29).
    const QString audio = m_store->audioDirectory() + QLatin1Char('/') + voice.audioPath;
    QVERIFY2(QFile::exists(audio), qPrintable(audio));
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes")), 2);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM tags")), 2);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM embeddings")), 1);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM transcribe_jobs")), 1);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposals")), 1);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposal_notes")), 2);
    QVERIFY(rows(QStringLiteral("SELECT COUNT(*) FROM notes_fts WHERE notes_fts MATCH 'backup'")) > 0);

    const BundleExportResult result =
        exportBundle(*m_store, proposal, vault(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY2(result.ok(), qPrintable(result.error));
    QVERIFY(QFile::exists(result.file));

    // Every table SPEC 8.1 names, counted on the database file through a
    // connection of this check's own.
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes")), 0);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM tags")), 0);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM embeddings")), 0);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM transcribe_jobs")), 0);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposals")), 0);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposal_notes")), 0);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes_fts WHERE notes_fts MATCH 'backup'")), 0);

    // The recording off the file system, not off a return value — and the path
    // is the one the database held, so this is the check that the export
    // deletes the file that was really written (CLAUDE.md, finding 26).
    QVERIFY2(!QFile::exists(audio), qPrintable(audio));
    QCOMPARE(QDir(m_store->audioDirectory()).entryList(QDir::Files, QDir::Name), QStringList());
}

void ExportTest::countsTheNameUpRatherThanOverwriting()
{
    QVERIFY(QDir().mkpath(vault()));
    const QDate date = QDate::fromString(QLatin1String(ExportDate), Qt::ISODate);

    Note first;
    first.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    first.content = QStringLiteral("Erste Notiz");
    first = add(first);
    const QString firstMarkdown = QStringLiteral("# Thema\n\n## 2026-07-31\n\nErste Notiz\n");

    const BundleExportResult one =
        exportBundle(*m_store, bundle(QStringLiteral("Thema"), firstMarkdown, {first}), vault(), date);
    QVERIFY2(one.ok(), qPrintable(one.error));
    QCOMPARE(one.file, inboxFile(QStringLiteral("Denkzettel Thema 2026-08-29.md")));

    Note second;
    second.createdAt = QDateTime::fromString(QStringLiteral("2026-08-02T11:00:00.000"), Qt::ISODateWithMs);
    second.content = QStringLiteral("Zweite Notiz");
    second = add(second);

    const BundleExportResult two = exportBundle(
        *m_store,
        bundle(QStringLiteral("Thema"), QStringLiteral("# Thema\n\n## 2026-08-02\n\nZweite Notiz\n"), {second}),
        vault(),
        date);
    QVERIFY2(two.ok(), qPrintable(two.error));
    QCOMPARE(two.file, inboxFile(QStringLiteral("Denkzettel Thema 2026-08-29 2.md")));

    // Both stand there, and the first one still holds its own note. An
    // overwrite would be the one real data loss of this story: the notes of the
    // first export are deleted by now, so its file is all that is left of them.
    QCOMPARE(QDir(vault() + QStringLiteral("/_INBOX")).entryList(QDir::Files, QDir::Name),
             QStringList({QStringLiteral("Denkzettel Thema 2026-08-29 2.md"),
                          QStringLiteral("Denkzettel Thema 2026-08-29.md")}));
    QVERIFY2(read(one.file).endsWith(firstMarkdown), qPrintable(read(one.file)));
    QVERIFY2(read(two.file).contains(QStringLiteral("Zweite Notiz")), qPrintable(read(two.file)));
}

void ExportTest::keepsTheCorpusWhenTheVaultIsNotThere()
{
    Note note;
    note.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    note.content = QStringLiteral("Erste Notiz");
    note.type = Note::Type::Audio;
    note.audioPath = writeAudio(note.createdAt, QByteArray("OggS-nicht-wirklich"));
    note = add(note, {QStringLiteral("backup")});
    const Proposal proposal =
        bundle(QStringLiteral("Thema"), QStringLiteral("# Thema\n\n## 2026-07-31\n\nErste Notiz\n"), {note});

    const QByteArray before = corpusFingerprint();
    const BundleExportResult result = exportBundle(*m_store,
                                                   proposal,
                                                   vault() + QStringLiteral("/gibt-es-nicht"),
                                                   QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY(!result.ok());
    QVERIFY2(!result.error.isEmpty(), "the error path has to say something");
    QVERIFY(result.file.isEmpty());
    // Not one byte of the corpus moved: an export that cannot write must not
    // have begun to delete.
    QCOMPARE(corpusFingerprint(), before);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes")), 1);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposals")), 1);
}

void ExportTest::keepsTheCorpusWhenTheInboxCannotBeMade()
{
    QVERIFY(QDir().mkpath(vault()));
    // A regular file where `_INBOX/` would have to be. mkpath cannot make a
    // directory out of it, and unlike a folder without write permission this
    // holds for a run as root too — which the CI is (CLAUDE.md, finding 46).
    QFile blocker(vault() + QStringLiteral("/_INBOX"));
    QVERIFY(blocker.open(QIODevice::WriteOnly));
    blocker.close();

    Note note;
    note.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    note.content = QStringLiteral("Erste Notiz");
    note = add(note, {QStringLiteral("backup")});
    const Proposal proposal =
        bundle(QStringLiteral("Thema"), QStringLiteral("# Thema\n\n## 2026-07-31\n\nErste Notiz\n"), {note});

    const QByteArray before = corpusFingerprint();
    const BundleExportResult result =
        exportBundle(*m_store, proposal, vault(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY(!result.ok());
    QVERIFY(result.file.isEmpty());
    QCOMPARE(corpusFingerprint(), before);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes")), 1);
}

void ExportTest::keepsEveryNoteWhenOneOfThemIsAlreadyGone()
{
    QVERIFY(QDir().mkpath(vault()));

    QList<Note> notes;
    for (int index = 0; index < 3; ++index) {
        Note note;
        note.createdAt =
            QDateTime::fromString(QStringLiteral("2026-07-1%1T14:05:23.123").arg(index), Qt::ISODateWithMs);
        note.content = QStringLiteral("Notiz %1").arg(index);
        notes.append(add(note, {QStringLiteral("backup")}));
    }
    const Proposal proposal =
        bundle(QStringLiteral("Thema"), QStringLiteral("# Thema\n\n## 2026-07-10\n\nNotiz 0\n"), notes);

    // The third note goes before the export runs — the library deleted it while
    // the suggestion stood. The suggestion handed in still names it, and that
    // is the state a deletion has to survive without taking half a bundle with
    // it.
    QVERIFY(m_store->removeNote(notes.at(2).id));
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes")), 2);

    const BundleExportResult result =
        exportBundle(*m_store, proposal, vault(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    // The file is written — the write comes first, and that is what makes this
    // failure the harmless one.
    QVERIFY(!result.ok());
    QVERIFY2(QFile::exists(inboxFile(QStringLiteral("Denkzettel Thema 2026-08-29.md"))),
             "the collective note is written before anything is deleted");

    // And **neither** of the two surviving notes was deleted, nor the
    // suggestion. All or nothing: a loop of single deletions would have taken
    // the first two and stopped at the third.
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes")), 2);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM tags")), 2);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposals")), 1);
}

void ExportTest::leavesADeselectedNoteInTheCorpus()
{
    QVERIFY(QDir().mkpath(vault()));

    QList<Note> notes;
    for (int index = 0; index < 3; ++index) {
        Note note;
        note.createdAt =
            QDateTime::fromString(QStringLiteral("2026-07-1%1T14:05:23.123").arg(index), Qt::ISODateWithMs);
        note.content = QStringLiteral("Notiz %1").arg(index);
        notes.append(add(note, {QStringLiteral("backup")}));
    }

    Proposal proposal =
        bundle(QStringLiteral("Thema"), QStringLiteral("# Thema\n\n## 2026-07-10\n\nNotiz 0\n"), notes);
    // What the review of SPEC 9 hands in once the third note is deselected: the
    // same suggestion over two notes. The third was not written into the
    // collective note, so it must not be deleted with it.
    const qint64 deselected = notes.at(2).id;
    proposal.noteIds.removeAll(deselected);

    const BundleExportResult result =
        exportBundle(*m_store, proposal, vault(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    QVERIFY2(result.ok(), qPrintable(result.error));
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes")), 1);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM notes WHERE id = %1").arg(deselected)), 1);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM tags")), 1);
    // The suggestion goes either way — accepting and discarding end the same
    // (SPEC 8.1), and its reference to the kept note goes with it.
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposals")), 0);
    QCOMPARE(rows(QStringLiteral("SELECT COUNT(*) FROM proposal_notes")), 0);
}

void ExportTest::refusesATaskSuggestion()
{
    QVERIFY(QDir().mkpath(vault()));

    Note note;
    note.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    note.content = QStringLiteral("Rechnung bezahlen");
    note.task = QStringLiteral("{\"description\": \"Rechnung bezahlen\"}");
    note = add(note);

    Proposal proposal;
    proposal.kind = Proposal::Kind::Task;
    proposal.createdAt = QDateTime::fromString(QStringLiteral("2026-08-29T10:00:00.000"), Qt::ISODateWithMs);
    proposal.payload = note.task;
    proposal.noteIds = {note.id};
    const std::optional<qint64> id = m_store->addProposal(proposal);
    QVERIFY2(id.has_value(), qPrintable(m_store->lastError()));
    proposal.id = *id;

    const QByteArray before = corpusFingerprint();
    const BundleExportResult result =
        exportBundle(*m_store, proposal, vault(), QDate::fromString(QLatin1String(ExportDate), Qt::ISODate));

    // Taskwarrior is the other road (SPEC 8.2). Put through here it would write
    // a file with no collective note in it and delete the note behind it.
    QVERIFY(!result.ok());
    QVERIFY(result.file.isEmpty());
    QVERIFY2(!QFileInfo::exists(vault() + QStringLiteral("/_INBOX")),
             "nothing may be created for a task suggestion");
    QCOMPARE(corpusFingerprint(), before);
}

QTEST_GUILESS_MAIN(ExportTest)

#include "exporttest.moc"
