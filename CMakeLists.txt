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

# ==================== Dependencies =====================
add_subdirectory("${DEPS_DIR}/im3d-cmake")

add_subdirectory("${DEPS_DIR}/etl")

# use static SDL
#set(SDL_STATIC ON CACHE INTERNAL "")
#set(SDL_SHARED OFF CACHE INTERNAL "")

add_subdirectory("${DEPS_DIR}/SDL2")
add_subdirectory("${DEPS_DIR}/tweeny")
add_subdirectory("${DEPS_DIR}/RavTar")
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
	"${DEPS_DIR}/plf/"
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
	"im3d"
	#"PhysXGPU"
	PUBLIC
	"etl"
	"tweeny"
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

# globals for managing state
set(shader_target "default")
define_property(GLOBAL PROPERTY ALL_SHADERS
		BRIEF_DOCS "Aggregate shader list"
		FULL_DOCS "GLOBAL shader list"
	)
set_property(GLOBAL PROPERTY ALL_SHADERS "")

# Define a shader
# all paths must be absolute
function(declare_shader shader_name vertex_src fragment_src varying_src)
	get_property(sc_comp_name GLOBAL PROPERTY SC_COMP_NAME)
	get_property(sc_include_dir GLOBAL PROPERTY SC_INCLUDE_DIR)

	set(output_file "${shader_name}.tar" )
	set(full_output_path "${CMAKE_BINARY_DIR}/${shader_target}/shaders/${output_file}")
	set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${full_output_path})
	add_custom_command(
		PRE_BUILD
		OUTPUT "${full_output_path}"
		DEPENDS "${vertex_src}" "${fragment_src}" "${varying_src}"
		COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${sc_comp_name}" -n "${shader_name}" -v "${vertex_src}" -f "${fragment_src}" -d "${varying_src}" -o ../${shader_target} -i "${sc_include_dir}"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
		COMMENT "Compiling Shader Descriptor ${shader_name} => ${output_file}"
		VERBATIM
	)
endfunction()

define_property(GLOBAL PROPERTY ENG_DIR
	BRIEF_DOCS "Engine Directory"
	FULL_DOCS "Engine Directory"
)
set_property(GLOBAL PROPERTY ENG_DIR "${CMAKE_CURRENT_LIST_DIR}")

# pack resources
function(pack_resources)
	set(optional)
	set(args TARGET)
	set(list_args SHADERS MESHES)
	cmake_parse_arguments(
		PARSE_ARGV 0
		ARGS
		"${optional}"
		"${args}"
		"${list_args}"
	)

	if(${ARGS_UNPARSED_ARGUMENTS})
		message(WARNING "Unparsed arguments: ${ARGS_UNPARSED_ARGUMENTS}")
	endif()

	get_property(eng_dir GLOBAL PROPERTY ENG_DIR)

	# add polygon primitives provided by engine
	file(GLOB ENG_MESHES "${eng_dir}/meshes/*")

	# add engine-provided shaders
	file(GLOB ENG_SHADERS "${eng_dir}/shaders/*.cmake")

	foreach(MESH ${ARGS_MESHES} ${ENG_MESHES})
		# copy meshes pre-build if they are changed
		get_filename_component(output_name "${MESH}" NAME)
		add_custom_command(TARGET "${ARGS_TARGET}" PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MESH} "${CMAKE_BINARY_DIR}/${ARGS_TARGET}/meshes/${output_name}")
	endforeach()

	# clear global shaders property
	set_property(GLOBAL PROPERTY ALL_SHADERS "")

	# setup shader compiler
	foreach(SHADER ${ARGS_SHADERS} ${ENG_SHADERS})
		set(shader_target "${ARGS_TARGET}")
		include("${SHADER}")
	endforeach()

	get_property(sc_comp_name GLOBAL PROPERTY SC_COMP_NAME)
	get_property(sc_include_dir GLOBAL PROPERTY SC_INCLUDE_DIR)

	#track all the shaders for compilation
	get_property(all_shaders_property GLOBAL PROPERTY ALL_SHADERS)
	add_custom_target("${ARGS_TARGET}_CompileShaders" ALL DEPENDS ${all_shaders_property} "RavEngine_shaderc_auto")
	add_dependencies("${ARGS_TARGET}" "${ARGS_TARGET}_CompileShaders" "RavEngine" "rscpack")

	set(outpack "${CMAKE_BINARY_DIR}/${ARGS_TARGET}.zip")

	# allow inserting into the mac / ios resource bundle
	set_target_properties(${ARGS_TARGET} PROPERTIES 
		MACOSX_BUNDLE TRUE
	)

	set(assets ${ARGS_MESHES} ${all_shaders_property} ${ENG_MESHES})

	# the command to rscpack
	add_custom_command(
		POST_BUILD 
		OUTPUT "${outpack}"
		DEPENDS ${assets}
		COMMENT "Packing resources for ${ARGS_TARGET}"
		COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/rscpack" -a ${ARGS_TARGET} -e "${CMAKE_BINARY_DIR}/RavEngine" -o "${outpack}"
		VERBATIM
	)

	# make part of the target, and add to the resources folder if applicable
	target_sources("${ARGS_TARGET}" PRIVATE "${outpack}")
	set_source_files_properties("${outpack}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endfunction()

