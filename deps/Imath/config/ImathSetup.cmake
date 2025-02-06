# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

include(GNUInstallDirs)

# Target configuration
if(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
  set(IMATH_IS_SUBPROJECT ON)
  message(STATUS "Imath is configuring as a cmake sub project")
endif()

option(IMATH_HALF_USE_LOOKUP_TABLE "Convert half-to-float using a lookup table (on by default)" ON)

option(IMATH_USE_DEFAULT_VISIBILITY "Makes the compile use default visibility (by default compiles tidy, hidden-by-default)"     OFF)

# This is primarily for the halfFunction code that enables a stack
# object (if you enable this) that contains a LUT of the function
option(IMATH_ENABLE_LARGE_STACK "Enables code to take advantage of large stack support"     OFF)

# Option to make it possible to build without the noexcept specifier
option(IMATH_USE_NOEXCEPT "Compile with noexcept specifier" ON)

# What C++ standard to compile for.  VFX Platform 18 is c++14, so
# that's the default.
set(tmp 14)
if(CMAKE_CXX_STANDARD)
  set(tmp ${CMAKE_CXX_STANDARD})
endif()
set(IMATH_CXX_STANDARD "${tmp}" CACHE STRING "C++ standard to compile against")
set(tmp)

# Namespace-related settings: allows one to customize the namespace
# generated, and to version the namespaces.
set(IMATH_NAMESPACE_CUSTOM "0" CACHE STRING "Whether the namespace has been customized (so external users know)")
set(IMATH_INTERNAL_NAMESPACE "Imath_${IMATH_VERSION_API}" CACHE STRING "Real namespace for Imath that will end up in compiled symbols")
set(IMATH_NAMESPACE "Imath" CACHE STRING "Public namespace alias for Imath")
set(IMATH_PACKAGE_NAME "Imath ${IMATH_VERSION}${IMATH_VERSION_RELEASE_TYPE}" CACHE STRING "Public string / label for displaying package")

# Whether to generate and install a pkg-config file Imath.pc on
if(WIN32)
  option(IMATH_INSTALL_SYM_LINK "Create symbolic links for shared objects" OFF)
else()
  option(IMATH_INSTALL_SYM_LINK "Create symbolic links for shared objects" ON)
endif()
option(IMATH_INSTALL_PKG_CONFIG "Install Imath.pc file" ON)

#
# Build related options
#

# This variable is for use in install lines. Care must be taken when
# changing this, as many things assume this is "Imath".
set(IMATH_OUTPUT_SUBDIR Imath CACHE STRING "Destination sub-folder of the include path for install")

# This does not seem to be available as a per-target property, but is
# pretty harmless to set globally.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Suffix for debug configuration libraries (if you should choose to
# install those)
set(CMAKE_DEBUG_POSTFIX "_d" CACHE STRING "Suffix for debug builds")

# Usual cmake option to build shared libraries or not
option(BUILD_SHARED_LIBS "Build shared library" ON)

# Suffix to append to root name, this helps with version management
# but can be turned off if you don't care, or otherwise customized
set(IMATH_LIB_SUFFIX "-${IMATH_VERSION_API}" CACHE STRING "string added to the end of all the libraries")

# When building static, the additional string to add to the library name such
# that a static build of Imath is easily distinguishable.
# To use the static library, you would use
# -lImath_static (or target_link_libraries(xxx Imath::Imath_static))
set(IMATH_STATIC_LIB_SUFFIX "_static" CACHE STRING "name to append to static library (in addition to normal suffix)")

# rpath related setup. Make sure we force an rpath to the rpath we're compiling
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# Add the automatically determined parts of the rpath which point to
# directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# If the user sets an install rpath then just use that, or otherwise
# set one for them.
if(NOT CMAKE_INSTALL_RPATH)
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    if("${CMAKE_SYSTEM}" MATCHES "Linux")
      get_filename_component(tmpSysPath "${CMAKE_INSTALL_FULL_LIBDIR}" NAME)
      if(NOT tmpSysPath)
        set(tmpSysPath "lib")
      endif()
      set(CMAKE_INSTALL_RPATH "\\\$ORIGIN/../${tmpSysPath};${CMAKE_INSTALL_FULL_LIBDIR}")
      set(tmpSysPath)
	elseif(APPLE)
      set(CMAKE_INSTALL_RPATH "@loader_path/../lib;@executable_path/../lib;${CMAKE_INSTALL_FULL_LIBDIR}")
    else()
      set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    endif()
  endif()
  set(isSystemDir)
endif()

# Set a default build type if not set
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Code check related features
option(IMATH_USE_CLANG_TIDY "Check if clang-tidy is available, and enable that" OFF)
if(IMATH_USE_CLANG_TIDY)
  find_program(IMATH_CLANG_TIDY_BIN clang-tidy)
  if(IMATH_CLANG_TIDY_BIN-NOTFOUND)
    message(FATAL_ERROR "clang-tidy processing requested, but no clang-tidy found")
  endif()
  # TODO: Need to define the list of valid checks and add a file with said list
  set(CMAKE_CXX_CLANG_TIDY
    ${IMATH_CLANG_TIDY_BIN};
    -header-filter=.;
    -checks=*;
  )
endif()
