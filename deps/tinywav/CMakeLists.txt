cmake_minimum_required(VERSION 3.13)

project(tinywav)

add_library(${PROJECT_NAME} tinywav/tinywav.c tinywav/tinywav.h)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/tinywav")