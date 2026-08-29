#include "store/searchquery.h"

namespace
{
/** One piece of the input, and whether it opened with a quotation mark. */
struct Token {
    QString text;
    bool phrase = false;
};

/**
 * Cuts the input at whitespace, with quoted stretches held together.
 *
 * The quotation marks themselves do not reach the token: they are the
 * delimiter, so no phrase can carry one and nothing that is built from a
 * phrase further down has to escape one.
 *
 * A quotation mark nobody closed keeps the rest of the input in one token
 * rather than reporting a mistake — the field is searched at every keystroke,
 * and every phrase is unbalanced while it is being typed.
 */
QList<Token> tokenize(const QString &input)
{
    QList<Token> tokens;
    Token current;
    bool started = false;
    bool insideQuotes = false;
    for (const QChar character : input) {
        if (character == QLatin1Char('"')) {
            // Only a quotation mark at the very front makes a phrase, which is
            // what tells `tag:"a b"` — one tag — from `"tag:a"`, one text.
            if (!started) {
                current.phrase = true;
            }
            started = true;
            insideQuotes = !insideQuotes;
            continue;
        }
        if (!insideQuotes && character.isSpace()) {
            if (started) {
                tokens.append(current);
                current = Token();
                started = false;
            }
            continue;
        }
        current.text.append(character);
        started = true;
    }
    if (started) {
        tokens.append(current);
    }
    return tokens;
}

/**
 * Adds what a token contributes to the full text.
 *
 * Everything that is not a letter or a digit separates, so nothing the user
 * types can reach FTS5 as syntax: quotation marks, hyphens, parentheses and
 * words such as AND are plain text, and a stray character is no search error.
 * Letters are letters whatever their alphabet — „Größe" stays one term, and so
 * does „Straßenbahn".
 */
void appendWords(const QString &text, QStringList &terms)
{
    QString current;
    for (const QChar character : text) {
        if (character.isLetterOrNumber()) {
            current.append(character);
        } else if (!current.isEmpty()) {
            terms.append(current);
            current.clear();
        }
    }
    if (!current.isEmpty()) {
        terms.append(current);
    }
}

/**
 * The day a `vor:`/`nach:` value stands for, invalid if it stands for none.
 *
 * Both operators answer the **first** day of what was written, a month being
 * the whole month: `vor:2026-07` and `nach:2026-07` alike resolve to the 1st
 * of July. The difference is what the query does with that day — `vor:` stays
 * below it, `nach:` includes it, so `nach:2026-06-15` finds the 15th itself
 * (SPEC 6, customer decision of 29.08.2026: everyday language over symmetry).
 *
 * The strictness is Qt's: `QDate::fromString()` with an explicit format takes
 * neither a trailing remainder nor a day that the month does not have, so
 * `2026-07-15x` and `2026-02-31` come back invalid (measured with Qt 6.11.2)
 * and their token becomes full text.
 */
QDate boundaryDay(const QString &value)
{
    const QDate day = QDate::fromString(value, QStringLiteral("yyyy-MM-dd"));
    if (day.isValid()) {
        return day;
    }
    return QDate::fromString(value, QStringLiteral("yyyy-MM"));
}

/**
 * Files a `prefix:value` token into the query, or reports that it is none.
 *
 * `false` means the token is full text, and it covers every way that can come
 * about: no colon at all, a colon at the very front, an unknown prefix, an
 * empty value, and a known prefix whose value cannot be used (SPEC 6: nothing
 * the user types is a search error).
 */
bool applyOperator(const QString &token, SearchQuery &query)
{
    // `> 0`, not `>= 0`: a token starting with a colon has no prefix.
    const qsizetype colon = token.indexOf(QLatin1Char(':'));
    if (colon <= 0) {
        return false;
    }
    const QString prefix = token.left(colon).toLower();
    const QString value = token.mid(colon + 1);
    if (value.isEmpty()) {
        return false;
    }

    if (prefix == QLatin1String("tag")) {
        query.tags.append(value);
        return true;
    }
    if (prefix == QLatin1String("kat")) {
        query.categories.append(value);
        return true;
    }
    if (prefix == QLatin1String("typ")) {
        // The two words `notes.type` holds and no others: everything else is a
        // note type that cannot exist, and a filter on it would answer with an
        // empty list where the user meant to search for the word.
        const QString type = value.toLower();
        if (type != QLatin1String("text") && type != QLatin1String("audio")) {
            return false;
        }
        query.types.append(type);
        return true;
    }
    if (prefix == QLatin1String("vor") || prefix == QLatin1String("nach")) {
        const bool after = prefix == QLatin1String("nach");
        const QDate day = boundaryDay(value);
        if (!day.isValid()) {
            return false;
        }
        // Two boundaries of the same direction are ANDed like everything else,
        // so the narrower one wins: `vor:2026-01 vor:2026-06` is January.
        QDate &boundary = after ? query.after : query.before;
        if (boundary.isValid()) {
            boundary = after ? qMax(boundary, day) : qMin(boundary, day);
        } else {
            boundary = day;
        }
        return true;
    }
    return false;
}
}

bool SearchQuery::isEmpty() const
{
    return tags.isEmpty() && categories.isEmpty() && types.isEmpty() && !before.isValid() && !after.isValid()
        && terms.isEmpty();
}

SearchQuery parseSearchQuery(const QString &text)
{
    SearchQuery query;
    const QList<Token> tokens = tokenize(text);
    for (const Token &token : tokens) {
        if (token.phrase) {
            // A phrase of nothing but spaces would match almost every note; it
            // is what an opened and immediately closed pair of quotes leaves.
            if (!token.text.trimmed().isEmpty()) {
                query.terms.append(token.text);
            }
            continue;
        }
        if (applyOperator(token.text, query)) {
            continue;
        }
        appendWords(token.text, query.terms);
    }
    return query;
}
