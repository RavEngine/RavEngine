cmake_minimum_required(VERSION 3.5)

project(libaiff)

include(CheckLibraryExists)
check_library_exists("m" "sin" "" HAVE_LIBM)

add_library(aiff STATIC "libaiff.all.c")
target_include_directories(aiff PUBLIC ".")
if (HAVE_LIBM)
  target_link_libraries(aiff PRIVATE "m")
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(aiff PRIVATE "-Wall")
endif()
add_library(aiff::aiff ALIAS aiff)
