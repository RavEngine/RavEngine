find_path(PD_INCLUDE_BASEDIR "m_pd.h" PATH_SUFFIXES "pd")
set(PD_IMP_DEF "${PROJECT_SOURCE_DIR}/plugins/puredata/external/pd/bin/pd.def")

if(PD_INCLUDE_BASEDIR)
    message(STATUS "Puredata headers: ${PUREDATA_INCLUDE_DIR}")
else()
    message(STATUS "Puredata headers not found, using our own")
    set(PD_INCLUDE_BASEDIR "${PROJECT_SOURCE_DIR}/plugins/puredata/external/pd/include")
endif()

if(WIN32)
    set(PUREDATA_SUFFIX ".dll")
elseif(APPLE)
    set(PUREDATA_SUFFIX ".pd_darwin")
else()
    set(PUREDATA_SUFFIX ".pd_linux")
endif()

if(APPLE)
    set(PDPLUGIN_INSTALL_DIR "$ENV{HOME}/Library/Pd" CACHE STRING
    "Install destination for Puredata bundle [default: $ENV{HOME}/Library/Pd]")
elseif(MSVC)
    set(PDPLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/Pd/extra" CACHE STRING
    "Install destination for Puredata bundle [default: ${CMAKE_INSTALL_PREFIX}/Pd/extra]")
else()
    set(PDPLUGIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/pd/extra" CACHE STRING
    "Install destination for Puredata bundle [default: ${CMAKE_INSTALL_PREFIX}/lib/pd/extra]")
endif()

if(WIN32)
    add_library(pdex-implib STATIC IMPORTED)
    if(MSVC)
        add_custom_command(
            OUTPUT "pd.lib"
            COMMAND "lib" "/out:pd.lib" "/def:${PD_IMP_DEF}" "/machine:${MSVC_C_ARCHITECTURE_ID}"
            DEPENDS "${PD_IMP_DEF}")
        add_custom_target(pdex-implib-generated
            DEPENDS "pd.lib")
        set_target_properties(pdex-implib PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/pd.lib")
    else()
        find_program(PD_DLLTOOL_PROGRAM "dlltool")
        if(NOT PD_DLLTOOL_PROGRAM)
            message(FATAL_ERROR "Cannot find dlltool")
        endif()
        add_custom_command(
            OUTPUT "libpd.dll.a"
            COMMAND "${PD_DLLTOOL_PROGRAM}" "-l" "libpd.dll.a" "-d" "${PD_IMP_DEF}"
            DEPENDS "${PD_IMP_DEF}")
        add_custom_target(pdex-implib-generated
            DEPENDS "libpd.dll.a")
        set_target_properties(pdex-implib PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/libpd.dll.a")
    endif()
    add_dependencies(pdex-implib pdex-implib-generated)
endif()

function(add_pd_external TARGET)
    add_library("${TARGET}" MODULE ${ARGN})
    target_include_directories("${TARGET}" PRIVATE "${PD_INCLUDE_BASEDIR}")
    set_target_properties("${TARGET}" PROPERTIES
        PREFIX ""
        SUFFIX "${PUREDATA_SUFFIX}")
    if(APPLE)
        set_property(TARGET "${TARGET}" APPEND_STRING
            PROPERTY LINK_FLAGS " -Wl,-undefined,suppress,-flat_namespace")
    elseif(WIN32)
        target_link_libraries("${TARGET}" PRIVATE pdex-implib)
    endif()
endfunction()
