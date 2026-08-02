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

# The offscreen platform keeps the run independent of a running compositor.
set_tests_properties(capturetest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(librarytest librarytest.cpp)

target_link_libraries(librarytest PRIVATE
    denkzettelui
    Qt6::Test
)

add_test(NAME librarytest COMMAND librarytest)

set_tests_properties(librarytest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Writes the picture series of the search states for the story handover. No
# add_test(): it produces pictures for a human to look at, and a broken
# screenshot writer must not colour the suite red.
add_executable(searchshots EXCLUDE_FROM_ALL searchshots.cpp)

target_link_libraries(searchshots PRIVATE
    denkzettelui
    Qt6::Test
)

# The same for the grouped note list (wireframes 3a/3b). Run it with
# QT_QPA_PLATFORMTHEME=kde, or the substitute font misstates the type sizes.
add_executable(libraryshots EXCLUDE_FROM_ALL libraryshots.cpp)

target_link_libraries(libraryshots PRIVATE
    denkzettelui
    Qt6::Test
)

# The reading, editing and guard-dialog states of wireframe 2a, plus the main
# path of S8 (#11). Same rules: run it by hand with QT_QPA_PLATFORMTHEME=kde.
add_executable(editshots EXCLUDE_FROM_ALL editshots.cpp)

target_link_libraries(editshots PRIVATE
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
# platform that runs without a compositor.
set_tests_properties(shelltest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

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
