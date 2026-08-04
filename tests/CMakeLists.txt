# Silence, and it applies to every executable created BELOW this line.
#
# The KDE guard dialog plays a system sound on every show — fourteen per
# librarytest run, in the room the customer sits in
# (docs/scrum/reviews/2026-08-04-testklaenge.md, W1). testsilence.cpp points
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
    Qt6::Test
)

add_test(NAME capturetest COMMAND capturetest)

# The offscreen platform keeps the run independent of a running compositor.
set_tests_properties(capturetest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

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

# Writes the picture series of the search states for the story handover. No
# add_test(): it produces pictures for a human to look at, and a broken
# screenshot writer must not colour the suite red.
#
# This bench and the ones below are built by an ordinary build all the same.
# They were EXCLUDE_FROM_ALL once, and a bench nobody rebuilds goes stale
# silently: in sprint 5 a 22 minute old runner wrote plausible pictures of an
# old state while the suite stayed green
# (docs/scrum/reviews/sprint-05-s-symbole/, §9.3). Staying out of the suite and
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
