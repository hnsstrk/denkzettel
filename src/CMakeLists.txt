add_executable(denkzetteld
    main.cpp
    shell/trayicon.cpp
)

target_include_directories(denkzetteld PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(denkzetteld PRIVATE
    Qt6::Widgets
    KF6::DBusAddons
    KF6::I18n
    KF6::StatusNotifierItem
)

install(TARGETS denkzetteld ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
