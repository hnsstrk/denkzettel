# Bauplan der Messsonden der UX-Beratung zu #83.
#
# Eigener Ordner neben `sonden/` des Vorberichts: dort arbeitet der zweite
# Bearbeiter der Vorprüfung. Diese Sonden fassen weder dessen Bauplatz an noch
# den Bauplatz der Repositoriumswurzel, und sie schreiben keine Einstellung des
# Kunden — jedes Farbschema laeuft in einem eigenen XDG_CONFIG_HOME unter /tmp.

cmake_minimum_required(VERSION 3.20)
project(ux-beratung-83 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(KF6Config REQUIRED)
find_package(KF6ColorScheme REQUIRED)
find_package(KF6Svg REQUIRED)

# Deckung der nativen Hülle je Desktop-Theme, mit und ohne Auswahlpfad opaque.
add_executable(deckung deckung.cpp)
target_link_libraries(deckung PRIVATE Qt6::Gui KF6::Svg KF6::ColorScheme KF6::ConfigCore)

# Dieselbe Hülle als Bild, über hellem und dunklem Grund.
add_executable(huellenbild huellenbild.cpp)
target_link_libraries(huellenbild PRIVATE Qt6::Gui KF6::Svg KF6::ColorScheme KF6::ConfigCore)
