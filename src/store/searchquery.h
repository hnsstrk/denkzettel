#pragma once

#include <QDate>
#include <QString>
#include <QStringList>

/**
 * What the user typed in the search field, taken apart (SPEC 6).
 *
 * All components are ANDed; there is no OR and there are no brackets. Every
 * list holds what the query asked for more than once — two `tag:` operators
 * mean a note carrying both tags.
 */
struct SearchQuery {
    /** Values of `tag:` — a note has to carry every one of them. */
    QStringList tags;

    /** Values of `kat:`, compared against `notes.category`. */
    QStringList categories;

    /** Values of `typ:`, already as the text `notes.type` holds: text, audio. */
    QStringList types;

    /**
     * `vor:` — the note has to be older than the start of this day.
     *
     * The parser has already resolved month and day into one boundary:
     * `vor:2026-07` and `vor:2026-07-01` both mean the 1st of July, so the
     * user of this struct never asks how precisely the date was written.
     */
    QDate before;

    /**
     * `nach:` — the note has to be at least as new as the start of this day.
     *
     * Resolved the same way, and to the day **after** what was typed: `nach:`
     * means after the period has passed, so `nach:2026-06-15` is the 16th of
     * June and `nach:2026-06` is the 1st of July.
     */
    QDate after;

    /**
     * Full text, ANDed: single words and phrases that were in quotation marks.
     *
     * A phrase stands here as one entry with its spaces and punctuation, a
     * word as one without either — that is the whole difference between the
     * two, and it is what makes `"Backup prüfen"` find the two words in that
     * order and `Backup prüfen` find them anywhere in the note.
     */
    QStringList terms;

    /** Nothing to search for — the query carried neither text nor operator. */
    bool isEmpty() const;
};

/**
 * Takes the search field apart (SPEC 6). A pure function of its argument.
 *
 * Known are `tag:`, `kat:`, `typ:`, `vor:` and `nach:`, the prefix in any
 * spelling of upper and lower case. Everything else is full text, and that
 * covers more than an unknown `foo:`: a prefix without a value (`tag:`), a
 * type nobody stores (`typ:bild`) and a date that does not exist
 * (`vor:2026-02-31`) all land in the text as well. **No input is a syntax
 * error** — the search field answers with notes or with nothing, never with a
 * complaint.
 *
 * Quotation marks make a phrase, and they do it wherever they stand:
 * `tag:"zwei woerter"` is one tag, `"tag:backup"` is the text `tag:backup`. A
 * quotation mark nobody closed reaches to the end of the input.
 */
SearchQuery parseSearchQuery(const QString &text);
