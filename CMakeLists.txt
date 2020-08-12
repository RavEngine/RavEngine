cmake_minimum_required(VERSION 3.17)
include(ProcessorCount)
project(RavEngine)
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


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable multiprocessor compilation with vs
if(MSVC)
  add_definitions(/MP)
  add_definitions(/Qpar)
  #add_definitions(/Ob1)		# required for SDL
endif()

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")

# ========== CMake Boilerplate ==============

# ban in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
#set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
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

#add_subdirectory("${DEPS_DIR}/SDL2")
if(MSVC)
  add_definitions(/std:c++latest)
endif()

set(FILAMENT_ENABLE_LTO OFF CACHE INTERNAL "")
set(FILAMENT_ENABLE_JAVA OFF CACHE INTERNAL "")
set(FILAMENT_SKIP_SAMPLES ON CACHE INTERNAL "")
IF(APPLE)
	set(FILAMENT_SUPPORTS_METAL ON CACHE INTERNAL "")
	set(FILAMENT_SUPPORTS_VULKAN OFF CACHE INTERNAL "")
ELSE()
	set(FILAMENT_SUPPORTS_METAL OFF CACHE INTERNAL "")
	set(FILAMENT_SUPPORTS_VULKAN ON CACHE INTERNAL "")
ENDIF()
add_subdirectory("${DEPS_DIR}/filament")

if(MSVC)
	remove_definitions(/std:c++latest)
endif()

# PhysX-specific CMake project setup
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "Use the debug version of the CRT")
set(PHYSX_ROOT_DIR ${DEPS_DIR}/physx/physx CACHE INTERNAL "")
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared CACHE INTERNAL "")
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")
set(PX_PHYSX_ ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")
set(CMAKEMODULES_VERSION "1.27" CACHE INTERNAL "")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules CACHE INTERNAL "")
set(PX_OUTPUT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(PX_OUTPUT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
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
add_subdirectory(${PHYSX_ROOT_DIR}/compiler/public)


# ========== Building engine and test game ==============

# get header search paths
include_directories("include/${PROJECT_NAME}/")
include_directories("include/${PROJECT_NAME}/stduuid/")
include_directories("${DEPS_DIR}/physx/physx/include/")
include_directories("${DEPS_DIR}/physx/pxshared/include/")
include_directories("${DEPS_DIR}/physx/physx/snippets/")
include_directories("${DEPS_DIR}/filament/filament/include")

# add filament header paths
SUBDIRLIST(filament_libs "${DEPS_DIR}/filament/libs")
FOREACH(subdir ${filament_libs})
	include_directories("${DEPS_DIR}/filament/libs/${subdir}/include")
ENDFOREACH()

SUBDIRLIST(filament_deps "${DEPS_DIR}/filament/third_party")
FOREACH(subdir ${filament_deps})
	IF(EXISTS "${DEPS_DIR}/filament/third_party/${subdir}/include")
		include_directories("${DEPS_DIR}/filament/third_party/${subdir}/include")
	endif()
ENDFOREACH()


set(METHANEKIT_MODULES "${DEPS_DIR}/MethaneKit/Modules/")
set(METHANEKIT_EXTERNALS "${DEPS_DIR}/MethaneKit/Externals/")

# get all sources for the library with glob
file(GLOB SOURCES "src/*.cpp")
file(GLOB CHEADERS "include/${PROJECT_NAME}/*.h")
file(GLOB HEADERS "include/${PROJECT_NAME}/*.hpp")

# register the library
add_library("${PROJECT_NAME}" ${SOURCES} ${CHEADERS} ${HEADERS})

# create the test executable
file(GLOB TEST_SOURCES "${PROJECT_NAME}_Test/*.cpp")
file(GLOB TEST_HEADERS "${PROJECT_NAME}_Test/*.hpp")
file(GLOB TEST_CHEADERS "${PROJECT_NAME}_Test/*.h")
add_executable("${PROJECT_NAME}_Test" ${TEST_SOURCES} ${TEST_HEADERS} ${TEST_CHEADERS})

# ====================== Linking ====================
#target_link_libraries("${PROJECT_NAME}" "SDL2")
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

target_link_libraries("${PROJECT_NAME}" "filament")
target_link_libraries("${PROJECT_NAME}" "sdl2")
target_link_libraries("${PROJECT_NAME}" "assimp")
target_link_libraries("${PROJECT_NAME}" "filagui")
target_link_libraries("${PROJECT_NAME}" "imgui")
target_link_libraries("${PROJECT_NAME}" "camutils")

# copy DLLs
if (WIN32)
	if(NOT PX_GENERATE_STATIC_LIBRARIES)
		add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${PROJECT_BINARY_DIR}/deps/bin/win.x86_64.vc142.md/$<CONFIGURATION>"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>")
	endif()

	add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_BINARY_DIR}/deps/filament/samples/$<CONFIGURATION>"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>")

	#startup project	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}_Test")
endif()
# link test game with library
target_link_libraries("${PROJECT_NAME}_Test" "${PROJECT_NAME}" )

