# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

cmake_minimum_required(VERSION 3.12)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0077)
  # enable variables set outside to override options
  cmake_policy(SET CMP0077 NEW)
endif()

# Imath version

project(Imath VERSION 3.1.12 LANGUAGES C CXX)

set(IMATH_VERSION_RELEASE_TYPE "" CACHE STRING "Extra version tag string for Imath build, such as -dev, -beta1, etc.")

set(IMATH_VERSION ${Imath_VERSION})
set(IMATH_VERSION_API "${Imath_VERSION_MAJOR}_${Imath_VERSION_MINOR}")

# Library/shared-object version using libtool versioning policy.
# See https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#
# Library API version (CMake's library VERSION attribute) is of the
# form CURRENT.REVISION.AGE; the CMake SOVERSION attribute corresonds
# to just CURRENT. These produce a .so and a symlink symlink, e.g.:
# libImath-3_1.so.29 -> libImath-3_1.so.29.0.0
#                 ^                     ^  ^ ^
#                 |                     |  | |
#                 CURRENT               |  | AGE
#                                       |  REVISION 
#                                       CURRENT
# When updating:
#   1. no API change: CURRENT.REVISION+1.AGE
#   2. API added:     CURRENT+1.0.AGE+1
#   3. API changed:   CURRENT+1.0.0
#
set(IMATH_LIBTOOL_CURRENT 29)
set(IMATH_LIBTOOL_REVISION 11)
set(IMATH_LIBTOOL_AGE 0)
set(IMATH_LIB_VERSION "${IMATH_LIBTOOL_CURRENT}.${IMATH_LIBTOOL_REVISION}.${IMATH_LIBTOOL_AGE}")
set(IMATH_LIB_SOVERSION ${IMATH_LIBTOOL_CURRENT})

# ImathSetup.cmake declares all the configuration variables visible
# in cmake-gui or similar and the rest of the global
# project setup. Check the context to see what is configurable.
include(config/ImathSetup.cmake)

message(STATUS "Configure ${IMATH_PACKAGE_NAME}, library API version: ${IMATH_LIB_VERSION}")

option(IMATH_INSTALL "Install Imath library" ON)
# Config headers and package config files
add_subdirectory(config)

# Utility function for the repeated boilerplate of defining the libraries
include(config/LibraryDefine.cmake)

# Source code is in src/Imath
add_subdirectory(src/Imath)

# Imath_DIR points to the location of ImathConfig.cmake, which tells
# downstream projects where to find Imath, via find_package(Imath).
set(Imath_DIR "${CMAKE_CURRENT_BINARY_DIR}/config" CACHE PATH "" FORCE)

# Add an empty ImathTargets.cmake file for the config to use. It can
# be empty since we already defined the targets in add_subdirectory().
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config/ImathTargets.cmake" "# Dummy file")

option(PYTHON "Set ON to compile PyImath bindings")
if (PYTHON)
  add_subdirectory(src/python)
endif()

option(BUILD_WEBSITE "Set ON to build the readthedocs website source")
if (BUILD_WEBSITE AND NOT IMATH_IS_SUBPROJECT)
  add_subdirectory(website)
endif()

# If you want to use ctest to configure, build and
# upload the results, cmake has builtin support for
# submitting to CDash, or any server who speaks the
# same protocol
# 
# These settings will need to be set for your environment,
# and then a script such as the example in
#
# cmake/SampleCTestScript.cmake
#
# edited and placed into the CI system, then run:
#
# cmake -S cmake/SampleCTestScript.cmake
#
# [or whatever you name the file you edit]
# 
#set(CTEST_PROJECT_NAME "Imath")
#set(CTEST_NIGHTLY_START_TIME "01:01:01 UTC")
#set(CTEST_DROP_METHOD "http") # there are others...
#set(CTEST_DROP_SITE "open.cdash.org")
#set(CTEST_DROP_LOCATION "/submit.php?project=MyProject")
#set(CTEST_DROP_SITE_CDASH TRUE)
include(CTest)
if(BUILD_TESTING AND NOT IMATH_IS_SUBPROJECT)
  enable_testing()
  add_subdirectory(src/ImathTest)
endif()

# Including this module will add a `clang-format` target to the build
# if the clang-format executable can be found. Only do this if we are
# top level.
if(NOT IMATH_IS_SUBPROJECT)
  include(cmake/clang-format.cmake)
endif()

