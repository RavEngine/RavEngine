# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
cmake_minimum_required(VERSION 3.10.2)
project(DirectX-Headers
    LANGUAGES CXX
    VERSION 1.613.1
)
include(CTest)
set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()

# It's useful to know if you are a top level project or not, if your project is
# being consumed via add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()

# Use DXHEADERS_* prefix to avoid potential name conflicts in cmake-gui, and allow
# grouping by prefix if more options are added
#
# Testing should only be enabled by default if we are top level. Otherwise clients can set it
# either via cmake or cmake-gui
option(DXHEADERS_BUILD_TEST "Build the test" ${IS_TOPLEVEL_PROJECT})
option(DXHEADERS_INSTALL "Installation logic" ${IS_TOPLEVEL_PROJECT})
option(DXHEADERS_BUILD_GOOGLE_TEST "Build the google test suite" ${IS_TOPLEVEL_PROJECT})

include(GNUInstallDirs)

# Enables consumers to add this library as a link target to automatically add
# these include directories, regardless of whether this is referenced via subdirectory
# or from an installed location
add_library(DirectX-Headers STATIC src/d3dx12_property_format_table.cpp)
target_include_directories(DirectX-Headers SYSTEM PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_include_directories(DirectX-Headers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/directx)

add_library(Microsoft::DirectX-Headers ALIAS DirectX-Headers)

add_library(DirectX-Guids STATIC src/dxguids.cpp)
target_link_libraries(DirectX-Guids PRIVATE DirectX-Headers)

add_library(Microsoft::DirectX-Guids ALIAS DirectX-Guids)

# For non-Windows targets, also add the WSL stubs to the include path
if (NOT WIN32)
    target_include_directories(DirectX-Headers SYSTEM PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/wsl/stubs>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/wsl/stubs>"
    )
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    # MinGW has RPC headers which define old versions, and complain if D3D
    # headers are included before the RPC headers, since D3D headers were
    # generated with new MIDL and "require" new RPC headers.
   target_compile_options(DirectX-Headers PRIVATE "-D__REQUIRED_RPCNDR_H_VERSION__=475")
   target_compile_options(DirectX-Guids PRIVATE "-D__REQUIRED_RPCNDR_H_VERSION__=475")
endif()

if (DXHEADERS_INSTALL)
    # Install the targets
    install(TARGETS DirectX-Headers DirectX-Guids
            EXPORT DirectX-Headers-Targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    # Create the targets CMake file which contains the above definitions
    install(EXPORT DirectX-Headers-Targets FILE directx-headers-targets.cmake
            NAMESPACE Microsoft::
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/directx-headers/cmake)

    # Install the actual includes
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    # Create the CMake config files
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file("directx-headers-config-version.cmake"
                                    VERSION ${PROJECT_VERSION}
                                    COMPATIBILITY SameMajorVersion)
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/directx-headers-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/directx-headers-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/directx-headers/cmake)

    # Install the CMake config files
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/directx-headers-config.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/directx-headers-config-version.cmake"
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/directx-headers/cmake)
    
    # Create pkg-config file 
    include(cmake/JoinPaths.cmake)
    # from: https://github.com/jtojnar/cmake-snips#concatenating-paths-when-building-pkg-config-files
    join_paths(DIRECTX_INCLUDEDIR_FOR_PKG_CONFIG "\${prefix}" "${CMAKE_INSTALL_INCLUDEDIR}")
    join_paths(DIRECTX_LIBDIR_FOR_PKG_CONFIG "\${prefix}"     "${CMAKE_INSTALL_LIBDIR}")        
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DirectX-Headers.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/DirectX-Headers.pc" @ONLY)

    # Install the pkg-config file
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DirectX-Headers.pc"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

endif()

if (BUILD_TESTING)
    if (DXHEADERS_BUILD_TEST)
       add_subdirectory(test) 
    endif()

    if (DXHEADERS_BUILD_GOOGLE_TEST)
       # We do not want to install GoogleTest when packaging DirectX-Headers.
       set(INSTALL_GTEST OFF)
       add_subdirectory(googletest)
    endif()
endif()
