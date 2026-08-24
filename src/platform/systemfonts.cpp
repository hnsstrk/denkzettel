#include "platform/systemfonts.h"

#include <KConfigGroup>
#include <KDirWatch>
#include <KSharedConfig>

#include <QApplication>
#include <QFontDatabase>
#include <QObject>
#include <QStandardPaths>

namespace
{
KConfigGroup generalGroup()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    // Read anew every time: the file changes under a running process, and a
    // config kept from the last read would hand back the value that made this
    // whole story (issue #68).
    config->reparseConfiguration();

    return KConfigGroup(config, QStringLiteral("General"));
}

QFont fontFromKdeglobals(const char *key, QFontDatabase::SystemFont fallback)
{
    return generalGroup().readEntry(key, QFontDatabase::systemFont(fallback));
}
}

QFont platform::generalFont()
{
    return fontFromKdeglobals("font", QFontDatabase::GeneralFont);
}

QFont platform::smallestReadableFont()
{
    return fontFromKdeglobals("smallestReadableFont", QFontDatabase::SmallestReadableFont);
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
}
