cmake_minimum_required(VERSION 3.20)

project(denkzettel VERSION 0.7.0 LANGUAGES CXX)

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
# The compile database both lint targets below read.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS DBus Multimedia Network Widgets Sql)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Config
    # KConfigDialog and KConfigDialogManager — the settings dialog of SPEC 13
    # (issue #16). Its own component: Config above brings ConfigCore and
    # ConfigGui, not this one.
    ConfigWidgets
    CoreAddons
    DBusAddons
    GlobalAccel
    # KColorUtils::mix() and contrastRatio() — the ground of a search mark is
    # deepened with them, and only where the type on it keeps 4.5 : 1
    # (issue #116).
    GuiAddons
    I18n
    Notifications
    StatusNotifierItem
    Svg
    WidgetsAddons
    WindowSystem
    XmlGui
)

# KColorScheme::frameContrast() needs KF 6.20 and is sought on its own rather
# than added to the list above: KF_MIN_VERSION feeds both find_package calls, so
# a 6.20 written there would raise the lower bound for ECM and all ten
# components at once.
find_package(KF6ColorScheme 6.20 REQUIRED)

add_subdirectory(src)

# The message catalogues. ki18n_install() comes with KF6::I18n itself
# (/usr/lib/cmake/KF6I18n/KF6I18nMacros.cmake, pulled in by KF6I18nConfig.cmake
# above): it compiles every po/<lang>/denkzettel.po into a .mo with msgfmt and
# installs it as ${KDE_INSTALL_LOCALEDIR}/<lang>/LC_MESSAGES/denkzettel.mo —
# the path KLocalizedString::setApplicationDomain("denkzettel") looks in.
#
# The directory layout is the macro's, not a preference: it reads the language
# from the *directory* and the domain from the file name. A po/de.po would
# install a catalogue named "de" outside any language directory, and no session
# would ever find it.
ki18n_install(po)

install(FILES desktop/io.github.hnsstrk.denkzettel.desktop DESTINATION ${KDE_INSTALL_APPDIR})

# The other half of DBusActivatable in that file (SPEC 2.4, issue #125). Once
# the key is set, KIO's launcher no longer runs the Exec line but calls
# ActivateAction on the bus name — and with nothing running, that call reaches
# the bus and not the program. This file is what lets the bus start it; without
# it the first shortcut press after a session in which the daemon never came up
# fails with ServiceUnknown, and nothing on screen says why.
#
# Configured rather than shipped verbatim: the Exec line of a D-Bus service file
# has to be absolute, and the one place that knows where the binary lands is the
# install prefix.
configure_file(desktop/io.github.hnsstrk.denkzettel.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/io.github.hnsstrk.denkzettel.service @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/io.github.hnsstrk.denkzettel.service
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})

# Autostart with the session (SPEC 2.5), deliberately under the SAME file name
# as above: kglobalacceld names the shortcut component after it (SPEC 2.4), and
# the Plasma autostart module switches a system-wide entry off by shadowing
# exactly that name in ~/.config/autostart. Only prefix /usr resolves this to
# /etc/xdg/autostart; installed to /usr/local no session reads the entry.
install(FILES desktop/io.github.hnsstrk.denkzettel.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})

# The AppStream description, without which a software centre shows the program
# as a bare name without a text and without a picture. ECM's own appstreamtest
# reads the install manifest of a real installation into the prefix; the check
# that runs everywhere is installtest below (issue #73).
install(FILES desktop/io.github.hnsstrk.denkzettel.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

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

# Linting on demand, in its own targets. Neither analysis hangs in
# CMAKE_CXX_CLANG_TIDY: an ordinary build would pay for it on every file. The
# file list is globbed rather than read back from the targets — a new source
# file only reaches the build through a CMakeLists, and editing one re-runs the
# configuration, so the glob cannot fall behind.
file(GLOB_RECURSE DENKZETTEL_LINT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
if(BUILD_TESTING)
    file(GLOB DENKZETTEL_LINT_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
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
    COMMENT "Preparing the compile database for the linters"
    VERBATIM
)

find_program(RUN_CLANG_TIDY_EXECUTABLE run-clang-tidy)
if(RUN_CLANG_TIDY_EXECUTABLE)
    # The positional arguments are regular expressions on the paths in the
    # compile database; an absolute path matches itself.
    #
    # -j4 and not the default: without it run-clang-tidy takes os.cpu_count()
    # processes, which is right for one run on an idle machine and wrong for
    # every other case. Measured on 30.08.2026 with several worktrees checking
    # at once — 144 clang-tidy processes on 24 threads, load average 179, and
    # 8 GB of the machine in swap. clang-tidy holds a whole translation unit in
    # memory, so the ceiling that bites is memory and not cores.
    add_custom_target(lint-tidy
        COMMAND ${RUN_CLANG_TIDY_EXECUTABLE} -p ${DENKZETTEL_LINT_BUILD_PATH} -quiet -j4 ${DENKZETTEL_LINT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "clang-tidy over src/ and tests/"
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
        COMMENT "clazy over src/ and tests/"
        USES_TERMINAL
        VERBATIM
    )
    add_dependencies(lint-clazy lint-database)
else()
    message(STATUS "clazy-standalone not found, target lint-clazy unavailable")
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
