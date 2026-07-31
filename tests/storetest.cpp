#include "store/store.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

/**
 * Integration tests of the store layer against a real SQLite file (SPEC 16).
 */
class StoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void createsSchemaOnFirstOpen();
    void defaultPathLivesInApplicationDataDirectory();
    void storesAndReadsNote();
    void updatesNote();
    void replacesTags();
    void removesNoteWithItsTags();
    void removesAudioFileAfterDeletingNote();
    void reopensExistingDatabaseWithoutMigrating();

private:
    QString databasePath() const;
    static Note sampleNote();

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Store> m_store;
};

void StoreTest::initTestCase()
{
    // defaultDatabasePath() derives from the application name, which the test
    // main sets to the test binary's name.
    QCoreApplication::setApplicationName(QStringLiteral("denkzettel"));
}

void StoreTest::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));
}

void StoreTest::cleanup()
{
    m_store.reset();
    m_dir.reset();
}

QString StoreTest::databasePath() const
{
    return m_dir->filePath(QStringLiteral("denkzettel.db"));
}

Note StoreTest::sampleNote()
{
    Note note;
    note.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T14:05:23.123"), Qt::ISODateWithMs);
    note.type = Note::Type::Text;
    note.content = QStringLiteral("Bücher über Straßenbahnen ansehen");
    note.state = Note::State::New;
    return note;
}

void StoreTest::createsSchemaOnFirstOpen()
{
    QVERIFY(QFile::exists(databasePath()));
    QCOMPARE(m_store->schemaVersion(), 1);
}

void StoreTest::defaultPathLivesInApplicationDataDirectory()
{
    const QString path = Store::defaultDatabasePath();

    QVERIFY2(path.endsWith(QStringLiteral("/denkzettel/denkzettel.db")), qPrintable(path));
    QVERIFY2(path.startsWith(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)), qPrintable(path));
}

void StoreTest::storesAndReadsNote()
{
    Note note = sampleNote();
    note.category = QStringLiteral("ideen");
    note.state = Note::State::Analysed;
    note.needsReembed = true;
    note.analysisAttempts = 2;
    note.analysisLastError = QStringLiteral("Zeitüberschreitung");

    const std::optional<qint64> id = m_store->addNote(note);
    QVERIFY2(id.has_value(), qPrintable(m_store->lastError()));

    const std::optional<Note> stored = m_store->note(*id);
    QVERIFY2(stored.has_value(), qPrintable(m_store->lastError()));

    QCOMPARE(stored->id, *id);
    QCOMPARE(stored->createdAt, note.createdAt);
    QCOMPARE(stored->type, note.type);
    QCOMPARE(stored->content, note.content);
    QVERIFY(stored->audioPath.isEmpty());
    QVERIFY(!stored->audioDurationS.has_value());
    QCOMPARE(stored->category, note.category);
    QCOMPARE(stored->state, note.state);
    QCOMPARE(stored->needsReembed, true);
    QCOMPARE(stored->analysisAttempts, 2);
    QCOMPARE(stored->analysisLastError, note.analysisLastError);

    // An unanalysed note keeps category NULL and reads back as an empty string.
    const std::optional<qint64> plainId = m_store->addNote(sampleNote());
    QVERIFY(plainId.has_value());
    const std::optional<Note> plain = m_store->note(*plainId);
    QVERIFY(plain.has_value());
    QVERIFY(plain->category.isEmpty());
    QCOMPARE(plain->analysisAttempts, 0);

    QVERIFY(!m_store->note(4711).has_value());
}

void StoreTest::updatesNote()
{
    const std::optional<qint64> id = m_store->addNote(sampleNote());
    QVERIFY(id.has_value());

    Note changed = sampleNote();
    changed.id = *id;
    changed.type = Note::Type::Audio;
    changed.content = QStringLiteral("Transkript der Sprachnotiz");
    changed.audioPath = QStringLiteral("2026-07-31T14-05-23.ogg");
    changed.audioDurationS = 42;
    changed.state = Note::State::Transcribed;

    QVERIFY2(m_store->updateNote(changed), qPrintable(m_store->lastError()));

    const std::optional<Note> stored = m_store->note(*id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->type, Note::Type::Audio);
    QCOMPARE(stored->content, changed.content);
    QCOMPARE(stored->audioPath, changed.audioPath);
    QCOMPARE(stored->audioDurationS, changed.audioDurationS);
    QCOMPARE(stored->state, Note::State::Transcribed);

    Note unknown = sampleNote();
    unknown.id = 4711;
    QVERIFY(!m_store->updateNote(unknown));
}

