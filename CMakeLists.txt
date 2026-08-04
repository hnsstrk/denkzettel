cmake_minimum_required(VERSION 3.20)

project(denkzettel VERSION 0.1.0 LANGUAGES CXX)

set(QT_MIN_VERSION 6.7.0)
set(KF_MIN_VERSION 6.0.0)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(FeatureSummary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
# The compile database both lint targets below read (issue #45).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS DBus Widgets Sql)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Config
    DBusAddons
    GlobalAccel
    I18n
    Notifications
    StatusNotifierItem
    WidgetsAddons
    WindowSystem
)

add_subdirectory(src)

install(FILES desktop/org.denkzettel.Denkzettel.desktop DESTINATION ${KDE_INSTALL_APPDIR})

# Autostart with the session (SPEC 2.5), deliberately under the same file name:
# kglobalacceld names the shortcut component after it (SPEC 2.4), and the Plasma
# autostart module switches a system-wide entry off by shadowing exactly that
# name in ~/.config/autostart.
#
# Only prefix /usr resolves this to /etc/xdg/autostart, the single autostart
# directory a stock XDG_CONFIG_DIRS lists. Installed to /usr/local the entry
# lands in /usr/local/etc/xdg/autostart, where no session reads it.
install(FILES desktop/org.denkzettel.Denkzettel.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})

ecm_install_icons(ICONS
    icons/sc-apps-denkzettel.svg
    icons/sc-status-denkzettel-tray.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

if(BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

# Linting on demand, in its own targets (issue #45). Neither analysis hangs in
# CMAKE_CXX_CLANG_TIDY: an ordinary build would pay for it on every file.
#
# The file list is globbed rather than read back from the targets. A new source
# file only reaches the build through a CMakeLists, and editing one re-runs the
# configuration, so the glob cannot fall behind. Generated sources
# (mocs_compilation.cpp, qrc_*.cpp) live in the build directory and stay out of
# the list by construction.
file(GLOB_RECURSE DENKZETTEL_LINT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
if(BUILD_TESTING)
    file(GLOB DENKZETTEL_LINT_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
    # The one file the glob must not hand on: tests/spellfixspike.cpp includes
    # spellfixspike.moc, and moc only writes that file when the spike target is
    # built. With the switch off — the normal case — there is no target, no moc
    # run and no include, so clazy stops on a fatal error and lint-clazy ends on
    # rc=2 whatever the code looks like. The file follows its own target.
    if(NOT DENKZETTEL_SPIKE_SPELLFIX)
        list(REMOVE_ITEM DENKZETTEL_LINT_TEST_SOURCES
             ${CMAKE_CURRENT_SOURCE_DIR}/tests/spellfixspike.cpp)
    endif()
    list(APPEND DENKZETTEL_LINT_SOURCES ${DENKZETTEL_LINT_TEST_SOURCES})
endif()

# Both linters read the same filtered copy of the compile database, see
# cmake/lint-compile-commands.cmake for what is filtered and why.
set(DENKZETTEL_LINT_BUILD_PATH ${CMAKE_BINARY_DIR}/lint)
add_custom_target(lint-database
    COMMAND ${CMAKE_COMMAND}
            -DINPUT=${CMAKE_BINARY_DIR}/compile_commands.json
            -DOUTPUT=${DENKZETTEL_LINT_BUILD_PATH}/compile_commands.json
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/lint-compile-commands.cmake
    COMMENT "Compile-Datenbank für die Linter aufbereiten"
    VERBATIM
)

find_program(RUN_CLANG_TIDY_EXECUTABLE run-clang-tidy)
if(RUN_CLANG_TIDY_EXECUTABLE)
    # The positional arguments are regular expressions on the paths in the
    # compile database; an absolute path matches itself.
    add_custom_target(lint-tidy
        COMMAND ${RUN_CLANG_TIDY_EXECUTABLE} -p ${DENKZETTEL_LINT_BUILD_PATH} -quiet ${DENKZETTEL_LINT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "clang-tidy über src/ und tests/"
        USES_TERMINAL
        VERBATIM
    )
    add_dependencies(lint-tidy lint-database)
else()
    message(STATUS "run-clang-tidy not found, target lint-tidy unavailable")
endif()

find_program(CLAZY_STANDALONE_EXECUTABLE clazy-standalone)
if(CLAZY_STANDALONE_EXECUTABLE)
    # No --export-fixes and no fixit automatism: upstream warns about it itself,
    # and it would edit sources behind the back of whoever started the run.
    add_custom_target(lint-clazy
        COMMAND ${CLAZY_STANDALONE_EXECUTABLE} -p ${DENKZETTEL_LINT_BUILD_PATH}
                --checks=level0,level1 ${DENKZETTEL_LINT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "clazy über src/ und tests/"
        USES_TERMINAL
        VERBATIM
    )
    add_dependencies(lint-clazy lint-database)
else()
    message(STATUS "clazy-standalone not found, target lint-clazy unavailable")
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
