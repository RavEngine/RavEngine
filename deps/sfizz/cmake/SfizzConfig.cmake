include(CMakeDependentOption)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(GNUWarnings)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

# Export the compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Only install what's explicitely said
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Set C++ compatibility level
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND CMAKE_CXX_STANDARD LESS 17)
    set(CMAKE_CXX_STANDARD 17)
elseif((SFIZZ_LV2_UI OR SFIZZ_VST OR SFIZZ_AU OR SFIZZ_VST2) AND CMAKE_CXX_STANDARD LESS 17)
    # if the UI is part of the build, make it 17
    set(CMAKE_CXX_STANDARD 17)
endif()

# Set build profiling options
if(SFIZZ_PROFILE_BUILD)
    check_c_compiler_flag("-ftime-trace" SFIZZ_HAVE_CFLAG_FTIME_TRACE)
    check_cxx_compiler_flag("-ftime-trace" SFIZZ_HAVE_CXXFLAG_FTIME_TRACE)
    if(SFIZZ_HAVE_CFLAG_FTIME_TRACE)
        add_compile_options("$<$<COMPILE_LANGUAGE:C>:-ftime-trace>")
    endif()
    if(SFIZZ_HAVE_CXXFLAG_FTIME_TRACE)
        add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-ftime-trace>")
    endif()
endif()

# Process sources as UTF-8
if(MSVC)
    add_compile_options("/utf-8")
endif()

# Set Windows compatibility level to 7
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x601)
endif()

# Define the math constants everywhere
if(WIN32)
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

# Set macOS compatibility level
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")
endif()

# If using C++17, check if aligned-new has runtime support on the platform;
# on macOS, this depends on the deployment target.
if(CMAKE_CXX_STANDARD LESS 17)
    # not necessary on older C++, it will call ordinary new and delete
    set(SFIZZ_IMPLEMENT_CXX17_ALIGNED_NEW_SUPPORT FALSE)
else()
    check_cxx_source_compiles("
struct Test { alignas(1024) int z; };
int main() { new Test; return 0; }
" SFIZZ_HAVE_CXX17_ALIGNED_NEW)
    # if support is absent, sfizz will provide a substitute implementation
    if(SFIZZ_HAVE_CXX17_ALIGNED_NEW)
        set(SFIZZ_IMPLEMENT_CXX17_ALIGNED_NEW_SUPPORT FALSE)
    else()
        # on macOS, this mandatory flag tells that allocation functions are user-provided
        check_cxx_compiler_flag("-faligned-allocation" SFIZZ_HAVE_CXXFLAG_FALIGNED_ALLOCATION)
        if(SFIZZ_HAVE_CXXFLAG_FALIGNED_ALLOCATION)
            add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-faligned-allocation>")
        endif()
        set(SFIZZ_IMPLEMENT_CXX17_ALIGNED_NEW_SUPPORT TRUE)
    endif()
endif()

# Do not define macros `min` and `max`
if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

# The variable CMAKE_SYSTEM_PROCESSOR is incorrect on Visual studio...
# see https://gitlab.kitware.com/cmake/cmake/issues/15170

if(NOT SFIZZ_SYSTEM_PROCESSOR)
    if(MSVC)
        set(SFIZZ_SYSTEM_PROCESSOR "${MSVC_CXX_ARCHITECTURE_ID}")
    else()
        set(SFIZZ_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

# Add required flags for the builds
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    gw_warn(-Wall -Wextra -Wno-multichar -Werror=return-type)
    if(SFIZZ_SYSTEM_PROCESSOR MATCHES "^(i.86|x86_64)$")
        add_compile_options(-msse2 -mavx)
    elseif(SFIZZ_SYSTEM_PROCESSOR MATCHES "^(arm.*)$")
        add_compile_options(-mfpu=neon -mavx)
        if(NOT ANDROID)
            add_compile_options(-mfloat-abi=hard)
        endif()
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/Zc:__cplusplus)
    #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

function(sfizz_enable_fast_math NAME)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options("${NAME}" PRIVATE "-ffast-math")
    elseif(MSVC)
        target_compile_options("${NAME}" PRIVATE "/fp:fast")
    endif()
endfunction()

# If we build with Clang, optionally use libc++. Enabled by default on Apple OS.
cmake_dependent_option(USE_LIBCPP "Use libc++ with clang" "${APPLE}"
    "CMAKE_CXX_COMPILER_ID MATCHES Clang" OFF)
if(USE_LIBCPP)
    add_compile_options(-stdlib=libc++)
    # Presumably need the above for linking too, maybe other options missing as well
    add_link_options(-stdlib=libc++)   # New command on CMake master, not in 3.12 release
    add_link_options(-lc++abi)   # New command on CMake master, not in 3.12 release
endif()

set(SFIZZ_REPOSITORY https://github.com/sfztools/sfizz)
include(GNUInstallDirs)

# Don't show build information when building a different project
function(show_build_info_if_needed)
    if(CMAKE_PROJECT_NAME STREQUAL "sfizz")
        message(STATUS "
Project name:                  ${PROJECT_NAME}
Build type:                    ${CMAKE_BUILD_TYPE}
Build processor:               ${SFIZZ_SYSTEM_PROCESSOR}
Build using LTO:               ${ENABLE_LTO}
Build as shared library:       ${SFIZZ_SHARED}
Build JACK stand-alone client: ${SFIZZ_JACK}
Build render client:           ${SFIZZ_RENDER}
Build LV2 plug-in:             ${SFIZZ_LV2}
Build LV2 user interface:      ${SFIZZ_LV2_UI}
Build VST plug-in:             ${SFIZZ_VST}
Build AU plug-in:              ${SFIZZ_AU}
Build benchmarks:              ${SFIZZ_BENCHMARKS}
Build tests:                   ${SFIZZ_TESTS}
Build demos:                   ${SFIZZ_DEMOS}
Build devtools:                ${SFIZZ_DEVTOOLS}
Use sndfile:                   ${SFIZZ_USE_SNDFILE}
Use vcpkg:                     ${SFIZZ_USE_VCPKG}
Statically link dependencies:  ${SFIZZ_STATIC_DEPENDENCIES}
Use clang libc++:              ${USE_LIBCPP}
Release asserts:               ${SFIZZ_RELEASE_ASSERTS}

Install prefix:                ${CMAKE_INSTALL_PREFIX}
LV2 destination directory:     ${LV2PLUGIN_INSTALL_DIR}
LV2 plugin-side CC automation  ${SFIZZ_LV2_PSA}

Compiler CXX debug flags:      ${CMAKE_CXX_FLAGS_DEBUG}
Compiler CXX release flags:    ${CMAKE_CXX_FLAGS_RELEASE}
Compiler CXX min size flags:   ${CMAKE_CXX_FLAGS_MINSIZEREL}
")
    endif()
endfunction()
