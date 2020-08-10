if (NOT WIN32)
    return()
endif()

set(TARGET D3DCompilerToDXC)

add_library(${TARGET} STATIC
    d3dcomp.cpp
)

target_link_libraries(${TARGET} PRIVATE DirectXCompiler)

set_target_properties(${TARGET}
    PROPERTIES
    FOLDER Externals
    PREREQUISITE_MODULES "DirectXCompiler"
)
