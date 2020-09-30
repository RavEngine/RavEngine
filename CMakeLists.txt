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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

OPTION( BUILD_SHARED_LIBS
  "Build package with shared libraries."
  OFF
)

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

# ==================== Dependencies =====================

# use static SDL
#set(SDL_STATIC ON CACHE INTERNAL "")
#set(SDL_SHARED OFF CACHE INTERNAL "")

add_subdirectory("${DEPS_DIR}/SDL2")

add_subdirectory("${DEPS_DIR}/RavTar")
add_subdirectory("${DEPS_DIR}/miniz-cpp")
add_subdirectory("${DEPS_DIR}/rscpack")

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
	set(PLATFORM "Windows")
elseif(APPLE)
	set(TARGET_BUILD_PLATFORM "mac" CACHE INTERNAL "")
	set(PLATFORM "macOS")
elseif(LINUX)
	set(TARGET_BUILD_PLATFORM "linux" CACHE INTERNAL "")
	set(CMAKE_LIBRARY_ARCHITECTURE "x86_64-linux-gnu" CACHE INTERNAL "")
	set(PLATFORM "Linux")
	#set(CMAKE_LIBRARY_ARCHITECTURE "aarch64-linux-gnu" CACHE INTERNAL "")
endif()

# Call into PhysX's CMake scripts
add_subdirectory("${PHYSX_ROOT_DIR}/compiler/public")
add_subdirectory("${DEPS_DIR}/ttvfs")

# ========== Building engine ==============

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
	"${DEPS_DIR}/ttvfs/ttvfs"
	"${DEPS_DIR}/ttvfs/ttvfs_zip"
	PRIVATE
	"include/${PROJECT_NAME}/"
	"${DEPS_DIR}/miniz-cpp/"
	
)

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
	"assimp"
	"ttvfs"
	"ttvfs_zip"
	"ravtar"
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

# ====================== Shader compiler =====================

set(SHADERCOMPILER "${PROJECT_NAME}_shaderc_auto")

FILE(GLOB AUTOSHADERC "${SHADERCOMPILER}" "src/shaderc_auto/*.cpp")
add_executable("${SHADERCOMPILER}" ${AUTOSHADERC})
target_include_directories("${SHADERCOMPILER}" PRIVATE "${DEPS_DIR}/cxxopts/include" "${DEPS_DIR}/json/" "${DEPS_DIR}/miniz-cpp/")
target_link_libraries("${SHADERCOMPILER}" PRIVATE "bgfx" "ravtar")

add_dependencies("${SHADERCOMPILER}" "shaderc")	# shaderc must build before auto executable so that auto shader comp can work

# define shader compilation actions
define_property(GLOBAL PROPERTY SC_COMP_NAME
	BRIEF_DOCS "Shader compiler name"
	FULL_DOCS "Shader compiler name"
)
set_property(GLOBAL PROPERTY SC_COMP_NAME "${SHADERCOMPILER}")

define_property(GLOBAL PROPERTY SC_INCLUDE_DIR
	BRIEF_DOCS "Shader include path"
	FULL_DOCS "Shader include path"
)
set_property(GLOBAL PROPERTY SC_INCLUDE_DIR "${DEPS_DIR}/bgfx.cmake/bgfx/src")

function (compile_shader_dir shader_dir resource_root)
	# get all the shader files
	file(GLOB SHADERS "${shader_dir}/*.json")

	define_property(GLOBAL PROPERTY ALL_SHADERS
		BRIEF_DOCS "Aggregate shader list"
		FULL_DOCS "GLOBAL shader list"
	)
	set_property(GLOBAL PROPERTY ALL_SHADERS "")

	get_property(sc_comp_name GLOBAL PROPERTY SC_COMP_NAME)
	get_property(sc_include_dir GLOBAL PROPERTY SC_INCLUDE_DIR)

	foreach(SHADER ${SHADERS})
		get_filename_component(output_file "${SHADER}" NAME_WE)
		set(output_file "${output_file}.tar" )
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS "${CMAKE_BINARY_DIR}/${resource_root}/shaders/${output_file}")
		add_custom_command(
			PRE_BUILD
			OUTPUT "${CMAKE_BINARY_DIR}/${resource_root}/shaders/${output_file}"
			COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${sc_comp_name}" -f "${SHADER}" -o ../${resource_root} -i "${sc_include_dir}"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
			COMMENT "Compiling Shader Descriptor ${SHADER} => ${output_file}"
			VERBATIM
		)
	endforeach()

	#track all the shaders for compilation
	get_property(all_shaders_property GLOBAL PROPERTY ALL_SHADERS)
	add_custom_target("${sc_comp_name}_${resource_root}_Compile" ALL DEPENDS ${all_shaders_property} "RavEngine_shaderc_auto")
	add_dependencies("${resource_root}" "${sc_comp_name}_${resource_root}_Compile" "rscpack" "RavEngine")
endfunction()


# compile all builtin shaders
compile_shader_dir("shaders" "${PROJECT_NAME}")

# Mesh adding macro
macro(add_meshes target meshes)
	foreach(MESH ${meshes})
		# copy meshes pre-build if they are changed
		get_filename_component(output_name "${MESH}" NAME)
		add_custom_command(TARGET "${target}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MESH} "${CMAKE_BINARY_DIR}/${target}/meshes/${output_name}")
	endforeach()
endmacro()

file(GLOB MESHES "meshes/*")
add_meshes("${PROJECT_NAME}" "${MESHES}")

# pack resources
macro(pack_resources target_rsc)
	set(outpack "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${target_rsc}.zip")
	if(APPLE)
		set(outpack "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${target_rsc}.app/Contents/Resources/${target_rsc}.zip")
	endif()

	add_custom_command(TARGET "${target_rsc}" POST_BUILD COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/rscpack" -a ${target_rsc} -e "${CMAKE_BINARY_DIR}/RavEngine" -o "${outpack}")
	# target_sources(${target_rsc} PUBLIC "${outpack}")

	set_source_files_properties("${outpack}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	set_target_properties(${target_rsc} PROPERTIES 
		MACOSX_BUNDLE TRUE
	)
endmacro()