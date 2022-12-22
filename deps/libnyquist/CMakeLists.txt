cmake_minimum_required(VERSION 3.10)

set(LIBNYQUIST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_MODULE_PATH ${LIBNYQUIST_ROOT}/cmake)

include(CXXhelpers)

if (CMAKE_OSX_ARCHITECTURES)
    if(CMAKE_OSX_SYSROOT MATCHES ".*iphoneos.*")
        # RtAudio is not portable to ios currently
        option(BUILD_EXAMPLE "Build example application" OFF)
    else()
        option(BUILD_EXAMPLE "Build example application" ON)
    endif()
else()
    option(BUILD_EXAMPLE "Build example application" ON)
endif()

#-------------------------------------------------------------------------------

# libopus

project(libopus)

file(GLOB third_opus_src
    "${LIBNYQUIST_ROOT}/third_party/opus/celt/*.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/opusfile/src/*.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/silk/*.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/silk/float/*.c"
)
set(lib_opus_src
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/analysis.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/mlp_data.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/mlp.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/opus_decoder.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/opus_multistream_decoder.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/opus_multistream_encoder.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/opus.c"
    "${LIBNYQUIST_ROOT}/third_party/opus/libopus/src/repacketizer.c"
)

add_library(libopus STATIC ${third_opus_src} ${lib_opus_src})

set_cxx_version(libopus)
_set_compile_options(libopus)

if (WIN32)
    _disable_warning(4244)
    _disable_warning(4018)
endif()

target_include_directories(libopus PRIVATE
    ${LIBNYQUIST_ROOT}/third_party/libogg/include
    ${LIBNYQUIST_ROOT}/third_party/opus/celt
    ${LIBNYQUIST_ROOT}/third_party/opus/libopus/include
    ${LIBNYQUIST_ROOT}/third_party/opus/opusfile/include
    ${LIBNYQUIST_ROOT}/third_party/opus/opusfile/src/include
    ${LIBNYQUIST_ROOT}/third_party/opus/silk
    ${LIBNYQUIST_ROOT}/third_party/opus/silk/float)

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties(libnyquist PROPERTIES PREFIX "../")
    set_target_properties(libopus PROPERTIES IMPORT_PREFIX "../")
endif()

target_compile_definitions(libopus PRIVATE OPUS_BUILD)
target_compile_definitions(libopus PRIVATE USE_ALLOCA)

