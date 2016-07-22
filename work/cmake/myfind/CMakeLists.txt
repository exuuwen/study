cmake_minimum_required(VERSION 2.8)

project(myfind)
set(MYFIND_VERSION "1.0.0")
set(MYFIND_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/myfind)
set(MYFIND_LIB_DIR ${CMAKE_BINARY_DIR}/myfind)

add_subdirectory(myfind)

export(PACKAGE myfind)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-config-version.cmake.in ${PROJECT_NAME}-config-version.cmake @ONLY)

