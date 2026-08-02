# Bauplan der vier Schätzsonden. Steht bewusst neben dem Projektbau und nicht
# darin: Diese Programme gehören zu einem Beleg, nicht zum Erzeugnis, und ein
# `cmake --build build` des Projekts soll sie nicht anfassen.
#
# Aufgerufen wird das hier normalerweise nicht von Hand, sondern von pruefen.sh.

cmake_minimum_required(VERSION 3.20)

project(schaetzbelege-s6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Wurzel des Repositories, aus der Lage dieser Datei abgeleitet — die Sonden
# laufen dadurch bei jedem, der das Repository irgendwo ausgecheckt hat, und
# nicht nur im Arbeitsbaum dessen, der sie geschrieben hat.
get_filename_component(DENKZETTEL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

# Verzeichnis mit den statischen Bibliotheken eines übersetzten Projektstandes.
# pruefen.sh füllt es; von Hand zeigt man es auf das lib/ eines eigenen Baus.
set(DENKZETTEL_LIB_DIR "${DENKZETTEL_ROOT}/build/lib" CACHE PATH
    "lib/-Verzeichnis eines übersetzten Denkzettel-Standes")

find_package(Qt6 REQUIRED COMPONENTS Sql Widgets)
find_package(KF6Config REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6WindowSystem REQUIRED)

# Messung 1 und 2 kommen ohne Projektcode aus: Sie messen Qt und KConfig selbst.
add_executable(schriftalterung schriftalterung.cpp)
target_link_libraries(schriftalterung PRIVATE Qt6::Widgets)

add_executable(kdeglobals-watcher kdeglobals-watcher.cpp)
target_link_libraries(kdeglobals-watcher PRIVATE Qt6::Widgets KF6::ConfigCore)

# Messung 3 und 4 messen am echten Produktivcode — eine nachgebaute Formel wäre
# ein Prüfaufbau, in dem der Fehler gar nicht auftreten kann.
add_executable(capture-schriftwechsel capture-schriftwechsel.cpp)
target_include_directories(capture-schriftwechsel PRIVATE "${DENKZETTEL_ROOT}/src")
target_link_libraries(capture-schriftwechsel PRIVATE
    "${DENKZETTEL_LIB_DIR}/libdenkzettelcapture.a"
    "${DENKZETTEL_LIB_DIR}/libdenkzettelstore.a"
    Qt6::Sql
    Qt6::Widgets
    KF6::I18n
)

add_executable(bibliothek-zeilenhoehe bibliothek-zeilenhoehe.cpp)
target_include_directories(bibliothek-zeilenhoehe PRIVATE "${DENKZETTEL_ROOT}/src")
target_link_libraries(bibliothek-zeilenhoehe PRIVATE
    "${DENKZETTEL_LIB_DIR}/libdenkzettelui.a"
    "${DENKZETTEL_LIB_DIR}/libdenkzettelstore.a"
    Qt6::Sql
    Qt6::Widgets
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
    KF6::WidgetsAddons
    KF6::WindowSystem
)
