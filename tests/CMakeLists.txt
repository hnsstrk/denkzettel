# Silence, and it applies to every executable created BELOW this line.
#
# The KDE guard dialog plays a system sound on every show — fourteen per
# librarytest run, in the room the customer sits in
# (Prüfbericht Testklänge, W1). testsilence.cpp points
# libcanberra at its null driver before main() begins; link_libraries() puts
# its object into every target this directory creates from here on, tests and
# benches alike, so nobody has to remember it. A ctest property could not do
# that: the benches are started by hand and no property reaches them.
#
# That is why this stands at the top and everything else below: a fifth bench
# appended at the end of this file inherits the silence, a target added above
# this line would not — so nothing goes above it.
add_library(testsilence OBJECT testsilence.cpp)

target_link_libraries(testsilence PRIVATE Qt6::Core)

link_libraries(testsilence)

add_executable(storetest storetest.cpp)

target_link_libraries(storetest PRIVATE
    denkzettelstore
    Qt6::Test
)

add_test(NAME storetest COMMAND storetest)

# Prototyp zum Spike #62, nur mit -DDENKZETTEL_SPIKE_SPELLFIX=ON. Steht der
# Schalter aus — der Normalfall —, existiert weder das Programm noch der Test,
# und `ctest` läuft wie zuvor.
if(DENKZETTEL_SPIKE_SPELLFIX)
    add_executable(spellfixspike spellfixspike.cpp)

    target_link_libraries(spellfixspike PRIVATE
        denkzettelstore
        Qt6::Test
    )

    add_test(NAME spellfixspike COMMAND spellfixspike)
endif()

add_executable(capturetest capturetest.cpp)

target_link_libraries(capturetest PRIVATE
    denkzettelcapture
    KF6::Svg
    Qt6::Test
)

# Two desktop themes of our own, so that the hull assertions of issue #55 do not
# hang on the distribution the tests were written on: every theme with a border
# other than 4 px on that machine comes from a CachyOS package, and a build host
# that installs only this project's KF6 parts has no desktop theme at all —
# `ksvg` does not depend on `libplasma`. tests/desktopthemes.h explains the
# division of labour; the installed themes are still measured where they exist.
target_compile_definitions(capturetest PRIVATE
    DENKZETTEL_TEST_THEMES="${CMAKE_CURRENT_SOURCE_DIR}/themes")

add_test(NAME capturetest COMMAND capturetest)

