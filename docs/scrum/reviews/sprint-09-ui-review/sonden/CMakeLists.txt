# Bauplan der Messsonde des UI/UX-Reviews zu Sprint 9.
#
# Steht neben dem Projektbau, nicht darin: Das Programm gehört zu einem Beleg
# und nicht zum Erzeugnis. Über DENKZETTEL_LIB_DIR bekommt es das lib/-
# Verzeichnis eines übersetzten Standes gereicht — dieselbe Bauart wie bei den
# Sonden zu #83 und #100.

cmake_minimum_required(VERSION 3.20)

project(uxreview-sprint09 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS DBus Gui Sql Test Widgets)
find_package(KF6ColorScheme REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6Svg REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6WindowSystem REQUIRED)

set(DENKZETTEL_LIB_DIR "" CACHE PATH "lib/-Verzeichnis eines übersetzten Denkzettel-Standes")
get_filename_component(DENKZETTEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

add_executable(uxsonde uxsonde.cpp)
target_include_directories(uxsonde PRIVATE "${DENKZETTEL_ROOT}/src")
target_link_libraries(uxsonde PRIVATE
    "${DENKZETTEL_LIB_DIR}/libdenkzettelui.a"
    "${DENKZETTEL_LIB_DIR}/libdenkzettelcapture.a"
    "${DENKZETTEL_LIB_DIR}/libdenkzettelstore.a"
    Qt6::DBus
    Qt6::Sql
    Qt6::Test
    Qt6::Widgets
    KF6::ColorScheme
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    KF6::Svg
    KF6::WidgetsAddons
    KF6::WindowSystem
)
