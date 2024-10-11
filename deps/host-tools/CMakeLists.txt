cmake_minimum_required(VERSION 3.13)

# protobuf needs separate build sub-directory so that it can be compiled for host machine
project(host-tools)

if(MSVC)
  add_definitions(/MP)
  add_definitions(/Qpar)
endif()

set(RGL_ENABLE_RGLC ON)
set(RGL_SKIP_BACKEND_CHECK ON)  # we're only interested in the compiler here, we don't need the SDKs
add_subdirectory("../RGL" "${CMAKE_BINARY_DIR}/RGL")
set_target_properties(rglc
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/RGL"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/RGL"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/RGL"
)

# protobuf for build machine
set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")
set(protobuf_MODULE_COMPATIBLE ON CACHE INTERNAL "")
add_subdirectory(../GameNetworkingSockets/protobuf/cmake "${CMAKE_BINARY_DIR}/protobuf")

set_target_properties("protoc" PROPERTIES UNITY_BUILD ON)

# flatc for build machine
set(FLATBUFFERS_BUILD_TESTS OFF CACHE INTERNAL "")
set(FLATBUFFERS_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(../SteamAudio-All/flatbuffers "${CMAKE_BINARY_DIR}/flatc")

# rvesc
include(../cmrc/CMakeRC.cmake)
add_subdirectory(../simdjson EXCLUDE_FROM_ALL "${CMAKE_BINARY_DIR}/simdjson")
add_subdirectory(../fmt EXCLUDE_FROM_ALL "${CMAKE_BINARY_DIR}/fmt")
set(CXXOPTS_BUILD_TESTS OFF CACHE INTERNAL "")
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(CXXOPTS_ENABLE_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(../cxxopts EXCLUDE_FROM_ALL "${CMAKE_BINARY_DIR}/cxxopts")

include(../../cmake/rvesc.cmake)

set_target_properties(rvesc
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/RVESC"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/RVESC"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/RVESC"
)

# tooling dependencies

add_subdirectory(../glm EXCLUDE_FROM_ALL "${CMAKE_BINARY_DIR}/glm")

SET(IGNORE_GIT_HASH ON CACHE INTERNAL "")
SET(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "")
set(ASSIMP_INSTALL OFF CACHEN INTERNAL "")
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "")
set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "")
add_subdirectory(../assimp EXCLUDE_FROM_ALL "${CMAKE_BINARY_DIR}/assimp")

add_subdirectory(../meshoptimizer "${CMAKE_BINARY_DIR}/meshoptimizer")

include(../../cmake/importers.cmake)

include(../../cmake/rtti.cmake)

disable_rtti_in_dir("${CMAKE_CURRENT_LIST_DIR}")