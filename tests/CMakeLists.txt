# Silence, and it applies to every executable created BELOW this line — so
# NOTHING goes above it. The KDE guard dialog plays a system sound on every
# show, fourteen per librarytest run, in the room the user sits in.
# testsilence.cpp points libcanberra at its null driver before main() begins;
# link_libraries() puts its object into every target this directory creates from
# here on, tests and benches alike. A ctest property could not do that: the
# benches are started by hand and no property reaches them.
add_library(testsilence OBJECT testsilence.cpp)

target_link_libraries(testsilence PRIVATE Qt6::Core)

link_libraries(testsilence)

# LANGUAGE names the source language, for which no catalogue exists, so every
# i18n() call hands back the msgid the code carries. Without it an installed
# German catalogue reaches the run through XDG_DATA_DIRS and the checks compare
# German wording against the English they were written for; the domain the tests
# set is what makes a catalogue findable in the first place (issue #53).
# It rides in the ENVIRONMENT of the three sets that render or compare strings.

add_executable(storetest storetest.cpp)

target_link_libraries(storetest PRIVATE
    denkzettelstore
    Qt6::Test
)

add_test(NAME storetest COMMAND storetest)

add_executable(capturetest capturetest.cpp)

target_link_libraries(capturetest PRIVATE
    denkzettelcapture
    Qt6::Test
)

add_test(NAME capturetest COMMAND capturetest)

