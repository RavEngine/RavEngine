cmake_minimum_required(VERSION 3.17)
set(OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
include(ProcessorCount)
project(RavEngine)

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
	set(LINUX TRUE)
endif()

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# ==================== Dependencies =====================

#add_subdirectory("${DEPS_DIR}/SDL2")

set(BGFX_BUILD_EXAMPLES OFF)
set(BGFX_INSTALL OFF)
set(BGFX_INSTALL_EXAMPLES OFF)
add_subdirectory("${DEPS_DIR}/bgfx.cmake")

# PhysX-specific CMake project setup
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "Use the debug version of the CRT")
set(PHYSX_ROOT_DIR ${DEPS_DIR}/physx/physx)
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared)
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(PX_PHYSX_ ${CMAKE_INSTALL_PREFIX})
set(CMAKEMODULES_VERSION "1.27")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules)
set(PX_OUTPUT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PX_OUTPUT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PX_GENERATE_STATIC_LIBRARIES ON)
#set(PX_FLOAT_POINT_PRECISE_MATH OFF)
if (WIN32)
	set(TARGET_BUILD_PLATFORM "windows")
elseif(APPLE)
	set(TARGET_BUILD_PLATFORM "mac")
elseif(LINUX)
	set(TARGET_BUILD_PLATFORM "linux")
endif()

# Call into PhysX's CMake scripts
add_subdirectory(${PHYSX_ROOT_DIR}/compiler/public)

# build OGRE
if(WIN32)
	list(APPEND EXTRA_LIBS vcruntime user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)
	
endif()
#cmake_policy(SET CMP0021 OLD)
set(OGRE_ROOT "${DEPS_DIR}/ogre-next/")
set(OGRE_USE_BOOST OFF)
set(OGRE_CONFIG_THREAD_PROVIDER OFF)
set(OGRE_BUILD_COMPONENT_SCENE_FORMAT ON)
set(OGRE_BUILD_SAMPLES2 OFF)
set(OGRE_BUILD_TESTS OFF)
set(OGRE_DEPENDENCIES_DIR "${OGRE_ROOT}/Dependencies/")
set(OGRE_BUILD_LIBS_AS_FRAMEWORKS OFF)
ProcessorCount(OGRE_CONFIG_THREADS)
if(APPLE)
	set(OGRE_BUILD_RENDERSYSTEM_GL3PLUS OFF)	# disable OpenGL on Mac
elseif(WIN32)
	set(CMAKE_CXX_STANDARD 11)
endif()
add_subdirectory("${OGRE_ROOT}")

set(CMAKE_CXX_STANDARD 17)


# ========== Building engine and test game ==============

# get header search paths
include_directories("include/RavEngine/")
include_directories("orge-next/OgreMain/include")
include_directories("orge-next/Components/Hlms/Common/include")
include_directories("orge-next/Components/Hlms/Pbs/include")
include_directories("orge-next/Components/Unlit/Pbs/include")
include_directories("orge-next/build/Release/include")
include_directories("${OGRE_DEPENDENCIES_DIR}/src/SDL2/include/")
include_directories("${DEPS_DIR}/bgfx.cmake/bimg/include/")
include_directories("${DEPS_DIR}/bgfx.cmake/bx/include/")
include_directories("${DEPS_DIR}/bgfx.cmake/bgfx/include/")
include_directories("${DEPS_DIR}/physx/physx/include/")
include_directories("${DEPS_DIR}/physx/pxshared/include/")
include_directories("${DEPS_DIR}/physx/physx/snippets/")
include_directories("${DEPS_DIR}/openal-soft/include/")

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
target_link_libraries("RavEngine" "bgfx")
target_link_libraries("RavEngine" "bimg")
#target_link_libraries("RavEngine" "bimg_decode")
target_link_libraries("RavEngine" "bx")
target_link_libraries("RavEngine" "fcpp")
target_link_libraries("RavEngine" "glslang")
target_link_libraries("RavEngine" "glsl-optimizer")
target_link_libraries("RavEngine" "OgreHlmsUnlit")
target_link_libraries("RavEngine" "OgreMain")
target_link_libraries("RavEngine" "OgreHlmsPbs")

add_dependencies("RavEngine" "geometryc")
add_dependencies("RavEngine" "shaderc")
add_dependencies("RavEngine" "spirv-cross")
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

