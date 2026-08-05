cmake_minimum_required(VERSION 3.20)
project(kopfsonde LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Test)
find_package(KF6 REQUIRED COMPONENTS I18n WidgetsAddons Config WindowSystem)

# Die Sonde misst am Quellstand des Projekts, aber in einem eigenen Bauplatz —
# `build/` gehört einem anderen Strang, `build-vor70/` dieser Vorprüfung.
set(DZ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src)

add_library(dzui STATIC
    ${DZ_SRC}/store/store.cpp
    ${DZ_SRC}/ui/elidedlines.cpp
    ${DZ_SRC}/ui/librarywindow.cpp
    ${DZ_SRC}/ui/notelistdelegate.cpp
    ${DZ_SRC}/ui/notelistmodel.cpp
    ${DZ_SRC}/ui/pendingdeletion.cpp
    ${DZ_SRC}/ui/timestampformat.cpp
)
target_include_directories(dzui PUBLIC ${DZ_SRC})
target_link_libraries(dzui PUBLIC
    Qt6::Widgets Qt6::Sql
    KF6::ConfigCore KF6::ConfigGui KF6::I18n KF6::WidgetsAddons KF6::WindowSystem)

add_executable(kopfsonde kopfsonde.cpp)
target_link_libraries(kopfsonde dzui Qt6::Test)

# Zweite Bibliothek aus derselben Quellmenge, aber mit der Kandidatenfassung
# von librarywindow.cpp aus ../mischprobe/a-70.cpp. Das Repositorium bleibt
# unangetastet — src/ui/librarywindow.cpp wird nicht geändert.
add_library(dzui70 STATIC
    ${DZ_SRC}/store/store.cpp
    ${DZ_SRC}/ui/elidedlines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../kandidat/librarywindow.cpp
    ${DZ_SRC}/ui/notelistdelegate.cpp
    ${DZ_SRC}/ui/notelistmodel.cpp
    ${DZ_SRC}/ui/pendingdeletion.cpp
    ${DZ_SRC}/ui/timestampformat.cpp
)
target_include_directories(dzui70 PUBLIC ${DZ_SRC})
target_link_libraries(dzui70 PUBLIC
    Qt6::Widgets Qt6::Sql
    KF6::ConfigCore KF6::ConfigGui KF6::I18n KF6::WidgetsAddons KF6::WindowSystem)

# Die echte Testauflage des Projekts, gegen die Kandidatenfassung gebaut.
set(DZ_TESTS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../tests)
add_library(testsilence70 OBJECT ${DZ_TESTS}/testsilence.cpp)
target_link_libraries(testsilence70 PRIVATE Qt6::Core)

add_executable(librarytest70 ${DZ_TESTS}/librarytest.cpp)
target_link_libraries(librarytest70 dzui70 Qt6::Test testsilence70)

add_executable(kopfsonde70 kopfsonde.cpp)
target_link_libraries(kopfsonde70 dzui70 Qt6::Test)
