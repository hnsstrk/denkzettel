# Bauplan der Messsonden zu den vier Kundenbefunden der Sprint-6-Abnahme.
#
# Steht neben dem Projektbau, nicht darin: Diese Programme gehören zu einem
# Beleg, nicht zum Erzeugnis. `pruefen.sh` eine Ebene höher ruft sie auf.

cmake_minimum_required(VERSION 3.20)

project(abnahmebefunde-s6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Sql Widgets)
find_package(KF6Config REQUIRED)
find_package(KF6ColorScheme REQUIRED)
find_package(KF6ConfigWidgets REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6Svg REQUIRED)
find_package(KF6WindowSystem REQUIRED)

set(DENKZETTEL_LIB_DIR "" CACHE PATH "lib/-Verzeichnis eines übersetzten Denkzettel-Standes")
get_filename_component(DENKZETTEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)

function(denkzettel_probe name)
    add_executable(${name} ${name}.cpp)
    target_include_directories(${name} PRIVATE "${DENKZETTEL_ROOT}/src")
    target_link_libraries(${name} PRIVATE
        "${DENKZETTEL_LIB_DIR}/libdenkzettelcapture.a"
        "${DENKZETTEL_LIB_DIR}/libdenkzettelstore.a"
        Qt6::Sql
        Qt6::Widgets
        KF6::ConfigCore
        KF6::ConfigWidgets
        KF6::CoreAddons
        KF6::I18n
        KF6::Svg
        KF6::WindowSystem
    )
endfunction()

# B1 — Eckform: Maske gegen gemaltes Fenster, unter mehreren Skalierungen.
denkzettel_probe(eckenraster)

# B3 — Flächenfarbe: Palette gegen tatsächlich gezeichnete Pixel.
denkzettel_probe(flaechenfarbe)

# B2 — Schattenkacheln: Herkunft und Vergleich mit einem nativen Fenster.
add_executable(schattenherkunft schattenherkunft.cpp)
target_link_libraries(schattenherkunft PRIVATE Qt6::Gui KF6::ConfigCore KF6::Svg)

# B1 — der Ring aus zwei Rahmen, Schritt für Schritt.
add_executable(huellenring huellenring.cpp)
target_link_libraries(huellenring PRIVATE Qt6::Gui KF6::Svg)

# B1 — das echte Pixelverhältnis am laufenden Bildschirm.
denkzettel_probe(echtelage)

# Nativer Weg — FrameSvg in einem Stueck, colorSet, Kontrast.
add_executable(nativehuelle nativehuelle.cpp)
target_link_libraries(nativehuelle PRIVATE Qt6::Gui KF6::Svg KF6::ColorScheme KF6::ConfigCore)

add_executable(nativekontrast nativekontrast.cpp)
target_link_libraries(nativekontrast PRIVATE Qt6::Gui KF6::Svg KF6::ColorScheme KF6::ConfigCore)
