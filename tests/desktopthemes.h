#pragma once

#include <KSvg/FrameSvg>
#include <KSvg/ImageSet>

#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include <memory>
#include <optional>

/**
 * Finding two desktop themes whose hulls differ — for `capturetest` and for
 * `captureshots`.
 *
 * The assertion of #55 AK 1 holds two themes with different borders against
 * each other. **On a machine that is not this one, such a pair may not exist**,
 * and that is measured, not feared: of the eight desktop themes installed here
 * the three that come from the official KDE stack — `default`, `breeze-dark`,
 * `breeze-light` — all carry 4/4/4/4, and every 8 px theme comes from a CachyOS
 * package. A checkout on Fedora, Debian or ordinary Arch finds no pair at all,
 * and a build host that installs only this project's KF6 parts finds **no
 * desktop theme whatsoever**: `ksvg` does not depend on `libplasma`.
 *
 * Hence two sources, and they do different jobs:
 *
 *  * `bundledThemes()` — the two themes under `tests/themes/`. Always there,
 *    so the assertion runs everywhere. What it cannot show is that the code
 *    reads a *real* Plasma theme; it only shows that it reads an SVG of ours.
 *  * `installedThemePair()` — two real themes off this machine, discovered by
 *    measurement. That is the one which proves the real thing, and it is the
 *    one that is not always available.
 *
 * Neither replaces the other. Both run where both can.
 */
namespace themes
{

/** The two themes shipped with the tests: a 4 px border against a 12 px one. */
inline QString bundledNarrow()
{
    return QStringLiteral("denkzettel-test-schmal");
}

inline QString bundledWide()
{
    return QStringLiteral("denkzettel-test-breit");
}

/**
 * The third bundled one: same build as the narrow theme, but with **square**
 * corner pieces (issue #83, AK 9).
 *
 * It moved onto the data path with that story, and it has to be named here or
 * it would be counted as an installed Plasma theme: `installedThemes()` below
 * lists everything on the path and knows the bundled ones by name. Sorted by
 * name it comes first, so `anyInstalledTheme()` would hand a **square** theme
 * to every assertion that wants a real, rounding one — measured, and it turned
 * three of them red the moment the theme was moved.
 */
inline QString bundledSquare()
{
    return QStringLiteral("denkzettel-pruef-eckig");
}

/**
 * Puts the bundled themes on the data path.
 *
 * Called before the first theme is resolved. `qputenv` in a running process is
 * enough — QStandardPaths reads `XDG_DATA_DIRS` afresh, which was measured
 * rather than assumed. The existing entries are kept: the installed themes have
 * to stay reachable, or `installedThemePair()` below would find nothing.
 */
inline void addBundledThemesToDataPath()
{
    const QByteArray bundled = QByteArrayLiteral(DENKZETTEL_TEST_THEMES);
    const QByteArray existing = qgetenv("XDG_DATA_DIRS");

    qputenv("XDG_DATA_DIRS", existing.isEmpty() ? bundled : bundled + ':' + existing);
}

/** The border a theme claims for itself, or 0 if it does not resolve. */
inline qreal borderOf(const QString &theme)
{
    KSvg::ImageSet imageSet(theme, QStringLiteral("plasma/desktoptheme"));

    KSvg::FrameSvg frame;
    frame.setImageSet(&imageSet);
    frame.setImagePath(QStringLiteral("dialogs/background"));
    frame.setEnabledBorders(KSvg::FrameSvg::AllBorders);
    frame.resizeFrame(QSizeF(600, 200));

    return frame.isValid() ? frame.marginSize(KSvg::FrameSvg::LeftMargin) : 0;
}

/** Every desktop theme on the data path, ours excluded. */
inline QStringList installedThemes()
{
    QStringList names;
    const QStringList roots = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("plasma/desktoptheme"),
                                                        QStandardPaths::LocateDirectory);

    for (const QString &root : roots) {
        const QStringList entries = QDir(root).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &name : entries) {
            if (!names.contains(name) && name != bundledNarrow() && name != bundledWide()
                && name != bundledSquare()) {
                names << name;
            }
        }
    }

    return names;
}

/**
 * Two installed themes whose borders differ — narrow first — or nothing.
 *
 * Measured, not named: a fixed pair of names would tie the test to the
 * distribution it was written on, which is exactly the fault this replaces.
 */
inline std::optional<std::pair<QString, QString>> installedThemePair()
{
    QString narrow;
    qreal narrowBorder = 0;

    const QStringList candidates = installedThemes();
    for (const QString &theme : candidates) {
        const qreal border = borderOf(theme);
        if (border <= 0) {
            continue;
        }

        if (narrow.isEmpty()) {
            narrow = theme;
            narrowBorder = border;
            continue;
        }

        if (!qFuzzyCompare(border, narrowBorder)) {
            return border > narrowBorder ? std::make_pair(narrow, theme) : std::make_pair(theme, narrow);
        }
    }

    return std::nullopt;
}

/** One resolvable installed theme, or nothing. */
inline std::optional<QString> anyInstalledTheme()
{
    const QStringList candidates = installedThemes();
    for (const QString &theme : candidates) {
        if (borderOf(theme) > 0) {
            return theme;
        }
    }

    return std::nullopt;
}

}
