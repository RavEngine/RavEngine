cmake_minimum_required(VERSION 3.17)
PROJECT(shader-transpiler)

SET(ST_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")

add_subdirectory("${ST_DEPS_DIR}/glslang")
add_subdirectory("${ST_DEPS_DIR}/SPIRV-Cross")
#add_subdirectory("${ST_DEPS_DIR}/SPIRV-Tools")

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/ShaderTranspiler/*.hpp")
file(GLOB CHEADERS "include/ShaderTranspiler/*.h")


add_executable("${PROJECT_NAME}" ${SOURCES} ${CHEADERS} ${HEADERS})
target_include_directories("${PROJECT_NAME}" PRIVATE 
    "${ST_DEPS_DIR}/json/"
    "${ST_DEPS_DIR}/cxxopts/"
    "include/ShaderTranspiler/"
    )

target_link_libraries("${PROJECT_NAME}" PRIVATE 
    "glslang"
    "SPIRV"
    "spirv-cross-glsl"
    "spirv-cross-hlsl"
    "spirv-cross-msl"
    "spirv-cross-reflect"
)