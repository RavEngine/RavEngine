cmake_minimum_required(VERSION 3.17)
set(CMAKE_XCODE_GENERATE_SCHEME OFF CACHE INTERNAL "")

PROJECT(ShaderTranspiler)
option(ST_ENABLE_TEST "Enable tests" ON)
option(ST_BUNDLED_DXC "Use the bundled DirectXShaderCompiler (required for cross-platform DXIL)" OFF)
option(ST_ENABLE_WGSL "Enable WGSL output" OFF)

SET(ST_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")

# dependencies
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE INTERNAL "")
set(SPIRV_SKIP_EXECUTABLES ON CACHE INTERNAL "")
set(SPIRV_SKIP_TESTS ON CACHE INTERNAL "")
set(SPIRV_WERROR OFF CACHE INTERNAL "")
add_subdirectory(deps/SPIRV-Tools EXCLUDE_FROM_ALL)

set(SKIP_GLSLANG_INSTALL ON CACHE INTERNAL "")
set(BUILD_EXTERNAL OFF CACHE INTERNAL "")
set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "")
set(ENABLE_CTEST OFF CACHE INTERNAL "")
set(BUILD_EXTERNAL OFF CACHE INTERNAL "")
set(ENABLE_HLSL OFF CACHE INTERNAL "")
add_subdirectory(deps/glslang EXCLUDE_FROM_ALL)

set(SPIRV_CROSS_CLI OFF CACHE INTERNAL "")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE INTERNAL "")
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE INTERNAL "")
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE INTERNAL "")
add_subdirectory(deps/SPIRV-Cross EXCLUDE_FROM_ALL)

set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)
add_subdirectory(deps/SPIRV-Reflect EXCLUDE_FROM_ALL)

if (ST_ENABLE_WGSL)
    set(TINT_BUILD_WGSL_READER OFF CACHE INTERNAL "")
    set(TINT_BUILD_GLSL_WRITER OFF CACHE INTERNAL "")
    set(TINT_BUILD_HLSL_WRITER OFF CACHE INTERNAL "")
    set(TINT_BUILD_MSL_WRITER OFF CACHE INTERNAL "")
    set(TINT_BUILD_SPV_WRITER OFF CACHE INTERNAL "")
    set(TINT_BUILD_SAMPLES OFF CACHE INTERNAL "")
    set(TINT_BUILD_IR OFF CACHE INTERNAL "")
    set(TINT_BUILD_SPV_READER ON CACHE INTERNAL "")
    set(TINT_BUILD_SPV_WRITER OFF CACHE INTERNAL "")
    set(TINT_BUILD_WGSL_WRITER ON CACHE INTERNAL "")
    function(tint_default_compile_options TARGET)   
        # stub this out because we don't want it
    endfunction()
    if (WIN32)
        set(TINT_OS_CC_SUFFIX "windows")
    elseif(UNIX)
        set(TINT_OS_CC_SUFFIX "posix")
    else()
        set(TINT_OS_CC_SUFFIX "other")
    endif()
    add_subdirectory(deps/tint/src/tint EXCLUDE_FROM_ALL)
    target_include_directories(tint_diagnostic_utils PUBLIC "${ST_DEPS_DIR}/tint")
    target_include_directories(libtint PUBLIC "${ST_DEPS_DIR}/tint/include")
    target_compile_features(tint_diagnostic_utils PUBLIC cxx_std_20)
    target_compile_features(libtint PUBLIC cxx_std_20)
    target_compile_definitions(libtint PUBLIC TINT_BUILD_SPV_READER=1 TINT_BUILD_WGSL_WRITER=1)
    set(TINT_LIB "libtint")
endif()

