cmake_minimum_required(VERSION 3.17)
PROJECT(shader-transpiler)

SET(ST_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")

add_subdirectory("${ST_DEPS_DIR}/ShaderConductor")

add_executable("${PROJECT_NAME}" "src/main.cpp")
target_include_directories("${PROJECT_NAME}" PRIVATE "${ST_DEPS_DIR}/json/json.hpp")

target_link_libraries("${PROJECT_NAME}" PRIVATE "shaderconductor")