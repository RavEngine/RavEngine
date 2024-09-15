cmake_minimum_required(VERSION 3.22)
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
		set(RVESC_PATH "${TOOLS_DIR}/RVESC/rvesc" CACHE INTERNAL "")
	else()
		set(PROTOC_CMD "${TOOLS_DIR}/protobuf/Release/protoc" CACHE INTERNAL "")
		set(rglc_path "${TOOLS_DIR}/RGL/Release/rglc${rglc_ext}" CACHE INTERNAL "")
  		set(FlatBuffers_EXECUTABLE "${TOOLS_DIR}/flatc/Release/flatc")
		 set(RVESC_PATH "${TOOLS_DIR}/RVESC/Release/rvesc" CACHE INTERNAL "")
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
		set(RVESC_PATH "${RVESC_PATH}.exe" CACHE INTERNAL "")
	endif()

	add_custom_command(
		PRE_BUILD
		OUTPUT "${PROTOC_CMD}" "${rglc_path}" "${FlatBuffers_EXECUTABLE}" "${RVESC_PATH}"
		COMMAND ${CMAKE_COMMAND} --build . --config Release --target protoc rglc flatc rvesc
		WORKING_DIRECTORY "${TOOLS_DIR}"
		VERBATIM
	)

	add_custom_target(flatc DEPENDS "${FlatBuffers_EXECUTABLE}")

	add_custom_target(rvesc DEPENDS "${RVESC_PATH}" flatc)
else()
	set(TOOLS_DIR ${CMAKE_CURRENT_BINARY_DIR}/host-tools CACHE INTERNAL "")
	set(PROTOC_CMD "protoc" CACHE INTERNAL "")
endif()

if (NOT RAVENGINE_SERVER)
	set(DISABLE_RTTI_AND_EXCEPTIONS OFF CACHE INTERNAL "")
	add_subdirectory(deps/RmlUi-freetype EXCLUDE_FROM_ALL)
	set(RML_LIB "RmlCore")
endif()

if (ANDROID)
	set(ANDROID_FUNCTION_LEVEL_LINKING OFF CACHE INTERNAL "")
endif()

# no extra flags required
add_subdirectory(deps/im3d-cmake EXCLUDE_FROM_ALL)
add_subdirectory(deps/tweeny EXCLUDE_FROM_ALL)
add_subdirectory(deps/concurrentqueue EXCLUDE_FROM_ALL)
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_subdirectory(deps/r8brain-cmake EXCLUDE_FROM_ALL)
add_subdirectory(deps/dr_wav EXCLUDE_FROM_ALL)
add_subdirectory(deps/fmt EXCLUDE_FROM_ALL)
add_subdirectory(deps/simdjson EXCLUDE_FROM_ALL)
add_subdirectory(deps/stbi EXCLUDE_FROM_ALL)

# randoms
set(Random_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(deps/random EXCLUDE_FROM_ALL)

set(CXXOPTS_BUILD_TESTS OFF CACHE INTERNAL "")
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(CXXOPTS_ENABLE_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(deps/cxxopts EXCLUDE_FROM_ALL)

# tracy
set(TRACY_ENABLE $<IF:$<CONFIG:profile>,ON,OFF> CACHE INTERNAL "")
add_subdirectory(deps/tracy EXCLUDE_FROM_ALL)


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
	set(SDL_REVISION "RVE Vendored SDL" CACHE INTERNAL "") # this prevents re-configures every time a git change occurs: https://github.com/libsdl-org/SDL/issues/9998
	add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)
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
add_subdirectory(deps/physfs EXCLUDE_FROM_ALL)

# ozz animation
set(ozz_build_samples OFF CACHE INTERNAL "")
set(ozz_build_howtos OFF CACHE INTERNAL "")
set(ozz_build_tests OFF CACHE INTERNAL "")
set(ozz_build_tools OFF CACHE INTERNAL "")
add_subdirectory(deps/ozz-animation EXCLUDE_FROM_ALL)

# libnyquist
SET(BUILD_EXAMPLE OFF CACHE INTERNAL "")
add_subdirectory(deps/libnyquist EXCLUDE_FROM_ALL)

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
	add_subdirectory(deps/RGL)
	set(RGL_LIB "RGL")
endif()

if (NOT CMAKE_CROSSCOMPILING AND NOT RAVENGINE_SERVER)
	include(cmake/importers.cmake)
	include(cmake/rvesc.cmake)
	set(RVESC_PATH rvesc CACHE INTERNAL "")
	set_target_properties(rvesc
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/RVESC"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/RVESC"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/RVESC"
	)

	set(RVEMC_PATH rvemc CACHE INTERNAL "")
	set(RVESKC_PATH rveskc CACHE INTERNAL "")
	set(RVEAC_PATH rveac CACHE INTERNAL "")
else()
	#host tools configures it
endif()


if(CMAKE_CROSSCOMPILING)
	set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE INTERNAL "")	# host-tools will build protoc
