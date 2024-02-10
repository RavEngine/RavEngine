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
add_subdirectory(../GameNetworkingSockets/protobuf/cmake ${CMAKE_BINARY_DIR}/protobuf)

set_target_properties("protoc" PROPERTIES UNITY_BUILD ON)
