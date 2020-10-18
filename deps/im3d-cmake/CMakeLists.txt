cmake_minimum_required(VERSION 3.17)
project(im3d)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRCS "im3d/*.h" "im3d/*.hpp" "im3d/*.cpp" "im3d/*.c")

add_library("${PROJECT_NAME}" ${SRCS})
target_include_directories("${PROJECT_NAME}" PUBLIC "im3d/")