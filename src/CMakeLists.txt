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

add_executable(denkzetteld
    main.cpp
    shell/daemonservice.cpp
    shell/trayicon.cpp
)

target_include_directories(denkzetteld PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzetteld PRIVATE
    denkzettelcapture
    Qt6::DBus
    Qt6::Widgets
    KF6::DBusAddons
    KF6::I18n
    KF6::StatusNotifierItem
)

install(TARGETS denkzetteld ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
