cmake_minimum_required(VERSION 3.17)
include(ProcessorCount)
project(RavEngine)
set(OSX_ARCHITECTURES "${ARCHS_STANDARD}" CACHE INTERNAL "")

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

# set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
# set(BGFX_INSTALL OFF CACHE INTERNAL "")
# set(BGFX_INSTALL_EXAMPLES OFF CACHE INTERNAL "")
# add_subdirectory("${DEPS_DIR}/bgfx.cmake")

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
set(PX_GENERATE_STATIC_LIBRARIES ON)
#set(PX_FLOAT_POINT_PRECISE_MATH OFF)
if (WIN32)
	set(TARGET_BUILD_PLATFORM "windows")
elseif(APPLE)
	set(TARGET_BUILD_PLATFORM "mac")
elseif(LINUX)
	set(TARGET_BUILD_PLATFORM "linux")
	set(CMAKE_LIBRARY_ARCHITECTURE "x86_64-linux-gnu" CACHE INTERNAL "")
	#set(CMAKE_LIBRARY_ARCHITECTURE "aarch64-linux-gnu")
endif()

# Call into PhysX's CMake scripts
add_subdirectory(${PHYSX_ROOT_DIR}/compiler/public)

# build OGRE
if(WIN32)
	list(APPEND EXTRA_LIBS vcruntime user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)
	
endif()
#cmake_policy(SET CMP0021 OLD)
set(OGRE_ROOT "${DEPS_DIR}/ogre-next/")
set(OGRE_USE_BOOST OFF CACHE INTERNAL "")
set(OGRE_CONFIG_THREAD_PROVIDER OFF CACHE INTERNAL "")
set(OGRE_BUILD_COMPONENT_SCENE_FORMAT ON CACHE INTERNAL "")
set(OGRE_BUILD_SAMPLES2 OFF CACHE INTERNAL "")
set(OGRE_BUILD_TESTS OFF CACHE INTERNAL "")
set(OGRE_DEPENDENCIES_DIR "${OGRE_ROOT}/Dependencies/" CACHE INTERNAL "")
set(OGRE_STATIC ON CACHE INTERNAL "")
set(OGRE_BUILD_LIBS_AS_FRAMEWORKS OFF CACHE INTERNAL "")
ProcessorCount(OGRE_CONFIG_THREADS)
set(CMAKE_CXX_STANDARD 14)
if(APPLE)
	#set(OGRE_BUILD_RENDERSYSTEM_GL3PLUS OFF CACHE INTERNAL "")	# disable OpenGL on Mac
elseif(WIN32)
endif()
add_subdirectory("${OGRE_ROOT}")

set(CMAKE_CXX_STANDARD 17)


# ========== Building engine and test game ==============

# get header search paths
include_directories("include/RavEngine/")
include_directories("include/RavEngine/stduuid/")
include_directories("${OGRE_ROOT}/OgreMain/include/")
include_directories("${OGRE_ROOT}/include/")
include_directories("${OGRE_ROOT}/Components/Hlms/Common/include")
include_directories("${OGRE_ROOT}/Components/Hlms/Pbs/include")
include_directories("${OGRE_ROOT}/Components/Unlit/Pbs/include")
include_directories("${OGRE_ROOT}/Components/Hlms/Unlit/include")
include_directories("${OGRE_ROOT}/Components/Hlms/UnlitMobile/include")
include_directories("${OGRE_ROOT}/RenderSystems/Direct3D11/include")
include_directories("${OGRE_ROOT}/RenderSystems/Metal/include")
include_directories("${OGRE_ROOT}/RenderSystems/GLES/include")
include_directories("${OGRE_ROOT}/RenderSystems/GL3Plus/include")
include_directories("${OGRE_ROOT}/RenderSystems/NULL/include")
include_directories("${OGRE_ROOT}/build/Release/include")
include_directories("${CMAKE_BINARY_DIR}/deps/ogre-next/include")
include_directories("${OGRE_DEPENDENCIES_DIR}/src/SDL2/include/")
#include_directories("${DEPS_DIR}/bgfx.cmake/bimg/include/")
#include_directories("${DEPS_DIR}/bgfx.cmake/bx/include/")
#include_directories("${DEPS_DIR}/bgfx.cmake/bgfx/include/")
include_directories("${DEPS_DIR}/physx/physx/include/")
include_directories("${DEPS_DIR}/physx/pxshared/include/")
include_directories("${DEPS_DIR}/physx/physx/snippets/")

