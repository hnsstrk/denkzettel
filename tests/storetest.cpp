#include "store/searchquery.h"
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
    void storesAVoiceNoteWithoutATranscript();
    void forgetsTheErrorOfAnEarlierCall();
    void listsNotesNewestFirst();
    void updatesNote();
    void replacesTags();
    void countsTheCategoriesOfTheLibraryColumn();
    void writesTheChosenCategoryIntoTheSearchText();
    void removesNoteWithItsTags();
    void removesAudioFileAfterDeletingNote();
    void removesOrphanedAudioFilesButKeepsReferencedOnes();
    void reopensExistingDatabaseWithoutMigrating();

    void findsNotesByFullText();
    void searchFindsWordsSpelledWithoutUmlauts();
    void searchMatchesAnyPartOfAWord();
    void searchFindsTermsShorterThanThreeCharacters();
    void searchTakesQueryTextLiterally();
    void searchWithoutTermsListsAllNotes();
    void parsesSearchOperators();
    void parsesUnknownOperatorsAsText();
    void searchAppliesOperatorsBesideFreeText();
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
    QCOMPARE(m_store->schemaVersion(), 4);
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

void StoreTest::storesAVoiceNoteWithoutATranscript()
{
    // The state a voice note lives in until its transcription is through, and
    // it is a lasting one: the note is visible and playable without a
    // transcript, and it stays that way if the transcription never succeeds
    // (SPEC 12). Measured 2026-08-28: Qt's SQLite driver binds a default
    // constructed QString as NULL, so this note ran into the NOT NULL
    // constraint of `content` and was never stored at all — the recording
    // would have been made and thrown away with nothing but a line in the
    // journal.
    Note note;
    note.createdAt = QDateTime::currentDateTime();
    note.type = Note::Type::Audio;
    note.audioPath = QStringLiteral("2026-08-28T21-07-03.250.ogg");
    note.audioDurationS = 7;

    const std::optional<qint64> id = m_store->addNote(note);
    QVERIFY2(id.has_value(), qPrintable(m_store->lastError()));

    const std::optional<Note> stored = m_store->note(*id);
    QVERIFY(stored.has_value());
    QVERIFY(stored->content.isEmpty());
    QCOMPARE(stored->type, Note::Type::Audio);
    QCOMPARE(stored->state, Note::State::New);
    QCOMPARE(stored->audioPath, note.audioPath);
}

