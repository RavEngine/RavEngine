cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(RmlUi VERSION 4.0) 

set(RMLUI_VERSION_RELEASE false)
set(ENABLE_LOTTIE_PLUGIN ON CACHE INTERNAL "") # rlottie plugin

if(RMLUI_VERSION_RELEASE)
	set(RMLUI_VERSION_SUFFIX "")
else()
	set(RMLUI_VERSION_SUFFIX "-dev")
endif()
set(RMLUI_VERSION_SHORT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}${RMLUI_VERSION_SUFFIX})

# main RML Library (includes debugger)
file(GLOB_RECURSE RML_SOURCES "RmlUi/Include/*.h" "RmlUi/Include/*.hpp" "RmlUi/Source/Core/*.cpp" "RmlUi/Source/Debugger/*.cpp")
add_library(RmlCore STATIC ${RML_SOURCES})
target_include_directories(RmlCore PUBLIC "RmlUi/Include/")
target_compile_definitions(RmlCore
    PRIVATE -DRMLUI_VERSION="${RMLUI_VERSION_SHORT}"
)

# link freetype (disable freetype dependencies since we do not need them)
set(CMAKE_DISABLE_FIND_PACKAGE_PNG ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 ON CACHE INTERNAL "")
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE INTERNAL "")
add_subdirectory("freetype")
SET(LOTTIE_TEST OFF CACHE INTERNAL "")
set(LIB_INSTALL_DIR "${CMAKE_BINARY_DIR}/rlottie" CACHE INTERNAL "")
add_subdirectory("rlottie")
target_link_libraries(RmlCore PUBLIC freetype rlottie)

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
