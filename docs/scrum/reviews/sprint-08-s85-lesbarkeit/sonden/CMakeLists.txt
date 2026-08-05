# Bauplan der Belegsonde zu #85.
#
# Eigener Bauplatz, installiert nichts nach /usr. `sitzungsbeleg` linkt gegen
# die im Bauplatz des Strangs gebaute Bibliothek des Projekts — dieselbe
# Binärlage, die die Tests fahren, statt eines Nachbaus.
#
# Die übrigen Sonden dieser Story stehen schon versioniert im Ordner der
# Vorprüfung (`docs/scrum/vorberichte/85-lesbarkeit-fremde-themes/sonden/`) und
# werden von dort gebaut, nicht hierher kopiert: zwei Fassungen derselben Sonde
# gehen auseinander, und dann misst man mit der falschen.
#
# Aufruf: cmake -B build -S sonden -DDENKZETTEL_BUILD=<Bauplatz des Strangs>

cmake_minimum_required(VERSION 3.20)
project(review-85 LANGUAGES CXX)
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

add_executable(sitzungsbeleg sitzungsbeleg.cpp)
target_include_directories(sitzungsbeleg PRIVATE "${DENKZETTEL_QUELLE}")
target_link_libraries(sitzungsbeleg PRIVATE
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
