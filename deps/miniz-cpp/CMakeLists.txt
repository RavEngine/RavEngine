cmake_minimum_required(VERSION 3.2)
project(miniz-cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/pipe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/read.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/write.cpp)

foreach(EXAMPLE_SOURCE IN ITEMS ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    set(EXAMPLE_EXECUTABLE example-${EXAMPLE_NAME})

    add_executable(${EXAMPLE_EXECUTABLE} ${EXAMPLE_SOURCE})

    target_include_directories(${EXAMPLE_EXECUTABLE}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()
