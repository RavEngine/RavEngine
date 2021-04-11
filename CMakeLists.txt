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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

OPTION( BUILD_SHARED_LIBS "Build package with shared libraries." OFF)
OPTION( RAVENGINE_BUILD_TESTS "Build tests" OFF)

# ban in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif()

# linux detection
if(UNIX AND NOT APPLE)
	set(LINUX TRUE CACHE INTERNAL "")
endif()

if(LINUX)
	set(CMAKE_CXX_STANDARD_LIBRARIES -latomic) # need libatomic on linux
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
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE INTERNAL "")
set(TOOLS_DIR ${CMAKE_BINARY_DIR}/host-tools CACHE INTERNAL "")

# ninja does not use separate config directories for some reason
if (CMAKE_GENERATOR STREQUAL "Ninja")
    set(SHADERC_PATH "${TOOLS_DIR}/bgfx.cmake/shaderc" CACHE INTERNAL "")
    set(PROTOC_CMD "${TOOLS_DIR}/protobuf/protoc" CACHE INTERNAL "")
else()
    set(SHADERC_PATH "${TOOLS_DIR}/bgfx.cmake/Release/shaderc" CACHE INTERNAL "")
    set(PROTOC_CMD "${TOOLS_DIR}/protobuf/Release/protoc" CACHE INTERNAL "")
endif()

if (APPLE OR LINUX)
	SET(SHADERC_CMD "${SHADERC_PATH}" CACHE INTERNAL "")
elseif(MSVC)
	SET(SHADERC_CMD "${SHADERC_PATH}.exe" CACHE INTERNAL "")
endif()

# ============ build machine tools ==============

# configure build machine tools
file(MAKE_DIRECTORY ${TOOLS_DIR})
execute_process(
    COMMAND cmake -G "${CMAKE_GENERATOR}" ${DEPS_DIR}/host-tools/
    WORKING_DIRECTORY ${TOOLS_DIR}
)

# compile build machine tools
add_custom_command(
    PRE_BUILD
    OUTPUT "${SHADERC_CMD}" 
	COMMAND cmake --build . --config Release --target shaderc protoc
	WORKING_DIRECTORY ${TOOLS_DIR}
    VERBATIM
)

# no extra flags required
add_subdirectory("${DEPS_DIR}/im3d-cmake")
add_subdirectory("${DEPS_DIR}/etl")
add_subdirectory("${DEPS_DIR}/tweeny")
add_subdirectory("${DEPS_DIR}/physfs")
add_subdirectory("${DEPS_DIR}/concurrentqueue")
add_subdirectory("${DEPS_DIR}/fmt")
add_subdirectory("${DEPS_DIR}/RmlUi-freetype")
add_subdirectory("${DEPS_DIR}/glm")

#SDL2
add_subdirectory("${DEPS_DIR}/SDL2")

