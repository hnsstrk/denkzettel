add_library(denkzettelstore STATIC
    store/store.cpp
)

target_include_directories(denkzettelstore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelstore PUBLIC
    Qt6::Core
    Qt6::Sql
)

# Spike #62: compile spellfix1 along. OFF by default — the spike ends with a
# report, not with a feature. With the switch off nothing below is configured,
# and the build is the same as without this block.
option(DENKZETTEL_SPIKE_SPELLFIX "Build the spellfix1 prototype of spike #62 (issue #62)" OFF)

if(DENKZETTEL_SPIKE_SPELLFIX)
    # Das Projekt ist als reines C++-Projekt deklariert; spellfix.c ist C. Die
    # Sprache wird erst hier angeschaltet, damit der Normalbau ohne den Schalter
    # keinen C-Compiler suchen muss.
    enable_language(C)
    find_package(SQLite3 REQUIRED)

    target_sources(denkzettelstore PRIVATE ${CMAKE_SOURCE_DIR}/third_party/spellfix/spellfix.c)

    set_source_files_properties(${CMAKE_SOURCE_DIR}/third_party/spellfix/spellfix.c PROPERTIES
        # Without SQLITE_CORE the file compiles as a loadable extension: every
        # SQLite call would then go through the `sqlite3_api` pointer, which only
        # sqlite3_load_extension() fills. With SQLITE_CORE those macros fall away
        # and the file calls the same libsqlite3 directly that the Qt driver is
        # linked against (see /usr/include/sqlite3ext.h, line 727 ff.).
        COMPILE_DEFINITIONS SQLITE_CORE=1
        # Third-party code, taken over unchanged: warnings in it are not ones we
        # would be allowed to fix without breaking the checksum in HERKUNFT.md.
        COMPILE_OPTIONS -w
    )

    target_link_libraries(denkzettelstore PUBLIC SQLite3::SQLite3)
endif()

# The transcription queue (SPEC 12). No library of whisper.cpp's is linked
# here and none is looked for at build time: both programs are subprocesses,
# their paths are settings, and that is what keeps the automated run — which
# has neither a graphics card nor a model — building and testing this code.
add_library(denkzetteltranscribe STATIC
    transcribe/transcriber.cpp
)

target_include_directories(denkzetteltranscribe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzetteltranscribe PUBLIC
    denkzettelstore
    Qt6::Core
    # The three settings of SPEC 12 out of denkzettelrc: the two program paths
    # and the model.
    KF6::ConfigCore
)

add_library(denkzettelplatform STATIC
    platform/systemfonts.cpp
)

target_include_directories(denkzettelplatform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelplatform PUBLIC
    Qt6::Widgets
    KF6::ConfigCore
    KF6::CoreAddons
)

add_library(denkzettelcapture STATIC
    capture/audiorecorder.cpp
    capture/capturewindow.cpp
    capture/textareaheight.cpp
    capture/textcontrast.cpp
)

target_include_directories(denkzettelcapture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelcapture PUBLIC
    denkzettelplatform
    denkzettelstore
    Qt6::Widgets
    # The recording pipeline of the voice note (SPEC 4): QAudioSource reads the
    # device, QMediaRecorder writes Opus into OGG. Nothing here talks to
    # PipeWire itself.
    #
    # Which backend does the encoding is NOT decided here and not by the Qt
    # package either: qt6-multimedia depends on the virtual qt6-multimedia-backend
    # with two providers, ffmpeg and gstreamer. Opus in OGG is what the ffmpeg
    # one writes; the READMEs and the CI name it explicitly for that reason.
    Qt6::Multimedia
    # Whether this session blurs behind windows at all, asked of KWin itself
    # (issue #83, AK 7). No new dependency of the project: Qt6::DBus is already
    # required for the daemon's entry points, and libPlasma — the other road to
    # the same answers — would pull QtQuick in for four numbers.
    Qt6::DBus
    # The hull of the capture window (issue #55, SPEC 3 and 15): KSvg renders
    # `dialogs/background` of the desktop theme, KWindowShadow hands its shadow
    # tiles to the compositor, and KWindowEffects registers the blur and the
    # contrast effect that Plasma registers beside it (issue #83). The theme
    # name is not KSvg's to find — it stands in `plasmarc` (KConfigCore), and
    # KDirWatch (KCoreAddons) is what notices it changing under a standing
    # window.
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::Svg
    KF6::WindowSystem
)

add_library(denkzettelui STATIC
    ui/elidedlines.cpp
    ui/librarywindow.cpp
    ui/notelistdelegate.cpp
    ui/notelistmodel.cpp
    ui/pendingdeletion.cpp
    ui/timestampformat.cpp
)

target_include_directories(denkzettelui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelui PUBLIC
    denkzettelplatform
    denkzettelstore
    Qt6::Widgets
    # KColorScheme::frameContrast() — the ratio the separator lines of the list
    # are mixed in (issue #101, SPEC 9). Sought with a minimum version of its
    # own in the root CMakeLists; that is where the reason stands.
    KF6::ColorScheme
    KF6::ConfigCore
    # KStandardShortcut (ConfigGui), KMessageWidget (WidgetsAddons) and
    # KWindowConfig (WindowSystem) — SPEC 15.
    KF6::ConfigGui
    KF6::I18n
    KF6::WidgetsAddons
    KF6::WindowSystem
)

# A library, not part of the executable: the tests reach the D-Bus entry points
# and the conflict rule that way (SPEC 16).
add_library(denkzettelshell STATIC
    shell/appidentity.cpp
    shell/daemonservice.cpp
    shell/firstrun.cpp
    shell/globalshortcuts.cpp
    shell/shortcutconflict.cpp
    shell/shortcutregistration.cpp
    shell/trayicon.cpp
)

target_include_directories(denkzettelshell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelshell PUBLIC
    denkzettelstore
    Qt6::DBus
    Qt6::Widgets
    KF6::ConfigCore
    # KAboutData — shell/appidentity.cpp, SPEC 15.
    KF6::CoreAddons
    KF6::GlobalAccel
    KF6::I18n
    KF6::Notifications
    KF6::StatusNotifierItem
    # KAboutApplicationDialog — the about dialog behind the tray entry of the
    # same name (issue #87, SPEC 10 and 15). The only thing this project uses
    # out of KXmlGui; writing the dialog by hand would be a copy of it.
    KF6::XmlGui
)

# The one road the version number takes from CMake into the code. It stands in
# project(... VERSION ...) of the root CMakeLists and nowhere else (SPEC 15);
# a second copy in a source file would be the thing that goes stale unnoticed.
target_compile_definitions(denkzettelshell PRIVATE
    DENKZETTEL_VERSION="${PROJECT_VERSION}")

add_executable(denkzetteld
    main.cpp
    resources.qrc
)

target_include_directories(denkzetteld PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzetteld PRIVATE
    denkzettelcapture
    denkzettelshell
    denkzetteltranscribe
    denkzettelui
    Qt6::Widgets
    KF6::ConfigCore
    KF6::DBusAddons
    KF6::I18n
)

install(TARGETS denkzetteld ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
