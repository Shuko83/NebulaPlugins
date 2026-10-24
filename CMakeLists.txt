cmake_minimum_required(VERSION 3.19)
project(NebulaPlugins VERSION 0.1.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/CMakeUtils/CMakeUtils.cmake)

set(NEBULA_VERSION 1.0.0)
cmu_download_package(
    NAME Nebula
    URL https://github.com/Shuko83/Nebula/releases/download/${NEBULA_VERSION}/Nebula-${NEBULA_VERSION}-x64.zip
    SHA256 bed76d8811e63743e043d50f59ab85d7bf00231cd1a0eb1c63829873ebc5d2b2
    FIND_PACKAGE_ARGS ${NEBULA_VERSION} REQUIRED COMPONENTS Core
)
