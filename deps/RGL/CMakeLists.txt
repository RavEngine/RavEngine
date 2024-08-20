cmake_minimum_required(VERSION 3.17)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE INTERNAL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)

if(MSVC)
	add_definitions(/MP)
endif()

if (LINUX)
find_package(X11)
endif()

project(RGL)

option(RGL_ENABLE_RGLC "Enable the separate compiler executable" ON)
option(RGL_DISABLE_VK "Force-disable the Vulkan backend" OFF)
option(RGL_DISABLE_DX "Force-disable the D3D12 backend" OFF)
option(RGL_DISABLE_WEBGPU "Force-disable the WebGPU backend" OFF)
option(RGL_ENABLE_WEBGPU "Force-enable the WebGPU backend" OFF)
option(RGL_RUNTIME_COMPILATION "Enable runtime shader compilation" OFF)
option(RGL_SKIP_BACKEND_CHECK "Skip the SDK check" OFF)
option(RGL_IDE_ROOT "IDE parent folder for targets" "RGL")
if (NOT RGL_IDE_ROOT)
	set(RGL_IDE_ROOT "RGL") # CMake sets this to "OFF" for some insane reason
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten" AND NOT RGL_DISABLE_WEBGPU )
	set(RGL_WEBGPU_AVAILABLE ON CACHE INTERNAL "RGL WebGPU")
else()
	set(RGL_WEBGPU_AVAILABLE OFF CACHE INTERNAL "RGL WebGPU")
endif()

if (RGL_ENABLE_WEBGPU)
	set(RGL_WEBGPU_AVAILABLE ON CACHE INTERNAL "RGL WebGPU")
endif()

