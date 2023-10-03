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
option(RGL_USE_MINIVKSDK "Use MiniVulkanSDK instead of the installed Vulkan SDK" OFF)
option(RGL_DISABLE_DX "Force-disable the D3D12 backend" OFF)
option(RGL_RUNTIME_COMPILATION "Enable runtime shader compilation" OFF)

if (RGL_RUNTIME_COMPILATION)
set(RGL_RUNTIME_COMPILER_LIB "librglc")
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
set(ST_BUNDLED_DXC OFF CACHE INTERNAL "")
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
	if(MSVC)
		set(rglc_ext ".exe")
	endif()
	set(rglc_path "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/rglc${rglc_ext}" CACHE INTERNAL "rglc compiler path")
	set(rglc_exe "rglc")
endif()

if(NOT RGL_USE_MINIVKSDK)
	find_package(Vulkan)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "WindowsStore" OR RGL_DISABLE_VK OR APPLE OR CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(VK_FORCE_DISABLED ON)
else()
	set(VK_FORCE_DISABLED OFF)
endif()

if (NOT VK_FORCE_DISABLED AND (RGL_USE_MINIVKSDK OR Vulkan_FOUND))
	if (RGL_USE_MINIVKSDK)
		add_subdirectory(deps/MiniVulkanSDK EXCLUDE_FROM_ALL)
		set(VK_LIBS MiniVulkanSDK VulkanMemoryAllocator)
		set(VK_INCLUDE deps/VulkanMemoryAllocator/include)
	else()
		set(VK_LIBS ${Vulkan_LIBRARIES} VulkanMemoryAllocator)
		set(VK_INCLUDE ${Vulkan_INCLUDE_DIRS} deps/VulkanMemoryAllocator/include)
	endif()

	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_VK_AVAILABLE=1)
	set(RGL_VK_AVAILABLE ON CACHE INTERNAL "RGL Vulkan")
	set(VMA_STATIC_VULKAN_FUNCTIONS ON)
	add_subdirectory(deps/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
	target_include_directories(VulkanMemoryAllocator PUBLIC ${Vulkan_INCLUDE_DIRS})
	target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS}) # so that library users get the vk paths if applicable
    target_compile_features(VulkanMemoryAllocator PUBLIC cxx_std_20)
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_VK_AVAILABLE=0)
	set(RGL_VK_AVAILABLE OFF CACHE INTERNAL "RGL Vulkan")
endif()

if(MSVC AND NOT RGL_DISABLE_DX)
	add_subdirectory(deps/DirectX-Headers EXCLUDE_FROM_ALL)
	set(DX12_LIBRARIES "d3d12.lib" "DirectX-Headers" "DirectX-Guids" "dxgi.lib" "d3dcompiler.lib" "D3D12MemoryAllocator" "DirectXTK12")
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_DX12_AVAILABLE=1)

	set(RGL_DX12_AVAILABLE ON CACHE INTERNAL "RGL DX12")
	add_subdirectory("deps/D3D12MemoryAllocator" EXCLUDE_FROM_ALL)
	set(BUILD_XAUDIO_WIN10 OFF CACHE INTERNAL "")
	add_subdirectory("deps/DirectXTK12" EXCLUDE_FROM_ALL)

	set_target_properties("DirectX-Guids"  "DirectXTK12"  PROPERTIES 
		FOLDER "RGL/Libraries"
	)
	target_include_directories(${PROJECT_NAME} PUBLIC "deps/DirectXTK12/Inc")
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
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_MTL_AVAILABLE=0)
	set(RGL_MTL_AVAILABLE OFF CACHE INTERNAL "RGL Metal")
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
	set(RGL_WEBGPU_AVAILABLE ON CACHE INTERNAL "RGL WebGPU")
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_WEBGPU_AVAILABLE=1)
else()
	set(RGL_WEBGPU_AVAILABLE OFF CACHE INTERNAL "RGL WebGPU")
	target_compile_definitions(${PROJECT_NAME} PUBLIC RGL_WEBGPU_AVAILABLE=0)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
	${VK_LIBS}
	${DX12_LIBRARIES}
	${FOUNDATION_LIB}
	${METAL_LIB}
	${QZC_LIB}
	${X11_LIBRARIES}
    ${RGL_RUNTIME_COMPILER_LIB}
	SPIRV-Reflect
)

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
	FOLDER "RGL"
	XCODE_GENERATE_SCHEME ON
)

set_target_properties("SPIRV-Reflect" "ShaderTranspiler" "libtint" "tint_diagnostic_utils" PROPERTIES 
	FOLDER "RGL/Libraries"
)

set_target_properties("GenericCodeGen" "glslang" "MachineIndependent" "OGLCompiler" "OSDependent" "SPIRV" PROPERTIES 
	FOLDER "RGL/Libraries/glslang"
)


set_target_properties(
	"SPIRV-Tools-opt" "SPIRV-Tools-reduce" "SPIRV-Tools-static"
	PROPERTIES 
	FOLDER "RGL/Libraries/SPIRV-Tools Libraries"
)

set_target_properties(
	"spirv-cross-core"
	"spirv-cross-glsl" "spirv-cross-hlsl" "spirv-cross-msl" "spirv-cross-reflect"
	PROPERTIES 
	FOLDER "RGL/Libraries/SPIRV-Cross"
)
if(ST_BUNDLED_DXC)
set_target_properties(
	"DxcDisassembler" "DxcOptimizer" "DxilConstants" "DxilDocs" "DxilInstructions" "DxilIntrinsicTables"
	"DxilMetadata" "DxilOperations" "DxilPIXPasses" "DxilShaderModel" "DxilShaderModelInc" "DxilSigPoint"
	"DxilValidation" "DxilValidationInc" "HCTGen" "HLSLIntrinsicOp" "HLSLOptions" "RDAT_LibraryTypes"
	PROPERTIES 
	FOLDER "RGL/Libraries/DirectXShaderCompiler"
)
endif()

if (RGL_VK_AVAILABLE)
	set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER "RGL/Libraries")
	if (TARGET loader-opt)
		set_target_properties(loader-opt loader-unknown-chain PROPERTIES FOLDER "RGL/Libraries")
	endif()
	if(RGL_USE_MINIVKSDK)
		set_target_properties(vulkan vvl VkLayer_utils loader_asm_gen_files PROPERTIES FOLDER "RGL/Libraries")
	endif()
endif()
if (RGL_DX12_AVAILABLE)
	set_target_properties(D3D12MemoryAllocator PROPERTIES FOLDER "RGL/Libraries")
endif()



if (RGL_VK_AVAILABLE OR RGL_DX12_AVAILABLE OR RGL_MTL_AVAILABLE OR RGL_WEBGPU_AVAILABLE)
else()
message(FATAL_ERROR "No backends are enabled! Check that all required SDKs are installed.")
endif()
