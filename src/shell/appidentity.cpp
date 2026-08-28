#include "shell/appidentity.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QCoreApplication>

void registerApplicationIdentity()
{
    // DENKZETTEL_VERSION is handed over by src/CMakeLists.txt from the
    // project() call of the root CMakeLists — the number lives there and
    // nowhere else (SPEC 15).
    KAboutData about(QStringLiteral("denkzettel"),
                     i18n("Denkzettel"),
                     QStringLiteral(DENKZETTEL_VERSION));

    // setApplicationData() below writes organizationDomain and desktopFileName
    // along with name and version, and the defaults KAboutData brings along are
    // KDE's own: kde.org and org.kde.denkzettel. Two names of the specification
    // hang off exactly those. KDBusService builds its bus name from the
    // reversed domain plus the application name, so the default would register
    // org.kde.denkzettel instead of io.github.hnsstrk.denkzettel (SPEC 2.3);
    // and kglobalacceld names the shortcut component after the desktop file,
    // which Wayland also uses as the application id (SPEC 2.4, the file itself
    // is installed from desktop/). Neither breakage reaches a return value and
    // neither shows up in a test that does not start a bus, so both values are
    // set here rather than left to the default (issue #61).
    //
    // The domain is the application id read backwards: hnsstrk.github.io
    // reversed gives io.github.hnsstrk, and the application name "denkzettel"
    // completes the bus name to the id itself, so the program carries one
    // identity instead of two (issue #112).
    about.setOrganizationDomain(QByteArrayLiteral("hnsstrk.github.io"));
    about.setDesktopFileName(QStringLiteral("io.github.hnsstrk.denkzettel"));

    // What the about dialog of issue #87 has to show beside the version. The
    // sentence is the one the tray tooltip already carries, so it needs no
    // second entry in the catalogue; the licence is the one of LICENSE in the
    // project root, and KAboutLicense turns the identifier into the text the
    // dialog links to.
    about.setShortDescription(i18n("Capture thoughts quickly"));
    about.setLicense(KAboutLicense::MIT);
    // Word for word the line of LICENSE in the project root — the dialog is
    // where the MIT licence names its holder, and the repository is public
    // anyway. Authors stay empty: without them the dialog leaves out a tab, and
    // there is nobody to list beside the holder.
    about.setCopyrightStatement(i18n("Copyright (c) 2026 hnsstrk"));

    KAboutData::setApplicationData(about);
}

void processCommandLineArguments(const QCoreApplication &app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(
        i18n("Denkzettel service: it receives notes and keeps the global shortcuts ready."));
    parser.addHelpOption();
    parser.addVersionOption();

    // Deliberately not KAboutData::setupCommandLine(): it declares
    // --desktopfile <name> beside the options above, and that switch overwrites
    // at runtime the very value SPEC 2.4 hangs the shortcut component on.
    // Undeclared, it is refused like any other unknown switch (issue #61).
    //
    // process() writes the line and ends the process on --version and --help,
    // and ends it with a non-zero code on a switch nobody declared. Before this
    // change the daemon accepted every argument in silence.
    parser.process(app);
}
