cmake_minimum_required(VERSION 3.17)

project(simdjson)

add_library(${PROJECT_NAME} src/simdjson.cpp include/simdjson.h)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")