#include "store/store.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

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
    void listsNotesNewestFirst();
    void updatesNote();
    void replacesTags();
    void removesNoteWithItsTags();
    void removesAudioFileAfterDeletingNote();
    void reopensExistingDatabaseWithoutMigrating();

    void findsNotesByFullText();
    void searchFindsWordsSpelledWithoutUmlauts();
    void searchMatchesAnyPartOfAWord();
    void searchFindsTermsShorterThanThreeCharacters();
    void searchTakesQueryTextLiterally();
    void searchWithoutTermsListsAllNotes();
    void keepsSearchIndexInSync();
    void migratesDatabaseFromSchemaVersion1();

private:
    QString databasePath() const;
    static Note sampleNote();
    /** Writes a database in the M1 schema (version 1) with two notes and a tag. */
    static bool writeSchemaVersion1Database(const QString &path, QString *error);
    /** Contents of the notes a search returns, in the order it returned them. */
    QStringList searchContents(const QString &text) const;

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
    QCOMPARE(m_store->schemaVersion(), 2);
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

void StoreTest::listsNotesNewestFirst()
{
    QVERIFY(m_store->notes().isEmpty());

    Note older = sampleNote();
    older.content = QStringLiteral("ältere Notiz");
    older.createdAt = QDateTime::fromString(QStringLiteral("2026-07-29T09:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(older).has_value());

    Note newer = sampleNote();
    newer.content = QStringLiteral("neuere Notiz");
    newer.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T18:30:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(newer).has_value());

    // Same millisecond as `newer`: the id decides, and it decides the same way
    // on every read.
    Note twin = newer;
    twin.content = QStringLiteral("gleichzeitige Notiz");
    const std::optional<qint64> twinId = m_store->addNote(twin);
    QVERIFY(twinId.has_value());

    const QList<Note> notes = m_store->notes();
    QCOMPARE(notes.size(), 3);
    QCOMPARE(notes.at(0).id, *twinId);
    QCOMPARE(notes.at(1).content, newer.content);
    QCOMPARE(notes.at(2).content, older.content);
    QCOMPARE(notes.at(2).createdAt, older.createdAt);
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

    QCOMPARE(m_store->schemaVersion(), 2);
    const std::optional<Note> stored = m_store->note(*id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->content, sampleNote().content);
    QCOMPARE(m_store->tags(*id), QStringList({QStringLiteral("backup")}));
}

QStringList StoreTest::searchContents(const QString &text) const
{
    QStringList contents;
    const QList<Note> found = m_store->search(text);
    contents.reserve(found.size());
    for (const Note &note : found) {
        contents.append(note.content);
    }
    return contents;
}

void StoreTest::findsNotesByFullText()
{
    Note books = sampleNote();
    books.content = QStringLiteral("Bücher über Straßenbahnen ansehen");
    books.createdAt = QDateTime::fromString(QStringLiteral("2026-07-31T18:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(books).has_value());

    Note backup = sampleNote();
    backup.content = QStringLiteral("Backup der Bücher-Datenbank prüfen");
    backup.createdAt = QDateTime::fromString(QStringLiteral("2026-07-30T09:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(backup).has_value());

    Note unrelated = sampleNote();
    unrelated.content = QStringLiteral("Milch kaufen");
    unrelated.createdAt = QDateTime::fromString(QStringLiteral("2026-07-29T09:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(unrelated).has_value());

    // One term hits both notes, and the newest comes first — a result list is
    // ordered like the library list, not by relevance (SPEC 9).
    QCOMPARE(searchContents(QStringLiteral("Bücher")), QStringList({books.content, backup.content}));

    // Two terms are ANDed, not ORed (SPEC 6).
    QCOMPARE(searchContents(QStringLiteral("Bücher Backup")), QStringList({backup.content}));

    QVERIFY(m_store->search(QStringLiteral("Fahrrad")).isEmpty());
}

void StoreTest::searchFindsWordsSpelledWithoutUmlauts()
{
    Note books = sampleNote();
    books.content = QStringLiteral("Bücher über Straßenbahnen ansehen");
    QVERIFY(m_store->addNote(books).has_value());

    // The acceptance criterion of the story, spelled out (SPEC 6).
    QCOMPARE(searchContents(QStringLiteral("bucher")), QStringList({books.content}));
    QCOMPARE(searchContents(QStringLiteral("uber")), QStringList({books.content}));

    // The other direction works as well: typing the umlaut finds the note.
    QCOMPARE(searchContents(QStringLiteral("BÜCHER")), QStringList({books.content}));

    // Documented limit: `remove_diacritics 1` folds diacritics, and ß carries
    // none — it is a letter of its own. „strassenbahnen" therefore does not
    // find „Straßenbahnen" (SPEC 6).
    QVERIFY(m_store->search(QStringLiteral("strassenbahnen")).isEmpty());
    QCOMPARE(searchContents(QStringLiteral("straßenbahnen")), QStringList({books.content}));
}

void StoreTest::searchMatchesAnyPartOfAWord()
{
    Note note = sampleNote();
    note.content = QStringLiteral("Straßenbahnen fotografieren");
    QVERIFY(m_store->addNote(note).has_value());

    Note meeting = sampleNote();
    meeting.content = QStringLiteral("Besprechung vorbereiten");
    meeting.createdAt = QDateTime::fromString(QStringLiteral("2026-07-30T09:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(meeting).has_value());

    // The trigram tokenizer matches a term anywhere inside a word — start,
    // middle and end (SPEC 6, customer decision 01.08.2026).
    QCOMPARE(searchContents(QStringLiteral("foto")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("grafieren")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("bahn")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("sprech")), QStringList({meeting.content}));

    QVERIFY(m_store->search(QStringLiteral("Fahrrad")).isEmpty());
}

void StoreTest::searchFindsTermsShorterThanThreeCharacters()
{
    Note pipeline = sampleNote();
    pipeline.content = QStringLiteral("Besprechung mit Ada zur KI-Pipeline");
    QVERIFY(m_store->addNote(pipeline).has_value());

    Note windows = sampleNote();
    windows.content = QStringLiteral("Fenster putzen");
    windows.createdAt = QDateTime::fromString(QStringLiteral("2026-07-30T09:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(windows).has_value());

    // A trigram index cannot hold anything shorter than three characters, so
    // these terms take the substring route instead. Without it the search
    // would stay silently empty for „KI" or „PO" (SPEC 6).
    QCOMPARE(searchContents(QStringLiteral("KI")), QStringList({pipeline.content}));
    QCOMPARE(searchContents(QStringLiteral("ad")), QStringList({pipeline.content}));

    // Case does not matter on that route either — „ki" has to find „KI".
    QCOMPARE(searchContents(QStringLiteral("ki")), QStringList({pipeline.content}));

    // A short and a long term in one query are ANDed across both routes.
    QCOMPARE(searchContents(QStringLiteral("ad sprech")), QStringList({pipeline.content}));
    QVERIFY(m_store->search(QStringLiteral("ad Fenster")).isEmpty());

    // A term that is nowhere still finds nothing — the fallback widens the
    // search, it does not weaken it.
    QVERIFY(m_store->search(QStringLiteral("qq")).isEmpty());
}

void StoreTest::searchTakesQueryTextLiterally()
{
    Note note = sampleNote();
    note.content = QStringLiteral("Backup und Notizen aufräumen");
    QVERIFY(m_store->addNote(note).has_value());

    // Both terms are in the note, so the search finds it.
    QCOMPARE(searchContents(QStringLiteral("Backup Notizen")), QStringList({note.content}));

    // Adding AND in between finds nothing — and that is the point: AND is
    // searched for as a word, and the note has none starting with „and". Were
    // it read as an FTS5 operator, the note would still show up. Operators
    // arrive with the search parser (S7).
    QVERIFY(m_store->search(QStringLiteral("Backup AND Notizen")).isEmpty());

    // Punctuation that is FTS5 syntax must not raise a search error either;
    // it separates terms like any other character.
    QCOMPARE(searchContents(QStringLiteral("\"Backup\"")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("Backup (Notizen)")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("Backup -Notizen")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("aufräumen^*")), QStringList({note.content}));

    // A query of pure punctuation carries no term and must not be a syntax
    // error either.
    QCOMPARE(m_store->search(QStringLiteral("\"*()")).size(), 1);
    QVERIFY2(m_store->lastError().isEmpty(), qPrintable(m_store->lastError()));
}

void StoreTest::searchWithoutTermsListsAllNotes()
{
    QVERIFY(m_store->addNote(sampleNote()).has_value());
    QVERIFY(m_store->addNote(sampleNote()).has_value());

    // An emptied search field restores the full list (acceptance criterion).
    QCOMPARE(m_store->search(QString()).size(), 2);
    QCOMPARE(m_store->search(QStringLiteral("   ")).size(), 2);
}

void StoreTest::keepsSearchIndexInSync()
{
    Note note = sampleNote();
    note.content = QStringLiteral("Zeltheringe nachkaufen");
    const std::optional<qint64> id = m_store->addNote(note);
    QVERIFY2(id.has_value(), qPrintable(m_store->lastError()));

    // Adding.
    QCOMPARE(searchContents(QStringLiteral("Zeltheringe")), QStringList({note.content}));

    // Editing: the old wording disappears from the index, the new one appears.
    Note edited = note;
    edited.id = *id;
    edited.content = QStringLiteral("Schlafsack lüften");
    QVERIFY2(m_store->updateNote(edited), qPrintable(m_store->lastError()));
    QVERIFY(m_store->search(QStringLiteral("Zeltheringe")).isEmpty());
    QCOMPARE(searchContents(QStringLiteral("Schlafsack")), QStringList({edited.content}));

    // Deleting.
    QVERIFY2(m_store->removeNote(*id), qPrintable(m_store->lastError()));
    QVERIFY(m_store->search(QStringLiteral("Schlafsack")).isEmpty());
    QVERIFY(m_store->search(QString()).isEmpty());
}

bool StoreTest::writeSchemaVersion1Database(const QString &path, QString *error)
{
    // The M1 schema, frozen as it shipped: a migration test is only worth
    // something if the starting point cannot drift along with the code.
    static const QStringList schema = {
        QStringLiteral("CREATE TABLE meta ("
                       "  key TEXT PRIMARY KEY,"
                       "  value TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE notes ("
                       "  id INTEGER PRIMARY KEY,"
                       "  created_at TEXT NOT NULL,"
                       "  type TEXT NOT NULL CHECK (type IN ('text', 'audio')),"
                       "  content TEXT NOT NULL,"
                       "  audio_path TEXT,"
                       "  audio_duration_s INTEGER,"
                       "  category TEXT,"
                       "  state TEXT NOT NULL CHECK (state IN ('neu', 'transkribiert', 'analysiert')),"
                       "  needs_reembed INTEGER NOT NULL DEFAULT 0,"
                       "  analysis_attempts INTEGER NOT NULL DEFAULT 0,"
                       "  analysis_last_error TEXT)"),
        QStringLiteral("CREATE TABLE tags ("
                       "  note_id INTEGER NOT NULL REFERENCES notes(id),"
                       "  tag TEXT NOT NULL,"
                       "  PRIMARY KEY (note_id, tag))"),
        QStringLiteral("INSERT INTO meta (key, value) VALUES ('schema_version', '1')"),
        QStringLiteral("INSERT INTO notes (id, created_at, type, content, state)"
                       " VALUES (1, '2026-07-20T08:15:00.000', 'text', 'Bücher über Straßenbahnen ansehen', 'neu')"),
        QStringLiteral("INSERT INTO notes (id, created_at, type, content, category, state, analysis_attempts)"
                       " VALUES (2, '2026-07-21T19:45:30.500', 'text', 'Backup der Fotos prüfen', 'todos', 'analysiert', 1)"),
        QStringLiteral("INSERT INTO tags (note_id, tag) VALUES (2, 'backup')"),
    };

    const QString connection = QStringLiteral("m1-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    bool ok = true;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        db.setDatabaseName(path);
        if (!db.open()) {
            *error = db.lastError().text();
            ok = false;
        }
        for (const QString &statement : schema) {
            if (!ok) {
                break;
            }
            QSqlQuery query(db);
            if (!query.exec(statement)) {
                *error = query.lastError().text() + QStringLiteral(" — ") + statement;
                ok = false;
            }
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connection);
    return ok;
}

void StoreTest::migratesDatabaseFromSchemaVersion1()
{
    // T3 (issue #9): the first real migration of an existing database.
    m_store.reset();
    QVERIFY(QFile::remove(databasePath()));

    QString error;
    QVERIFY2(writeSchemaVersion1Database(databasePath(), &error), qPrintable(error));

    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));

    QCOMPARE(m_store->schemaVersion(), 2);

    // Every field of the existing rows survives the upgrade.
    const QList<Note> notes = m_store->notes();
    QCOMPARE(notes.size(), 2);
    QCOMPARE(notes.at(0).id, qint64(2));
    QCOMPARE(notes.at(0).content, QStringLiteral("Backup der Fotos prüfen"));
    QCOMPARE(notes.at(0).createdAt,
             QDateTime::fromString(QStringLiteral("2026-07-21T19:45:30.500"), Qt::ISODateWithMs));
    QCOMPARE(notes.at(0).category, QStringLiteral("todos"));
    QCOMPARE(notes.at(0).state, Note::State::Analysed);
    QCOMPARE(notes.at(0).analysisAttempts, 1);
    QCOMPARE(notes.at(1).id, qint64(1));
    QCOMPARE(notes.at(1).content, QStringLiteral("Bücher über Straßenbahnen ansehen"));
    QCOMPARE(notes.at(1).type, Note::Type::Text);
    QCOMPARE(m_store->tags(2), QStringList({QStringLiteral("backup")}));

    // The new index covers the notes that were already there — a migration
    // that only creates the table would leave the old notes unfindable.
    QCOMPARE(searchContents(QStringLiteral("bucher")), QStringList({QStringLiteral("Bücher über Straßenbahnen ansehen")}));
    QCOMPARE(searchContents(QStringLiteral("Fotos")), QStringList({QStringLiteral("Backup der Fotos prüfen")}));

    // And it keeps working for notes written after the migration.
    Note added = sampleNote();
    added.content = QStringLiteral("Nach der Migration erfasst");
    QVERIFY(m_store->addNote(added).has_value());
    QCOMPARE(searchContents(QStringLiteral("Migration")), QStringList({added.content}));

    // Opening again must not migrate a second time.
    m_store.reset();
    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));
    QCOMPARE(m_store->schemaVersion(), 2);
    QCOMPARE(m_store->notes().size(), 3);
}

QTEST_GUILESS_MAIN(StoreTest)

#include "storetest.moc"
