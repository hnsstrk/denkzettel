# Bauplan der Messsonde zur Vorprüfung von #71.
#
# Steht neben dem Projektbau, nicht darin: Das Programm gehört zu einem Beleg,
# nicht zum Erzeugnis. `pruefen.sh` eine Ebene höher baut das Projekt in einen
# eigenen Bauplatz und reicht ihn hier über DENKZETTEL_BUILD herein —
# `build/` der Repositoriumswurzel gehört während eines Sprints den Strängen.

cmake_minimum_required(VERSION 3.20)

project(vorpruefung-71 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT DEFINED DENKZETTEL_BUILD)
    message(FATAL_ERROR "DENKZETTEL_BUILD fehlt — der Bauplatz mit libdenkzettelui.a")
endif()
if(NOT DEFINED DENKZETTEL_SRC)
    message(FATAL_ERROR "DENKZETTEL_SRC fehlt — das Verzeichnis src/ des Projekts")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
find_package(KF6Config REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6WindowSystem REQUIRED)

add_executable(klicksonde klicksonde.cpp)
target_include_directories(klicksonde PRIVATE ${DENKZETTEL_SRC})
target_link_libraries(klicksonde PRIVATE
    ${DENKZETTEL_BUILD}/lib/libdenkzettelui.a
    ${DENKZETTEL_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets
    Qt6::Sql
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
    KF6::WidgetsAddons
    KF6::WindowSystem
)