if (CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
	set(RGL_DISABLE_VK ON CACHE INTERNAL "")
endif()

#setup main library
if(APPLE)
	file(GLOB_RECURSE MM_SOURCES "src/*.mm")
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "include/*.hpp" "src/*.h" )
add_library(${PROJECT_NAME} ${SOURCES} ${MM_SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if (APPLE)
	set_source_files_properties(${MM_SOURCES} PROPERTIES
		COMPILE_FLAGS "-x objective-c++"
	)
endif()

# setup compiler
if (RGL_WEBGPU_AVAILABLE)
	set(ST_ENABLE_WGSL ON CACHE INTERNAL "")
else()
	set(ST_ENABLE_WGSL OFF CACHE INTERNAL "")
endif()
set(ST_ENABLE_TEST OFF CACHE INTERNAL "")
if((WINDOWS_STORE OR WIN32) AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/ShaderTranspiler/deps/DirectXShaderCompiler/CMakeLists.txt")
	message("Building DXC from source")
	set(ST_BUNDLED_DXC ON)
else()
	set(ST_BUNDLED_DXC OFF)
endif()
add_subdirectory(deps/ShaderTranspiler EXCLUDE_FROM_ALL)
file(GLOB_RECURSE LIBRGLC_SOURCES "tools/librglc/*.cpp" "tools/librglc/*.hpp")
add_library(librglc ${LIBRGLC_SOURCES}) 
target_link_libraries(librglc PRIVATE ShaderTranspiler)
target_compile_features(librglc PRIVATE cxx_std_20)
target_include_directories(librglc
	PRIVATE
		"tools/librglc/include/"
	PUBLIC
		"tools/librglc/include/"
)

# setup command line compiler utility
if (RGL_ENABLE_RGLC)
	file(GLOB RGLC_SOURCES "tools/rglc/*.cpp" "tools/rglc/*.hpp")
	add_executable(rglc ${RGLC_SOURCES})
	target_compile_features(rglc PRIVATE cxx_std_20)
	target_link_libraries(rglc PRIVATE librglc)
	target_include_directories(rglc PRIVATE "deps/cxxopts/include")
	set(rglc_path "rglc" CACHE INTERNAL "rglc compiler path")
	set(rglc_exe "rglc")
	set_target_properties(rglc
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/rglc"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/rglc"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/rglc"
	)
endif()

if(WINDOWS_STORE OR RGL_DISABLE_VK OR APPLE OR CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(VK_FORCE_DISABLED ON)
else()
	set(VK_FORCE_DISABLED OFF)
endif()

if (NOT VK_FORCE_DISABLED)
	set(VOLK_INCLUDES ON)
	if (WIN32 OR WINDOWS_STORE)
		set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_WIN32_KHR")
	elseif(LINUX) 
		set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_XLIB_KHR" "VK_USE_PLATFORM_WAYLAND_KHR")
	elseif(ANDROID)
		set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_ANDROID_KHR")
	endif()
	add_subdirectory(deps/volk EXCLUDE_FROM_ALL)
	target_compile_definitions(volk PUBLIC NOMINMAX=1)
		
	# Get our vendored headers 
	set(VK_INCLUDE deps/Vulkan-Headers/include deps/Vulkan-Utility-Libraries/include)

	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_VK_AVAILABLE=1 )
	set(RGL_VK_AVAILABLE ON CACHE INTERNAL "RGL Vulkan")
	add_subdirectory(deps/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
	target_compile_definitions(VulkanMemoryAllocator INTERFACE VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0)

	target_link_libraries(${PROJECT_NAME} 
	PUBLIC
		volk
	PRIVATE
		GPUOpen::VulkanMemoryAllocator
	)

	set_target_properties(VulkanMemoryAllocator volk PROPERTIES FOLDER "${RGL_IDE_ROOT}/Libraries")
	if (TARGET loader-opt)
		set_target_properties(loader-opt loader-unknown-chain PROPERTIES FOLDER "${RGL_IDE_ROOT}/Libraries")
	endif()
	if(RGL_USE_MINIVKSDK)
		set_target_properties(vulkan vvl VkLayer_utils loader_asm_gen_files PROPERTIES FOLDER "${RGL_IDE_ROOT}/Libraries")
	endif()
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_VK_AVAILABLE=0)
	set(RGL_VK_AVAILABLE OFF CACHE INTERNAL "RGL Vulkan")
endif()

if(MSVC AND NOT RGL_DISABLE_DX)
	add_subdirectory(deps/DirectX-Headers EXCLUDE_FROM_ALL)
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_DX12_AVAILABLE=1)

	set(RGL_DX12_AVAILABLE ON CACHE INTERNAL "RGL DX12")
	add_subdirectory("deps/D3D12MemoryAllocator" EXCLUDE_FROM_ALL)
	set(BUILD_XAUDIO_WIN10 OFF CACHE INTERNAL "")
	add_subdirectory("deps/DirectXTK12" EXCLUDE_FROM_ALL)

	set_target_properties("DirectX-Guids"  "DirectXTK12"  PROPERTIES 
		FOLDER "${RGL_IDE_ROOT}/Libraries"
	)
	target_include_directories(${PROJECT_NAME} PUBLIC "deps/DirectXTK12/Inc")

	# PIX stuff
	add_subdirectory(deps/PixEvents-cmake)

	set_target_properties("WinPixEventRuntime" "WinPixEventRuntime_ETW" "PixDecoder" PROPERTIES FOLDER "${RGL_IDE_ROOT}/Libraries/")

	target_link_libraries(${PROJECT_NAME} 
	PRIVATE
		d3d12.lib 
		DirectX-Headers
		DirectX-Guids
		dxgi.lib
		d3dcompiler.lib
		D3D12MemoryAllocator 
		DirectXTK12
		$<$<NOT:$<CONFIG:release>>:WinPixEventRuntime>
	)

	set_target_properties(D3D12MemoryAllocator PROPERTIES FOLDER "${RGL_IDE_ROOT}/Libraries")
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_DX12_AVAILABLE=0)
	set(RGL_DX12_AVAILABLE OFF CACHE INTERNAL "RGL DX12")
endif()

if(APPLE)
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_MTL_AVAILABLE=1)
	set(MTL_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/deps/metal-cpp" "${CMAKE_CURRENT_LIST_DIR}/deps/metal-cpp-extensions")
	find_library(FOUNDATION_LIB Cocoa REQUIRED)
    find_library(METAL_LIB Metal REQUIRED)
    find_library(QZC_LIB QuartzCore REQUIRED)
	set(RGL_MTL_AVAILABLE ON CACHE INTERNAL "RGL Metal")

	target_link_libraries(${PROJECT_NAME} 
		PRIVATE
		${FOUNDATION_LIB}
		${METAL_LIB}
		${QZC_LIB}
	)
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_MTL_AVAILABLE=0)
	set(RGL_MTL_AVAILABLE OFF CACHE INTERNAL "RGL Metal")
endif()

