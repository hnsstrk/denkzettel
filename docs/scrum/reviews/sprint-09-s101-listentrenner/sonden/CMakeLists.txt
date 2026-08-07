# Sonden zu Issue #101. Sie linken gegen nichts aus dem Projekt — gemessen wird
# die Rasterung von Qt, nicht der Delegate.
cmake_minimum_required(VERSION 3.20)
project(denkzettel_sonden_101 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.7.0 REQUIRED COMPONENTS Gui)

add_executable(rasterlage rasterlage.cpp)
target_link_libraries(rasterlage PRIVATE Qt6::Gui)
