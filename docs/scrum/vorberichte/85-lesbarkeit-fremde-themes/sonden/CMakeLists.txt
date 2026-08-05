# Bauplan der Messsonden zur Vorprüfung von #85, Bearbeiter A (UX).
#
# Eigener Bauplatz. Fasst `build/` der Repositoriumswurzel nicht an und
# installiert nichts nach /usr. `fenstertext` linkt gegen die im Bauplatz
# `build-vor85` gebaute Bibliothek des Projekts — dieselbe Binärlage, die die
# Tests fahren, statt eines Nachbaus.
#
# Aufruf: cmake -B build -S sonden -DDENKZETTEL_BUILD=<Pfad zu build-vor85>

cmake_minimum_required(VERSION 3.20)
project(vorbericht-85 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets DBus Sql)
find_package(KF6Config REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6Svg REQUIRED)
find_package(KF6WindowSystem REQUIRED)

if(NOT DENKZETTEL_BUILD)
    message(FATAL_ERROR "DENKZETTEL_BUILD muss auf den Bauplatz des Projekts zeigen")
endif()
get_filename_component(DENKZETTEL_QUELLE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src" ABSOLUTE)

# Woher die Schriftfarbe käme: colors-Datei, KSvg, gezeichnete Fläche.
add_executable(themetext themetext.cpp)
target_link_libraries(themetext PRIVATE Qt6::Gui KF6::Svg KF6::ConfigCore)

# Was das gebaute Fenster von heute mit seinen beiden Textklassen tut.
add_executable(fenstertext fenstertext.cpp)
target_include_directories(fenstertext PRIVATE "${DENKZETTEL_QUELLE}")
target_link_libraries(fenstertext PRIVATE
    "${DENKZETTEL_BUILD}/lib/libdenkzettelcapture.a"
    "${DENKZETTEL_BUILD}/lib/libdenkzettelstore.a"
    Qt6::Widgets
    Qt6::DBus
    Qt6::Sql
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::Svg
    KF6::WindowSystem
)