if(ST_BUNDLED_DXC)
	include(deps/DirectXShaderCompiler/cmake/caches/PredefinedParams.cmake)
	set(LLVM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
    set(CLANG_INCLUDE_TESTS OFF CACHE BOOL "" FORCE) 
    set(HLSL_INCLUDE_TESTS OFF CACHE BOOL "" FORCE) 
    set(ENABLE_SPIRV_CODEGEN OFF CACHE BOOL "" FORCE)  
    set(SPIRV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(LLVM_TARGETS_TO_BUILD "None" CACHE INTERNAL "" FORCE)
    set(HLSL_BUILD_DXILCONV OFF CACHE INTERNAL "" FORCE)
    set(HLSL_DISABLE_SOURCE_GENERATION ON CACHE BOOL "" FORCE)
    set(LLVM_OPTIMIZED_TABLEGEN OFF CACHE BOOL "" FORCE)
    add_subdirectory(deps/DirectXShaderCompiler/projects/include/Tracing EXCLUDE_FROM_ALL) # needed for an ETW
    add_subdirectory(deps/DirectXShaderCompiler EXCLUDE_FROM_ALL)
    set(DXCLIBS "dxcompiler")
    set(ST_DXC_EXE_PATH "$<TARGET_FILE:dxc>" CACHE INTERNAL "")
    set_target_properties(dxcompiler 
        PROPERTIES
        DEBUG_POSTFIX ""
    )
    add_dependencies(dxcompiler dxildll)
    set_target_properties(dxildll PROPERTIES DEBUG_POSTFIX "")
else()
    set(DXCLIBS "dxcompiler.lib")
    set(ST_DXC_EXE_PATH "dxc.exe" CACHE INTERNAL "")
endif()

# make library
file(GLOB SOURCES "src/*.cpp" "include/ShaderTranspiler/*.hpp" "include/ShaderTranspiler/*.h")

if(APPLE)
	set(APPLEFILES "src/AppleFns.mm")
    find_library(FOUNDATION_LIB Foundation REQUIRED)
    add_definitions("-x objective-c++")
endif()

add_library("${PROJECT_NAME}" ${SOURCES} ${APPLEFILES})
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_20)
set_target_properties("${PROJECT_NAME}" PROPERTIES XCODE_GENERATE_SCHEME ON)
if (ST_ENABLE_WGSL)
    target_compile_definitions("${PROJECT_NAME}" PRIVATE "ST_ENABLE_WGSL=1")
else()
    target_compile_definitions("${PROJECT_NAME}" PRIVATE "ST_ENABLE_WGSL=0")
endif()

target_include_directories("${PROJECT_NAME}" PRIVATE 
    "include/ShaderTranspiler/"
)

target_link_libraries("${PROJECT_NAME}" PRIVATE 
    glslang
    SPIRV
    SPIRV-Tools-opt
    SPIRV-Tools-reduce
    spirv-cross-glsl
    spirv-cross-hlsl
    spirv-cross-msl
    spirv-cross-reflect
    SPIRV-Tools
    spirv-reflect-static
    ${TINT_LIB}
    ${FOUNDATION_LIB}
)
if(ST_BUNDLED_DXC)
    target_link_libraries("${PROJECT_NAME}" PRIVATE 
        dxclib
        dxcompiler
    )
    target_compile_definitions("${PROJECT_NAME}" PUBLIC -DST_BUNDLED_DXC=1)
    target_include_directories("${PROJECT_NAME}" PRIVATE
        PUBLIC "${ST_DEPS_DIR}/DirectXShaderCompiler/include"
    )
else()
    target_compile_definitions("${PROJECT_NAME}" PUBLIC -DST_BUNDLED_DXC=0)
    if(MSVC)
         target_link_libraries("${PROJECT_NAME}" PRIVATE 
            d3d11.lib
            ${DXCLIBS}
         )
    endif()
endif()

target_include_directories("${PROJECT_NAME}" 
    PUBLIC "include/"
    PUBLIC "${ST_DEPS_DIR}/glslang/glslang/" # TODO: figure out how to not need this   
)

if (ST_ENABLE_TEST)
    add_executable("${PROJECT_NAME}_test" "test/main.cpp")
    target_link_libraries("${PROJECT_NAME}_test" "${PROJECT_NAME}")
    set_target_properties("${PROJECT_NAME}_test" PROPERTIES XCODE_GENERATE_SCHEME ON)
    target_compile_features("${PROJECT_NAME}_test" PRIVATE cxx_std_17)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}_test")
endif()