# The offscreen platform keeps the run independent of a running compositor. The
# KDE platform theme comes on top since the hull of issue #55: the geometry
# assertions of wireframe 4b measure spacings the font decides, and under a
# substitute font they would be measuring something else (SPEC 16).
set_tests_properties(capturetest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde")

add_executable(librarytest librarytest.cpp)

target_link_libraries(librarytest PRIVATE
    denkzettelui
    Qt6::Test
)

add_test(NAME librarytest COMMAND librarytest)

# The KDE platform theme comes on top, as it does for shelltest: the library
# checks its buttons for their icon names, and without a resolvable theme
# QIcon::fromTheme() hands back a null icon whose name is empty (issue #67).
# It is more than a search path here, though — under this theme the platform
# integration replaces a built QMessageBox with a substitute dialog of its own,
# so the guard dialog the test measures is the one a Plasma user sees
# (issue #66, SPEC 9).
set_tests_properties(librarytest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde")

# The same binary a second time, under the scaling the customer runs — and only
# the one check that needs it (issue #101, UI review of Sprint 9, L9).
#
# QT_SCALE_FACTOR is read once, before the application exists, so no test
# function can set it; a second run is the only way to reach that state.
#
# What this one measures is that a hairline is the same number of DEVICE pixel
# rows wherever it lands in the grid. At scaling 1 that can never go wrong,
# which is why the fault stood for a whole story without a red test — and the
# eye cannot judge it either, which is why it is a test and not a picture.
#
# 1.5 since 11.08.2026 (issue #104). It stood at 1.6 because that was the
# customer's setting when the check was written; he has changed it since
# (`kwinrc`, measured 11.08.2026). A check whose own name says „the customer's
# scaling" and measures another one is a false witness, not a stricter one.
add_test(NAME librarytestskaliert
    COMMAND librarytest keepsEverySeparatorOneThicknessUnderTheCustomersScaling)

set_tests_properties(librarytestskaliert PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde;QT_SCALE_FACTOR=1.5")

# Writes the picture series of the search states for the story handover. No
# add_test(): it produces pictures for a human to look at, and a broken
# screenshot writer must not colour the suite red.
#
# This bench and the ones below are built by an ordinary build all the same.
# They were EXCLUDE_FROM_ALL once, and a bench nobody rebuilds goes stale
# silently: in sprint 5 a 22 minute old runner wrote plausible pictures of an
# old state while the suite stayed green
# (Prüfbericht Symbole, Sprint 5, §9.3). Staying out of the suite and
# staying out of the build are two different things — only the first one is
# wanted here.
add_executable(searchshots searchshots.cpp)

target_link_libraries(searchshots PRIVATE
    denkzettelui
    Qt6::Test
)

# The same for the grouped note list (wireframes 3a/3b). Run it with
# QT_QPA_PLATFORMTHEME=kde, or the substitute font misstates the type sizes.
add_executable(libraryshots libraryshots.cpp)

target_link_libraries(libraryshots PRIVATE
    denkzettelui
    Qt6::Test
)

# The hull of the capture window under two desktop themes and two colour
# schemes (wireframes 4a/4b, issues #55 and #56). Same rules as the benches
# above; it needs the capture window, not the library.
add_executable(captureshots captureshots.cpp)

target_link_libraries(captureshots PRIVATE
    denkzettelcapture
    KF6::Svg
    Qt6::Test
)

target_compile_definitions(captureshots PRIVATE
    DENKZETTEL_TEST_THEMES="${CMAKE_CURRENT_SOURCE_DIR}/themes")

# The reading, editing and guard-dialog states of wireframe 2a, plus the main
# path of S8 (#11). Same rules: run it by hand with QT_QPA_PLATFORMTHEME=kde.
add_executable(editshots editshots.cpp)

target_link_libraries(editshots PRIVATE
    denkzettelui
    Qt6::Test
)

# The two pictures of the README — capture window and library, with invented
# notes. Same rules as the benches above; it needs the capture window as well,
# so it links both libraries.
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
# because the menu entries are checked for their icon names: without it the icon
# search paths hold nothing but the Qt resource, QIcon::fromTheme() returns a
# null icon, and every name would read empty (issue #60).
set_tests_properties(shelltest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QPA_PLATFORMTHEME=kde")

add_executable(firstruntest firstruntest.cpp)

target_link_libraries(firstruntest PRIVATE
    denkzettelshell
    Qt6::Test
)

add_test(NAME firstruntest COMMAND firstruntest)

# The names the session hangs the daemon on, read back in process after the
# registration that writes them (issue #61). No bus needed here — what the
# daemon actually announces on one is the last case of commandlinetest below.
add_executable(identitytest identitytest.cpp)

target_link_libraries(identitytest PRIVATE
    denkzettelshell
    Qt6::Test
)

add_test(NAME identitytest COMMAND identitytest)

# desktopFileName is a property of QGuiApplication, so the test needs a
# QApplication and with it a platform that runs without a compositor. No
# platform theme: nothing here is measured that a font or an icon decides.
set_tests_properties(identitytest PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# What denkzetteld does with its arguments, measured on the built binary (issue
# #61). The version has to travel from project(... VERSION ...) through the
# compiler onto stdout, and no in-process assertion covers that road.
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
# reads back the name it announces, and that is the only automated proof that
# SPEC 2.3 survives the KAboutData registration. A test that silently passes
# where dbus-run-session is missing would be a guard that never sounds. D-Bus is
# a runtime condition of this project anyway (SPEC 2.3), and qt6-base pulls the
# package in.
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session REQUIRED)

# The session of its own is what keeps the started daemon away from the one the
# customer is logged into: on their bus the name is already taken, and the test
# would measure the running service instead of the built one.
add_test(NAME commandlinetest
    COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} -- $<TARGET_FILE:commandlinetest>)

# XDG_DATA_DIRS points at a directory that holds nothing, and it has to be set
# here rather than inside the test: the private bus reads its list of
# activatable services once, when dbus-run-session starts it, and the test
# process is its child. Left at the session's own value, the daemon's first
# notification activates the notification service of the desktop into the
# private session, where it then outlives the bus and holds the output pipe of
# the run open — measured on 05.08.2026 as a test that took 138 ms and 60 s at
# the same time. Nothing measured here depends on a service being activatable.
set_tests_properties(commandlinetest PROPERTIES
    ENVIRONMENT "XDG_DATA_DIRS=${CMAKE_CURRENT_BINARY_DIR}/commandlinetest-ohne-dienste")

# The autostart entry only exists once the project is installed, so its rule has
# no other test that would notice it disappearing (SPEC 2.5).
add_test(NAME installtest
    COMMAND ${CMAKE_COMMAND}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DSTAGING_DIR=${CMAKE_CURRENT_BINARY_DIR}/installtest-root
        -DAPPLICATION_ENTRY=${KDE_INSTALL_FULL_APPDIR}/org.denkzettel.Denkzettel.desktop
        -DAUTOSTART_ENTRY=${KDE_INSTALL_FULL_AUTOSTARTDIR}/org.denkzettel.Denkzettel.desktop
        -P ${CMAKE_CURRENT_SOURCE_DIR}/installtest.cmake
)
