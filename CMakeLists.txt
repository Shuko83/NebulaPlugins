cmake_minimum_required(VERSION 3.19)
project(NebulaPlugins VERSION 0.1.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/CMakeUtils/CMakeUtils.cmake)

set(NEBULA_VERSION 3.0.0)
# The package is named after what it is built for and with: build the plugins with the same compiler and Qt.
set(NEBULA_QT_VERSION 6.12)
set(NEBULA_PACKAGE Nebula_${NEBULA_VERSION}_win-x64_msvc2026_Qt${NEBULA_QT_VERSION})
cmu_download_package(
    NAME Nebula
    URL https://github.com/Shuko83/Nebula/releases/download/${NEBULA_VERSION}/${NEBULA_PACKAGE}.zip
    SHA256 921779c728a412951a01840814592361946a5d9d0cd94a8b0e32a1af8ccb1908
    FIND_PACKAGE_ARGS ${NEBULA_VERSION} REQUIRED COMPONENTS Core
)
# With an earlier Qt, Core.dll does not load: it uses functions that Qt only has since the version it is built with, and
# the tests, which load it, end with 0xc0000139 (entry point not found).
if(Qt6_VERSION VERSION_LESS NEBULA_QT_VERSION)
    message(FATAL_ERROR "Nebula ${NEBULA_VERSION} is built with Qt ${NEBULA_QT_VERSION}, but Qt ${Qt6_VERSION} is found "
        "(${Qt6_DIR}): point QTDIR at Qt ${NEBULA_QT_VERSION} or later, then configure a new build directory, or pass "
        "-DQt6_DIR=<Qt>/lib/cmake/Qt6")
endif()

