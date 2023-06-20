# ~~~
# Copyright (c) 2014-2023 Valve Corporation
# Copyright (c) 2014-2023 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~
cmake_minimum_required(VERSION 3.17.2)

project(VULKAN_LOADER VERSION 1.3.254)

add_subdirectory(scripts)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# By default, loader & tests are built without sanitizers
# Use these options to force a specific sanitizer on the loader and test executables
if (UNIX)
    option(LOADER_ENABLE_ADDRESS_SANITIZER "Linux & macOS only: Advanced memory checking" OFF)
    option(LOADER_ENABLE_THREAD_SANITIZER "Linux & macOS only: Advanced thread checking" OFF)
endif()

if(APPLE)
    option(BUILD_STATIC_LOADER "Build a loader that can be statically linked" OFF)
endif()

if(WIN32)
    # Optional: Allow specify the exact version used in the loader dll
    # Format is major.minor.patch.build
    set(BUILD_DLL_VERSIONINFO "" CACHE STRING "Set the version to be used in the loader.rc file. Default value is the currently generated header version")
endif()

if(BUILD_STATIC_LOADER)
    message(WARNING "The BUILD_STATIC_LOADER option has been set. Note that this will only work on MacOS and is not supported "
        "or tested as part of the loader. Use it at your own risk.")
endif()

if (NOT TARGET Vulkan::Headers)
    find_package(VulkanHeaders REQUIRED CONFIG QUIET)
endif()

include(GNUInstallDirs)

set(GIT_BRANCH_NAME "--unknown--")
set(GIT_TAG_INFO "--unknown--")
find_package (Git)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git/HEAD")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_TAG_INFO)
    string(REGEX REPLACE "\n$" "" GIT_TAG_INFO "${GIT_TAG_INFO}")

    file(READ "${CMAKE_CURRENT_LIST_DIR}/.git/HEAD" GIT_HEAD_REF_INFO)
    if (GIT_HEAD_REF_INFO)
        string(REGEX MATCH "ref: refs/heads/(.*)" _ ${GIT_HEAD_REF_INFO})
        if (CMAKE_MATCH_1)
            set(GIT_BRANCH_NAME ${CMAKE_MATCH_1})
        else()
            set(GIT_BRANCH_NAME ${GIT_HEAD_REF_INFO})
        endif()
        string(REGEX REPLACE "\n$" "" GIT_BRANCH_NAME "${GIT_BRANCH_NAME}")
    endif()
endif()

# Enable IDE GUI folders.  "Helper targets" that don't have interesting source code should set their FOLDER property to this
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(LOADER_HELPER_FOLDER "Helper Targets")

if(UNIX)
    set(FALLBACK_CONFIG_DIRS "/etc/xdg" CACHE STRING
            "Search path to use when XDG_CONFIG_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
    set(FALLBACK_DATA_DIRS "/usr/local/share:/usr/share" CACHE STRING
            "Search path to use when XDG_DATA_DIRS is unset or empty or the current process is SUID/SGID. Default is freedesktop compliant.")
    set(SYSCONFDIR "" CACHE STRING
            "System-wide search directory. If not set or empty, CMAKE_INSTALL_FULL_SYSCONFDIR and /etc are used.")
endif()

# For MSVC/Windows, replace /GR with an empty string, this prevents warnings of /GR being overriden by /GR-
# Newer CMake versions (3.20) have better solutions for this through policy - using the old
# way while waiting for when updating can occur
string(REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if(WIN32)
    option(ENABLE_WIN10_ONECORE "Link the loader with OneCore umbrella libraries" OFF)
endif()

add_library(platform_wsi INTERFACE)
if(WIN32)
    target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_MACOS_MVK VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX AND NOT APPLE) # i.e.: Linux
    option(BUILD_WSI_XCB_SUPPORT "Build XCB WSI support" ON)
    option(BUILD_WSI_XLIB_SUPPORT "Build Xlib WSI support" ON)
    option(BUILD_WSI_WAYLAND_SUPPORT "Build Wayland WSI support" ON)
    option(BUILD_WSI_DIRECTFB_SUPPORT "Build DirectFB WSI support" OFF)
    option(BUILD_WSI_SCREEN_QNX_SUPPORT "Build QNX Screen WSI support" OFF)

    find_package(PkgConfig REQUIRED QUIET) # Use PkgConfig to find Linux system libraries

    if(BUILD_WSI_XCB_SUPPORT)
        pkg_check_modules(XCB REQUIRED QUIET IMPORTED_TARGET xcb)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_XCB_KHR)
        target_link_libraries(platform_wsi INTERFACE PkgConfig::XCB)
    endif()
    if(BUILD_WSI_XLIB_SUPPORT)
        pkg_check_modules(X11 REQUIRED QUIET IMPORTED_TARGET x11)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_XLIB_XRANDR_EXT)
        target_link_libraries(platform_wsi INTERFACE PkgConfig::X11)
    endif()
    if(BUILD_WSI_WAYLAND_SUPPORT)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
    if(BUILD_WSI_DIRECTFB_SUPPORT)
        pkg_check_modules(DirectFB QUIET REQUIRED IMPORTED_TARGET directfb)
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_DIRECTFB_EXT)
        target_link_libraries(platform_wsi INTERFACE PkgConfig::DirectFB)
    endif()
    if(BUILD_WSI_SCREEN_QNX_SUPPORT)
        # Part of OS, no additional include directories are required
        target_compile_definitions(platform_wsi INTERFACE VK_USE_PLATFORM_SCREEN_QNX)
    endif()
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

