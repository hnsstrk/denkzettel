# Sonden der Messung B zu Issue #100.
#
# Eigenes Projekt, eigenes Bauverzeichnis (build-b/) — nicht `build/` im
# Projektwurzelverzeichnis, in dem während dieser Vorprüfung drei weitere
# Stränge arbeiten.
#
#   cmake -B ../build-b -S . -DCMAKE_BUILD_TYPE=Debug && cmake --build ../build-b

cmake_minimum_required(VERSION 3.28)
project(feldsonden LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(KF6Svg REQUIRED)

# Der Projektstamm, aus der Lage dieses Ordners abgeleitet. Als Variable und
# nicht als eingetragener Pfad: ein Prüfmittel, das nur auf einer Maschine baut,
# ist keines.
get_filename_component(DENKZETTEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

# M1 (Auflösung, Rand und Farben je Theme), M4 (Svg::color je Farbsatz),
# M5 (frisches ImageSet gegen Umbenennen).
add_executable(feldsonde feldsonde.cpp)
target_link_libraries(feldsonde PRIVATE Qt6::Widgets KF6::Svg)

# M2: das Feld auf der Hülle über zwei Gründen — die Schichtung, ohne die die
# Abhebung nicht zu messen ist.
add_executable(schichtsonde schichtsonde.cpp)
target_link_libraries(schichtsonde PRIVATE Qt6::Widgets KF6::Svg)

# Welches Theme-Paar die vorhandenen Wähler der Testsuite liefern.
add_executable(paarsonde paarsonde.cpp)
target_include_directories(paarsonde PRIVATE "${DENKZETTEL_ROOT}/tests")
target_compile_definitions(paarsonde PRIVATE DENKZETTEL_TEST_THEMES="${DENKZETTEL_ROOT}/tests/themes")
target_link_libraries(paarsonde PRIVATE Qt6::Widgets KF6::Svg)

# M6: die Deckung von Hülle und Feld — die Zahl, an der die beiden Prüfsätze
# hängen, die den Fenstermittelpunkt abtasten.
add_executable(alphasonde alphasonde.cpp)
target_link_libraries(alphasonde PRIVATE Qt6::Widgets KF6::Svg)

# `geometriesonde.cpp` steht bewusst nicht hier: sie braucht das Fenster selbst
# und wird gegen einen eigenen Projektbau gelinkt. Der vollständige Aufruf steht
# im Kopf der Datei.

# Nachprüfung der Messungen aus Bericht A, die in Messung B fehlten oder ihr
# widersprechen (V1 Deckung mit `opaque`, V2 unbekannter Themename, V3 Grund).
add_executable(nachpruefung nachpruefung.cpp)
target_link_libraries(nachpruefung PRIVATE Qt6::Widgets KF6::Svg)
