cmake_minimum_required(VERSION 3.25)
project(RavEngine)

# ========== CMake Boilerplate ==============
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}"
"${DEPS_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_XCODE_VERSION "12.0.0" CACHE INTERNAL "")
set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON CACHE INTERNAL "")

OPTION( BUILD_SHARED_LIBS "Build package with shared libraries." OFF)

OPTION( RAVENGINE_BUILD_TESTS "Build tests" OFF)
option( RAVENGINE_SERVER "Build as a headless server" ${RAVENGINE_BUILD_TESTS})

# ban in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if(CMAKE_SYSTEM_NAME MATCHES iOS OR CMAKE_SYSTEM_NAME MATCHES tvOS)
	set(CMAKE_SYSTEM_PROCESSOR "aarch64")
endif()


include(deps/cmrc/CMakeRC.cmake)

if (APPLE)
	add_definitions(-fvisibility=default -ftemplate-backtrace-limit=0 -fobjc-arc)	# silence warning when building ARM fat library on Apple platforms, enable arc
elseif(EMSCRIPTEN)
	# required for higher memory, atomics, and threads
	add_definitions(-pthread)
	add_definitions(-fexceptions)
	set(EM_LINK "-fexceptions" "-s MAX_WEBGL_VERSION=2" "-s MIN_WEBGL_VERSION=2" "-s FULL_ES3=1" "-s USE_WEBGPU" "-s GL_ASSERTIONS=1" "-s OFFSCREEN_FRAMEBUFFER=1" "-s OFFSCREENCANVAS_SUPPORT=1" "-s GL_DEBUG=1" "-s LLD_REPORT_UNDEFINED" "-s NO_DISABLE_EXCEPTION_CATCHING" "-s NO_DISABLE_EXCEPTION_THROWING" "-s PTHREAD_POOL_SIZE=4" "-s ASSERTIONS=1" "-s ALLOW_MEMORY_GROWTH=1" "-s MAXIMUM_MEMORY=4GB")
endif()

# call this macro to add IPO to profile and release builds
macro(rve_enable_IPO target)

	set_target_properties(${target} PROPERTIES
		INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
		INTERPROCEDURAL_OPTIMIZATION_PROFILE TRUE
	)

endmacro()

# enable multiprocessor compilation with vs
# Remove 'lib' prefix for shared libraries on Windows
if(MSVC)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
    	add_definitions(/MP)				# parallelize each target, unless Ninja is the generator
	endif()
	if (WINDOWS_STORE)
		add_definitions(/sdl-)
	endif()
endif()

# ============ build machine tools ==============

if(NOT (CMAKE_VS_PLATFORM_NAME STREQUAL ""))
	if(CMAKE_VS_PLATFORM_NAME_DEFAULT STREQUAL CMAKE_VS_PLATFORM_NAME)
		set(VS_CROSSCOMP OFF CACHE INTERNAL "")
	else()
		set(VS_CROSSCOMP ON CACHE INTERNAL "")
	endif()
else()
	set(VS_CROSSCOMP OFF CACHE INTERNAL "")
endif()

if (VS_CROSSCOMP AND CMAKE_HOST_WIN32)
	set(CMAKE_CROSSCOMPILING ON CACHE INTERNAL "")
endif()