# get all sources for the library with glob
file(GLOB SOURCES "src/*.cpp")
file(GLOB CHEADERS "include/RavEngine/*.h")
file(GLOB HEADERS "include/RavEngine/*.hpp")

# register the library
add_library("RavEngine" ${SOURCES} ${CHEADERS} ${HEADERS})

# create the test executable
file(GLOB TEST_SOURCES "RavEngine_Test/*.cpp")
file(GLOB TEST_HEADERS "RavEngine_Test/*.hpp")
file(GLOB TEST_CHEADERS "RavEngine_Test/*.h")
add_executable("RavEngine_Test" ${TEST_SOURCES} ${TEST_HEADERS} ${TEST_CHEADERS})

# ====================== Linking ====================
#target_link_libraries("RavEngine" "SDL2")
target_link_libraries("RavEngine" "PhysXExtensions")
target_link_libraries("RavEngine" "PhysX")
target_link_libraries("RavEngine" "PhysXPvdSDK")
target_link_libraries("RavEngine" "PhysXVehicle")
target_link_libraries("RavEngine" "PhysXCharacterKinematic")
target_link_libraries("RavEngine" "PhysXCooking")
target_link_libraries("RavEngine" "PhysXCommon")
#target_link_libraries("RavEngine" "PhysXGPU")
target_link_libraries("RavEngine" "PhysXFoundation")
target_link_libraries("RavEngine" "PhysXTask")
#target_link_libraries("RavEngine" "SnippetUtils")
target_link_libraries("RavEngine" "FastXml")
target_link_libraries("RavEngine" "LowLevel")
target_link_libraries("RavEngine" "LowLevelAABB")
target_link_libraries("RavEngine" "LowLevelDynamics")
target_link_libraries("RavEngine" "SceneQuery")
target_link_libraries("RavEngine" "SimulationController")
#target_link_libraries("RavEngine" "bgfx")
#target_link_libraries("RavEngine" "bimg")
#target_link_libraries("RavEngine" "bimg_decode")
#target_link_libraries("RavEngine" "bx")
#target_link_libraries("RavEngine" "fcpp")
#target_link_libraries("RavEngine" "glslang")
#target_link_libraries("RavEngine" "glsl-optimizer")
target_link_libraries("RavEngine" "OgreHlmsUnlit")
target_link_libraries("RavEngine" "OgreMain")
target_link_libraries("RavEngine" "OgreHlmsPbs")
if(WIN32)
	target_link_libraries("RavEngine" "RenderSystem_Direct3D11")
	#target_link_libraries("RavEngine" "RenderSystem_Direct3D9")
elseif(APPLE)
	target_link_libraries("RavEngine" "RenderSystem_Metal")
endif()
#target_link_libraries("RavEngine" "RenderSystem_GLES2")
target_link_libraries("RavEngine" "RenderSystem_GL3Plus")
target_link_libraries("RavEngine" "RenderSystem_NULL")

#add_dependencies("RavEngine" "geometryc")
#add_dependencies("RavEngine" "shaderc")
#add_dependencies("RavEngine" "spirv-cross")
#add_dependencies("RavEngine" "spirv-opt")


# copy DLLs
if (WIN32)
	if(NOT PX_GENERATE_STATIC_LIBRARIES)
		add_custom_command(TARGET "RavEngine" POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${PROJECT_BINARY_DIR}/deps/bin/win.x86_64.vc142.md/$<CONFIGURATION>"
				"$<TARGET_FILE_DIR:RavEngine>")
	endif()

	# add_custom_command(TARGET "RavEngine" POST_BUILD
	# 	COMMAND ${CMAKE_COMMAND} -E copy_directory
	# 		"${PROJECT_BINARY_DIR}/deps/SDL2/$<CONFIGURATION>"
	# 		"$<TARGET_FILE_DIR:RavEngine>")

	#startup project	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "RavEngine_Test")
endif()
# link test game with library
target_link_libraries("RavEngine_Test" "RavEngine" )