add_library(loader_common_options INTERFACE)
target_compile_definitions(loader_common_options INTERFACE API_NAME="Vulkan")
target_link_libraries(loader_common_options INTERFACE platform_wsi)

# Enable beta Vulkan extensions
target_compile_definitions(loader_common_options INTERFACE VK_ENABLE_BETA_EXTENSIONS)

target_compile_features(loader_common_options INTERFACE c_std_99)
target_compile_features(loader_common_options INTERFACE cxx_std_11)
set(LOADER_STANDARD_C_PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED YES C_EXTENSIONS OFF)
set(LOADER_STANDARD_CXX_PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS OFF)

set(TESTS_STANDARD_CXX_PROPERTIES ${LOADER_STANDARD_CXX_PROPERTIES} MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Force the use of the multithreaded, static version of the C runtime.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(ENABLE_WERROR "Enable warnings as errors" ON)

# Set warnings as errors and the main diagnostic flags
# Must be set first so the warning silencing later on works properly
# Note that clang-cl.exe should use MSVC flavor flags, not GNU
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" MATCHES "MSVC"))
    if (ENABLE_WERROR)
        target_compile_options(loader_common_options INTERFACE /WX)
    endif()
    target_compile_options(loader_common_options INTERFACE /W4)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using GCC or Clang with the regular front end
    if (ENABLE_WERROR)
        target_compile_options(loader_common_options INTERFACE -Werror)
    endif()
    target_compile_options(loader_common_options INTERFACE -Wall -Wextra)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(loader_common_options INTERFACE -Wno-missing-field-initializers)

    # need to prepend /clang: to compiler arguments when using clang-cl
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" MATCHES "MSVC")
        target_compile_options(loader_common_options INTERFACE /clang:-fno-strict-aliasing /clang:-fno-builtin-memcmp)
    else()
        target_compile_options(loader_common_options INTERFACE -fno-strict-aliasing -fno-builtin-memcmp)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(loader_common_options INTERFACE -Wno-stringop-truncation -Wno-stringop-overflow)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.1)
            target_compile_options(loader_common_options INTERFACE -Wshadow=local) #only added in GCC 7
        endif()
    endif()

    if(UNIX)
        target_compile_options(loader_common_options INTERFACE -fvisibility=hidden)
    endif()

    target_compile_options(loader_common_options INTERFACE -Wpointer-arith)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" MATCHES "MSVC"))
    # /GR-: Disable RTTI
    # /guard:cf: Enable control flow guard
    # /wd4152: Disable warning on conversion of a function pointer to a data pointer
    # /wd4201: Disable warning on anonymous struct/unions
    target_compile_options(loader_common_options INTERFACE /GR- /guard:cf /wd4152 /wd4201)

    # Enable control flow guard
    target_link_options(loader_common_options INTERFACE "LINKER:/guard:cf")

    # Prevent <windows.h> from polluting the code. guards against things like MIN and MAX
    target_compile_definitions(loader_common_options INTERFACE WIN32_LEAN_AND_MEAN)