# ninja does not use separate config directories for some reason
if (CMAKE_CROSSCOMPILING AND NOT RAVENGINE_SERVER)
	set(TOOLS_DIR ${CMAKE_BINARY_DIR}/host-tools CACHE INTERNAL "")
	if (CMAKE_HOST_WIN32)
		set(rglc_ext ".exe")
	endif()
	if ((CMAKE_GENERATOR STREQUAL "Ninja" AND NOT (ANDROID AND CMAKE_HOST_WIN32)) OR CMAKE_GENERATOR STREQUAL "Unix Makefiles")
		set(PROTOC_CMD "${TOOLS_DIR}/protobuf/protoc" CACHE INTERNAL "")
		set(rglc_path "${TOOLS_DIR}/RGL/rglc${rglc_ext}" CACHE INTERNAL "")
  		set(FlatBuffers_EXECUTABLE "${TOOLS_DIR}/flatc/flatc")
	else()
		set(PROTOC_CMD "${TOOLS_DIR}/protobuf/Release/protoc" CACHE INTERNAL "")
		set(rglc_path "${TOOLS_DIR}/RGL/Release/rglc${rglc_ext}" CACHE INTERNAL "")
  		set(FlatBuffers_EXECUTABLE "${TOOLS_DIR}/flatc/Release/flatc")
	endif()

	file(MAKE_DIRECTORY ${TOOLS_DIR})
	if(LINUX OR (CMAKE_HOST_APPLE AND EMSCRIPTEN) OR (CMAKE_HOST_APPLE AND ANDROID))
		# need to ensure that if cross-compiling, we don't use the cross-compiler for the host tools
		set(LINUX_HOST_CC "-DCMAKE_C_COMPILER=cc" CACHE INTERNAL "")
		set(LINUX_HOST_CXX "-DCMAKE_CXX_COMPILER=c++" CACHE INTERNAL "")
	endif()

	if (ANDROID AND CMAKE_HOST_WIN32)
		set(HT_GENERATOR "Visual Studio 17 2022")
		set(HT_MAKEPROG "")
	else()
		set(HT_GENERATOR "${CMAKE_GENERATOR}")
		set(HT_MAKEPROG "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
	endif()

	execute_process(
		COMMAND ${CMAKE_COMMAND} -G "${HT_GENERATOR}" ${HT_MAKEPROG} ${LINUX_HOST_CC} ${LINUX_HOST_CXX} -DCMAKE_BUILD_TYPE=Release ${DEPS_DIR}/host-tools/
		WORKING_DIRECTORY ${TOOLS_DIR}
		RESULT_VARIABLE HOST_TOOLS_RESULT
	)
	if (NOT (HOST_TOOLS_RESULT EQUAL 0))
		message(FATAL_ERROR "Failed to configure host tools. See above for output.")
	endif()

	if(CMAKE_HOST_WIN32)
		set(FlatBuffers_EXECUTABLE "${FlatBuffers_EXECUTABLE}.exe")
	endif()

	add_custom_command(
		PRE_BUILD
		OUTPUT "${PROTOC_CMD}" "${rglc_path}" "${FlatBuffers_EXECUTABLE}"
		COMMAND ${CMAKE_COMMAND} --build . --config Release --target protoc rglc flatc
		WORKING_DIRECTORY "${TOOLS_DIR}"
		VERBATIM
	)

	add_custom_target(flatc DEPENDS "${FlatBuffers_EXECUTABLE}")
else()
	set(TOOLS_DIR ${CMAKE_CURRENT_BINARY_DIR}/host-tools CACHE INTERNAL "")
	set(PROTOC_CMD "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/protoc" CACHE INTERNAL "")
endif()

if (NOT RAVENGINE_SERVER)
	set(DISABLE_RTTI_AND_EXCEPTIONS OFF CACHE INTERNAL "")
	add_subdirectory("${DEPS_DIR}/RmlUi-freetype" EXCLUDE_FROM_ALL)
	set(RML_LIB "RmlCore")
endif()

if (ANDROID)
	set(ANDROID_FUNCTION_LEVEL_LINKING OFF CACHE INTERNAL "")
endif()

# no extra flags required
add_subdirectory("${DEPS_DIR}/im3d-cmake" EXCLUDE_FROM_ALL)
add_subdirectory("${DEPS_DIR}/tweeny" EXCLUDE_FROM_ALL)
add_subdirectory("${DEPS_DIR}/concurrentqueue" EXCLUDE_FROM_ALL)
add_subdirectory("${DEPS_DIR}/glm" EXCLUDE_FROM_ALL)
add_subdirectory("${DEPS_DIR}/r8brain-cmake" EXCLUDE_FROM_ALL)
add_subdirectory(deps/dr_wav EXCLUDE_FROM_ALL)
add_subdirectory(deps/fmt EXCLUDE_FROM_ALL)

# randoms
set(Random_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/random" EXCLUDE_FROM_ALL)

#SDL
# ensure library is built correctly for static
if (NOT RAVENGINE_SERVER)
	if (NOT ANDROID)
		set(SDL_STATIC ON CACHE INTERNAL "" FORCE)
		set(SDL_SHARED OFF CACHE INTERNAL "" FORCE)
	else()
		set(SDL_STATIC OFF CACHE INTERNAL "" FORCE)
		set(SDL_SHARED ON CACHE INTERNAL "" FORCE)
	endif()
	set(SDL_LIBC ON CACHE BOOL "" FORCE)
	set(SDL_TESTS OFF CACHE INTERNAL "")
    set(SDL_TEST_LIBRARY OFF CACHE INTERNAL "")
	add_subdirectory("${DEPS_DIR}/SDL" EXCLUDE_FROM_ALL)
		if(SDL_STATIC)
			set(SDL2STATIC_LIB 	"SDL3-static")
		else()
			set(SDL2STATIC_LIB 	"SDL3-shared")
		endif()
endif()
if (ANDROID)
	# SDL android is hardcoded to load "SDL3.so" with no "d" postfix
	set_target_properties(SDL3-shared PROPERTIES DEBUG_POSTFIX "")
	# we get a linker error without this
	target_link_libraries(SDL3-shared PUBLIC camera2ndk mediandk)
endif()

# if on a platform other than windows or mac, ensure that an audio backend was found
if (LINUX)
	find_package(ALSA)
	find_package(PulseAudio)                                    
	if (NOT ALSA_FOUND AND NOT PulseAudio_FOUND)
		message(FATAL_ERROR "Either ALSA or PulseAudio dev packages required, but neither were found.")
	endif()
endif()

set(PHYSFS_BUILD_TEST OFF CACHE INTERNAL "")
set(PHYSFS_BUILD_STATIC ON CACHE INTERNAL "")
set(PHYSFS_BUILD_SHARED OFF CACHE INTERNAL "")
set(PHYSFS_BUILD_DOCS OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/physfs" EXCLUDE_FROM_ALL)

# ozz animation
set(ozz_build_samples OFF CACHE INTERNAL "")
set(ozz_build_howtos OFF CACHE INTERNAL "")
set(ozz_build_tests OFF CACHE INTERNAL "")
set(ozz_build_tools OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/ozz-animation" EXCLUDE_FROM_ALL)

# libnyquist
SET(BUILD_EXAMPLE OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/libnyquist" EXCLUDE_FROM_ALL)

# RavEngine Graphics Library (RGL)
if (NOT CMAKE_CROSSCOMPILING AND NOT RAVENGINE_SERVER)
	set(RGL_ENABLE_RGLC ON CACHE INTERNAL "")
else()
	set(RGL_ENABLE_RGLC OFF CACHE INTERNAL "")
endif()
set(SPIRV_SKIP_TESTS ON CACHE INTERNAL "")
set(SPIRV_SKIP_EXECUTABLES ON CACHE INTERNAL "")
set(RGL_IDE_ROOT "RavEngine SDK/Libraries/RGL/")
if (NOT RAVENGINE_SERVER)
	add_subdirectory("${DEPS_DIR}/RGL")
	set(RGL_LIB "RGL")
endif()


if(CMAKE_CROSSCOMPILING)
	set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE INTERNAL "")	# host-tools will build protoc
else()
	set(protobuf_BUILD_PROTOC_BINARIES ON CACHE INTERNAL "")	# this instance will build protoc
endif()
add_subdirectory("${DEPS_DIR}/GameNetworkingSockets" EXCLUDE_FROM_ALL)
if (CMAKE_CROSSCOMPILING)
	if (NOT RAVENGINE_SERVER)
		set(test_rglc "${rglc_path}")
	endif()
	add_custom_target("GNS_Deps" DEPENDS "${PROTOC_CMD}" "${test_rglc}" "flatc")
else()
	if (NOT RAVENGINE_SERVER)
		set(test_rglc "rglc")
		add_custom_target("GNS_Deps" DEPENDS "${test_rglc}" "protoc" "flatc")
	else()
		add_custom_target("GNS_Deps" DEPENDS "${test_rglc}" "protoc")
	endif()
endif()
add_dependencies("GameNetworkingSockets_s" "GNS_Deps")

# taskflow
SET(TF_BUILD_BENCHMARKS OFF CACHE INTERNAL "" )
SET(TF_BUILD_CUDA OFF CACHE INTERNAL "")
SET(TF_BUILD_TESTS OFF CACHE INTERNAL "")
SET(TF_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/taskflow" EXCLUDE_FROM_ALL)

# assimp
SET(IGNORE_GIT_HASH ON CACHE INTERNAL "")
SET(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "")
set(ASSIMP_INSTALL OFF CACHEN INTERNAL "")
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/assimp" EXCLUDE_FROM_ALL)

# steam audio
if (NOT RAVENGINE_SERVER)
	set(SA_BUILD_ZLIB OFF CACHE INTERNAL "")
	set(ZLIB_LIBRARY zlibstatic CACHE INTERNAL "")
	set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/assimp/contrib/zlib" CACHE INTERNAL "")
	set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB OFF CACHE INTERNAL "")
	if (CMAKE_CROSSCOMPILING)
		set(SA_BUILD_FLATC OFF CACHE INTERNAL "")
	endif()
	add_subdirectory(deps/SteamAudio-All EXCLUDE_FROM_ALL)
	set(SA_LIB "phonon")
	target_link_libraries(mysofa-static PRIVATE zlibstatic)
	target_include_directories(mysofa-static PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp/contrib/zlib/")

	# resonance-audio
	set(BUILD_RESONANCE_AUDIO_API ON CACHE INTERNAL "")
	add_subdirectory("${DEPS_DIR}/resonance-audio" EXCLUDE_FROM_ALL)
	set(RESONANCE_LIB ResonanceAudioObj)
	set(SADIE_LIB SadieHrtfsObj)
endif()

# recast
SET(RECASTNAVIGATION_DEMO OFF CACHE INTERNAL "")
SET(RECASTNAVIGATION_TESTS OFF CACHE INTERNAL "")
SET(RECASTNAVIGATION_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory("${DEPS_DIR}/recastnavigation" EXCLUDE_FROM_ALL)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
set(CMAKE_CXX_STANDARD 17)	# workaround g++ issue with C++20 and PhysX
else()
set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# PhysX-specific CMake project setup
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "Use the debug version of the CRT")
set(PHYSX_ROOT_DIR ${DEPS_DIR}/physx/physx CACHE INTERNAL "")
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared CACHE INTERNAL "")
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")
set(PX_PHYSX_ ${CMAKE_INSTALL_PREFIX} CACHE INTERNAL "")
set(CMAKEMODULES_VERSION "1.27" CACHE INTERNAL "")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules CACHE INTERNAL "")
set(PX_OUTPUT_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/physx/output_lib/$<CONFIGURATION>" CACHE INTERNAL "")
set(PX_OUTPUT_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/deps/physx/output_bin/$<CONFIGURATION>" CACHE INTERNAL "")
set(PX_GENERATE_STATIC_LIBRARIES ON CACHE INTERNAL "")
set(GPU_LIB_COPIED ON CACHE INTERNAL "")
#set(PX_FLOAT_POINT_PRECISE_MATH OFF)
if(EMSCRIPTEN)
	set(TARGET_BUILD_PLATFORM "linux" CACHE INTERNAL "")
	set(PLATFORM "Linux" CACHE INTERNAL "")
elseif (WIN32 OR WINDOWS_STORE)
	set(TARGET_BUILD_PLATFORM "windows" CACHE INTERNAL "")
	if (WINDOWS_STORE)
		set(PLATFORM "uwp")
	elseif(WIN32)
		set(PLATFORM "Windows")
	endif()
elseif(APPLE)
	set(TARGET_BUILD_PLATFORM "linux" CACHE INTERNAL "")
	set(PLATFORM "macOS")
	if (CMAKE_SYSTEM_NAME MATCHES visionOS)
		set(CMAKE_SYSTEM_PROCESSOR "aarch64")
	endif()
elseif(LINUX)
	set(TARGET_BUILD_PLATFORM "linux" CACHE INTERNAL "")
	set(CMAKE_LIBRARY_ARCHITECTURE "x86_64-linux-gnu" CACHE INTERNAL "")
	set(PLATFORM "Linux")
	#set(CMAKE_LIBRARY_ARCHITECTURE "aarch64-linux-gnu" CACHE INTERNAL "")
elseif(ANDROID)
	set(TARGET_BUILD_PLATFORM "linux" CACHE INTERNAL "")
	set(PLATFORM "Linux")
endif()

# Call into PhysX's CMake scripts
add_subdirectory("${PHYSX_ROOT_DIR}/compiler/public" EXCLUDE_FROM_ALL)
if(EMSCRIPTEN OR ( (WIN32 OR WINDOWS_STORE) AND CMAKE_C_COMPILER_ARCHITECTURE_ID MATCHES "ARM64"))
	# disable vectorization
	target_compile_definitions(LowLevelAABB PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(SceneQuery PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(SimulationController PRIVATE "PX_SIMD_DISABLED" "DISABLE_CUDA_PHYSX")
	target_compile_definitions(PhysXExtensions PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(PhysXVehicle PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(PhysXCommon PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(PhysX PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(PhysXFoundation PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(LowLevel PRIVATE "PX_SIMD_DISABLED" "DISABLE_CUDA_PHYSX")
	target_compile_definitions(PhysXCooking PRIVATE "PX_SIMD_DISABLED")
	target_compile_definitions(PhysXCharacterKinematic PRIVATE "PX_SIMD_DISABLED")

	# endianness checks
	target_compile_definitions(libnyquist PUBLIC "ARCH_CPU_LITTLE_ENDIAN")
	target_compile_definitions("physfs-static" PUBLIC "MY_CPU_LE")
endif()
if(ANDROID)
	# without this x86 32-bit build fails
	target_compile_options(LowLevel PUBLIC "-malign-double")
	target_compile_options(LowLevelAABB PUBLIC "-malign-double")
	target_compile_options(LowLevelDynamics PUBLIC "-malign-double")
	target_compile_options(PhysX PUBLIC "-malign-double")
endif()

# OpenXR - available on Windows only
if(WIN32 AND NOT RAVENGINE_SERVER)
	set(DYNAMIC_LOADER OFF)
	set(BUILD_TESTS OFF)
	set(BUILD_CONFORMANCE_TESTS OFF)
	set(BUILD_WITH_SYSTEM_JSONCPP OFF)
	add_subdirectory(deps/OpenXR-SDK)
	set(OPENXR_LOADER openxr_loader)
endif()

if(LINUX)
	set(LibUUID_LIBRARIES "uuid")
elseif(ANDROID)
	add_subdirectory(deps/android-uuid)
	set(LibUUID_LIBRARIES "uuid-android")
endif()

# ========== Building engine ==============

# get all sources for the library with glob
if(APPLE)
	# also need to compile Objective-C++ files
	file(GLOB MM_SOURCES "src/*.mm")
	set_source_files_properties(${MM_SOURCES} PROPERTIES
		COMPILE_FLAGS "-x objective-c++ "
	)
endif()
file(GLOB SOURCES "src/*.cpp" "src/*.hpp")
file(GLOB_RECURSE NATVIS "deps/*.natvis")
file(GLOB HEADERS "include/${PROJECT_NAME}/*.h" "include/${PROJECT_NAME}/*.hpp" )
file(GLOB SHADERS "shaders/*.glsl" "shaders/*.csh" "shaders/*.vsh" "shaders/*.fsh" "shaders/*.sc" "shaders/*.glsl" "shaders/*.hlsl")
set_source_files_properties(${SHADERS} PROPERTIES HEADER_FILE_ONLY TRUE)	# prevent VS from compiling these

# register the library
add_library("${PROJECT_NAME}" ${HEADERS} ${SOURCES} ${MM_SOURCES} ${NATVIS} ${SHADERS})
rve_enable_IPO(${PROJECT_NAME})

# set server define
if(RAVENGINE_SERVER)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "RVE_SERVER=1")
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC "RVE_SERVER=0")
endif()

# disable RTTI
# target_compile_definitions(${PROJECT_NAME} PUBLIC "RMLUI_USE_CUSTOM_RTTI")
# if (NOT MSVC)
# 	target_compile_options(${PROJECT_NAME} PUBLIC "-fno-rtti")
# else()
# 	target_compile_options(${PROJECT_NAME} PUBLIC "/GR-")
# endif()

# disable the dllimport stuff in RMLUI
target_compile_definitions(${PROJECT_NAME} PUBLIC -DRMLUI_STATIC_LIB=1)
	
set_target_properties(${PROJECT_NAME} PROPERTIES
	XCODE_GENERATE_SCHEME ON
)
set_source_files_properties(${SHADERS} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "sourcecode.glsl")
source_group("Shaders" FILES ${SHADERS})

# vectorization
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
if(APPLE OR LINUX)
	target_compile_options("${PROJECT_NAME}" PUBLIC -ffast-math -ffp-contract=fast)
endif()

if (NOT APPLE AND NOT WINDOWS_STORE)
target_precompile_headers("${PROJECT_NAME}" PRIVATE 
	"<phmap.h>"
	"<vector>"
	"<algorithm>"
	"<functional>"
	"<thread>"
	"<atomic>"
	"<memory>"
	"<RavEngine/CTTI.hpp>"
	"<optional>"
	"<concurrentqueue.h>"
	"<mutex>"
	"<chrono>"
	"<plf_list.h>"
	"<array>"
	"<string>"
	"<tuple>"
)
endif()

# include paths
target_include_directories("${PROJECT_NAME}" 
	PUBLIC 
	"include/"
	"shaders/"
	"${DEPS_DIR}/physx/physx/include/" 
	"${DEPS_DIR}/physx/pxshared/include/" 
	"${DEPS_DIR}/physx/physx/snippets/"
	"include/${PROJECT_NAME}/stduuid/"
	"${DEPS_DIR}/physfs/src"
	"${DEPS_DIR}/plf/"
	"${DEPS_DIR}/parallel-hashmap/parallel_hashmap"
	"${DEPS_DIR}/taskflow"
	"${DEPS_DIR}/RmlUi-freetype/RmlUi/Include"
	"${DEPS_DIR}/resonance-audio/resonance_audio/"
	"${DEPS_DIR}/resonance-audio/platforms/"
	"${DEPS_DIR}/resonance-audio/third_party/eigen"
	"${DEPS_DIR}/resonance-audio/"
	"${DEPS_DIR}/GameNetworkingSockets/GameNetworkingSockets/include"
	"${DEPS_DIR}/date/include"
	PRIVATE
	"include/${PROJECT_NAME}/"
	"${DEPS_DIR}/miniz-cpp/"	
	"${DEPS_DIR}/stbi"
)

# ====================== Linking ====================
if (APPLE)
    # some apple-specific libraries
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	    find_library(SM_LIB ServiceManagement REQUIRED)
    endif()

    find_library(METAL_LIB Metal REQUIRED)
    find_library(QZC_LIB QuartzCore REQUIRED)
	find_library(ACCEL_LIB Accelerate REQUIRED)
    find_library(CORETEXT_LIB CoreText REQUIRED)    # visionOS needs this for some reason   

    SET(ICONV_LIB "iconv")

endif()

if(LINUX)
	set(ATOMIC_LIB "atomic") # need to explicitly link libatomic on linux
endif()

if(WIN32 OR WINDOWS_STORE)
	set(DXGI_LIB "dxgi.lib")
	set(WIN32_LIBS "Rpcrt4.lib")
endif()

target_link_libraries("${PROJECT_NAME}" 
    PRIVATE 
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
	${RESONANCE_LIB}
	${SADIE_LIB}
	#"PhysXGPU"
	${RML_LIB}
	"libnyquist"
	"GameNetworkingSockets_s"
	"r8brain"
	PUBLIC
	"dr_wav"
	"fmt"
	${SA_LIB}
	"effolkronium_random"
	"glm"
	"tweeny"
	${SDL2STATIC_LIB}
	${RGL_LIB}
	"Recast"
	"Detour"
	"DetourCrowd"
	"DebugUtils"
	"concurrentqueue"
	"ozz_animation"
	"ozz_animation_offline"
	"ozz_base"
	"ozz_geometry"
	"ozz_options"
	${WIN32_LIBS}
	${ICONV_LIB}
	${SM_LIB}
	${METAL_LIB}
    ${CORETEXT_LIB}
	${QZC_LIB}
    ${ATOMIC_LIB}
	${DXGI_LIB}
	${EM_LINK}
	${OPENXR_LOADER}
	${ACCEL_LIB}
	${LibUUID_LIBRARIES}
)

# raspberry pi needs this set explicitly, incompatible with other targets 
if(LINUX)
	target_link_libraries("${PROJECT_NAME}" PRIVATE "stdc++fs")
endif()

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

# globals for managing state
define_property(GLOBAL PROPERTY ALL_SHADERS
		BRIEF_DOCS "Aggregate shader list"
		FULL_DOCS "GLOBAL shader list"
	)
set_property(GLOBAL PROPERTY ALL_SHADERS "")
define_property(GLOBAL PROPERTY ALL_SHADER_SOURCES
	BRIEF_DOCS "Aggregate shader source list"
	FULL_DOCS "GLOBAL shader source list"
)
set_property(GLOBAL PROPERTY ALL_SHADER_SOURCES "")

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

# unity builds
macro(enable_unity targets)
	foreach(target ${targets})
		if(TARGET ${target})
			set_target_properties("${target}" PROPERTIES UNITY_BUILD ON)
		endif()
	endforeach()
endmacro()

set(all_unity 
"LowLevel;FastXml;SceneQuery;SimulationController;PhysXTask;PhysXCharacterKinematic;im3d;libnyquist;Detour;ozz_animation;ozz_animation_offline;\
ozz_animation_tools;ozz_base;ozz_geometry;ozz_options;json;libopus;DebugUtils;DetourCrowd;DetourTileCache;harfbuzz;"
)

if ((CMAKE_SYSTEM_NAME STREQUAL "Windows"))
	set(platform_unity "")	 
endif()

enable_unity("${all_unity}"
"${platform_unity}")

# project organization
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "RavEngine SDK")

group_in("Libraries" "assimp;assimp_cmd;DebugUtils;Detour;DetourCrowd;DetourTileCache;freetype;GameNetworkingSockets_s;GNS_Deps;\
im3d;libnyquist;libopus;libprotobuf;libprotobuf-lite;libwavpack;openssl;physfs;physfs-static;BUILD_FUSE_ALL;\
Recast;ResonanceAudioObj;ResonanceAudioShared;ResonanceAudioStatic;lunasvg;rlottie;rlottie-image-loader;RmlCore;ssl;\
test_physfs;tweeny-dummy;zlib;zlibstatic;SDL3-static;json;physfs_uninstall;dist;BUILD_CLANG_FORMAT;crypto;r8brain;harfbuzz;harfbuzz-subset;\
sdl_headers_copy;libprotoc;protoc;dr_wav;SadieHrtfsObj;
")

group_in("Libraries/PhysX SDK" 
"FastXml;LowLevel;LowLevelAABB;LowLevelDynamics;PhysX;PhysXCharacterKinematic;PhysXCommon;\
PhysXCooking;PhysXExtensions;PhysXFoundation;PhysXPvdSDK;PhysXTask;PhysXVehicle;SceneQuery;SimulationController;PhysXVehicle2"
)

group_in("Libraries/ozz" 
	"ozz_animation;ozz_animation_offline;ozz_base;ozz_geometry;ozz_options"
)
group_in("Libraries/ozz/tools" 
	"dump2ozz;gltf2ozz;ozz_animation_tools"
)
group_in("Libraries/ozz/fuse"
"BUILD_FUSE_ozz_animation;BUILD_FUSE_ozz_animation_offline;BUILD_FUSE_ozz_animation_tools;\
BUILD_FUSE_ozz_base;BUILD_FUSE_ozz_geometry;BUILD_FUSE_ozz_options"
)

group_in("Libraries/openxr" "openxr_loader" "generate_openxr_header" "xr_global_generated_files")

group_in("Libraries/SteamAudio"
"core;fbschemas;hrtf;phonon;flatc;PFFFT;mysofa-static"
)

macro(shader_compile infile stage api extension binary)
	if (NOT RAVENGINE_SERVER)
		set(bindir "${shader_target}_ShaderIntermediate")
		set(shader_inc_dir "${eng_dir}/shaders")

		get_filename_component(name_only ${infile} NAME)
		set(outname "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/${api}/${name_only}.${extension}")
		set(finalrootpath "${CMAKE_CURRENT_BINARY_DIR}/${bindir}/${api}/${name_only}")
		if (${api} MATCHES "Metal")
			STRING(REPLACE "." "_" name_fixed ${name_only} )
			set(entrypoint "--entrypoint" "${name_fixed}")
		endif()
		add_custom_command(
			PRE_BUILD
			OUTPUT "${outname}"
			DEPENDS ${infile} GNS_Deps "${eng_dir}/shaders/ravengine_shader.glsl" "${eng_dir}/shaders/ravengine_fsh.h" "${eng_dir}/shaders/ravengine_vsh.h" "${eng_dir}/shaders/ravengine_shader_defs.h" "${eng_dir}/shaders/lightingbindings_shared.h" "${eng_dir}/shaders/BRDF.glsl" "${eng_dir}/shaders/lighting_preamble.glsl"
			COMMAND ${rglc_path} -f "${infile}" -o "${outname}" --api ${api} --stage ${stage} --include "${shader_inc_dir}" --debug ${binary} ${entrypoint}
		)
	endif()
endmacro()

function(declare_shader infile shader_target)
	if (RAVENGINE_SERVER)
		return()
	endif()

	get_filename_component(shader_ext ${infile} EXT)

	if (shader_ext MATCHES "vsh")
		set(stage "vertex")
		set(dxc_profile "v")
	elseif (shader_ext MATCHES "fsh")
		set(stage "fragment")
		set(dxc_profile "p")
	elseif (shader_ext MATCHES "csh")
		set(stage "compute")
		set(dxc_profile "c")
	endif()

	if(RGL_VK_AVAILABLE)
		shader_compile("${infile}" "${stage}" "Vulkan" "spv" "")
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${outname})
	endif()
	if (RGL_WEBGPU_AVAILABLE)
		shader_compile("${infile}" "${stage}" "WebGPU" "wgsl" "")
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${outname})
	endif()
	if(RGL_DX12_AVAILABLE)
		shader_compile("${infile}" "${stage}" "Direct3D12" "hlsl" "")

		if (stage MATCHES "vertex")
			set(dxc_type "Vertex")
		elseif (stage MATCHES "fragment")
			set(dxc_type "Pixel")
		elseif (stage MATCHES "compute")
			set(dxc_type "Compute")
		endif()
		target_sources(${shader_target} PUBLIC ${outname})
		source_group("Generated" FILES "${outname}")
		set(dx_final_name "${finalrootpath}.cso")
		set_source_files_properties(${outname} PROPERTIES 
			GENERATED TRUE
			HEADER_FILE_ONLY ON
			VS_SHADER_MODEL "6.4"
			VS_SHADER_TYPE ${dxc_type}
			VS_SHADER_ENABLE_DEBUG ON
			VS_SHADER_DISABLE_OPTIMIZATIONS ON
			# VS_SHADER_FLAGS "/Fd ${CMAKE_BINARY_DIR}/$<CONFIG>/"
			VS_SHADER_OBJECT_FILE_NAME ${dx_final_name}
		)
		set_source_files_properties(dx_final_name PROPERTIES GENERATED TRUE)
		set_property(GLOBAL APPEND PROPERTY ALL_SHADERS ${dx_final_name})
		# manually invoke dxc.exe
		add_custom_command(
			PRE_BUILD 
			OUTPUT "${dx_final_name}"
			DEPENDS "${outname}"
			COMMAND dxc.exe $<$<CONFIG:Debug>:-Qembed_debug> $<$<CONFIG:Debug>:-Zi> $<$<CONFIG:Debug>:-Od> -Fo \"${dx_final_name}\" -T ${dxc_profile}s_6_4 \"${outname}\"
		)
	endif()
	if(RGL_MTL_AVAILABLE)
		shader_compile("${infile}" "${stage}" "Metal" "metal" "")
		set_source_files_properties(${outname} PROPERTIES 
			GENERATED TRUE
			HEADER_FILE_ONLY OFF
			LANGUAGE METAL
			XCODE_EXPLICIT_FILE_TYPE "sourcecode.metal"
            COMPILE_FLAGS "-gline-tables-only -frecord-sources" # enable shader source debugging
		)
			# goes into the bundle Metallib, so we don't need to include the shader in the asset package
		target_sources(${shader_target} PUBLIC ${outname})
		source_group("Generated" FILES "${outname}")
	endif()
endfunction()

# pack resources
function(pack_resources)
	set(optional )
	set(args TARGET OUTPUT_FILE STREAMING_INPUT_ROOT)
	set(list_args SHADERS OBJECTS TEXTURES UIS FONTS SOUNDS STREAMING_ASSETS)
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

	if(APPLE)
		set_property(TARGET ${ARGS_TARGET} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
		set_target_properties(${ARGS_TARGET} PROPERTIES 
			XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "$<$<NOT:$<CONFIG:RELEASE>>:INCLUDE_SOURCE>"	# enable shader debugging in all but release mode
			XCODE_ATTRIBUTE_MTL_OPTIMIZATION_LEVEL "$<$<CONFIG:RELEASE>:default>"				# optimize shaders in release builds
			XCODE_ATTRIBUTE_MTL_LANGUAGE_REVISION "Metal30"										# use Metal 3
			)
	endif()	

	get_property(eng_dir GLOBAL PROPERTY ENG_DIR)

	# add polygon primitives provided by engine
	file(GLOB ENG_OBJECTS "${eng_dir}/objects/*")

	# add engine-provided shaders
	file(GLOB ENG_SHADERS "${eng_dir}/shaders/*.vsh" "${eng_dir}/shaders/*.fsh" "${eng_dir}/shaders/*.csh")

	# add engine-provided fonts
	file(GLOB ENG_FONTS "${eng_dir}/fonts/*.ttf")

	file(GLOB ENG_UIS "${eng_dir}/ui/*.rcss" "${eng_dir}/ui/*.rml")

	# clear copy-depends
	set_property(GLOBAL PROPERTY COPY_DEPENDS "")

	function(copy_helper_impl FILE_LIST output_dir root_dir)
		foreach(FILE ${FILE_LIST})
			# copy objects pre-build if they are changed
			get_filename_component(output_name "${FILE}" NAME)
			set(outname "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}/${root_dir}/${output_name}")
			add_custom_command(PRE_BUILD 
				OUTPUT "${outname}" 
				COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} "${outname}"
				DEPENDS ${FILE}
				)
			set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS ${outname})
		endforeach()
	endfunction()

	# helper for copying to staging directory
	function(copy_helper FILE_LIST output_dir)
		copy_helper_impl("${FILE_LIST}" ${ARGS_TARGET} "${output_dir}")
	endfunction()

	function(copy_streaming_helper FILE_LIST output_root input_root)
		foreach(FILE ${FILE_LIST})
			if (APPLE)
				# put it in the resources bundle
				file(RELATIVE_PATH relpath "${input_root}" "${FILE}")
				get_filename_component(relpath "${relpath}" DIRECTORY)
				target_sources(${ARGS_TARGET} PRIVATE ${FILE})
				set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${ARGS_TARGET}_Streaming/${relpath}")
			elseif(NOT EMSCRIPTEN)
				# put it next to the executable
				file(RELATIVE_PATH relpath "${input_root}" "${FILE}")
				if (WINDOWS_STORE)
					set(UWP_APPX "AppX")
				endif()
				set(asset_relpath "${ARGS_TARGET}_Streaming/${output_root}/${relpath}")
				set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${UWP_APPX}/${asset_relpath}")
				add_custom_command(PRE_BUILD 
					OUTPUT "${outname}" 
					COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} "${outname}"
					DEPENDS ${FILE}
				)
				target_sources("${ARGS_TARGET}" PRIVATE "${FILE}")
				source_group("Streaming Assets" FILES ${FILE})
				set_source_files_properties(${FILE} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "archive.ar")
				set_property(SOURCE "${FILE}" PROPERTY VS_DEPLOYMENT_CONTENT 1)
				set_property(SOURCE "${FILE}" PROPERTY VS_DEPLOYMENT_LOCATION "${asset_relpath}")
				set_property(GLOBAL APPEND PROPERTY COPY_DEPENDS ${outname})
			endif()
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
	
	copy_streaming_helper("${ARGS_STREAMING_ASSETS}" "" "${ARGS_STREAMING_INPUT_ROOT}")

	target_sources(${ARGS_TARGET} PUBLIC ${ARGS_OBJECTS} ${ARGS_TEXTURES} ${ARGS_SOUNDS})
	set_source_files_properties(${ARGS_OBJECTS} ${ARGS_TEXTURES} ${ARGS_SOUNDS} PROPERTIES HEADER_FILE_ONLY ON)
	
	source_group("Objects" FILES ${ARGS_OBJECTS})
	source_group("Textures" FILES ${ARGS_TEXTURES})
	source_group("Sounds" FILES ${ARGS_SOUNDS})
	source_group("UI" FILES ${ARGS_UIS})
	source_group("Streaming" FILES ${ARGS_STREAMING_ASSETS})


	# get dependency outputs
	get_property(copy_depends GLOBAL PROPERTY COPY_DEPENDS)

	# clear global shaders property
	set_property(GLOBAL PROPERTY ALL_SHADERS "")

	# setup shader compiler
	foreach(SHADER ${ENG_SHADERS} ${ARGS_SHADERS})
		declare_shader("${SHADER}" "${ARGS_TARGET}")
	endforeach()

	set_property(GLOBAL PROPERTY ALL_SHADER_SOURCES "")
	foreach(SHADER ${ARGS_SHADERS})
		list(APPEND all_shader_sources "${SHADER}")
	endforeach()

	get_property(sc_comp_name GLOBAL PROPERTY SC_COMP_NAME)
	get_property(sc_include_dir GLOBAL PROPERTY SC_INCLUDE_DIR)

	#track all the shaders for compilation
	get_property(all_shaders_property GLOBAL PROPERTY ALL_SHADERS)
	add_custom_target("${ARGS_TARGET}_CompileShaders" ALL DEPENDS ${all_shaders_property})
	add_dependencies("${ARGS_TARGET}" "${ARGS_TARGET}_CompileShaders" "RavEngine")

	if(NOT APPLE)
		set(rve_cmrc_resource_name "${ARGS_TARGET}_ShaderResources")
		cmrc_add_resource_library("${rve_cmrc_resource_name}" WHENCE "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}_ShaderIntermediate/" ${all_shaders_property} )
		set_target_properties("${rve_cmrc_resource_name}" PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
		)
		target_link_libraries("${ARGS_TARGET}" PRIVATE "${rve_cmrc_resource_name}")
		set(cmrc_shader_libfn_cpp "${CMAKE_BINARY_DIR}/__cmrc_${rve_cmrc_resource_name}/lib_extern.cpp")
		file(GENERATE OUTPUT "${cmrc_shader_libfn_cpp}" CONTENT "
		#include <cmrc/cmrc.hpp>
		#include <span>
		#include <string_view>
		#include <string>
		CMRC_DECLARE(${rve_cmrc_resource_name});

		#define CMRC_NS cmrc::${rve_cmrc_resource_name}

		const std::span<const char> cmrc_get_file_data(const std::string_view& path){
			auto fs = CMRC_NS::get_filesystem();

			auto file = fs.open(std::string(path));


			return {file.begin(),file.size()};
		}
		")
		target_sources(${rve_cmrc_resource_name} PRIVATE "${cmrc_shader_libfn_cpp}")
		target_compile_features(${rve_cmrc_resource_name} PRIVATE cxx_std_20)
	endif()

	set(rve_externs_cpp "${CMAKE_BINARY_DIR}/${ARGS_TARGET}_ShaderIntermediate/externs.cpp")
	file(GENERATE OUTPUT "${rve_externs_cpp}" CONTENT "
	#include <string_view>
	const std::string_view RVE_VFS_get_name(){
		return \"${ARGS_TARGET}\";
	}
	")
	set(rve_externs_lib "${ARGS_TARGET}_Externs")
	add_library(${rve_externs_lib} "${rve_externs_cpp}")
	set_target_properties("${rve_externs_lib}" PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmrc/$<CONFIGURATION>/"
	)
	target_compile_features(${rve_externs_lib} PRIVATE cxx_std_20)
	target_link_libraries("${ARGS_TARGET}" PRIVATE "${rve_externs_lib}")

	set_target_properties(${rve_cmrc_resource_name} "${ARGS_TARGET}_CompileShaders" ${rve_externs_lib} PROPERTIES
		FOLDER "RavEngine Auxillary"
	)

	# add files to IDE sidebar for convenience
	target_sources("${ARGS_TARGET}" PUBLIC ${ARGS_UIS} ${all_shader_sources})
	set_source_files_properties(${ARGS_UIS} ${all_shader_sources} PROPERTIES HEADER_FILE_ONLY TRUE)	# prevents visual studio from trying to build these
	set_source_files_properties(${all_shader_sources} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "sourcecode.glsl")
	set_source_files_properties(${ARGS_UIS} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "text.html")
	set_source_files_properties(${ARGS_OBJECTS} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "compiled")
	set_source_files_properties(${ARGS_SOUNDS} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "audio.wav")
	source_group("Shaders" FILES ${all_shader_sources})

	set(outpack "${CMAKE_BINARY_DIR}/${ARGS_TARGET}.rvedata")

	# allow inserting into the mac / ios resource bundle
	set_target_properties(${ARGS_TARGET} PROPERTIES 
		MACOSX_BUNDLE TRUE
		#XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>,$<CONFIG:PROFILE>>:YES>
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
		XCODE_GENERATE_SCHEME ON	# create a scheme in Xcode
	)

	set(assets ${ARGS_OBJECTS} ${ENG_OBJECTS} ${ARGS_TEXTURES} ${copy_depends})

	# the command to pack into a zip
	add_custom_command(
		POST_BUILD 
		OUTPUT "${outpack}"
		DEPENDS ${assets}
		COMMENT "Packing resources for ${ARGS_TARGET}"
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${outpack}" --format=zip "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}"
		VERBATIM
	)

	# make part of the target, and add to the resources folder if applicable
	target_sources("${ARGS_TARGET}" PRIVATE "${outpack}")
	set_source_files_properties("${outpack}" PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources
	)
	source_group("Resources" FILES ${outpack})
	set_source_files_properties(${outpack} PROPERTIES XCODE_EXPLICIT_FILE_TYPE "archive.ar")

	# Set the assets zip location on UWP
	set_property(SOURCE "${outpack}" PROPERTY VS_DEPLOYMENT_CONTENT 1)
	set_property(SOURCE "${outpack}" PROPERTY VS_DEPLOYMENT_LOCATION "")	# tells the deployment to put the assets zip in the AppX root directory
	
	# copy to target dir on Win
	if((WIN32 AND NOT WINDOWS_STORE) OR LINUX)
		get_filename_component(outfile ${outpack} NAME)
		SET(outfile "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${outfile}")
		add_custom_command(
			POST_BUILD
			TARGET "${ARGS_TARGET}"
			DEPENDS "${outpack}"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${outpack}" "${outfile}"
			COMMENT "Copying assets package to executable directory"
		)
	endif()

	set(${ARGS_OUTPUT_FILE} ${outpack} CACHE INTERNAL "")
endfunction()

# tests
if (RAVENGINE_BUILD_TESTS)
	include(CTest)
	add_executable("${PROJECT_NAME}_TestBasics" EXCLUDE_FROM_ALL "test/basics.cpp")
	target_link_libraries("${PROJECT_NAME}_TestBasics" PUBLIC "RavEngine" )

	add_executable("${PROJECT_NAME}_DSPerf" EXCLUDE_FROM_ALL "test/dsperf.cpp")
	target_link_libraries("${PROJECT_NAME}_DSPerf" PUBLIC "RavEngine")

	target_compile_features("${PROJECT_NAME}_TestBasics" PRIVATE cxx_std_20)
	target_compile_features("${PROJECT_NAME}_DSPerf" PRIVATE cxx_std_20)

	set_target_properties("${PROJECT_NAME}_TestBasics" "${PROJECT_NAME}_DSPerf" PROPERTIES 
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
		XCODE_GENERATE_SCHEME ON	# create a scheme in Xcode
	)

	macro(test name executable)
	add_test(
		NAME ${name} 
		COMMAND ${executable} "${name}" -C $<CONFIGURATION> 
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>
	)
	endmacro()

	test("CTTI" "${PROJECT_NAME}_TestBasics")
	test("Test_UUID" "${PROJECT_NAME}_TestBasics")
    test("Test_AddDel" "${PROJECT_NAME}_TestBasics")
    test("Test_SpawnDestroy" "${PROJECT_NAME}_TestBasics")
    test("Test_MoveBetweenWorlds" "${PROJECT_NAME}_TestBasics")
endif()

# Disable unecessary build / install of targets
function(get_all_targets var)
    set(targets)
    get_all_targets_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
    set(${var} ${targets} PARENT_SCOPE)
endfunction()

macro(get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets} ${subdir})
    endforeach()

    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()

get_all_targets(all_targets)

if(WINDOWS_STORE)
	# WINNT version is messed up when compiling for UWP, fixes here
	target_compile_definitions("GameNetworkingSockets_s" PUBLIC "_CRT_SECURE_NO_WARNINGS" "BUILD_DLL" "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" "_CRT_NONSTDC_NO_DEPRECATE" "_WIN32_WINNT=9501")
endif()

# disable warnings in subdirectory targets
foreach(TGT ${all_targets})
	if(NOT "${TGT}" STREQUAL "${PROJECT_NAME}")
		get_target_property(target_type ${TGT} TYPE)

		# only run this command on compatible targets
		if (NOT ("${target_type}" STREQUAL "INTERFACE_LIBRARY" OR "${target_type}" STREQUAL "UTILITY"))
			if(MSVC)
				target_compile_options(${TGT} PRIVATE "/W0")
			else()
				target_compile_options(${TGT} PRIVATE "-w")
			endif()

			if (WINDOWS_STORE)
				target_compile_definitions(${TGT} PUBLIC "_CRT_SECURE_NO_WARNINGS" "BUILD_DLL" "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS" "_CRT_NONSTDC_NO_DEPRECATE")
			endif()

			#set_target_properties(${TGT} PROPERTIES
			#	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH $<$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>,$<CONFIG:PROFILE>>:YES>
			#)
		
		endif()
	endif()
endforeach()

