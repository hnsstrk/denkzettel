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
