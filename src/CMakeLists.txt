add_library(denkzettelstore STATIC
    store/store.cpp
)

target_include_directories(denkzettelstore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelstore PUBLIC
    Qt6::Core
    Qt6::Sql
)

add_library(denkzettelcapture STATIC
    capture/capturewindow.cpp
    capture/textareaheight.cpp
)

target_include_directories(denkzettelcapture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelcapture PUBLIC
    denkzettelstore
    Qt6::Widgets
    KF6::I18n
)

# A library, not part of the executable: the tests reach the D-Bus entry points
# and the conflict rule that way (SPEC 16).
add_library(denkzettelshell STATIC
    shell/daemonservice.cpp
    shell/globalshortcuts.cpp
    shell/shortcutconflict.cpp
    shell/trayicon.cpp
)

target_include_directories(denkzettelshell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzettelshell PUBLIC
    denkzettelstore
    Qt6::DBus
    Qt6::Widgets
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
    Qt6::Widgets
    KF6::ConfigCore
    KF6::DBusAddons
    KF6::I18n
)

install(TARGETS denkzetteld ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