set_target_properties(libopus
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(libopus PROPERTIES OUTPUT_NAME_DEBUG libopus_d)

install (TARGETS libopus
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         RUNTIME DESTINATION bin)

install (TARGETS libopus DESTINATION lib)

# folders

source_group(src FILES ${third_opus_src})

#-------------------------------------------------------------------------------

# libwavpack

project(libwavpack)

if(MSVC)
    # Disable warning C4996 regarding fopen(), strcpy(), etc.
    _add_define("_CRT_SECURE_NO_WARNINGS")

    # Disable warning C4996 regarding unchecked iterators for std::transform,
    # std::copy, std::equal, et al.
    _add_define("_SCL_SECURE_NO_WARNINGS")

    # Make sure WinDef.h does not define min and max macros which
    # will conflict with std::min() and std::max().
    _add_define("NOMINMAX")
endif()

add_definitions(${_NQR_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_NQR_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")

file(GLOB third_wavpack_src "${LIBNYQUIST_ROOT}/third_party/wavpack/src/*.c" "${LIBNYQUIST_ROOT}/third_party/wavpack/src/*.h")

add_library(libwavpack STATIC ${third_wavpack_src})

set_cxx_version(libwavpack)
_set_compile_options(libwavpack)

if (WIN32)
    _disable_warning(181)
    _disable_warning(111)
    _disable_warning(4267)
    _disable_warning(4996)
    _disable_warning(4244)
    _disable_warning(4701)
    _disable_warning(4702)
    _disable_warning(4133)
    _disable_warning(4100)
    _disable_warning(4127)
    _disable_warning(4206)
    _disable_warning(4312)
    _disable_warning(4505)
    _disable_warning(4365)
    _disable_warning(4005)
    _disable_warning(4013)
    _disable_warning(4334)
    _disable_warning(4703)
endif()

target_include_directories(libwavpack PRIVATE ${LIBNYQUIST_ROOT}/third_party/wavpack/include)

if (MSVC_IDE)
    set_target_properties(libwavpack PROPERTIES IMPORT_PREFIX "../")
endif()

set_target_properties(libwavpack
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(libwavpack PROPERTIES OUTPUT_NAME_DEBUG libwavpack_d)

install(TARGETS libwavpack
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install (TARGETS libwavpack DESTINATION lib)

#-------------------------------------------------------------------------------

# libnyquist static library

project(libnyquist)

file(GLOB nyquist_include "${LIBNYQUIST_ROOT}/include/libnyquist/*")
file(GLOB nyquist_src     "${LIBNYQUIST_ROOT}/src/*")
file(GLOB wavpack_src     "${LIBNYQUIST_ROOT}/third_party/wavpack/src/*.c" "${LIBNYQUIST_ROOT}/third_party/wavpack/src/*.h")

add_library(libnyquist STATIC
    ${nyquist_include}
    ${nyquist_src}
)

set_cxx_version(libnyquist)
_set_compile_options(libnyquist)

if (WIN32)
    _disable_warning(4244)
    _disable_warning(4018)
endif()

target_include_directories(libnyquist
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${LIBNYQUIST_ROOT}/include>
  PRIVATE
    ${LIBNYQUIST_ROOT}/include/libnyquist
    ${LIBNYQUIST_ROOT}/third_party
    ${LIBNYQUIST_ROOT}/third_party/FLAC/src/include
    ${LIBNYQUIST_ROOT}/third_party/libogg/include
    ${LIBNYQUIST_ROOT}/third_party/libvorbis/include
    ${LIBNYQUIST_ROOT}/third_party/libvorbis/src
    ${LIBNYQUIST_ROOT}/third_party/musepack/include
    ${LIBNYQUIST_ROOT}/third_party/opus/celt
    ${LIBNYQUIST_ROOT}/third_party/opus/libopus/include
    ${LIBNYQUIST_ROOT}/third_party/opus/opusfile/include
    ${LIBNYQUIST_ROOT}/third_party/opus/opusfile/src/include
    ${LIBNYQUIST_ROOT}/third_party/opus/silk
    ${LIBNYQUIST_ROOT}/third_party/opus/silk/float
    ${LIBNYQUIST_ROOT}/third_party/wavpack/include
    ${LIBNYQUIST_ROOT}/src
)

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties(libnyquist PROPERTIES PREFIX "../")
    set_target_properties(libnyquist PROPERTIES IMPORT_PREFIX "../")
endif()

set_target_properties(libnyquist PROPERTIES OUTPUT_NAME_DEBUG libnyquist_d)

set_target_properties(libnyquist
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(libnyquist PRIVATE libopus libwavpack)

install(TARGETS libnyquist
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(TARGETS libnyquist DESTINATION lib)

# folders
source_group(src FILES ${nyquist_src})

#-------------------------------------------------------------------------------

# libnyquist-examples

if(BUILD_EXAMPLE)

    set(NQR_EXAMPLE_APP_NAME "libnyquist-examples")

    set(
        EXAMPLE_SOURCES
        ${LIBNYQUIST_ROOT}/examples/src/Main.cpp
        ${LIBNYQUIST_ROOT}/examples/src/AudioDevice.cpp
        ${LIBNYQUIST_ROOT}/examples/src/AudioDevice.h
        ${LIBNYQUIST_ROOT}/examples/src/RingBuffer.h
        ${LIBNYQUIST_ROOT}/third_party/rtaudio/RtAudio.cpp
        ${LIBNYQUIST_ROOT}/third_party/rtaudio/RtAudio.h
    )

    add_executable(${NQR_EXAMPLE_APP_NAME} ${EXAMPLE_SOURCES})

    if(WIN32)
        target_compile_definitions(${NQR_EXAMPLE_APP_NAME} PRIVATE __WINDOWS_WASAPI__)
    elseif(APPLE)
        target_compile_definitions(${NQR_EXAMPLE_APP_NAME} PRIVATE __MACOSX_CORE__)
    elseif(LIBNYQUIST_JACK)
        target_compile_definitions(${NQR_EXAMPLE_APP_NAME} PRIVATE __UNIX_JACK__)
        target_link_libraries(${NQR_EXAMPLE_APP_NAME} PRIVATE jack pthread)
    elseif(LIBNYQUIST_PULSE)
        target_compile_definitions(${NQR_EXAMPLE_APP_NAME} PRIVATE __LINUX_PULSE__)
        target_link_libraries(${NQR_EXAMPLE_APP_NAME} PRIVATE pulse pthread)
    elseif(LIBNYQUIST_ASOUND)
        target_compile_definitions(${NQR_EXAMPLE_APP_NAME} PRIVATE __LINUX_ALSA__)
        target_link_libraries(${NQR_EXAMPLE_APP_NAME} PRIVATE asound pthread)
    else()
        message(FATAL, "On Linux, one of LIBNYQUIST_JACK, LIBNYQUIST_PULSE, or LIBNYQUIST_ASOUND must be set.")
    endif()

    target_include_directories(${NQR_EXAMPLE_APP_NAME} PRIVATE
        ${LIBNYQUIST_ROOT}/examples/src
        ${LIBNYQUIST_ROOT}/third_party
    )
    target_link_libraries(${NQR_EXAMPLE_APP_NAME} PRIVATE libnyquist)

    set_target_properties(${NQR_EXAMPLE_APP_NAME}
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    if(APPLE)
        target_link_libraries(${NQR_EXAMPLE_APP_NAME} PRIVATE
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework Accelerate"
            "-framework CoreAudio"
            "-framework Cocoa"
        )
    ENDIF(APPLE)

endif()