void StoreTest::forgetsTheErrorOfAnEarlierCall()
{
    // The order is the whole case: a call that fails, then one that succeeds,
    // then the message. Written the other way round it would be green whether
    // the store forgets or not.
    Note missing = sampleNote();
    missing.id = 4711;
    QVERIFY(!m_store->updateNote(missing));
    QVERIFY(!m_store->lastError().isEmpty());

    QVERIFY2(m_store->addNote(sampleNote()).has_value(), qPrintable(m_store->lastError()));

    // What the failed call left in lastError() must not still be standing
    // here. That string becomes the reason a transcription was given up on
    // (SPEC 12, `transcribe_jobs.last_error`), and a reason belonging to a
    // different call is worse than none.
    QVERIFY2(m_store->lastError().isEmpty(), qPrintable(m_store->lastError()));

    // And a reader that only asks after a failure keeps getting the right
    // answer — the contract the callers of this class actually go by.
    QVERIFY(!m_store->updateNote(missing));
    QCOMPARE(m_store->lastError(), QStringLiteral("Notiz 4711 existiert nicht"));
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

void StoreTest::countsTheCategoriesOfTheLibraryColumn()
{
    // The counters of the category column break silently: a number beside a
    // heading looks right whatever it counts, and the wrong one only shows
    // when somebody counts the list by hand (SPEC 9, issue #18).
    //
    // Every expectation below is a number written down **here** and not one
    // read back out of the same query that produced it — nine notes, laid out
    // so that no two of the three answers can be right by accident: three
    // categories of different sizes, one value the fixed list of SPEC 6 does
    // not know, and two notes that separate the two halves of the condition
    // "given up on".
    const auto add = [this](const QString &category, Note::State state, int attempts) {
        Note note = sampleNote();
        note.category = category;
        note.state = state;
        note.analysisAttempts = attempts;
        QVERIFY(m_store->addNote(note).has_value());
    };
    const auto addWithoutText = [this](int attempts) {
        Note note = sampleNote();
        note.content = QStringLiteral("   ");
        note.state = Note::State::New;
        note.analysisAttempts = attempts;
        QVERIFY(m_store->addNote(note).has_value());
    };

    add(QStringLiteral("todos"), Note::State::Analysed, 0);
    add(QStringLiteral("todos"), Note::State::Analysed, 0);
    add(QStringLiteral("todos"), Note::State::Analysed, 0);
    add(QStringLiteral("ideen"), Note::State::Analysed, 0);
    add(QStringLiteral("software"), Note::State::Analysed, 0);
    // A category no classifier writes any more. It counts under "All" and under
    // no entry of the column — the fixed list stays the shape of the column.
    add(QStringLiteral("obsolet"), Note::State::Analysed, 0);
    // Analysed although it once failed: not given up on, the attempts were
    // reset by the success (SPEC 7.2). It separates the state clause from the
    // counter clause.
    add(QStringLiteral("todos"), Note::State::Analysed, Store::analysisAttemptLimit);
    // Waiting, with one attempt left — not given up on either.
    add(QString(), Note::State::New, Store::analysisAttemptLimit - 1);
    // The only one the column's last entry stands for.
    add(QString(), Note::State::New, Store::analysisAttemptLimit);
    // A note without text never reaches the queue (unanalysedNotes()), so it
    // was never given up on either — the third part of the condition, and the
    // one that is easiest to leave out.
    addWithoutText(Store::analysisAttemptLimit);
    // Written by a foreign or older database in a different case. It has to
    // land under the entry `kat:` finds it under, or the column would show it
    // as `0` beside a search that returns it (SPEC 6: ASCII case folded).
    add(QStringLiteral("TODOs"), Note::State::Analysed, 0);

    const CategoryCounts counts = m_store->categoryCounts();

    QCOMPARE(counts.total, 11);
    QCOMPARE(counts.byCategory.value(QStringLiteral("todos")), 5);
    QCOMPARE(counts.byCategory.value(QStringLiteral("ideen")), 1);
    QCOMPARE(counts.byCategory.value(QStringLiteral("software")), 1);
    QCOMPARE(counts.byCategory.value(QStringLiteral("obsolet")), 1);
    // Not an entry of the column, and never counted into one either: a note
    // without a category belongs to none of them.
    QCOMPARE(counts.byCategory.value(QStringLiteral("cli")), 0);
    QCOMPARE(counts.byCategory.size(), 4);
    QCOMPARE(counts.unclassified, 1);
}

void StoreTest::writesTheChosenCategoryIntoTheSearchText()
{
    // What a click in the category column leaves in the search field (SPEC 9,
    // UX decision 2026-08-29). It breaks silently in both directions: a term
    // eaten here disappears from a search the user typed, and a `kat:` left
    // standing beside the new one asks for a note with two categories, which
    // is a list that stays empty for ever.
    const auto written = [](const QString &text, const QString &category) {
        return withSearchCategory(text, category);
    };

    QCOMPARE(written(QString(), QStringLiteral("software")), QStringLiteral("kat:software"));

    // Everything else the field carries survives the click — and the operators
    // are checked one by one, because a mistake in the token test would eat
    // exactly one of them.
    QCOMPARE(written(QStringLiteral("nach:2026-06 tag:backup typ:text Bücher \"zwei Wörter\""),
                     QStringLiteral("cli")),
             QStringLiteral("nach:2026-06 tag:backup typ:text Bücher \"zwei Wörter\" kat:cli"));

    // The old category is replaced and not kept beside the new one, wherever it
    // stood and however it was spelled.
    QCOMPARE(written(QStringLiteral("kat:todos Bücher"), QStringLiteral("ideen")),
             QStringLiteral("Bücher kat:ideen"));
    QCOMPARE(written(QStringLiteral("Bücher KAT:TODOs"), QStringLiteral("ideen")),
             QStringLiteral("Bücher kat:ideen"));

    // "All" is the absence of a category, so it takes the operator out and
    // leaves the rest — the counter-case to the two above, and the one that
    // shows the replacement is not simply an append.
    QCOMPARE(written(QStringLiteral("kat:todos nach:2026-06"), QString()),
             QStringLiteral("nach:2026-06"));
    QCOMPARE(written(QStringLiteral("kat:todos"), QString()), QString());

    // What the parser reads as text stays text: a quoted `kat:` is a phrase and
    // a prefix without a value is a word. Removing either would change a query
    // nobody asked to change — and both are exactly the conditions
    // parseSearchQuery() files a `kat:` under, read the other way round.
    QCOMPARE(written(QStringLiteral("\"kat:todos\""), QStringLiteral("cli")),
             QStringLiteral("\"kat:todos\" kat:cli"));
    QCOMPARE(written(QStringLiteral("kat:"), QStringLiteral("cli")), QStringLiteral("kat: kat:cli"));

    // And the result has to survive the parser it was written for: the click is
    // only worth anything if the store sees one category and the rest untouched.
    const SearchQuery query = parseSearchQuery(written(QStringLiteral("kat:todos tag:backup"),
                                                       QStringLiteral("ideen")));
    QCOMPARE(query.categories, QStringList({QStringLiteral("ideen")}));
    QCOMPARE(query.tags, QStringList({QStringLiteral("backup")}));
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

void StoreTest::removesOrphanedAudioFilesButKeepsReferencedOnes()
{
    // SPEC 2.5, the one permissible piece of self-healing: a recording nothing
    // points at any more — an aborted recording, a deletion cut off between
    // commit and unlink — goes at the service start.
    const QDir audio(m_store->audioDirectory());
    QVERIFY(QDir().mkpath(audio.path()));
    const QString transcribed = QStringLiteral("2026-07-31T14-05-23.123.ogg");
    const QString queued = QStringLiteral("2026-07-31T15-11-02.004.ogg");
    const QString orphan = QStringLiteral("2026-07-31T16-42-19.900.ogg");
    for (const QString &name : {transcribed, queued, orphan}) {
        QFile file(audio.filePath(name));
        QVERIFY2(file.open(QIODevice::WriteOnly), qPrintable(name));
        file.write("ogg");
    }

    Note note = sampleNote();
    note.type = Note::Type::Audio;
    note.audioPath = transcribed;
    note.audioDurationS = 12;
    note.state = Note::State::Transcribed;
    QVERIFY2(m_store->addNote(note).has_value(), qPrintable(m_store->lastError()));

    // The second one is what the question "and while a transcription is still
    // outstanding?" comes down to: the note keeps its audio through the whole
    // queue, so it is referenced like any other. A sweep that went by the
    // state instead of by the column would take the recording out from under
    // the job.
    note.audioPath = queued;
    note.content.clear();
    note.state = Note::State::New;
    const std::optional<qint64> queuedId = m_store->addNote(note);
    QVERIFY2(queuedId.has_value(), qPrintable(m_store->lastError()));
    QVERIFY(m_store->enqueueTranscription(*queuedId));

    // "removed **and logged**" is one criterion, not two: without this line the
    // set stays green when somebody drops the qInfo, because a deleted file
    // looks the same either way.
    QTest::ignoreMessage(QtInfoMsg,
                         qPrintable(QStringLiteral("Removed the orphaned audio file ") + orphan));

    m_store->sweepOrphanedAudio();

    QVERIFY2(!QFile::exists(audio.filePath(orphan)), qPrintable(orphan));
    QVERIFY2(QFile::exists(audio.filePath(transcribed)), qPrintable(transcribed));
    QVERIFY2(QFile::exists(audio.filePath(queued)), qPrintable(queued));
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

    QCOMPARE(m_store->schemaVersion(), 4);
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
    // middle and end (SPEC 6, user decision 01.08.2026).
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
    // it read as an FTS5 operator, the note would still show up. The parser of
    // S7 knows five operators and AND is not one of them.
    QVERIFY(m_store->search(QStringLiteral("Backup AND Notizen")).isEmpty());

    // Punctuation that is FTS5 syntax must not raise a search error either;
    // outside a phrase it separates terms like any other character.
    QCOMPARE(searchContents(QStringLiteral("\"Backup\"")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("Backup (Notizen)")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("Backup -Notizen")), QStringList({note.content}));
    QCOMPARE(searchContents(QStringLiteral("aufräumen^*")), QStringList({note.content}));

    // A query of pure punctuation carries no term and must not be a syntax
    // error either.
    QCOMPARE(m_store->search(QStringLiteral("*()")).size(), 1);
    QVERIFY2(m_store->lastError().isEmpty(), qPrintable(m_store->lastError()));

    // The same characters behind a quotation mark are a phrase, and there they
    // are searched for as they stand: the note does not carry them. An empty
    // list is the answer, an FTS5 syntax error would not be.
    QVERIFY(m_store->search(QStringLiteral("\"*()")).isEmpty());
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

    QCOMPARE(m_store->schemaVersion(), 4);

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

    // And the queue of schema version 3 stands on a database that was written
    // before it existed — a transcription of a note that is already there is
    // the case this migration exists for (SPEC 12).
    QVERIFY2(m_store->enqueueTranscription(1), qPrintable(m_store->lastError()));
    const std::optional<TranscribeJob> job = m_store->takeTranscribeJob();
    QVERIFY(job.has_value());
    QCOMPARE(job->noteId, qint64(1));
    QVERIFY2(m_store->completeTranscription(1, QStringLiteral("Straßenbahn nach Süden")),
             qPrintable(m_store->lastError()));
    QCOMPARE(searchContents(QStringLiteral("Süden")), QStringList({QStringLiteral("Straßenbahn nach Süden")}));

    // And the classification of schema version 4 writes onto a note that was
    // stored before its column existed (SPEC 7.2, issue #14). The counter it
    // reads is the one the version 1 row already carried: note 2 stands at one
    // failed attempt, and the next failure is its second.
    QVERIFY2(m_store->completeAnalysis(1,
                                       QStringLiteral("cli"),
                                       {QStringLiteral("bahn")},
                                       QStringLiteral(R"({"description":"Fahrplan ansehen"})")),
             qPrintable(m_store->lastError()));
    const std::optional<Note> analysed = m_store->note(1);
    QVERIFY(analysed.has_value());
    QCOMPARE(analysed->state, Note::State::Analysed);
    QCOMPARE(analysed->category, QStringLiteral("cli"));
    QCOMPARE(analysed->task, QStringLiteral(R"({"description":"Fahrplan ansehen"})"));
    QCOMPARE(m_store->tags(1), QStringList({QStringLiteral("bahn")}));
    QCOMPARE(m_store->failAnalysis(2, QStringLiteral("Ollama antwortete nicht")), std::optional<int>(2));

    // And it keeps working for notes written after the migration.
    Note added = sampleNote();
    added.content = QStringLiteral("Nach der Migration erfasst");
    QVERIFY(m_store->addNote(added).has_value());
    QCOMPARE(searchContents(QStringLiteral("Migration")), QStringList({added.content}));

    // Opening again must not migrate a second time.
    m_store.reset();
    m_store = std::make_unique<Store>(databasePath());
    QVERIFY2(m_store->open(), qPrintable(m_store->lastError()));
    QCOMPARE(m_store->schemaVersion(), 4);
    QCOMPARE(m_store->notes().size(), 3);
}

void StoreTest::parsesSearchOperators()
{
    // The five operators of SPEC 6, one at a time, and the prefix in whatever
    // case it was typed.
    QCOMPARE(parseSearchQuery(QStringLiteral("tag:backup")).tags, QStringList({QStringLiteral("backup")}));
    QCOMPARE(parseSearchQuery(QStringLiteral("TAG:backup")).tags, QStringList({QStringLiteral("backup")}));
    QCOMPARE(parseSearchQuery(QStringLiteral("kat:todos")).categories, QStringList({QStringLiteral("todos")}));
    QCOMPARE(parseSearchQuery(QStringLiteral("typ:audio")).types, QStringList({QStringLiteral("audio")}));
    QCOMPARE(parseSearchQuery(QStringLiteral("Typ:TEXT")).types, QStringList({QStringLiteral("text")}));

    // German special characters reach the operator as themselves — the tags
    // and categories of this application are written with them.
    QCOMPARE(parseSearchQuery(QStringLiteral("tag:straßenbahn")).tags, QStringList({QStringLiteral("straßenbahn")}));
    QCOMPARE(parseSearchQuery(QStringLiteral("kat:persönlich")).categories,
             QStringList({QStringLiteral("persönlich")}));

    // Month and day come out as one boundary, and both operators answer the
    // **first** day of what was written: `nach:` includes the period it names
    // (SPEC 6, user decision 2026-08-29). The two `…-06-15` cases are the pair
    // that carries it — the same date, and only `vor:` leaves it out.
    QCOMPARE(parseSearchQuery(QStringLiteral("vor:2026-07")).before, QDate(2026, 7, 1));
    QCOMPARE(parseSearchQuery(QStringLiteral("vor:2026-07-15")).before, QDate(2026, 7, 15));
    QCOMPARE(parseSearchQuery(QStringLiteral("nach:2026-06")).after, QDate(2026, 6, 1));
    QCOMPARE(parseSearchQuery(QStringLiteral("nach:2026-06-15")).after, QDate(2026, 6, 15));

    // Everything is ANDed, so two boundaries pointing the same way keep the
    // narrower one. The narrower one stands **first** here on purpose: written
    // the other way round the case would come out the same whether the parser
    // narrows or simply keeps the last value it read.
    const SearchQuery narrowed =
        parseSearchQuery(QStringLiteral("vor:2026-01 vor:2026-06 nach:2026-04 nach:2026-02"));
    QCOMPARE(narrowed.before, QDate(2026, 1, 1));
    QCOMPARE(narrowed.after, QDate(2026, 4, 1));

    // The last day QDate holds is a boundary like any other. Under the earlier
    // excluding reading `nach:` added a day here and landed in the year 10000
    // — a valid QDate whose ISO string is empty, which SQLite bound as NULL so
    // that nothing matched at all.
    QCOMPARE(parseSearchQuery(QStringLiteral("nach:9999-12-31")).after, QDate(9999, 12, 31));

    // Operators and free text in one query, each in its own place.
    const SearchQuery mixed = parseSearchQuery(QStringLiteral("tag:ki tag:backup Bücher \"zwei Wörter\" typ:text"));
    QCOMPARE(mixed.tags, QStringList({QStringLiteral("ki"), QStringLiteral("backup")}));
    QCOMPARE(mixed.types, QStringList({QStringLiteral("text")}));
    QCOMPARE(mixed.terms, QStringList({QStringLiteral("Bücher"), QStringLiteral("zwei Wörter")}));

    // Nothing to search for — the library answers that with the whole list.
    QVERIFY(parseSearchQuery(QString()).isEmpty());
    QVERIFY(parseSearchQuery(QStringLiteral("   ")).isEmpty());
    QVERIFY(!parseSearchQuery(QStringLiteral("typ:text")).isEmpty());
}

void StoreTest::parsesUnknownOperatorsAsText()
{
    // An unknown prefix is full text and no error (SPEC 6) — and a known
    // prefix whose value the parser cannot use is the same: no value at all, a
    // note type that does not exist, a date that does not exist. Every one of
    // them would otherwise answer with an empty list where the user meant to
    // search for the words.
    QCOMPARE(parseSearchQuery(QStringLiteral("foo:bar")).terms,
             QStringList({QStringLiteral("foo"), QStringLiteral("bar")}));
    QVERIFY(parseSearchQuery(QStringLiteral("foo:bar")).tags.isEmpty());
    QCOMPARE(parseSearchQuery(QStringLiteral("tag:")).terms, QStringList({QStringLiteral("tag")}));
    QVERIFY(parseSearchQuery(QStringLiteral("tag:")).tags.isEmpty());
    QCOMPARE(parseSearchQuery(QStringLiteral("typ:bild")).terms,
             QStringList({QStringLiteral("typ"), QStringLiteral("bild")}));
    QVERIFY(parseSearchQuery(QStringLiteral("typ:bild")).types.isEmpty());

    const SearchQuery impossible = parseSearchQuery(QStringLiteral("vor:2026-02-31"));
    QVERIFY(!impossible.before.isValid());
    QCOMPARE(impossible.terms,
             QStringList({QStringLiteral("vor"), QStringLiteral("2026"), QStringLiteral("02"), QStringLiteral("31")}));

    // A colon inside a word is no operator, and one at the front has no prefix
    // in front of it.
    QCOMPARE(parseSearchQuery(QStringLiteral("https://kde.org")).terms,
             QStringList({QStringLiteral("https"), QStringLiteral("kde"), QStringLiteral("org")}));
    QCOMPARE(parseSearchQuery(QStringLiteral(":backup")).terms, QStringList({QStringLiteral("backup")}));

    // Quotation marks make one term out of several words, and where they stand
    // decides what they hold together: in front of the prefix they quote the
    // operator away, behind it they quote its value.
    QCOMPARE(parseSearchQuery(QStringLiteral("\"Backup prüfen\"")).terms,
             QStringList({QStringLiteral("Backup prüfen")}));
    QCOMPARE(parseSearchQuery(QStringLiteral("\"tag:backup\"")).terms, QStringList({QStringLiteral("tag:backup")}));
    QVERIFY(parseSearchQuery(QStringLiteral("\"tag:backup\"")).tags.isEmpty());
    QCOMPARE(parseSearchQuery(QStringLiteral("tag:\"zwei wörter\"")).tags,
             QStringList({QStringLiteral("zwei wörter")}));

    // A quotation mark nobody closed reaches to the end of the input — the
    // state every phrase passes through while it is being typed.
    QCOMPARE(parseSearchQuery(QStringLiteral("Backup \"prüfen und")).terms,
             QStringList({QStringLiteral("Backup"), QStringLiteral("prüfen und")}));

    // A pair of quotes around nothing carries no term. Kept as a phrase it
    // would be a search for spaces and would find nearly every note.
    QVERIFY(parseSearchQuery(QStringLiteral("\"\"")).isEmpty());
    QVERIFY(parseSearchQuery(QStringLiteral("\"   \"")).isEmpty());
}

void StoreTest::searchAppliesOperatorsBesideFreeText()
{
    Note books = sampleNote();
    books.content = QStringLiteral("Backup der Bücher-Datenbank prüfen");
    books.category = QStringLiteral("todos");
    books.createdAt = QDateTime::fromString(QStringLiteral("2026-07-15T10:00:00.000"), Qt::ISODateWithMs);
    const std::optional<qint64> booksId = m_store->addNote(books);
    QVERIFY2(booksId.has_value(), qPrintable(m_store->lastError()));
    QVERIFY(m_store->setTags(*booksId, {QStringLiteral("backup"), QStringLiteral("ki")}));

    Note tram = sampleNote();
    tram.content = QStringLiteral("Straßenbahnen fotografieren");
    tram.type = Note::Type::Audio;
    tram.category = QStringLiteral("ideen");
    tram.createdAt = QDateTime::fromString(QStringLiteral("2026-06-20T10:00:00.000"), Qt::ISODateWithMs);
    const std::optional<qint64> tramId = m_store->addNote(tram);
    QVERIFY2(tramId.has_value(), qPrintable(m_store->lastError()));
    QVERIFY(m_store->setTags(*tramId, {QStringLiteral("backup"), QStringLiteral("straßenbahn")}));

    Note milk = sampleNote();
    milk.content = QStringLiteral("Milch kaufen, Version 2026 planen");
    milk.createdAt = QDateTime::fromString(QStringLiteral("2026-08-05T10:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(milk).has_value());

    Note progress = sampleNote();
    progress.content = QStringLiteral("Fortschritt 100% erreicht");
    progress.createdAt = QDateTime::fromString(QStringLiteral("2026-05-01T10:00:00.000"), Qt::ISODateWithMs);
    QVERIFY(m_store->addNote(progress).has_value());

    // A tag filter, and the result list is the library's order (SPEC 9).
    QCOMPARE(searchContents(QStringLiteral("tag:backup")), QStringList({books.content, tram.content}));
    QCOMPARE(searchContents(QStringLiteral("tag:backup tag:ki")), QStringList({books.content}));

    // A tag of two characters is a tag and not a search term: the three
    // character boundary of the trigram index is a rule of the full text and
    // has nothing to do with this road.
    QCOMPARE(searchContents(QStringLiteral("tag:ki")), QStringList({books.content}));

    // ASCII case does not matter, and ß reaches the comparison as itself.
    QCOMPARE(searchContents(QStringLiteral("tag:BACKUP tag:straßenbahn")), QStringList({tram.content}));

    QCOMPARE(searchContents(QStringLiteral("kat:todos")), QStringList({books.content}));
    QCOMPARE(searchContents(QStringLiteral("typ:audio")), QStringList({tram.content}));
    QCOMPARE(searchContents(QStringLiteral("typ:text")).size(), 3);

    // The month is the whole month: before July leaves out the 15th of July,
    // and from July on the 15th is in.
    QCOMPARE(searchContents(QStringLiteral("vor:2026-07")), QStringList({tram.content, progress.content}));
    QCOMPARE(searchContents(QStringLiteral("nach:2026-07")), QStringList({milk.content, books.content}));

    // The named day itself, and the books note is dated exactly on it. `nach:`
    // takes that day, `vor:` leaves it out (SPEC 6, user decision
    // 2026-08-29) — so every note is on one side or the other and none falls
    // between the two. Both lines come out differently under the earlier
    // excluding reading of `nach:`, where the search began on the 16th and
    // found the milk note alone.
    QCOMPARE(searchContents(QStringLiteral("nach:2026-07-15")), QStringList({milk.content, books.content}));
    QCOMPARE(searchContents(QStringLiteral("vor:2026-07-15")), QStringList({tram.content, progress.content}));

    // Operators and free text together — the second acceptance criterion of
    // the story.
    QCOMPARE(searchContents(QStringLiteral("tag:backup fotografieren")), QStringList({tram.content}));
    QVERIFY(m_store->search(QStringLiteral("tag:ki fotografieren")).isEmpty());
    QCOMPARE(searchContents(QStringLiteral("typ:text vor:2026-06 100")), QStringList({progress.content}));

    // A phrase is one sequence, two words are two conditions: both words stand
    // in the note, in the other order.
    QCOMPARE(searchContents(QStringLiteral("\"Bücher-Datenbank prüfen\"")), QStringList({books.content}));
    QVERIFY(m_store->search(QStringLiteral("\"prüfen Bücher\"")).isEmpty());
    QCOMPARE(searchContents(QStringLiteral("prüfen Bücher")), QStringList({books.content}));

    // A phrase under three characters takes the substring route, and there its
    // wildcards are text: „0%" finds the percentage and not the year 2026,
    // which carries a nought as well.
    QCOMPARE(searchContents(QStringLiteral("\"0%\"")), QStringList({progress.content}));
}

QTEST_GUILESS_MAIN(StoreTest)

#include "storetest.moc"
