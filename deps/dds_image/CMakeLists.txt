cmake_minimum_required(VERSION 3.11)

project(dds_image LANGUAGES CXX)

option(DDS_BUILD_TESTS "Builds DDS related tests for the library" OFF)

add_library(dds_image INTERFACE)
target_include_directories(dds_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

if (DDS_BUILD_TESTS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()
