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
endif ()

# linux detection
if(UNIX AND NOT APPLE)
	set(LINUX TRUE CACHE INTERNAL "")
endif()

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()


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


if(MSVC)
	list(APPEND EXTRA_LIBS vcruntime user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32)
endif()

# enable multiprocessor compilation with vs
if(MSVC)
  add_definitions(/MP)
  add_definitions(/Qpar)
endif()


# ==================== Dependencies =====================

# shader compiler
#add_subdirectory("${DEPS_DIR}/shader-transpiler")

# use static SDL
#set(SDL_STATIC ON CACHE INTERNAL "")
#set(SDL_SHARED OFF CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory("${DEPS_DIR}/SDL2")

# bgfx
set(BGFX_BUILD_EXAMPLES OFF)
set(BGFX_INSTALL_EXAMPLES OFF)
set(BGFX_INSTALL OFF)
#set(BGFX_CONFIG_RENDERER_WEBGPU ON)
add_subdirectory("${DEPS_DIR}/bgfx.cmake")

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
file(GLOB TEST_SOURCES "${PROJECT_NAME}_Test/*.cpp")
file(GLOB TEST_HEADERS "${PROJECT_NAME}_Test/*.hpp")
file(GLOB TEST_CHEADERS "${PROJECT_NAME}_Test/*.h")

# creates a mac app if applicable
add_executable("${PROJECT_NAME}_Test" MACOSX_BUNDLE ${TEST_SOURCES} ${TEST_HEADERS} ${TEST_CHEADERS})
set_target_properties("${PROJECT_NAME}_Test" PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}_Test/Info.plist")

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
	"bgfx"
	"bx"
	"bimg"
	PUBLIC
	"SDL2-static"
)

#target_link_libraries("${PROJECT_NAME}" PUBLIC "PhysXGPU")

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
define_property(GLOBAL PROPERTY ALL_SHADERS
	BRIEF_DOCS "Aggregate shader list"
    FULL_DOCS "Aggregate shader list")
set_property(GLOBAL PROPERTY ALL_SHADERS "")

function (compile_shader_dir shader_dir)
	# get all the shader files
	file(GLOB SHADERS "${shader_dir}/*.json")
	foreach(SHADER ${SHADERS})
		get_filename_component(output_file "${SHADER}" NAME_WE)
		set(output_file "${output_file}.bin")
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS "${output_file}")
		add_custom_command(
			PRE_BUILD
			OUTPUT "${output_file}"
			COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/shader-transpiler" -f "${SHADER}" -o .
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
			COMMENT "Compiling Shader Descriptor ${SHADER} => ${output_file}"
			VERBATIM
		)
	endforeach()
endfunction()

# compile all builtin shaders
#compile_shader_dir("shaders")
# TODO: compile user-defined shaders

# track all the shaders for compilation
# get_property(all_shaders_property GLOBAL PROPERTY ALL_SHADERS)
# add_custom_target("${PROJECT_NAME}_ShaderCompiler" ALL DEPENDS ${all_shaders_property} "shader-transpiler")
# add_dependencies("${PROJECT_NAME}" "${PROJECT_NAME}_ShaderCompiler")

# # mark shaders as RESOURECS for MacOS .app and Linux .appimage
# set_target_properties("${PROJECT_NAME}" PROPERTIES
# 	RESOURCE "${all_shaders_property}"
# )

# INSTALL(CODE "
# 	include(BundleUtilities)
# 	fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/$<CONFIGURATION>/${PROJECT_NAME}_Test.app\" \"\" \"\")
# 	" COMPONENT Runtime)
