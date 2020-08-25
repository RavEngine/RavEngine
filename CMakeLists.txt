cmake_minimum_required(VERSION 3.17)
include(ProcessorCount)
project(RavEngine)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(OSX_ARCHITECTURES "${ARCHS_STANDARD}" CACHE INTERNAL "")

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

function (add_subdirectory_with_folder folder_name)
  function (add_library name type)
    _add_library (${ARGV})
    # Do not wrap creation of INTERFACE libraries
    if (type STREQUAL INTERFACE)
      return()
    endif()

    set_target_properties(${name}
      PROPERTIES
      FOLDER "${folder_name}"
    )
  endfunction()

  function (add_executable name)
    _add_executable (${ARGV})
    set_target_properties(${name}
      PROPERTIES
      FOLDER "${folder_name}"
    )
  endfunction()

  add_subdirectory (${ARGN})
endfunction()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable multiprocessor compilation with vs
if(MSVC)
  add_definitions(/MP)
  add_definitions(/Qpar)
endif()

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${DEPS_DIR}")

# ========== CMake Boilerplate ==============

# ban in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
#set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

# linux detection
if(UNIX AND NOT APPLE)
	set(LINUX TRUE CACHE INTERNAL "")
endif()

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# ==================== Dependencies =====================

# use static SDL
#set(SDL_STATIC ON CACHE INTERNAL "")
#set(SDL_SHARED OFF CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory("${DEPS_DIR}/SDL2")

if(APPLE)
  set(LLGL_BUILD_RENDERER_METAL ON CACHE INTERNAL "")
endif()
add_subdirectory("${DEPS_DIR}/LLGL")

# PhysX-specific CMake project setup
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "Use the debug version of the CRT")
set(PHYSX_ROOT_DIR ${DEPS_DIR}/physx/physx CACHE INTERNAL "")
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared CACHE INTERNAL "")
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")
set(PX_PHYSX_ ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")
set(CMAKEMODULES_VERSION "1.27" CACHE INTERNAL "")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules CACHE INTERNAL "")
set(PX_OUTPUT_LIB_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} CACHE INTERNAL "")
set(PX_OUTPUT_BIN_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} CACHE INTERNAL "")
set(PX_GENERATE_STATIC_LIBRARIES ON CACHE INTERNAL "")
#set(PX_FLOAT_POINT_PRECISE_MATH OFF)
if (WIN32)
	set(TARGET_BUILD_PLATFORM "windows" CACHE INTERNAL "")
elseif(APPLE)
	set(TARGET_BUILD_PLATFORM "mac" CACHE INTERNAL "")
elseif(LINUX)
	set(TARGET_BUILD_PLATFORM "linux" CACHE INTERNAL "")
	set(CMAKE_LIBRARY_ARCHITECTURE "x86_64-linux-gnu" CACHE INTERNAL "")
	#set(CMAKE_LIBRARY_ARCHITECTURE "aarch64-linux-gnu" CACHE INTERNAL "")
endif()

# Call into PhysX's CMake scripts
add_subdirectory("${PHYSX_ROOT_DIR}/compiler/public")

# ========== Building engine and test game ==============

# get header search paths
include_directories("include/${PROJECT_NAME}/")
include_directories("include/${PROJECT_NAME}/stduuid/")
include_directories("${DEPS_DIR}/physx/physx/include/")
include_directories("${DEPS_DIR}/physx/pxshared/include/")
include_directories("${DEPS_DIR}/physx/physx/snippets/")
include_directories("${DEPS_DIR}/SDL2/include")
include_directories("${DEPS_DIR}/LLGL/include")


# get all sources for the library with glob
file(GLOB SOURCES "src/*.cpp")
if(APPLE)
	# also need to compile Objective-C++ files
	file(GLOB MM_SOURCES "src/*.mm")
	add_definitions("-x objective-c++")
	set(CMAKE_EXE_LINKER_FLAGS "-framework ServiceManagement -framework Foundation -framework Cocoa -framework Metal -framework QuartzCore")
endif()
file(GLOB CHEADERS "include/${PROJECT_NAME}/*.h")
file(GLOB HEADERS "include/${PROJECT_NAME}/*.hpp")

# register the library
add_library("${PROJECT_NAME}" ${SOURCES} ${MM_SOURCES} ${CHEADERS} ${HEADERS})

# create the test executable
file(GLOB TEST_SOURCES "${PROJECT_NAME}_Test/*.cpp")
file(GLOB TEST_HEADERS "${PROJECT_NAME}_Test/*.hpp")
file(GLOB TEST_CHEADERS "${PROJECT_NAME}_Test/*.h")

# creates a mac app if applicable
add_executable("${PROJECT_NAME}_Test" MACOSX_BUNDLE ${TEST_SOURCES} ${TEST_HEADERS} ${TEST_CHEADERS})
set_target_properties("${PROJECT_NAME}_Test" PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}_Test/Info.plist")

#startup project	
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}_Test")

get_directory_property(targets BUILDSYSTEM_TARGETS)
message(STATUS "Targets: ${targets}")
set_target_properties(${BUILDSYSTEM_TARGETS} PROPERTIES FOLDER "Libraries")

# ====================== Linking ====================
target_link_libraries("${PROJECT_NAME}" "SDL2")
target_link_libraries("${PROJECT_NAME}" "PhysXExtensions")
target_link_libraries("${PROJECT_NAME}" "PhysX")
target_link_libraries("${PROJECT_NAME}" "PhysXPvdSDK")
target_link_libraries("${PROJECT_NAME}" "PhysXVehicle")
target_link_libraries("${PROJECT_NAME}" "PhysXCharacterKinematic")
target_link_libraries("${PROJECT_NAME}" "PhysXCooking")
target_link_libraries("${PROJECT_NAME}" "PhysXCommon")
#target_link_libraries("${PROJECT_NAME}" "PhysXGPU")
target_link_libraries("${PROJECT_NAME}" "PhysXFoundation")
target_link_libraries("${PROJECT_NAME}" "PhysXTask")
#target_link_libraries("${PROJECT_NAME}" "SnippetUtils")
target_link_libraries("${PROJECT_NAME}" "FastXml")
target_link_libraries("${PROJECT_NAME}" "LowLevel")
target_link_libraries("${PROJECT_NAME}" "LowLevelAABB")
target_link_libraries("${PROJECT_NAME}" "LowLevelDynamics")
target_link_libraries("${PROJECT_NAME}" "SceneQuery")
target_link_libraries("${PROJECT_NAME}" "SimulationController")
if(MSVC)
  target_link_libraries("${PROJECT_NAME}" "LLGL_Direct3D11")
elseif(APPLE)
  target_link_libraries("${PROJECT_NAME}" "LLGL_Metal")
endif()
target_link_libraries("${PROJECT_NAME}" "LLGL_OpenGL")

# copy DLLs
if (WIN32)
	# PhysX
	if(NOT PX_GENERATE_STATIC_LIBRARIES)
		add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${CMAKE_BINARY_DIR}/deps/bin/win.x86_64.vc142.md/$<CONFIGURATION>"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>")
	endif()

endif()
# link test game with library
target_link_libraries("${PROJECT_NAME}_Test" "${PROJECT_NAME}" )

INSTALL(CODE "
	include(BundleUtilities)
	fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/$<CONFIGURATION>/${PROJECT_NAME}_Test.app\" \"\" \"\")
	" COMPONENT Runtime)