# The offscreen platform keeps the run independent of a running compositor, the
# KDE platform theme on top of it: the geometry assertions measure spacings the
# font decides, and under a substitute font they would measure something else
# (SPEC 16).
set_tests_properties(capturetest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde;LANGUAGE=en_US")

add_executable(librarytest librarytest.cpp)

target_link_libraries(librarytest PRIVATE
    denkzettelui
    Qt6::Test
)

add_test(NAME librarytest COMMAND librarytest)

# The KDE platform theme is load-bearing twice over: the library checks its
# buttons for icon names, and without a resolvable theme QIcon::fromTheme()
# hands back a null icon whose name is empty. And under this theme the platform
# integration replaces a built QMessageBox with a substitute dialog of its own,
# so the guard dialog the test measures is the one a Plasma user sees (SPEC 9).
set_tests_properties(librarytest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde;LANGUAGE=en_US")

# The two pictures of the README — capture window and library, with invented
# notes. No add_test(): it produces pictures for a human to look at, and a
# broken screenshot writer must not colour the suite red. It is built by an
# ordinary build all the same — it was EXCLUDE_FROM_ALL once, and a runner
# nobody rebuilds writes plausible pictures of an old state while the suite
# stays green. Run it with QT_QPA_PLATFORMTHEME=kde, or the substitute font
# misstates the type sizes.
add_executable(readmeshots readmeshots.cpp)

target_link_libraries(readmeshots PRIVATE
    denkzettelcapture
    denkzettelui
    Qt6::Test
)

add_executable(shelltest shelltest.cpp)

target_link_libraries(shelltest PRIVATE
    denkzettelshell
    Qt6::Test
)

add_test(NAME shelltest COMMAND shelltest)

# The tray item carries a menu, so the test needs a QApplication and with it a
# platform that runs without a compositor. The KDE platform theme comes on top
# because the menu entries are checked for their icon names — without it every
# name would read empty.
set_tests_properties(shelltest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde;LANGUAGE=en_US")

add_executable(firstruntest firstruntest.cpp)

target_link_libraries(firstruntest PRIVATE
    denkzettelshell
    Qt6::Test
)

add_test(NAME firstruntest COMMAND firstruntest)

# The names the session hangs the daemon on, read back in process after the
# registration that writes them. No bus needed here — what the daemon actually
# announces on one is the last case of commandlinetest below.
add_executable(identitytest identitytest.cpp)

target_link_libraries(identitytest PRIVATE
    denkzettelshell
    Qt6::Test
)

add_test(NAME identitytest COMMAND identitytest)

# desktopFileName is a property of QGuiApplication, so the test needs a
# QApplication and with it a platform that runs without a compositor. No
# platform theme: nothing here depends on a font or an icon.
set_tests_properties(identitytest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# What denkzetteld does with its arguments, measured on the built binary. The
# version has to travel from project(... VERSION ...) through the compiler onto
# stdout, and no in-process assertion covers that road.
add_executable(commandlinetest commandlinetest.cpp)

target_link_libraries(commandlinetest PRIVATE
    Qt6::DBus
    Qt6::Test
)

# The daemon is started as a child process, not linked, so the dependency has to
# be said out loud — otherwise the test runs against yesterday's binary or none.
add_dependencies(commandlinetest denkzetteld)

target_compile_definitions(commandlinetest PRIVATE
    DENKZETTEL_DAEMON_BINARY="$<TARGET_FILE:denkzetteld>"
    DENKZETTEL_VERSION="${PROJECT_VERSION}")

# REQUIRED, not a quiet skip: the last case of this test starts the daemon and
# reads back the name it announces, the only automated proof that SPEC 2.3
# survives the KAboutData registration. A test that silently passes where
# dbus-run-session is missing would be a guard that never sounds.
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session REQUIRED)

# The session of its own is what keeps the started daemon away from the one the
# user is logged into: on their bus the name is already taken, and the test
# would measure the running service instead of the built one.
add_test(NAME commandlinetest
    COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} -- $<TARGET_FILE:commandlinetest>)

# XDG_DATA_DIRS points at a directory that holds nothing, and it has to be set
# here rather than inside the test: the private bus reads its list of
# activatable services once, when dbus-run-session starts it, and the test
# process is its child. Left at the session's own value, the daemon's first
# notification activates the desktop's notification service into the private
# session, where it outlives the bus and holds the output pipe of the run open.
# Nothing measured here depends on a service being activatable.
set_tests_properties(commandlinetest PROPERTIES
    ENVIRONMENT "XDG_DATA_DIRS=${CMAKE_CURRENT_BINARY_DIR}/commandlinetest-ohne-dienste")

# REQUIRED, and the same reasoning as for dbus-run-session above: without
# appstreamcli the metainfo check below would fall away silently, and a metainfo
# file nobody validates is exactly the state issue #73 describes. The name is
# the cache entry KDECMakeSettings already fills; where it found the program
# this call is a no-op.
find_program(APPSTREAMCLI appstreamcli REQUIRED)

# The autostart entry only exists once the project is installed, so its rule has
# no other test that would notice it disappearing (SPEC 2.5). The metainfo file
# is checked from here for a second reason: ECM's own appstreamtest validates at
# the *installed* location out of the install manifest, so it only ever comes
# alive after an installation into the prefix with root — in the automated run
# it keeps skipping itself and is green either way (#73). This one installs into
# a staging directory via DESTDIR and validates there, without root.
add_test(NAME installtest
    COMMAND ${CMAKE_COMMAND}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DSTAGING_DIR=${CMAKE_CURRENT_BINARY_DIR}/installtest-root
        -DAPPLICATION_ENTRY=${KDE_INSTALL_FULL_APPDIR}/org.denkzettel.Denkzettel.desktop
        -DAUTOSTART_ENTRY=${KDE_INSTALL_FULL_AUTOSTARTDIR}/org.denkzettel.Denkzettel.desktop
        -DMETAINFO_ENTRY=${KDE_INSTALL_FULL_METAINFODIR}/org.denkzettel.Denkzettel.metainfo.xml
        -DAPPSTREAMCLI=${APPSTREAMCLI}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/installtest.cmake
)

# That a change of the system font still reaches both windows while they are
# running (issues #68 and #110). A set of its own because it rewrites
# `kdeglobals` under the running process: its main() puts a HOME of its own in
# place before QApplication, which no test function could do afterwards — the
# platform theme has opened the file by then and KSharedConfig hands that same
# instance out to everything. Nothing here touches the user's configuration.
add_executable(systemfontstest systemfontstest.cpp)

target_link_libraries(systemfontstest PRIVATE
    denkzettelcapture
    denkzettelplatform
    denkzettelui
    Qt6::Test
)

add_test(NAME systemfontstest COMMAND systemfontstest)

# Offscreen so the run needs no compositor, and the KDE platform theme on top of
# it: without it a substitute font stands in and the point sizes this set
# compares are no longer the ones kdeglobals names.
set_tests_properties(systemfontstest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde;LANGUAGE=en_US")

# What a broad search costs at a large corpus (issue #78), and where the time of
# a list build goes (issue #110). No add_test(): it measures, it does not
# assert, and its numbers belong in a report, not in a suite that goes red on a
# busy machine.
add_executable(searchbench searchbench.cpp)

target_link_libraries(searchbench PRIVATE
    denkzettelplatform
    denkzettelstore
    denkzettelui
    Qt6::Sql
    Qt6::Test
)
