# Bauplan der Wechselsonde dieses UI-Reviews.
#
# Eigener Bauplatz, installiert nichts nach /usr. `wechselbeleg` linkt gegen die
# Bibliotheken aus einem **eigenen** Bau des Sprint-Standes — nicht gegen den
# Bauplatz des Strangs, damit der Review nicht auf dessen Zwischenständen misst.
#
# Die Belegsonde des Strangs (`sitzungsbeleg`) wird von ihrem Ort gebaut und
# nicht hierher kopiert: Zwei Fassungen derselben Sonde gehen auseinander, und
# dann misst man mit der falschen.
#
# Aufruf: cmake -B <bauplatz> -S sonden -DDENKZETTEL_BUILD=<eigener Bauplatz>

cmake_minimum_required(VERSION 3.20)
project(ux-review-08 LANGUAGES CXX)
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

add_executable(wechselbeleg wechselbeleg.cpp)
target_include_directories(wechselbeleg PRIVATE "${DENKZETTEL_QUELLE}")
target_link_libraries(wechselbeleg PRIVATE
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
