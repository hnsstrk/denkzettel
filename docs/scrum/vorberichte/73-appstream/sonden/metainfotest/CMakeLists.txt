# Sonde: das Nachweisverfahren aus AK 4 von #73 im Kleinen — Installation über
# DESTDIR in den Bauordner, danach appstreamcli validate am Installationsort.
# Baut nichts aus dem Produkt; misst nur den Mechanismus.
cmake_minimum_required(VERSION 3.20)
project(metainfoprobe NONE)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
enable_testing()

install(FILES org.denkzettel.Denkzettel.metainfo.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

find_program(APPSTREAMCLI appstreamcli)

add_test(NAME metainfotest
    COMMAND ${CMAKE_COMMAND}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DSTAGING_DIR=${CMAKE_CURRENT_BINARY_DIR}/metainfotest-root
        -DAPPSTREAMCLI=${APPSTREAMCLI}
        -DMETAINFO_ENTRY=${KDE_INSTALL_FULL_METAINFODIR}/org.denkzettel.Denkzettel.metainfo.xml
        -P ${CMAKE_CURRENT_SOURCE_DIR}/metainfotest.cmake)