# ozz animation
set(ozz_build_samples OFF CACHE INTERNAL "")
set(ozz_build_howtos OFF CACHE INTERNAL "")
set(ozz_build_tests OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/ozz-animation")

# libnyquist
SET(BUILD_EXAMPLE OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/libnyquist")

# GNS
add_subdirectory("${DEPS_DIR}/GameNetworkingSockets")
add_custom_target("GNS_Deps" DEPENDS "${SHADERC_CMD}")
add_dependencies("GameNetworkingSockets_s" "GNS_Deps")

# resonance-audio
set(BUILD_RESONANCE_AUDIO_API ON CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/resonance-audio")

# taskflow
SET(TF_BUILD_BENCHMARKS OFF CACHE INTERNAL "")
SET(TF_BUILD_CUDA OFF CACHE INTERNAL "")
SET(TF_BUILD_TESTS OFF CACHE INTERNAL "")
SET(TF_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/taskflow")

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

# recast
SET(RECASTNAVIGATION_DEMO OFF CACHE INTERNAL "")
SET(RECASTNAVIGATION_TESTS OFF CACHE INTERNAL "")
SET(RECASTNAVIGATION_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/recastnavigation")

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
set(GPU_DLL_COPIED 1 CACHE INTERNAL "")
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
	"${DEPS_DIR}/physfs/src"
	"${DEPS_DIR}/plf/"
	"${DEPS_DIR}/parallel-hashmap/parallel_hashmap"
	"${DEPS_DIR}/taskflow"
	"${DEPS_DIR}/RmlUi-freetype/RmlUi/Include"
	"${DEPS_DIR}/Uuids/include"
	"${DEPS_DIR}/resonance-audio/resonance_audio/"
	"${DEPS_DIR}/resonance-audio/platforms/"
	"${DEPS_DIR}/resonance-audio/third_party/eigen"
	"${DEPS_DIR}/resonance-audio/"
	"${DEPS_DIR}/GameNetworkingSockets/GameNetworkingSockets/include"
	PRIVATE
	"include/${PROJECT_NAME}/"
	"${DEPS_DIR}/miniz-cpp/"	
	"${DEPS_DIR}/stbi"
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
	"im3d"
	"physfs-static"
	"PffftObj"
	"SadieHrtfsObj"
	"ResonanceAudioObj"
	#"PhysXGPU"
	"RmlCore"
	"libnyquist"
	"GameNetworkingSockets_s"
	PUBLIC
	"glm"
	"fmt"
	"etl"
	"tweeny"
	"SDL2-static"
	"SDL2main"
	"bgfx"
	"bx"
	"bimg"
	"Recast"
	"Detour"
	"concurrentqueue"
	"ozz_animation"
	"ozz_animation_offline"
	"ozz_animation_tools"
	"ozz_base"
	"ozz_geometry"
	"ozz_options"
)

# raspberry pi needs this set explicitly, incompatible with other targets 
if(LINUX)
	target_link_libraries("${PROJECT_NAME}" PRIVATE "stdc++fs")
endif()

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
	get_property(sc_include_dir GLOBAL PROPERTY SC_INCLUDE_DIR)

	if(CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES iOS OR CMAKE_SYSTEM_NAME MATCHES tvOS)
		set(PLATFORM "osx")
		set(PROFILE_VS "metal")
		set(PROFILE_FS "metal")
	elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
		set(PLATFORM "windows")
		set(PROFILE_VS "vs_5_0")
		set(PROFILE_FS "ps_5_0")
	elseif(CMAKE_SYSTEM_NAME MATCHES Linux)
		set(PLATFORM "linux")
		set(PROFILE_VS "spirv")
		set(PROFILE_FS "spirv")
	else()
		message(FATAL_ERROR "Shader compiler is not supported on this platform")
	endif()
	
	set(OUTPUT_ROOT "${CMAKE_BINARY_DIR}/${shader_target}/shaders/${shader_name}")
	set(VS_OUTPUT_NAME "${OUTPUT_ROOT}/vertex.bin")
	set(FS_OUTPUT_NAME "${OUTPUT_ROOT}/fragment.bin")

	set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${VS_OUTPUT_NAME})
	set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${FS_OUTPUT_NAME})

	# compile shaders
	add_custom_command(
		PRE_BUILD
		OUTPUT "${VS_OUTPUT_NAME}" "${FS_OUTPUT_NAME}"
		DEPENDS "${vertex_src}" "${fragment_src}" "${varying_src}" "${SHADERC_CMD}"
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_ROOT}
		COMMAND "${SHADERC_CMD}" -f "${vertex_src}" -o "${VS_OUTPUT_NAME}" -i "${sc_include_dir}" --type "vertex" --platform "${PLATFORM}" --varyingdef "${varying_src}" --profile "${PROFILE_VS}"
		COMMAND "${SHADERC_CMD}" -f "${fragment_src}" -o "${FS_OUTPUT_NAME}" -i "${sc_include_dir}" --type "fragment" --platform "${PLATFORM}" --varyingdef "${varying_src}" --profile "${PROFILE_FS}"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
		COMMENT "Compiling Shader Descriptor ${shader_name} => ${VS_OUTPUT_NAME}, ${FS_OUTPUT_NAME}"
		VERBATIM
	)

endfunction()

define_property(GLOBAL PROPERTY ENG_DIR
	BRIEF_DOCS "Engine Directory"
	FULL_DOCS "Engine Directory"
)
set_property(GLOBAL PROPERTY ENG_DIR "${CMAKE_CURRENT_LIST_DIR}")

define_property(GLOBAL PROPERTY COPY_DEPENDS
	BRIEF_DOCS "Engine Directory"
	FULL_DOCS "Engine Directory"
)

# group libraries and projects
macro(group_in destination targets)
	foreach(target ${targets})
		if(TARGET ${target})
			SET_PROPERTY(TARGET "${target}" PROPERTY FOLDER "RavEngine SDK/${destination}")
		endif()
	endforeach()
endmacro()

SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "RavEngine SDK")
group_in("Libraries" "assimp;assimp_cmd;crypto;DebugUtils;Detour;DetourCrowd;DetourTileCache;fmt;freetype;GameNetworkingSockets_s;GNS_Deps;\
im3d;IrrXML;libnyquist;libopus;libprotobuf;libprotobuf-lite;libwavpack;openssl;PffftObj;physfs;physfs-static;BUILD_FUSE_ALL;\
Recast;ResonanceAudioObj;ResonanceAudioShared;ResonanceAudioStatic;rlottie;rlottie-image-loader;RmlCore;SadieHrtfsObj;ssl;\
test_physfs;tweeny-dummy;zlib;zlibstatic;SDL2main;SDL2-static;json;physfs_uninstall;dist\
")

