#include "platform/systemfonts.h"

#include <KConfigGroup>
#include <KDirWatch>
#include <KSharedConfig>

#include <QApplication>
#include <QFontDatabase>
#include <QObject>
#include <QStandardPaths>

#include <optional>

namespace
{
/** Whether `followSystemFonts()` has its watch on the file — see `held()`. */
bool watching = false;

struct Held {
    std::optional<QFont> general;
    std::optional<QFont> smallest;
};

/**
 * The two fonts as the last read of `kdeglobals` found them.
 *
 * Only filled while `watching` — that watch is what empties them again, and
 * nothing else in this process can notice the file changing. Without it every
 * call reads the file, exactly as it did before issue #110.
 *
 * Why they are held at all: the note list's delegate asks for a font for every
 * row it measures, and `QListView` measures every row it lays out. Reading and
 * parsing `kdeglobals` once per row cost 3.649,3 ms on a list of 20,000 notes,
 * against 6,2 ms with the fonts held (measured 28.08.2026, Release, median of
 * three).
 *
 * Inside a function rather than beside one: a QFont at namespace scope would be
 * built before `main()`, and clazy says so (`non-pod-global-static`). Read and
 * written from the GUI thread only, like every caller of this unit.
 */
Held &held()
{
    static Held fonts;
    return fonts;
}

QFont fontFromKdeglobals(const char *key, QFontDatabase::SystemFont fallback)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    // Read anew: the file changes under a running process, and a config kept
    // from the last read would hand back the value that made this whole story
    // (issue #68).
    config->reparseConfiguration();

    return KConfigGroup(config, QStringLiteral("General"))
        .readEntry(key, QFontDatabase::systemFont(fallback));
}

QFont keptFont(std::optional<QFont> &slot, const char *key, QFontDatabase::SystemFont fallback)
{
    if (!watching) {
        return fontFromKdeglobals(key, fallback);
    }

    if (!slot) {
        slot = fontFromKdeglobals(key, fallback);
    }

    return *slot;
}
}

QFont platform::generalFont()
{
    return keptFont(held().general, "font", QFontDatabase::GeneralFont);
}

QFont platform::smallestReadableFont()
{
    return keptFont(held().smallest, "smallestReadableFont", QFontDatabase::SmallestReadableFont);
}

void platform::followSystemFonts(QObject *owner)
{
    const QString kdeglobals = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QStringLiteral("/kdeglobals");
    KDirWatch::self()->addFile(kdeglobals);

    auto apply = [kdeglobals](const QString &path) {
        if (path != kdeglobals) {
            return;
        }

        // Before anything reads a font again, this call included: what the two
        // functions above are holding is precisely what has just changed
        // (issue #110).
        held().general.reset();
        held().smallest.reset();

        const QFont wanted = platform::generalFont();
        if (wanted == QApplication::font()) {
            return;
        }

        // This also delivers a QEvent::ApplicationFontChange to every widget,
        // which is where those that carry a font of their own ask again.
        QApplication::setFont(wanted);
    };

    // Both spelled out rather than looped over the two signals: clazy cannot
    // see through a loop variable that a pointer-to-member is a signal and
    // reports the pair as a non-signal connect. KConfig replaces the file
    // rather than rewriting it, which is why `created` counts too.
    QObject::connect(KDirWatch::self(), &KDirWatch::dirty, owner, apply);
    QObject::connect(KDirWatch::self(), &KDirWatch::created, owner, apply);

    // And only from here on may a font be held: the two connections above are
    // the whole of what throws it away again.
    watching = true;
}
