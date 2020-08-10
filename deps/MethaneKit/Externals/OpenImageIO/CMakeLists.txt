set(TARGET OpenImageIO)

include(MethaneModules)
include(ExternalProject)

get_platform_dir()

file(GLOB OPEN_EXR_INCLUDES include/OpenEXR/*.h)
file(GLOB OPEN_IMAGE_IO_INCLUDES include/OpenImageIO/*.h)

add_library(${TARGET} STATIC
    ${OPEN_EXR_INCLUDES}
    ${OPEN_IMAGE_IO_INCLUDES}
    OpenImageIO.cpp
)

set(BINARIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/binaries/${PLATFORM_DIR})
set(LIB_DIR ${BINARIES_DIR}/lib)
set(BIN_DIR ${BINARIES_DIR}/bin)

if (WIN32)

    set(LIBRARIES
        ${LIB_DIR}/OpenImageIO.lib
        ${LIB_DIR}/OpenImageIO_Util.lib)

    set(BINARIES_MD5 ddf7e262aa14c4975768e37cdb7477d1)
    set(PREREQUISITE_BINARIES_LIST
        "${BIN_DIR}/boost_chrono-vc141-mt-1_65_1.dll"
        "${BIN_DIR}/boost_filesystem-vc141-mt-1_65_1.dll"
        "${BIN_DIR}/boost_system-vc141-mt-1_65_1.dll"
        "${BIN_DIR}/boost_thread-vc141-mt-1_65_1.dll"
        "${BIN_DIR}/Half-2_3.dll"
        "${BIN_DIR}/Iex-2_3.dll"
        "${BIN_DIR}/IexMath-2_3.dll"
        "${BIN_DIR}/IlmImf-2_3.dll"
        "${BIN_DIR}/IlmImfUtil-2_3.dll"
        "${BIN_DIR}/IlmThread-2_3.dll"
        "${BIN_DIR}/Imath-2_3.dll"
        "${BIN_DIR}/jpeg62.dll"
        "${BIN_DIR}/libpng16.dll"
        "${BIN_DIR}/OpenImageIO.dll"
        "${BIN_DIR}/OpenImageIO_Util.dll"
        "${BIN_DIR}/Ptex.dll"
        "${BIN_DIR}/tiff.dll"
        "${BIN_DIR}/tiffxx.dll"
        "${BIN_DIR}/turbojpeg.dll"
        "${BIN_DIR}/zlib.dll"
    )

elseif(APPLE)

    set(LIBRARIES
        ${LIB_DIR}/libOpenImageIO.dylib
        ${LIB_DIR}/libOpenImageIO_Util.dylib)

    set(BINARIES_MD5 460526794ac9a093c5943fe532553fce)
    set(PREREQUISITE_BINARIES_LIST
        "${LIB_DIR}/libboost_atomic.dylib"
        "${LIB_DIR}/libboost_chrono.dylib"
        "${LIB_DIR}/libboost_date_time.dylib"
        "${LIB_DIR}/libboost_filesystem.dylib"
        "${LIB_DIR}/libboost_system.dylib"
        "${LIB_DIR}/libboost_thread.dylib"
        "${LIB_DIR}/libHalf-2_3.2.3.0.dylib"
        "${LIB_DIR}/libHalf-2_3.24.dylib"
        "${LIB_DIR}/libHalf-2_3.dylib"
        "${LIB_DIR}/libIex-2_3.2.3.0.dylib"
        "${LIB_DIR}/libIex-2_3.24.dylib"
        "${LIB_DIR}/libIex-2_3.dylib"
        "${LIB_DIR}/libIexMath-2_3.2.3.0.dylib"
        "${LIB_DIR}/libIexMath-2_3.24.dylib"
        "${LIB_DIR}/libIexMath-2_3.dylib"
        "${LIB_DIR}/libIlmImf-2_3.2.3.0.dylib"
        "${LIB_DIR}/libIlmImf-2_3.24.dylib"
        "${LIB_DIR}/libIlmImf-2_3.dylib"
        "${LIB_DIR}/libIlmImfUtil-2_3.2.3.0.dylib"
        "${LIB_DIR}/libIlmImfUtil-2_3.24.dylib"
        "${LIB_DIR}/libIlmImfUtil-2_3.dylib"
        "${LIB_DIR}/libIlmThread-2_3.2.3.0.dylib"
        "${LIB_DIR}/libIlmThread-2_3.24.dylib"
        "${LIB_DIR}/libIlmThread-2_3.dylib"
        "${LIB_DIR}/libImath-2_3.2.3.0.dylib"
        "${LIB_DIR}/libImath-2_3.24.dylib"
        "${LIB_DIR}/libImath-2_3.dylib"
        "${LIB_DIR}/libjpeg.8.dylib"
        "${LIB_DIR}/libjpeg.dylib"
        "${LIB_DIR}/libOpenImageIO.2.0.5.dylib"
        "${LIB_DIR}/libOpenImageIO.2.0.dylib"
        "${LIB_DIR}/libOpenImageIO.dylib"
        "${LIB_DIR}/libOpenImageIO_Util.2.0.5.dylib"
        "${LIB_DIR}/libOpenImageIO_Util.2.0.dylib"
        "${LIB_DIR}/libOpenImageIO_Util.dylib"
        "${LIB_DIR}/libpng.dylib"
        "${LIB_DIR}/libpng16.16.22.0.dylib"
        "${LIB_DIR}/libpng16.16.dylib"
        "${LIB_DIR}/libpng16.dylib"
        "${LIB_DIR}/libPtex.dylib"
        "${LIB_DIR}/libtiff.5.dylib"
        "${LIB_DIR}/libtiff.dylib"
        "${LIB_DIR}/libturbojpeg.0.2.0.dylib"
        "${LIB_DIR}/libturbojpeg.0.dylib"
        "${LIB_DIR}/libturbojpeg.dylib"
        # Binaries list command: "env LC_COLLATE=en_US.UTF-8 ls -A ./binaries/MacOS/lib/*.dylib | sort -f"
    )

endif()

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