if (RGL_WEBGPU_AVAILABLE)
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_WEBGPU_AVAILABLE=1)
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_WEBGPU_AVAILABLE=0)
endif()

if (RGL_RUNTIME_COMPILATION)
	target_link_libraries(${PROJECT_NAME} PRIVATE librglc)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SPIRV-Reflect ${X11_LIBRARIES})

if (APPLE)
	target_include_directories(${PROJECT_NAME} PRIVATE "tools/librglc/include/")
	if (RGL_RUNTIME_COMPILATION)
		target_compile_definitions(${PROJECT_NAME} PRIVATE "RGL_CAN_RUNTIME_COMPILE")
	endif()
endif()

set(AFTERMATH_DIR "AftermathSDK")
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/${AFTERMATH_DIR}")
	target_include_directories(${PROJECT_NAME} PRIVATE "deps/${AFTERMATH_DIR}/include")
	file(GLOB AFTERMATH_LIBS "deps/${AFTERMATH_DIR}/lib/x64/*.lib")
	target_link_libraries(${PROJECT_NAME} PRIVATE ${AFTERMATH_LIBS})
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_AFTERMATH_AVAILABLE=1)
	message("Aftermath SDK enabled")
else()
	message("Aftermath SDK disabled")
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_AFTERMATH_AVAILABLE=0)
endif()

target_include_directories(${PROJECT_NAME} 
	PRIVATE
		${X11_INCLUDE_DIR}
		${MTL_INCLUDE}
		"include/RGL"
	PUBLIC
		${VK_INCLUDE}
		"${CMAKE_CURRENT_LIST_DIR}/deps/DirectX-Headers/include"
		"include"
)

set_target_properties("RGL" ${rglc_exe} "librglc" PROPERTIES 
	FOLDER "${RGL_IDE_ROOT}"
	XCODE_GENERATE_SCHEME ON
)
set_target_properties("SPIRV-Reflect" "ShaderTranspiler" PROPERTIES 
	FOLDER "${RGL_IDE_ROOT}/Libraries"
)
if (RGL_WEBGPU_AVAILABLE)
set_target_properties("libtint" "tint_diagnostic_utils" PROPERTIES 
	FOLDER "${RGL_IDE_ROOT}/Libraries"
)
endif()

set_target_properties("GenericCodeGen" "glslang" "MachineIndependent" "OSDependent" "SPIRV" "SPVRemapper" "glslang-default-resource-limits" PROPERTIES 
	FOLDER "${RGL_IDE_ROOT}/Libraries/glslang"
)

set_target_properties(
	"SPIRV-Tools-opt" "SPIRV-Tools-reduce" "SPIRV-Tools-static" "SPIRV-Tools-shared" "SPIRV-Tools-lint" "SPIRV-Tools-diff" "SPIRV-Tools-link"
	"spirv-tools-shared-pkg-config" "spirv-tools-pkg-config"
	PROPERTIES 
	FOLDER "${RGL_IDE_ROOT}/Libraries/SPIRV-Tools Libraries"
)

set_target_properties(
	"spirv-cross-core"
	"spirv-cross-glsl" "spirv-cross-hlsl" "spirv-cross-msl" "spirv-cross-reflect" "spirv-cross-cpp" "spirv-cross-util"
	PROPERTIES 
	FOLDER "${RGL_IDE_ROOT}/Libraries/SPIRV-Cross"
)

if(ST_BUNDLED_DXC)
	function(dxc_set_for_dir DIR)
		get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
		set_target_properties(
			${TGTS}
			PROPERTIES 
			FOLDER "${RGL_IDE_ROOT}/Libraries/DirectXShaderCompiler"
		)

		get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
		foreach(SUBDIR IN LISTS SUBDIRS)
			dxc_set_for_dir("${SUBDIR}")
		endforeach()
	endfunction()
	dxc_set_for_dir("${CMAKE_CURRENT_LIST_DIR}/deps/ShaderTranspiler/deps/DirectXShaderCompiler")
endif()

if (NOT RGL_SKIP_BACKEND_CHECK)
	if (RGL_VK_AVAILABLE OR RGL_DX12_AVAILABLE OR RGL_MTL_AVAILABLE OR RGL_WEBGPU_AVAILABLE)
	else()
		message(FATAL_ERROR "No backends are enabled! Check that all required SDKs are installed.")
	endif()
endif()
