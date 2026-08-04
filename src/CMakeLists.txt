add_library(denkzettelstore STATIC
    store/store.cpp
)

target_include_directories(denkzettelstore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelstore PUBLIC
    Qt6::Core
    Qt6::Sql
)

# Spike #62: spellfix1 mitkompilieren. Standardmäßig AUS — der Spike endet mit
# einem Bericht, nicht mit einem Feature. Ist der Schalter aus, wird unterhalb
# nichts konfiguriert, und der Build ist derselbe wie ohne diesen Block.
option(DENKZETTEL_SPIKE_SPELLFIX "spellfix1-Prototyp des Spikes #62 mitbauen (Issue #62)" OFF)

if(DENKZETTEL_SPIKE_SPELLFIX)
    # Das Projekt ist als reines C++-Projekt deklariert; spellfix.c ist C. Die
    # Sprache wird erst hier angeschaltet, damit der Normalbau ohne den Schalter
    # keinen C-Compiler suchen muss.
    enable_language(C)
    find_package(SQLite3 REQUIRED)

    target_sources(denkzettelstore PRIVATE ${CMAKE_SOURCE_DIR}/third_party/spellfix/spellfix.c)

    set_source_files_properties(${CMAKE_SOURCE_DIR}/third_party/spellfix/spellfix.c PROPERTIES
        # Ohne SQLITE_CORE übersetzt sich die Datei als ladbare Erweiterung: Jeder
        # SQLite-Aufruf ginge dann über den Zeiger `sqlite3_api`, den nur
        # sqlite3_load_extension() füllt. Mit SQLITE_CORE fallen diese Makros weg
        # und die Datei ruft dieselbe libsqlite3 direkt, gegen die auch der
        # Qt-Treiber gelinkt ist (siehe /usr/include/sqlite3ext.h, Zeile 727 ff.).
        COMPILE_DEFINITIONS SQLITE_CORE=1
        # Fremdcode, unverändert übernommen: Warnungen darin sind keine, die wir
        # beheben dürften, ohne die Prüfsumme in HERKUNFT.md zu brechen.
        COMPILE_OPTIONS -w
    )

    target_link_libraries(denkzettelstore PUBLIC SQLite3::SQLite3)
endif()

add_library(denkzettelcapture STATIC
    capture/capturewindow.cpp
    capture/textareaheight.cpp
)

target_include_directories(denkzettelcapture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelcapture PUBLIC
    denkzettelstore
    Qt6::Widgets
    # The hull of the capture window (issue #55, SPEC 3 and 15): KSvg renders
    # `dialogs/background` of the desktop theme, KWindowShadow hands its shadow
    # tiles to the compositor. The theme name is not KSvg's to find — it stands
    # in `plasmarc` (KConfigCore), and KDirWatch (KCoreAddons) is what notices
    # it changing under a standing window.
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
    denkzettelstore
    Qt6::Widgets
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
    KF6::GlobalAccel
    KF6::I18n
    KF6::Notifications
    KF6::StatusNotifierItem
)

add_executable(denkzetteld
    main.cpp
    resources.qrc
)

target_include_directories(denkzetteld PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzetteld PRIVATE
    denkzettelcapture
    denkzettelshell
    denkzettelui
    Qt6::Widgets
    KF6::ConfigCore
    KF6::DBusAddons
    KF6::I18n
)

install(TARGETS denkzetteld ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
