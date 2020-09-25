cmake_minimum_required(VERSION 3.17)
project(RavEngine)

# ========== CMake Boilerplate ==============
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${DEPS_DIR}")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

# ban in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif()

# linux detection
if(UNIX AND NOT APPLE)
	set(LINUX TRUE CACHE INTERNAL "")
endif()

# enable multiprocessor compilation with vs
# Remove 'lib' prefix for shared libraries on Windows
if(MSVC)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
  add_definitions(/MP)
  add_definitions(/Qpar)
  list(APPEND EXTRA_LIBS vcruntime user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32)
endif()

# include CMRC
include("${DEPS_DIR}/cmrc/CMakeRC.cmake")

# ==================== Dependencies =====================

# use static SDL
#set(SDL_STATIC ON CACHE INTERNAL "")
#set(SDL_SHARED OFF CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory("${DEPS_DIR}/SDL2")

# bgfx
set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_INSTALL_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_INSTALL OFF CACHE INTERNAL "")
#set(BGFX_CONFIG_RENDERER_WEBGPU ON)
add_subdirectory("${DEPS_DIR}/bgfx.cmake")

# assimp
SET(IGNORE_GIT_HASH ON CACHE INTERNAL "")
SET(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/assimp")

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

add_subdirectory("${DEPS_DIR}/tarlib")

# ========== Building engine and test game ==============

# get all sources for the library with glob
if(APPLE)
	# also need to compile Objective-C++ files
	file(GLOB MM_SOURCES "src/*.mm")
	add_definitions("-x objective-c++")
	set(CMAKE_EXE_LINKER_FLAGS "-framework ServiceManagement -framework Foundation -framework Cocoa -framework Metal -framework QuartzCore")
endif()
file(GLOB SOURCES "include/${PROJECT_NAME}/*.h" "include/${PROJECT_NAME}/*.hpp" "src/*.cpp")

# register the library
add_library("${PROJECT_NAME}" ${SOURCES} ${MM_SOURCES})

# include paths
target_include_directories("${PROJECT_NAME}" 
	PUBLIC 
	"include/"
	"${DEPS_DIR}/physx/physx/include/" 
	"${DEPS_DIR}/physx/pxshared/include/" 
	"${DEPS_DIR}/physx/physx/snippets/"
	"include/${PROJECT_NAME}/stduuid/"
	PRIVATE
	"include/${PROJECT_NAME}/"
)

# create the test executable
file(GLOB TEST_SOURCES "${PROJECT_NAME}_Test/*.cpp" "${PROJECT_NAME}_Test/*.hpp" "${PROJECT_NAME}_Test/*.h")

# creates a mac app if applicable
add_executable("${PROJECT_NAME}_Test" MACOSX_BUNDLE ${TEST_SOURCES})
set_target_properties("${PROJECT_NAME}_Test" PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}_Test/Info.plist"
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
)

#startup project	
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}_Test")

# ====================== Linking ====================
target_link_libraries("${PROJECT_NAME}" PRIVATE 
	"PhysXExtensions"
	"PhysX"
	"PhysXPvdSDK"
	"PhysXVehicle"
	"PhysXCharacterKinematic"
	"PhysXCooking"
	"PhysXCommon"
	"PhysXFoundation"
	"PhysXTask"
	"FastXml"
	"LowLevel"
	"LowLevelAABB"
	"LowLevelDynamics"
	"SceneQuery"
	"SimulationController"
	"tarlib"
	"assimp"
	#"PhysXGPU"
	PUBLIC
	"SDL2-static"
	"bgfx"
	"bx"
	"bimg"
)
#target_link_libraries("${PROJECT_NAME}" PUBLIC "SnippetUtils")

# copy DLLs
if (WIN32)
	# PhysX
	if(NOT PX_GENERATE_STATIC_LIBRARIES)
		add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${CMAKE_BINARY_DIR}/deps/bin/win.x86_64.vc142.md/$<CONFIGURATION>"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<CONFIGURATION>")
	endif()

endif()

# link test game with library
target_link_libraries("${PROJECT_NAME}_Test" "${PROJECT_NAME}" )

# ====================== Shader compiler =====================

set(SHADERCOMPILER "${PROJECT_NAME}_shaderc_auto")

FILE(GLOB AUTOSHADERC "${SHADERCOMPILER}" "src/shaderc_auto/*.cpp")
add_executable("${SHADERCOMPILER}" ${AUTOSHADERC})
target_include_directories("${SHADERCOMPILER}" PRIVATE "${DEPS_DIR}/cxxopts/include" "${DEPS_DIR}/json/")
target_link_libraries("${SHADERCOMPILER}" PRIVATE "bgfx" "tarlib")

add_dependencies("${SHADERCOMPILER}" "shaderc")	# shaderc must build before auto executable so that auto shader comp can work

# define shader compilation actions
define_property(GLOBAL PROPERTY ALL_SHADERS
	BRIEF_DOCS "Aggregate shader list"
    FULL_DOCS "Aggregate shader list")
set_property(GLOBAL PROPERTY ALL_SHADERS "")

function (compile_shader_dir shader_dir)
	# get all the shader files
	file(GLOB SHADERS "${shader_dir}/*.json")
	foreach(SHADER ${SHADERS})
		get_filename_component(output_file "${SHADER}" NAME_WE)
		set(output_file "${output_file}.tar")
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS "${CMAKE_BINARY_DIR}/shaders/${output_file}")
		add_custom_command(
			PRE_BUILD
			OUTPUT "${CMAKE_BINARY_DIR}/shaders/${output_file}"
			COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${SHADERCOMPILER}" -f "${SHADER}" -o .. -i "${DEPS_DIR}/bgfx.cmake/bgfx/src"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
			COMMENT "Compiling Shader Descriptor ${SHADER} => ${output_file}"
			VERBATIM
		)
	endforeach()
endfunction()


# compile all builtin shaders
compile_shader_dir("shaders")
#TODO: compile user-defined shaders

#track all the shaders for compilation
get_property(all_shaders_property GLOBAL PROPERTY ALL_SHADERS)
add_custom_target("${SHADERCOMPILER}_Compile" ALL DEPENDS ${all_shaders_property} "${SHADERCOMPILER}")
add_dependencies("${PROJECT_NAME}" "${SHADERCOMPILER}_Compile")

# Create the resource library using these paths
cmrc_add_resource_library("${PROJECT_NAME}_RSC" ALIAS "${PROJECT_NAME}::RSC" 
	"${all_shaders_property}"
	WHENCE "${CMAKE_BINARY_DIR}/"
)
target_link_libraries("${PROJECT_NAME}" PRIVATE "${PROJECT_NAME}::RSC")
message(${all_shaders_property})
add_dependencies("${PROJECT_NAME}_RSC" "${SHADERCOMPILER}_Compile")	# force shaders to build before resource library

# INSTALL(CODE "
# 	include(BundleUtilities)
# 	fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/$<CONFIGURATION>/${PROJECT_NAME}_Test.app\" \"\" \"\")
# 	" COMPONENT Runtime)