endif()

# DEBUG enables runtime loader ICD verification
# Add git branch and tag info in debug mode
target_compile_definitions(loader_common_options INTERFACE $<$<CONFIG:DEBUG>:DEBUG;GIT_BRANCH_NAME="${GIT_BRANCH_NAME}";GIT_TAG_INFO="${GIT_TAG_INFO}">)

# Check for the existance of the secure_getenv or __secure_getenv commands
include(CheckFunctionExists)
include(CheckIncludeFile)

check_function_exists(secure_getenv HAVE_SECURE_GETENV)
check_function_exists(__secure_getenv HAVE___SECURE_GETENV)

if (HAVE_SECURE_GETENV)
    target_compile_definitions(loader_common_options INTERFACE HAVE_SECURE_GETENV)
endif()
if (HAVE___SECURE_GETENV)
    target_compile_definitions(loader_common_options INTERFACE HAVE___SECURE_GETENV)
endif()
if(NOT MSVC AND NOT (HAVE_SECURE_GETENV OR HAVE___SECURE_GETENV))
    message(WARNING "Using non-secure environmental lookups. This loader will not properly disable environent variables when run with elevated permissions.")
endif()

option(LOADER_CODEGEN "Enable vulkan loader code generation")
if(LOADER_CODEGEN)
    find_package(Python3 REQUIRED)
    add_custom_target(loader_codegen
        COMMAND Python3::Interpreter ${PROJECT_SOURCE_DIR}/scripts/generate_source.py ${VULKAN_HEADERS_REGISTRY_DIRECTORY}
                --generated-version ${VulkanHeaders_VERSION} --incremental
    )
endif()

if(UNIX)
    target_compile_definitions(loader_common_options INTERFACE FALLBACK_CONFIG_DIRS="${FALLBACK_CONFIG_DIRS}" FALLBACK_DATA_DIRS="${FALLBACK_DATA_DIRS}")

    if(NOT (SYSCONFDIR STREQUAL ""))
        # SYSCONFDIR is specified, use it and do not force /etc.
        target_compile_definitions(loader_common_options INTERFACE SYSCONFDIR="${SYSCONFDIR}")
    else()
        target_compile_definitions(loader_common_options INTERFACE SYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}")

        # Make sure /etc is searched by the loader
        if(NOT (CMAKE_INSTALL_FULL_SYSCONFDIR STREQUAL "/etc"))
            target_compile_definitions(loader_common_options INTERFACE EXTRASYSCONFDIR="/etc")
        endif()
    endif()
endif()

add_subdirectory(loader)

