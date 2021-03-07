cmake_minimum_required(VERSION 3.13)

# protobuf and shaderc need separate build sub-directory so that they can be compiled for host machine
project(host-tools)

if(MSVC)
  add_definitions(/MP)
  add_definitions(/Qpar)
endif()

# shaderc for build machine
message("Making host tools")
set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_INSTALL_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(../bgfx.cmake ${CMAKE_BINARY_DIR}/bgfx.cmake)

# protobuf for build machine
set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")
set(protobuf_MODULE_COMPATIBLE ON CACHE INTERNAL "")
add_subdirectory(../GameNetworkingSockets/protobuf/cmake ${CMAKE_BINARY_DIR}/protobuf)
