# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

cmake_minimum_required(VERSION 3.10)

project(imath-examples)

find_package(Imath)

# The introductory example on the main docs page
add_executable(imath-intro intro.cpp)

# A main() that executes all the example code snippets
add_executable(imath-examples
  main.cpp
  Color3.cpp
  Color4.cpp
  Euler.cpp
  Frustum.cpp
  Interval.cpp
  Line3.cpp
  Matrix22.cpp
  Matrix33.cpp
  Matrix44.cpp
  Plane3.cpp
  Quat.cpp
  Shear6.cpp
  Sphere3.cpp
  Vec2.cpp
  Vec3.cpp
  Vec4.cpp
  half.cpp
)

target_link_libraries(imath-intro Imath::Imath)
target_link_libraries(imath-examples Imath::Imath)

set_target_properties(imath-examples PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
set_target_properties(imath-intro PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME Imath.imath-intro COMMAND $<TARGET_FILE:imath-intro>)
add_test(NAME Imath.imath-examples COMMAND $<TARGET_FILE:imath-examples>)

enable_testing()
