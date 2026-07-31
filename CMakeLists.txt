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

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