group_in("Libraries/PhysX SDK" "FastXml;LowLevel;LowLevelAABB;LowLevelDynamics;PhysX;PhysXCharacterKinematic;PhysXCommon;\
PhysXCooking;PhysXExtensions;PhysXFoundation;PhysXPvdSDK;PhysXTask;PhysXVehicle;SceneQuery;SimulationController")

group_in("Libraries/ozz" "ozz_animation;ozz_animation_offline;ozz_base;ozz_geometry;ozz_options")
group_in("Libraries/ozz/tools" "dump2ozz;gltf2ozz;ozz_animation_tools")
group_in("Libraries/ozz/fuse" "BUILD_FUSE_ozz_animation;BUILD_FUSE_ozz_animation_offline;BUILD_FUSE_ozz_animation_tools;\
BUILD_FUSE_ozz_base;BUILD_FUSE_ozz_geometry;BUILD_FUSE_ozz_options")

group_in("Libraries/bgfx" "bgfx;bimg;bx")
group_in("Libraries/bgfx/tools" "shaderc;geometryc;geometryv;texturec;texturev;tools")
group_in("Libraries/bgfx/3rdparty" "astc;astc-codec;edtaa3;etc1;etc2;fcpp;glcpp;glslang;glsl-optimizer;iqa;mesa;meshoptimizer;nvtt;pvrtc;spirv-cross;spirv-tools;squish")

# exclude some targets from install
SET_PROPERTY(TARGET "texturev" PROPERTY EXCLUDE_FROM_ALL ON)
SET_PROPERTY(TARGET "geometryv" PROPERTY EXCLUDE_FROM_ALL ON)
SET_PROPERTY(TARGET "geometryc" PROPERTY EXCLUDE_FROM_ALL ON)
SET_PROPERTY(TARGET "texturec" PROPERTY EXCLUDE_FROM_ALL ON)

