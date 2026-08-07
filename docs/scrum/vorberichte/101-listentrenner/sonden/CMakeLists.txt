# Bauplan der Messsonden zur Vorprüfung von Issue #101 (Bearbeiter A).
#
# Eigener Bauplatz, installiert nichts nach /usr, ändert keinen Produktivcode.
# Die Sonden linken gegen die im angegebenen Bauplatz gebaute Bibliothek —
# dieselbe Binärlage, die die Tests fahren, statt eines Nachbaus.
#
# Aufruf: cmake -B <bau> -S sonden -DDENKZETTEL_BUILD=<Bauplatz des Projekts>

cmake_minimum_required(VERSION 3.20)
project(vorbericht-101 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets DBus Sql)
find_package(KF6ColorScheme REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6WindowSystem REQUIRED)

if(NOT DENKZETTEL_BUILD)
    message(FATAL_ERROR "DENKZETTEL_BUILD muss auf den Bauplatz des Projekts zeigen")
endif()
get_filename_component(DENKZETTEL_QUELLE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src" ABSOLUTE)

foreach(sonde kanten farbe kontrastwert)
    add_executable(${sonde} ${sonde}.cpp)
    target_include_directories(${sonde} PRIVATE "${DENKZETTEL_QUELLE}")
    target_link_libraries(${sonde} PRIVATE
        "${DENKZETTEL_BUILD}/lib/libdenkzettelui.a"
        "${DENKZETTEL_BUILD}/lib/libdenkzettelstore.a"
        Qt6::Widgets
        Qt6::DBus
        Qt6::Sql
        KF6::ColorScheme
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::CoreAddons
        KF6::I18n
        KF6::WidgetsAddons
        KF6::WindowSystem
    )
endforeach()
