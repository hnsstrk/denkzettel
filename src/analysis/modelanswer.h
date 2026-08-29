#pragma once

#include <QJsonObject>
#include <QLatin1String>
#include <QString>

/**
 * The JSON object a model answered with, out of whatever it wrote around it.
 *
 * Two steps of the analysis run ask a model for JSON — the classification of
 * SPEC 7.2 and the naming of a bundle in SPEC 7.3 — and both meet the same
 * text. So the reading stands here once rather than twice; `requiredKey` is
 * what tells the two answers apart.
 *
 * **A thinking block is the ordinary case, not an edge one.** The qwen3 family
 * reasons before it answers, and the block may carry a JSON draft of its own —
 * so the object is not looked for from the front of the text:
 *
 * - Everything up to and including the **last** `</think>` is dropped. What the
 *   model thought is not what it answered, and a draft in there would otherwise
 *   be read as the answer.
 * - A `<think>` that never closes cuts the text off at itself: an answer that
 *   broke off inside its reasoning has no answer in it, and the JSON standing
 *   in the unfinished block is a draft by definition.
 * - The markers are looked for **outside** the JSON objects of the text. A
 *   `</think>` inside a tag or a note text is text of the answer, and cutting
 *   there threw the answer away whole: measured 2026-08-29 on
 *   `{"category":"ideen","tags":["das </think> steht im text"],…}`, which came
 *   out as "carried no JSON object". That is the damage the error counter does
 *   — the answer is lost and an attempt is spent on it.
 * - In what is left, every `{` is tried in turn as the start of an object,
 *   braces inside strings not counted, and the first one that parses **and
 *   carries `requiredKey`** is the answer. That is what carries a `{` in the
 *   prose around the object, a fenced code block, or a "here is your JSON:"
 *   before it. The key is what tells the answer from anything else in braces:
 *   an example the model quoted, a stray `{` in a sentence.
 *
 * Measured on 2026-08-29 against Ollama 0.32.15: that version answers with the
 * block in a **separate** `thinking` field, so nothing of it reaches this
 * function through OllamaProvider — on `/api/chat` and `/api/generate` alike,
 * with `think` true and false. The robustness is for what SPEC 7.1 puts beside
 * Ollama: openrouter and OpenAI hand the text through as the model wrote it
 * (CLAUDE.md, finding 45).
 *
 * An empty object means the answer carried none; the caller says so in its own
 * words, because what it costs differs per step.
 */
QJsonObject modelAnswerObject(const QString &answer, QLatin1String requiredKey);