# pack resources
function(pack_resources)
	set(optional)
	set(args TARGET)
	set(list_args SHADERS OBJECTS TEXTURES UIS FONTS SOUNDS)
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
	file(GLOB ENG_OBJECTS "${eng_dir}/objects/*")

	# add engine-provided shaders
	file(GLOB ENG_SHADERS "${eng_dir}/shaders/*.cmake")

	# add engine-provided fonts
	file(GLOB ENG_FONTS "${eng_dir}/fonts/*.ttf")

	file(GLOB ENG_UIS "${eng_dir}/ui/*.rcss" "${eng_dir}/ui/*.rml")

	# clear copy-depends
	set_property(GLOBAL PROPERTY COPY_DEPENDS "")

	# helper for copying to staging directory
	function(copy_helper FILE_LIST output_dir)
		foreach(FILE ${FILE_LIST})
			# copy objects pre-build if they are changed
			get_filename_component(output_name "${FILE}" NAME)
			set(outname "${CMAKE_BINARY_DIR}/${ARGS_TARGET}/${output_dir}/${output_name}")
			add_custom_command(PRE_BUILD 
				OUTPUT "${outname}" 
				COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} "${outname}"
				DEPENDS ${FILE}
				)
			set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS ${outname})
		endforeach()
	endfunction()

	copy_helper("${ARGS_OBJECTS}" "objects")
	copy_helper("${ENG_OBJECTS}" "objects")
	copy_helper("${ARGS_TEXTURES}" "textures")
	copy_helper("${ARGS_UIS}" "uis")
	copy_helper("${ENG_UIS}" "uis")
	copy_helper("${ARGS_FONTS}" "fonts")
	copy_helper("${ENG_FONTS}" "fonts")
	copy_helper("${ARGS_SOUNDS}" "sounds")


	# get dependency outputs
	get_property(copy_depends GLOBAL PROPERTY COPY_DEPENDS)

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
	add_custom_target("${ARGS_TARGET}_CompileShaders" ALL DEPENDS ${all_shaders_property})
	add_dependencies("${ARGS_TARGET}" "${ARGS_TARGET}_CompileShaders" "RavEngine")

	set(outpack "${CMAKE_BINARY_DIR}/${ARGS_TARGET}.zip")

	# allow inserting into the mac / ios resource bundle
	set_target_properties(${ARGS_TARGET} PROPERTIES 
		MACOSX_BUNDLE TRUE
		OSX_ARCHITECTURES "arm64;x86_64"
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
	)

	set(assets ${ARGS_OBJECTS} ${all_shaders_property} ${ENG_OBJECTS} ${ARGS_TEXTURES} ${copy_depends})

	# the command to pack into a zip
	add_custom_command(
		POST_BUILD 
		OUTPUT "${outpack}"
		DEPENDS ${assets}
		COMMENT "Packing resources for ${ARGS_TARGET}"
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${outpack}" --format=zip ${ARGS_TARGET} 
		VERBATIM
	)

	# make part of the target, and add to the resources folder if applicable
	target_sources("${ARGS_TARGET}" PRIVATE "${outpack}")
	set_source_files_properties("${outpack}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	
	# copy to target dir on Win
	if(MSVC OR LINUX)
		get_filename_component(outfile ${outpack} NAME)
		SET(outfile "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${outfile}")
		add_custom_command(
			TARGET "${ARGS_TARGET}"
			DEPENDS "${outpack}"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${outpack}" "${outfile}"
			COMMENT "Copying assets package to executable directory"
		)
	endif()
endfunction()

# tests
if (RAVENGINE_BUILD_TESTS)
	include(CTest)
	add_executable("${PROJECT_NAME}_TestBasics" "test/basics.cpp")
	target_link_libraries("${PROJECT_NAME}_TestBasics" PUBLIC "RavEngine" )

	macro(test name executable)
	add_test(
		NAME ${name} 
		COMMAND ${executable} "${name}" -C $<CONFIGURATION> 
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>
	)
	endmacro()

	test("CTTI" "${PROJECT_NAME}_TestBasics")
	test("Test_UUID" "${PROJECT_NAME}_TestBasics")
endif()
