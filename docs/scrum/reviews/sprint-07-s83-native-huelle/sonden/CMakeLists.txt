# Bauplan der Messsonden zu #83 — der native Vertrag der Hülle.
#
# Steht neben dem Projektbau, nicht darin: Diese Programme gehören zu einem
# Beleg, nicht zum Erzeugnis. `pruefen.sh` eine Ebene höher ruft sie auf und
# reicht ihnen über DENKZETTEL_LIB_DIR das lib/-Verzeichnis eines übersetzten
# Standes herein — dieselbe Bauart wie bei den Abnahmebefunden von Sprint 6.

cmake_minimum_required(VERSION 3.20)

project(nativehuelle-s83 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS DBus Gui Sql Widgets)
find_package(KF6ColorScheme REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6Svg REQUIRED)
find_package(KF6WindowSystem REQUIRED)

set(DENKZETTEL_LIB_DIR "" CACHE PATH "lib/-Verzeichnis eines übersetzten Denkzettel-Standes")
get_filename_component(DENKZETTEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

# Sonden ohne Fenster: sie messen die Grafik des Themes, nicht das Erzeugnis.
foreach(probe huellengrafik schemafarbe ksvgselektoren)
    add_executable(${probe} ${probe}.cpp)
    target_link_libraries(${probe} PRIVATE Qt6::Gui KF6::ColorScheme KF6::ConfigCore KF6::Svg)
endforeach()

# Sonden mit dem **echten** Erfassungsfenster. Sie hängen an der übersetzten
# Bibliothek des Projekts: Was sie messen, soll der gelieferte Code sein und
# nicht ein Nachbau davon.
foreach(probe fensterlage weichzeichner)
    add_executable(${probe} ${probe}.cpp)
    target_include_directories(${probe} PRIVATE "${DENKZETTEL_ROOT}/src")
    target_link_libraries(${probe} PRIVATE
        "${DENKZETTEL_LIB_DIR}/libdenkzettelcapture.a"
        "${DENKZETTEL_LIB_DIR}/libdenkzettelstore.a"
        Qt6::DBus
        Qt6::Sql
        Qt6::Widgets
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::I18n
        KF6::Svg
        KF6::WindowSystem
    )
endforeach()
