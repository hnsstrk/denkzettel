cmake_minimum_required(VERSION 3.20)

project(uxshots LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.7.0 REQUIRED COMPONENTS Core Gui Widgets Sql Test)
find_package(KF6Config REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6WindowSystem REQUIRED)

set(WORKTREE "" CACHE PATH "Arbeitsbaum mit Quellen und Build der Story")

add_executable(uxshots uxshots.cpp)

target_include_directories(uxshots PRIVATE ${WORKTREE}/src)

target_link_libraries(uxshots PRIVATE
    ${WORKTREE}/build/lib/libdenkzettelui.a
    ${WORKTREE}/build/lib/libdenkzettelstore.a
    Qt6::Widgets
    Qt6::Sql
    Qt6::Test
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::I18n
    KF6::WidgetsAddons
    KF6::WindowSystem
)

add_executable(uxshots-nachpruefung uxshots-nachpruefung.cpp)
target_include_directories(uxshots-nachpruefung PRIVATE ${WORKTREE}/src)
target_link_libraries(uxshots-nachpruefung PRIVATE
    ${WORKTREE}/build/lib/libdenkzettelui.a
    ${WORKTREE}/build/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::Test
    KF6::ConfigCore KF6::ConfigGui KF6::I18n KF6::WidgetsAddons KF6::WindowSystem
)

add_executable(iconprobe iconprobe.cpp)
target_link_libraries(iconprobe PRIVATE Qt6::Widgets Qt6::Test KF6::WidgetsAddons KF6::I18n)
