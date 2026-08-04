cmake_minimum_required(VERSION 3.20)
project(uxprobe CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql DBus)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(KF6 REQUIRED COMPONENTS I18n Svg WindowSystem Config CoreAddons)
add_executable(mass mass.cpp)
target_include_directories(mass PRIVATE ${DZ_SRC}/src ${DZ_SRC}/tests)
target_link_libraries(mass PRIVATE
    ${DZ_BUILD}/lib/libdenkzettelcapture.a
    ${DZ_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::DBus
    KF6::I18n KF6::Svg KF6::WindowSystem KF6::ConfigCore KF6::CoreAddons)
target_compile_definitions(mass PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
find_package(Qt6 REQUIRED COMPONENTS Test)
add_executable(wieder wieder.cpp)
target_include_directories(wieder PRIVATE ${DZ_SRC}/src ${DZ_SRC}/tests)
target_link_libraries(wieder PRIVATE
    ${DZ_BUILD}/lib/libdenkzettelcapture.a
    ${DZ_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::DBus Qt6::Test
    KF6::I18n KF6::Svg KF6::WindowSystem KF6::ConfigCore KF6::CoreAddons)
target_compile_definitions(wieder PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
add_executable(maske maske.cpp)
target_include_directories(maske PRIVATE ${DZ_SRC}/tests)
target_link_libraries(maske PRIVATE Qt6::Gui KF6::Svg)
target_compile_definitions(maske PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
add_executable(ecke ecke.cpp)
target_include_directories(ecke PRIVATE ${DZ_SRC}/src ${DZ_SRC}/tests)
target_link_libraries(ecke PRIVATE ${DZ_BUILD}/lib/libdenkzettelcapture.a ${DZ_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::DBus KF6::I18n KF6::Svg KF6::WindowSystem KF6::ConfigCore KF6::CoreAddons)
target_compile_definitions(ecke PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
add_executable(rollbalken rollbalken.cpp)
target_include_directories(rollbalken PRIVATE ${DZ_SRC}/src ${DZ_SRC}/tests)
target_link_libraries(rollbalken PRIVATE ${DZ_BUILD}/lib/libdenkzettelcapture.a ${DZ_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::DBus KF6::I18n KF6::Svg KF6::WindowSystem KF6::ConfigCore KF6::CoreAddons)
target_compile_definitions(rollbalken PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
add_executable(rollbalken2 rollbalken2.cpp)
target_include_directories(rollbalken2 PRIVATE ${DZ_SRC}/src ${DZ_SRC}/tests)
target_link_libraries(rollbalken2 PRIVATE ${DZ_BUILD}/lib/libdenkzettelcapture.a ${DZ_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::DBus Qt6::Test KF6::I18n KF6::Svg KF6::WindowSystem KF6::ConfigCore KF6::CoreAddons)
target_compile_definitions(rollbalken2 PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
add_executable(setzen setzen.cpp)
target_include_directories(setzen PRIVATE ${DZ_SRC}/src ${DZ_SRC}/tests)
target_link_libraries(setzen PRIVATE ${DZ_BUILD}/lib/libdenkzettelcapture.a ${DZ_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::DBus Qt6::Test KF6::I18n KF6::Svg KF6::WindowSystem KF6::ConfigCore KF6::CoreAddons)
target_compile_definitions(setzen PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
add_executable(schrift schrift.cpp)
target_include_directories(schrift PRIVATE ${DZ_SRC}/src ${DZ_SRC}/tests)
target_link_libraries(schrift PRIVATE ${DZ_BUILD}/lib/libdenkzettelcapture.a ${DZ_BUILD}/lib/libdenkzettelstore.a
    Qt6::Widgets Qt6::Sql Qt6::DBus Qt6::Test KF6::I18n KF6::Svg KF6::WindowSystem KF6::ConfigCore KF6::CoreAddons)
target_compile_definitions(schrift PRIVATE DENKZETTEL_TEST_THEMES="${DZ_SRC}/tests/themes")