else()
	set(protobuf_BUILD_PROTOC_BINARIES ON CACHE INTERNAL "")	# this instance will build protoc
endif()
add_subdirectory(deps/GameNetworkingSockets EXCLUDE_FROM_ALL)
if (CMAKE_CROSSCOMPILING)
	if (NOT RAVENGINE_SERVER)
		set(test_rglc "${rglc_path}")
	endif()
	add_custom_target("GNS_Deps" DEPENDS "${PROTOC_CMD}" "${test_rglc}" "flatc" )
else()
	if (NOT RAVENGINE_SERVER)
		set(test_rglc "rglc")
		add_custom_target("GNS_Deps" DEPENDS "${test_rglc}" "protoc" "flatc")
	else()
		add_custom_target("GNS_Deps" DEPENDS "${test_rglc}" "protoc")
	endif()
	set_target_properties(protoc
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/protoc"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/protoc"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/protoc"
	)
endif()
add_dependencies("GameNetworkingSockets_s" "GNS_Deps")

# taskflow
SET(TF_BUILD_BENCHMARKS OFF CACHE INTERNAL "" )
SET(TF_BUILD_CUDA OFF CACHE INTERNAL "")
SET(TF_BUILD_TESTS OFF CACHE INTERNAL "")
SET(TF_BUILD_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(deps/taskflow EXCLUDE_FROM_ALL)

# assimp
SET(IGNORE_GIT_HASH ON CACHE INTERNAL "")
SET(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "")
set(ASSIMP_INSTALL OFF CACHEN INTERNAL "")
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "")
set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "")
add_subdirectory(deps/assimp EXCLUDE_FROM_ALL)

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
	add_subdirectory(deps/resonance-audio EXCLUDE_FROM_ALL)
	set(RESONANCE_LIB ResonanceAudioObj)
	set(SADIE_LIB SadieHrtfsObj)
endif()

# recast
SET(RECASTNAVIGATION_DEMO OFF CACHE INTERNAL "")
SET(RECASTNAVIGATION_TESTS OFF CACHE INTERNAL "")
SET(RECASTNAVIGATION_EXAMPLES OFF CACHE INTERNAL "")
add_subdirectory(deps/recastnavigation EXCLUDE_FROM_ALL)

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
file(GLOB SHADERS
    "shaders/*.glsl" "shaders/*.csh" "shaders/*.vsh" "shaders/*.fsh"
    "materials/*.glsl" "materials/*.vsh" "materials/*.fsh" "materials/*.csh"
    "tools/rvesc/*.glsl" "tools/rvesc/*.vsh" "tools/rvesc/*.fsh" "tools/rvesc/*.csh"
)
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
target_compile_definitions(${PROJECT_NAME} PUBLIC -DRMLUI_STATIC_LIB=1 NOMINMAX=1)
	
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
	"im3d"
	"physfs-static"
	${RESONANCE_LIB}
	${SADIE_LIB}
	#"PhysXGPU"
	${RML_LIB}
	"libnyquist"
	"GameNetworkingSockets_s"
	"r8brain"
	$<$<CONFIG:profile>:TracyClient>
	stb_image
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

include(cmake/shaders.cmake)
include(cmake/pack_resources.cmake)


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
sdl_headers_copy;libprotoc;protoc;dr_wav;SadieHrtfsObj;fmt;simdjson;TracyClient;stb_image;
")

group_in("Tools" "rvesc;rvesc_resources;rvemc;rveskc;rve_importlib;rveac")

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