option(BUILD_TESTS "Build Tests")
if(BUILD_TESTS)
    # Set gtest build configuration
    # Attempt to enable if it is available.
    if(TARGET gtest)
        # Already enabled as a target (perhaps by a project enclosing this one)
        message(STATUS "Vulkan-Loader/external: " "googletest already configured - using it")
    elseif(IS_DIRECTORY "${GOOGLETEST_INSTALL_DIR}/googletest")
        set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject")
        set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject")
        set(gtest_force_shared_crt ON CACHE BOOL "Link gtest runtimes dynamically" FORCE)
        set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
        set(INSTALL_GTEST OFF CACHE BOOL "Don't install gtest")
        # The googletest directory exists, so enable it as a target.
        message(STATUS "Vulkan-Loader/external: " "googletest found - configuring it for tests")
        add_subdirectory("${GOOGLETEST_INSTALL_DIR}")
    else()
        message(SEND_ERROR "Could not find googletest directory. Be sure to run update_deps.py with the --tests option to download the appropriate version of googletest")
        set(BUILD_TESTS OFF)
    endif()

    # make sure gtest uses the dynamic runtime instead
    set_target_properties(gtest PROPERTIES ${TESTS_STANDARD_CXX_PROPERTIES})
    set_target_properties(gtest_main PROPERTIES ${TESTS_STANDARD_CXX_PROPERTIES})

    if (WIN32)
        if(TARGET detours)
            # Already enabled as a target (perhaps by a project enclosing this one)
            message(STATUS "Vulkan-Loader/external: " "detours already configured - using it")
        else()
            if(IS_DIRECTORY ${DETOURS_INSTALL_DIR})
                # The detours directory exists, so enable it as a target.
                message(STATUS "Vulkan-Loader/external: " "detours found - configuring it for tests")
            else()
                message(SEND_ERROR "Could not find detours directory. Be sure to run update_deps.py with the --tests option to download the appropriate version of detours")
                set(BUILD_TESTS OFF)
            endif()
            add_library(detours STATIC
                ${DETOURS_INSTALL_DIR}/src/creatwth.cpp
                ${DETOURS_INSTALL_DIR}/src/detours.cpp
                ${DETOURS_INSTALL_DIR}/src/detours.h
                ${DETOURS_INSTALL_DIR}/src/detver.h
                ${DETOURS_INSTALL_DIR}/src/disasm.cpp
                ${DETOURS_INSTALL_DIR}/src/disolarm.cpp
                ${DETOURS_INSTALL_DIR}/src/disolarm64.cpp
                ${DETOURS_INSTALL_DIR}/src/disolia64.cpp
                ${DETOURS_INSTALL_DIR}/src/disolx64.cpp
                ${DETOURS_INSTALL_DIR}/src/disolx86.cpp
                ${DETOURS_INSTALL_DIR}/src/image.cpp
                ${DETOURS_INSTALL_DIR}/src/modules.cpp
                )
            target_include_directories(detours PUBLIC ${DETOURS_INSTALL_DIR}/src)

            macro(GET_WIN32_WINNT version)
                if(WIN32 AND CMAKE_SYSTEM_VERSION)
            		set(ver ${CMAKE_SYSTEM_VERSION})
            		string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            		string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            		# Check for Windows 10, b/c we'll need to convert to hex 'A'.
            		if("${verMajor}" MATCHES "10")
            			set(verMajor "A")
            			string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            		endif("${verMajor}" MATCHES "10")
            		# Remove all remaining '.' characters.
            		string(REPLACE "." "" ver ${ver})
            		# Prepend each digit with a zero.
            		string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            		set(${version} "0x${ver}")
                endif()
            endmacro()

            set(DETOURS_MAJOR_VERSION "4")
            set(DETOURS_MINOR_VERSION "0")
            set(DETOURS_PATCH_VERSION "1")
            set(DETOURS_VERSION "${DETOURS_MAJOR_VERSION}.${DETOURS_MINOR_VERSION}.${DETOURS_PATCH_VERSION}")

            target_include_directories(detours PUBLIC ${DETOURS_INSTALL_DIR}/src)

            if(MSVC_VERSION GREATER_EQUAL 1700)
                target_compile_definitions(detours PUBLIC DETOURS_CL_17_OR_NEWER)
            endif(MSVC_VERSION GREATER_EQUAL 1700)
            GET_WIN32_WINNT(ver)
            if(ver EQUAL 0x0700)
                target_compile_definitions(detours PUBLIC _USING_V110_SDK71_ DETOURS_WIN_7)
            endif(ver EQUAL 0x0700)
            target_compile_definitions(detours PUBLIC "_WIN32_WINNT=${ver}")

            if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
                target_compile_definitions(detours PUBLIC "DETOURS_TARGET_PROCESSOR=X64" DETOURS_X64 DETOURS_64BIT _AMD64_)
            else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
                target_compile_definitions(detours PUBLIC "DETOURS_TARGET_PROCESSOR=X86" DETOURS_X86 _X86_)
            endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

            target_compile_definitions(detours PUBLIC  "DETOURS_VERSION=0x4c0c1" WIN32_LEAN_AND_MEAN)

            if(MSVC)
                target_compile_definitions(detours PUBLIC  "_CRT_SECURE_NO_WARNINGS=1")
                set_target_properties(detours PROPERTIES COMPILE_FLAGS /EHsc ${TESTS_STANDARD_CXX_PROPERTIES})
            endif()

            # Silence errors found in clang-cl
            if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" MATCHES "MSVC")
                target_compile_options(detours PRIVATE -Wno-sizeof-pointer-memaccess -Wno-microsoft-goto -Wno-microsoft-cast)
            endif()
        endif()
    endif()

    if (BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