void StoreTest::replacesTags()
{
    const std::optional<qint64> id = m_store->addNote(sampleNote());
    QVERIFY(id.has_value());

    QVERIFY(m_store->tags(*id).isEmpty());

    QVERIFY2(m_store->setTags(*id, {QStringLiteral("backup"), QStringLiteral("bücher")}), qPrintable(m_store->lastError()));
    QCOMPARE(m_store->tags(*id), QStringList({QStringLiteral("backup"), QStringLiteral("bücher")}));

    QVERIFY(m_store->setTags(*id, {QStringLiteral("cli")}));
    QCOMPARE(m_store->tags(*id), QStringList({QStringLiteral("cli")}));

    QVERIFY(m_store->setTags(*id, {}));
    QVERIFY(m_store->tags(*id).isEmpty());

    // Tags need a note to hang on; the foreign key rejects orphans.
    QVERIFY(!m_store->setTags(4711, {QStringLiteral("cli")}));
}

void StoreTest::removesNoteWithItsTags()
{
    const std::optional<qint64> id = m_store->addNote(sampleNote());
    QVERIFY(id.has_value());
    QVERIFY(m_store->setTags(*id, {QStringLiteral("backup")}));

    const std::optional<qint64> otherId = m_store->addNote(sampleNote());
    QVERIFY(otherId.has_value());
    QVERIFY(m_store->setTags(*otherId, {QStringLiteral("cli")}));

    QVERIFY2(m_store->removeNote(*id), qPrintable(m_store->lastError()));

    QVERIFY(!m_store->note(*id).has_value());
    QVERIFY(m_store->tags(*id).isEmpty());

    // The other note is untouched.
    QVERIFY(m_store->note(*otherId).has_value());
    QCOMPARE(m_store->tags(*otherId), QStringList({QStringLiteral("cli")}));

    QVERIFY(!m_store->removeNote(4711));
}

void StoreTest::removesAudioFileAfterDeletingNote()
{
    QVERIFY(QDir().mkpath(m_store->audioDirectory()));
    const QString audioFile = m_store->audioDirectory() + QStringLiteral("/2026-07-31T14-05-23.ogg");
    QFile file(audioFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("ogg");
    file.close();

    Note note = sampleNote();
    note.type = Note::Type::Audio;
    note.audioPath = QStringLiteral("2026-07-31T14-05-23.ogg");
    note.audioDurationS = 12;
    note.state = Note::State::Transcribed;

    const std::optional<qint64> id = m_store->addNote(note);
    QVERIFY(id.has_value());

    QVERIFY2(m_store->removeNote(*id), qPrintable(m_store->lastError()));

    QVERIFY(!m_store->note(*id).has_value());
    QVERIFY2(!QFile::exists(audioFile), qPrintable(audioFile));
}

void StoreTest::reopensExistingDatabaseWithoutMigrating()
{
    const std::optional<qint64> id = m_store->addNote(sampleNote());
    QVERIFY(id.has_value());
    QVERIFY(m_store->setTags(*id, {QStringLiteral("backup")}));

    m_store.reset();
    m_store = std::make_unique<Store>(databasePath());

    // The migration statements deliberately carry no IF NOT EXISTS, so a
    // second migration run on an up-to-date database would fail here.
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    QCOMPARE(m_store->schemaVersion(), 1);
    const std::optional<Note> stored = m_store->note(*id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->content, sampleNote().content);
    QCOMPARE(m_store->tags(*id), QStringList({QStringLiteral("backup")}));
}

QTEST_GUILESS_MAIN(StoreTest)

#include "storetest.moc"
