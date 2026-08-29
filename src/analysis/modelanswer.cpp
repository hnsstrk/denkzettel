#include "analysis/modelanswer.h"

#include <QJsonDocument>
#include <QList>
#include <QStringView>

namespace
{
constexpr QLatin1String thinkingOpen("<think>");
constexpr QLatin1String thinkingClose("</think>");

/**
 * Where the object beginning at `start` ends, or -1 if nothing closes it.
 *
 * Braces inside a JSON string do not count, and neither does one that an
 * escape put there — a task description reading `{ so gemeint }` would
 * otherwise end the object in the middle.
 */
qsizetype objectEnd(const QString &text, qsizetype start)
{
    int depth = 0;
    bool inString = false;
    bool escaped = false;
    for (qsizetype index = start; index < text.size(); ++index) {
        const QChar character = text.at(index);
        if (inString) {
            if (escaped) {
                escaped = false;
            } else if (character == u'\\') {
                escaped = true;
            } else if (character == u'"') {
                inString = false;
            }
            continue;
        }
        if (character == u'"') {
            inString = true;
        } else if (character == u'{') {
            ++depth;
        } else if (character == u'}') {
            --depth;
            if (depth == 0) {
                return index;
            }
        }
    }
    return -1;
}

/** What is left of the answer once the reasoning is out (see the header). */
QString withoutThinking(const QString &answer)
{
    QList<qsizetype> closes;
    QList<qsizetype> opens;

    for (qsizetype index = 0; index < answer.size();) {
        if (answer.at(index) == u'{') {
            const qsizetype end = objectEnd(answer, index);
            if (end >= 0) {
                index = end + 1;
                continue;
            }
        }
        const QStringView rest = QStringView(answer).sliced(index);
        if (rest.startsWith(thinkingClose)) {
            closes.append(index + thinkingClose.size());
            index += thinkingClose.size();
            continue;
        }
        if (rest.startsWith(thinkingOpen)) {
            opens.append(index);
            index += thinkingOpen.size();
            continue;
        }
        ++index;
    }

    // Everything up to the last close, and from the first open that follows it:
    // an opening marker before that close belongs to the reasoning that was
    // just cut away.
    const qsizetype from = closes.isEmpty() ? 0 : closes.constLast();
    qsizetype to = answer.size();
    for (const qsizetype open : opens) {
        if (open >= from) {
            to = open;
            break;
        }
    }
    return answer.mid(from, to - from);
}
}

QJsonObject modelAnswerObject(const QString &answer, QLatin1String requiredKey)
{
    const QString text = withoutThinking(answer);
    // ponytail: every `{` is tried, so a text full of them costs O(n²). An
    // answer is a few thousand characters; the way up, if a model ever writes
    // an essay, is to try only the last opening brace of each depth-0 stretch.
    for (qsizetype start = text.indexOf(u'{'); start >= 0; start = text.indexOf(u'{', start + 1)) {
        const qsizetype end = objectEnd(text, start);
        // Not a break: an object that never closes can still have a closed one
        // inside it, and that one is tried on the next round.
        if (end < 0) {
            continue;
        }
        const QJsonDocument document = QJsonDocument::fromJson(text.mid(start, end - start + 1).toUtf8());
        if (document.isObject() && document.object().contains(requiredKey)) {
            return document.object();
        }
    }
    return {};
}
