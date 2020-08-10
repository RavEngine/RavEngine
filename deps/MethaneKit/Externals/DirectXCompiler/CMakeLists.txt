set(TARGET DirectXCompiler)

include(MethaneModules)
include(ExternalProject)

get_platform_dir()

add_library(${TARGET} STATIC
    DirectXCompiler.cpp
)

set(BINARIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/binaries/${PLATFORM_DIR})

if(WIN32)
    get_target_arch(WIN_ARCH)
    set(BINARIES_DIR ${BINARIES_DIR}-${WIN_ARCH})
endif()

set(LIB_DIR ${BINARIES_DIR}/lib)
set(BIN_DIR ${BINARIES_DIR}/bin)

if(WIN32)

    if(${WIN_ARCH} STREQUAL "x64")
       set(BINARIES_MD5 cb428de9a084987323376dee63611a56)
    elseif(${WIN_ARCH} STREQUAL "x86")
       set(BINARIES_MD5 24df1b6f9e6403d320c34b3ddc04b9f8)
    endif()

    set(LIBRARIES
        "${LIB_DIR}/dxcompiler.lib")
    set(PREREQUISITE_BINARIES_LIST 
        "${BIN_DIR}/dxcompiler.dll")

elseif(APPLE)

    set(BINARIES_MD5 6f647d917debb826fef66c86f7bfdbf4)

endif()

if(WIN32 OR APPLE)

    ExternalProject_Add(${TARGET}Unpack
        URL ${BINARIES_DIR}.zip
        URL_MD5 ${BINARIES_MD5}
        SOURCE_DIR ${BINARIES_DIR}
        BUILD_BYPRODUCTS ${LIBRARIES}
        BUILD_IN_SOURCE TRUE
        EXCLUDE_FROM_ALL TRUE
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        STEP_TARGETS build
    )

    add_dependencies(${TARGET} ${TARGET}Unpack-build)

endif()

set_target_properties(${TARGET} PROPERTIES PREREQUISITE_BINARIES "${PREREQUISITE_BINARIES_LIST}")

target_link_libraries(${TARGET}
    PUBLIC
        ${LIBRARIES}
)

target_include_directories(${TARGET}
    PUBLIC
        ./include
)

set_target_properties(${TARGET}
    PROPERTIES
    FOLDER Externals
)
