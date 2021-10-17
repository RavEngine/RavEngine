cmake_minimum_required(VERSION 3.16)
project(im3d)

file(GLOB SRCS "im3d/*.h" "im3d/*.hpp" "im3d/*.cpp" "im3d/*.c")

add_library("${PROJECT_NAME}" ${SRCS})
target_include_directories("${PROJECT_NAME}" PUBLIC "im3d/")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)