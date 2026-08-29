#include "proposals/fullexport.h"
#include "store/note.h"
#include "store/store.h"

#include <KLocalizedString>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

/**
 * The full export of SPEC 8.3 (issue #36).
 *
 * Four things about it cannot be looked at: whether the folder holds every
 * note, whether umlauts survive the way through file name and file, whether
 * the corpus really is untouched afterwards, and what happens when a folder or
 * an audio file is not where it should be. The rest — how the folder looks in
 * a file manager — is looked at.
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

private:
    /** Adds a note and its tags, and returns it with the id the store gave it. */
    Note add(Note note, const QStringList &tags = {});
    /** Writes `content` as the note's audio file and returns the file name. */
    QString writeAudio(const QDateTime &createdAt, const QByteArray &content);
    QString target() const;
    /** Everything the corpus consists of, in one hash: database and audio files. */
    QByteArray corpusFingerprint() const;
    static QString read(const QString &path);

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

QTEST_GUILESS_MAIN(ExportTest)

#include "exporttest.moc"
