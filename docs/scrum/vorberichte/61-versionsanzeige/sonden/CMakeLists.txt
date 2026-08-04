cmake_minimum_required(VERSION 3.20)
project(vorpruefung61 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 6.7.0 REQUIRED COMPONENTS DBus Widgets)
find_package(KF6 6.0.0 REQUIRED COMPONENTS CoreAddons DBusAddons I18n)

add_executable(versionswege versionswege.cpp)
target_link_libraries(versionswege PRIVATE Qt6::Widgets KF6::CoreAddons KF6::I18n)

add_executable(busname busname.cpp)
target_link_libraries(busname PRIVATE Qt6::DBus Qt6::Widgets KF6::CoreAddons KF6::DBusAddons)
target_compile_definitions(busname PRIVATE DENKZETTEL_VERSION="${PROJECT_VERSION}")

# Genau der Weg, den eine Umsetzung nehmen könnte: die Zahl aus project(...)
# als Präprozessorzeichenkette. Hier steht sie in der Sonde selbst, damit die
# Messung ohne Änderung an der Wurzel-CMakeLists auskommt.
target_compile_definitions(versionswege PRIVATE DENKZETTEL_VERSION="${PROJECT_VERSION}")
