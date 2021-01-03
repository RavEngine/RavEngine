cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(RmlUi LANGUAGES C CXX VERSION 3.3)

# main RML Library
file(GLOB_RECURSE RML_SOURCES "RmlUi/Include/*.h" "RmlUi/Include/*.hpp" "RmlUi/Source/Core/*.cpp")
add_library(RmlCore STATIC ${RML_SOURCES})
target_include_directories(RmlCore PUBLIC "RmlUi/Include/")
target_compile_definitions(RmlCore
    PRIVATE -DRMLUI_VERSION="${RMLUI_VERSION_SHORT}" -D$<$<NOT:$<CONFIG:Debug>>:N>DEBUG=1 PUBLIC -DRMLUI_NO_THIRDPARTY_CONTAINERS=1
)

if (APPLE)
    # Disables naked builtins from AssertMacros.h which
    # This prevents naming collisions such as those from the check()
    # function macro with LuaType::check
    target_compile_definitions(RmlCore PRIVATE -D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0)
	target_compile_definitions(RmlDebugger PRIVATE -D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0)
endif ()

# RML debugger library
file(GLOB_RECURSE RML_DBG "RmlUi/Include/Debugger/*.h" "RmlUi/Include/Debugger/*.hpp" "RmlUi/Source/Debugger/*.cpp")
add_library(RmlDebugger STATIC ${RML_DBG})
target_compile_definitions(RmlDebugger
    PRIVATE -DRMLUI_VERSION="${RMLUI_VERSION_SHORT}" -D$<$<NOT:$<CONFIG:Debug>>:N>DEBUG=1 PUBLIC -DRMLUI_NO_THIRDPARTY_CONTAINERS=1
)

# link freetype
add_subdirectory("freetype")
target_link_libraries(RmlCore PUBLIC freetype)
target_link_libraries(RmlDebugger PUBLIC freetype)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(RmlCore PRIVATE -DRmlCore_EXPORTS=1 -DRmlDebugger_EXPORTS=1 -DEASTL_EXPORTS -DEASTDC_EXPORTS -DEATHREAD_EXPORTS)
	target_compile_definitions(RmlDebugger PRIVATE -DRmlCore_EXPORTS=1 -DRmlDebugger_EXPORTS=1 -DEASTL_EXPORTS -DEASTDC_EXPORTS -DEATHREAD_EXPORTS)
else ()
    target_compile_definitions(RmlCore PUBLIC -DRMLUI_STATIC_LIB=1)
	target_compile_definitions(RmlDebugger PUBLIC -DRMLUI_STATIC_LIB=1)
endif ()
