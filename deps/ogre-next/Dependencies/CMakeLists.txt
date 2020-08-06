cmake_minimum_required(VERSION 2.8.10)

project(OGREDEPS)
set(CMAKE_MODULE_PATH "${OGREDEPS_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

if (WIN32)
  add_definitions(-DWINVER=0x0500)
endif ()

if (APPLE AND NOT ANDROID)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
endif()

if (APPLE AND OGRE_BUILD_PLATFORM_APPLE_IOS)
  # Set build variables
  set(XCODE_ATTRIBUTE_SDKROOT iphoneos)
  set(CMAKE_OSX_SYSROOT iphoneos)
  if(XCODE_VERSION VERSION_LESS 5.0)
    set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD_32_BIT)")
  else()
    set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
  endif()
  add_definitions(-DHAVE_UNISTD_H=1)
elseif (APPLE AND NOT OGRE_BUILD_PLATFORM_APPLE_IOS)
  # 10.7 is the minimum for libc++ support
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)

  if (NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD_32_64_BIT)")
  endif()
endif ()

if (ANDROID)
  SET(TargetPlatform "Android")
  SET(OGRE_PLATFORM OGRE_PLATFORM_ANDROID)
endif()

if (EMSCRIPTEN)
  set(TargetPlatform "Emscripten")
  set(OGRE_PLATFORM OGRE_PLATFORM_EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-warn-absolute-paths -Werror=implicit-function-declaration")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-warn-absolute-paths -Werror=implicit-function-declaration")
endif()

add_subdirectory(src)
