# Bauplan der Messsonden zur Vorprüfung von #83.
#
# Steht neben dem Projektbau, nicht darin: Diese Programme gehören zu einem
# Beleg, nicht zum Erzeugnis. `pruefen.sh` eine Ebene höher ruft sie auf und
# baut in einen eigenen Bauplatz — `build/` der Repositoriumswurzel gehört
# während eines Sprints den Strängen.

cmake_minimum_required(VERSION 3.20)

project(vorpruefung-83 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)
find_package(KF6Svg REQUIRED)
find_package(KF6WindowSystem REQUIRED)

# Sonde 1 — FrameSvg: Bildpunktverhältnis, Maskenregion, Ränder.
add_executable(rahmenmasse rahmenmasse.cpp)
target_link_libraries(rahmenmasse PRIVATE Qt6::Gui KF6::Svg)

# Sonde 2 — das Fenster: Plattformgleichheit, Bindung, Weichzeichner.
add_executable(fensterlauf fensterlauf.cpp)
target_link_libraries(fensterlauf PRIVATE Qt6::Widgets KF6::Svg KF6::WindowSystem)

# Sonde 4 — Beleg der Weichzeichner-Anmeldung über eine Bildschirmaufnahme.
add_executable(weichzeichnerbeleg weichzeichnerbeleg.cpp)
target_link_libraries(weichzeichnerbeleg PRIVATE Qt6::Widgets KF6::Svg KF6::WindowSystem)
