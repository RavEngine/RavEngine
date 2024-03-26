cmake_minimum_required(VERSION 3.20)

project(SteamAudio)

option(SA_BUILD_FLATC "Whether to compile the bundled flatbuffers. If set to OFF (for example, if cross-compiling), then the caller must set FlatBuffers_EXECUTABLE" ON)
option(SA_BUILD_ZLIB "Build the bundled Zlib" ON)
option(SA_ENABLE_EMBREE "Enable Embree support" OFF)

if (SA_BUILD_ZLIB)
    add_subdirectory(zlib EXCLUDE_FROM_ALL)    
	set(ZLIB_LIBRARY zlibstatic)
	set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/zlib" CACHE INTERNAL "")
endif()

set(PFFFT_USE_FFTPACK OFF)
add_subdirectory(pffft EXCLUDE_FROM_ALL)

set(BUILD_TESTS OFF CACHE INTERNAL "")
add_subdirectory(libmysofa EXCLUDE_FROM_ALL)

if (SA_BUILD_FLATC)
    set(FLATBUFFERS_BUILD_TESTS OFF CACHE INTERNAL "")
    set(FLATBUFFERS_INSTALL OFF CACHE INTERNAL "")
    add_subdirectory(flatbuffers EXCLUDE_FROM_ALL)
    set_target_properties(flatc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flatbuffers/")
    if(CMAKE_HOST_WIN32)
        set(FlatBuffers_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/flatbuffers/$<CONFIGURATION>/flatc.exe")
    else()
        set(FlatBuffers_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/flatbuffers/$<CONFIGURATION>/flatc")
    endif()
endif()

if (SA_ENABLE_EMBREE)
    set(ISPC_INCLUDE_EXAMPLES OFF)
    add_subdirectory(ispc EXCLUDE_FROM_ALL)

    set(EMBREE_TUTORIALS OFF CACHE INTERNAL "")
    add_subdirectory(embree EXCLUDE_FROM_ALL)

    set(Embree_sse2_LIBRARY embree)
    set(Embree_sse4_LIBRARY embree_sse42)
    set(Embree_avx_LIBRARY embree_avx)
    set(Embree_avx2_LIBRARY embree_avx2)
    set(Embree_lexers_LIBRARY lexers)
    set(Embree_math_LIBRARY math)
    set(Embree_simd_LIBRARY simd)
    set(Embree_sys_LIBRARY sys)
    set(Embree_tasking_LIBRARY tasking)
    set(Embree_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/embree/include/embree2")
endif()



set(PFFFT_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake/")
set(MySOFA_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake/")
set(FlatBuffers_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake/")


set(STEAMAUDIO_BUILD_TESTS OFF CACHE INTERNAL "")
set(STEAMAUDIO_BUILD_BENCHMARKS OFF CACHE INTERNAL "")
set(STEAMAUDIO_BUILD_SAMPLES OFF CACHE INTERNAL "")
set(STEAMAUDIO_BUILD_ITESTS OFF CACHE INTERNAL "")
set(STEAMAUDIO_BUILD_DOCS OFF CACHE INTERNAL "")
set(STEAMAUDIO_ENABLE_EMBREE ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")

set(PFFFT_LIBRARY PFFFT)
set(PFFFT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/pffft")

set(MySOFA_LIBRARY mysofa-static)
set(MySOFA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/libmysofa/src/hrtf")

set(FlatBuffers_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/flatbuffers/include")
add_subdirectory(steam-audio/core)

target_link_libraries(phonon PUBLIC pffft)

add_dependencies(phonon flatc)