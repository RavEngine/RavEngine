cmake_minimum_required(VERSION 3.13)

# protobuf and shaderc need separate build sub-directory so that they can be compiled for host machine
project(host-tools)

# shaderc for build machine

message("Making host tools")
set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_INSTALL_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(../bgfx.cmake ${CMAKE_BINARY_DIR}/bgfx.cmake)

set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")
set(protobuf_MODULE_COMPATIBLE ON CACHE INTERNAL "")
add_subdirectory(../GameNetworkingSockets/protobuf/cmake ${CMAKE_BINARY_DIR}/protobuf)
