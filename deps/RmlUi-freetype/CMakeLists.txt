cmake_minimum_required(VERSION 3.17)

project(RmlUi VERSION 4.2) 

set(RMLUI_VERSION_RELEASE TRUE)
set(ENABLE_LOTTIE_PLUGIN ON CACHE INTERNAL "") # rlottie plugin

if(RMLUI_VERSION_RELEASE)
	set(RMLUI_VERSION_SUFFIX "")
else()
	set(RMLUI_VERSION_SUFFIX "-dev")
endif()
set(RMLUI_VERSION_SHORT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}${RMLUI_VERSION_SUFFIX})

# main RML Library (includes debugger)
file(GLOB_RECURSE RML_SOURCES 
    "RmlUi/Include/*.h" 
    "RmlUi/Include/*.hpp" 
    "RmlUi/Source/Core/*.cpp" 
    "RmlUi/Source/Core/*.h" 
    "RmlUi/Source/Lottie/*.cpp" 
    "RmlUi/Source/Lottie/*.h" 
    "RmlUi/Source/SVG/*.h" 
    "RmlUi/Source/SVG/*.cpp" 
    "RmlUi/Source/Debugger/*.cpp"
)
add_library(RmlCore STATIC ${RML_SOURCES})
target_include_directories(RmlCore PUBLIC "RmlUi/Include/" "rlottie/inc" "lunasvg/include")
list(APPEND CORE_PRIVATE_DEFS RMLUI_ENABLE_LOTTIE_PLUGIN)
list(APPEND CORE_PRIVATE_DEFS RMLUI_ENABLE_SVG_PLUGIN)
target_compile_definitions(RmlCore
    PRIVATE -DRMLUI_VERSION="${RMLUI_VERSION_SHORT}" ${CORE_PRIVATE_DEFS}
)
target_compile_features(RmlCore PRIVATE cxx_std_14)

# link freetype (disable freetype dependencies since we do not need them)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE INTERNAL "")
set(SKIP_INSTALL_ALL ON CACHE INTERNAL "")
set(HARFBUZZ_FOUND ON CACHE INTERNAL "")

if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR CMAKE_SYSTEM_NAME STREQUAL "tvOS")
    set(HB_IOS ON CACHE INTERNAL "")
endif()

set(HB_HAVE_FREETYPE ON CACHE INTERNAL "")
set(FREETYPE_FOUND ON CACHE INTERNAL "")
add_subdirectory("harfbuzz")

add_subdirectory("freetype")
target_include_directories(freetype PUBLIC "harfbuzz/src")
target_include_directories(harfbuzz PUBLIC "freetype/include")

SET(LOTTIE_TEST OFF CACHE INTERNAL "")
set(LIB_INSTALL_DIR "${CMAKE_BINARY_DIR}/rlottie" CACHE INTERNAL "")
set(LOTTIE_MODULE OFF CACHE INTERNAL "")
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(ARCH "arm"  CACHE INTERNAL "")
endif()
add_subdirectory("rlottie")
add_subdirectory("lunasvg")
target_link_libraries(RmlCore PUBLIC freetype rlottie lunasvg harfbuzz)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(RmlCore PRIVATE -DRmlCore_EXPORTS=1 -DEASTL_EXPORTS -DEASTDC_EXPORTS -DEATHREAD_EXPORTS)
else ()
    target_compile_definitions(RmlCore PUBLIC -DRMLUI_STATIC_LIB=1)
endif ()

if (APPLE)
    # Disables naked builtins from AssertMacros.h which
    # This prevents naming collisions such as those from the check()
    # function macro with LuaType::check
    target_compile_definitions(RmlCore PRIVATE -D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0)
endif ()
