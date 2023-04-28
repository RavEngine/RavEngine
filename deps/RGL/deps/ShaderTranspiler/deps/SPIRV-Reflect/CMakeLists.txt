cmake_minimum_required(VERSION 3.16)
project(SPIRV-Reflect)

add_library(${PROJECT_NAME} spirv_reflect.c spirv_reflect.h include/spirv/unified1/spirv.h)
target_include_directories(${PROJECT_NAME} PUBLIC ./)