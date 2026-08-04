# Bauplan der vier Messsonden zu #55. Steht bewusst neben dem Projektbau und
# nicht darin: Diese Programme gehören zu einem Beleg, nicht zum Erzeugnis, und
# ein `cmake --build build` des Projekts soll sie nicht anfassen.
#
# Aufgerufen wird das hier normalerweise nicht von Hand, sondern von pruefen.sh
# eine Ebene höher.

cmake_minimum_required(VERSION 3.20)

project(huellenbelege-s6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(KF6Config REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6Svg REQUIRED)

# Alle vier messen fremde Bibliotheken — KSvg, KConfig, KDirWatch — und keinen
# Projektcode. Sie brauchen deshalb keinen übersetzten Denkzettel-Stand.
add_executable(ksvg-themequelle ksvg-themequelle.cpp)
target_link_libraries(ksvg-themequelle PRIVATE Qt6::Gui KF6::ConfigCore KF6::Svg)

add_executable(themewechsel-zustellung themewechsel-zustellung.cpp)
target_link_libraries(themewechsel-zustellung PRIVATE Qt6::Core KF6::ConfigCore KF6::CoreAddons)

add_executable(framesvg-nachziehen framesvg-nachziehen.cpp)
target_link_libraries(framesvg-nachziehen PRIVATE Qt6::Gui KF6::Svg)

add_executable(theme-eckstuecke theme-eckstuecke.cpp)
target_link_libraries(theme-eckstuecke PRIVATE Qt6::Gui KF6::Svg)

add_executable(schattenkacheln schattenkacheln.cpp)
target_link_libraries(schattenkacheln PRIVATE Qt6::Gui KF6::Svg)

# Messung 5 misst am echten Produktivcode und läuft als Einzige auf der
# laufenden Sitzung, nicht offscreen. Sie braucht deshalb einen übersetzten
# Denkzettel-Stand; LIESMICH.md nennt den Aufruf.
set(DENKZETTEL_LIB_DIR "" CACHE PATH "lib/-Verzeichnis eines übersetzten Denkzettel-Standes")

if(DENKZETTEL_LIB_DIR)
    get_filename_component(DENKZETTEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

    find_package(Qt6 REQUIRED COMPONENTS Sql Widgets)
    find_package(KF6I18n REQUIRED)
    find_package(KF6WindowSystem REQUIRED)

    add_executable(schatten-am-compositor schatten-am-compositor.cpp)
    target_include_directories(schatten-am-compositor PRIVATE "${DENKZETTEL_ROOT}/src")
    target_link_libraries(schatten-am-compositor PRIVATE
        "${DENKZETTEL_LIB_DIR}/libdenkzettelcapture.a"
        "${DENKZETTEL_LIB_DIR}/libdenkzettelstore.a"
        Qt6::Sql
        Qt6::Widgets
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::I18n
        KF6::Svg
        KF6::WindowSystem
    )
endif()
